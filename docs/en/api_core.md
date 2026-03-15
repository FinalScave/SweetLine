# SweetLine Core API

This document covers core concepts and the C++ API.

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

    // Tab width used to compute indent guide levels (1 tab = tab_size spaces)
    int32_t tab_size {4};

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

    // Analyze indent guides
    // Prefer the provided highlight, otherwise reuse the latest analyzeText cache
    // If no highlight is available, fallback to indentation-only analysis
    SharedPtr<IndentGuideResult> analyzeIndentGuides(
        const U8String& text, const SharedPtr<DocumentHighlight>& highlight = nullptr);

    // Get current highlight configuration
    const HighlightConfig& getHighlightConfig() const;
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

#### Indent Guide Analysis

```cpp
auto analyzer = engine->createAnalyzerByName("python");
auto highlight = analyzer->analyzeText(source_code);

// Explicitly pass highlight
auto guides1 = analyzer->analyzeIndentGuides(source_code, highlight);

// Omit highlight and reuse the latest analyzeText cache
auto guides2 = analyzer->analyzeIndentGuides(source_code);

// If no cache is available, it falls back to indentation-only analysis
auto fresh_analyzer = engine->createAnalyzerByName("python");
auto guides3 = fresh_analyzer->analyzeIndentGuides(source_code);
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

    // Incremental analysis that returns only a visible line-range slice
    SharedPtr<DocumentHighlightSlice> analyzeIncrementalInLineRange(
        const TextRange& range, const U8String& new_text, const LineRange& visible_range) const;

    // Incremental analysis (by character index)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        size_t start_index, size_t end_index, const U8String& new_text) const;

    // Get managed document
    SharedPtr<Document> getDocument() const;

    // Get current highlight configuration
    const HighlightConfig& getHighlightConfig() const;

    // Analyze indent guides (call analyze or analyzeIncremental first)
    SharedPtr<IndentGuideResult> analyzeIndentGuides() const;
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

// Return only the visible slice [100, 100 + 60)
LineRange visible {100, 60};
auto slice = analyzer->analyzeIncrementalInLineRange(range, "modified", visible);

// Build indent guides
auto guides = analyzer->analyzeIndentGuides();
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

// Line range
struct LineRange {
    size_t start_line {0};
    size_t line_count {0};
};

// Highlight slice for a visible line range
struct DocumentHighlightSlice {
    size_t start_line {0};
    size_t total_line_count {0};
    List<LineHighlight> lines;
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

