#include <catch2/catch_amalgamated.hpp>
#include "sweetline/c_sweetline.h"
#include "sweetline/highlight.h"
#include "test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  U8String makeBracketSyntax() {
    return R"JSON({
  "name": "bracketToy",
  "fileSuffixes": [".bt"],
  "states": {
    "default": [
      { "pattern": "//[^\\n]*", "style": "comment" },
      { "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"", "style": "string" },
      { "pattern": "[()\\[\\]{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": {
    "skips": [
      { "kind": "lineComment", "start": "//" },
      { "kind": "string", "start": "\"", "end": "\"", "escape": "\\" }
    ],
    "rules": [
      { "kind": "delimiter", "start": "{", "end": "}" }
    ]
  },
  "bracketRules": {
    "pairs": [
      { "start": "(", "end": ")" },
      { "start": "[", "end": "]" },
      { "start": "{", "end": "}" }
    ]
  }
})JSON";
  }

  U8String makeMultiCharBracketSyntax() {
    return R"JSON({
  "name": "multiCharBracketToy",
  "fileSuffixes": [".mcbt"],
  "states": {
    "default": [
      { "pattern": ".", "style": "text" }
    ]
  },
  "bracketRules": {
    "pairs": [
      { "start": "<%", "end": "%>" },
      { "start": "<", "end": "%%>" },
      { "start": "a", "end": "xy" },
      { "start": "bbb", "end": "x" }
    ]
  }
})JSON";
  }

  SharedPtr<HighlightEngine> makeBracketEngine() {
    SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
    REQUIRE_NOTHROW(engine->compileSyntaxFromJson(makeBracketSyntax()));
    return engine;
  }
}

TEST_CASE("BracketRules match nested pairs and inherit scope skips") {
  SharedPtr<HighlightEngine> engine = makeBracketEngine();
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("bracketToy");
  REQUIRE(analyzer != nullptr);

  SharedPtr<BracketPairResult> result = analyzer->analyzeBracketPairs(
    "{\n"
    "  \"[ignored]\" // )\n"
    "  [()]\n"
    "}");
  REQUIRE(result != nullptr);
  REQUIRE(result->lines.size() == 4);

  REQUIRE(result->lines[0].tokens.size() == 1);
  const BracketToken& open_brace = result->lines[0].tokens[0];
  CHECK(open_brace.kind == BracketTokenKind::OPEN);
  CHECK(open_brace.match_state == BracketMatchState::MATCHED);
  CHECK(open_brace.partner_range.start.line == 3);
  CHECK(open_brace.partner_range.start.column == 0);

  CHECK(result->lines[1].tokens.empty());

  REQUIRE(result->lines[2].tokens.size() == 4);
  const BracketToken& open_square = result->lines[2].tokens[0];
  const BracketToken& open_paren = result->lines[2].tokens[1];
  const BracketToken& close_paren = result->lines[2].tokens[2];
  const BracketToken& close_square = result->lines[2].tokens[3];
  CHECK(open_square.range.start.column == 2);
  CHECK(open_square.partner_range.start.column == 5);
  CHECK(open_paren.range.start.column == 3);
  CHECK(open_paren.partner_range.start.column == 4);
  CHECK(close_paren.kind == BracketTokenKind::CLOSE);
  CHECK(close_paren.partner_range.start.column == 3);
  CHECK(close_square.match_state == BracketMatchState::MATCHED);
  CHECK(close_square.partner_range.start.column == 2);
}

TEST_CASE("BracketRules sort opening and closing markers independently") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(makeMultiCharBracketSyntax()));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("multiCharBracketToy");
  REQUIRE(analyzer != nullptr);

  SharedPtr<BracketPairResult> open_result = analyzer->analyzeBracketPairs("<%%>");
  REQUIRE(open_result != nullptr);
  REQUIRE(open_result->lines.size() == 1);
  REQUIRE(open_result->lines[0].tokens.size() == 2);
  CHECK(open_result->lines[0].tokens[0].range.start.column == 0);
  CHECK(open_result->lines[0].tokens[0].range.end.column == 2);
  CHECK(open_result->lines[0].tokens[0].match_state == BracketMatchState::MATCHED);
  CHECK(open_result->lines[0].tokens[1].range.start.column == 2);
  CHECK(open_result->lines[0].tokens[1].range.end.column == 4);
  CHECK(open_result->lines[0].tokens[1].match_state == BracketMatchState::MATCHED);

  SharedPtr<BracketPairResult> close_result = analyzer->analyzeBracketPairs("axy");
  REQUIRE(close_result != nullptr);
  REQUIRE(close_result->lines.size() == 1);
  REQUIRE(close_result->lines[0].tokens.size() == 2);
  CHECK(close_result->lines[0].tokens[0].range.start.column == 0);
  CHECK(close_result->lines[0].tokens[0].match_state == BracketMatchState::MATCHED);
  CHECK(close_result->lines[0].tokens[1].range.start.column == 1);
  CHECK(close_result->lines[0].tokens[1].range.end.column == 3);
  CHECK(close_result->lines[0].tokens[1].match_state == BracketMatchState::MATCHED);
}

TEST_CASE("BracketRules line range can match partners outside the visible range") {
  SharedPtr<HighlightEngine> engine = makeBracketEngine();
  SharedPtr<Document> document = makeSharedPtr<Document>("example.bt", "{\n  [\n    call()\n  ]\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<BracketPairResult> result = analyzer->analyzeBracketPairsInLineRange({3, 1});
  REQUIRE(result != nullptr);
  CHECK(result->start_line == 3);
  CHECK(result->total_line_count == 5);
  REQUIRE(result->lines.size() == 1);
  REQUIRE(result->lines[0].tokens.size() == 1);

  const BracketToken& close_square = result->lines[0].tokens[0];
  CHECK(close_square.kind == BracketTokenKind::CLOSE);
  CHECK(close_square.match_state == BracketMatchState::MATCHED);
  CHECK(close_square.range.start.line == 3);
  CHECK(close_square.range.start.column == 2);
  CHECK(close_square.partner_range.start.line == 1);
  CHECK(close_square.partner_range.start.column == 2);
}

TEST_CASE("BracketRules reports unmatched and unknown open brackets") {
  SharedPtr<HighlightEngine> engine = makeBracketEngine();
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("bracketToy");
  REQUIRE(analyzer != nullptr);

  SharedPtr<BracketPairResult> full_result = analyzer->analyzeBracketPairs("{\n  value");
  REQUIRE(full_result != nullptr);
  REQUIRE(full_result->lines.size() == 2);
  REQUIRE(full_result->lines[0].tokens.size() == 1);
  CHECK(full_result->lines[0].tokens[0].match_state == BracketMatchState::UNMATCHED);

  U8String long_text = "{\n";
  for (int i = 0; i < 140; ++i) {
    long_text += "  value\n";
  }
  SharedPtr<Document> document = makeSharedPtr<Document>("long.bt", long_text);
  SharedPtr<DocumentAnalyzer> document_analyzer = engine->loadDocument(document);
  REQUIRE(document_analyzer != nullptr);

  SharedPtr<BracketPairResult> visible_result = document_analyzer->analyzeBracketPairsInLineRange({0, 1});
  REQUIRE(visible_result != nullptr);
  REQUIRE(visible_result->lines.size() == 1);
  REQUIRE(visible_result->lines[0].tokens.size() == 1);
  CHECK(visible_result->lines[0].tokens[0].match_state == BracketMatchState::UNKNOWN);
}

TEST_CASE("C API bracket pair buffer keeps documented field order") {
  sl_engine_handle_t engine = sl_create_engine(true, false, 4);
  REQUIRE(engine != nullptr);
  sl_syntax_error_t error = sl_engine_compile_json(engine, makeBracketSyntax().c_str());
  REQUIRE(error.err_code == SL_OK);

  sl_analyzer_handle_t analyzer = sl_engine_create_text_analyzer(engine, "bracketToy");
  REQUIRE(analyzer != nullptr);
  int32_t* buffer = sl_text_analyze_bracket_pairs(analyzer, "()");
  REQUIRE(buffer != nullptr);

  CHECK(buffer[0] == 1);
  CHECK(buffer[1] == 10);
  CHECK(buffer[2] == 1);
  CHECK(buffer[3] == 2);

  CHECK(buffer[4] == 0);
  CHECK(buffer[5] == 1);
  CHECK(buffer[6] == 0);
  CHECK(buffer[7] == 0);
  CHECK(buffer[8] == 0);
  CHECK(buffer[9] == 0);
  CHECK(buffer[10] == 0);
  CHECK(buffer[11] == 1);
  CHECK(buffer[12] == 1);
  CHECK(buffer[13] == 1);

  CHECK(buffer[14] == 1);
  CHECK(buffer[15] == 1);
  CHECK(buffer[16] == 1);
  CHECK(buffer[17] == 0);
  CHECK(buffer[18] == 1);
  CHECK(buffer[19] == 0);
  CHECK(buffer[20] == 0);
  CHECK(buffer[21] == 0);
  CHECK(buffer[22] == 1);
  CHECK(buffer[23] == 0);

  sl_free_buffer(buffer);
  CHECK(sl_free_engine(engine) == SL_OK);
}
