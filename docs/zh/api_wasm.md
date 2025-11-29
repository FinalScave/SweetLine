# SweetLine WebAssembly API 文档

本文档描述 Emscripten 导出的 JavaScript/TypeScript API。

---

## WebAssembly API

WebAssembly 版本通过 Emscripten 编译，提供 JavaScript/TypeScript API。

### 引入方式

```javascript
// ES Module
import Module from './libsweetline.js';

const sweetline = await Module();
```

### TypeScript 类型

完整的 TypeScript 类型定义文件位于 `platform/Emscripten/libsweetline.d.ts`。

### HighlightEngine

```typescript
class HighlightEngine {
    constructor(config: HighlightConfig);

    // 编译语法规则
    compileSyntaxFromJson(json: string): SyntaxRule;
    compileSyntaxFromFile(path: string): SyntaxRule;

    // 查找语法规则
    getSyntaxRuleByName(name: string): SyntaxRule;
    getSyntaxRuleByExtension(extension: string): SyntaxRule;

    // 样式管理
    registerStyleName(styleName: string, styleId: number): void;
    getStyleName(styleId: number): string;

    // 宏定义
    defineMacro(macroName: string): void;
    undefineMacro(macroName: string): void;

    // 创建分析器
    createAnalyzerByName(syntaxName: string): TextAnalyzer;
    createAnalyzerByExtension(extension: string): TextAnalyzer;
    loadDocument(document: Document): DocumentAnalyzer;
    removeDocument(uri: string): void;
}
```

### 核心类型

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

### 完整 WASM 示例

```javascript
import Module from './libsweetline.js';

async function main() {
    const sl = await Module();

    // 创建引擎
    const config = new sl.HighlightConfig();
    config.showIndex = false;
    config.inlineStyle = false;
    const engine = new sl.HighlightEngine(config);

    // 注册样式
    engine.registerStyleName("keyword", 1);
    engine.registerStyleName("string", 2);
    engine.registerStyleName("comment", 3);

    // 编译语法规则
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

    // 全量分析
    const analyzer = engine.createAnalyzerByName("demo");
    const code = 'if (x > 0) { return "hello"; }';
    const highlight = analyzer.analyzeText(code);

    // 遍历结果
    for (let i = 0; i < highlight.lines.size(); i++) {
        const line = highlight.lines.get(i);
        for (let j = 0; j < line.spans.size(); j++) {
            const span = line.spans.get(j);
            console.log(`(${span.range.start.line}:${span.range.start.column})-` +
                        `(${span.range.end.line}:${span.range.end.column}) ` +
                        `style=${span.styleId}`);
        }
    }

    // 增量分析
    const doc = new sl.Document("file:///main.demo", code);
    const docAnalyzer = engine.loadDocument(doc);
    let result = docAnalyzer.analyze();

    // 模拟文本编辑
    const range = new sl.TextRange();
    range.start = new sl.TextPosition();
    range.start.line = 0;
    range.start.column = 0;
    range.end = new sl.TextPosition();
    range.end.line = 0;
    range.end.column = 2;
    result = docAnalyzer.analyzeIncremental(range, "while");

    // 导出 JSON
    console.log(result.toJson());
}

main();
```

---

