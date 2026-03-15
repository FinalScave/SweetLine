# SweetLine HarmonyOS API 文档

SweetLine 通过 NAPI 提供 HarmonyOS ArkTS 绑定。

## 包信息

- 包名：`@qiplat/sweetline`
- 入口：`platform/OHOS/sweetline/src/main/ets/Index.ets`
- 类型定义：`platform/OHOS/sweetline/src/main/ets/Index.d.ts`

## 核心类型

- `sweetline.HighlightEngine`
- `sweetline.HighlightConfig`
- `sweetline.Document`
- `sweetline.TextAnalyzer`
- `sweetline.DocumentAnalyzer`
- `sweetline.SyntaxRule`

## 关键 API（ArkTS）

```ts
class HighlightConfig {
  showIndex: boolean;
  inlineStyle: boolean;
  tabSize: number;
}

class TextAnalyzer {
  analyzeText(text: string): DocumentHighlight;
  analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;
  analyzeIndentGuides(text: string): IndentGuideResult;
}

class DocumentAnalyzer {
  analyze(): DocumentHighlight;
  analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;
  analyzeIncrementalByIndex(startIndex: number, endIndex: number, newText: string): DocumentHighlight;
  analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;
  analyzeIndentGuides(): IndentGuideResult;
}
```

## ArkTS 使用示例

```ts
import { sweetline } from "@qiplat/sweetline";

const engine = new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
engine.compileSyntaxFromJson(syntaxJson);

const analyzer = engine.createAnalyzerByName("java");
if (analyzer) {
  const highlight = analyzer.analyzeText(sourceCode);
}
```

## 说明

- API 命名风格整体与 WebAssembly/TypeScript 侧一致。
- 编辑器场景建议优先使用 `DocumentAnalyzer` 做增量分析。
- 缩进划线可使用 `TextAnalyzer.analyzeIndentGuides` / `DocumentAnalyzer.analyzeIndentGuides`。
- 构建命令请参见 [构建文档](api_build.md)。

