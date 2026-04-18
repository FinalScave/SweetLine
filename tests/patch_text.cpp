#include <catch2/catch_amalgamated.hpp>
#include "foundation.h"

using namespace NS_SWEETLINE;

const char* text = R"(
行1: 你好
行2: World
行3: 结束)";

TEST_CASE("Patch Text") {
  Document document("test.txt", text);
  REQUIRE(document.getLineCount() == 4);
  REQUIRE(document.getLine(0).text.empty());
  REQUIRE(document.getLine(1).text == "行1: 你好");
  REQUIRE(document.getLine(2).text == "行2: World");
  REQUIRE(document.getLine(3).text == "行3: 结束");
  REQUIRE(document.totalChars() == 25);
  REQUIRE(document.charIndexOfLine(0) == 0);
  REQUIRE(document.charIndexOfLine(1) == 1);
  REQUIRE(document.charIndexOfLine(2) == 8);
  REQUIRE(document.charIndexOfLine(3) == 18);
  REQUIRE(document.charIndexToPosition(8) == TextPosition{2, 0, 8});
  REQUIRE(document.charIndexToPosition(18) == TextPosition{3, 0, 18});

  // 单行更新：替换 "你好" 为 "您不好"
  TextRange range = {{1, 4}, {1, 6}};
  PatchResult single_line_result = document.patch(range, "您不好");
  REQUIRE(single_line_result.line_delta == 0);
  REQUIRE(single_line_result.char_delta == 1);
  REQUIRE(document.getLine(1).text == "行1: 您不好");
  REQUIRE(document.totalChars() == 26);
  REQUIRE(document.charIndexOfLine(2) == 9);
  REQUIRE(document.charIndexOfLine(3) == 19);

  // 跨行更新：替换 "World\n行3" 为 "宇宙\n最后一行"
  range = {{2, 4}, {3, 2}};
  PatchResult multi_line_result = document.patch(range, "宇宙\n最后一行");
  REQUIRE(multi_line_result.line_delta == 0);
  REQUIRE(multi_line_result.char_delta == -1);
  REQUIRE(document.getLineCount() == 4);
  REQUIRE(document.getLine(2).text == "行2: 宇宙");
  REQUIRE(document.getLine(3).text == "最后一行: 结束");
  REQUIRE(document.totalChars() == 25);
  REQUIRE(document.charIndexOfLine(2) == 9);
  REQUIRE(document.charIndexOfLine(3) == 16);
  REQUIRE(document.charIndexToPosition(16) == TextPosition{3, 0, 16});

  // 插入文本
  TextPosition position = {2, 1};
  document.insert(position, "=====");
  REQUIRE(document.getLine(2).text == "行=====2: 宇宙");
  REQUIRE(document.totalChars() == 30);
  REQUIRE(document.charIndexOfLine(3) == 21);

  // 删除文本
  range = {{1, 0}, {2, 9}};
  document.remove(range);
  REQUIRE(document.getLineCount() == 3);
  REQUIRE(document.getLine(0).text.empty());
  REQUIRE(document.getLine(1).text == "宇宙");
  REQUIRE(document.getLine(2).text == "最后一行: 结束");
  REQUIRE(document.totalChars() == 13);
  REQUIRE(document.charIndexOfLine(0) == 0);
  REQUIRE(document.charIndexOfLine(1) == 1);
  REQUIRE(document.charIndexOfLine(2) == 4);
  REQUIRE(document.charIndexToPosition(4) == TextPosition{2, 0, 4});
}

TEST_CASE("Append Text Keeps Cached Indices In Sync") {
  Document document("test.txt", "第一行");

  PatchResult append_result = document.appendText("\n第二行");
  REQUIRE(append_result.line_delta == 1);
  REQUIRE(append_result.char_delta == 4);
  REQUIRE(document.getLineCount() == 2);
  REQUIRE(document.getLine(0).text == "第一行");
  REQUIRE(document.getLine(1).text == "第二行");
  REQUIRE(document.totalChars() == 8);
  REQUIRE(document.charIndexOfLine(0) == 0);
  REQUIRE(document.charIndexOfLine(1) == 4);
  REQUIRE(document.charIndexToPosition(4) == TextPosition{1, 0, 4});
}

TEST_CASE("Patch Benchmark") {
  BENCHMARK("Patch Performance") {
    Document document("test.txt", text);
    TextRange range = {{1, 0}, {1, 1}};
    document.patch(range, "您");
  };
}
