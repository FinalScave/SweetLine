# SweetLine API Reference

To reduce maintenance cost and improve readability, the API documentation is split by core/platform.

## Documentation Map

| Scope | Document | Description |
|------|------|------|
| Core Concepts + C++ | [Core API](api_core.md) | Workflow, core data structures, and C++ APIs |
| C / FFI | [C API](api_c.md) | Pure C interfaces and return buffer format |
| macOS (Swift) | [SweetLine-macOS](https://github.com/Xiue233/SweetLine-macOS) | Swift Package API and native artifact guide |
| iOS (Swift) | [SweetLine-iOS](https://github.com/Xiue233/SweetLine-iOS) | Swift Package API and native artifact guide |
| Android (Java/Kotlin) | [Android README](../../platform/Android/sweetline/README.md) | JNI-based Java API |
| Kotlin Multiplatform | [KMP README](../../platform/KMP/sweetline/README.md) | Kotlin API for Android, iOS, and JVM desktop |
| Flutter / Dart | [Flutter README](../../platform/Flutter/sweetline/README.md) | Dart FFI wrapper API |
| Java 22 (FFM) | [Java 22 README](../../platform/Java22/sweetline/README.md) | FFM wrapper API and native loading guide |
| .NET (C#) | [.NET README](../../platform/CSharp/SweetLine/README.md) | P/Invoke wrapper API |
| WebAssembly (JS/TS) | [Emscripten README](../../platform/Emscripten/README.md) | Emscripten JS/TS API |
| HarmonyOS (ArkTS/NAPI) | [HarmonyOS README](../../platform/OHOS/sweetline/README.md) | ArkTS bindings and usage entry |
| Build & CMake Options | [Build Guide](api_build.md) | Platform build commands and options |

## Recommended Reading Order

1. Read [Core API](api_core.md) first.
2. Then choose your platform document based on integration target.
3. Check [Build Guide](api_build.md) when preparing local/CI builds.
