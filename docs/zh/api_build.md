# SweetLine 构建文档

本文档汇总多平台构建命令与 CMake 选项。

---

## 附录：平台构建

### C++ (CMake)

```bash
mkdir build && cd build
cmake .. -DSWEETLINE_BUILD_TESTS=ON
cmake --build .
```

### Android (Gradle)

```bash
cd platform/Android
./gradlew :sweetline:assembleRelease
```

### Java 22（FFM, Gradle）

```bash
cd platform/Java22
./gradlew :sweetline:build
./gradlew :demo:run
```

Java 22 模块说明：
- 构建脚本已配置 `--enable-preview`。
- 运行时需要 `--enable-native-access=ALL-UNNAMED`。
- 若 native 库无法自动发现，可设置 `-Dsweetline.lib.path=<native-lib-dir>`。
- 在 JAR 打包场景可使用 `NativeLibraryExtractor.extractToDefaultDir()` 自动解包并加载库。

### WebAssembly (Emscripten)

```bash
mkdir build-wasm && cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
```

### 编译选项

| CMake 选项 | 默认值 | 说明 |
|-----------|--------|------|
| `SWEETLINE_BUILD_TESTS` | 顶层构建默认 ON，作为子项目默认 OFF | 是否构建测试 |
| `SWEETLINE_BUILD_SHARED` | ON | 是否构建动态库目标 |
| `SWEETLINE_BUILD_STATIC` | ON | 是否构建静态库目标 |
| `SWEETLINE_BUILD_WASM_EMBIND` | ON | 是否构建 Emscripten embind 目标 |

说明：
- 当启用 `EMSCRIPTEN` 时，`SWEETLINE_BUILD_SHARED` 会跳过，因为该平台不使用普通动态库目标。
- Android/OHOS/Emscripten 平台会自动跳过单元测试目标。
