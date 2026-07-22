#!/bin/bash

set -euo pipefail

# Use real paths so /var and /private/var are treated as the same place.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
APPLE_DIR="$(cd "$SCRIPT_DIR/.." && pwd -P)"
PROJECT_DIR="$(cd "$APPLE_DIR/../.." && pwd -P)"
STAGING_DIR=""
DESTINATION=""

fail() {
  echo "$1" >&2
  return 1
}

require_command() {
  command -v "$1" >/dev/null 2>&1 || fail "Required command is unavailable: $1"
}

show_usage() {
  cat <<'EOF'
Usage:
  ./platform/Apple/Scripts/sync-apple-package.sh <apple-repository-path>

Synchronizes the publishable Apple SDK sources and XCFrameworks into an
existing, clean local Git repository. The destination repository owns its
Package.swift, README, .gitignore, and GitHub configuration.
EOF
}

cleanup() {
  if [ -n "$STAGING_DIR" ] && [ -d "$STAGING_DIR" ]; then
    rm -rf "$STAGING_DIR"
  fi
}

resolve_destination() {
  local requested_path="$1"

  if [ ! -d "$requested_path" ]; then
    fail "Apple package repository does not exist or is not a directory: $requested_path"
    return 1
  fi

  DESTINATION="$(cd "$requested_path" && pwd -P)" || return 1
}

assert_source_repository() {
  local source_root
  local changes
  local required_path
  local required_paths=(
    "$PROJECT_DIR/platform/Apple/SweetLine"
    "$PROJECT_DIR/prebuilt/ios/SweetLineCoreIOS.xcframework.zip"
    "$PROJECT_DIR/prebuilt/macos/SweetLineCoreMacOS.xcframework.zip"
    "$PROJECT_DIR/LICENSE"
  )

  source_root="$(git -C "$PROJECT_DIR" rev-parse --show-toplevel 2>/dev/null)" || {
    fail "SweetLine source directory is not a Git repository: $PROJECT_DIR"
    return 1
  }
  source_root="$(cd "$source_root" && pwd -P)" || return 1
  if [ "$source_root" != "$PROJECT_DIR" ]; then
    fail "Script must run from the SweetLine repository: $PROJECT_DIR"
    return 1
  fi

  # Copy only committed source. This keeps each release tied to a source commit.
  changes="$(git -C "$PROJECT_DIR" status --porcelain)" || return 1
  if [ -n "$changes" ]; then
    fail "SweetLine source repository has uncommitted changes. Commit or discard them before synchronization."
    return 1
  fi

  for required_path in "${required_paths[@]}"; do
    if [ ! -e "$required_path" ]; then
      fail "Required Apple package input is missing: $required_path"
      return 1
    fi
  done
}

assert_target_repository() {
  local target_root
  local changes
  local probe_path
  local ignored_path
  # These sample paths must be tracked by Git. They are not paths to ignore.
  # Info.plist always exists in an XCFramework, so it also checks if a parent folder is ignored.
  local managed_path_probes=(
    "SweetLine/SweetLine.swift"
    "Vendor/iOS/SweetLineCoreIOS.xcframework/Info.plist"
    "Vendor/macOS/SweetLineCoreMacOS.xcframework/Info.plist"
  )

  target_root="$(git -C "$DESTINATION" rev-parse --show-toplevel 2>/dev/null)" || {
    fail "Apple package destination is not a Git repository: $DESTINATION"
    return 1
  }
  target_root="$(cd "$target_root" && pwd -P)" || return 1
  if [ "$target_root" != "$DESTINATION" ]; then
    fail "Apple package path must be the destination Git repository root: $target_root"
    return 1
  fi

  # Keep the two repositories apart. A delete sync must never point into the source repo.
  if [ "$DESTINATION" = "$PROJECT_DIR" ]; then
    fail "Apple package destination cannot be the SweetLine source repository."
    return 1
  fi
  case "$DESTINATION/" in
    "$PROJECT_DIR/"*)
      fail "Apple package destination cannot be inside the SweetLine source repository: $DESTINATION"
      return 1
      ;;
  esac
  case "$PROJECT_DIR/" in
    "$DESTINATION/"*)
      fail "SweetLine source repository cannot be inside the Apple package destination: $DESTINATION"
      return 1
      ;;
  esac

  if [ ! -f "$DESTINATION/Package.swift" ]; then
    fail "Apple package destination must provide its release Package.swift: $DESTINATION/Package.swift"
    return 1
  fi

  changes="$(git -C "$DESTINATION" status --porcelain)" || return 1
  if [ -n "$changes" ]; then
    fail "Apple package repository has uncommitted changes. Commit or discard them before synchronization."
    return 1
  fi

  # --no-index checks ignore rules even when the file is already tracked.
  for probe_path in "${managed_path_probes[@]}"; do
    if ignored_path="$(git -C "$DESTINATION" check-ignore --no-index -v "$probe_path" 2>/dev/null)"; then
      fail "Apple package repository ignores a managed path: $probe_path ($ignored_path)"
      return 1
    fi
  done
}

prepare_staging() {
  local ios_archive="$PROJECT_DIR/prebuilt/ios/SweetLineCoreIOS.xcframework.zip"
  local macos_archive="$PROJECT_DIR/prebuilt/macos/SweetLineCoreMacOS.xcframework.zip"

  # Build a full package in a temp folder before changing the destination.
  STAGING_DIR="$(mktemp -d "${TMPDIR:-/tmp}/sweetline-apple-sync.XXXXXX")" || return 1
  mkdir -p \
    "$STAGING_DIR/SweetLine" \
    "$STAGING_DIR/Vendor/iOS" \
    "$STAGING_DIR/Vendor/macOS"

  # The destination owns Package.swift. Copy it here only to test the staged package.
  cp "$DESTINATION/Package.swift" "$STAGING_DIR/Package.swift"
  cp "$PROJECT_DIR/LICENSE" "$STAGING_DIR/LICENSE"
  rsync -a --delete "$PROJECT_DIR/platform/Apple/SweetLine/" "$STAGING_DIR/SweetLine/"
  ditto -x -k "$ios_archive" "$STAGING_DIR/Vendor/iOS"
  ditto -x -k "$macos_archive" "$STAGING_DIR/Vendor/macOS"
}

validate_staging_layout() {
  local required_path
  local required_paths=(
    "$STAGING_DIR/Package.swift"
    "$STAGING_DIR/LICENSE"
    "$STAGING_DIR/SweetLine/SweetLine.swift"
    "$STAGING_DIR/Vendor/iOS/SweetLineCoreIOS.xcframework/Info.plist"
    "$STAGING_DIR/Vendor/macOS/SweetLineCoreMacOS.xcframework/Info.plist"
  )

  for required_path in "${required_paths[@]}"; do
    if [ ! -e "$required_path" ]; then
      fail "Staged Apple package is incomplete: $required_path"
      return 1
    fi
  done
}

run_external() {
  local working_dir="$1"
  shift
  printf '\n> '
  printf '%q ' "$@"
  printf '\n'
  (cd "$working_dir" && "$@")
}

validate_staging_package() {
  # Check that Package.swift can read the staged paths, then build the package.
  run_external "$STAGING_DIR" swift package describe >/dev/null
  run_external "$STAGING_DIR" swift build
}

apply_staging() {
  # --delete is limited to the two folders owned by this script.
  mkdir -p "$DESTINATION/SweetLine" "$DESTINATION/Vendor"
  rsync -a --delete "$STAGING_DIR/SweetLine/" "$DESTINATION/SweetLine/"
  rsync -a --delete "$STAGING_DIR/Vendor/" "$DESTINATION/Vendor/"
  cp "$STAGING_DIR/LICENSE" "$DESTINATION/LICENSE"
}

show_result() {
  local changes

  changes="$(git -C "$DESTINATION" status --short)" || return 1
  if [ -z "$changes" ]; then
    echo
    echo "Apple package repository is already synchronized."
    return 0
  fi

  echo
  echo "Apple package synchronization completed."
  echo
  echo "Source:"
  echo "  $PROJECT_DIR"
  echo
  echo "Destination:"
  echo "  $DESTINATION"
  echo
  echo "Managed paths:"
  echo "  SweetLine/"
  echo "  Vendor/"
  echo "  LICENSE"
  echo
  echo "Changes:"
  printf '%s\n' "$changes"
  echo
  git -C "$DESTINATION" diff --stat
  echo "Review and commit these changes from the destination repository."
}

main() {
  if [ "$#" -eq 1 ] && { [ "$1" = "-h" ] || [ "$1" = "--help" ]; }; then
    show_usage
    return 0
  fi
  if [ "$#" -ne 1 ]; then
    show_usage >&2
    return 1
  fi
  if [ "$(uname -s)" != "Darwin" ]; then
    fail "sync-apple-package.sh only supports macOS."
    return 1
  fi

  require_command git || return 1
  require_command swift || return 1
  require_command ditto || return 1
  require_command rsync || return 1

  resolve_destination "$1" || return 1
  assert_source_repository || return 1
  assert_target_repository || return 1
  prepare_staging || return 1
  validate_staging_layout || return 1
  validate_staging_package || return 1
  apply_staging || return 1
  show_result
}

# Remove the temp folder after success or failure.
trap cleanup EXIT
main "$@"
