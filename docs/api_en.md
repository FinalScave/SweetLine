# SweetLine API Reference

SweetLine provides multi-platform APIs. The core is written in C++17, with binding layers for C, Android (Java/Kotlin), WebAssembly (JavaScript/TypeScript), and more.

## Table of Contents

- [Core Concepts](#core-concepts)
- [C++ API](#c-api)
- [C API](#c-api-1)
- [Android API](#android-api)
- [WebAssembly API](#webassembly-api)

---

## Core Concepts

### Workflow

```
1. Create HighlightEngine
       │
2. Compile syntax rules (compileSyntaxFromJson / compileSyntaxFromFile)
       │
3. Choose analysis method
       ├── TextAnalyzer (full analysis, no document management)
       │     ├── analyzeText(text)     → full text analysis
       │     └── analyzeLine(text)     → single line analysis
       │
       └── DocumentAnalyzer (incremental analysis, managed document)
             ├── analyze()              → initial full analysis
             └── analyzeIncremental()   → incremental update analysis
```

### Two Types of Analyzers

| Feature | TextAnalyzer | DocumentAnalyzer |
|---------|-------------|-----------------|
| Document Management | No document management | Automatic document management |
| Incremental Updates | Not supported | Supported |
| Single Line Analysis | Supported | Not supported |
| Use Case | Static display, custom incremental logic | Real-time editor highlighting |

### Two Style Modes

| Mode | Description | Result Content |
|------|-------------|---------------|
| **Style ID Mode** (default) | Register external style name-to-ID mappings | `TokenSpan.style_id` |
| **Inline Style Mode** | Define colors directly in syntax JSON | `TokenSpan.inline_style` (foreground/background/font) |

---

## C++ API

### Headers

```cpp
#include "highlight.h"   // Core highlighting API
#include "foundation.h"  // Base data structures
#include "syntax.h"      // Syntax rule definitions
```

Namespace: `sweetline` (macro `NS_SWEETLINE`)

---

### HighlightEngine

The highlight engine is responsible for compiling syntax rules, creating analyzers, and managing style mappings.

```cpp
class HighlightEngine {
public:
    // Constructor
    explicit HighlightEngine(const HighlightConfig& config = HighlightConfig::kDefault);

    // Compile syntax rules (from JSON string)
    SharedPtr<SyntaxRule> compileSyntaxFromJson(const U8String& json);

    // Compile syntax rules (from file path)
    SharedPtr<SyntaxRule> compileSyntaxFromFile(const U8String& file);

    // Get compiled syntax rule by name
    SharedPtr<SyntaxRule> getSyntaxRuleByName(const U8String& name) const;

    // Get syntax rule by file extension
    SharedPtr<SyntaxRule> getSyntaxRuleByExtension(const U8String& extension) const;

    // Register style name-to-ID mapping
    void registerStyleName(const U8String& style_name, int32_t style_id) const;

    // Get style name by ID
    const U8String& getStyleName(int32_t style_id) const;

    // Define macros (for conditional compilation in syntax rules)
    void defineMacro(const U8String& macro_name);
    void undefineMacro(const U8String& macro_name);
    bool isMacroDefined(const U8String& macro_name) const;

    // Create TextAnalyzer (no incremental support)
    SharedPtr<TextAnalyzer> createAnalyzerByName(const U8String& syntax_name) const;
    SharedPtr<TextAnalyzer> createAnalyzerByExtension(const U8String& extension) const;

    // Load managed document, create DocumentAnalyzer (incremental support)
    SharedPtr<DocumentAnalyzer> loadDocument(const SharedPtr<Document>& document);

    // Remove managed document
    void removeDocument(const U8String& uri);
};
```

#### Usage Example

```cpp
#include "highlight.h"
using namespace sweetline;

auto engine = std::make_shared<HighlightEngine>();

// Register styles
engine->registerStyleName("keyword", 1);
engine->registerStyleName("string", 2);
engine->registerStyleName("comment", 3);

// Compile syntax rules
try {
    auto rule = engine->compileSyntaxFromFile("syntaxes/java.json");
} catch (const SyntaxRuleParseError& e) {
    std::cerr << "Compilation failed: " << e.what() << " (code=" << e.code() << ")" << std::endl;
}
```

---

### HighlightConfig

Highlighting configuration options.

```cpp
struct HighlightConfig {
    // Whether TokenSpan includes character index, default false (only line/column)
    bool show_index {false};

    // Whether to use inline style mode, default false (uses style IDs)
    bool inline_style {false};

    static HighlightConfig kDefault;
};
```

---

### Document

A managed document supporting incremental updates.

```cpp
class Document {
public:
    // Constructor
    explicit Document(const U8String& uri, const U8String& initial_text = "");

    // Set complete text
    void setText(const U8String& text);

    // Get document information
    U8String getUri() const;
    U8String getText() const;
    size_t totalChars() const;
    size_t getLineCount() const;
    size_t getLineCharCount(size_t line) const;
    const DocumentLine& getLine(size_t line) const;

    // Incremental updates
    int32_t patch(const TextRange& range, const U8String& new_text);
    int32_t appendText(const U8String& text);
    void insert(const TextPosition& position, const U8String& text);
    void remove(const TextRange& range);

    // Position conversion
    size_t charIndexOfLine(size_t line) const;
    TextPosition charIndexToPosition(size_t char_index) const;
};
```

---

### TextAnalyzer

Full-text analyzer, suitable for static highlighting scenarios.

```cpp
class TextAnalyzer {
public:
    // Analyze entire text
    SharedPtr<DocumentHighlight> analyzeText(const U8String& text);

    // Analyze single line (can be used to implement custom incremental analysis)
    void analyzeLine(const U8String& text, const TextLineInfo& line_info,
                     LineAnalyzeResult& result) const;
};
```

#### Full Analysis

```cpp
auto analyzer = engine->createAnalyzerByName("java");
auto highlight = analyzer->analyzeText(source_code);

for (auto& line : highlight->lines) {
    for (auto& span : line.spans) {
        // span.range.start.line, span.range.start.column
        // span.range.end.line, span.range.end.column
        // span.style_id
    }
}
```

#### Single Line Analysis

```cpp
auto analyzer = engine->createAnalyzerByName("java");

TextLineInfo info;
info.line = 0;
info.start_state = 0;  // Line 0 starts from default state
info.start_char_offset = 0;

LineAnalyzeResult result;
analyzer->analyzeLine("public class Hello {", info, result);

// result.highlight.spans - highlight spans for the current line
// result.end_state - end state (pass to the next line's start_state)
// result.char_count - character count of the current line
```

---

### DocumentAnalyzer

Incremental document analyzer, suitable for real-time editor highlighting.

```cpp
class DocumentAnalyzer {
public:
    // Full analysis
    SharedPtr<DocumentHighlight> analyze() const;

    // Incremental analysis (by range)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        const TextRange& range, const U8String& new_text) const;

    // Incremental analysis (by character index)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        size_t start_index, size_t end_index, const U8String& new_text) const;

    // Get managed document
    SharedPtr<Document> getDocument() const;
};
```

#### Usage Example

```cpp
auto doc = std::make_shared<Document>("file:///main.java", source_code);
auto analyzer = engine->loadDocument(doc);

// Initial full analysis
auto highlight = analyzer->analyze();

// User edits: replace line 2, columns 4-8 with "modified"
TextRange range {{2, 4}, {2, 8}};
auto new_highlight = analyzer->analyzeIncremental(range, "modified");
```

---

### Data Structures

```cpp
// Text position
struct TextPosition {
    size_t line {0};     // Line number (0-based)
    size_t column {0};   // Column number (0-based)
    size_t index {0};    // Character index in full text (requires show_index)
};

// Text range
struct TextRange {
    TextPosition start;
    TextPosition end;
};

// Highlight span
struct TokenSpan {
    TextRange range;           // Highlight range
    U8String matched_text;     // Matched text
    int32_t style_id;          // Style ID
    InlineStyle inline_style;  // Inline style (inline_style mode only)
};

// Line highlight
struct LineHighlight {
    List<TokenSpan> spans;
};

// Document highlight
struct DocumentHighlight {
    List<LineHighlight> lines;
    void toJson(U8String& result) const;  // Export as JSON
};

// Inline style
struct InlineStyle {
    int32_t foreground {0};    // Foreground color ARGB
    int32_t background {0};    // Background color ARGB
    bool is_bold {false};
    bool is_italic {false};
    bool is_strikethrough {false};
};
```

---

## C API

The C API provides a pure C interface wrapper, suitable for FFI integration (e.g., calling from Rust, Go, Python, etc.).

### Header

```c
#include "c_sweetline.h"
```

### Error Codes

```c
typedef enum sl_error {
    SL_OK                  =  0,  // Success
    SL_HANDLE_INVALID      =  1,  // Invalid handle
    SL_JSON_PROPERTY_MISSED = -1,  // Missing JSON property
    SL_JSON_PROPERTY_INVALID = -2, // Invalid JSON property value
    SL_PATTERN_INVALID     = -3,  // Regex pattern error
    SL_STATE_INVALID       = -4,  // State error
    SL_JSON_INVALID        = -5,  // JSON syntax error
    SL_FILE_IO_ERR         = -6,  // File IO error
    SL_FILE_EMPTY          = -7,  // File is empty
} sl_error_t;
```

### Handle Types

```c
typedef sl_document_handle* sl_document_handle_t;  // Document handle
typedef sl_engine_handle*   sl_engine_handle_t;     // Engine handle
typedef sl_analyzer_handle* sl_analyzer_handle_t;   // Analyzer handle
```

### Engine Management

```c
// Create engine
sl_engine_handle_t sl_create_engine(bool show_index, bool inline_style);

// Destroy engine
sl_error_t sl_free_engine(sl_engine_handle_t engine_handle);

// Compile syntax rules
sl_syntax_error_t sl_engine_compile_json(sl_engine_handle_t engine, const char* json);
sl_syntax_error_t sl_engine_compile_file(sl_engine_handle_t engine, const char* path);

// Style management
sl_error_t sl_engine_register_style_name(sl_engine_handle_t engine, const char* name, int32_t id);
const char* sl_engine_get_style_name(sl_engine_handle_t engine, int32_t id);

// Macro definitions
sl_error_t sl_engine_define_macro(sl_engine_handle_t engine, const char* macro_name);
sl_error_t sl_engine_undefine_macro(sl_engine_handle_t engine, const char* macro_name);
```

### Document Management

```c
// Create/destroy document
sl_document_handle_t sl_create_document(const char* uri, const char* text);
sl_error_t sl_free_document(sl_document_handle_t document);
```

### Full Analysis

```c
// Create text analyzer
sl_analyzer_handle_t sl_engine_create_text_analyzer(sl_engine_handle_t engine, const char* syntax_name);
sl_analyzer_handle_t sl_engine_create_text_analyzer2(sl_engine_handle_t engine, const char* extension);

// Full analysis
int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer, const char* text);

// Single line analysis
int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer, const char* text, int32_t* line_info);
```

### Incremental Analysis

```c
// Load managed document
sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine, sl_document_handle_t doc);

// Full analysis
int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer);

// Incremental analysis
int32_t* sl_document_analyze_incremental(sl_analyzer_handle_t analyzer,
                                          int32_t* changes_range,
                                          const char* new_text);
```

### Memory Management

```c
// Free analysis result buffer (must be called!)
void sl_free_buffer(int32_t* result);
```

### Return Value Format

All analysis functions return an `int32_t*` buffer with the following format:

```
result[0] = number of highlight spans (span_count)
result[1] = number of fields per span (fields_per_span)
followed by: span_count × fields_per_span int32_t values
```

**Style ID Mode** (`fields_per_span = 7`):
```
[startLine, startColumn, startIndex, endLine, endColumn, endIndex, styleId]
```

**Inline Style Mode** (`fields_per_span = 9`):
```
[startLine, startColumn, startIndex, endLine, endColumn, endIndex,
 foregroundColor, backgroundColor, fontAttributes]
```

`fontAttributes` bit flags:
- `fontAttributes & 1` → Bold
- `fontAttributes & 2` → Italic
- `fontAttributes & 4` → Strikethrough

Single line analysis `sl_text_analyze_line` additionally includes:
```
result[2] = end state ID (end_state)
result[3] = line character count (char_count)
```

### Complete C Example

```c
#include "c_sweetline.h"
#include <stdio.h>

int main() {
    // Create engine
    sl_engine_handle_t engine = sl_create_engine(false, false);

    // Register styles
    sl_engine_register_style_name(engine, "keyword", 1);
    sl_engine_register_style_name(engine, "string", 2);

    // Compile syntax rules
    sl_syntax_error_t err = sl_engine_compile_file(engine, "syntaxes/java.json");
    if (err.err_code != SL_OK) {
        printf("Compilation failed: %s\n", err.err_msg);
        sl_free_engine(engine);
        return 1;
    }

    // Create analyzer
    sl_analyzer_handle_t analyzer = sl_engine_create_text_analyzer(engine, "java");

    // Analyze text
    const char* code = "public class Hello { }";
    int32_t* result = sl_text_analyze(analyzer, code);

    if (result) {
        int32_t span_count = result[0];
        int32_t fields = result[1];

        for (int i = 0; i < span_count; i++) {
            int offset = 2 + i * fields;
            int startLine = result[offset];
            int startCol  = result[offset + 1];
            int endLine   = result[offset + 3];
            int endCol    = result[offset + 4];
            int styleId   = result[offset + 6];

            printf("(%d:%d)-(%d:%d) style=%d\n",
                   startLine, startCol, endLine, endCol, styleId);
        }

        // Free buffer
        sl_free_buffer(result);
    }

    sl_free_engine(engine);
    return 0;
}
```

---

## Android API

Android provides a Java API through JNI bindings. Class and function names are consistent with the C++ API.

### Dependency

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:0.0.4'
```

Or import via source dependency by adding the `platform/Android/sweetline` module to your project.

### Package

```java
import com.qiplat.sweetline.*;
```

### HighlightEngine

```java
public class HighlightEngine {
    public HighlightEngine(HighlightConfig config);

    // Compile syntax rules
    public SyntaxRule compileSyntaxFromJson(String json) throws SyntaxCompileError;
    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError;

    // Style management
    public void registerStyleName(String styleName, int styleId);
    public String getStyleName(int styleId);

    // Macro definitions
    public void defineMacro(String macroName);
    public void undefineMacro(String macroName);

    // Create analyzers
    public TextAnalyzer createAnalyzerByName(String syntaxName);
    public TextAnalyzer createAnalyzerByExtension(String extension);
    public DocumentAnalyzer loadDocument(Document document);
    public void removeDocument(String uri);
}
```

### HighlightConfig

```java
public class HighlightConfig {
    public boolean showIndex;     // Whether to include character index
    public boolean inlineStyle;   // Whether to use inline styles
}
```

### Document

```java
public class Document {
    public Document(String uri, String content);

    public String getUri();
    public String getText();
    public int totalChars();
    public int getLineCount();
    public int getLineCharCount(int line);
    public String getLine(int line);
    public int charIndexOfLine(int line);
    public TextPosition charIndexToPosition(int index);
}
```

### TextAnalyzer

```java
public class TextAnalyzer {
    // Full text analysis
    public DocumentHighlight analyzeText(String text);

    // Full text analysis, returns Android Spannable directly
    public Spannable analyzeTextAsSpannable(String text, SpannableStyleFactory factory);

    // Single line analysis
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info);
}
```

### DocumentAnalyzer

```java
public class DocumentAnalyzer {
    // Full analysis
    public DocumentHighlight analyze();

    // Incremental analysis (by range)
    public DocumentHighlight analyzeIncremental(TextRange range, String newText);

    // Incremental analysis (by character index)
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText);

    // Spannable variants
    public Spannable analyzeAsSpannable(SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText,
                                                    SpannableStyleFactory factory);

    // Get managed document
    public Document getDocument();
}
```

### Data Structures

```java
// Text position
public class TextPosition {
    public int line;
    public int column;
    public int index;
}

// Text range
public class TextRange {
    public TextPosition start;
    public TextPosition end;
}

// Highlight span
public class TokenSpan {
    public TextRange range;
    public int styleId;
    public InlineStyle inlineStyle;  // inlineStyle mode only
}

// Line highlight
public class LineHighlight {
    public List<TokenSpan> spans;
}

// Document highlight
public class DocumentHighlight {
    public List<LineHighlight> lines;
}

// Single line analysis result
public class LineAnalyzeResult {
    public LineHighlight highlight;
    public int endState;
    public int charCount;
}

// Line metadata
public class TextLineInfo {
    public int line;
    public int startState;
    public int startCharOffset;
}
```

### Complete Android Example

```java
import com.qiplat.sweetline.*;

// Create engine
HighlightConfig config = new HighlightConfig();
config.showIndex = true;
HighlightEngine engine = new HighlightEngine(config);

// Register styles
engine.registerStyleName("keyword", 1);
engine.registerStyleName("string", 2);
engine.registerStyleName("comment", 3);

// Read syntax rules from assets and compile
String json = readAssetFile("syntaxes/java.json");
try {
    engine.compileSyntaxFromJson(json);
} catch (SyntaxCompileError e) {
    Log.e("SweetLine", "Compilation failed: " + e.getMessage());
}

// Option 1: Full analysis
TextAnalyzer textAnalyzer = engine.createAnalyzerByName("java");
DocumentHighlight result = textAnalyzer.analyzeText(sourceCode);
for (LineHighlight line : result.lines) {
    for (TokenSpan span : line.spans) {
        Log.d("SweetLine", String.format("(%d:%d)-(%d:%d) style=%d",
            span.range.start.line, span.range.start.column,
            span.range.end.line, span.range.end.column,
            span.styleId));
    }
}

// Option 2: Incremental analysis
Document doc = new Document("file:///main.java", sourceCode);
DocumentAnalyzer docAnalyzer = engine.loadDocument(doc);
DocumentHighlight highlight = docAnalyzer.analyze();

// Incremental update on edit
TextRange changeRange = new TextRange(
    new TextPosition(2, 4, 0),
    new TextPosition(2, 8, 0)
);
DocumentHighlight newHighlight = docAnalyzer.analyzeIncremental(changeRange, "modified");

// Option 3: Convert to Spannable directly (for use with TextView)
Spannable spannable = textAnalyzer.analyzeTextAsSpannable(sourceCode, styleId -> {
    switch (styleId) {
        case 1: return new ForegroundColorSpan(Color.BLUE);   // keyword
        case 2: return new ForegroundColorSpan(Color.GREEN);  // string
        case 3: return new ForegroundColorSpan(Color.GRAY);   // comment
        default: return null;
    }
});
textView.setText(spannable);
```

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
}

class Document {
    constructor(uri: string, content: string);
    getUri(): string;
}

class TextAnalyzer {
    analyzeText(text: string): DocumentHighlight;
    analyzeLine(text: string, info: TextLineInfo): LineAnalyzeResult;
}

class DocumentAnalyzer {
    analyze(): DocumentHighlight;
    analyzeIncremental(range: TextRange, newText: string): DocumentHighlight;
    analyzeIncremental(startOffset: number, endOffset: number, newText: string): DocumentHighlight;
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

## Appendix: Platform Build

### C++ (CMake)

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
```

### Android (Gradle)

```bash
cd platform/Android
./gradlew :sweetline:assembleRelease
```

### WebAssembly (Emscripten)

```bash
mkdir build-wasm && cd build-wasm
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make
```

### Build Options

| CMake Option | Default | Description |
|-------------|---------|-------------|
| `BUILD_TESTING` | OFF | Whether to build tests |
| `STATIC_LIB` | ON | Whether to build static library |
| `SHARED_LIB` | OFF | Whether to build shared library |
