# SweetLine C API 文档

本文档描述 C 语言接口，适用于 FFI 集成。

---

## C API

C API 提供纯 C 接口封装，适用于 FFI 集成（如 Rust、Go、Python 等语言调用）。

### 头文件

```c
#include "c_sweetline.h"
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
sl_engine_handle_t sl_create_engine(bool show_index, bool inline_style);

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
sl_analyzer_handle_t sl_engine_create_text_analyzer2(sl_engine_handle_t engine, const char* extension);

// 全量分析
int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer, const char* text);

// 单行分析
int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer, const char* text, int32_t* line_info);

// 纯文本缩进划线分析 (需先调用 sl_text_analyze)
int32_t* sl_text_analyze_indent_guides(sl_analyzer_handle_t analyzer, const char* text);
```

### 增量分析

```c
// 加载托管文档
sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine, sl_document_handle_t doc);

// 全量分析
int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer);

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

// 托管文档缩进划线分析 (需先调用 sl_document_analyze 或 sl_document_analyze_incremental)
int32_t* sl_document_analyze_indent_guides(sl_analyzer_handle_t analyzer);
```

### 内存管理

```c
// 释放分析结果缓冲区 (必须调用!)
void sl_free_buffer(int32_t* result);
```

### 返回值格式

所有分析函数返回 `int32_t*` 缓冲区，格式如下：

```
result[0] = 高亮块数量 (span_count)
result[1] = 每个高亮块的字段数 (fields_per_span)
后续: span_count × fields_per_span 个 int32_t
```

**样式 ID 模式** (`fields_per_span = 7`)：
```
[startLine, startColumn, startIndex, endLine, endColumn, endIndex, styleId]
```

**内联样式模式** (`fields_per_span = 9`)：
```
[startLine, startColumn, startIndex, endLine, endColumn, endIndex,
 foregroundColor, backgroundColor, fontAttributes]
```

`fontAttributes` 位标志：
- `fontAttributes & 1` → 粗体
- `fontAttributes & 2` → 斜体
- `fontAttributes & 4` → 删除线

单行分析 `sl_text_analyze_line` 额外包含：
```
result[2] = 行结束状态 ID (end_state)
result[3] = 行字符总数 (char_count)
```

行范围切片增量分析 `sl_document_analyze_incremental_in_line_range` 返回格式：
```
result[0] = 切片起始行 (start_line)
result[1] = patch 后文档总行数 (total_line_count)
result[2] = 切片行数 (line_count)
result[3] = 高亮块数量 (span_count)
result[4] = 每个高亮块字段数 (stride)
后续: span_count × stride 个 int32_t
```

缩进划线分析 `sl_text_analyze_indent_guides` / `sl_document_analyze_indent_guides` 返回格式：
```
result[0] = 缩进划线数量 (guide_count)
result[1] = 每条划线固定字段数 (stride = 6)
result[2] = 行状态数量 (line_count)
result[3] = 每行状态字段数 (line_stride = 4)

之后是 guide_count 条划线数据，每条结构：
[column, startLine, endLine, nestingLevel, scopeRuleId, branchCount, branchLine0, branchColumn0, ...]
每条实际长度 = stride + branchCount * 2

之后是 line_count 条行状态数据，每条结构：
[nestingLevel, scopeState, scopeColumn, indentLevel]
scopeState: 0=START, 1=END, 2=CONTENT
```

### 完整 C 示例

```c
#include "c_sweetline.h"
#include <stdio.h>

int main() {
    // 创建引擎
    sl_engine_handle_t engine = sl_create_engine(false, false);

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

        // 释放缓冲区
        sl_free_buffer(result);
    }

    sl_free_engine(engine);
    return 0;
}
```

---

