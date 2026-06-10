# SweetLine C API

This document describes the C API for FFI integration.

---

## C API

The C API provides a pure C interface wrapper, suitable for FFI integration (e.g., calling from Rust, Go, Python, etc.).

### Header

```c
#include "sweetline/c_sweetline.h"
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
sl_analyzer_handle_t sl_engine_create_text_analyzer_by_file_name(sl_engine_handle_t engine, const char* file_name);

// Full analysis
int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer, const char* text);

// Single line analysis
int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer, const char* text, int32_t* line_info);

// Indent guide analysis for plain text; no highlight pass is required
int32_t* sl_text_analyze_indent_guides(sl_analyzer_handle_t analyzer, const char* text);

// Bracket pair analysis for plain text; no highlight pass is required
int32_t* sl_text_analyze_bracket_pairs(sl_analyzer_handle_t analyzer, const char* text);
```

### Incremental Analysis

```c
// Load managed document
sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine, sl_document_handle_t doc);

// Full analysis
int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer);

// Analyze enough lines to cover the requested visible line-range slice
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_line_range(sl_analyzer_handle_t analyzer,
                                          int32_t* visible_range);

// Incremental analysis
int32_t* sl_document_analyze_incremental(sl_analyzer_handle_t analyzer,
                                          int32_t* changes_range,
                                          const char* new_text);

// Incremental analysis and return only a visible line-range slice
// changes_range layout: [startLine, startColumn, endLine, endColumn]
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_incremental_in_line_range(sl_analyzer_handle_t analyzer,
                                                         int32_t* changes_range,
                                                         const char* new_text,
                                                         int32_t* visible_range);

// Read only a visible line-range slice from the current cached highlight result
// Requires sl_document_analyze or sl_document_analyze_incremental first
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_get_highlight_slice(sl_analyzer_handle_t analyzer,
                                          int32_t* visible_range);

// Indent guide analysis for managed document
int32_t* sl_document_analyze_indent_guides(sl_analyzer_handle_t analyzer);

// Indent guide analysis for a visible line range
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_indent_guides_in_line_range(sl_analyzer_handle_t analyzer,
                                                          int32_t* visible_range);

// Bracket pair analysis for managed document
int32_t* sl_document_analyze_bracket_pairs(sl_analyzer_handle_t analyzer);

// Bracket pair analysis for a visible line range
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_bracket_pairs_in_line_range(sl_analyzer_handle_t analyzer,
                                                          int32_t* visible_range);
```

`sl_document_analyze_line_range(...)` analyzes enough lines from the current managed document state to satisfy the requested visible range.
`sl_document_get_highlight_slice(...)` only reads from the latest cached result and does not trigger a new analysis.

### Memory Management

```c
// Free analysis result buffer (must be called!)
void sl_free_buffer(int32_t* result);
```

### Return Value Format

All analysis functions return an `int32_t*` buffer.

For full document highlight results (`sl_text_analyze`, `sl_document_analyze`,
`sl_document_analyze_incremental`):

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = lineCount
followed by lineCount line entries:
  lineEntry[0] = spanCount of current line
  followed by spanCount * spanStride int32_t values
```

Span payload (compact protocol):

```
common fields: [column, length]
if show_index=true: append [startIndex]
if inline_style=true: append [foregroundColor, backgroundColor, fontAttributes]
else: append [styleId]
```

So `spanStride` is one of:
- `3` => `[column, length, styleId]`
- `4` => `[column, length, startIndex, styleId]`
- `5` => `[column, length, foregroundColor, backgroundColor, fontAttributes]`
- `6` => `[column, length, startIndex, foregroundColor, backgroundColor, fontAttributes]`

`fontAttributes` bit flags:
- `fontAttributes & 1` => Bold
- `fontAttributes & 2` => Italic
- `fontAttributes & 4` => Strikethrough

Single line analysis `sl_text_analyze_line` layout:

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = spanCount
result[3] = endState
result[4] = charCount
followed by spanCount * spanStride values (same compact span payload)
```

Visible line-range slice `sl_document_analyze_incremental_in_line_range` /
`sl_document_get_highlight_slice` layout:

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = startLine
result[3] = totalLineCount (after patch)
result[4] = lineCount (slice line count)
followed by lineCount line entries:
  lineEntry[0] = spanCount
  followed by spanCount * spanStride values
```

Indent guide analysis `sl_text_analyze_indent_guides`,
`sl_document_analyze_indent_guides`, and
`sl_document_analyze_indent_guides_in_line_range` return:

```
result[0] = slice start line (start_line)
result[1] = number of line states (line_state_count)
result[2] = number of indent guides (guide_count)

Then follow guide_count guide records:
[column, startLine, endLine, flags, branchCount, branchLine0, branchColumn0, ...]
flags bit0: continuesBefore
flags bit1: continuesAfter
Actual length of each guide record = 5 + branchCount * 2

Then follow line_state_count line-state records:
[nestingLevel, scopeState, scopeColumn, indentLevel]
scopeState: 0=START, 1=END, 2=CONTENT
```

Bracket pair analysis `sl_text_analyze_bracket_pairs` and
`sl_document_analyze_bracket_pairs` return:

```
result[0] = flags
            bit0: hasStartIndex
result[1] = bracketStride
result[2] = lineCount
followed by lineCount line entries:
  lineEntry[0] = bracket token count
  followed by tokenCount * bracketStride values
```

Bracket pair visible range analysis `sl_document_analyze_bracket_pairs_in_line_range` returns:

```
result[0] = flags
            bit0: hasStartIndex
result[1] = bracketStride
result[2] = startLine
result[3] = totalLineCount
result[4] = lineCount
followed by lineCount line entries:
  lineEntry[0] = bracket token count
  followed by tokenCount * bracketStride values
```

Bracket token payload:

```
common fields:
[column, length, depth, kind, matchState, partnerLine, partnerColumn, partnerLength]

if show_index=true:
[column, length, startIndex, depth, kind, matchState,
 partnerLine, partnerColumn, partnerLength, partnerStartIndex]
```

`kind`: `0=OPEN`, `1=CLOSE`.
`matchState`: `0=MATCHED`, `1=UNMATCHED`, `2=UNKNOWN`.
Partner fields are `-1` when no known partner exists.

### Complete C Example

```c
#include "sweetline/c_sweetline.h"
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
        int32_t flags = result[0];
        int32_t stride = result[1];
        int32_t line_count = result[2];
        int offset = 3;

        // Example assumes show_index=false, inline_style=false => span payload [column, length, styleId]
        (void)flags;
        for (int line = 0; line < line_count; line++) {
            int32_t span_count = result[offset++];
            for (int i = 0; i < span_count; i++) {
                int col = result[offset++];
                int len = result[offset++];
                int styleId = result[offset++];
                printf("line=%d col=%d len=%d style=%d\n", line, col, len, styleId);
            }
        }

        // Free buffer
        sl_free_buffer(result);
    }

    sl_free_engine(engine);
    return 0;
}
```

---
