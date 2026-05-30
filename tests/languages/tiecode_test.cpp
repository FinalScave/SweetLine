#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "sweetline/util.h"
#include "../test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode.json";
  static const char* kTiecodeExampleFilePath = TESTS_DIR"/files/example.t";
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
