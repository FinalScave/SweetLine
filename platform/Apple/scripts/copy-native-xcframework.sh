#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPLE_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="${SWEETLINE_REPO_ROOT:-$(cd "${APPLE_DIR}/../.." && pwd)}"
PREBUILT_DIR="${SWEETLINE_PREBUILT_DIR:-${REPO_ROOT}/prebuilt}"
OUTPUT_DIR="${SWEETLINE_APPLE_OUTPUT_DIR:-${APPLE_DIR}/binaries}"
BUILD_SCOPE="${1:-all}"

IOS_DEFAULT_NAME="SweetLineCoreIOS.xcframework"
OSX_DEFAULT_NAME="SweetLineCoreOSX.xcframework"

require_archive() {
  local archive_path="$1"

  if [[ ! -f "$archive_path" ]]; then
    echo "XCFramework archive not found: $archive_path" >&2
    exit 1
  fi
}

extract_archive_to_name() {
  local archive_path="$1"
  local target_name="$2"
  local temp_dir=""
  local extracted_dir=""
  local target_path="$OUTPUT_DIR/$target_name"

  require_archive "$archive_path"
  mkdir -p "$OUTPUT_DIR"

  temp_dir="$(mktemp -d)"
  unzip -q "$archive_path" -d "$temp_dir"

  extracted_dir="$temp_dir/$(basename "$archive_path" .zip)"
  if [[ ! -d "$extracted_dir" ]]; then
    echo "Extracted XCFramework directory not found: $extracted_dir" >&2
    rm -rf "$temp_dir"
    exit 1
  fi

  rm -rf "$target_path"
  mv "$extracted_dir" "$target_path"
  rm -rf "$temp_dir"

  echo "Installed $target_path"
}

copy_ios() {
  local target_name="${1:-$IOS_DEFAULT_NAME}"
  extract_archive_to_name "$PREBUILT_DIR/ios/${IOS_DEFAULT_NAME}.zip" "$target_name"
}

copy_osx() {
  local target_name="${1:-$OSX_DEFAULT_NAME}"
  extract_archive_to_name "$PREBUILT_DIR/osx/${OSX_DEFAULT_NAME}.zip" "$target_name"
}

main() {
  if [[ "$BUILD_SCOPE" = "all" && $# -ge 2 ]]; then
    echo "Custom output name is not supported for copy scope 'all'" >&2
    exit 1
  fi

  case "$BUILD_SCOPE" in
    ios)
      copy_ios "${2:-$IOS_DEFAULT_NAME}"
      ;;
    osx)
      copy_osx "${2:-$OSX_DEFAULT_NAME}"
      ;;
    all)
      copy_osx
      copy_ios
      ;;
    *)
      echo "Unsupported copy scope: $BUILD_SCOPE" >&2
      exit 1
      ;;
  esac
}

main "$@"
