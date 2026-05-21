# SweetLine Build Guide

This document summarizes platform build commands and options.

---

## Appendix: Platform Build

### C++ (CMake)

```bash
mkdir build && cd build
cmake .. -DSWEETLINE_BUILD_TESTS=ON
cmake --build .
```

### CMake Source Integration

SweetLine can be embedded as a source dependency from another CMake project:

```cmake
add_subdirectory(path/to/SweetLine)
target_link_libraries(my_target PRIVATE sweetline_static)
```

Use `sweetline` when you want the shared library target instead.

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
| `SWEETLINE_BUILD_TESTS` | ON for top-level builds, OFF as a subproject | Whether to build tests |
| `SWEETLINE_BUILD_SHARED` | ON | Whether to build shared library target |
| `SWEETLINE_BUILD_STATIC` | ON | Whether to build static library target |
| `SWEETLINE_BUILD_WASM_EMBIND` | ON | Whether to build the Emscripten embind target |

Notes:
- When `EMSCRIPTEN` is enabled, `SWEETLINE_BUILD_SHARED` is skipped because normal dynamic libraries are not used there.
- Unit tests are skipped automatically on Android/OHOS/Emscripten.
