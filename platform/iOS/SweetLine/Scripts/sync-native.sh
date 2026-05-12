#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$PACKAGE_DIR/../../.." && pwd)"
SOURCE_ZIP="$REPO_ROOT/prebuilt/ios/SweetLineCoreIOS.xcframework.zip"
VENDOR_DIR="$PACKAGE_DIR/Vendor/iOS"

if [ ! -f "$SOURCE_ZIP" ]; then
  echo "Missing native artifact: $SOURCE_ZIP" >&2
  echo "Run scripts/build-shared.sh --platform ios from the repository root first." >&2
  exit 1
fi

rm -rf "$VENDOR_DIR/SweetLineCoreIOS.xcframework"
mkdir -p "$VENDOR_DIR"
unzip -q -o "$SOURCE_ZIP" -d "$VENDOR_DIR"
