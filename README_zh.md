简体中文 | [English](./README.md)

# SweetLine 语法高亮引擎

## 概述

SweetLine 是一款跨平台、高性能、可扩展的语法高亮引擎，专为现代代码编辑器和代码展示场景设计。基于 Oniguruma 正则引擎和有限状态机模型，能够实时处理大型代码文件并提供精准的语法高亮。

## 效果展示

<p align="center">
  <img src="docs/snapshot/swing-java.png" width="45%" alt="Swing 示例（Java）" />
  <img src="docs/snapshot/winforms-c.png" width="45%" alt="WinForms 示例（C#）" />
</p>
<p align="center">
  <img src="docs/snapshot/web-kt.png" width="90%" alt="Html Web Demo" />
</p>
<p align="center">
  <img src="docs/snapshot/android-cpp.png" width="45%" alt="Android 示例（C++）" />
  <img src="docs/snapshot/android-toml.png" width="45%" alt="Android 示例（TOML）" />
</p>

## 核心特性

### 高性能
- 基于 [Oniguruma](https://github.com/kkos/oniguruma) 正则引擎，提供快速的 pattern 匹配
- 增量更新算法，仅重新分析变更部分，适用于编辑器实时高亮场景
- 多行状态保持，避免全文档重新分析

### 高精度
- 基于有限状态机（FSM）模型，支持复杂的语法规则嵌套
- 多捕获组样式映射，精准控制高亮粒度
- 子状态（subStates）机制，处理嵌套语法结构（如泛型、模板参数）
- 支持零宽匹配（zero-width match），处理上下文相关的状态切换

### 高度可扩展
- 使用 JSON 配置语法规则，无需编写代码即可新增语言支持
- 支持变量替换、`fragments`（`include` / `includes`）与 pattern 复用，减少规则冗余
- 内置 33 种语言语法规则（Java、C/C++、Python、Kotlin、Rust、Go、TypeScript 等）

### 跨平台
- 核心引擎使用 C++17 编写
- 提供 C API 封装，方便 FFI 集成
- 原生支持 Android（JNI）、Java 22（FFM）、WebAssembly（Emscripten）、HarmonyOS（NAPI）、.NET/WinForms（P/Invoke）
- 支持 Windows、Linux、macOS 等桌面平台

## 架构总览

```text
┌────────────────────────────────────────────────────────────────────────────────────┐
│                              SweetLine Architecture                               │
├────────────────────────────────────────────────────────────────────────────────────┤
│ Application / Platform Bindings                                                   │
│  Android(JNI) | Java22(FFM) | .NET/C#(P/Invoke) | WASM(Emscripten)               │
│  HarmonyOS(NAPI) | C API(FFI) | C++ Native API                                    │
├────────────────────────────────────────────────────────────────────────────────────┤
│                         SweetLine C++ Core (C++17)                                │
│                                                                                    │
│  ┌──────────────────────┐        ┌──────────────────────────────┐                 │
│  │    HighlightEngine   │ -----> │  SyntaxRule Compiler (JSON)  │                 │
│  └──────────┬───────────┘        └──────────────────────────────┘                 │
│             │                                                                      │
│    ┌────────▼─────────┐       ┌──────────────────────┐                            │
│    │   TextAnalyzer   │       │   DocumentAnalyzer   │                            │
│    │   (Full Scan)    │       │   (Incremental)      │                            │
│    └────────┬─────────┘       └──────────┬───────────┘                            │
│             │                            │                                         │
│             │                   ┌────────▼─────────┐                               │
│             │                   │  Document Model  │                               │
│             │                   │ (Managed Text)   │                               │
│             │                   └──────────────────┘                               │
│             │                                                                      │
│             └──────────────┬───────────────────────┘                               │
│                            ▼                                                       │
│          ┌────────────────────────────────────────────────────────────┐            │
│          │ Regex + FSM Runtime (Oniguruma + State Machine)           │            │
│          └──────────────────────────┬─────────────────────────────────┘            │
│                                     ▼                                              │
│              Highlight Result + Scope/Indent Guide Analysis                        │
└────────────────────────────────────────────────────────────────────────────────────┘
```

## 快速开始

### C++ 使用

```cpp
#include "highlight.h"
using namespace sweetline;

// 1. 创建高亮引擎
auto engine = std::make_shared<HighlightEngine>();

// 2. 编译语法规则
auto rule = engine->compileSyntaxFromFile("syntaxes/java.json");

// 3. 创建文档对象
auto document = std::make_shared<Document>("file:///example.java", R"(
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
)");

// 4. 加载文档并分析
auto analyzer = engine->loadDocument(document);
auto highlight = analyzer->analyze();

// 5. 遍历高亮结果
for (size_t i = 0; i < highlight->lines.size(); i++) {
    auto& line = highlight->lines[i];
    for (auto& span : line.spans) {
        // span.range  - 文本范围 (行/列/索引)
        // span.style_id - 样式ID (keyword=1, string=2, ...)
    }
}
```

### 增量更新

```cpp
// 文档发生编辑时，只重新分析变更部分
TextRange change_range { {2, 4}, {2, 8} };
std::string new_text = "modified";
auto new_highlight = analyzer->analyzeIncremental(change_range, new_text);

// 仅从最新缓存的高亮结果中读取当前可见行
LineRange visible_range {100, 60};
auto visible_slice = analyzer->getHighlightSlice(visible_range);

// 也可以把补丁应用和可见区切片合并成一次调用
auto updated_slice = analyzer->analyzeIncrementalInLineRange(change_range, new_text, visible_range);
```

当渲染层只需要当前可见窗口时，优先在 `analyze()` 或 `analyzeIncremental()` 之后调用 `getHighlightSlice()`。

### Java 22（FFM）使用

```java
import com.qiplat.sweetline.*;

try (HighlightEngine engine = new HighlightEngine(new HighlightConfig(true, false))) {
    engine.compileSyntaxFromFile("syntaxes/java.json");

    try (TextAnalyzer analyzer = engine.createAnalyzerByName("java")) {
        DocumentHighlight result = analyzer.analyzeText(sourceCode);
    }
}
```

Java 22 FFM 封装位于 `platform/Java22`。
运行时需要开启 native access（例如 `--enable-native-access=ALL-UNNAMED`），并确保 SweetLine 原生库路径可被加载。

### Android 使用

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:1.2.0'
```

```java
// 创建引擎
HighlightEngine engine = new HighlightEngine(new HighlightConfig());

// 编译语法规则
engine.compileSyntaxFromJson(jsonString);

// 全量分析
TextAnalyzer analyzer = engine.createAnalyzerByName("java");
DocumentHighlight result = analyzer.analyzeText(sourceCode);

// 遍历结果
for (LineHighlight line : result.lines) {
    for (TokenSpan span : line.spans) {
        // span.range, span.styleId
    }
}
```

### WebAssembly 使用

```javascript
import { sweetline } from './libsweetline.js';

// 创建引擎
const config = new sweetline.HighlightConfig();
const engine = new sweetline.HighlightEngine(config);

// 编译语法规则
engine.compileSyntaxFromJson(jsonString);

// 分析文本
const analyzer = engine.createAnalyzerByName("javascript");
const highlight = analyzer.analyzeText(sourceCode);

// 遍历结果
for (let i = 0; i < highlight.lines.size(); i++) {
    const line = highlight.lines.get(i);
    for (let j = 0; j < line.spans.size(); j++) {
        const span = line.spans.get(j);
        // span.range, span.styleId
    }
}
```

### 自定义语法规则

SweetLine 使用 JSON 定义语法规则，以下是一个简单示例：

```json
{
  "name": "myLanguage",
  "fileExtensions": [".mylang"],
  "variables": {
    "identifier": "[a-zA-Z_]\\w*"
  },
  "fragments": {
    "commonLiterals": [
      { "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"", "style": "string" },
      { "pattern": "//[^\\n]*", "style": "comment" }
    ]
  },
  "states": {
    "default": [
      {
        "pattern": "\\b(if|else|while|return)\\b",
        "styles": [1, "keyword"]
      },
      { "include": "commonLiterals" }
    ]
  }
}
```

完整语法规则配置说明请参见 [语法规则配置文档](docs/zh/syntax_rule.md)。

如果您希望更快、更规范地新增或完善语法规则，也可以直接使用仓库内的 [`skills/`](skills)。推荐组合是：
- [`syntax-highlighting-authoring`](skills/syntax-highlighting-authoring/SKILL.md)：通用的语法规则、路由、example 和验证编写流程
- [`sweetline-syntax-profile`](skills/sweetline-syntax-profile/SKILL.md)：SweetLine 仓库专用约束，包括目录结构、style names、测试入口、demo 注册以及 `120~150` 行 example 建议

## 文档

| 文档 | 说明 |
|------|------|
| [语法规则配置文档](docs/zh/syntax_rule.md) | 详细介绍如何编写 JSON 语法规则文件 |
| [API 文档（索引）](docs/zh/api.md) | API 总入口与阅读顺序 |
| [核心 API](docs/zh/api_core.md) | 核心概念与 C++ API |
| [C API](docs/zh/api_c.md) | 面向 FFI 的 C 接口 |
| [Android API](docs/zh/api_android.md) | Android 平台 Java/Kotlin API |
| [Java 22 API](docs/zh/api_java22.md) | Java 22 FFM API |
| [.NET / WinForms API](docs/zh/api_dotnet.md) | C# API（P/Invoke 封装） |
| [WebAssembly API](docs/zh/api_wasm.md) | JavaScript/TypeScript API |
| [HarmonyOS API](docs/zh/api_ohos.md) | ArkTS/NAPI API 使用说明 |
| [构建文档](docs/zh/api_build.md) | 多平台构建命令与参数 |
| [项目协作说明](docs/zh/join.md) | 参与项目共建指南，包含使用仓库 skills 快速编写语法规则的说明 |

## 内置语言支持

| 语言 | 文件 | 语言 | 文件 |
|------|------|------|------|
| Java | `java.json` | Python | `python.json` |
| C | `c.json` | C++ | `c++.json` |
| C# | `csharp.json` | Kotlin | `kotlin.json` |
| Swift | `swift.json` | Rust | `rust.json` |
| Go | `go.json` | Dart | `dart.json` |
| TypeScript | `typescript.json` | JavaScript | `javascript.json` |
| HTML | `html.json` | XML | `xml.json` |
| SQL | `sql.json` | Shell | `shell.json` |
| Lua | `lua.json` | Groovy | `groovy.json` |
| YAML | `yaml.json` | TOML | `toml.json` |
| Markdown | `markdown.json` | JSON | `json-sweetline.json` |

## 性能建议

- **预编译语法规则**：在应用启动时一次性编译所有需要的语法规则，编译后的规则可重复使用
- **优先使用增量更新**：对于编辑器场景，使用 `DocumentAnalyzer` 的增量分析而非全量分析
- **优化正则表达式**：避免过于复杂的回溯密集型 pattern，善用 `variables` 复用常见 pattern
- **合理设计状态机**：控制状态数量，确保每个状态都有明确的退出路径

## 共建

欢迎一起共建 SweetLine 高亮引擎！如果您有参与项目的想法，可直接拉分支修改并提交合并请求。对于语法规则相关工作，建议结合仓库中的 [`skills/`](skills) 与 [项目协作说明](docs/zh/join.md) 一起使用。
