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

  // 单行更新：替换 "你好" 为 "您不好"
  TextRange range = {{1, 4}, {1, 6}};
  REQUIRE(document.patch(range, "您不好") == 0);
  REQUIRE(document.getLine(1).text == "行1: 您不好");

  // 跨行更新：替换 "World\n行3" 为 "宇宙\n最后一行"
  range = {{2, 4}, {3, 2}};
  REQUIRE(document.patch(range, "宇宙\n最后一行") == 1);
  REQUIRE(document.getLineCount() == 4);
  REQUIRE(document.getLine(2).text == "行2: 宇宙");
  REQUIRE(document.getLine(3).text == "最后一行: 结束");

  // 插入文本
  TextPosition position = {2, 1};
  document.insert(position, "=====");
  REQUIRE(document.getLine(2).text == "行=====2: 宇宙");

  // 删除文本
  range = {{1, 0}, {2, 9}};
  document.remove(range);
  REQUIRE(document.getLineCount() == 3);
  REQUIRE(document.getLine(0).text.empty());
  REQUIRE(document.getLine(1).text == "宇宙");
  REQUIRE(document.getLine(2).text == "最后一行: 结束");
}

TEST_CASE("Patch Benchmark") {
  BENCHMARK("Patch Performance") {
    Document document("test.txt", text);
    TextRange range = {{1, 0}, {1, 1}};
    document.patch(range, "H");
  };
}
