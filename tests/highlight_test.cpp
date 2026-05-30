#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "test_helpers.h"

using namespace NS_SWEETLINE;
using namespace NS_SWEETLINE_TEST;

TEST_CASE("Host syntaxes keep every embedded importSyntax state active") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(R"JSON(
{
  "name": "embedded-alpha",
  "fileSuffixes": [".alpha"],
  "states": {
    "default": [
      { "pattern": "\\balpha\\b", "style": "keyword" }
    ]
  }
}
)JSON"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(R"JSON(
{
  "name": "embedded-beta",
  "fileSuffixes": [".beta"],
  "states": {
    "default": [
      { "pattern": "\\bbeta\\b", "style": "keyword" }
    ]
  }
}
)JSON"));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(R"JSON(
{
  "name": "embedded-host",
  "fileSuffixes": [".embed"],
  "states": {
    "default": [
      {
        "pattern": "^(```)(alpha)$",
        "styles": [1, "punctuation", 2, "annotation"],
        "state": "alphaBlock"
      },
      {
        "pattern": "^(```)(beta)$",
        "styles": [1, "punctuation", 2, "annotation"],
        "state": "betaBlock"
      }
    ],
    "alphaBlock": [
      {
        "pattern": "^(```)$",
        "styles": [1, "punctuation"],
        "state": "default"
      },
      {
        "importSyntax": "embedded-alpha"
      }
    ],
    "betaBlock": [
      {
        "pattern": "^(```)$",
        "styles": [1, "punctuation"],
        "state": "default"
      },
      {
        "importSyntax": "embedded-beta"
      }
    ]
  }
}
)JSON"));

  constexpr int32_t kKeyword = 1;

  SharedPtr<TextAnalyzer> host = engine->createAnalyzerBySyntaxName("embedded-host");
  REQUIRE(host != nullptr);
  SharedPtr<DocumentHighlight> highlight = host->analyzeText(
    "```alpha\n"
    "alpha\n"
    "```\n"
    "```beta\n"
    "beta\n"
    "```\n");
  REQUIRE(highlight != nullptr);
  CHECK(styleAtColumn(highlight->lines[1], 0) == kKeyword);
  CHECK(styleAtColumn(highlight->lines[4], 0) == kKeyword);
}
