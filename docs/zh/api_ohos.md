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

class HighlightEngine {
  compileSyntaxFromJson(json: string): SyntaxRule;
  compileSyntaxFromFile(path: string): SyntaxRule;
  createAnalyzerBySyntaxName(syntaxName: string): TextAnalyzer | null;
  createAnalyzerByFileName(fileName: string): TextAnalyzer | null;
  loadDocument(document: Document): DocumentAnalyzer | null;
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
  analyzeLineRange(visibleRange: LineRange): DocumentHighlightSlice;
  analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;
  getHighlightSlice(visibleRange: LineRange): DocumentHighlightSlice;
  analyzeIndentGuides(): IndentGuideResult;
}
```

## ArkTS 使用示例

```ts
import { sweetline } from "@qiplat/sweetline";

const engine = new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
engine.compileSyntaxFromJson(syntaxJson);

const analyzer = engine.createAnalyzerByFileName("Main.java");
if (analyzer) {
  const highlight = analyzer.analyzeText(sourceCode);
}

const document = new sweetline.Document("file:///Main.java", sourceCode);
const documentAnalyzer = engine.loadDocument(document);
if (documentAnalyzer) {
  documentAnalyzer.analyze();
  const analyzed = documentAnalyzer.analyzeLineRange(new sweetline.LineRange(0, 80));
  const visible = documentAnalyzer.getHighlightSlice(new sweetline.LineRange(0, 80));
}
```

## 说明

- API 命名风格整体与 WebAssembly/TypeScript 侧一致。
- 编辑器场景建议优先使用 `DocumentAnalyzer` 做增量分析。
- 当需要某个可见区切片，并希望 SweetLine 先基于当前文档状态分析足够的行时，使用 `analyzeLineRange(...)`。
- 只需要当前可见窗口时，可在 `analyze()` / `analyzeIncremental(...)` 之后调用 `getHighlightSlice(...)`。
- 缩进划线可使用 `TextAnalyzer.analyzeIndentGuides` / `DocumentAnalyzer.analyzeIndentGuides`。
- 构建命令请参见 [构建文档](api_build.md)。
