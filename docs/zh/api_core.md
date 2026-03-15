# SweetLine Core API 文档

本文档包含核心概念与 C++ API。

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

    // Tab 宽度, 用于缩进划线的缩进等级计算 (1 tab = tab_size 个空格)
    int32_t tab_size {4};

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

    // 缩进划线分析
    // 优先使用传入的高亮结果，其次复用 analyzeText 最近一次缓存
    // 如果没有可用高亮结果，则回退到纯缩进分析
    SharedPtr<IndentGuideResult> analyzeIndentGuides(
        const U8String& text, const SharedPtr<DocumentHighlight>& highlight = nullptr);

    // 获取当前高亮配置
    const HighlightConfig& getHighlightConfig() const;
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

#### 缩进划线分析

```cpp
auto analyzer = engine->createAnalyzerByName("python");
auto highlight = analyzer->analyzeText(source_code);

// 显式传入高亮结果
auto guides1 = analyzer->analyzeIndentGuides(source_code, highlight);

// 省略 highlight 时，自动复用最近一次 analyzeText 的缓存
auto guides2 = analyzer->analyzeIndentGuides(source_code);

// 如果没有可用缓存，则自动回退到按缩进分析
auto fresh_analyzer = engine->createAnalyzerByName("python");
auto guides3 = fresh_analyzer->analyzeIndentGuides(source_code);
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

    // 增量分析并返回指定可见行区域高亮切片
    SharedPtr<DocumentHighlightSlice> analyzeIncrementalInLineRange(
        const TextRange& range, const U8String& new_text, const LineRange& visible_range) const;

    // 增量分析 (通过字符索引)
    SharedPtr<DocumentHighlight> analyzeIncremental(
        size_t start_index, size_t end_index, const U8String& new_text) const;

    // 获取托管文档
    SharedPtr<Document> getDocument() const;

    // 获取当前高亮配置
    const HighlightConfig& getHighlightConfig() const;

    // 缩进划线分析 (需先调用 analyze 或 analyzeIncremental)
    SharedPtr<IndentGuideResult> analyzeIndentGuides() const;
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

// 仅返回可见行范围 [100, 100 + 60) 的高亮切片
LineRange visible {100, 60};
auto slice = analyzer->analyzeIncrementalInLineRange(range, "modified", visible);

// 生成缩进划线
auto guides = analyzer->analyzeIndentGuides();
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

// 行范围
struct LineRange {
    size_t start_line {0};
    size_t line_count {0};
};

// 指定行区域高亮切片
struct DocumentHighlightSlice {
    size_t start_line {0};
    size_t total_line_count {0};
    List<LineHighlight> lines;
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

