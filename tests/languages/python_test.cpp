#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "../test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

namespace {
  static const char* kPythonSyntaxPath = SYNTAX_DIR"/python.json";
}

TEST_CASE("Python indentStart guides use block header columns") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kPythonSyntaxPath));

  SharedPtr<Document> document = makeSharedPtr<Document>("example.py",
    "def collect_notes(root):\n"
    "    notes = []\n"
    "    for path in root.rglob(\"*.md\"):\n"
    "        if path.name.startswith(\"_\"):\n"
    "            continue\n"
    "    return notes\n"
    "print(collect_notes(\"docs\"))");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 3);

  const IndentGuideLine* function_guide = findGuideByColumn(*result, 0);
  const IndentGuideLine* loop_guide = findGuideByColumn(*result, 4);
  const IndentGuideLine* condition_guide = findGuideByColumn(*result, 8);
  REQUIRE(function_guide != nullptr);
  REQUIRE(loop_guide != nullptr);
  REQUIRE(condition_guide != nullptr);

  CHECK(function_guide->start_line == 0);
  CHECK(function_guide->end_line == 6);
  CHECK(loop_guide->start_line == 2);
  CHECK(loop_guide->end_line == 5);
  CHECK(condition_guide->start_line == 3);
  CHECK(condition_guide->end_line == 5);
}
