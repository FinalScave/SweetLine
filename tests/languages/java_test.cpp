#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "sweetline/util.h"
#include "../test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kJavaExampleFilePath = TESTS_DIR"/files/example.java";
}

TEST_CASE("Java sample has indent guides") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<IndentGuideResult> res = analyzer->analyzeIndentGuides();
  REQUIRE(res != nullptr);
  REQUIRE_FALSE(res->guide_lines.empty());
  REQUIRE(res->line_states.size() == document->getLineCount());
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

TEST_CASE("URL inside string and comment gets dedicated style") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("java");
  REQUIRE(analyzer != nullptr);

  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(
    "String site = \"Visit https://example.com/docs?q=1\";\n"
    "// mirror http://mirror.example.org/path");
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() == 2);

  constexpr int32_t kUrl = 16;
  CHECK(styleAtColumn(highlight->lines[0], 21) == kUrl);
  CHECK(styleAtColumn(highlight->lines[1], 10) == kUrl);
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

TEST_CASE("Visible range example.java Benchmarks") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kJavaSyntaxPath);
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  const LineRange visible_range = {0, 80};
  const TextRange insert_range = {{0, 0}, {0, 0}};
  const U8String inserted_text = "// inserted\n";

  BENCHMARK_ADVANCED("AnalyzeLineRange example.java Performance")(Catch::Benchmark::Chronometer meter) {
    meter.measure([&] {
      SharedPtr<Document> benchmark_document = makeSharedPtr<Document>("example.java", code_txt);
      SharedPtr<DocumentAnalyzer> benchmark_analyzer = engine->loadDocument(benchmark_document);
      return benchmark_analyzer->analyzeLineRange(visible_range);
    });
  };

  BENCHMARK_ADVANCED("AnalyzeIncrementalInLineRange example.java Performance")(Catch::Benchmark::Chronometer meter) {
    meter.measure([&] {
      SharedPtr<Document> benchmark_document = makeSharedPtr<Document>("example.java", code_txt);
      SharedPtr<DocumentAnalyzer> benchmark_analyzer = engine->loadDocument(benchmark_document);
      return benchmark_analyzer->analyzeIncrementalInLineRange(insert_range, inserted_text, visible_range);
    });
  };

  BENCHMARK_ADVANCED("AnalyzeIncremental example.java Performance")(Catch::Benchmark::Chronometer meter) {
    meter.measure([&] {
      SharedPtr<Document> benchmark_document = makeSharedPtr<Document>("example.java", code_txt);
      SharedPtr<DocumentAnalyzer> benchmark_analyzer = engine->loadDocument(benchmark_document);
      benchmark_analyzer->analyze();
      return benchmark_analyzer->analyzeIncremental(insert_range, inserted_text);
    });
  };
}

TEST_CASE("Analyze Java comment transitions incrementally") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  const U8String code_txt =
    "class Main {\n"
    "  int value = 1;\n"
    "  int other = 2;\n"
    "  int third = 3;\n"
    "}\n";
  SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);

  TextRange range = {{1, 16}, {1, 16}};
  SharedPtr<DocumentHighlight> updated_highlight = analyzer->analyzeIncremental(range, "//aaa");
  REQUIRE(updated_highlight->lines[1].spans.back().style_id == 4);

  range = {{2, 2}, {2, 2}};
  updated_highlight = analyzer->analyzeIncremental(range, "/*");
  REQUIRE(updated_highlight->lines[2].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[3].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);

  range = {{3, 16}, {3, 16}};
  updated_highlight = analyzer->analyzeIncremental(range, "*/");
  REQUIRE(updated_highlight->lines[3].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[4].spans.front().style_id == 8);
}

TEST_CASE("Analyze incremental by char index accepts EOF boundaries") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));

  SECTION("append at EOF on non-empty document") {
    SharedPtr<Document> document = makeSharedPtr<Document>("Main.java", "class Main {}");
    SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
    REQUIRE(analyzer != nullptr);
    REQUIRE(analyzer->analyze() != nullptr);

    const size_t original_total = analyzer->getDocument()->totalChars();
    SharedPtr<DocumentHighlight> appended = analyzer->analyzeIncremental(original_total, original_total, "\n// tail");
    REQUIRE(appended != nullptr);
    REQUIRE(analyzer->getDocument()->getText() == "class Main {}\n// tail");

    const size_t appended_total = analyzer->getDocument()->totalChars();
    SharedPtr<DocumentHighlight> trimmed = analyzer->analyzeIncremental(appended_total - 5, appended_total, "");
    REQUIRE(trimmed != nullptr);
    REQUIRE(analyzer->getDocument()->getText() == "class Main {}\n//");
  }

  SECTION("insert into empty document") {
    SharedPtr<Document> document = makeSharedPtr<Document>("Empty.java", "");
    SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
    REQUIRE(analyzer != nullptr);

    SharedPtr<DocumentHighlight> updated = analyzer->analyzeIncremental(0, 0, "class Empty {}");
    REQUIRE(updated != nullptr);
    REQUIRE(analyzer->getDocument()->getText() == "class Empty {}");
    REQUIRE(analyzer->getDocument()->getLineCount() == 1);
  }
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
  SharedPtr<Document> document4 = makeSharedPtr<Document>("Main4.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer4 = engine->loadDocument(document4);
  REQUIRE(analyzer != nullptr);
  REQUIRE(analyzer2 != nullptr);
  REQUIRE(analyzer3 != nullptr);
  REQUIRE(analyzer4 != nullptr);
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

  SharedPtr<DocumentHighlightSlice> cold_range_slice = analyzer3->analyzeLineRange(visible_range);
  REQUIRE(cold_range_slice != nullptr);
  REQUIRE(cold_range_slice->start_line == visible_range.start_line);
  REQUIRE(cold_range_slice->total_line_count == analyzer3->getDocument()->getLineCount());
  REQUIRE(cold_range_slice->lines.size() == visible_range.line_count);
  for (size_t i = 0; i < cold_range_slice->lines.size(); ++i) {
    CHECK(cold_range_slice->lines[i] == initial->lines[cold_range_slice->start_line + i]);
  }

  SharedPtr<DocumentHighlightSlice> cold_cached_slice = analyzer3->getHighlightSlice(visible_range);
  REQUIRE(cold_cached_slice != nullptr);
  REQUIRE(cold_cached_slice->start_line == cold_range_slice->start_line);
  REQUIRE(cold_cached_slice->total_line_count == cold_range_slice->total_line_count);
  REQUIRE(cold_cached_slice->lines.size() == cold_range_slice->lines.size());
  for (size_t i = 0; i < cold_cached_slice->lines.size(); ++i) {
    CHECK(cold_cached_slice->lines[i] == cold_range_slice->lines[i]);
  }

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

  SharedPtr<DocumentHighlightSlice> cold_incremental_slice =
    analyzer4->analyzeIncrementalInLineRange(range, "// inserted\n", visible_range);
  REQUIRE(cold_incremental_slice != nullptr);
  REQUIRE(cold_incremental_slice->start_line == visible_range.start_line);
  REQUIRE(cold_incremental_slice->total_line_count == analyzer4->getDocument()->getLineCount());
  REQUIRE(cold_incremental_slice->lines.size() == visible_range.line_count);
  for (size_t i = 0; i < cold_incremental_slice->lines.size(); ++i) {
    CHECK(cold_incremental_slice->lines[i] == full->lines[cold_incremental_slice->start_line + i]);
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
