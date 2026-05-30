#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"
#include "sweetline/util.h"

using namespace NS_SWEETLINE;

namespace {
  static const char* kJavaSyntaxPath = SYNTAX_DIR"/java.json";
  static const char* kPythonSyntaxPath = SYNTAX_DIR"/python.json";
  static const char* kTiecodeSyntaxPath = SYNTAX_DIR"/tiecode.json";
  static const char* kJavaExampleFilePath = TESTS_DIR"/files/example.java";

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
    engine->registerStyleName("url", 16);
    return engine;
  }

  U8String makeBraceScopeSyntax(const U8String& name, const U8String& suffix) {
    return R"({
  "name": ")" + name + R"(",
  "fileSuffixes": [")" + suffix + R"("],
  "states": {
    "default": [
      { "pattern": "[{}]", "style": "punctuation" }
    ]
  },
  "scopeRules": {
    "skips": [],
    "rules": [
      { "kind": "delimiter", "start": "{", "end": "}" }
    ]
  }
})";
  }

  U8String makeIndentOnlySyntax(const U8String& name, const U8String& suffix) {
    return R"({
  "name": ")" + name + R"(",
  "fileSuffixes": [")" + suffix + R"("],
  "states": {
    "default": [
      { "pattern": "[^\\n]+", "style": "keyword" }
    ]
  }
})";
  }

  const IndentGuideLine* findGuideByColumn(const IndentGuideResult& result, int32_t column) {
    for (const IndentGuideLine& guide : result.guide_lines) {
      if (guide.column == column) {
        return &guide;
      }
    }
    return nullptr;
  }
}

TEST_CASE("Indent example.java") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kJavaSyntaxPath));
  U8String code_txt = FileUtil::readString(kJavaExampleFilePath);
  REQUIRE_FALSE(code_txt.empty());
  SharedPtr<Document> document = makeSharedPtr<Document>("example.java", code_txt);
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);
  SharedPtr<IndentGuideResult> res = analyzer->analyzeIndentGuides();
  REQUIRE(res != nullptr);
  REQUIRE_FALSE(res->guide_lines.empty());
  REQUIRE(res->line_states.size() == document->getLineCount());
}

TEST_CASE("ScopeRules ignores markers in string/comment without highlight") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "toy",
  "fileSuffixes": [".toy"],
  "states": {
    "default": [
      { "pattern": "//[^\\n]*", "style": "comment" },
      { "pattern": "\"(?:[^\"\\\\]|\\\\.)*\"", "style": "string" },
      { "pattern": "[{}]", "style": "punctuation" }
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
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.toy", "{\n  \"}\" // comment {\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);

  const IndentGuideLine& guide = result->guide_lines[0];
  CHECK(guide.start_line == 0);
  CHECK(guide.end_line == 2);
  CHECK(guide.nesting_level == 0);
  CHECK(guide.scope_rule_id == 0);
}

TEST_CASE("ScopeRules closes innermost block when end token is shared") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "luaLike",
  "fileSuffixes": [".ll"],
  "states": {
    "default": [
      { "pattern": "\\b(function|then|else|end)\\b", "style": "keyword" }
    ]
  },
  "scopeRules": {
    "skips": [],
    "rules": [
      { "kind": "word", "start": "function", "end": "end" },
      { "kind": "word", "start": "then", "end": "end", "branches": ["else"] }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.ll", "function\n  then\n  else\n  end\nend");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* outer = nullptr;
  const IndentGuideLine* inner = nullptr;
  for (const IndentGuideLine& line : result->guide_lines) {
    if (line.start_line == 0) {
      outer = &line;
    } else if (line.start_line == 1) {
      inner = &line;
    }
  }
  REQUIRE(outer != nullptr);
  REQUIRE(inner != nullptr);

  CHECK(outer->end_line == 4);
  CHECK(outer->nesting_level == 0);
  CHECK(inner->end_line == 3);
  CHECK(inner->nesting_level == 1);
  REQUIRE(inner->branches.size() == 1);
  CHECK(inner->branches[0].line == 2);
}

TEST_CASE("ScopeRules keeps nesting level for unclosed blocks at EOF") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeBraceScopeSyntax("brace", ".br");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.br", "{\n  {");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  bool has_level0 = false;
  bool has_level1 = false;
  for (const IndentGuideLine& line : result->guide_lines) {
    if (line.nesting_level == 0) {
      has_level0 = true;
    }
    if (line.nesting_level == 1) {
      has_level1 = true;
    }
    CHECK(line.end_line == 1);
  }
  CHECK(has_level0);
  CHECK(has_level1);
}

TEST_CASE("ScopeRules guide column uses min of start and end columns") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeBraceScopeSyntax("braceCol", ".bc");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.bc", "if (ok) {\n    run();\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].column == 0);
}

TEST_CASE("ScopeRules indentStart uses parent indent column") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "indentStartColumn",
  "fileSuffixes": [".isc"],
  "states": {
    "default": [
      { "pattern": "[:]", "style": "punctuation" },
      { "pattern": "[^\\n]+", "style": "keyword" }
    ]
  },
  "scopeRules": {
    "skips": [],
    "rules": [
      { "kind": "indentStart", "start": ":" }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>(
    "example.isc", "if ok:\n    run()\nend");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].column == 0);
  CHECK(result->guide_lines[0].start_line == 0);
  CHECK(result->guide_lines[0].end_line == 2);
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

TEST_CASE("Indentation fallback anchors guides to previous nonblank line") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeIndentOnlySyntax("indentOnly", ".io");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.io", "event\n    first\n    second\ndone");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);

  const IndentGuideLine& guide = result->guide_lines[0];
  CHECK(guide.column == 0);
  CHECK(guide.start_line == 0);
  CHECK(guide.end_line == 3);
  CHECK(guide.start_line + 1 == 1);
  CHECK(guide.end_line - 1 == 2);
  CHECK(guide.scope_rule_id == -1);
}

TEST_CASE("Indentation fallback uses parent indent columns") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeIndentOnlySyntax("indentColumns", ".ic");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>(
    "example.ic", "root\n  child\n      grand\n  sibling\ndone");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* outer = findGuideByColumn(*result, 0);
  const IndentGuideLine* inner = findGuideByColumn(*result, 2);
  REQUIRE(outer != nullptr);
  REQUIRE(inner != nullptr);
  CHECK(outer->start_line == 0);
  CHECK(outer->end_line == 4);
  CHECK(outer->start_line + 1 == 1);
  CHECK(outer->end_line - 1 == 3);
  CHECK(inner->start_line == 1);
  CHECK(inner->end_line == 3);
  CHECK(inner->start_line + 1 == 2);
  CHECK(inner->end_line - 1 == 2);
}

TEST_CASE("Indentation fallback reports parent columns for tabs") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeIndentOnlySyntax("indentTabs", ".it");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>(
    "example.it", "root\n\tchild\n\t\tgrand\n\tchild2\ndone");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* outer = findGuideByColumn(*result, 0);
  const IndentGuideLine* inner = findGuideByColumn(*result, 1);
  REQUIRE(outer != nullptr);
  REQUIRE(inner != nullptr);
  CHECK(outer->start_line == 0);
  CHECK(outer->end_line == 4);
  CHECK(inner->start_line == 1);
  CHECK(inner->end_line == 3);
}

TEST_CASE("Indentation fallback keeps guides through blank lines") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeIndentOnlySyntax("indentBlank", ".ib");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.ib", "root\n\n    child\n\nnext");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].start_line == 0);
  CHECK(result->guide_lines[0].end_line == 4);
  CHECK(result->guide_lines[0].start_line + 1 == 1);
  CHECK(result->guide_lines[0].end_line - 1 == 3);
}

TEST_CASE("Indentation fallback reports continuation for visible slices") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeIndentOnlySyntax("indentVisible", ".iv");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.iv", "root\n    child1\n    child2\ndone");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuidesInLineRange({1, 1});
  REQUIRE(result != nullptr);
  CHECK(result->start_line == 1);
  CHECK(result->total_line_count == 4);
  REQUIRE(result->line_states.size() == 1);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].column == 0);
  CHECK(result->guide_lines[0].start_line == 1);
  CHECK(result->guide_lines[0].end_line == 1);
  CHECK(result->guide_lines[0].continues_before);
  CHECK(result->guide_lines[0].continues_after);
}

TEST_CASE("Tiecode indentation fallback anchors event and method blocks") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(kTiecodeSyntaxPath));

  SharedPtr<Document> document = makeSharedPtr<Document>("example.t",
    "类 启动窗口:窗口\n"
    "    事件 启动窗口:创建完毕()\n"
    "        变量 网站2:文本 = \"http://***.**\"\n"
    "        提示网址1()\n"
    "    结束 事件\n"
    "结束 类");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  REQUIRE(result != nullptr);
  REQUIRE(result->guide_lines.size() == 2);

  const IndentGuideLine* class_guide = findGuideByColumn(*result, 0);
  const IndentGuideLine* event_guide = findGuideByColumn(*result, 4);
  REQUIRE(class_guide != nullptr);
  REQUIRE(event_guide != nullptr);
  CHECK(class_guide->start_line == 0);
  CHECK(class_guide->end_line == 5);
  CHECK(class_guide->start_line + 1 == 1);
  CHECK(class_guide->end_line - 1 == 4);
  CHECK(event_guide->start_line == 1);
  CHECK(event_guide->end_line == 4);
  CHECK(event_guide->start_line + 1 == 2);
  CHECK(event_guide->end_line - 1 == 3);
}

TEST_CASE("TextAnalyzer indent guides are independent from highlight analysis") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeBraceScopeSyntax("braceText", ".bt");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("braceText");
  REQUIRE(analyzer != nullptr);

  const U8String text = "{\n    x\n}";
  SharedPtr<IndentGuideResult> before = analyzer->analyzeIndentGuides(text);
  REQUIRE(before != nullptr);
  REQUIRE(before->guide_lines.size() == 1);

  REQUIRE(analyzer->analyzeText(text) != nullptr);

  SharedPtr<IndentGuideResult> after = analyzer->analyzeIndentGuides(text);
  REQUIRE(after != nullptr);
  CHECK(after->guide_lines == before->guide_lines);
  CHECK(after->line_states == before->line_states);
  CHECK(after->guide_lines[0].start_line == 0);
  CHECK(after->guide_lines[0].end_line == 2);
  CHECK(after->guide_lines[0].column == 0);
  CHECK(after->guide_lines[0].scope_rule_id == 0);
}

TEST_CASE("DocumentAnalyzer analyzes visible indent guides without highlight") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = makeBraceScopeSyntax("braceVisible", ".bv");

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  SharedPtr<Document> document = makeSharedPtr<Document>("example.bv", "{\n  a\n  b\n}");
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  REQUIRE(analyzer != nullptr);

  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuidesInLineRange({1, 2});
  REQUIRE(result != nullptr);
  CHECK(result->start_line == 1);
  CHECK(result->total_line_count == 4);
  REQUIRE(result->line_states.size() == 2);
  REQUIRE(result->guide_lines.size() == 1);
  CHECK(result->guide_lines[0].start_line == 1);
  CHECK(result->guide_lines[0].end_line == 2);
  CHECK(result->guide_lines[0].continues_before);
  CHECK(result->guide_lines[0].continues_after);
}
