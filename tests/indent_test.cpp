#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

namespace {
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kJavaExampleFilePath = TESTS_DIR"/files/example.java";

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

TEST_CASE("Indent example.java") {
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
  SharedPtr<IndentGuideResult> res = analyzer->analyzeIndentGuides();
  REQUIRE(res != nullptr);
  REQUIRE_FALSE(res->guide_lines.empty());
  REQUIRE(res->line_states.size() == document->getLineCount());
}

TEST_CASE("ScopeRules ignores markers in string/comment for style id mode") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "toy",
  "fileExtensions": [".toy"],
  "states": {
    "default": [
      { "pattern": "//[^\\n]*", "style": "comment" },
      { "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"", "style": "string" },
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": [
    { "start": "{", "end": "}" }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.toy", "{\n  \"}\" // comment {\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);

  const IndentGuideLine& guide = result->guide_lines[0];
  CHECK(guide.start_line == 0);
  CHECK(guide.end_line == 2);
  CHECK(guide.nesting_level == 0);
  CHECK(guide.scope_rule_id == 0);
}

TEST_CASE("ScopeRules closes innermost block when end token is shared") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "luaLike",
  "fileExtensions": [".ll"],
  "states": {
    "default": [
      { "pattern": "\\b(function|then|else|end)\\b", "style": "keyword" }
    ]
  },
  "scopeRules": [
    { "start": "function", "end": "end" },
    { "start": "then", "end": "end", "branches": ["else"] }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.ll", "function\n  then\n  else\n  end\nend");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* outer = nullptr;
  const IndentGuideLine* inner = nullptr;
  for (const IndentGuideLine& line : result->guide_lines) {
    if (line.start_line == 0) {
      outer = &line;
    } else if (line.start_line == 1) {
      inner = &line;
    }
  }
  REQUIRE(outer != nullptr);
  REQUIRE(inner != nullptr);

  CHECK(outer->end_line == 4);
  CHECK(outer->nesting_level == 0);
  CHECK(inner->end_line == 3);
  CHECK(inner->nesting_level == 1);
  REQUIRE(inner->branches.size() == 1);
  CHECK(inner->branches[0].line == 2);
}

TEST_CASE("ScopeRules keeps nesting level for unclosed blocks at EOF") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "brace",
  "fileExtensions": [".br"],
  "states": {
    "default": [
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": [
    { "start": "{", "end": "}" }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.br", "{\n  {");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  bool has_level0 = false;
  bool has_level1 = false;
  for (const IndentGuideLine& line : result->guide_lines) {
    if (line.nesting_level == 0) {
      has_level0 = true;
    }
    if (line.nesting_level == 1) {
      has_level1 = true;
    }
    CHECK(line.end_line == 1);
  }
  CHECK(has_level0);
  CHECK(has_level1);
}

TEST_CASE("ScopeRules guide column uses min of start and end columns") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "braceCol",
  "fileExtensions": [".bc"],
  "states": {
    "default": [
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": [
    { "start": "{", "end": "}" }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.bc", "if (ok) {\n    run();\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  REQUIRE(highlight != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].column == 0);
}

TEST_CASE("TextAnalyzer reuses cached highlight for scope indent guides") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "braceCached",
  "fileExtensions": [".bc2"],
  "states": {
    "default": [
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": [
    { "start": "{", "end": "}" }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("braceCached");
  REQUIRE(analyzer != nullptr);

  const U8String text = "{\n    x\n}";
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(text);
  REQUIRE(highlight != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides(text);
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].start_line == 0);
  CHECK(result->guide_lines[0].end_line == 2);
  CHECK(result->guide_lines[0].scope_rule_id == 0);
}

TEST_CASE("TextAnalyzer falls back to indentation guides without cached highlight") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "braceFallback",
  "fileExtensions": [".bf"],
  "states": {
    "default": [
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": [
    { "start": "{", "end": "}" }
  ]
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("braceFallback");
  REQUIRE(analyzer != nullptr);

  const U8String text = "{\n    x\n}";
  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides(text);
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].start_line == 1);
  CHECK(result->guide_lines[0].end_line == 1);
  CHECK(result->guide_lines[0].column == 4);
  CHECK(result->guide_lines[0].scope_rule_id == -1);
}
