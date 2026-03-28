# @qiplat/sweetline

SweetLine 语法高亮引擎的 HarmonyOS ArkTS 绑定。

## 简介

SweetLine 是一个跨平台、高性能、可扩展的语法高亮引擎，专为现代代码编辑器和代码展示场景设计。核心基于 Oniguruma 正则引擎和有限状态机模型，支持大文件实时语法高亮、增量更新，以及作用域/缩进引导线分析。

本包提供 HarmonyOS 平台的 NAPI 原生绑定，支持：

- 全量高亮分析
- 托管文档增量高亮
- 可见行范围高亮切片
- 单行分析与行状态传递
- 缩进引导线分析
- JSON 语法规则编译
- 样式名称注册与宏定义

## 安装

```bash
ohpm install @qiplat/sweetline@1.1.0
```

OpenHarmony ohpm 环境配置等更多内容，请参考 [如何安装 OpenHarmony ohpm 包](https://ohpm.openharmony.cn/#/cn/help/downloadandinstall)。

## 快速开始

### 全量高亮分析

```typescript
import { sweetline } from '@qiplat/sweetline';

const config = new sweetline.HighlightConfig();
const engine = new sweetline.HighlightEngine(config);

engine.compileSyntaxFromJson(syntaxJson);

const analyzer = engine.createAnalyzerByName('java');
if (analyzer) {
  const result = analyzer.analyzeText(sourceCode);
  for (const line of result.lines) {
    for (const span of line.spans) {
      // span.range
      // span.styleId
    }
  }
}
```

### 托管文档增量分析

```typescript
import { sweetline } from '@qiplat/sweetline';

const config = new sweetline.HighlightConfig();
const engine = new sweetline.HighlightEngine(config);
engine.compileSyntaxFromJson(syntaxJson);

const document = new sweetline.Document('file:///example.java', sourceCode);
const analyzer = engine.loadDocument(document);

if (analyzer) {
  const full = analyzer.analyze();

  const changeRange = new sweetline.TextRange(
    new sweetline.TextPosition(2, 4),
    new sweetline.TextPosition(2, 8)
  );

  const updated = analyzer.analyzeIncremental(changeRange, 'modified');
}
```

### 增量分析并仅返回可见区切片

```typescript
const visibleRange = new sweetline.LineRange(100, 80);

const slice = analyzer?.analyzeIncrementalInLineRange(
  new sweetline.TextRange(
    new sweetline.TextPosition(120, 0),
    new sweetline.TextPosition(120, 0)
  ),
  '// inserted\\n',
  visibleRange
);
```

### 基于缓存结果直接读取可见区切片

```typescript
// 先执行 analyze() 或 analyzeIncremental(...)
analyzer?.analyze();

const visibleSlice = analyzer?.getHighlightSlice(
  new sweetline.LineRange(0, 60)
);
```

`getHighlightSlice(...)` 不会重新执行高亮分析，只会从当前缓存结果中截取指定行范围，适合编辑器滚动、可视区刷新等场景。

### 单行分析

```typescript
const analyzer = engine.createAnalyzerByName('python');
if (analyzer) {
  const lineInfo = new sweetline.TextLineInfo(0, 0, 0);
  const result = analyzer.analyzeLine('def hello():', lineInfo);
  // result.highlight
  // result.endState
  // result.charCount
}
```

### 缩进引导线分析

```typescript
const analyzer = engine.createAnalyzerByName('java');
if (analyzer) {
  const guideResult = analyzer.analyzeIndentGuides(sourceCode);
  for (const guide of guideResult.guideLines) {
    // guide.column
    // guide.startLine
    // guide.endLine
  }
}
```

### 内联样式模式

```typescript
const config = new sweetline.HighlightConfig(false, true);
const engine = new sweetline.HighlightEngine(config);

// TokenSpan 将直接携带 inlineStyle
// span.inlineStyle.foreground
// span.inlineStyle.background
// span.inlineStyle.isBold
// span.inlineStyle.isItalic
```

## API 概览

### 核心类

| 类名 | 说明 |
|------|------|
| `HighlightEngine` | 高亮引擎，负责编译语法规则、创建分析器 |
| `HighlightConfig` | 高亮配置（索引显示、内联样式、Tab 宽度） |
| `SyntaxRule` | 已编译的语法规则 |
| `TextAnalyzer` | 纯文本分析器，适用于全量分析和单行分析 |
| `DocumentAnalyzer` | 文档分析器，支持托管文档增量更新和切片读取 |
| `Document` | 托管文档，支持增量编辑 |

### `TextAnalyzer`

| 方法 | 说明 |
|------|------|
| `analyzeText(text)` | 对整段文本做全量高亮分析 |
| `analyzeLine(text, lineInfo)` | 对单行文本做分析，并返回行结束状态 |
| `analyzeIndentGuides(text)` | 对文本执行缩进引导线分析 |

### `DocumentAnalyzer`

| 方法 | 说明 |
|------|------|
| `analyze()` | 对托管文档做全量高亮分析 |
| `analyzeIncremental(range, newText)` | 应用补丁并返回整篇文档高亮结果 |
| `analyzeIncrementalByIndex(startIndex, endIndex, newText)` | 按字符索引范围执行增量分析 |
| `analyzeIncrementalInLineRange(range, newText, visibleRange)` | 增量分析后，仅返回指定行范围切片 |
| `getHighlightSlice(visibleRange)` | 从当前缓存高亮结果中直接截取指定行范围 |
| `analyzeIndentGuides()` | 基于托管文档当前高亮结果执行缩进引导线分析 |
| `getDocument()` | 获取当前关联的托管文档 |

### 数据类

| 类名 | 说明 |
|------|------|
| `TextPosition` | 文本位置（行、列、索引） |
| `TextRange` | 文本范围（起止位置） |
| `TokenSpan` | 高亮标记片段（范围 + 样式） |
| `InlineStyle` | 内联样式（前景色、背景色、粗体、斜体等） |
| `LineHighlight` | 单行高亮结果 |
| `DocumentHighlight` | 全文档高亮结果 |
| `LineRange` | 行范围描述（起始行 + 行数） |
| `DocumentHighlightSlice` | 指定行范围的高亮切片 |
| `IndentGuideResult` | 缩进引导线分析结果 |
| `IndentGuideLine` | 单条缩进引导线 |
| `LineScopeState` | 行作用域状态 |
| `TextLineInfo` | 文本行元数据 |
| `LineAnalyzeResult` | 单行分析结果 |

## 语法规则

引擎通过 JSON 文件定义语法规则。开源仓库提供了 Java、Python、C/C++、Kotlin、Rust、Go、TypeScript、JavaScript、HTML 等 30 余种语言的语法规则文件，可从 [SweetLine 开源仓库](https://github.com/FinalScave/SweetLine) 的 `syntaxes/` 目录下载使用。

你也可以按照 JSON 语法规则格式自定义语言支持。

## 约束与限制

- 支持架构：`arm64-v8a`、`x86_64`
- 依赖 C++ 共享运行时（`c++_shared`）
- `getHighlightSlice(...)` 依赖已有缓存结果；请先调用 `analyze()` 或 `analyzeIncremental(...)`

## 开源协议

本项目基于 [LGPL-2.1](./LICENSE) 协议发布。
