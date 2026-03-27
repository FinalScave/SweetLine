# @qiplat/sweetline

SweetLine 语法高亮引擎的 HarmonyOS ArkTS 绑定。

## 简介

SweetLine 是一个跨平台、高性能、可扩展的语法高亮引擎，专为现代代码编辑器和代码展示场景设计。基于 Oniguruma 正则引擎和有限状态机模型，支持大文件实时语法高亮分析。

本包提供 HarmonyOS 平台的 NAPI 原生绑定，支持：

- 全量高亮分析与增量更新
- 单行分析（可自定义增量策略）
- 缩进引导线分析
- 支持通过 JSON 定义语法规则，也可自定义扩展

## 使用示例

### 基本用法：全量高亮分析

```typescript
import { sweetline } from '@qiplat/sweetline';

// 1. 创建高亮引擎
const config = new sweetline.HighlightConfig();
const engine = new sweetline.HighlightEngine(config);

// 2. 编译语法规则（从 JSON 字符串）
const rule = engine.compileSyntaxFromJson(syntaxJson);

// 3. 创建分析器并分析文本
const analyzer = engine.createAnalyzerByName("java");
if (analyzer) {
  const result = analyzer.analyzeText(sourceCode);

  // 4. 遍历高亮结果
  for (const line of result.lines) {
    for (const span of line.spans) {
      // span.range  - 文本范围（行/列/索引）
      // span.styleId - 样式 ID
    }
  }
}
```

### 增量高亮分析

```typescript
import { sweetline } from '@qiplat/sweetline';

const config = new sweetline.HighlightConfig();
const engine = new sweetline.HighlightEngine(config);
engine.compileSyntaxFromJson(syntaxJson);

// 创建托管文档
const document = new sweetline.Document("file:///example.java", sourceCode);

// 加载文档并获取文档分析器
const docAnalyzer = engine.loadDocument(document);
if (docAnalyzer) {
  // 全量分析
  const highlight = docAnalyzer.analyze();

  // 增量更新：当文档编辑时，仅重新分析变更部分
  const changeRange = new sweetline.TextRange(
    new sweetline.TextPosition(2, 4),
    new sweetline.TextPosition(2, 8)
  );
  const newHighlight = docAnalyzer.analyzeIncremental(changeRange, "modified");
}
```

### 单行分析

```typescript
const analyzer = engine.createAnalyzerByName("python");
if (analyzer) {
  const lineInfo = new sweetline.TextLineInfo(0, 0, 0);
  const result = analyzer.analyzeLine("def hello():", lineInfo);
  // result.highlight - 当前行高亮
  // result.endState  - 行结束状态，传入下一行的 startState
}
```

### 缩进引导线分析

```typescript
// 使用 TextAnalyzer
const analyzer = engine.createAnalyzerByName("java");
if (analyzer) {
  const guideResult = analyzer.analyzeIndentGuides(sourceCode);
  for (const guide of guideResult.guideLines) {
    // guide.column    - 引导线列位置
    // guide.startLine - 起始行
    // guide.endLine   - 结束行
  }
}
```

### 内联样式模式

```typescript
// 启用内联样式：高亮结果直接包含颜色信息，无需样式 ID 映射
const config = new sweetline.HighlightConfig(false, true);
const engine = new sweetline.HighlightEngine(config);

// 分析结果中 TokenSpan 将包含 inlineStyle
// span.inlineStyle.foreground - 前景色
// span.inlineStyle.background - 背景色
// span.inlineStyle.isBold     - 是否粗体
// span.inlineStyle.isItalic   - 是否斜体
```

## API 说明

### 核心类

| 类名 | 说明 |
|------|------|
| `HighlightEngine` | 高亮引擎，负责编译语法规则、创建分析器 |
| `HighlightConfig` | 高亮配置（索引显示、内联样式、Tab 宽度） |
| `SyntaxRule` | 已编译的语法规则 |
| `TextAnalyzer` | 纯文本分析器，适用于全量分析和单行分析 |
| `DocumentAnalyzer` | 文档分析器，支持增量更新 |
| `Document` | 托管文档，支持增量编辑 |

### 数据类

| 类名 | 说明 |
|------|------|
| `TextPosition` | 文本位置（行、列、索引） |
| `TextRange` | 文本范围（起止位置） |
| `TokenSpan` | 高亮标记片段（范围 + 样式） |
| `InlineStyle` | 内联样式（前景色、背景色、粗体、斜体等） |
| `LineHighlight` | 单行高亮结果 |
| `DocumentHighlight` | 全文档高亮结果 |
| `DocumentHighlightSlice` | 指定行范围的高亮切片 |
| `LineRange` | 行范围描述 |
| `IndentGuideResult` | 缩进引导线分析结果 |
| `IndentGuideLine` | 单条缩进引导线 |
| `LineScopeState` | 行作用域状态 |
| `TextLineInfo` | 文本行元数据 |
| `LineAnalyzeResult` | 单行分析结果 |

## 语法规则

引擎通过 JSON 文件定义语法规则。开源仓库提供了 Java、Python、C/C++、Kotlin、Rust、Go、TypeScript、JavaScript、HTML 等 30 余种语言的语法规则文件，可从 [SweetLine 开源仓库](https://github.com/FinalScave/SweetLine) 的 `syntaxes/` 目录下载使用。

你也可以按照 JSON 语法规则格式自定义语言支持。

## 约束与限制

- 支持架构：arm64-v8a, x86_64
- 依赖 C++ 共享运行时（c++_shared）

## 开源协议

本项目基于 [LGPL-2.1](./LICENSE) 协议发布。
