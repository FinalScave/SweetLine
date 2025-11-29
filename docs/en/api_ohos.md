# SweetLine HarmonyOS API

SweetLine provides ArkTS bindings for HarmonyOS via NAPI.

## Package

- Package name: `@qiplat/sweetline`
- Main entry: `platform/OHOS/sweetline/src/main/ets/Index.ets`
- Type definitions: `platform/OHOS/sweetline/src/main/ets/Index.d.ts`

## Core Classes

- `sweetline.HighlightEngine`
- `sweetline.HighlightConfig`
- `sweetline.Document`
- `sweetline.TextAnalyzer`
- `sweetline.DocumentAnalyzer`
- `sweetline.SyntaxRule`

## Key API Surface (ArkTS)

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

## Quick Example (ArkTS)

```ts
import { sweetline } from "@qiplat/sweetline";

const engine = new sweetline.HighlightEngine(new sweetline.HighlightConfig(true, false));
engine.compileSyntaxFromJson(syntaxJson);

const analyzer = engine.createAnalyzerByName("java");
if (analyzer) {
  const highlight = analyzer.analyzeText(sourceCode);
}
```

## Notes

- API naming largely follows WebAssembly/TypeScript style.
- For incremental highlighting, prefer `DocumentAnalyzer`.
- For indent guides, use `TextAnalyzer.analyzeIndentGuides` / `DocumentAnalyzer.analyzeIndentGuides`.
- For build commands, see [Build Guide](api_build.md).

