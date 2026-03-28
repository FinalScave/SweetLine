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
  static const char* kKotlinSyntaxPath = SYNTAX_DIR"/kotlin.json";
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

  int32_t styleAtColumn(const LineHighlight& line, size_t column) {
    for (const TokenSpan& span : line.spans) {
      if (column >= span.range.start.column && column < span.range.end.column) {
        return span.style_id;
      }
    }
    return -1;
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

TEST_CASE("Kotlin declaration keywords stay keyword style") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kKotlinSyntaxPath));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("kotlin");
  REQUIRE(analyzer != nullptr);

  const U8String code = "enum class Color\n"
                        "abstract class Type<T>\n"
                        "interface Service\n"
                        "abstract interface class Weird<T>\n";
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(code);
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 4);

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;

  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);   // enum
  CHECK(styleAtColumn(highlight->lines[0], 5) == kKeyword);   // class
  CHECK(styleAtColumn(highlight->lines[0], 11) == kClass);    // Color

  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);   // abstract
  CHECK(styleAtColumn(highlight->lines[1], 9) == kKeyword);   // class
  CHECK(styleAtColumn(highlight->lines[1], 15) == kClass);    // Type

  CHECK(styleAtColumn(highlight->lines[2], 0) == kKeyword);   // interface
  CHECK(styleAtColumn(highlight->lines[2], 10) == kClass);    // Service

  CHECK(styleAtColumn(highlight->lines[3], 0) == kKeyword);   // abstract
  CHECK(styleAtColumn(highlight->lines[3], 9) == kKeyword);   // interface
  CHECK(styleAtColumn(highlight->lines[3], 19) == kKeyword);  // class
  CHECK(styleAtColumn(highlight->lines[3], 25) == kClass);    // Weird
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

TEST_CASE("Analyze incremental in visible line range") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<Document> document2 = makeSharedPtr<Document>("Main2.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer2 = engine->loadDocument(document2);
  SharedPtr<Document> document3 = makeSharedPtr<Document>("Main3.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer3 = engine->loadDocument(document3);
  REQUIRE(analyzer != nullptr);
  REQUIRE(analyzer2 != nullptr);
  REQUIRE(analyzer3 != nullptr);
  SharedPtr<DocumentHighlight> initial = analyzer->analyze();
  SharedPtr<DocumentHighlight> initial2 = analyzer2->analyze();
  REQUIRE(initial != nullptr);
  REQUIRE(initial2 != nullptr);

  LineRange visible_range = {1, 6};
  SharedPtr<DocumentHighlightSlice> initial_slice = analyzer->getHighlightSlice(visible_range);
  REQUIRE(initial_slice != nullptr);
  REQUIRE(initial_slice->start_line == visible_range.start_line);
  REQUIRE(initial_slice->total_line_count == analyzer->getDocument()->getLineCount());
  REQUIRE(initial_slice->lines.size() == visible_range.line_count);
  for (size_t i = 0; i < initial_slice->lines.size(); ++i) {
    CHECK(initial_slice->lines[i] == initial->lines[initial_slice->start_line + i]);
  }

  SharedPtr<DocumentHighlightSlice> cold_slice = analyzer3->getHighlightSlice(visible_range);
  REQUIRE(cold_slice != nullptr);
  CHECK(cold_slice->start_line == visible_range.start_line);
  CHECK(cold_slice->total_line_count == analyzer3->getDocument()->getLineCount());
  CHECK(cold_slice->lines.empty());

  TextRange range = {{0, 0}, {0, 0}};
  SharedPtr<DocumentHighlightSlice> slice = analyzer->analyzeIncrementalInLineRange(range, "// inserted\n", visible_range);
  SharedPtr<DocumentHighlight> full = analyzer2->analyzeIncremental(range, "// inserted\n");
  REQUIRE(slice != nullptr);
  REQUIRE(full != nullptr);
  REQUIRE(slice->start_line == visible_range.start_line);
  REQUIRE(slice->total_line_count == analyzer->getDocument()->getLineCount());
  REQUIRE(slice->lines.size() == visible_range.line_count);
  for (size_t i = 0; i < slice->lines.size(); ++i) {
    CHECK(slice->lines[i] == full->lines[slice->start_line + i]);
  }

  SharedPtr<DocumentHighlightSlice> cached_updated_slice = analyzer->getHighlightSlice(visible_range);
  REQUIRE(cached_updated_slice != nullptr);
  REQUIRE(cached_updated_slice->start_line == slice->start_line);
  REQUIRE(cached_updated_slice->total_line_count == slice->total_line_count);
  REQUIRE(cached_updated_slice->lines.size() == slice->lines.size());
  for (size_t i = 0; i < cached_updated_slice->lines.size(); ++i) {
    CHECK(cached_updated_slice->lines[i] == slice->lines[i]);
  }

  LineRange out_of_bound_range = {full->lines.size() + 10, 5};
  SharedPtr<DocumentHighlightSlice> empty_slice = analyzer->analyzeIncrementalInLineRange(range, "", out_of_bound_range);
  REQUIRE(empty_slice != nullptr);
  CHECK(empty_slice->start_line == empty_slice->total_line_count);
  CHECK(empty_slice->lines.empty());
}
