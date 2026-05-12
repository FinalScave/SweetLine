# SweetLine API 文档

为降低维护成本并提升可读性，API 文档已按核心/平台拆分。

## 文档索引

| 范围 | 文档 | 说明 |
|------|------|------|
| 核心概念 + C++ | [核心 API](api_core.md) | 工作流程、核心数据结构与 C++ 接口 |
| C / FFI | [C API](api_c.md) | C 语言接口与返回缓冲区格式 |
| macOS (Swift) | [macOS Swift API](api_macos.md) | Swift Package API 与 Native 产物说明 |
| Android (Java/Kotlin) | [Android API](api_android.md) | JNI 绑定的 Java API |
| Flutter / Dart | [Flutter API](api_flutter.md) | Dart FFI 封装 API |
| Java 22 (FFM) | [Java 22 API](api_java22.md) | FFM 封装 API 与 Native 加载说明 |
| .NET / WinForms (C#) | [.NET / WinForms API](api_dotnet.md) | P/Invoke 封装 API |
| WebAssembly (JS/TS) | [WebAssembly API](api_wasm.md) | Emscripten 导出的 JS/TS API |
| HarmonyOS (ArkTS/NAPI) | [HarmonyOS API](api_ohos.md) | ArkTS 绑定与入口说明 |
| 构建与 CMake 选项 | [构建文档](api_build.md) | 多平台构建命令与参数 |

## 推荐阅读顺序

1. 先阅读 [核心 API](api_core.md)。
2. 再按集成平台选择对应文档。
3. 准备本地/CI 构建时查看 [构建文档](api_build.md)。
