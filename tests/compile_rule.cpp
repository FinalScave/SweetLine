#include <iostream>
#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

Ptr<SyntaxRuleCompiler> makeSyntaxRuleCompiler() {
  Ptr<StyleMapping> mapping = MAKE_PTR<StyleMapping>();
  mapping->registerStyleName("keyword", 1);
  mapping->registerStyleName("string", 2);
  mapping->registerStyleName("number", 3);
  mapping->registerStyleName("comment", 4);
  mapping->registerStyleName("class", 5);
  mapping->registerStyleName("method", 6);
  mapping->registerStyleName("variable", 7);
  mapping->registerStyleName("punctuation", 8);
  Ptr<SyntaxRuleCompiler> compiler = MAKE_PTR<SyntaxRuleCompiler>(mapping);
  return compiler;
}

TEST_CASE("Compile Rule") {
  try {
    Ptr<SyntaxRuleCompiler> compiler = makeSyntaxRuleCompiler();
    compiler->compileSyntaxFromFile(SYNTAX_DIR"/java-syntax.json");
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
  }
}

TEST_CASE("Compile Rule Benchmark") {
  Ptr<SyntaxRuleCompiler> compiler = makeSyntaxRuleCompiler();
  String text = FileUtil::readString(SYNTAX_DIR"/java-syntax.json");
  BENCHMARK("Compile Rule Performance") {
    compiler->compileSyntaxFromJson(text);
  };
}
