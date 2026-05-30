#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "sweetline/util.h"

using namespace NS_SWEETLINE;

namespace {
  static const char* kMultiGroupSyntaxPath = TESTS_DIR"/multi-group/multi-group.json";
  static const char* kMultiGroupTestPath = TESTS_DIR"/multi-group/multi-group.test";
  static const char* kMultiLineSyntaxPath = TESTS_DIR"/multi-line/multi-line.json";
  static const char* kMultiLineTestPath = TESTS_DIR"/multi-line/multi-line.test";
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kKotlinSyntaxPath = SYNTAX_DIR"/kotlin.json";
  static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode.json";
  static const char* kCangjieExampleFilePath = TESTS_DIR"/files/example.cj";
  static const char* kAbnfSyntaxPath = SYNTAX_DIR"/abnf.json";
  static const char* kAsmAarch64SyntaxPath = SYNTAX_DIR"/asm-aarch64.json";
  static const char* kAsmAttSyntaxPath = SYNTAX_DIR"/asm-att.json";
  static const char* kAsmIntelSyntaxPath = SYNTAX_DIR"/asm-intel.json";
  static const char* kBatchSyntaxPath = SYNTAX_DIR"/batch.json";
  static const char* kBrainfuckSyntaxPath = SYNTAX_DIR"/brainfuck.json";
  static const char* kCangjieSyntaxPath = SYNTAX_DIR"/cangjie.json";
  static const char* kDtsSyntaxPath = SYNTAX_DIR"/dts.json";
  static const char* kGlslSyntaxPath = SYNTAX_DIR"/glsl.json";
  static const char* kGroovySyntaxPath = SYNTAX_DIR"/groovy.json";
  static const char* kHlslSyntaxPath = SYNTAX_DIR"/hlsl.json";
  static const char* kJavascriptSyntaxPath = SYNTAX_DIR"/javascript.json";
  static const char* kJsxSyntaxPath = SYNTAX_DIR"/jsx.json";
  static const char* kJasmSyntaxPath = SYNTAX_DIR"/jasm.json";
  static const char* kTsxSyntaxPath = SYNTAX_DIR"/tsx.json";
  static const char* kTypescriptSyntaxPath = SYNTAX_DIR"/typescript.json";
  static const char* kCssSyntaxPath = SYNTAX_DIR"/css.json";
  static const char* kScssSyntaxPath = SYNTAX_DIR"/scss.json";
  static const char* kLessSyntaxPath = SYNTAX_DIR"/less.json";
  static const char* kJsonSyntaxPath = SYNTAX_DIR"/json-sweetline.json";
  static const char* kJsoncSyntaxPath = SYNTAX_DIR"/jsonc.json";
  static const char* kJson5SyntaxPath = SYNTAX_DIR"/json5.json";
  static const char* kCmakeSyntaxPath = SYNTAX_DIR"/cmake.json";
  static const char* kDockerfileSyntaxPath = SYNTAX_DIR"/dockerfile.json";
  static const char* kMakefileSyntaxPath = SYNTAX_DIR"/makefile.json";
  static const char* kPropertiesSyntaxPath = SYNTAX_DIR"/properties.json";
  static const char* kEnvSyntaxPath = SYNTAX_DIR"/env.json";
  static const char* kProtobufSyntaxPath = SYNTAX_DIR"/protobuf.json";
  static const char* kGraphqlSyntaxPath = SYNTAX_DIR"/graphql.json";
  static const char* kNginxSyntaxPath = SYNTAX_DIR"/nginx.json";
  static const char* kNixSyntaxPath = SYNTAX_DIR"/nix.json";
  static const char* kGitignoreSyntaxPath = SYNTAX_DIR"/gitignore.json";
  static const char* kDiffSyntaxPath = SYNTAX_DIR"/diff.json";
  static const char* kRubySyntaxPath = SYNTAX_DIR"/ruby.json";
  static const char* kHclSyntaxPath = SYNTAX_DIR"/hcl.json";
  static const char* kShellSyntaxPath = SYNTAX_DIR"/shell.json";
  static const char* kSmaliSyntaxPath = SYNTAX_DIR"/smali.json";
  static const char* kSvgSyntaxPath = SYNTAX_DIR"/svg.json";
  static const char* kTerraformSyntaxPath = SYNTAX_DIR"/terraform.json";
  static const char* kVueSyntaxPath = SYNTAX_DIR"/vue.json";
  static const char* kXmlSyntaxPath = SYNTAX_DIR"/xml.json";
  static const char* kZigSyntaxPath = SYNTAX_DIR"/zig.json";
  static const char* kSvelteSyntaxPath = SYNTAX_DIR"/svelte.json";
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
    engine->registerStyleName("builtin", 10);
    engine->registerStyleName("preprocessor", 11);
    engine->registerStyleName("macro", 12);
    engine->registerStyleName("property", 13);
    engine->registerStyleName("selector", 15);
    engine->registerStyleName("url", 16);
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

  void requireNoTransparentSpans(const SharedPtr<DocumentHighlight>& highlight, const U8String& file_name) {
    REQUIRE(highlight != nullptr);
    for (size_t line_index = 0; line_index < highlight->lines.size(); ++line_index) {
      const LineHighlight& line = highlight->lines[line_index];
      for (const TokenSpan& span : line.spans) {
        CAPTURE(file_name, line_index, span.range.start.column, span.range.end.column, span.style_id);
        REQUIRE(span.style_id > 0);
      }
    }
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
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("kotlin");
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

TEST_CASE("Visible range example.java Benchmarks") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  engine->compileSyntaxFromFile(kJavaSyntaxPath);
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  const LineRange visible_range = {0, 80};
  const TextRange insert_range = {{0, 0}, {0, 0}};
  const U8String inserted_text = "// inserted\n";

  SharedPtr<Document> sample_document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> sample_analyzer = engine->loadDocument(sample_document);
  REQUIRE(sample_analyzer != nullptr);

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

  // Insert a line comment at the end of the assignment line.
  TextRange range = {{1, 16}, {1, 16}};
  SharedPtr<DocumentHighlight> updated_highlight = analyzer->analyzeIncremental(range, "//aaa");
  REQUIRE(updated_highlight->lines[1].spans.back().style_id == 4);

  // Start a block comment before the next statement and let it flow to EOF.
  range = {{2, 2}, {2, 2}};
  updated_highlight = analyzer->analyzeIncremental(range, "/*");
  REQUIRE(updated_highlight->lines[2].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[3].spans.back().style_id == 4);
  REQUIRE(updated_highlight->lines[4].spans.back().style_id == 4);

  // Close the block comment at the end of the following line and recover parsing.
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

TEST_CASE("CSS family keeps selectors, properties, variables, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCssSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kScssSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kLessSyntaxPath));

  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kSelector = 15;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> css = engine->createAnalyzerBySyntaxName("css");
  REQUIRE(css != nullptr);
  SharedPtr<DocumentHighlight> cssHighlight = css->analyzeText(
    ".card:hover {\n"
    "  content: \"https://example.com\";\n"
    "}\n");
  REQUIRE(cssHighlight != nullptr);
  CHECK(styleAtColumn(cssHighlight->lines[0], 1) == kClass);
  CHECK(styleAtColumn(cssHighlight->lines[0], 6) == kSelector);
  CHECK(styleAtColumn(cssHighlight->lines[1], 2) == kProperty);
  CHECK(styleAtColumn(cssHighlight->lines[1], 12) == kUrl);

  SharedPtr<TextAnalyzer> scss = engine->createAnalyzerBySyntaxName("scss");
  REQUIRE(scss != nullptr);
  SharedPtr<DocumentHighlight> scssHighlight = scss->analyzeText(
    "$accent-color: #fff;\n"
    "@mixin badge($radius) {\n"
    "  border-radius: $radius;\n"
    "  content: \"See #{$accent-color}\";\n"
    "}\n");
  REQUIRE(scssHighlight != nullptr);
  CHECK(styleAtColumn(scssHighlight->lines[0], 0) == kVariable);
  CHECK(styleAtColumn(scssHighlight->lines[1], 0) == 1);
  CHECK(styleAtColumn(scssHighlight->lines[1], 7) == kMethod);
  CHECK(styleAtColumn(scssHighlight->lines[2], 2) == kProperty);
  CHECK(styleAtColumn(scssHighlight->lines[2], 17) == kVariable);
  CHECK(styleAtColumn(scssHighlight->lines[3], 18) == kAnnotation);

  SharedPtr<TextAnalyzer> less = engine->createAnalyzerBySyntaxName("less");
  REQUIRE(less != nullptr);
  SharedPtr<DocumentHighlight> lessHighlight = less->analyzeText(
    "@brand-color: #409eff;\n"
    ".button(@radius) {\n"
    "  color: @brand-color;\n"
    "  content: \"https://cdn.example.com\";\n"
    "}\n");
  REQUIRE(lessHighlight != nullptr);
  CHECK(styleAtColumn(lessHighlight->lines[0], 0) == kVariable);
  CHECK(styleAtColumn(lessHighlight->lines[1], 1) == kMethod);
  CHECK(styleAtColumn(lessHighlight->lines[1], 9) == kVariable);
  CHECK(styleAtColumn(lessHighlight->lines[2], 2) == kProperty);
  CHECK(styleAtColumn(lessHighlight->lines[2], 9) == kVariable);
  CHECK(styleAtColumn(lessHighlight->lines[3], 12) == kUrl);
}

TEST_CASE("JSON, JSONC, and JSON5 keep keys, values, numbers, comments, and builtins distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJsonSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJsoncSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJson5SyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> json = engine->createAnalyzerBySyntaxName("json");
  REQUIRE(json != nullptr);
  SharedPtr<DocumentHighlight> jsonHighlight = json->analyzeText(
    "{\n"
    "  \"enabled\": true,\n"
    "  \"fallback\": null\n"
    "}\n");
  REQUIRE(jsonHighlight != nullptr);
  CHECK(styleAtColumn(jsonHighlight->lines[1], 3) == kKeyword);
  CHECK(styleAtColumn(jsonHighlight->lines[1], 13) == kKeyword);
  CHECK(styleAtColumn(jsonHighlight->lines[2], 14) == kKeyword);

  SharedPtr<TextAnalyzer> jsonc = engine->createAnalyzerBySyntaxName("jsonc");
  REQUIRE(jsonc != nullptr);
  SharedPtr<DocumentHighlight> jsoncHighlight = jsonc->analyzeText(
    "{\n"
    "  \"site\": \"https://example.com\",\n"
    "  \"enabled\": true,\n"
    "  // mirror https://mirror.example.com\n"
    "}\n");
  REQUIRE(jsoncHighlight != nullptr);
  CHECK(styleAtColumn(jsoncHighlight->lines[1], 3) == kKeyword);
  CHECK(styleAtColumn(jsoncHighlight->lines[1], 11) == kUrl);
  CHECK(styleAtColumn(jsoncHighlight->lines[2], 13) == kKeyword);
  CHECK(styleAtColumn(jsoncHighlight->lines[3], 12) == kUrl);

  SharedPtr<TextAnalyzer> json5 = engine->createAnalyzerBySyntaxName("json5");
  REQUIRE(json5 != nullptr);
  SharedPtr<DocumentHighlight> json5Highlight = json5->analyzeText(
    "{\n"
    "  theme: 'midnight',\n"
    "  special: Infinity,\n"
    "  ratio: .75\n"
    "}\n");
  REQUIRE(json5Highlight != nullptr);
  CHECK(styleAtColumn(json5Highlight->lines[1], 2) == kKeyword);
  CHECK(styleAtColumn(json5Highlight->lines[2], 11) == kBuiltin);
  CHECK(styleAtColumn(json5Highlight->lines[3], 9) == kNumber);
}

TEST_CASE("Config syntaxes keep commands, keys, variables, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCmakeSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kDockerfileSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kMakefileSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kPropertiesSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kEnvSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kUrl = 16;
  constexpr int32_t kSelector = 15;

  SharedPtr<TextAnalyzer> cmake = engine->createAnalyzerBySyntaxName("cmake");
  REQUIRE(cmake != nullptr);
  SharedPtr<DocumentHighlight> cmakeHighlight = cmake->analyzeText(
    "set(APP_DOCS_URL \"https://example.com\")\n"
    "if(DEFINED APP_DOCS_URL)\n");
  REQUIRE(cmakeHighlight != nullptr);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 0) == kMethod);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 4) == kVariable);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 18) == kUrl);
  CHECK(styleAtColumn(cmakeHighlight->lines[1], 0) == kKeyword);

  SharedPtr<TextAnalyzer> dockerfile = engine->createAnalyzerBySyntaxName("dockerfile");
  REQUIRE(dockerfile != nullptr);
  SharedPtr<DocumentHighlight> dockerHighlight = dockerfile->analyzeText(
    "FROM alpine:3.20 AS base\n"
    "ENV APP_HOME=/opt/app\n"
    "ADD https://example.com/config.json ./config.json\n");
  REQUIRE(dockerHighlight != nullptr);
  CHECK(styleAtColumn(dockerHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(dockerHighlight->lines[0], 20) == kClass);
  CHECK(styleAtColumn(dockerHighlight->lines[1], 4) == kVariable);
  CHECK(styleAtColumn(dockerHighlight->lines[2], 4) == kUrl);

  SharedPtr<TextAnalyzer> makefile = engine->createAnalyzerBySyntaxName("makefile");
  REQUIRE(makefile != nullptr);
  SharedPtr<DocumentHighlight> makeHighlight = makefile->analyzeText(
    "APP := app\n"
    "build/$(APP): src/main.cpp\n"
    "\t@echo \"https://example.com\"\n");
  REQUIRE(makeHighlight != nullptr);
  CHECK(styleAtColumn(makeHighlight->lines[0], 0) == kVariable);
  CHECK(styleAtColumn(makeHighlight->lines[1], 0) == kSelector);
  CHECK(styleAtColumn(makeHighlight->lines[2], 2) == kMethod);
  CHECK(styleAtColumn(makeHighlight->lines[2], 8) == kUrl);

  SharedPtr<TextAnalyzer> properties = engine->createAnalyzerBySyntaxName("properties");
  REQUIRE(properties != nullptr);
  SharedPtr<DocumentHighlight> propertiesHighlight = properties->analyzeText(
    "app.docs=https://example.com\n"
    "app.message=Welcome ${app.docs}\n");
  REQUIRE(propertiesHighlight != nullptr);
  CHECK(styleAtColumn(propertiesHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(propertiesHighlight->lines[0], 9) == kUrl);
  CHECK(styleAtColumn(propertiesHighlight->lines[1], 22) == kVariable);

  SharedPtr<TextAnalyzer> env = engine->createAnalyzerBySyntaxName("env");
  REQUIRE(env != nullptr);
  SharedPtr<DocumentHighlight> envHighlight = env->analyzeText(
    "export API_URL=\"https://api.example.com\"\n"
    "DEBUG=true # https://example.com/debug\n");
  REQUIRE(envHighlight != nullptr);
  CHECK(styleAtColumn(envHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(envHighlight->lines[0], 7) == kVariable);
  CHECK(styleAtColumn(envHighlight->lines[0], 16) == kUrl);
  CHECK(styleAtColumn(envHighlight->lines[1], 6) == kBuiltin);
  CHECK(styleAtColumn(envHighlight->lines[1], 13) == kUrl);
}

TEST_CASE("Protobuf keeps options, field metadata, rpc signatures, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kProtobufSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> protobuf = engine->createAnalyzerBySyntaxName("protobuf");
  REQUIRE(protobuf != nullptr);
  SharedPtr<DocumentHighlight> highlight = protobuf->analyzeText(
    "option go_package = \"https://example.com/proto\";\n"
    "message User {\n"
    "  string name = 1 [deprecated = true];\n"
    "}\n"
    "service UserService {\n"
    "  rpc GetUser (GetUserRequest) returns (stream User);\n"
    "}\n"
    "// docs https://example.com/proto\n");
  REQUIRE(highlight != nullptr);

  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[0], 7) == kProperty);
  CHECK(styleAtColumn(highlight->lines[0], 21) == kUrl);
  CHECK(styleAtColumn(highlight->lines[2], 9) == kVariable);
  CHECK(styleAtColumn(highlight->lines[2], 19) == kProperty);
  CHECK(styleAtColumn(highlight->lines[2], 32) == kBuiltin);
  CHECK(styleAtColumn(highlight->lines[5], 6) == kMethod);
  CHECK(styleAtColumn(highlight->lines[5], 47) == kClass);
  CHECK(styleAtColumn(highlight->lines[7], 8) == kUrl);
}

TEST_CASE("GraphQL keeps operation metadata, directives, variables, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kGraphqlSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> graphql = engine->createAnalyzerBySyntaxName("graphql");
  REQUIRE(graphql != nullptr);
  SharedPtr<DocumentHighlight> highlight = graphql->analyzeText(
    "\"\"\"Docs https://example.com/graphql\"\"\"\n"
    "query GetUser($id: ID!) {\n"
    "  user(id: $id) @include(if: true)\n"
    "}\n");
  REQUIRE(highlight != nullptr);

  CHECK(styleAtColumn(highlight->lines[0], 8) == kUrl);
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 6) == kClass);
  CHECK(styleAtColumn(highlight->lines[1], 15) == kVariable);
  CHECK(styleAtColumn(highlight->lines[2], 2) == kMethod);
  CHECK(styleAtColumn(highlight->lines[2], 12) == kVariable);
  CHECK(styleAtColumn(highlight->lines[2], 17) == kAnnotation);
  CHECK(styleAtColumn(highlight->lines[2], 29) == kBuiltin);
}

TEST_CASE("Infra syntaxes keep block heads, properties, traversals, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kHclSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTerraformSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> hcl = engine->createAnalyzerBySyntaxName("hcl");
  REQUIRE(hcl != nullptr);
  SharedPtr<DocumentHighlight> hclHighlight = hcl->analyzeText(
    "service \"api\" \"primary\" {\n"
    "  endpoint = \"https://api.example.com\"\n"
    "  command  = format(\"%s-%s\", local.service_name, \"v1\")\n"
    "}\n");
  REQUIRE(hclHighlight != nullptr);
  CHECK(styleAtColumn(hclHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(hclHighlight->lines[0], 9) == kClass);
  CHECK(styleAtColumn(hclHighlight->lines[1], 2) == kProperty);
  CHECK(styleAtColumn(hclHighlight->lines[1], 14) == kUrl);
  CHECK(styleAtColumn(hclHighlight->lines[2], 13) == kBuiltin);

  SharedPtr<TextAnalyzer> terraform = engine->createAnalyzerBySyntaxName("terraform");
  REQUIRE(terraform != nullptr);
  SharedPtr<DocumentHighlight> terraformHighlight = terraform->analyzeText(
    "resource \"aws_s3_bucket\" \"logs\" {\n"
    "  bucket = local.name\n"
    "}\n");
  REQUIRE(terraformHighlight != nullptr);
  CHECK(styleAtColumn(terraformHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(terraformHighlight->lines[0], 10) == kClass);
  CHECK(styleAtColumn(terraformHighlight->lines[1], 2) == kProperty);
}

TEST_CASE("Markup component syntaxes keep directives, bindings, scripts, styles, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavascriptSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJsxSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTypescriptSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTsxSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCssSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kScssSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kLessSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kVueSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kSvelteSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> vue = engine->createAnalyzerBySyntaxName("vue");
  REQUIRE(vue != nullptr);
  SharedPtr<DocumentHighlight> vueHighlight = vue->analyzeText(
    "<DemoCard :href=\"profileUrl\" title=\"https://example.com\">\n"
    "  {{ total }}\n"
    "</DemoCard>\n"
    "<script setup lang=\"ts\">\n"
    "const total = 1;\n"
    "</script>\n"
    "<style scoped lang=\"scss\">\n"
    ".card { content: \"https://example.com\"; }\n"
    "</style>\n");
  REQUIRE(vueHighlight != nullptr);
  CHECK(styleAtColumn(vueHighlight->lines[0], 1) == kClass);
  CHECK(styleAtColumn(vueHighlight->lines[0], 10) == kAnnotation);
  CHECK(styleAtColumn(vueHighlight->lines[0], 29) == kProperty);
  CHECK(styleAtColumn(vueHighlight->lines[0], 36) == kUrl);
  CHECK(styleAtColumn(vueHighlight->lines[1], 5) == kAnnotation);
  CHECK(styleAtColumn(vueHighlight->lines[4], 0) == kKeyword);
  CHECK(styleAtColumn(vueHighlight->lines[7], 8) == kProperty);
  CHECK(styleAtColumn(vueHighlight->lines[7], 18) == kUrl);

  SharedPtr<TextAnalyzer> svelte = engine->createAnalyzerBySyntaxName("svelte");
  REQUIRE(svelte != nullptr);
  SharedPtr<DocumentHighlight> svelteHighlight = svelte->analyzeText(
    "<!-- docs https://example.com -->\n"
    "<button on:click={increment} title=\"https://example.com\" class:active={count > 0}>\n"
    "  Count {count}\n"
    "</button>\n"
    "<script lang=\"ts\">\n"
    "const count = 1;\n"
    "</script>\n");
  REQUIRE(svelteHighlight != nullptr);
  CHECK(styleAtColumn(svelteHighlight->lines[0], 10) == kUrl);
  CHECK(styleAtColumn(svelteHighlight->lines[1], 8) == kAnnotation);
  CHECK(styleAtColumn(svelteHighlight->lines[1], 29) == kProperty);
  CHECK(styleAtColumn(svelteHighlight->lines[1], 36) == kUrl);
  CHECK(styleAtColumn(svelteHighlight->lines[1], 57) == kAnnotation);
  CHECK(styleAtColumn(svelteHighlight->lines[2], 10) == kAnnotation);
  CHECK(styleAtColumn(svelteHighlight->lines[5], 0) == kKeyword);
}

TEST_CASE("Host syntaxes keep every embedded importSyntax state active") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavascriptSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTypescriptSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(R"JSON(
{
  "name": "embedded-host",
  "fileSuffixes": [".embed"],
  "states": {
    "default": [
      {
        "pattern": "^(```)(js)$",
        "styles": [1, "punctuation", 2, "annotation"],
        "state": "jsBlock"
      },
      {
        "pattern": "^(```)(ts)$",
        "styles": [1, "punctuation", 2, "annotation"],
        "state": "tsBlock"
      }
    ],
    "jsBlock": [
      {
        "pattern": "^(```)$",
        "styles": [1, "punctuation"],
        "state": "default"
      },
      {
        "importSyntax": "javascript"
      }
    ],
    "tsBlock": [
      {
        "pattern": "^(```)$",
        "styles": [1, "punctuation"],
        "state": "default"
      },
      {
        "importSyntax": "typescript"
      }
    ]
  }
}
)JSON"));

  constexpr int32_t kKeyword = 1;

  SharedPtr<TextAnalyzer> host = engine->createAnalyzerBySyntaxName("embedded-host");
  REQUIRE(host != nullptr);
  SharedPtr<DocumentHighlight> highlight = host->analyzeText(
    "```js\n"
    "const first = 1;\n"
    "```\n"
    "```ts\n"
    "const total: number = 1;\n"
    "```\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[4], 0) == kKeyword);
}

TEST_CASE("Ops syntaxes keep directives, globs, patch metadata, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kNginxSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kGitignoreSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kDiffSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kComment = 4;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kSelector = 15;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> nginx = engine->createAnalyzerBySyntaxName("nginx");
  REQUIRE(nginx != nullptr);
  SharedPtr<DocumentHighlight> nginxHighlight = nginx->analyzeText(
    "location ~* ^/assets/ {\n"
    "  proxy_pass https://api.example.com;\n"
    "}\n");
  REQUIRE(nginxHighlight != nullptr);
  CHECK(styleAtColumn(nginxHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(nginxHighlight->lines[0], 9) == kAnnotation);
  CHECK(styleAtColumn(nginxHighlight->lines[1], 2) == kKeyword);
  CHECK(styleAtColumn(nginxHighlight->lines[1], 13) == kUrl);

  SharedPtr<TextAnalyzer> gitignore = engine->createAnalyzerBySyntaxName("gitignore");
  REQUIRE(gitignore != nullptr);
  SharedPtr<DocumentHighlight> gitignoreHighlight = gitignore->analyzeText(
    "!/dist/keep.txt\n"
    "**/node_modules/\n");
  REQUIRE(gitignoreHighlight != nullptr);
  CHECK(styleAtColumn(gitignoreHighlight->lines[0], 0) == kAnnotation);
  CHECK(styleAtColumn(gitignoreHighlight->lines[0], 2) == kSelector);
  CHECK(styleAtColumn(gitignoreHighlight->lines[1], 0) == kBuiltin);
  CHECK(styleAtColumn(gitignoreHighlight->lines[1], 3) == kSelector);

  SharedPtr<TextAnalyzer> diff = engine->createAnalyzerBySyntaxName("diff");
  REQUIRE(diff != nullptr);
  SharedPtr<DocumentHighlight> diffHighlight = diff->analyzeText(
    "@@ -1,1 +1,2 @@ title\n"
    "+https://example.com\n"
    "-removed\n");
  REQUIRE(diffHighlight != nullptr);
  CHECK(styleAtColumn(diffHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(diffHighlight->lines[1], 1) == kUrl);
  CHECK(styleAtColumn(diffHighlight->lines[2], 1) == kComment);
}

TEST_CASE("Ruby keeps operator defs, regexes, heredocs, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kRubySyntaxPath));

  constexpr int32_t kMethod = 6;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> ruby = engine->createAnalyzerBySyntaxName("ruby");
  REQUIRE(ruby != nullptr);
  SharedPtr<DocumentHighlight> highlight = ruby->analyzeText(
    "class Demo\n"
    "  def []=(key, value)\n"
    "  end\n"
    "\n"
    "  def call\n"
    "    %q{https://example.com}\n"
    "    %r{https://example.com/items/\\\\d+}i\n"
    "    note = <<~TEXT\n"
    "https://example.com/docs\n"
    "TEXT\n"
    "  end\n"
    "end\n");
  REQUIRE(highlight != nullptr);

  CHECK(styleAtColumn(highlight->lines[1], 6) == kMethod);
  CHECK(styleAtColumn(highlight->lines[5], 4) == 2);
  CHECK(styleAtColumn(highlight->lines[6], 4) == kAnnotation);
  CHECK(styleAtColumn(highlight->lines[8], 0) == kUrl);
}

TEST_CASE("F# keeps keywords, types, let bindings, attributes, and preprocessor distinct") {
  static const char* kFsharpSyntaxPath = SYNTAX_DIR"/fsharp.json";
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kFsharpSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kComment = 4;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kPreprocessor = 11;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> fsharp = engine->createAnalyzerBySyntaxName("fsharp");
  REQUIRE(fsharp != nullptr);

  SharedPtr<DocumentHighlight> highlight = fsharp->analyzeText(
    "namespace MyApp\n"
    "open System.Collections\n"
    "#if DEBUG\n"
    "[<Obsolete(\"use v2\")>]\n"
    "type Color = Red | Green | Blue\n"
    "let rec fib n = if n <= 1 then n else fib (n-1) + fib (n-2)\n"
    "let x = 42\n"
    "let msg = \"hello\"\n"
    "let flag = true\n"
    "// https://example.com/fsharp\n"
    "(* https://example.com/block *)\n");
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 11);

  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);       // namespace
  CHECK(styleAtColumn(highlight->lines[0], 10) == kClass);        // MyApp
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);       // open
  CHECK(styleAtColumn(highlight->lines[1], 5) == kClass);         // System.Collections
  CHECK(styleAtColumn(highlight->lines[2], 1) == kPreprocessor);  // #
  CHECK(styleAtColumn(highlight->lines[2], 1) == kPreprocessor);  // #if token
  CHECK(styleAtColumn(highlight->lines[3], 1) == kPunctuation);   // [<
  CHECK(styleAtColumn(highlight->lines[3], 3) == kAnnotation);    // Obsolete
  CHECK(styleAtColumn(highlight->lines[4], 0) == kKeyword);       // type
  CHECK(styleAtColumn(highlight->lines[4], 5) == kClass);         // Color
  CHECK(styleAtColumn(highlight->lines[5], 0) == kKeyword);       // let
  CHECK(styleAtColumn(highlight->lines[5], 4) == kKeyword);       // rec
  CHECK(styleAtColumn(highlight->lines[5], 8) == kMethod);        // fib
  CHECK(styleAtColumn(highlight->lines[6], 0) == kKeyword);       // let
  CHECK(styleAtColumn(highlight->lines[6], 4) == kVariable);      // x
  CHECK(styleAtColumn(highlight->lines[6], 8) == kNumber);        // 42
  CHECK(styleAtColumn(highlight->lines[7], 4) == kVariable);      // msg
  CHECK(styleAtColumn(highlight->lines[7], 10) == kString);       // "hello"
  CHECK(styleAtColumn(highlight->lines[8], 4) == kVariable);      // flag
  CHECK(styleAtColumn(highlight->lines[8], 11) == kBuiltin);      // true
  CHECK(styleAtColumn(highlight->lines[9], 0) == kComment);       // // comment start
  CHECK(styleAtColumn(highlight->lines[9], 3) == kUrl);           // line comment URL
  CHECK(styleAtColumn(highlight->lines[10], 0) == kComment);      // (* comment start
  CHECK(styleAtColumn(highlight->lines[10], 3) == kUrl);          // block comment URL
}

TEST_CASE("Grammar syntaxes keep comments, loops, descriptors, and interpolation distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kAbnfSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kBrainfuckSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJasmSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kNixSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kComment = 4;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kString = 2;

  SharedPtr<TextAnalyzer> abnf = engine->createAnalyzerBySyntaxName("abnf");
  REQUIRE(abnf != nullptr);
  SharedPtr<DocumentHighlight> abnfHighlight = abnf->analyzeText(
    "rule = [item ; note\n"
    " ] / %x41-5A\n");
  REQUIRE(abnfHighlight != nullptr);
  CHECK(styleAtColumn(abnfHighlight->lines[0], 0) == kVariable);
  CHECK(styleAtColumn(abnfHighlight->lines[0], 7) == kPunctuation);
  CHECK(styleAtColumn(abnfHighlight->lines[0], 13) == kComment);
  CHECK(styleAtColumn(abnfHighlight->lines[1], 1) == kPunctuation);
  CHECK(styleAtColumn(abnfHighlight->lines[1], 3) == kPunctuation);
  CHECK(styleAtColumn(abnfHighlight->lines[1], 5) == 3);

  SharedPtr<TextAnalyzer> brainfuck = engine->createAnalyzerBySyntaxName("brainfuck");
  REQUIRE(brainfuck != nullptr);
  SharedPtr<DocumentHighlight> brainfuckHighlight = brainfuck->analyzeText("]+[->+<] note\n");
  REQUIRE(brainfuckHighlight != nullptr);
  CHECK(styleAtColumn(brainfuckHighlight->lines[0], 0) == kPunctuation);
  CHECK(styleAtColumn(brainfuckHighlight->lines[0], 1) == kKeyword);
  CHECK(styleAtColumn(brainfuckHighlight->lines[0], 2) == kPunctuation);
  CHECK(styleAtColumn(brainfuckHighlight->lines[0], 7) == kPunctuation);
  CHECK(styleAtColumn(brainfuckHighlight->lines[0], 9) == kComment);

  SharedPtr<TextAnalyzer> jasm = engine->createAnalyzerBySyntaxName("jasm");
  REQUIRE(jasm != nullptr);
  SharedPtr<DocumentHighlight> jasmHighlight = jasm->analyzeText(
    "Method \"main\":\"()V\" public static {\n"
    "@interface com/example/MyAnno {\n");
  REQUIRE(jasmHighlight != nullptr);
  CHECK(styleAtColumn(jasmHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(jasmHighlight->lines[0], 8) == kMethod);
  CHECK(styleAtColumn(jasmHighlight->lines[0], 20) == kKeyword);
  CHECK(styleAtColumn(jasmHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(jasmHighlight->lines[1], 11) == 5);

  SharedPtr<TextAnalyzer> nix = engine->createAnalyzerBySyntaxName("nix");
  REQUIRE(nix != nullptr);
  SharedPtr<DocumentHighlight> nixHighlight = nix->analyzeText(
    "config = import <nixpkgs> // { value = \"${name}\"; };\n"
    "size = builtins.length list;\n");
  REQUIRE(nixHighlight != nullptr);
  CHECK(styleAtColumn(nixHighlight->lines[0], 0) == kProperty);
  CHECK(styleAtColumn(nixHighlight->lines[0], 9) == kKeyword);
  CHECK(styleAtColumn(nixHighlight->lines[0], 16) == kString);
  CHECK(styleAtColumn(nixHighlight->lines[0], 26) == kPunctuation);
  CHECK(styleAtColumn(nixHighlight->lines[0], 42) == kVariable);
  CHECK(styleAtColumn(nixHighlight->lines[1], 7) == kBuiltin);
  CHECK(styleAtColumn(nixHighlight->lines[1], 16) == kMethod);
}

TEST_CASE("Shader syntaxes keep directives, resources, and semantics distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kGlslSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kHlslSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kComment = 4;
  constexpr int32_t kClass = 5;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPreprocessor = 11;
  constexpr int32_t kAnnotation = 9;

  SharedPtr<TextAnalyzer> glsl = engine->createAnalyzerBySyntaxName("glsl");
  REQUIRE(glsl != nullptr);
  SharedPtr<DocumentHighlight> glslHighlight = glsl->analyzeText(
    "#version 450 core\n"
    "layout(location = 0) out vec4 FragColor;\n"
    "uniform samplerCubeArrayShadow shadowMap;\n"
    "gl_Position = vec4(1.0);\n"
    "// hello\n"
    "struct Light { vec3 pos; };\n");
  REQUIRE(glslHighlight != nullptr);
  CHECK(styleAtColumn(glslHighlight->lines[0], 0) == kPreprocessor);
  CHECK(styleAtColumn(glslHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(glslHighlight->lines[1], 25) == kBuiltin);
  CHECK(styleAtColumn(glslHighlight->lines[2], 8) == kBuiltin);
  CHECK(styleAtColumn(glslHighlight->lines[3], 0) == kVariable);
  CHECK(styleAtColumn(glslHighlight->lines[3], 14) == kBuiltin);
  CHECK(styleAtColumn(glslHighlight->lines[4], 0) == kComment);
  CHECK(styleAtColumn(glslHighlight->lines[5], 0) == kKeyword);
  CHECK(styleAtColumn(glslHighlight->lines[5], 7) == kClass);

  SharedPtr<TextAnalyzer> hlsl = engine->createAnalyzerBySyntaxName("hlsl");
  REQUIRE(hlsl != nullptr);
  SharedPtr<DocumentHighlight> hlslHighlight = hlsl->analyzeText(
    "#include \"common.hlsl\"\n"
    "Texture2DMS tex : register(t0);\n"
    "float4 color : SV_TARGET;\n"
    "// hello\n"
    "struct Light { float3 pos : POSITION; };\n");
  REQUIRE(hlslHighlight != nullptr);
  CHECK(styleAtColumn(hlslHighlight->lines[0], 0) == kPreprocessor);
  CHECK(styleAtColumn(hlslHighlight->lines[1], 0) == kBuiltin);
  CHECK(styleAtColumn(hlslHighlight->lines[1], 18) == kKeyword);
  CHECK(styleAtColumn(hlslHighlight->lines[1], 27) == kAnnotation);
  CHECK(styleAtColumn(hlslHighlight->lines[2], 0) == kBuiltin);
  CHECK(styleAtColumn(hlslHighlight->lines[2], 15) == kAnnotation);
  CHECK(styleAtColumn(hlslHighlight->lines[3], 0) == kComment);
  CHECK(styleAtColumn(hlslHighlight->lines[4], 0) == kKeyword);
  CHECK(styleAtColumn(hlslHighlight->lines[4], 7) == kClass);
}

TEST_CASE("Assembly dialect syntaxes keep directives, labels, and registers distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kAsmAttSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kAsmAarch64SyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kAsmIntelSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kComment = 4;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kPreprocessor = 11;
  constexpr int32_t kMacro = 12;
  constexpr int32_t kAnnotation = 9;

  SharedPtr<TextAnalyzer> att = engine->createAnalyzerBySyntaxName("asm-att");
  REQUIRE(att != nullptr);
  SharedPtr<DocumentHighlight> attHighlight = att->analyzeText(
    ".globl _start\n"
    ".Ltmp0:\n"
    "  vmovaps %zmm16, %zmm17\n"
    "  leaq label(%rip), %rax\n");
  REQUIRE(attHighlight != nullptr);
  CHECK(styleAtColumn(attHighlight->lines[0], 1) == kPreprocessor);
  CHECK(styleAtColumn(attHighlight->lines[1], 0) == kMethod);
  CHECK(styleAtColumn(attHighlight->lines[2], 11) == kBuiltin);
  CHECK(styleAtColumn(attHighlight->lines[3], 2) == kKeyword);

  SharedPtr<TextAnalyzer> aarch64 = engine->createAnalyzerBySyntaxName("asm-aarch64");
  REQUIRE(aarch64 != nullptr);
  SharedPtr<DocumentHighlight> aarch64Highlight = aarch64->analyzeText(
    "@ just comment\n"
    "#ifdef DEBUG\n"
    ".globl _start\n"
    "_start:\n"
    "  adrp x0, _msg@PAGE\n"
    "#endif\n");
  REQUIRE(aarch64Highlight != nullptr);
  CHECK(styleAtColumn(aarch64Highlight->lines[0], 0) == kComment);
  CHECK(styleAtColumn(aarch64Highlight->lines[1], 0) == kPreprocessor);
  CHECK(styleAtColumn(aarch64Highlight->lines[2], 1) == kPreprocessor);
  CHECK(styleAtColumn(aarch64Highlight->lines[3], 0) == kMethod);
  CHECK(styleAtColumn(aarch64Highlight->lines[4], 7) == kBuiltin);
  CHECK(styleAtColumn(aarch64Highlight->lines[4], 16) == kAnnotation);

  SharedPtr<TextAnalyzer> intel = engine->createAnalyzerBySyntaxName("asm-intel");
  REQUIRE(intel != nullptr);
  SharedPtr<DocumentHighlight> intelHighlight = intel->analyzeText(
    "%define SIZE 4\n"
    "section .data\n"
    "loop.start:\n"
    "  mov zmm31, [rbp+16] ; comment\n");
  REQUIRE(intelHighlight != nullptr);
  CHECK(styleAtColumn(intelHighlight->lines[0], 1) == kMacro);
  CHECK(styleAtColumn(intelHighlight->lines[1], 0) == kPreprocessor);
  CHECK(styleAtColumn(intelHighlight->lines[2], 0) == kMethod);
  CHECK(styleAtColumn(intelHighlight->lines[3], 6) == kBuiltin);
  CHECK(styleAtColumn(intelHighlight->lines[3], 14) == kBuiltin);
  CHECK(styleAtColumn(intelHighlight->lines[3], 18) == 3);
  CHECK(styleAtColumn(intelHighlight->lines[3], 23) == kComment);
}

TEST_CASE("DTS keeps hash-prefixed and boolean properties distinct from node names") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kDtsSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kProperty = 13;

  SharedPtr<TextAnalyzer> dts = engine->createAnalyzerBySyntaxName("dts");
  REQUIRE(dts != nullptr);
  SharedPtr<DocumentHighlight> highlight = dts->analyzeText(
    "/dts-v1/;\n"
    "/ {\n"
    "  #address-cells = <2>;\n"
    "  status;\n"
    "  /delete-property/ #size-cells;\n"
    "};\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[2], 2) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 2) == kProperty);
  CHECK(styleAtColumn(highlight->lines[4], 20) == kProperty);
  CHECK(styleAtColumn(highlight->lines[4], 31) == 8);
}

TEST_CASE("Smali keeps directives, descriptors, and member references distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kSmaliSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kClass = 5;
  constexpr int32_t kProperty = 13;

  SharedPtr<TextAnalyzer> smali = engine->createAnalyzerBySyntaxName("smali");
  REQUIRE(smali != nullptr);
  SharedPtr<DocumentHighlight> highlight = smali->analyzeText(
    ".field private counter:I\n"
    ".method public constructor <init>()V\n"
    ".end method\n"
    "iget-object v0, p0, Lfoo/bar/Baz;->field-name:Ljava/lang/String;\n"
    "Lcom/example/Foo;->counter:I\n"
    "Lcom/example/Foo;->name()V\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[0], 15) == kProperty);
  CHECK(styleAtColumn(highlight->lines[0], 23) == kBuiltin);
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 8) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 27) == kMethod);
  CHECK(styleAtColumn(highlight->lines[1], 35) == kBuiltin);
  CHECK(styleAtColumn(highlight->lines[3], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[3], 35) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 46) == kClass);
  CHECK(styleAtColumn(highlight->lines[4], 0) == kClass);
  CHECK(styleAtColumn(highlight->lines[4], 19) == kProperty);
  CHECK(styleAtColumn(highlight->lines[5], 19) == kMethod);
}

TEST_CASE("Zig keeps control keywords, line strings, and member access distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kZigSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kClass = 5;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kString = 2;

  SharedPtr<TextAnalyzer> zig = engine->createAnalyzerBySyntaxName("zig");
  REQUIRE(zig != nullptr);
  SharedPtr<DocumentHighlight> highlight = zig->analyzeText(
    "defer _ = 1;\n"
    "const Point = struct { x: i32 };\n"
    "const p = Point{ .x = 1 };\n"
    "const rgb = Color.red.toRgb();\n"
    "const value = 1.25;\n"
    "\\\\line string\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[0], 6) == kVariable);
  CHECK(styleAtColumn(highlight->lines[1], 6) == kClass);
  CHECK(styleAtColumn(highlight->lines[1], 14) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[2], 10) == kClass);
  CHECK(styleAtColumn(highlight->lines[2], 18) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 12) == kClass);
  CHECK(styleAtColumn(highlight->lines[3], 18) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 22) == kMethod);
  CHECK(styleAtColumn(highlight->lines[4], 12) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[4], 14) == kNumber);
  CHECK(styleAtColumn(highlight->lines[5], 0) == kString);
}

TEST_CASE("Batch comments only trigger for line-leading comment forms") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kBatchSyntaxPath));

  constexpr int32_t kComment = 4;

  SharedPtr<TextAnalyzer> batch = engine->createAnalyzerBySyntaxName("batch");
  REQUIRE(batch != nullptr);
  SharedPtr<DocumentHighlight> highlight = batch->analyzeText(
    "echo rem\n"
    "set VALUE=prefix::suffix\n"
    "REM real comment\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[0], 5) != kComment);
  CHECK(styleAtColumn(highlight->lines[1], 16) != kComment);
  CHECK(styleAtColumn(highlight->lines[2], 0) == kComment);
}

TEST_CASE("R and Julia keep package calls, macros, builtins, and URLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/r.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/julia.json"));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kMacro = 12;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> r = engine->createAnalyzerBySyntaxName("r");
  REQUIRE(r != nullptr);
  SharedPtr<DocumentHighlight> rHighlight = r->analyzeText(
    "stats::median(values)\n"
    "value <- function(x, y = 1) x + y\n"
    "flag <- TRUE\n"
    "text <- \"https://example.com/r\"\n");
  REQUIRE(rHighlight != nullptr);
  CHECK(styleAtColumn(rHighlight->lines[0], 0) == kClass);
  CHECK(styleAtColumn(rHighlight->lines[0], 7) == kMethod);
  CHECK(styleAtColumn(rHighlight->lines[1], 0) == kMethod);
  CHECK(styleAtColumn(rHighlight->lines[1], 9) == kKeyword);
  CHECK(styleAtColumn(rHighlight->lines[3], 9) == kUrl);

  SharedPtr<TextAnalyzer> julia = engine->createAnalyzerBySyntaxName("julia");
  REQUIRE(julia != nullptr);
  SharedPtr<DocumentHighlight> juliaHighlight = julia->analyzeText(
    "@show value\n"
    "struct Point{T<:Real}\n"
    "cmd = `echo $value`\n"
    "text = \"https://example.com/julia\"\n");
  REQUIRE(juliaHighlight != nullptr);
  CHECK(styleAtColumn(juliaHighlight->lines[0], 1) == kMacro);
  CHECK(styleAtColumn(juliaHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(juliaHighlight->lines[1], 7) == kClass);
  CHECK(styleAtColumn(juliaHighlight->lines[2], 6) == kString);
  CHECK(styleAtColumn(juliaHighlight->lines[3], 8) == kUrl);
}

TEST_CASE("Julia keeps local bindings, typed defaults, where generics, and strings styled") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kClass = 5;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;

  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/julia.json"));

  U8String code_txt = FileUtil::readString(TESTS_DIR"/files/example.jl");
  REQUIRE_FALSE(code_txt.empty());

  SharedPtr<Document> document = makeSharedPtr<Document>("example.jl", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 143);

  CHECK(styleAtColumn(highlight->lines[20], 8) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[20], 14) == kVariable);
  CHECK(styleAtColumn(highlight->lines[20], 20) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[46], 46) == kVariable);
  CHECK(styleAtColumn(highlight->lines[46], 71) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[46], 73) == kString);
  CHECK(styleAtColumn(highlight->lines[54], 50) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[54], 71) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[64], 44) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[64], 51) == kClass);
  CHECK(styleAtColumn(highlight->lines[72], 14) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[101], 12) == kString);
  CHECK(styleAtColumn(highlight->lines[105], 16) == kString);
  CHECK(styleAtColumn(highlight->lines[112], 68) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[112], 75) == kClass);
  CHECK(styleAtColumn(highlight->lines[142], 4) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[142], 8) == kVariable);
  CHECK(styleAtColumn(highlight->lines[142], 14) == kPunctuation);
}

TEST_CASE("R and Perl sample calls keep method names, punctuation, and sigils distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  constexpr int32_t kKeyword = 1;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kBuiltin = 10;

  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/r.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/perl.json"));

  U8String rCode = FileUtil::readString(TESTS_DIR"/files/example.r");
  REQUIRE_FALSE(rCode.empty());
  SharedPtr<Document> rDocument = makeSharedPtr<Document>("example.r", rCode);
  SharedPtr<DocumentAnalyzer> rAnalyzer = engine->loadDocument(rDocument);
  REQUIRE(rAnalyzer != nullptr);
  SharedPtr<DocumentHighlight> rHighlight = rAnalyzer->analyze();
  REQUIRE(rHighlight != nullptr);
  CHECK(styleAtColumn(rHighlight->lines[128], 16) == kMethod);
  CHECK(styleAtColumn(rHighlight->lines[128], 32) == kPunctuation);
  CHECK(styleAtColumn(rHighlight->lines[149], 15) == kBuiltin);
  CHECK(styleAtColumn(rHighlight->lines[149], 21) == kPunctuation);
  CHECK(styleAtColumn(rHighlight->lines[149], 49) == kKeyword);
  CHECK(styleAtColumn(rHighlight->lines[149], 57) == kPunctuation);
  CHECK(styleAtColumn(rHighlight->lines[160], 11) == kBuiltin);
  CHECK(styleAtColumn(rHighlight->lines[160], 17) == kPunctuation);
  CHECK(styleAtColumn(rHighlight->lines[164], 24) == kMethod);
  CHECK(styleAtColumn(rHighlight->lines[164], 29) == kPunctuation);

  U8String perlCode = FileUtil::readString(TESTS_DIR"/files/example.pl");
  REQUIRE_FALSE(perlCode.empty());
  SharedPtr<Document> perlDocument = makeSharedPtr<Document>("example.pl", perlCode);
  SharedPtr<DocumentAnalyzer> perlAnalyzer = engine->loadDocument(perlDocument);
  REQUIRE(perlAnalyzer != nullptr);
  SharedPtr<DocumentHighlight> perlHighlight = perlAnalyzer->analyze();
  REQUIRE(perlHighlight != nullptr);
  CHECK(styleAtColumn(perlHighlight->lines[84], 11) == kVariable);
  CHECK(styleAtColumn(perlHighlight->lines[84], 24) == kVariable);
  CHECK(styleAtColumn(perlHighlight->lines[84], 33) == kPunctuation);
  CHECK(styleAtColumn(perlHighlight->lines[175], 16) == kMethod);
  CHECK(styleAtColumn(perlHighlight->lines[175], 30) == kPunctuation);
  CHECK(styleAtColumn(perlHighlight->lines[176], 18) == kMethod);
  CHECK(styleAtColumn(perlHighlight->lines[176], 29) == kPunctuation);
  CHECK(styleAtColumn(perlHighlight->lines[176], 30) == kBuiltin);
  CHECK(styleAtColumn(perlHighlight->lines[176], 36) == kPunctuation);
}

TEST_CASE("Perl and Erlang keep sigils, directives, records, and regexes distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/perl.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/erlang.json"));

  constexpr int32_t kComment = 4;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPreprocessor = 11;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> perl = engine->createAnalyzerBySyntaxName("perl");
  REQUIRE(perl != nullptr);
  SharedPtr<DocumentHighlight> perlHighlight = perl->analyzeText(
    "sub render {\n"
    "    my $name = \"https://example.com/perl\";\n"
    "    $name =~ qr{^\\\\w+$};\n"
    "}\n");
  REQUIRE(perlHighlight != nullptr);
  CHECK(styleAtColumn(perlHighlight->lines[0], 4) == kMethod);
  CHECK(styleAtColumn(perlHighlight->lines[1], 7) == kVariable);
  CHECK(styleAtColumn(perlHighlight->lines[1], 17) == kUrl);
  CHECK(styleAtColumn(perlHighlight->lines[2], 4) == kVariable);
  CHECK(styleAtColumn(perlHighlight->lines[2], 13) == kAnnotation);

  SharedPtr<TextAnalyzer> erlang = engine->createAnalyzerBySyntaxName("erlang");
  REQUIRE(erlang != nullptr);
  SharedPtr<DocumentHighlight> erlangHighlight = erlang->analyzeText(
    "-module(demo).\n"
    "-record(person, {name, age = 0}).\n"
    "render(#person{name = <<\"Ada\">>}) ->\n"
    "    io:format(\"https://example.com/erl~n\"),\n"
    "    ok.\n");
  REQUIRE(erlangHighlight != nullptr);
  CHECK(styleAtColumn(erlangHighlight->lines[0], 1) == kPreprocessor);
  CHECK(styleAtColumn(erlangHighlight->lines[1], 8) == kClass);
  CHECK(styleAtColumn(erlangHighlight->lines[2], 0) == kMethod);
  CHECK(styleAtColumn(erlangHighlight->lines[2], 8) == kClass);
  CHECK(styleAtColumn(erlangHighlight->lines[3], 4) == kClass);
  CHECK(styleAtColumn(erlangHighlight->lines[3], 15) == kUrl);
}

TEST_CASE("Starlark, Bazel, and Gradle DSLs keep block heads and calls distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/starlark.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/bazel.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/gradle.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/gradle-kts.json"));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kProperty = 13;

  SharedPtr<TextAnalyzer> starlark = engine->createAnalyzerBySyntaxName("starlark");
  REQUIRE(starlark != nullptr);
  SharedPtr<DocumentHighlight> starlarkHighlight = starlark->analyzeText(
    "load(\"//tools:defs.bzl\", \"demo_rule\")\n"
    "def build_demo(name):\n"
    "    return struct(enabled = True)\n");
  REQUIRE(starlarkHighlight != nullptr);
  CHECK(styleAtColumn(starlarkHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(starlarkHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(starlarkHighlight->lines[1], 4) == kMethod);
  CHECK(styleAtColumn(starlarkHighlight->lines[2], 28) == kBuiltin);

  SharedPtr<TextAnalyzer> bazel = engine->createAnalyzerBySyntaxName("bazel");
  REQUIRE(bazel != nullptr);
  SharedPtr<DocumentHighlight> bazelHighlight = bazel->analyzeText(
    "cc_binary(\n"
    "    name = \"demo\",\n"
    "    deps = [\":core\"],\n"
    ")\n");
  REQUIRE(bazelHighlight != nullptr);
  CHECK(styleAtColumn(bazelHighlight->lines[0], 0) == kMethod);
  CHECK(styleAtColumn(bazelHighlight->lines[1], 4) == kProperty);
  CHECK(styleAtColumn(bazelHighlight->lines[2], 12) == kString);

  SharedPtr<TextAnalyzer> gradle = engine->createAnalyzerBySyntaxName("gradle");
  REQUIRE(gradle != nullptr);
  SharedPtr<DocumentHighlight> gradleHighlight = gradle->analyzeText(
    "plugins {\n"
    "    id 'java'\n"
    "}\n"
    "dependencies {\n"
    "    implementation 'com.google.guava:guava:1.0'\n"
    "}\n");
  REQUIRE(gradleHighlight != nullptr);
  CHECK(styleAtColumn(gradleHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(gradleHighlight->lines[1], 4) == kKeyword);
  CHECK(styleAtColumn(gradleHighlight->lines[3], 0) == kKeyword);
  CHECK(styleAtColumn(gradleHighlight->lines[4], 4) == kKeyword);

  SharedPtr<TextAnalyzer> gradleKts = engine->createAnalyzerBySyntaxName("gradle-kts");
  REQUIRE(gradleKts != nullptr);
  SharedPtr<DocumentHighlight> gradleKtsHighlight = gradleKts->analyzeText(
    "plugins {\n"
    "    kotlin(\"jvm\") version \"2.0.21\"\n"
    "}\n"
    "tasks.register(\"demo\") {\n"
    "    doLast {\n"
    "        println(\"https://example.com\")\n"
    "    }\n"
    "}\n");
  REQUIRE(gradleKtsHighlight != nullptr);
  CHECK(styleAtColumn(gradleKtsHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(gradleKtsHighlight->lines[1], 4) == kMethod);
  CHECK(styleAtColumn(gradleKtsHighlight->lines[3], 6) == kMethod);
  CHECK(styleAtColumn(gradleKtsHighlight->lines[5], 8) == kMethod);
}

TEST_CASE("QSharp and RISC-V keep declarations, interpolation, directives, and relocations distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/qsharp.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/asm-riscv.json"));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kPreprocessor = 11;
  constexpr int32_t kAnnotation = 9;

  SharedPtr<TextAnalyzer> qsharp = engine->createAnalyzerBySyntaxName("qsharp");
  REQUIRE(qsharp != nullptr);
  SharedPtr<DocumentHighlight> qsharpHighlight = qsharp->analyzeText(
    "newtype Angle = Double;\n"
    "newtype Pair = (Int, Int);\n"
    "function Clamp(value : Int) : Int {\n"
    "    let result = value;\n"
    "    let (first, second) = (1, 2);\n"
    "    Message($\"value={result}\");\n"
    "}\n"
    "operation Main() : Unit is Adj + Ctl {\n"
    "    Message($\"value={1}\");\n"
    "}\n");
  REQUIRE(qsharpHighlight != nullptr);
  CHECK(styleAtColumn(qsharpHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[0], 8) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[0], 16) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[1], 8) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[1], 17) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[2], 0) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[2], 9) == kMethod);
  CHECK(styleAtColumn(qsharpHighlight->lines[2], 15) == kVariable);
  CHECK(styleAtColumn(qsharpHighlight->lines[2], 23) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[2], 30) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[3], 4) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[3], 8) == kVariable);
  CHECK(styleAtColumn(qsharpHighlight->lines[4], 4) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[4], 9) == kVariable);
  CHECK(styleAtColumn(qsharpHighlight->lines[4], 16) == kVariable);
  CHECK(styleAtColumn(qsharpHighlight->lines[5], 4) == kMethod);
  CHECK(styleAtColumn(qsharpHighlight->lines[5], 20) == kAnnotation);
  CHECK(styleAtColumn(qsharpHighlight->lines[7], 0) == kKeyword);
  CHECK(styleAtColumn(qsharpHighlight->lines[7], 10) == kMethod);
  CHECK(styleAtColumn(qsharpHighlight->lines[7], 19) == kClass);
  CHECK(styleAtColumn(qsharpHighlight->lines[8], 4) == kMethod);
  CHECK(styleAtColumn(qsharpHighlight->lines[8], 20) == kAnnotation);

  SharedPtr<TextAnalyzer> riscv = engine->createAnalyzerBySyntaxName("asm-riscv");
  REQUIRE(riscv != nullptr);
  SharedPtr<DocumentHighlight> riscvHighlight = riscv->analyzeText(
    ".globl _start\n"
    "_start:\n"
    "    auipc t0, %hi(message)\n"
    "    addi t0, t0, %lo(message)\n");
  REQUIRE(riscvHighlight != nullptr);
  CHECK(styleAtColumn(riscvHighlight->lines[0], 1) == kPreprocessor);
  CHECK(styleAtColumn(riscvHighlight->lines[1], 0) == kMethod);
  CHECK(styleAtColumn(riscvHighlight->lines[2], 10) == kBuiltin);
  CHECK(styleAtColumn(riscvHighlight->lines[2], 14) == kAnnotation);
  CHECK(styleAtColumn(riscvHighlight->lines[3], 19) == kAnnotation);
}

TEST_CASE("Elixir keeps multiline strings and directive module names styled") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kClass = 5;

  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/elixir.json"));

  U8String code_txt = FileUtil::readString(TESTS_DIR"/files/example.ex");
  REQUIRE_FALSE(code_txt.empty());

  SharedPtr<Document> document = makeSharedPtr<Document>("example.ex", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 163);

  CHECK(styleAtColumn(highlight->lines[1], 13) == kString);
  CHECK(styleAtColumn(highlight->lines[1], 15) == kString);
  CHECK(styleAtColumn(highlight->lines[2], 2) == kString);
  CHECK(styleAtColumn(highlight->lines[6], 2) == kString);
  CHECK(styleAtColumn(highlight->lines[6], 4) == kString);
  CHECK(styleAtColumn(highlight->lines[8], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[8], 8) == kClass);
  CHECK(styleAtColumn(highlight->lines[8], 18) == kClass);
  CHECK(styleAtColumn(highlight->lines[9], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[9], 9) == kClass);
  CHECK(styleAtColumn(highlight->lines[10], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[10], 10) == kClass);
  CHECK(styleAtColumn(highlight->lines[12], 13) == kClass);
  CHECK(styleAtColumn(highlight->lines[97], 16) == kClass);
  CHECK(styleAtColumn(highlight->lines[100], 11) == kClass);
  CHECK(styleAtColumn(highlight->lines[159], 6) == kString);
  CHECK(styleAtColumn(highlight->lines[160], 6) == kString);
  CHECK(styleAtColumn(highlight->lines[162], 6) == kString);
  CHECK(styleAtColumn(highlight->lines[162], 8) == kString);
}

TEST_CASE("Haskell, Meson, and Just keep declarations, builtins, and exact-name DSLs distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kAnnotation = 9;

  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/haskell.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/meson.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/just.json"));

  SharedPtr<TextAnalyzer> haskell = engine->createAnalyzerBySyntaxName("haskell");
  REQUIRE(haskell != nullptr);
  SharedPtr<DocumentHighlight> haskellHighlight = haskell->analyzeText(
    "{-# LANGUAGE OverloadedStrings #-}\n"
    "module Example.Syntax where\n"
    "import qualified Data.Text as Text\n"
    "infixl 6 <+>\n"
    "renderTask task = show (TaskId 1)\n");
  REQUIRE(haskellHighlight != nullptr);
  CHECK(styleAtColumn(haskellHighlight->lines[0], 0) == kAnnotation);
  CHECK(styleAtColumn(haskellHighlight->lines[0], 4) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[1], 7) == kClass);
  CHECK(styleAtColumn(haskellHighlight->lines[1], 22) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[2], 0) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[2], 7) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[2], 17) == kClass);
  CHECK(styleAtColumn(haskellHighlight->lines[2], 27) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[2], 30) == kClass);
  CHECK(styleAtColumn(haskellHighlight->lines[3], 0) == kKeyword);
  CHECK(styleAtColumn(haskellHighlight->lines[3], 7) == 3);
  CHECK(styleAtColumn(haskellHighlight->lines[3], 9) == kMethod);
  CHECK(styleAtColumn(haskellHighlight->lines[4], 0) == kMethod);
  CHECK(styleAtColumn(haskellHighlight->lines[4], 11) == kVariable);
  CHECK(styleAtColumn(haskellHighlight->lines[4], 18) == kBuiltin);
  CHECK(styleAtColumn(haskellHighlight->lines[4], 24) == kClass);

  SharedPtr<TextAnalyzer> meson = engine->createAnalyzerBySyntaxName("meson");
  REQUIRE(meson != nullptr);
  SharedPtr<DocumentHighlight> mesonHighlight = meson->analyzeText(
    "project('demo', 'c')\n"
    "project_name = meson.project_name()\n"
    "if true\n"
    "  configure_file(input : 'a', output : 'b')\n"
    "endif\n");
  REQUIRE(mesonHighlight != nullptr);
  CHECK(styleAtColumn(mesonHighlight->lines[0], 0) == kBuiltin);
  CHECK(styleAtColumn(mesonHighlight->lines[0], 7) == kPunctuation);
  CHECK(styleAtColumn(mesonHighlight->lines[1], 0) == kVariable);
  CHECK(styleAtColumn(mesonHighlight->lines[1], 13) == kPunctuation);
  CHECK(styleAtColumn(mesonHighlight->lines[1], 15) == kBuiltin);
  CHECK(styleAtColumn(mesonHighlight->lines[1], 21) == kMethod);
  CHECK(styleAtColumn(mesonHighlight->lines[2], 0) == kKeyword);
  CHECK(styleAtColumn(mesonHighlight->lines[2], 3) == kBuiltin);
  CHECK(styleAtColumn(mesonHighlight->lines[3], 2) == kBuiltin);
  CHECK(styleAtColumn(mesonHighlight->lines[3], 17) == 13);
  CHECK(styleAtColumn(mesonHighlight->lines[3], 30) == 13);
  CHECK(styleAtColumn(mesonHighlight->lines[4], 0) == kKeyword);

  SharedPtr<Document> justDocument = makeSharedPtr<Document>(
    "Justfile",
    "set shell := [\"bash\", \"-eu\"]\n"
    "mod? tools 'tools.just'\n"
    "alias fmt := clean\n"
    "name := \"sweetline\"\n"
    "[default]\n"
    "default:\n"
    "  just --list\n");
  SharedPtr<DocumentAnalyzer> justAnalyzer = engine->loadDocument(justDocument);
  REQUIRE(justAnalyzer != nullptr);
  SharedPtr<DocumentHighlight> justHighlight = justAnalyzer->analyze();
  REQUIRE(justHighlight != nullptr);
  CHECK(styleAtColumn(justHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(justHighlight->lines[0], 4) == 13);
  CHECK(styleAtColumn(justHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(justHighlight->lines[1], 3) == kPunctuation);
  CHECK(styleAtColumn(justHighlight->lines[2], 0) == kKeyword);
  CHECK(styleAtColumn(justHighlight->lines[3], 0) == kVariable);
  CHECK(styleAtColumn(justHighlight->lines[3], 5) == kPunctuation);
  CHECK(styleAtColumn(justHighlight->lines[3], 8) == kString);
  CHECK(styleAtColumn(justHighlight->lines[4], 1) == kAnnotation);
  CHECK(styleAtColumn(justHighlight->lines[5], 0) == kMethod);
  CHECK(styleAtColumn(justHighlight->lines[6], 2) == kBuiltin);
}

TEST_CASE("SystemVerilog and Solidity keep preprocessors, declarations, and DSL bodies distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  constexpr int32_t kKeyword = 1;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kPreprocessor = 11;

  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/systemverilog.json"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/solidity.json"));

  SharedPtr<TextAnalyzer> systemverilog = engine->createAnalyzerBySyntaxName("systemverilog");
  REQUIRE(systemverilog != nullptr);
  SharedPtr<DocumentHighlight> systemverilogHighlight = systemverilog->analyzeText(
    "`timescale 1ns/1ps\n"
    "package demo_pkg;\n"
    "  typedef enum logic [2:0] { OP_ADD = 3'd0 } op_t;\n"
    "  function automatic logic [31:0] sat_add(input logic [31:0] a);\n"
    "    $display(\"ok\");\n"
    "  endfunction\n"
    "endpackage\n");
  REQUIRE(systemverilogHighlight != nullptr);
  CHECK(styleAtColumn(systemverilogHighlight->lines[0], 1) == kPreprocessor);
  CHECK(styleAtColumn(systemverilogHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(systemverilogHighlight->lines[1], 8) == kClass);
  CHECK(styleAtColumn(systemverilogHighlight->lines[2], 2) == kKeyword);
  CHECK(styleAtColumn(systemverilogHighlight->lines[2], 10) == kKeyword);
  CHECK(styleAtColumn(systemverilogHighlight->lines[2], 15) == kKeyword);
  CHECK(styleAtColumn(systemverilogHighlight->lines[3], 2) == kKeyword);
  CHECK(styleAtColumn(systemverilogHighlight->lines[3], 34) == kMethod);
  CHECK(styleAtColumn(systemverilogHighlight->lines[4], 5) == kBuiltin);

  SharedPtr<TextAnalyzer> solidity = engine->createAnalyzerBySyntaxName("solidity");
  REQUIRE(solidity != nullptr);
  SharedPtr<DocumentHighlight> solidityHighlight = solidity->analyzeText(
    "pragma solidity ^0.8.24;\n"
    "interface IPriceOracle {\n"
    "    function quote(uint256 amount) external view returns (uint256 price);\n"
    "}\n"
    "contract ExampleVault is IPriceOracle {\n"
    "    mapping(address => uint256) private balances;\n"
    "    function quote(uint256 amount) external view returns (uint256 price) {\n"
    "        assembly { let x := amount }\n"
    "    }\n"
    "}\n");
  REQUIRE(solidityHighlight != nullptr);
  CHECK(styleAtColumn(solidityHighlight->lines[0], 0) == kPreprocessor);
  CHECK(styleAtColumn(solidityHighlight->lines[0], 7) == kKeyword);
  CHECK(styleAtColumn(solidityHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(solidityHighlight->lines[1], 10) == kClass);
  CHECK(styleAtColumn(solidityHighlight->lines[2], 4) == kKeyword);
  CHECK(styleAtColumn(solidityHighlight->lines[2], 13) == kMethod);
  CHECK(styleAtColumn(solidityHighlight->lines[4], 0) == kKeyword);
  CHECK(styleAtColumn(solidityHighlight->lines[4], 9) == kClass);
  CHECK(styleAtColumn(solidityHighlight->lines[4], 25) == kClass);
  CHECK(styleAtColumn(solidityHighlight->lines[5], 4) == kBuiltin);
  CHECK(styleAtColumn(solidityHighlight->lines[6], 13) == kMethod);
  CHECK(styleAtColumn(solidityHighlight->lines[7], 8) == kKeyword);
  CHECK(styleAtColumn(solidityHighlight->lines[7], 17) == kPunctuation);
}

TEST_CASE("New language samples do not emit transparent spans") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> syntax_files = {
    "r.json", "julia.json", "perl.json", "clojure.json", "elixir.json", "erlang.json"
  };
  for (const U8String& syntax_file : syntax_files) {
    CAPTURE(syntax_file);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/" + syntax_file));
  }

  const List<U8String> sample_files = {
    TESTS_DIR"/files/example.r",
    TESTS_DIR"/files/example.jl",
    TESTS_DIR"/files/example.pl",
    TESTS_DIR"/files/example.clj",
    TESTS_DIR"/files/example.ex",
    TESTS_DIR"/files/example.erl"
  };
  for (const U8String& file_path : sample_files) {
    CAPTURE(file_path);
    U8String code_txt = FileUtil::readString(file_path);
    REQUIRE_FALSE(code_txt.empty());
    U8String file_name = std::filesystem::u8path(file_path).filename().u8string();
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByFileName(file_name);
    REQUIRE(analyzer != nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(code_txt);
    requireNoTransparentSpans(highlight, file_name);
  }
}

TEST_CASE("Second batch language samples do not emit transparent spans") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> syntax_files = {
    "haskell.json", "systemverilog.json", "solidity.json", "meson.json", "just.json"
  };
  for (const U8String& syntax_file : syntax_files) {
    CAPTURE(syntax_file);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/" + syntax_file));
  }

  const List<U8String> sample_files = {
    TESTS_DIR"/files/example.hs",
    TESTS_DIR"/files/example.sv",
    TESTS_DIR"/files/example.sol",
    TESTS_DIR"/files/meson.build",
    TESTS_DIR"/files/Justfile"
  };
  for (const U8String& file_path : sample_files) {
    CAPTURE(file_path);
    U8String code_txt = FileUtil::readString(file_path);
    REQUIRE_FALSE(code_txt.empty());
    U8String file_name = std::filesystem::u8path(file_path).filename().u8string();
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByFileName(file_name);
    REQUIRE(analyzer != nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(code_txt);
    requireNoTransparentSpans(highlight, file_name);
  }
}

TEST_CASE("Cangjie declarations and literals keep core styles") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCangjieSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> cangjie = engine->createAnalyzerBySyntaxName("cangjie");
  REQUIRE(cangjie != nullptr);
  SharedPtr<DocumentHighlight> highlight = cangjie->analyzeText(
    "package demo.routing\n"
    "public class Router<T> where T <: Notifier {\n"
    "  public func route(ticket: Ticket): Result<User> {\n"
    "    let retries: UInt32 = 0x10u32\n"
    "    let ratio = 0x1.fp4f64\n"
    "    let body = \"https://example.com/${ticket.id}\"\n"
    "    match true { case _ => return Result<User>() }\n"
    "  }\n"
    "}\n"
    "main() { println(\"ready\") }\n");
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 10);

  CHECK(styleAtColumn(highlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[0], 8) == kVariable);
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 7) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 13) == kClass);
  CHECK(styleAtColumn(highlight->lines[1], 19) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[1], 23) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 40) == kClass);
  CHECK(styleAtColumn(highlight->lines[2], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[2], 9) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[2], 14) == kMethod);
  CHECK(styleAtColumn(highlight->lines[2], 20) == kVariable);
  CHECK(styleAtColumn(highlight->lines[2], 28) == kClass);
  CHECK(styleAtColumn(highlight->lines[2], 37) == kClass);
  CHECK(styleAtColumn(highlight->lines[2], 44) == kClass);
  CHECK(styleAtColumn(highlight->lines[3], 4) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[3], 8) == kVariable);
  CHECK(styleAtColumn(highlight->lines[3], 17) == kClass);
  CHECK(styleAtColumn(highlight->lines[3], 26) == kNumber);
  CHECK(styleAtColumn(highlight->lines[4], 16) == kNumber);
  CHECK(styleAtColumn(highlight->lines[5], 16) == kUrl);
  CHECK(styleAtColumn(highlight->lines[5], 37) == kAnnotation);
  CHECK(styleAtColumn(highlight->lines[6], 4) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[6], 10) == kBuiltin);
  CHECK(styleAtColumn(highlight->lines[6], 17) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[6], 32) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[6], 39) == kClass);
  CHECK(styleAtColumn(highlight->lines[9], 0) == kMethod);
  CHECK(styleAtColumn(highlight->lines[9], 9) == kBuiltin);
  CHECK(styleAtColumn(highlight->lines[9], 17) == kString);
}

TEST_CASE("Cangjie bindings and member access keep semantic styles") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCangjieSyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kProperty = 13;

  SharedPtr<TextAnalyzer> cangjie = engine->createAnalyzerBySyntaxName("cangjie");
  REQUIRE(cangjie != nullptr);
  SharedPtr<DocumentHighlight> highlight = cangjie->analyzeText(
    "public class Router {\n"
    "  public init(channel: String, retry: UInt32) {\n"
    "    this.channel = channel\n"
    "    this.headers[\"trace\"] = channel\n"
    "  }\n"
    "  func route(items: Array<String>): Unit {\n"
    "    for item in items {\n"
    "      println(item)\n"
    "    }\n"
    "    try {\n"
    "      send()\n"
    "    } catch (error: Exception) {\n"
    "      println(error)\n"
    "    }\n"
    "  }\n"
    "}\n");
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->lines.size() >= 16);

  CHECK(styleAtColumn(highlight->lines[1], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 9) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[1], 14) == kVariable);
  CHECK(styleAtColumn(highlight->lines[1], 23) == kClass);
  CHECK(styleAtColumn(highlight->lines[1], 31) == kVariable);
  CHECK(styleAtColumn(highlight->lines[1], 38) == kClass);

  CHECK(styleAtColumn(highlight->lines[2], 4) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[2], 8) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[2], 9) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 9) == kProperty);
  CHECK(styleAtColumn(highlight->lines[3], 16) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[3], 17) == kString);

  CHECK(styleAtColumn(highlight->lines[5], 2) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[5], 7) == kMethod);
  CHECK(styleAtColumn(highlight->lines[5], 13) == kVariable);
  CHECK(styleAtColumn(highlight->lines[5], 20) == kClass);
  CHECK(styleAtColumn(highlight->lines[5], 26) == kClass);
  CHECK(styleAtColumn(highlight->lines[5], 37) == kClass);

  CHECK(styleAtColumn(highlight->lines[6], 4) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[6], 8) == kVariable);
  CHECK(styleAtColumn(highlight->lines[6], 13) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[6], 16) == kVariable);

  CHECK(styleAtColumn(highlight->lines[10], 6) == kMethod);
  CHECK(styleAtColumn(highlight->lines[11], 6) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[11], 12) == kPunctuation);
  CHECK(styleAtColumn(highlight->lines[11], 13) == kVariable);
  CHECK(styleAtColumn(highlight->lines[11], 20) == kClass);
}

TEST_CASE("Cangjie sample does not emit transparent spans") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kCangjieSyntaxPath));

  U8String code_txt = FileUtil::readString(kCangjieExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());

  SharedPtr<TextAnalyzer> cangjie = engine->createAnalyzerBySyntaxName("cangjie");
  REQUIRE(cangjie != nullptr);
  SharedPtr<DocumentHighlight> highlight = cangjie->analyzeText(code_txt);
  requireNoTransparentSpans(highlight, "example.cj");
}

TEST_CASE("AI-era syntaxes keep core declarations and template markers styled") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> syntax_files = {
    "moonbit.json", "mojo.json", "bend.json", "baml.json", "lmql.json", "prompty.json"
  };
  for (const U8String& syntax_file : syntax_files) {
    CAPTURE(syntax_file);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/" + syntax_file));
  }

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kString = 2;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kClass = 5;
  constexpr int32_t kMethod = 6;
  constexpr int32_t kVariable = 7;
  constexpr int32_t kPunctuation = 8;
  constexpr int32_t kAnnotation = 9;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kProperty = 13;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> moonbit = engine->createAnalyzerBySyntaxName("moonbit");
  REQUIRE(moonbit != nullptr);
  SharedPtr<DocumentHighlight> moonbitHighlight = moonbit->analyzeText(
    "pub struct Ticket {\n"
    "  id: String\n"
    "}\n"
    "fn route(ticket: Ticket) -> String {\n"
    "  \"https://example.com/mbt\"\n"
    "}\n");
  REQUIRE(moonbitHighlight != nullptr);
  CHECK(styleAtColumn(moonbitHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitHighlight->lines[0], 4) == kKeyword);
  CHECK(styleAtColumn(moonbitHighlight->lines[0], 11) == kClass);
  CHECK(styleAtColumn(moonbitHighlight->lines[3], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitHighlight->lines[3], 3) == kMethod);
  CHECK(styleAtColumn(moonbitHighlight->lines[4], 3) == kUrl);

  SharedPtr<DocumentHighlight> moonbitRecordHighlight = moonbit->analyzeText(
    "pub struct Customer {\n"
    "  id: String\n"
    "  tier: Tier\n"
    "}\n"
    "match tier {\n"
    "  Team => 3\n"
    "}\n"
    "for a in b {\n"
    "}\n"
    "pub fn main {\n"
    "}\n");
  REQUIRE(moonbitRecordHighlight != nullptr);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[0], 4) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[0], 11) == kClass);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[1], 2) == kProperty);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[1], 6) == kBuiltin);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[2], 2) == kProperty);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[2], 8) == kClass);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[4], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[4], 6) == -1);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[7], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[7], 4) == -1);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[7], 6) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[7], 9) == -1);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[9], 0) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[9], 4) == kKeyword);
  CHECK(styleAtColumn(moonbitRecordHighlight->lines[9], 7) == kMethod);

  SharedPtr<TextAnalyzer> mojo = engine->createAnalyzerBySyntaxName("mojo");
  REQUIRE(mojo != nullptr);
  SharedPtr<DocumentHighlight> mojoHighlight = mojo->analyzeText(
    "struct Router:\n"
    "    fn score(self, ticket: Ticket) -> Float64:\n"
    "        return 12.0\n");
  REQUIRE(mojoHighlight != nullptr);
  CHECK(styleAtColumn(mojoHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(mojoHighlight->lines[0], 7) == kClass);
  CHECK(styleAtColumn(mojoHighlight->lines[1], 4) == kKeyword);
  CHECK(styleAtColumn(mojoHighlight->lines[1], 7) == kMethod);
  CHECK(styleAtColumn(mojoHighlight->lines[1], 27) == kClass);

  SharedPtr<TextAnalyzer> bend = engine->createAnalyzerBySyntaxName("bend");
  REQUIRE(bend != nullptr);
  SharedPtr<DocumentHighlight> bendHighlight = bend->analyzeText(
    "def checked route(ticket):\n"
    "  return fork(IO/print(\"https://example.com/bend\"))\n");
  REQUIRE(bendHighlight != nullptr);
  CHECK(styleAtColumn(bendHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(bendHighlight->lines[0], 4) == kKeyword);
  CHECK(styleAtColumn(bendHighlight->lines[0], 12) == kMethod);
  CHECK(styleAtColumn(bendHighlight->lines[1], 2) == kKeyword);
  CHECK(styleAtColumn(bendHighlight->lines[1], 9) == kBuiltin);
  CHECK(styleAtColumn(bendHighlight->lines[1], 24) == kUrl);

  SharedPtr<DocumentHighlight> bendImportHighlight = bend->analyzeText(
    "import (lib/list/map, lib/list/fold)\n"
    "import ./effects/io as effects_io\n");
  REQUIRE(bendImportHighlight != nullptr);
  CHECK(styleAtColumn(bendImportHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(bendImportHighlight->lines[0], 7) == kPunctuation);
  CHECK(styleAtColumn(bendImportHighlight->lines[0], 8) == kString);
  CHECK(styleAtColumn(bendImportHighlight->lines[0], 20) == kPunctuation);
  CHECK(styleAtColumn(bendImportHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(bendImportHighlight->lines[1], 7) == kString);
  CHECK(styleAtColumn(bendImportHighlight->lines[1], 20) == kKeyword);
  CHECK(styleAtColumn(bendImportHighlight->lines[1], 23) == kVariable);

  SharedPtr<DocumentHighlight> bendFunHighlight = bend->analyzeText(
    "MakePair fst snd = Pair { fst: fst, snd: snd }\n"
    "FunLength (Cons head tail) = (+ 1 (FunLength tail))\n"
    "FunMapMaybe (Option/Some value) fn = (Option/Some (fn value))\n");
  REQUIRE(bendFunHighlight != nullptr);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 0) == kMethod);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 9) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 13) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 19) == kClass);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 26) == kProperty);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 31) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 36) == kProperty);
  CHECK(styleAtColumn(bendFunHighlight->lines[0], 41) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[1], 0) == kMethod);
  CHECK(styleAtColumn(bendFunHighlight->lines[1], 11) == kClass);
  CHECK(styleAtColumn(bendFunHighlight->lines[1], 16) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[1], 21) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[1], 45) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 0) == kMethod);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 13) == kClass);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 25) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 32) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 51) == kVariable);
  CHECK(styleAtColumn(bendFunHighlight->lines[2], 54) == kVariable);

  SharedPtr<TextAnalyzer> baml = engine->createAnalyzerBySyntaxName("baml");
  REQUIRE(baml != nullptr);
  SharedPtr<DocumentHighlight> bamlHighlight = baml->analyzeText(
    "function RouteTicket(ticket: SupportTicket) -> RoutingDecision {\n"
    "  prompt #\"Hello {{ ticket.subject }}\"#\n"
    "}\n");
  REQUIRE(bamlHighlight != nullptr);
  CHECK(styleAtColumn(bamlHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(bamlHighlight->lines[0], 9) == kMethod);
  CHECK(styleAtColumn(bamlHighlight->lines[0], 29) == kClass);
  CHECK(styleAtColumn(bamlHighlight->lines[1], 2) == kKeyword);
  CHECK(styleAtColumn(bamlHighlight->lines[1], 17) == kPunctuation);
  CHECK(styleAtColumn(bamlHighlight->lines[1], 20) == kVariable);
  CHECK(styleAtColumn(bamlHighlight->lines[1], 27) == kProperty);

  SharedPtr<DocumentHighlight> bamlConfigHighlight = baml->analyzeText(
    "client<llm> FastOpenAI {\n"
    "  options {\n"
    "    api_key env.OPENAI_API_KEY\n"
    "  }\n"
    "}\n"
    "retry_policy ShortRetry {\n"
    "  max_retries 2\n"
    "  strategy \"exponential_backoff\"\n"
    "}\n");
  REQUIRE(bamlConfigHighlight != nullptr);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[0], 6) == kPunctuation);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[0], 7) == kBuiltin);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[0], 10) == kPunctuation);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[0], 12) == kClass);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[2], 4) == kProperty);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[2], 12) == kBuiltin);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[2], 15) == kPunctuation);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[2], 16) == kProperty);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[5], 0) == kKeyword);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[5], 13) == kClass);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[6], 2) == kProperty);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[6], 14) == kNumber);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[7], 2) == kProperty);
  CHECK(styleAtColumn(bamlConfigHighlight->lines[7], 11) == kString);

  SharedPtr<DocumentHighlight> bamlTestHighlight = baml->analyzeText(
    "test DraftReplySmoke {\n"
    "  functions [DraftReply]\n"
    "  args {\n"
    "    tone Formal\n"
    "    decision {\n"
    "      priority High\n"
    "      team \"platform-support\"\n"
    "      reason \"Deployment is blocked\"\n"
    "      confidence 0.82\n"
    "    }\n"
    "  }\n"
    "}\n");
  REQUIRE(bamlTestHighlight != nullptr);
  CHECK(styleAtColumn(bamlTestHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(bamlTestHighlight->lines[0], 5) == kVariable);
  CHECK(styleAtColumn(bamlTestHighlight->lines[1], 2) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[2], 2) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[3], 4) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[3], 9) == kVariable);
  CHECK(styleAtColumn(bamlTestHighlight->lines[4], 4) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[5], 6) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[5], 15) == kVariable);
  CHECK(styleAtColumn(bamlTestHighlight->lines[6], 6) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[6], 11) == kString);
  CHECK(styleAtColumn(bamlTestHighlight->lines[7], 6) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[7], 13) == kString);
  CHECK(styleAtColumn(bamlTestHighlight->lines[8], 6) == kProperty);
  CHECK(styleAtColumn(bamlTestHighlight->lines[8], 17) == kNumber);

  SharedPtr<TextAnalyzer> lmql = engine->createAnalyzerBySyntaxName("lmql");
  REQUIRE(lmql != nullptr);
  SharedPtr<DocumentHighlight> lmqlHighlight = lmql->analyzeText(
    "argmax\n"
    "    \"Tone: [TONE]\" where TONE in [\" calm\"]\n"
    "from\n"
    "    \"openai/gpt-4o-mini\"\n");
  REQUIRE(lmqlHighlight != nullptr);
  CHECK(styleAtColumn(lmqlHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(lmqlHighlight->lines[1], 4) == kString);
  CHECK(styleAtColumn(lmqlHighlight->lines[1], 12) == kVariable);
  CHECK(styleAtColumn(lmqlHighlight->lines[1], 20) == kKeyword);
  CHECK(styleAtColumn(lmqlHighlight->lines[2], 0) == kKeyword);

  SharedPtr<DocumentHighlight> lmqlPythonHighlight = lmql->analyzeText(
    "import lmql\n"
    "from typing import Any\n"
    "for a in b:\n"
    "    pass\n"
    "\"Summary: [SUMMARY]\" where len(TOKENS(SUMMARY)) < 30\n");
  REQUIRE(lmqlPythonHighlight != nullptr);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[0], 7) == kVariable);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[1], 5) == kVariable);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[1], 12) == kKeyword);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[1], 19) == kClass);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[2], 0) == kKeyword);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[2], 4) == kVariable);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[2], 6) == kKeyword);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[2], 9) == kVariable);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[4], 11) == kVariable);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[4], 32) == kBuiltin);
  CHECK(styleAtColumn(lmqlPythonHighlight->lines[4], 39) == kVariable);

  SharedPtr<TextAnalyzer> prompty = engine->createAnalyzerBySyntaxName("prompty");
  REQUIRE(prompty != nullptr);
  SharedPtr<DocumentHighlight> promptyHighlight = prompty->analyzeText(
    "---\n"
    "name: demo\n"
    "---\n"
    "system:\n"
    "Hello {{ user.name }}\n");
  REQUIRE(promptyHighlight != nullptr);
  CHECK(styleAtColumn(promptyHighlight->lines[0], 0) == kPunctuation);
  CHECK(styleAtColumn(promptyHighlight->lines[1], 0) == kProperty);
  CHECK(styleAtColumn(promptyHighlight->lines[3], 0) == kKeyword);
  CHECK(styleAtColumn(promptyHighlight->lines[4], 6) == kPunctuation);
  CHECK(styleAtColumn(promptyHighlight->lines[4], 9) == kVariable);
}

TEST_CASE("AI-era language samples do not emit transparent spans") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> syntax_files = {
    "moonbit.json", "mojo.json", "bend.json", "baml.json", "lmql.json", "prompty.json"
  };
  for (const U8String& syntax_file : syntax_files) {
    CAPTURE(syntax_file);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(SYNTAX_DIR"/" + syntax_file));
  }

  const List<U8String> sample_files = {
    TESTS_DIR"/files/example.mbt",
    TESTS_DIR"/files/example.mojo",
    TESTS_DIR"/files/example.bend",
    TESTS_DIR"/files/example.baml",
    TESTS_DIR"/files/example.lmql",
    TESTS_DIR"/files/example.prompty"
  };
  for (const U8String& file_path : sample_files) {
    CAPTURE(file_path);
    U8String code_txt = FileUtil::readString(file_path);
    REQUIRE_FALSE(code_txt.empty());
    U8String file_name = std::filesystem::u8path(file_path).filename().u8string();
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByFileName(file_name);
    REQUIRE(analyzer != nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(code_txt);
    requireNoTransparentSpans(highlight, file_name);
  }
}

TEST_CASE("SVG routes cleanly alongside XML") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kXmlSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kSvgSyntaxPath));

  CHECK(engine->createAnalyzerByFileName("icon.svg") != nullptr);
  CHECK(engine->createAnalyzerByFileName("layout.xml") != nullptr);
}

TEST_CASE("HLSL routes cleanly alongside shell and groovy") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kShellSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kGroovySyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kHlslSyntaxPath));

  CHECK(engine->createAnalyzerByFileName("tool.csh") != nullptr);
  CHECK(engine->createAnalyzerByFileName("build.gsh") != nullptr);
  CHECK(engine->createAnalyzerByFileName("shader.hlsl") != nullptr);
}
