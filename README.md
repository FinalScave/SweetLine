简体中文 | [English](./README_en.md)

# SweetLine 语法高亮引擎

## 概述

SweetLine 是一款跨平台、高性能、可扩展的语法高亮引擎，专为现代代码编辑器和代码展示场景设计。基于 Oniguruma 正则引擎和有限状态机模型，能够实时处理大型代码文件并提供精准的语法高亮。

## 效果展示

<p align="center">
  <img src="docs/snapshot/java.png" width="45%" alt="Java" />
  <img src="docs/snapshot/cpp.png" width="45%" alt="C++" />
</p>
<p align="center">
  <img src="docs/snapshot/shell.png" width="45%" alt="Shell" />
  <img src="docs/snapshot/toml.png" width="45%" alt="TOML" />
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
- 支持变量替换和 pattern 复用，减少规则冗余
- 内置 50+ 语言语法规则（Java、C/C++、Python、Kotlin、Rust、Go、TypeScript 等）

### 跨平台
- 核心引擎使用 C++17 编写
- 提供 C API 封装，方便 FFI 集成
- 原生支持 Android（JNI）、WebAssembly（Emscripten）、HarmonyOS（NAPI）
- 支持 Windows、Linux、macOS 等桌面平台

## 架构总览

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
├──────────┬──────────┬──────────┬────────────┬───────────┤
│ Android  │   WASM   │   OHOS   │   C API    │   C++     │
│  (JNI)   │(Emscript)│  (NAPI)  │  (FFI)     │  (Native) │
├──────────┴──────────┴──────────┴────────────┴───────────┤
│                 SweetLine Core (C++17)                   │
│  ┌─────────────┐ ┌──────────────┐ ┌──────────────────┐  │
│  │HighlightEng │ │ TextAnalyzer │ │DocumentAnalyzer  │  │
│  │    ine      │ │(全量分析)    │ │ (增量分析)       │  │
│  └──────┬──────┘ └──────┬───────┘ └────────┬─────────┘  │
│         │               │                  │             │
│  ┌──────▼───────────────▼──────────────────▼─────────┐  │
│  │           State Machine + Regex Engine             │  │
│  │              (Oniguruma + FSM)                     │  │
│  └───────────────────────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────┐  │
│  │           SyntaxRule (JSON Compiled)               │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
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
```

### Android 使用

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:0.0.4'
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
  "states": {
    "default": [
      {
        "pattern": "\\b(if|else|while|return)\\b",
        "styles": [1, "keyword"]
      },
      {
        "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"",
        "style": "string"
      },
      {
        "pattern": "//[^\\n]*",
        "style": "comment"
      }
    ]
  }
}
```

完整语法规则配置说明请参见 [语法规则配置文档](docs/syntax_rule.md)。

## 文档

| 文档 | 说明 |
|------|------|
| [语法规则配置文档](docs/syntax_rule.md) | 详细介绍如何编写 JSON 语法规则文件 |
| [API 文档](docs/api.md) | C++、C、Android、WebAssembly 各平台 API 使用说明 |
| [项目协作说明](docs/join.md) | 参与项目共建指南 |

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

欢迎一起共建 SweetLine 高亮引擎！如果您有参与项目的想法，可直接拉分支修改并提交合并请求，详见 [项目协作说明](docs/join.md)。
