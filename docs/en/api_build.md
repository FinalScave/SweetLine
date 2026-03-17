# SweetLine Build Guide

This document summarizes platform build commands and options.

---

## Appendix: Platform Build

### C++ (CMake)

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
```

### Android (Gradle)

```bash
cd platform/Android
./gradlew :sweetline:assembleRelease
```

### Java 22 (FFM, Gradle)

```bash
cd platform/Java22
./gradlew :sweetline:build
./gradlew :demo:run
```

Java 22 module notes:
- Build scripts are configured with `--enable-preview`.
- Runtime needs `--enable-native-access=ALL-UNNAMED`.
- If native library is not auto-discovered, set `-Dsweetline.lib.path=<native-lib-dir>`.
- For JAR packaging scenarios, you can use `NativeLibraryExtractor.extractToDefaultDir()`.

### WebAssembly (Emscripten)

```bash
mkdir build-wasm && cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
```

### Build Options

| CMake Option | Default | Description |
|-------------|---------|-------------|
| `BUILD_TESTING` | ON | Whether to build tests |
| `BUILD_SHARED_LIB` | ON | Whether to build shared library target |
| `BUILD_STATIC_LIB` | ON | Whether to build static library target |

Notes:
- When `EMSCRIPTEN` is enabled, `BUILD_SHARED_LIB` is forced to `OFF`.
- Unit tests are skipped automatically on Android/OHOS/Emscripten.
