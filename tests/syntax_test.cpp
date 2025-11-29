#include <unordered_set>
#include <vector>
#include <catch2/catch_amalgamated.hpp>
#include "highlight.h"
#include "util.h"

using namespace NS_SWEETLINE;

namespace {
  SharedPtr<HighlightEngine> makeTestHighlightEngine(const HighlightConfig& config = HighlightConfig::kDefault) {
    SharedPtr<HighlightEngine> engine = makeSharedPtr<HighlightEngine>(config);
    engine->registerStyleName("keyword", 1);
    engine->registerStyleName("string", 2);
    engine->registerStyleName("number", 3);
    engine->registerStyleName("comment", 4);
    engine->registerStyleName("class", 5);
    engine->registerStyleName("method", 6);
    engine->registerStyleName("variable", 7);
    engine->registerStyleName("punctuation", 8);
    engine->registerStyleName("annotation", 9);
    engine->registerStyleName("builtin", 10);
    engine->registerStyleName("preprocessor", 11);
    engine->registerStyleName("macro", 12);
    engine->registerStyleName("property", 13);
    engine->registerStyleName("lifetime", 14);
    engine->registerStyleName("selector", 15);
    return engine;
  }

  U8String syntaxPath(const U8String& file_name) {
    return U8String(SYNTAX_DIR) + "/" + file_name;
  }
}

TEST_CASE("Compile built-in syntaxes from syntaxes directory") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> files = {
    "c.json", "cpp.json", "csharp.json", "dart.json", "go.json", "groovy.json", "iapp.json",
    "java.json", "javascript.json", "json-sweetline.json", "kotlin.json", "lua.json", "objc.json", "php.json",
    "powershell.json", "python.json", "rust.json", "shell.json", "sql.json", "swift.json", "tiecode.json",
    "toml.json", "typescript.json", "vb.json", "wenyan.json", "xml.json", "yaml.json", "html.json",
    "scala.json",
    "java-inlineStyle.json", "tiecode-inlineStyle.json", "yaml(non zero width).json"
  };

  for (const U8String& file_name : files) {
    CAPTURE(file_name);
    SharedPtr<SyntaxRule> rule;
    REQUIRE_NOTHROW(rule = engine->compileSyntaxFromFile(syntaxPath(file_name)));
    REQUIRE(rule != nullptr);
    REQUIRE(rule->containsRule(SyntaxRule::kDefaultStateId));
    REQUIRE_FALSE(rule->name.empty());
  }

  // markdown 依赖 importSyntax="c++"，此处注入一个 "name=c++" 的别名语法后再编译 markdown
  U8String cpp_alias_json = FileUtil::readString(syntaxPath("cpp.json"));
  REQUIRE_FALSE(cpp_alias_json.empty());
  REQUIRE(StrUtil::replaceFirst(cpp_alias_json, "\"name\": \"cpp\"", "\"name\": \"c++\""));
  SharedPtr<SyntaxRule> cpp_alias;
  REQUIRE_NOTHROW(cpp_alias = engine->compileSyntaxFromJson(cpp_alias_json));
  REQUIRE(cpp_alias != nullptr);

  SharedPtr<SyntaxRule> markdown_rule;
  REQUIRE_NOTHROW(markdown_rule = engine->compileSyntaxFromFile(syntaxPath("markdown.json")));
  REQUIRE(markdown_rule != nullptr);
  REQUIRE(markdown_rule->containsRule(SyntaxRule::kDefaultStateId));
}

TEST_CASE("Create analyzers by extension for sample files") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> files = {
    "c.json", "cpp.json", "csharp.json", "dart.json", "go.json", "groovy.json", "iapp.json",
    "java.json", "javascript.json", "json-sweetline.json", "kotlin.json", "lua.json", "objc.json", "php.json",
    "powershell.json", "python.json", "rust.json", "shell.json", "sql.json", "swift.json", "tiecode.json",
    "toml.json", "typescript.json", "vb.json", "wenyan.json", "xml.json", "yaml.json", "html.json",
    "scala.json"
  };
  for (const U8String& file_name : files) {
    CAPTURE(file_name);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath(file_name)));
  }

  U8String cpp_alias_json = FileUtil::readString(syntaxPath("cpp.json"));
  REQUIRE_FALSE(cpp_alias_json.empty());
  REQUIRE(StrUtil::replaceFirst(cpp_alias_json, "\"name\": \"cpp\"", "\"name\": \"c++\""));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(cpp_alias_json));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath("markdown.json")));

  const List<U8String> sample_files = {
    TESTS_DIR"/files/example.c",
    TESTS_DIR"/files/example.cpp",
    TESTS_DIR"/files/example.cs",
    TESTS_DIR"/files/example.dart",
    TESTS_DIR"/files/example.go",
    TESTS_DIR"/files/example.groovy",
    TESTS_DIR"/files/example.html",
    TESTS_DIR"/files/example.java",
    TESTS_DIR"/files/example.js",
    TESTS_DIR"/files/example.kt",
    TESTS_DIR"/files/example.lua",
    TESTS_DIR"/files/example.m",
    TESTS_DIR"/files/example.md",
    TESTS_DIR"/files/example.myu",
    TESTS_DIR"/files/example.php",
    TESTS_DIR"/files/example.ps1",
    TESTS_DIR"/files/example.py",
    TESTS_DIR"/files/example.rs",
    TESTS_DIR"/files/example.scala",
    TESTS_DIR"/files/example.sh",
    TESTS_DIR"/files/example.sql",
    TESTS_DIR"/files/example.swift",
    TESTS_DIR"/files/example.t",
    TESTS_DIR"/files/example.toml",
    TESTS_DIR"/files/example.ts",
    TESTS_DIR"/files/example.vb",
    TESTS_DIR"/files/example.wenyan",
    TESTS_DIR"/files/example.xml",
    TESTS_DIR"/files/example.yaml"
  };

  for (const U8String& file_path : sample_files) {
    CAPTURE(file_path);
    U8String extension = FileUtil::getExtension(file_path);
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByExtension(extension);
    REQUIRE(analyzer != nullptr);
  }
}

TEST_CASE("importSyntax merges rules from source syntax") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String source_syntax = R"({
  "name": "sourceLang",
  "fileExtensions": [".src"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String host_syntax = R"({
  "name": "hostLang",
  "fileExtensions": [".host"],
  "states": {
    "default": [
      { "pattern": "\\b(bar)\\b", "styles": [1, "number"] },
      { "importSyntax": "sourceLang" }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(source_syntax));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(host_syntax));

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("hostLang");
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("foo bar");
  REQUIRE(highlight != nullptr);

  std::unordered_set<int32_t> style_ids;
  for (const LineHighlight& line : highlight->lines) {
    for (const TokenSpan& span : line.spans) {
      style_ids.insert(span.style_id);
    }
  }
  REQUIRE(style_ids.find(1) != style_ids.end()); // keyword from imported syntax
  REQUIRE(style_ids.find(3) != style_ids.end()); // number from host syntax
}

TEST_CASE("importSyntax with #ifdef is controlled by macros") {
  const U8String source_syntax = R"({
  "name": "sourceLang",
  "fileExtensions": [".src"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String host_syntax = R"({
  "name": "hostIfdef",
  "fileExtensions": [".hid"],
  "states": {
    "default": [
      { "pattern": "\\b(bar)\\b", "styles": [1, "number"] },
      { "importSyntax": "sourceLang", "#ifdef": "FEATURE_X" }
    ]
  }
})";

  SECTION("Without macro only host rules are active") {
    SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
    REQUIRE_NOTHROW(engine->compileSyntaxFromJson(source_syntax));
    REQUIRE_NOTHROW(engine->compileSyntaxFromJson(host_syntax));
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("hostIfdef");
    REQUIRE(analyzer != nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("foo bar");
    REQUIRE(highlight != nullptr);

    std::unordered_set<int32_t> style_ids;
    for (const LineHighlight& line : highlight->lines) {
      for (const TokenSpan& span : line.spans) {
        style_ids.insert(span.style_id);
      }
    }
    REQUIRE(style_ids.find(3) != style_ids.end()); // number from host syntax
    REQUIRE(style_ids.find(1) == style_ids.end()); // keyword from source syntax should be absent
  }

  SECTION("With macro imported rules are active") {
    SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
    engine->defineMacro("FEATURE_X");
    REQUIRE_NOTHROW(engine->compileSyntaxFromJson(source_syntax));
    REQUIRE_NOTHROW(engine->compileSyntaxFromJson(host_syntax));
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("hostIfdef");
    REQUIRE(analyzer != nullptr);
    SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("foo bar");
    REQUIRE(highlight != nullptr);

    std::unordered_set<int32_t> style_ids;
    for (const LineHighlight& line : highlight->lines) {
      for (const TokenSpan& span : line.spans) {
        style_ids.insert(span.style_id);
      }
    }
    REQUIRE(style_ids.find(3) != style_ids.end()); // number from host syntax
    REQUIRE(style_ids.find(1) != style_ids.end()); // keyword from source syntax should be present
  }
}

TEST_CASE("Inline-style syntax produces inline style spans") {
  HighlightConfig config;
  config.inline_style = true;
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine(config);

  SharedPtr<SyntaxRule> rule;
  REQUIRE_NOTHROW(rule = engine->compileSyntaxFromFile(syntaxPath("tiecode-inlineStyle.json")));
  REQUIRE(rule != nullptr);

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName("tiecode");
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("类 示例\n");
  REQUIRE(highlight != nullptr);
  REQUIRE(highlight->spanCount() > 0);

  bool has_inline_style = false;
  for (const LineHighlight& line : highlight->lines) {
    for (const TokenSpan& span : line.spans) {
      if (span.inline_style.foreground != 0 || span.inline_style.background != 0
        || span.inline_style.is_bold || span.inline_style.is_italic || span.inline_style.is_strikethrough) {
        has_inline_style = true;
      }
    }
  }
  REQUIRE(has_inline_style);
}
