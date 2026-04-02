# SweetLine Apple SDK

This directory contains the Apple-side workspace for SweetLine.

## Architecture

- `SweetLineCoreIOS` / `SweetLineCoreOSX` — platform-specific xcframework binary targets built from the native SweetLine library
- `SweetLineBridge` — thin C bridge module that re-exports `c_sweetline.h`
- `SweetLineCoreWrapper` — SDK-side native link anchor for the static xcframework
- `SweetLineCoreInternal` — Swift SDK core wrappers, native handle management, and result decoders
- `SweetLineMacOS` — macOS-facing library target
- `Examples-MacOS` — separate demo package that validates the SDK and includes its own demo-side native wrapper for final executable linkage

## Current status

This workspace now builds as a Swift-direct-C Apple SDK backed by split `SweetLineCoreIOS.xcframework` and `SweetLineCoreOSX.xcframework` artifacts.

## Intended local workflow

- Rebuild prebuilt Apple xcframework archives via `make build-prebuilt` (optional)
- Install Apple xcframeworks into `platform/Apple/binaries` via `scripts/copy-native-xcframework.sh` or `make copy-native`
- Validate manifest via `swift package dump-package --package-path platform/Apple`
- Build SDK via `swift build --package-path platform/Apple`
- Build demo via `cd Examples-MacOS && swift build`
