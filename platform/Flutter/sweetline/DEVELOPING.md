# Developing sweetline

This document is for maintainers of the Flutter package inside the SweetLine
monorepo.

## Sync native binaries

The package build hook reads native libraries from the package-local `native/`
directory. Refresh it from the repository root `prebuilt/` directory before
local verification or publishing:

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter/sweetline
dart tool/sync_native_binaries.dart
```

Use `dart tool/...`, not `dart run tool/...`. Running through `dart run`
triggers build hooks before `native/` has been populated.

## Native layout

The sync script mirrors selected `prebuilt/` directories into
`platform/Flutter/sweetline/native/`.

Current build hook mappings:

- `windows/x64/sweetline.dll`
- `linux/aarch64/libsweetline.so`
- `linux/x86_64/libsweetline.so`
- `android/arm64-v8a/libsweetline.so`
- `android/x86_64/libsweetline.so`
- `macos/arm64/libsweetline.dylib`
- `macos/x86_64/libsweetline.dylib`
- `ios/arm64/libsweetline.dylib`
- `ios/simulator-arm64/libsweetline.dylib`

## Local validation

```bash
cd D:/Projects/CrossPlatform/SweetLine/platform/Flutter/sweetline
dart tool/sync_native_binaries.dart
dart analyze
dart pub publish --dry-run
```

## Publishing notes

- `README.md` is the `pub.dev` landing page and should stay user-facing.
- Maintenance flow such as syncing native binaries belongs in this document or
  repository docs, not in the package README.
