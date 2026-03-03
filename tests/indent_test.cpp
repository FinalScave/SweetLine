#include <iostream>
#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

namespace {
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kJavaExampleFilePath = TESTS_DIR"/files/exampla.java";

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
  try {
    engine->compileSyntaxFromFile(kJavaSyntaxPath);
  } catch (SyntaxRuleParseError& error) {
    std::cerr << error.what() << ": " << error.message() << std::endl;
    return;
  }
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  SharedPtr<Document> document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  SharedPtr<IndentGuideResult> res = analyzer->analyzeIndentGuides();
  for (const IndentGuideLine& guide_line : res->guide_lines) {
    std::cout << "startLine: " << guide_line.start_line << ", endLine: " << guide_line.end_line << ", column: " << guide_line.column << std::endl;
  }
}
