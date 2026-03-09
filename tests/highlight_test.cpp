#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

namespace {
  static const char* kMultiGroupSyntaxPath = TESTS_DIR"/multi-group/multi-group.json";
  static const char* kMultiGroupTestPath = TESTS_DIR"/multi-group/multi-group.test";
  static const char* kMultiLineSyntaxPath = TESTS_DIR"/multi-line/multi-line.json";
  static const char* kMultiLineTestPath = TESTS_DIR"/multi-line/multi-line.test";
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode.json";
  static const char* kJavaExampleFilePath = TESTS_DIR"/files/example.java";
  static const char* kTiecodeExampleFilePath = TESTS_DIR"/files/example.t";

  SharedPtr<HighlightEngine> makeTestHighlightEngine() {
    SharedPtr<HighlightEngine> engine = makeSharedPtr<HighlightEngine>();
    engine->registerStyleName("keyword", 1);
    engine->registerStyleName("string", 2);
    engine->registerStyleName("number", 3);
    engine->registerStyleName("comment", 4);
    engine->registerStyleName("class", 5);
    engine->registerStyleName("method", 6);
    engine->registerStyleName("variable", 7);
    engine->registerStyleName("punctuation", 8);
    engine->registerStyleName("annotation", 9);
    return engine;
  }
}

TEST_CASE("Highlight multi-group") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kMultiGroupSyntaxPath));
  U8String code_txt = FileUtil::readString(kMultiGroupTestPath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("multi-group.test", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == document->getLineCount());
  REQUIRE(highlight->spanCount() > 0);
}

TEST_CASE("Highlight multi-line") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kMultiLineSyntaxPath));
  U8String code_txt = FileUtil::readString(kMultiLineTestPath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("multi-line.test", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == document->getLineCount());
  REQUIRE(highlight->spanCount() > 0);
}

TEST_CASE("Highlight example.java") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == document->getLineCount());
  REQUIRE(highlight->spanCount() > 0);
}

TEST_CASE("Highlight example.t") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTiecodeSyntaxPath));
  U8String code_txt = FileUtil::readString(kTiecodeExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("结绳.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == document->getLineCount());
  REQUIRE(highlight->spanCount() > 0);
}

TEST_CASE("Highlight example.java Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kJavaSyntaxPath);
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight Test.java Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  };
}


TEST_CASE("Highlight example.t Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kTiecodeSyntaxPath);
  U8String code_txt = FileUtil::readString(kTiecodeExampleFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("结绳.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight  结绳.t Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  };
}

TEST_CASE("Analyze example.java Incrementally") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  // 第1行末尾插入'//'注释
  TextRange range = {{0, 19}, {0, 19}};
  SharedPtr<DocumentHighlight> updated_highlight = analyzer->analyzeIncremental(range, "//aaa");
  REQUIRE(updated_highlight->lines[0].spans.back().style_id == 4);
  // 第4行'/'删除，后面应该全部变为注释
  range = {{3, 6}, {3, 7}};
  updated_highlight = analyzer->analyzeIncremental(range, "");
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[9].spans.back().style_id == 4);
  // 第4行末尾插入 *AAA\n*/，第5行是注释结束，第6行正常高亮
  range = {{3, 6}, {3, 6}};
  updated_highlight = analyzer->analyzeIncremental(range, "*AAA\n*/");
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[5].spans.front().style_id == 1);
  // 第3-5行替换为*/，第3行注释结束，后续正常高亮
  range = {{2, 0}, {4, 2}};
  updated_highlight = analyzer->analyzeIncremental(range, "*/");
  REQUIRE(updated_highlight->lines[2].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[3].spans.front().style_id == 1);
}
