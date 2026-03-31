# SweetLine Apple SDK

This directory contains the Apple-side workspace for SweetLine.

## Architecture

- `SweetLineNative` — xcframework binary target built from the native SweetLine library
- `SweetLineBridge` — thin C bridge module that re-exports `c_sweetline.h`
- `SweetLineNativeWrapper` — SDK-side native link anchor for the static xcframework
- `SweetLineCoreInternal` — Swift SDK core wrappers, native handle management, and result decoders
- `SweetLineMacOS` — macOS-facing library target
- `Examples-MacOS` — separate demo package that validates the SDK and includes its own demo-side native wrapper for final executable linkage

## Current status

This workspace now builds as a Swift-direct-C Apple SDK backed by `SweetLineNative.xcframework`.

## Intended local workflow

- Build native xcframework via `scripts/build_native_xcframework.sh`
- Validate manifest via `swift package dump-package --package-path platform/Apple`
- Build SDK via `swift build --package-path platform/Apple`
- Build demo via `cd Examples-MacOS && swift build`
