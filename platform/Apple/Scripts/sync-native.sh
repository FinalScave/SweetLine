#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPLE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$APPLE_DIR/../.." && pwd)"

sync_xcframework() {
  local platform="$1"
  local native_name="$2"
  local vendor_platform="$3"
  local source_zip="$REPO_ROOT/prebuilt/$platform/${native_name}.xcframework.zip"
  local vendor_dir="$APPLE_DIR/Vendor/$vendor_platform"
  local destination="$vendor_dir/${native_name}.xcframework"

  if [[ ! -f "$source_zip" ]]; then
    echo "Missing native artifact: $source_zip" >&2
    echo "Run scripts/build-shared.sh --platform $platform from the repository root first." >&2
    return 1
  fi

  rm -rf "$destination"
  mkdir -p "$vendor_dir"
  ditto -x -k "$source_zip" "$vendor_dir"

  if [[ ! -f "$destination/Info.plist" ]]; then
    echo "Failed to sync XCFramework: $destination" >&2
    return 1
  fi

  echo "Synced $destination"
}

case "${1:-all}" in
  ios)
    sync_xcframework ios SweetLineCoreIOS iOS
    ;;
  macos)
    sync_xcframework macos SweetLineCoreMacOS macOS
    ;;
  all)
    sync_xcframework ios SweetLineCoreIOS iOS
    sync_xcframework macos SweetLineCoreMacOS macOS
    ;;
  *)
    echo "Usage: $0 [ios|macos|all]" >&2
    exit 1
    ;;
esac
