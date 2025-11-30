简体中文 | [English](./README_en.md)
# SweetLine 高亮引擎

## 概述
SweetLine 是一款跨平台、高性能、可扩展的语法高亮引擎，专为现代代码编辑器设计。其采用先进的正则表达式匹配技术和增量更新算法，能够实时处理大型代码文件并提供精准的语法高亮

## 核心特性
### 🚀 高性能
- 基于 Oniguruma 正则引擎，提供快速的pattern匹配 
- 增量更新算法，只重新分析变更部分 
- 多行状态保持，避免全文档重新分析
### 🎯 高精度
- 支持复杂的语法规则嵌套
- 多自动状态机支持（如字符串、注释等不同状态上下文）
- 多捕获组样式映射
### 🔧 高度可扩展
- 支持使用JSON配置语法规则
- 支持变量替换和pattern复用
### 📦 现代化设计
- C++17 标准，类型安全
- 清晰的 API 设计

## 快速开始
### 基础使用
```c++
#include "highlight.h"

using namespace NS_SWEETLINE;

// 创建高亮引擎
Ptr<HighlightEngine> engine = MAKE_PTR<HighlightEngine>();
// 编译语法规则
Ptr<SyntaxRule> syntax_rule = engine->compileSyntaxFromFile("java_syntax.json");
// 创建文档对象
Ptr<Document> document = std::make_shared<Document>("file:///example.java", R"(
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
)");
// 加载文档对象并进行分析
Ptr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
Ptr<DocumentHighlight> highlight = analyzer->analyze();
```

### 自定义语法规则
以下是示例，可参考[语法配置规范](docs/syntax_rule.md)

```json
{
  "name": "Java",
  "file_extensions": [
    ".java"
  ],
  "states": {
    "default": [
      {
        "pattern": "\\b(public|private|class|static)\\b",
        "styles": [
          1,
          "keyword"
        ]
      },
      {
        "pattern": "\"",
        "styles": "string",
        "state": "quotedString"
      }
    ],
    "quotedString": [
      {
        "pattern": "\"",
        "style": "string",
        "state": "default"
      },
      {
        "pattern": "[^\"]*",
        "style": "string"
      }
    ]
  }
}
```

### 增量更新
```c++
TextRange change_range { {2, 4}, {2, 8} };
String new_text = "modified";
// 只重新分析变更部分
Ptr<DocumentHighlight> new_highlight = analyzer->analyzeChanges(change_range, new_text);
```

### 高亮样式管理
```c++
// 注册自定义样式
engine->registerStyleName("keyword", 1);
engine->registerStyleName("number", 2);
engine->registerStyleName("string", 3);
// 获取样式名称
const String& style_name = engine->getStyleName(1); // 返回 "keyword"
```

## 高级功能
### 多语言支持
```c++
// 编译多种语法规则
Ptr<SyntaxRule> java_rule = engine->compileSyntaxFromFile("java.json");
Ptr<SyntaxRule> cpp_rule = engine->compileSyntaxFromFile("cpp.json");
Ptr<SyntaxRule> python_rule = engine->compileSyntaxFromFile("python.json");
// 根据文件扩展名获取语法规则
Ptr<Document> document = MAKE_PTR<Document>("file:///example.py", "print('Hello')");
Ptr<SyntaxRule> syntax = engine->getSyntaxRuleByExtension(".py");
// 后续会支持语法规则中引用已编译语法规则
// TODO: 在语法规则配置文件中引用已编译语法规则
```
### 性能建议
- 预编译语法规则：在应用启动时编译所有需要的语法规则
- 合理使用增量更新：对于大型文件，优先使用增量更新而非全量分析
- 优化正则表达式：避免过于复杂的pattern，使用变量复用常见pattern
- 批量更新：对于连续的小变更，可以合并为一次增量更新

## 原生平台集成
### Android集成
Android提供了便捷的JNI绑定，类名和函数名与C++侧保持一致，可直接源码依赖，也可以从maven引入：
```groovy
implementation 'com.qiplat:sweetline:0.0.2'
```

## 共建代码仓库
欢迎各位小伙伴一起共建该高亮引擎，如果您有参与项目的想法可直接拉分支修改并提交合并请求，项目协作说明可参见[项目协作说明](docs/join.md)