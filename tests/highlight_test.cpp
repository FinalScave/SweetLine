#include <iostream>
#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

static const char* kMultiGroupSyntaxPath = TESTS_DIR"/multi-group/multi-group.json";
static const char* kMultiGroupTestPath = TESTS_DIR"/multi-group/multi-group.test";
static const char* kMultiLineSyntaxPath = TESTS_DIR"/multi-line/multi-line.json";
static const char* kMultiLineTestPath = TESTS_DIR"/multi-line/multi-line.test";
static const char* kJavaSyntaxPath = SYNTAX_DIR"/java-syntax.json";
static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode-syntax.json";
static const char* kJavaMainFilePath = TESTS_DIR"/files/Main.java";
static const char* kJavaTestFilePath = TESTS_DIR"/files/Test.java";
static const char* kJavaViewFilePath = TESTS_DIR"/files/View.java";
static const char* kTiecodeFilePath = TESTS_DIR"/files/结绳.t";

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

TEST_CASE("Highlight multi-group") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kMultiGroupSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kMultiGroupTestPath);
  SharedPtr<Document> document = makeSharedPtr<Document>("multi-group.test", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  highlight->dump();
}

TEST_CASE("Highlight multi-line") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kMultiLineSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kMultiLineTestPath);
  SharedPtr<Document> document = makeSharedPtr<Document>("multi-line.test", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  highlight->dump();
}

TEST_CASE("Highlight Test.java") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kJavaSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kJavaTestFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("Test.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  highlight->dump();
}

TEST_CASE("Highlight View.java") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kJavaSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kJavaViewFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("View.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  highlight->dump();
}

TEST_CASE("Highlight 结绳.t") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kTiecodeSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kTiecodeFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("结绳.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  highlight->dump();
}

TEST_CASE("Highlight Test.java Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kJavaSyntaxPath);
  U8String code_txt = FileUtil::readString(kJavaTestFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("test.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight Test.java Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  };
}

TEST_CASE("Highlight View.java Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kJavaSyntaxPath);
  U8String code_txt = FileUtil::readString(kJavaViewFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("View.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight View.java Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  };
}

TEST_CASE("Highlight 结绳.t Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kTiecodeSyntaxPath);
  U8String code_txt = FileUtil::readString(kTiecodeFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("结绳.t", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  BENCHMARK("Highlight  结绳.t Performance") {
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  };
}

TEST_CASE("Analyze Main.java Incrementally") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kJavaSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kJavaMainFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  // 第1行末尾插入'//'注释
  TextRange range = {{0, 19}, {0, 19}};
  SharedPtr<DocumentHighlight> updated_highlight = analyzer->analyzeHighlightIncremental(range, "//aaa");
  REQUIRE(updated_highlight->lines[0].spans.back().style_id == 4);
  // 第4行'/'删除，后面应该全部变为注释
  range = {{3, 6}, {3, 7}};
  updated_highlight = analyzer->analyzeHighlightIncremental(range, "");
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[9].spans.back().style_id == 4);
  // 第4行末尾插入 *AAA\n*/，第5行是注释结束，第6行正常高亮
  range = {{3, 6}, {3, 6}};
  updated_highlight = analyzer->analyzeHighlightIncremental(range, "*AAA\n*/");
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[5].spans.front().style_id == 1);
  // 第3-5行替换为*/，第3行注释结束，后续正常高亮
  range = {{2, 0}, {4, 2}};
  updated_highlight = analyzer->analyzeHighlightIncremental(range, "*/");
  REQUIRE(updated_highlight->lines[2].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[3].spans.front().style_id == 1);
}

TEST_CASE("Analyze Main.java Incrementally Benchmark") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  try {
    engine->compileSyntaxFromFile(kJavaSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kJavaMainFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeHighlight();
  BENCHMARK("Analyze Main.java Incrementally Performance") {
    // 第 1 行末尾插入 //注释
    TextRange range = {{0, 19}, {0, 19}};
    SharedPtr<DocumentHighlight> updated_highlight = analyzer->analyzeHighlightIncremental(range, "//");
    REQUIRE(updated_highlight->lines[0].spans.back().style_id == 4);
  };
}
