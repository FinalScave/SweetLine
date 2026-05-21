# SweetLine Third-Party Dependencies

All vendored third-party dependencies live under `3dparty`.

Each dependency should provide:

- `METADATA` with the upstream version, source, license name, and retrieved files.
- `LICENSE` with the upstream license text or license selection note.
- `sweetline_3p.cmake` defining stable `SweetLine3p::*` targets.

Source dependencies should keep upstream files under `include` and `src`.

Binary dependencies should separate artifacts by platform, architecture, and build
configuration:

```text
lib/<platform>/<arch>/<config>/
bin/<platform>/<arch>/<config>/
```

When a platform has only one architecture in this project, the architecture
directory can be omitted:

```text
lib/<platform>/<config>/
bin/<platform>/<config>/
```

Examples:

```text
lib/android/arm64-v8a/release/
lib/emscripten/release/
lib/ios/arm64/release/
lib/ios/simulator-arm64/release/
lib/macos/arm64/release/
lib/linux/x86_64/release/
lib/ohos/x86_64/release/
lib/windows/x64/debug/
lib/windows/x64/release/
```

For iOS, keep device and simulator artifacts under the same `ios` platform
directory. Use `arm64` for device builds and `simulator-arm64` for Apple Silicon
simulator builds.

Oniguruma currently uses these prebuilt library paths:

```text
lib/android/arm64-v8a/release/libonig.a
lib/android/x86_64/release/libonig.a
lib/emscripten/release/libonig.a
lib/ios/arm64/release/libonig.a
lib/ios/simulator-arm64/release/libonig.a
lib/linux/aarch64/release/libonig.a
lib/linux/x86_64/release/libonig.a
lib/macos/arm64/release/libonig.a
lib/macos/x86_64/release/libonig.a
lib/ohos/arm64-v8a/release/libonig.a
lib/ohos/x86_64/release/libonig.a
lib/windows/x64/debug/onig.lib
lib/windows/x64/release/libonig.a
lib/windows/x64/release/onig.lib
```

Project code should depend on `SweetLine3p::*` targets instead of referencing
`3dparty` paths directly.
