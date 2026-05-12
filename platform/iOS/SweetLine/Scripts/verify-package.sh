#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PACKAGE_DIR"
swift package describe
xcodebuild \
  -scheme SweetLine \
  -destination "generic/platform=iOS" \
  -configuration Debug \
  -derivedDataPath "$PACKAGE_DIR/.build/xcode-derived-data" \
  build
