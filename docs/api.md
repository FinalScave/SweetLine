# SweetLine API 文档

SweetLine 提供多平台 API，核心使用 C++17 编写，并通过不同的绑定层支持 C、Android (Java/Kotlin)、WebAssembly (JavaScript/TypeScript) 等平台。

## 目录

- [核心概念](#核心概念)
- [C++ API](#c-api)
- [C API](#c-api-1)
- [Android API](#android-api)
- [WebAssembly API](#webassembly-api)

---

## 核心概念

### 工作流程

```
1. 创建 HighlightEngine (引擎)
       │
2. 编译语法规则 (compileSyntaxFromJson / compileSyntaxFromFile)
       │
3. 选择分析方式
       ├── TextAnalyzer (全量分析，不托管文档)
       │     ├── analyzeText(text)     → 全文分析
       │     └── analyzeLine(text)     → 单行分析
       │
       └── DocumentAnalyzer (增量分析，托管文档)
             ├── analyze()              → 首次全量分析
             └── analyzeIncremental()   → 增量更新分析
```

### 两种分析器

| 特性 | TextAnalyzer | DocumentAnalyzer |
|------|-------------|-----------------|
| 文档管理 | 不托管文档 | 自动托管文档 |
| 增量更新 | 不支持 | 支持 |
| 单行分析 | 支持 | 不支持 |
| 适用场景 | 静态展示、自行管理增量 | 编辑器实时高亮 |

### 两种样式模式

| 模式 | 说明 | 结果内容 |
|------|------|----------|
| **样式 ID 模式**（默认） | 外部注册样式名称到 ID 映射 | `TokenSpan.style_id` |
| **内联样式模式** | 语法 JSON 中直接定义颜色 | `TokenSpan.inline_style` (前景色/背景色/字体) |

---

## C++ API

### 头文件

```cpp
#include "highlight.h"   // 核心高亮 API
#include "foundation.h"  // 基础数据结构
#include "syntax.h"      // 语法规则定义
```

命名空间：`sweetline`（宏 `NS_SWEETLINE`）

---

### HighlightEngine

高亮引擎，负责编译语法规则、创建分析器、管理样式映射。

```cpp
class HighlightEngine {
public:
    // 构造函数
    explicit HighlightEngine(const HighlightConfig& config = HighlightConfig::kDefault);

    // 编译语法规则 (从 JSON 字符串)
    SharedPtr<SyntaxRule> compileSyntaxFromJson(const U8String& json);

    // 编译语法规则 (从文件路径)
    SharedPtr<SyntaxRule> compileSyntaxFromFile(const U8String& file);

    // 根据名称获取已编译的语法规则
    SharedPtr<SyntaxRule> getSyntaxRuleByName(const U8String& name) const;

    // 根据文件扩展名获取语法规则
    SharedPtr<SyntaxRule> getSyntaxRuleByExtension(const U8String& extension) const;

    // 注册样式名称到 ID 的映射
    void registerStyleName(const U8String& style_name, int32_t style_id) const;

    // 根据 ID 获取样式名称
    const U8String& getStyleName(int32_t style_id) const;

    // 定义宏 (用于语法规则的条件编译)
    void defineMacro(const U8String& macro_name);
    void undefineMacro(const U8String& macro_name);
    bool isMacroDefined(const U8String& macro_name) const;

    // 创建 TextAnalyzer (不支持增量分析)
    SharedPtr<TextAnalyzer> createAnalyzerByName(const U8String& syntax_name) const;
    SharedPtr<TextAnalyzer> createAnalyzerByExtension(const U8String& extension) const;

    // 加载托管文档，创建 DocumentAnalyzer (支持增量分析)
    SharedPtr<DocumentAnalyzer> loadDocument(const SharedPtr<Document>& document);

    // 移除托管文档
    void removeDocument(const U8String& uri);
};
```

#### 使用示例

```cpp
#include "highlight.h"
using namespace sweetline;

auto engine = std::make_shared<HighlightEngine>();

// 注册样式
engine->registerStyleName("keyword", 1);
engine->registerStyleName("string", 2);
engine->registerStyleName("comment", 3);

// 编译语法规则
try {
    auto rule = engine->compileSyntaxFromFile("syntaxes/java.json");
} catch (const SyntaxRuleParseError& e) {
    std::cerr << "编译失败: " << e.what() << " (code=" << e.code() << ")" << std::endl;
}
```

---

### HighlightConfig

高亮配置选项。

```cpp
struct HighlightConfig {
    // TokenSpan 是否包含字符索引 (index), 默认 false (只有 line/column)
    bool show_index {false};

    // 是否使用内联样式模式, 默认 false (使用样式 ID)
    bool inline_style {false};

    static HighlightConfig kDefault;
};
```

---

### Document

支持增量更新的托管文档。

```cpp
class Document {
public:
    // 构造函数
    explicit Document(const U8String& uri, const U8String& initial_text = "");

    // 设置完整文本
    void setText(const U8String& text);

    // 获取文档信息
    U8String getUri() const;
    U8String getText() const;
    size_t totalChars() const;
    size_t getLineCount() const;
    size_t getLineCharCount(size_t line) const;
    const DocumentLine& getLine(size_t line) const;

    // 增量更新
    int32_t patch(const TextRange& range, const U8String& new_text);
    int32_t appendText(const U8String& text);
    void insert(const TextPosition& position, const U8String& text);
    void remove(const TextRange& range);

    // 位置转换
    size_t charIndexOfLine(size_t line) const;
    TextPosition charIndexToPosition(size_t char_index) const;
};
```

---

### TextAnalyzer

全量文本分析器，适用于静态高亮场景。

```cpp
class TextAnalyzer {
public:
    // 分析整段文本
    SharedPtr<DocumentHighlight> analyzeText(const U8String& text);

    // 分析单行文本 (可用于自行实现增量分析)
    void analyzeLine(const U8String& text, const TextLineInfo& line_info,
                     LineAnalyzeResult& result) const;
};
```

#### 全量分析

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

#### 单行分析

```cpp
auto analyzer = engine->createAnalyzerByName("java");

TextLineInfo info;
info.line = 0;
info.start_state = 0;  // 第 0 行从 default 状态开始
info.start_char_offset = 0;

LineAnalyzeResult result;
analyzer->analyzeLine("public class Hello {", info, result);

// result.highlight.spans - 当前行的高亮块
// result.end_state - 行结束状态 (传给下一行的 start_state)
// result.char_count - 当前行字符数
```

---

### DocumentAnalyzer

增量文档分析器，适用于编辑器实时高亮场景。

```cpp
class DocumentAnalyzer {
public:
    // 全量分析
    SharedPtr<DocumentHighlight> analyze() const;

    // 增量分析 (通过范围)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        const TextRange& range, const U8String& new_text) const;

    // 增量分析 (通过字符索引)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        size_t start_index, size_t end_index, const U8String& new_text) const;

    // 获取托管文档
    SharedPtr<Document> getDocument() const;
};
```

#### 使用示例

```cpp
auto doc = std::make_shared<Document>("file:///main.java", source_code);
auto analyzer = engine->loadDocument(doc);

// 首次全量分析
auto highlight = analyzer->analyze();

// 用户编辑: 将第 2 行第 4-8 列替换为 "modified"
TextRange range {{2, 4}, {2, 8}};
auto new_highlight = analyzer->analyzeIncremental(range, "modified");
```

---

### 数据结构

```cpp
// 文本位置
struct TextPosition {
    size_t line {0};     // 行号 (0 起始)
    size_t column {0};   // 列号 (0 起始)
    size_t index {0};    // 全文字符索引 (需开启 show_index)
};

// 文本范围
struct TextRange {
    TextPosition start;
    TextPosition end;
};

// 高亮块
struct TokenSpan {
    TextRange range;           // 高亮范围
    U8String matched_text;     // 匹配的文本
    int32_t style_id;          // 样式 ID
    InlineStyle inline_style;  // 内联样式 (仅 inline_style 模式)
};

// 行高亮
struct LineHighlight {
    List<TokenSpan> spans;
};

// 文档高亮
struct DocumentHighlight {
    List<LineHighlight> lines;
    void toJson(U8String& result) const;  // 导出为 JSON
};

// 内联样式
struct InlineStyle {
    int32_t foreground {0};    // 前景色 ARGB
    int32_t background {0};    // 背景色 ARGB
    bool is_bold {false};
    bool is_italic {false};
    bool is_strikethrough {false};
};
```

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

## Android API

Android 通过 JNI 绑定提供 Java API，类名和函数名与 C++ API 保持一致。

### 依赖引入

```groovy
// build.gradle
implementation 'com.qiplat:sweetline:0.1.1'
```

或通过源码依赖，将 `platform/Android/sweetline` 模块导入项目。

### 包名

```java
import com.qiplat.sweetline.*;
```

### HighlightEngine

```java
public class HighlightEngine {
    public HighlightEngine(HighlightConfig config);

    // 编译语法规则
    public SyntaxRule compileSyntaxFromJson(String json) throws SyntaxCompileError;
    public SyntaxRule compileSyntaxFromFile(String path) throws SyntaxCompileError;

    // 样式管理
    public void registerStyleName(String styleName, int styleId);
    public String getStyleName(int styleId);

    // 宏定义
    public void defineMacro(String macroName);
    public void undefineMacro(String macroName);

    // 创建分析器
    public TextAnalyzer createAnalyzerByName(String syntaxName);
    public TextAnalyzer createAnalyzerByExtension(String extension);
    public DocumentAnalyzer loadDocument(Document document);
    public void removeDocument(String uri);
}
```

### HighlightConfig

```java
public class HighlightConfig {
    public boolean showIndex;     // 是否包含字符索引
    public boolean inlineStyle;   // 是否使用内联样式
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
    // 全文分析
    public DocumentHighlight analyzeText(String text);

    // 全文分析，直接返回 Android Spannable
    public Spannable analyzeTextAsSpannable(String text, SpannableStyleFactory factory);

    // 单行分析
    public LineAnalyzeResult analyzeLine(String text, TextLineInfo info);
}
```

### DocumentAnalyzer

```java
public class DocumentAnalyzer {
    // 全量分析
    public DocumentHighlight analyze();

    // 增量分析 (通过范围)
    public DocumentHighlight analyzeIncremental(TextRange range, String newText);

    // 增量分析 (通过字符索引)
    public DocumentHighlight analyzeIncremental(int startIndex, int endIndex, String newText);

    // Spannable 相关
    public Spannable analyzeAsSpannable(SpannableStyleFactory factory);
    public Spannable analyzeIncrementalAsSpannable(TextRange range, String newText,
                                                    SpannableStyleFactory factory);

    // 获取托管文档
    public Document getDocument();
}
```

### 数据结构

```java
// 文本位置
public class TextPosition {
    public int line;
    public int column;
    public int index;
}

// 文本范围
public class TextRange {
    public TextPosition start;
    public TextPosition end;
}

// 高亮块
public class TokenSpan {
    public TextRange range;
    public int styleId;
    public InlineStyle inlineStyle;  // 仅 inlineStyle 模式
}

// 行高亮
public class LineHighlight {
    public List<TokenSpan> spans;
}

// 文档高亮
public class DocumentHighlight {
    public List<LineHighlight> lines;
}

// 单行分析结果
public class LineAnalyzeResult {
    public LineHighlight highlight;
    public int endState;
    public int charCount;
}

// 行元数据
public class TextLineInfo {
    public int line;
    public int startState;
    public int startCharOffset;
}
```

### 完整 Android 示例

```java
import com.qiplat.sweetline.*;

// 创建引擎
HighlightConfig config = new HighlightConfig();
config.showIndex = true;
HighlightEngine engine = new HighlightEngine(config);

// 注册样式
engine.registerStyleName("keyword", 1);
engine.registerStyleName("string", 2);
engine.registerStyleName("comment", 3);

// 从 assets 读取语法规则并编译
String json = readAssetFile("syntaxes/java.json");
try {
    engine.compileSyntaxFromJson(json);
} catch (SyntaxCompileError e) {
    Log.e("SweetLine", "编译失败: " + e.getMessage());
}

// 方式一: 全量分析
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

// 方式二: 增量分析
Document doc = new Document("file:///main.java", sourceCode);
DocumentAnalyzer docAnalyzer = engine.loadDocument(doc);
DocumentHighlight highlight = docAnalyzer.analyze();

// 编辑发生时增量更新
TextRange changeRange = new TextRange(
    new TextPosition(2, 4, 0),
    new TextPosition(2, 8, 0)
);
DocumentHighlight newHighlight = docAnalyzer.analyzeIncremental(changeRange, "modified");

// 方式三: 直接转换为 Spannable (可直接用于 TextView)
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

## 附录：平台构建

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

### 编译选项

| CMake 选项 | 默认值 | 说明 |
|-----------|--------|------|
| `BUILD_TESTING` | OFF | 是否构建测试 |
| `STATIC_LIB` | ON | 是否构建静态库 |
| `SHARED_LIB` | OFF | 是否构建动态库 |
