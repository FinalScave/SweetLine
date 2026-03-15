# SweetLine 构建文档

本文档汇总多平台构建命令与 CMake 选项。

---

## 附录：平台构建

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

### WebAssembly (Emscripten)

```bash
mkdir build-wasm && cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
```

### 编译选项

| CMake 选项 | 默认值 | 说明 |
|-----------|--------|------|
| `BUILD_TESTING` | ON | 是否构建测试 |
| `BUILD_SHARED_LIB` | ON | 是否构建动态库目标 |
| `BUILD_STATIC_LIB` | ON | 是否构建静态库目标 |

说明：
- 当启用 `EMSCRIPTEN` 时，`BUILD_SHARED_LIB` 会被强制设置为 `OFF`。
- Android/OHOS/Emscripten 平台会自动跳过单元测试目标。
