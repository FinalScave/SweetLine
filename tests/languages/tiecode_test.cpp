#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "sweetline/util.h"
#include "../test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode.json";
  static const char* kTiecodeExampleFilePath = TESTS_DIR"/files/example.t";

  size_t utf8ColumnOf(const U8String& line, const U8String& needle) {
    size_t byte_index = line.find(needle);
    REQUIRE(byte_index != U8String::npos);

    size_t column = 0;
    for (size_t i = 0; i < byte_index; ++column) {
      unsigned char ch = static_cast<unsigned char>(line[i]);
      if ((ch & 0x80) == 0) {
        i += 1;
      } else if ((ch & 0xE0) == 0xC0) {
        i += 2;
      } else if ((ch & 0xF0) == 0xE0) {
        i += 3;
      } else if ((ch & 0xF8) == 0xF0) {
        i += 4;
      } else {
        i += 1;
      }
    }
    return column;
  }

  int32_t styleAtText(const LineHighlight& highlight_line, const U8String& source_line, const U8String& needle) {
    return styleAtColumn(highlight_line, utf8ColumnOf(source_line, needle));
  }
}

TEST_CASE("Highlight example.t") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTiecodeSyntaxPath));
  U8String code_txt = FileUtil::readString(kTiecodeExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("example.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == document->getLineCount());
  REQUIRE(highlight->spanCount() > 0);
}

TEST_CASE("Tiecode highlights operators, generic types, legacy separators, annotation long strings, properties, events, and embedded refs") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTiecodeSyntaxPath));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("tiecode");
  REQUIRE(analyzer != nullptr);

  const U8String lines[] = {
    "变量 数据: 集合<哈希表<文本, 整数[]>>",
    "常量 超时 为 整数 = 0L",
    "方法 ==(另一个: 启动窗口<文本, 集合<整数>>): 逻辑型",
    "方法 []=(索引 为 整数, 值: 文本)",
    "属性写 标题(值: 文本)",
    "方法 旧参数(路径 为 文本)",
    "方法 旧返回() 为 对象",
    "结束 属性",
    "定义事件 数据已加载(来源: 对象, 列表: 集合<文本>): 逻辑型",
    "事件 根布局.显示按钮:被单击()",
    "code #this #标题 #cls<启动窗口> #ncls<窗口> #mem<根布局.取根布局()> #sys<line>",
    "@组件配置({提示=[[第一行 \"双引号\"",
    "第二行]]})",
    "返回 本对象",
    "本对象.标题 = 可空标题",
    "父对象.刷新()"
  };

  U8String code;
  for (const U8String& line : lines) {
    code += line;
    code += "\n";
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(code);
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 16);

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kProperty = 13;

  CHECK(styleAtText(highlight->lines[0], lines[0], "变量") == kKeyword);
  CHECK(styleAtText(highlight->lines[0], lines[0], "数据") == kVariable);
  CHECK(styleAtText(highlight->lines[0], lines[0], "集合") == kClass);
  CHECK(styleAtText(highlight->lines[0], lines[0], "<") == kPunctuation);
  CHECK(styleAtText(highlight->lines[0], lines[0], "哈希表") == kClass);
  CHECK(styleAtText(highlight->lines[0], lines[0], "整数") == kClass);
  CHECK(styleAtText(highlight->lines[0], lines[0], "[") == kPunctuation);

  CHECK(styleAtText(highlight->lines[1], lines[1], "常量") == kKeyword);
  CHECK(styleAtText(highlight->lines[1], lines[1], "超时") == kVariable);
  CHECK(styleAtText(highlight->lines[1], lines[1], "为") == kKeyword);
  CHECK(styleAtText(highlight->lines[1], lines[1], "整数") == kClass);
  CHECK(styleAtText(highlight->lines[1], lines[1], "0L") == kNumber);

  CHECK(styleAtText(highlight->lines[2], lines[2], "方法") == kKeyword);
  CHECK(styleAtText(highlight->lines[2], lines[2], "==") == kMethod);
  CHECK(styleAtText(highlight->lines[2], lines[2], "另一个") == kVariable);
  CHECK(styleAtText(highlight->lines[2], lines[2], "启动窗口") == kClass);
  CHECK(styleAtText(highlight->lines[2], lines[2], "逻辑型") == kClass);

  CHECK(styleAtText(highlight->lines[3], lines[3], "[]=") == kMethod);
  CHECK(styleAtText(highlight->lines[3], lines[3], "索引") == kVariable);
  CHECK(styleAtText(highlight->lines[3], lines[3], "为") == kKeyword);
  CHECK(styleAtText(highlight->lines[3], lines[3], "整数") == kClass);

  CHECK(styleAtText(highlight->lines[4], lines[4], "属性写") == kKeyword);
  CHECK(styleAtText(highlight->lines[4], lines[4], "标题") == kProperty);

  CHECK(styleAtText(highlight->lines[5], lines[5], "旧参数") == kMethod);
  CHECK(styleAtText(highlight->lines[5], lines[5], "路径") == kVariable);
  CHECK(styleAtText(highlight->lines[5], lines[5], "为") == kKeyword);
  CHECK(styleAtText(highlight->lines[5], lines[5], "文本") == kClass);

  CHECK(styleAtText(highlight->lines[6], lines[6], "旧返回") == kMethod);
  CHECK(styleAtText(highlight->lines[6], lines[6], "为") == kKeyword);
  CHECK(styleAtText(highlight->lines[6], lines[6], "对象") == kClass);

  CHECK(styleAtText(highlight->lines[7], lines[7], "结束") == kKeyword);
  CHECK(styleAtText(highlight->lines[7], lines[7], "属性") == kKeyword);

  CHECK(styleAtText(highlight->lines[8], lines[8], "定义事件") == kKeyword);
  CHECK(styleAtText(highlight->lines[8], lines[8], "数据已加载") == kMethod);
  CHECK(styleAtText(highlight->lines[8], lines[8], "来源") == kVariable);
  CHECK(styleAtText(highlight->lines[8], lines[8], "对象") == kClass);

  CHECK(styleAtText(highlight->lines[9], lines[9], "事件") == kKeyword);
  CHECK(styleAtText(highlight->lines[9], lines[9], "根布局") == kVariable);
  CHECK(styleAtText(highlight->lines[9], lines[9], "被单击") == kMethod);

  CHECK(styleAtText(highlight->lines[10], lines[10], "code") == kKeyword);
  CHECK(styleAtText(highlight->lines[10], lines[10], "#this") == kPunctuation);
  CHECK(styleAtText(highlight->lines[10], lines[10], "this") == kKeyword);
  CHECK(styleAtText(highlight->lines[10], lines[10], "标题") == kVariable);
  CHECK(styleAtText(highlight->lines[10], lines[10], "cls") == kKeyword);
  CHECK(styleAtText(highlight->lines[10], lines[10], "启动窗口") == kClass);
  CHECK(styleAtText(highlight->lines[10], lines[10], "ncls") == kKeyword);
  CHECK(styleAtText(highlight->lines[10], lines[10], "mem") == kKeyword);
  CHECK(styleAtText(highlight->lines[10], lines[10], "取根布局") == kMethod);
  CHECK(styleAtText(highlight->lines[10], lines[10], "sys") == kKeyword);

  CHECK(styleAtText(highlight->lines[11], lines[11], "@") == kPunctuation);
  CHECK(styleAtText(highlight->lines[11], lines[11], "组件配置") == kAnnotation);
  CHECK(styleAtText(highlight->lines[11], lines[11], "[[") == kString);
  CHECK(styleAtText(highlight->lines[11], lines[11], "\"") == kString);
  CHECK(styleAtText(highlight->lines[12], lines[12], "第二行") == kString);
  CHECK(styleAtText(highlight->lines[12], lines[12], "]]") == kString);

  CHECK(styleAtText(highlight->lines[13], lines[13], "返回") == kKeyword);
  CHECK(styleAtText(highlight->lines[13], lines[13], "本对象") == kKeyword);

  CHECK(styleAtText(highlight->lines[14], lines[14], "本对象") == kKeyword);
  CHECK(styleAtText(highlight->lines[14], lines[14], ".") == kPunctuation);
  CHECK(styleAtText(highlight->lines[14], lines[14], "标题") == kProperty);

  CHECK(styleAtText(highlight->lines[15], lines[15], "父对象") == kKeyword);
  CHECK(styleAtText(highlight->lines[15], lines[15], ".") == kPunctuation);
  CHECK(styleAtText(highlight->lines[15], lines[15], "刷新") == kMethod);
}

TEST_CASE("Highlight example.t Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kTiecodeSyntaxPath);
  U8String code_txt = FileUtil::readString(kTiecodeExampleFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("example.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight example.t Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  };
}

TEST_CASE("Tiecode indentation fallback anchors event and method blocks") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTiecodeSyntaxPath));

  SharedPtr<Document> document = makeSharedPtr<Document>("example.t",
    "类 启动窗口:窗口\n"
    "    事件 启动窗口:创建完毕()\n"
    "        变量 网站2:文本 = \"http://***.**\"\n"
    "        提示网址1()\n"
    "    结束 事件\n"
    "结束 类");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* class_guide = findGuideByColumn(*result, 0);
  const IndentGuideLine* event_guide = findGuideByColumn(*result, 4);
  REQUIRE(class_guide != nullptr);
  REQUIRE(event_guide != nullptr);
  CHECK(class_guide->start_line == 0);
  CHECK(class_guide->end_line == 5);
  CHECK(class_guide->start_line + 1 == 1);
  CHECK(class_guide->end_line - 1 == 4);
  CHECK(event_guide->start_line == 1);
  CHECK(event_guide->end_line == 4);
  CHECK(event_guide->start_line + 1 == 2);
  CHECK(event_guide->end_line - 1 == 3);
}
