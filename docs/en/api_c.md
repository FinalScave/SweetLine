# SweetLine C API

This document describes the C API for FFI integration.

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

// Indent guide analysis for plain text (requires sl_text_analyze first)
int32_t* sl_text_analyze_indent_guides(sl_analyzer_handle_t analyzer, const char* text);
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

// Incremental analysis and return only a visible line-range slice
// changes_range layout: [startLine, startColumn, endLine, endColumn]
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_incremental_in_line_range(sl_analyzer_handle_t analyzer,
                                                         int32_t* changes_range,
                                                         const char* new_text,
                                                         int32_t* visible_range);

// Indent guide analysis for managed document
// (requires sl_document_analyze or sl_document_analyze_incremental first)
int32_t* sl_document_analyze_indent_guides(sl_analyzer_handle_t analyzer);
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

Incremental line-range slice analysis `sl_document_analyze_incremental_in_line_range` returns:
```
result[0] = slice start line (start_line)
result[1] = total line count after patch (total_line_count)
result[2] = line count in slice (line_count)
result[3] = number of highlight spans (span_count)
result[4] = number of fields per span (stride)
followed by: span_count × stride int32_t values
```

Indent guide analysis `sl_text_analyze_indent_guides` / `sl_document_analyze_indent_guides`
returns:
```
result[0] = number of indent guides (guide_count)
result[1] = fixed field count per guide (stride = 6)
result[2] = number of line states (line_count)
result[3] = field count per line state (line_stride = 4)

Then follow guide_count guide records:
[column, startLine, endLine, nestingLevel, scopeRuleId, branchCount, branchLine0, branchColumn0, ...]
Actual length of each guide record = stride + branchCount * 2

Then follow line_count line-state records:
[nestingLevel, scopeState, scopeColumn, indentLevel]
scopeState: 0=START, 1=END, 2=CONTENT
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

