#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "../test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  static const char* kFsharpSyntaxPath = SYNTAX_DIR"/fsharp.json";
}

TEST_CASE("F# keeps keywords, types, let bindings, attributes, and preprocessor distinct") {
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

TEST_CASE("F# indent guides cover offside blocks and delimiters") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kFsharpSyntaxPath));

  SharedPtr<Document> document = makeSharedPtr<Document>("example.fs",
    "module Report\n"
    "\n"
    "type Entry =\n"
    "    { Name: string\n"
    "      Score: int\n"
    "      Tags: string list }\n"
    "\n"
    "let classify entry =\n"
    "    match entry.Score with\n"
    "    | score when score >= 90 -> \"excellent\"\n"
    "    | score when score >= 70 -> \"steady\"\n"
    "    | _ -> \"needs-work\"\n"
    "\n"
    "let summarize entries =\n"
    "    entries\n"
    "    |> List.filter (fun entry ->\n"
    "        entry.Tags\n"
    "        |> List.exists (fun tag -> tag = \"active\"))\n"
    "    |> List.groupBy classify\n"
    "    |> List.map (fun (bucket, items) ->\n"
    "        let names =\n"
    "            items\n"
    "            |> List.map _.Name\n"
    "            |> String.concat \", \"\n"
    "\n"
    "        $\"{bucket}: {names}\")");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  CHECK(findGuideByPosition(*result, 0, 2, 7) != nullptr);
  CHECK(findGuideByPosition(*result, 4, 3, 5) != nullptr);
  CHECK(findGuideByPosition(*result, 0, 7, 13) != nullptr);
  CHECK(findGuideByPosition(*result, 0, 13, 25) != nullptr);
  CHECK(findGuideByPosition(*result, 4, 15, 18) != nullptr);
  CHECK(findGuideByPosition(*result, 4, 19, 25) != nullptr);
  CHECK(findGuideByPosition(*result, 8, 20, 25) != nullptr);
}
