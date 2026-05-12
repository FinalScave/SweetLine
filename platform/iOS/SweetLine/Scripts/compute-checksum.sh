#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
REPO_ROOT="$(cd "$PACKAGE_DIR/../../.." && pwd)"
SOURCE_ZIP="$REPO_ROOT/prebuilt/ios/SweetLineCoreIOS.xcframework.zip"

swift package compute-checksum "$SOURCE_ZIP"
