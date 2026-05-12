# SweetLine API Reference

To reduce maintenance cost and improve readability, the API documentation is split by core/platform.

## Documentation Map

| Scope | Document | Description |
|------|------|------|
| Core Concepts + C++ | [Core API](api_core.md) | Workflow, core data structures, and C++ APIs |
| C / FFI | [C API](api_c.md) | Pure C interfaces and return buffer format |
| macOS (Swift) | [macOS Swift API](api_macos.md) | Swift Package API and native artifact guide |
| iOS (Swift) | [iOS Swift API](api_ios.md) | Swift Package API and native artifact guide |
| Android (Java/Kotlin) | [Android API](api_android.md) | JNI-based Java API |
| Flutter / Dart | [Flutter API](api_flutter.md) | Dart FFI wrapper API |
| Java 22 (FFM) | [Java 22 API](api_java22.md) | FFM wrapper API and native loading guide |
| .NET / WinForms (C#) | [.NET / WinForms API](api_dotnet.md) | P/Invoke wrapper API |
| WebAssembly (JS/TS) | [WebAssembly API](api_wasm.md) | Emscripten JS/TS API |
| HarmonyOS (ArkTS/NAPI) | [HarmonyOS API](api_ohos.md) | ArkTS bindings and usage entry |
| Build & CMake Options | [Build Guide](api_build.md) | Platform build commands and options |

## Recommended Reading Order

1. Read [Core API](api_core.md) first.
2. Then choose your platform document based on integration target.
3. Check [Build Guide](api_build.md) when preparing local/CI builds.
