#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$PACKAGE_DIR/../../.." && pwd)"
SOURCE_ZIP="$REPO_ROOT/prebuilt/osx/SweetLineCoreOSX.xcframework.zip"
VENDOR_DIR="$PACKAGE_DIR/Vendor/macOS"

if [ ! -f "$SOURCE_ZIP" ]; then
  echo "Missing native artifact: $SOURCE_ZIP" >&2
  echo "Run scripts/build-shared.sh --platform osx from the repository root first." >&2
  exit 1
fi

rm -rf "$VENDOR_DIR/SweetLineCoreOSX.xcframework"
mkdir -p "$VENDOR_DIR"
unzip -q -o "$SOURCE_ZIP" -d "$VENDOR_DIR"
