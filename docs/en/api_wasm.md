# SweetLine WebAssembly API

This document describes JavaScript/TypeScript APIs exported by the Emscripten build.

---

## WebAssembly API

The WebAssembly version is compiled via Emscripten and provides a JavaScript/TypeScript API.

### Import

```javascript
// ES Module
import Module from './libsweetline.js';

const sweetline = await Module();
```

### TypeScript Types

The complete TypeScript type definition file is located at `platform/Emscripten/libsweetline.d.ts`.

### HighlightEngine

```typescript
class HighlightEngine {
    constructor(config: HighlightConfig);

    // Compile syntax rules
    compileSyntaxFromJson(json: string): SyntaxRule;
    compileSyntaxFromFile(path: string): SyntaxRule;

    // Find syntax rules
    getSyntaxRuleByName(name: string): SyntaxRule;
    getSyntaxRuleByExtension(extension: string): SyntaxRule;

    // Style management
    registerStyleName(styleName: string, styleId: number): void;
    getStyleName(styleId: number): string;

    // Macro definitions
    defineMacro(macroName: string): void;
    undefineMacro(macroName: string): void;

    // Create analyzers
    createAnalyzerByName(syntaxName: string): TextAnalyzer;
    createAnalyzerByExtension(extension: string): TextAnalyzer;
    loadDocument(document: Document): DocumentAnalyzer;
    removeDocument(uri: string): void;
}
```

### Core Types

```typescript
class HighlightConfig {
    showIndex: boolean;
    inlineStyle: boolean;
    tabSize: number;
}

class Document {
    constructor(uri: string, content: string);
    getUri(): string;
}

class TextAnalyzer {
    analyzeText(text: string): DocumentHighlight;
    analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;
    analyzeIndentGuides(text: string): IndentGuideResult;
}

class DocumentAnalyzer {
    analyze(): DocumentHighlight;
    analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;
    analyzeIncremental(startOffset: number, endOffset: number, newText: string): DocumentHighlight;
    analyzeIncrementalInLineRange(range: TextRange, newText: string, visibleRange: LineRange): DocumentHighlightSlice;
    analyzeIndentGuides(): IndentGuideResult;
}

class LineRange {
    startLine: number;
    lineCount: number;
}

class DocumentHighlightSlice {
    startLine: number;
    totalLineCount: number;
    lines: LineHighlightList;
}

class DocumentHighlight {
    lines: LineHighlightList;
    toJson(): string;
}

class LineHighlight {
    spans: TokenSpanList;
    toJson(): string;
}

class TokenSpan {
    range: TextRange;
    styleId: number;
    inlineStyle: InlineStyle;
}

class TextLineInfo {
    line: number;
    startState: number;
    startCharOffset: number;
}

class LineAnalyzeResult {
    highlight: LineHighlight;
    endState: number;
    charCount: number;
}
```

### Complete WASM Example

```javascript
import Module from './libsweetline.js';

async function main() {
    const sl = await Module();

    // Create engine
    const config = new sl.HighlightConfig();
    config.showIndex = false;
    config.inlineStyle = false;
    const engine = new sl.HighlightEngine(config);

    // Register styles
    engine.registerStyleName("keyword", 1);
    engine.registerStyleName("string", 2);
    engine.registerStyleName("comment", 3);

    // Compile syntax rules
    const jsonRule = `{
        "name": "demo",
        "fileExtensions": [".demo"],
        "states": {
            "default": [
                { "pattern": "\\\\b(if|else|while)\\\\b", "styles": [1, "keyword"] },
                { "pattern": "\\"[^\\"]*\\"", "style": "string" }
            ]
        }
    }`;
    engine.compileSyntaxFromJson(jsonRule);

    // Full analysis
    const analyzer = engine.createAnalyzerByName("demo");
    const code = 'if (x > 0) { return "hello"; }';
    const highlight = analyzer.analyzeText(code);

    // Iterate results
    for (let i = 0; i < highlight.lines.size(); i++) {
        const line = highlight.lines.get(i);
        for (let j = 0; j < line.spans.size(); j++) {
            const span = line.spans.get(j);
            console.log(`(${span.range.start.line}:${span.range.start.column})-` +
                        `(${span.range.end.line}:${span.range.end.column}) ` +
                        `style=${span.styleId}`);
        }
    }

    // Incremental analysis
    const doc = new sl.Document("file:///main.demo", code);
    const docAnalyzer = engine.loadDocument(doc);
    let result = docAnalyzer.analyze();

    // Simulate text edit
    const range = new sl.TextRange();
    range.start = new sl.TextPosition();
    range.start.line = 0;
    range.start.column = 0;
    range.end = new sl.TextPosition();
    range.end.line = 0;
    range.end.column = 2;
    result = docAnalyzer.analyzeIncremental(range, "while");

    // Export JSON
    console.log(result.toJson());
}

main();
```

---

