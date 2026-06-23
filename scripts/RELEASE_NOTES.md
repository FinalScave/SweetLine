# SweetLine v{{VERSION}}

Cross-platform release assets for SweetLine.

## Assets

- Prebuilt binaries: `{{PREBUILT_ASSET_NAME}}`
- C/C++ headers: `{{HEADERS_ASSET_NAME}}`

## Included Platforms In The Prebuilt Package

- Android: `arm64-v8a`, `x86_64`
- iOS: platform XCFramework `SweetLineCoreIOS.xcframework.zip`; per-arch frameworks under `arm64/SweetLineCore.framework.zip` and `simulator-arm64/SweetLineCore.framework.zip`
- macOS: platform XCFramework `SweetLineCoreOSX.xcframework.zip`; per-arch frameworks under `arm64/SweetLineCore.framework.zip` and `x86_64/SweetLineCore.framework.zip`
- Linux: `x86_64`, `aarch64`
- Windows: `x64`
- OHOS: `arm64-v8a`, `x86_64`
- WebAssembly: `sweetline.js`, `sweetline.wasm`, `sweetline.d.ts`

## Package Layout

- `{{PREBUILT_ASSET_NAME}}`
  - archive root contains platform directories directly
  - includes `README.txt` and `SHA256SUMS.txt` by default
- `{{HEADERS_ASSET_NAME}}`
  - archive root contains `include/sweetline/...`
  - includes `SHA256SUMS.txt` by default

## Notes

- Commit: `{{COMMIT}}`
- The prebuilt package is built from the repository `prebuilt/` artifacts.
- The headers package is built from `include/` and uses the install-style layout `include/sweetline/`.
