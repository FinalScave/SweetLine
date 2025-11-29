#include <iostream>
#include <catch2/catch_amalgamated.hpp>
#include "foundation.h"

using namespace NS_SWEETLINE;

const char* text = R"(
行1: 你好
行2: World
行3: 结束)";

TEST_CASE("Patch Text") {
  Document document("test.txt", text);

  std::cout << "原始文本:" << std::endl;
  std::cout << document.getText() << std::endl << std::endl;

  // 单行更新：替换 "你好" 为 "您不好"
  TextRange range = {{1, 4}, {1, 6}};
  document.patch(range, "您不好");
  std::cout << "单行替换后:" << std::endl;
  std::cout << document.getText() << std::endl << std::endl;

  // 跨行更新：替换 "World\n行3" 为 "宇宙\n最后一行"
  range = {{2, 4}, {3, 2}};
  document.patch(range, "宇宙\n最后一行");
  std::cout << "跨行替换后:" << std::endl;
  std::cout << document.getText() << std::endl << std::endl;

  // 插入文本
  TextPosition position = {2, 1};
  document.insert(position, "=====");
  std::cout << "插入后:" << std::endl;
  std::cout << document.getText() << std::endl << std::endl;

  // 删除文本
  range = {{1, 0}, {2, 9}};
  document.remove(range);
  std::cout << "删除后:" << std::endl;
  std::cout << document.getText() << std::endl;
}

TEST_CASE("Patch Benchmark") {
  BENCHMARK("Patch Performance") {
    Document document("test.txt", text);
    TextRange range = {{1, 0}, {1, 1}};
    document.patch(range, "H");
  };
}
