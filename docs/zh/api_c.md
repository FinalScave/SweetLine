# SweetLine C API 文档

本文档描述 C 语言接口，适用于 FFI 集成。

---

## C API

C API 提供纯 C 接口封装，适用于 FFI 集成（如 Rust、Go、Python 等语言调用）。

### 头文件

```c
#include "sweetline/c_sweetline.h"
```

### 错误码

```c
typedef enum sl_error {
    SL_OK                  =  0,  // 成功
    SL_HANDLE_INVALID      =  1,  // 句柄不合法
    SL_JSON_PROPERTY_MISSED = -1,  // JSON 缺少属性
    SL_JSON_PROPERTY_INVALID = -2, // JSON 属性值错误
    SL_PATTERN_INVALID     = -3,  // 正则表达式错误
    SL_STATE_INVALID       = -4,  // 状态错误
    SL_JSON_INVALID        = -5,  // JSON 语法错误
    SL_FILE_IO_ERR         = -6,  // 文件 IO 错误
    SL_FILE_EMPTY          = -7,  // 文件内容为空
} sl_error_t;
```

### 句柄类型

```c
typedef sl_document_handle* sl_document_handle_t;  // 文档句柄
typedef sl_engine_handle*   sl_engine_handle_t;     // 引擎句柄
typedef sl_analyzer_handle* sl_analyzer_handle_t;   // 分析器句柄
```

### 引擎管理

```c
// 创建引擎
sl_engine_handle_t sl_create_engine(bool show_index, bool inline_style, int32_t tab_size);

// 销毁引擎
sl_error_t sl_free_engine(sl_engine_handle_t engine_handle);

// 编译语法规则
sl_syntax_error_t sl_engine_compile_json(sl_engine_handle_t engine, const char* json);
sl_syntax_error_t sl_engine_compile_file(sl_engine_handle_t engine, const char* path);

// 样式管理
sl_error_t sl_engine_register_style_name(sl_engine_handle_t engine, const char* name, int32_t id);
const char* sl_engine_get_style_name(sl_engine_handle_t engine, int32_t id);

// 宏定义
sl_error_t sl_engine_define_macro(sl_engine_handle_t engine, const char* macro_name);
sl_error_t sl_engine_undefine_macro(sl_engine_handle_t engine, const char* macro_name);
```

### 文档管理

```c
// 创建/销毁文档
sl_document_handle_t sl_create_document(const char* uri, const char* text);
sl_error_t sl_free_document(sl_document_handle_t document);
```

### 全量分析

```c
// 创建文本分析器
sl_analyzer_handle_t sl_engine_create_text_analyzer(sl_engine_handle_t engine, const char* syntax_name);
sl_analyzer_handle_t sl_engine_create_text_analyzer_by_file_name(sl_engine_handle_t engine, const char* file_name);

// 全量分析
int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer, const char* text);

// 单行分析
int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer, const char* text, int32_t* line_info);

// Plain text indent guide analysis; no highlight pass is required
int32_t* sl_text_analyze_indent_guides(sl_analyzer_handle_t analyzer, const char* text);

// Bracket pair analysis for plain text; no highlight pass is required
int32_t* sl_text_analyze_bracket_pairs(sl_analyzer_handle_t analyzer, const char* text);
```

### 增量分析

```c
// 加载托管文档
sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine, sl_document_handle_t doc);

// 全量分析
int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer);

// 按当前文档状态分析足够的行，并返回指定可见行范围切片
// visible_range 数组结构: [startLine, lineCount]
int32_t* sl_document_analyze_line_range(sl_analyzer_handle_t analyzer,
                                          int32_t* visible_range);

// 增量分析
int32_t* sl_document_analyze_incremental(sl_analyzer_handle_t analyzer,
                                          int32_t* changes_range,
                                          const char* new_text);

// 增量分析并只返回可见行范围高亮切片
// changes_range 数组结构: [startLine, startColumn, endLine, endColumn]
// visible_range 数组结构: [startLine, lineCount]
int32_t* sl_document_analyze_incremental_in_line_range(sl_analyzer_handle_t analyzer,
                                                         int32_t* changes_range,
                                                         const char* new_text,
                                                         int32_t* visible_range);

// 从当前缓存的高亮结果中只读取可见行区域切片
// 需先调用 sl_document_analyze 或 sl_document_analyze_incremental
// visible_range 数组结构: [startLine, lineCount]
int32_t* sl_document_get_highlight_slice(sl_analyzer_handle_t analyzer,
                                          int32_t* visible_range);

// 托管文档缩进划线分析
int32_t* sl_document_analyze_indent_guides(sl_analyzer_handle_t analyzer);

// 可见行范围缩进划线分析
// visible_range 数组结构: [startLine, lineCount]
int32_t* sl_document_analyze_indent_guides_in_line_range(sl_analyzer_handle_t analyzer,
                                                          int32_t* visible_range);

// Bracket pair analysis for managed document
int32_t* sl_document_analyze_bracket_pairs(sl_analyzer_handle_t analyzer);

// Bracket pair analysis for a visible line range
// visible_range layout: [startLine, lineCount]
int32_t* sl_document_analyze_bracket_pairs_in_line_range(sl_analyzer_handle_t analyzer,
                                                          int32_t* visible_range);
```

`sl_document_analyze_line_range(...)` 会基于当前托管文档状态分析足够的行，以覆盖请求的可见区。
`sl_document_get_highlight_slice(...)` 只读取最近缓存结果，不会触发新的分析。

### 内存管理

```c
// 释放分析结果缓冲区 (必须调用!)
void sl_free_buffer(int32_t* result);
```

### 返回值格式

所有分析函数都返回一个 `int32_t*` 缓冲区。

完整文档高亮结果（`sl_text_analyze`、`sl_document_analyze`、`sl_document_analyze_incremental`）布局：

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = lineCount
后续是 lineCount 个行条目：
  lineEntry[0] = 当前行 spanCount
  后续是 spanCount * spanStride 个 int32_t 字段
```

紧凑 span 协议：

```
公共字段: [column, length]
如果 show_index=true: 追加 [startIndex]
如果 inline_style=true: 追加 [foregroundColor, backgroundColor, fontAttributes]
否则: 追加 [styleId]
```

因此 `spanStride` 只会是：
- `3` => `[column, length, styleId]`
- `4` => `[column, length, startIndex, styleId]`
- `5` => `[column, length, foregroundColor, backgroundColor, fontAttributes]`
- `6` => `[column, length, startIndex, foregroundColor, backgroundColor, fontAttributes]`

`fontAttributes` 位标记：
- `fontAttributes & 1` => 粗体
- `fontAttributes & 2` => 斜体
- `fontAttributes & 4` => 删除线

单行分析 `sl_text_analyze_line` 布局：

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = spanCount
result[3] = endState
result[4] = charCount
后续是 spanCount * spanStride（同一套紧凑 span 协议）
```

按可见行返回切片 `sl_document_analyze_incremental_in_line_range` /
`sl_document_get_highlight_slice` 布局：

```
result[0] = flags
            bit0: hasStartIndex
            bit1: inlineStyle
result[1] = spanStride
result[2] = startLine
result[3] = totalLineCount (patch 后总行数)
result[4] = lineCount (切片行数)
后续是 lineCount 个行条目：
  lineEntry[0] = spanCount
  后续是 spanCount * spanStride 个字段
```

缩进划线分析 `sl_text_analyze_indent_guides`、`sl_document_analyze_indent_guides`
和 `sl_document_analyze_indent_guides_in_line_range` 返回格式：

```
result[0] = 切片起始行 (start_line)
result[1] = 行状态数量 (line_state_count)
result[2] = 缩进划线数量 (guide_count)

之后是 guide_count 条划线记录：
[column, startLine, endLine, flags, branchCount, branchLine0, branchColumn0, ...]
flags bit0: continuesBefore
flags bit1: continuesAfter
每条实际长度 = 5 + branchCount * 2

之后是 line_state_count 条行状态记录：
[nestingLevel, scopeState, scopeColumn, indentLevel]
scopeState: 0=START, 1=END, 2=CONTENT
```

括号匹配分析 `sl_text_analyze_bracket_pairs` 和
`sl_document_analyze_bracket_pairs` 返回格式：

```
result[0] = flags
            bit0: hasStartIndex
result[1] = bracketStride
result[2] = lineCount
后续是 lineCount 个行条目：
  lineEntry[0] = 当前行括号 token 数量
  后续是 tokenCount * bracketStride 个字段
```

可见行范围括号匹配分析 `sl_document_analyze_bracket_pairs_in_line_range` 返回格式：

```
result[0] = flags
            bit0: hasStartIndex
result[1] = bracketStride
result[2] = startLine
result[3] = totalLineCount
result[4] = lineCount
后续是 lineCount 个行条目：
  lineEntry[0] = 当前行括号 token 数量
  后续是 tokenCount * bracketStride 个字段
```

括号 token 载荷：

```
公共字段:
[column, length, depth, kind, matchState, partnerLine, partnerColumn, partnerLength]

如果 show_index=true:
[column, length, startIndex, depth, kind, matchState,
 partnerLine, partnerColumn, partnerLength, partnerStartIndex]
```

`kind`: `0=OPEN`, `1=CLOSE`。
`matchState`: `0=MATCHED`, `1=UNMATCHED`, `2=UNKNOWN`。
没有已知匹配对象时，partner 字段为 `-1`。

### 完整 C 示例

```c
#include "sweetline/c_sweetline.h"
#include <stdio.h>

int main() {
    // 创建引擎
    sl_engine_handle_t engine = sl_create_engine(false, false, 4);

    // 注册样式
    sl_engine_register_style_name(engine, "keyword", 1);
    sl_engine_register_style_name(engine, "string", 2);

    // 编译语法规则
    sl_syntax_error_t err = sl_engine_compile_file(engine, "syntaxes/java.json");
    if (err.err_code != SL_OK) {
        printf("编译失败: %s\n", err.err_msg);
        sl_free_engine(engine);
        return 1;
    }

    // 创建分析器
    sl_analyzer_handle_t analyzer = sl_engine_create_text_analyzer(engine, "java");

    // 分析文本
    const char* code = "public class Hello { }";
    int32_t* result = sl_text_analyze(analyzer, code);

    if (result) {
        int32_t flags = result[0];
        int32_t stride = result[1];
        int32_t line_count = result[2];
        int offset = 3;

        // 此示例假设 show_index=false 且 inline_style=false
        // span 负载为 [column, length, styleId]
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

        // 释放缓冲区
        sl_free_buffer(result);
    }

    sl_free_engine(engine);
    return 0;
}
```

---
