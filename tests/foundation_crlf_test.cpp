#include <catch2/catch_amalgamated.hpp>
#include "sweetline/foundation.h"

using namespace NS_SWEETLINE;

TEST_CASE("CRLF char indexes map to positions without gaps") {
  Document document("test.txt", "a\r\nb");

  REQUIRE(document.totalChars() == 4);
  REQUIRE(document.charIndexOfLine(0) == 0);
  REQUIRE(document.charIndexOfLine(1) == 3);

  REQUIRE(document.charIndexToPosition(0) == TextPosition{0, 0, 0});
  REQUIRE(document.charIndexToPosition(1) == TextPosition{0, 1, 1});
  REQUIRE(document.charIndexToPosition(2) == TextPosition{0, 2, 2});
  REQUIRE(document.charIndexToPosition(3) == TextPosition{1, 0, 3});
  REQUIRE_THROWS_AS(document.charIndexToPosition(4), std::out_of_range);
}
