#!/usr/bin/env bash
set -euo pipefail

APPLE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'EOF'
Usage: bash ./build.sh <command> [platform]

Commands:
  sync [ios|macos|all]  Sync XCFrameworks from repository prebuilts
  build                 Sync native artifacts and build the Apple package
  test                  Sync native artifacts and run Apple package tests
  verify                Describe, build, and test the Apple package
  all                   Verify the package and build the macOS demo
  demo-macos-build      Build the SwiftPM macOS demo
  demo-macos-run        Run the SwiftPM macOS demo
  clean                 Remove generated Apple and demo build outputs
EOF
}

sync_native() {
  "$APPLE_DIR/Scripts/sync-native.sh" "${1:-all}"
}

build_package() {
  sync_native all
  (cd "$APPLE_DIR" && swift build)
}

test_package() {
  sync_native all
  (cd "$APPLE_DIR" && swift test)
}

verify_package() {
  sync_native all
  (
    cd "$APPLE_DIR"
    swift package describe >/dev/null
    swift build
    swift test
  )
}

build_macos_demo() {
  sync_native macos
  (cd "$APPLE_DIR/Examples-macOS" && swift build)
}

run_macos_demo() {
  sync_native macos
  (cd "$APPLE_DIR/Examples-macOS" && swift run SweetLineMacDemo)
}

clean_outputs() {
  rm -rf \
    "$APPLE_DIR/.build" \
    "$APPLE_DIR/.swiftpm" \
    "$APPLE_DIR/Examples-macOS/.build" \
    "$APPLE_DIR/Examples-macOS/.swiftpm"
}

case "${1:-}" in
  sync)
    sync_native "${2:-all}"
    ;;
  build)
    build_package
    ;;
  test)
    test_package
    ;;
  verify)
    verify_package
    ;;
  all)
    verify_package
    (cd "$APPLE_DIR/Examples-macOS" && swift build)
    ;;
  demo-macos-build)
    build_macos_demo
    ;;
  demo-macos-run)
    run_macos_demo
    ;;
  clean)
    clean_outputs
    ;;
  help|-h|--help)
    usage
    ;;
  *)
    usage >&2
    exit 1
    ;;
esac
