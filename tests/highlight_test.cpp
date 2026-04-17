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
  static const char* kJavascriptSyntaxPath = SYNTAX_DIR"/javascript.json";
  static const char* kTypescriptSyntaxPath = SYNTAX_DIR"/typescript.json";
  static const char* kCssSyntaxPath = SYNTAX_DIR"/css.json";
  static const char* kScssSyntaxPath = SYNTAX_DIR"/scss.json";
  static const char* kLessSyntaxPath = SYNTAX_DIR"/less.json";
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
  static const char* kGitignoreSyntaxPath = SYNTAX_DIR"/gitignore.json";
  static const char* kDiffSyntaxPath = SYNTAX_DIR"/diff.json";
  static const char* kRubySyntaxPath = SYNTAX_DIR"/ruby.json";
  static const char* kHclSyntaxPath = SYNTAX_DIR"/hcl.json";
  static const char* kTerraformSyntaxPath = SYNTAX_DIR"/terraform.json";
  static const char* kVueSyntaxPath = SYNTAX_DIR"/vue.json";
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

TEST_CASE("URL inside string and comment gets dedicated style") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("java");
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

  SharedPtr<TextAnalyzer> css = engine->createAnalyzerByName("css");
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

  SharedPtr<TextAnalyzer> scss = engine->createAnalyzerByName("scss");
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

  SharedPtr<TextAnalyzer> less = engine->createAnalyzerByName("less");
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

TEST_CASE("JSONC and JSON5 keep keys, values, numbers, comments, and builtins distinct") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJsoncSyntaxPath));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJson5SyntaxPath));

  constexpr int32_t kKeyword = 1;
  constexpr int32_t kNumber = 3;
  constexpr int32_t kBuiltin = 10;
  constexpr int32_t kUrl = 16;

  SharedPtr<TextAnalyzer> jsonc = engine->createAnalyzerByName("jsonc");
  REQUIRE(jsonc != nullptr);
  SharedPtr<DocumentHighlight> jsoncHighlight = jsonc->analyzeText(
    "{\n"
    "  \"site\": \"https://example.com\",\n"
    "  // mirror https://mirror.example.com\n"
    "}\n");
  REQUIRE(jsoncHighlight != nullptr);
  CHECK(styleAtColumn(jsoncHighlight->lines[1], 3) == kKeyword);
  CHECK(styleAtColumn(jsoncHighlight->lines[1], 11) == kUrl);
  CHECK(styleAtColumn(jsoncHighlight->lines[2], 12) == kUrl);

  SharedPtr<TextAnalyzer> json5 = engine->createAnalyzerByName("json5");
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

  SharedPtr<TextAnalyzer> cmake = engine->createAnalyzerByName("cmake");
  REQUIRE(cmake != nullptr);
  SharedPtr<DocumentHighlight> cmakeHighlight = cmake->analyzeText(
    "set(APP_DOCS_URL \"https://example.com\")\n"
    "if(DEFINED APP_DOCS_URL)\n");
  REQUIRE(cmakeHighlight != nullptr);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 0) == kMethod);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 4) == kVariable);
  CHECK(styleAtColumn(cmakeHighlight->lines[0], 18) == kUrl);
  CHECK(styleAtColumn(cmakeHighlight->lines[1], 0) == kKeyword);

  SharedPtr<TextAnalyzer> dockerfile = engine->createAnalyzerByName("dockerfile");
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

  SharedPtr<TextAnalyzer> makefile = engine->createAnalyzerByName("makefile");
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

  SharedPtr<TextAnalyzer> properties = engine->createAnalyzerByName("properties");
  REQUIRE(properties != nullptr);
  SharedPtr<DocumentHighlight> propertiesHighlight = properties->analyzeText(
    "app.docs=https://example.com\n"
    "app.message=Welcome ${app.docs}\n");
  REQUIRE(propertiesHighlight != nullptr);
  CHECK(styleAtColumn(propertiesHighlight->lines[0], 0) == kKeyword);
  CHECK(styleAtColumn(propertiesHighlight->lines[0], 9) == kUrl);
  CHECK(styleAtColumn(propertiesHighlight->lines[1], 22) == kVariable);

  SharedPtr<TextAnalyzer> env = engine->createAnalyzerByName("env");
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

  SharedPtr<TextAnalyzer> protobuf = engine->createAnalyzerByName("protobuf");
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

  SharedPtr<TextAnalyzer> graphql = engine->createAnalyzerByName("graphql");
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

  SharedPtr<TextAnalyzer> hcl = engine->createAnalyzerByName("hcl");
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

  SharedPtr<TextAnalyzer> terraform = engine->createAnalyzerByName("terraform");
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
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTypescriptSyntaxPath));
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

  SharedPtr<TextAnalyzer> vue = engine->createAnalyzerByName("vue");
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

  SharedPtr<TextAnalyzer> svelte = engine->createAnalyzerByName("svelte");
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

  SharedPtr<TextAnalyzer> nginx = engine->createAnalyzerByName("nginx");
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

  SharedPtr<TextAnalyzer> gitignore = engine->createAnalyzerByName("gitignore");
  REQUIRE(gitignore != nullptr);
  SharedPtr<DocumentHighlight> gitignoreHighlight = gitignore->analyzeText(
    "!/dist/keep.txt\n"
    "**/node_modules/\n");
  REQUIRE(gitignoreHighlight != nullptr);
  CHECK(styleAtColumn(gitignoreHighlight->lines[0], 0) == kAnnotation);
  CHECK(styleAtColumn(gitignoreHighlight->lines[0], 2) == kSelector);
  CHECK(styleAtColumn(gitignoreHighlight->lines[1], 0) == kBuiltin);
  CHECK(styleAtColumn(gitignoreHighlight->lines[1], 3) == kSelector);

  SharedPtr<TextAnalyzer> diff = engine->createAnalyzerByName("diff");
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

  SharedPtr<TextAnalyzer> ruby = engine->createAnalyzerByName("ruby");
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
