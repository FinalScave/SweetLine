# SweetLine v{{VERSION}}

Cross-platform native SDK for SweetLine.

## Assets

- Native SDK: `{{NATIVE_ASSET_NAME}}`

## Included Platforms

- Android: `arm64-v8a`, `x86_64`
- iOS: `arm64/libsweetline.dylib`, `simulator-arm64/libsweetline.dylib`, `SweetLineCoreIOS.xcframework.zip`
- macOS: `arm64/libsweetline.dylib`, `x86_64/libsweetline.dylib`, `SweetLineCoreMacOS.xcframework.zip`
- Linux: `x86_64`, `aarch64`
- Windows: `x64`
- OHOS: `arm64-v8a`, `x86_64`
- WebAssembly: `sweetline.js`, `sweetline.wasm`, `sweetline.d.ts`

## Package Layout

- `include/sweetline/`: C/C++ headers
- `prebuilt/`: native binaries grouped by platform
- `README.txt`: package metadata and included platforms
- `SHA256SUMS.txt`: checksums for all packaged files

## Notes

- Commit: `{{COMMIT}}`
- The SDK is built from the repository `prebuilt/` and `include/` directories.
