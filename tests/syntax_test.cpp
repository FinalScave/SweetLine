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
    engine->registerStyleName("url", 16);
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
    "scala.json", "css.json", "scss.json", "less.json", "jsonc.json", "json5.json", "cmake.json",
    "dockerfile.json", "makefile.json", "properties.json", "env.json", "protobuf.json", "graphql.json",
    "nginx.json", "gitignore.json", "diff.json", "ruby.json", "hcl.json", "terraform.json", "vue.json",
    "svelte.json",
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

  SharedPtr<SyntaxRule> markdown_rule;
  REQUIRE_NOTHROW(markdown_rule = engine->compileSyntaxFromFile(syntaxPath("markdown.json")));
  REQUIRE(markdown_rule != nullptr);
  REQUIRE(markdown_rule->containsRule(SyntaxRule::kDefaultStateId));
}

TEST_CASE("Create analyzers by file name for sample files") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const List<U8String> files = {
    "c.json", "cpp.json", "csharp.json", "dart.json", "go.json", "groovy.json", "iapp.json",
    "java.json", "javascript.json", "json-sweetline.json", "kotlin.json", "lua.json", "objc.json", "php.json",
    "powershell.json", "python.json", "rust.json", "shell.json", "sql.json", "swift.json", "tiecode.json",
    "toml.json", "typescript.json", "vb.json", "wenyan.json", "xml.json", "yaml.json", "html.json",
    "scala.json", "css.json", "scss.json", "less.json", "jsonc.json", "json5.json", "cmake.json",
    "dockerfile.json", "makefile.json", "properties.json", "env.json", "protobuf.json", "graphql.json",
    "nginx.json", "gitignore.json", "diff.json", "ruby.json", "hcl.json", "terraform.json", "vue.json",
    "svelte.json"
  };
  for (const U8String& file_name : files) {
    CAPTURE(file_name);
    REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath(file_name)));
  }

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
    TESTS_DIR"/files/example.yaml",
    TESTS_DIR"/files/example.css",
    TESTS_DIR"/files/example.scss",
    TESTS_DIR"/files/example.less",
    TESTS_DIR"/files/example.jsonc",
    TESTS_DIR"/files/example.json5",
    TESTS_DIR"/files/example.cmake",
    TESTS_DIR"/files/example.dockerfile",
    TESTS_DIR"/files/example.mk",
    TESTS_DIR"/files/example.properties",
    TESTS_DIR"/files/example.env",
    TESTS_DIR"/files/example.proto",
    TESTS_DIR"/files/example.graphql",
    TESTS_DIR"/files/example.nginx",
    TESTS_DIR"/files/example.gitignore",
    TESTS_DIR"/files/example.diff",
    TESTS_DIR"/files/example.rb",
    TESTS_DIR"/files/example.hcl",
    TESTS_DIR"/files/example.tf",
    TESTS_DIR"/files/example.vue",
    TESTS_DIR"/files/example.svelte"
  };

  for (const U8String& file_path : sample_files) {
    CAPTURE(file_path);
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByFileName(file_path);
    REQUIRE(analyzer != nullptr);
  }
}

TEST_CASE("Exact file names take priority over suffix routing") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath("cmake.json")));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath("dockerfile.json")));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath("makefile.json")));
  REQUIRE_NOTHROW(engine->compileSyntaxFromFile(syntaxPath("gitignore.json")));

  CHECK(engine->createAnalyzerByFileName("CMakeLists.txt") != nullptr);
  CHECK(engine->createAnalyzerByFileName("Dockerfile") != nullptr);
  CHECK(engine->createAnalyzerByFileName("Containerfile") != nullptr);
  CHECK(engine->createAnalyzerByFileName("Makefile") != nullptr);
  CHECK(engine->createAnalyzerByFileName("GNUmakefile") != nullptr);
  CHECK(engine->createAnalyzerByFileName(".gitignore") != nullptr);
}

TEST_CASE("File name patterns are full matches and can create analyzers") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "patternLang",
  "fileNamePattern": "(?:generated|templated)\\.route",
  "states": {
    "default": [
      { "pattern": "\\b(route)\\b", "styles": [1, "keyword"] }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax));
  CHECK(engine->createAnalyzerByFileName("generated.route") != nullptr);
  CHECK(engine->createAnalyzerByFileName("templated.route") != nullptr);
  CHECK(engine->createAnalyzerByFileName("generated.route.bak") == nullptr);
  CHECK(engine->createAnalyzerByFileName("prefix-generated.route") == nullptr);
}

TEST_CASE("Ambiguous file suffix routing returns no analyzer") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax_a = R"({
  "name": "dupSuffixA",
  "fileSuffix": ".dup",
  "states": {
    "default": [
      { "pattern": "\\b(a)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String syntax_b = R"({
  "name": "dupSuffixB",
  "fileSuffix": ".dup",
  "states": {
    "default": [
      { "pattern": "\\b(b)\\b", "styles": [1, "keyword"] }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax_a));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax_b));
  CHECK(engine->createAnalyzerByFileName("sample.dup") == nullptr);
}

TEST_CASE("Ambiguous file name pattern routing returns no analyzer") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax_a = R"({
  "name": "patternA",
  "fileNamePattern": ".*\\.route",
  "states": {
    "default": [
      { "pattern": "\\b(a)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String syntax_b = R"({
  "name": "patternB",
  "fileNamePattern": "example\\.route",
  "states": {
    "default": [
      { "pattern": "\\b(b)\\b", "styles": [1, "keyword"] }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax_a));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(syntax_b));
  CHECK(engine->createAnalyzerByFileName("example.route") == nullptr);
}

TEST_CASE("importSyntax merges rules from source syntax") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String source_syntax = R"({
  "name": "sourceLang",
  "fileSuffixes": [".src"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String host_syntax = R"({
  "name": "hostLang",
  "fileSuffixes": [".host"],
  "states": {
    "default": [
      { "pattern": "\\b(bar)\\b", "styles": [1, "number"] },
      { "importSyntax": "sourceLang" }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(source_syntax));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(host_syntax));

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("hostLang");
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

TEST_CASE("fragments include and includes expand reusable rules") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "fragmentLang",
  "fileSuffixes": [".frag"],
  "fragments": {
    "keywordRule": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ],
    "numberRule": [
      { "pattern": "\\b([0-9]+)\\b", "styles": [1, "number"] }
    ],
    "combinedRules": [
      { "includes": ["keywordRule", "numberRule"] }
    ]
  },
  "states": {
    "default": [
      { "include": "combinedRules" }
    ]
  }
})";
  SharedPtr<SyntaxRule> rule;
  REQUIRE_NOTHROW(rule = engine->compileSyntaxFromJson(syntax));
  REQUIRE(rule != nullptr);

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("fragmentLang");
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("foo 42");
  REQUIRE(highlight != nullptr);

  std::unordered_set<int32_t> style_ids;
  for (const LineHighlight& line : highlight->lines) {
    for (const TokenSpan& span : line.spans) {
      style_ids.insert(span.style_id);
    }
  }
  REQUIRE(style_ids.find(1) != style_ids.end()); // keyword
  REQUIRE(style_ids.find(3) != style_ids.end()); // number
}

TEST_CASE("fragments includes keeps declared order") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "fragmentOrder",
  "fileSuffixes": [".fgo"],
  "fragments": {
    "first": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ],
    "second": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "number"] }
    ]
  },
  "states": {
    "default": [
      { "includes": ["second", "first"] }
    ]
  }
})";
  SharedPtr<SyntaxRule> rule;
  REQUIRE_NOTHROW(rule = engine->compileSyntaxFromJson(syntax));
  REQUIRE(rule != nullptr);

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("fragmentOrder");
  REQUIRE(analyzer != nullptr);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText("foo");
  REQUIRE(highlight != nullptr);
  REQUIRE_FALSE(highlight->lines.empty());
  REQUIRE_FALSE(highlight->lines[0].spans.empty());
  CHECK(highlight->lines[0].spans[0].style_id == 3); // "second" matches first
}

TEST_CASE("fragments include rejects missing fragment") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "fragmentMissing",
  "fileSuffixes": [".fgm"],
  "states": {
    "default": [
      { "include": "notExists" }
    ]
  }
})";
  try {
    engine->compileSyntaxFromJson(syntax);
    FAIL("Expected SyntaxCompileError");
  } catch (const SyntaxCompileError& e) {
    CHECK(e.code() == SyntaxCompileError::ERR_JSON_PROPERTY_INVALID);
    CHECK(e.message().find("fragment not found") != U8String::npos);
  }
}

TEST_CASE("fragments include rejects circular references") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String syntax = R"({
  "name": "fragmentCycle",
  "fileSuffixes": [".fgc"],
  "fragments": {
    "a": [
      { "include": "b" }
    ],
    "b": [
      { "include": "a" }
    ]
  },
  "states": {
    "default": [
      { "include": "a" }
    ]
  }
})";
  try {
    engine->compileSyntaxFromJson(syntax);
    FAIL("Expected SyntaxCompileError");
  } catch (const SyntaxCompileError& e) {
    CHECK(e.code() == SyntaxCompileError::ERR_STATE_INVALID);
    CHECK(e.message().find("circular fragments include") != U8String::npos);
  }
}

TEST_CASE("importSyntax missing dependency reports dedicated error code") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String host_syntax = R"({
  "name": "hostMissingImport",
  "fileSuffixes": [".hmi"],
  "states": {
    "default": [
      { "importSyntax": "sourceLang" }
    ]
  }
})";

  try {
    engine->compileSyntaxFromJson(host_syntax);
    FAIL("Expected SyntaxCompileError");
  } catch (const SyntaxCompileError& e) {
    CHECK(e.code() == SyntaxCompileError::ERR_IMPORT_SYNTAX_NOT_FOUND);
    CHECK(e.message().find("importSyntax not found") != U8String::npos);
  }
}

TEST_CASE("failed importSyntax retry does not invalidate imported source states") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();
  const U8String source_syntax = R"({
  "name": "sourceRetry",
  "fileSuffixes": [".srt"],
  "states": {
    "default": [
      { "pattern": "\"", "style": "punctuation", "state": "string" }
    ],
    "string": [
      { "pattern": "\"", "style": "punctuation", "state": "default" },
      { "pattern": ".", "style": "string" }
    ]
  }
})";
  const U8String host_syntax = R"({
  "name": "hostRetry",
  "fileSuffixes": [".hrt"],
  "states": {
    "default": [
      { "importSyntax": "sourceRetry" },
      { "importSyntax": "missingRetry" }
    ]
  }
})";
  const U8String missing_syntax = R"({
  "name": "missingRetry",
  "fileSuffixes": [".mrs"],
  "states": {
    "default": [
      { "pattern": "\\b(bar)\\b", "styles": [1, "keyword"] }
    ]
  }
})";

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(source_syntax));
  try {
    engine->compileSyntaxFromJson(host_syntax);
    FAIL("Expected SyntaxCompileError");
  } catch (const SyntaxCompileError& e) {
    CHECK(e.code() == SyntaxCompileError::ERR_IMPORT_SYNTAX_NOT_FOUND);
    CHECK(e.message().find("missingRetry") != U8String::npos);
  }

  SharedPtr<TextAnalyzer> source_analyzer = engine->createAnalyzerBySyntaxName("sourceRetry");
  REQUIRE(source_analyzer != nullptr);
  SharedPtr<DocumentHighlight> source_highlight = source_analyzer->analyzeText("\"x\"");
  REQUIRE(source_highlight != nullptr);

  std::unordered_set<int32_t> source_style_ids;
  for (const LineHighlight& line : source_highlight->lines) {
    for (const TokenSpan& span : line.spans) {
      source_style_ids.insert(span.style_id);
    }
  }
  CHECK(source_style_ids.find(8) != source_style_ids.end()); // punctuation
  CHECK(source_style_ids.find(2) != source_style_ids.end()); // string

  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(missing_syntax));
  REQUIRE_NOTHROW(engine->compileSyntaxFromJson(host_syntax));

  SharedPtr<TextAnalyzer> host_analyzer = engine->createAnalyzerBySyntaxName("hostRetry");
  REQUIRE(host_analyzer != nullptr);
  SharedPtr<DocumentHighlight> host_highlight = host_analyzer->analyzeText("\"x\" bar");
  REQUIRE(host_highlight != nullptr);

  std::unordered_set<int32_t> host_style_ids;
  for (const LineHighlight& line : host_highlight->lines) {
    for (const TokenSpan& span : line.spans) {
      host_style_ids.insert(span.style_id);
    }
  }
  CHECK(host_style_ids.find(8) != host_style_ids.end()); // punctuation from imported source
  CHECK(host_style_ids.find(2) != host_style_ids.end()); // string from imported source state
  CHECK(host_style_ids.find(1) != host_style_ids.end()); // keyword from newly resolved import
}

TEST_CASE("missing state references report dedicated error code") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine();

  SECTION("missing state target") {
    const U8String syntax = R"({
  "name": "missingState",
  "fileSuffixes": [".mst"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "style": "keyword", "state": "unknownState" }
    ]
  }
})";
    try {
      engine->compileSyntaxFromJson(syntax);
      FAIL("Expected SyntaxCompileError");
    } catch (const SyntaxCompileError& e) {
      CHECK(e.code() == SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND);
      CHECK(e.message().find("state: unknownState") != U8String::npos);
    }
  }

  SECTION("missing subState target") {
    const U8String syntax = R"({
  "name": "missingSubState",
  "fileSuffixes": [".mss"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "subState": "unknownSubState" }
    ]
  }
})";
    try {
      engine->compileSyntaxFromJson(syntax);
      FAIL("Expected SyntaxCompileError");
    } catch (const SyntaxCompileError& e) {
      CHECK(e.code() == SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND);
      CHECK(e.message().find("subState: unknownSubState") != U8String::npos);
    }
  }

  SECTION("missing onLineEndState target") {
    const U8String syntax = R"({
  "name": "missingLineEndState",
  "fileSuffixes": [".mle"],
  "states": {
    "default": [
      { "onLineEndState": "unknownLineEndState" },
      { "pattern": "\\b(foo)\\b", "style": "keyword" }
    ]
  }
})";
    try {
      engine->compileSyntaxFromJson(syntax);
      FAIL("Expected SyntaxCompileError");
    } catch (const SyntaxCompileError& e) {
      CHECK(e.code() == SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND);
      CHECK(e.message().find("onLineEndState: unknownLineEndState") != U8String::npos);
    }
  }
}

TEST_CASE("inline style references must be declared in styles") {
  SharedPtr<HighlightEngine> engine = makeTestHighlightEngine({true, true});
  const U8String syntax = R"({
  "name": "inlineMissingStyle",
  "fileSuffixes": [".ims"],
  "styles": [
    { "name": "keyword", "foreground": "#FF569CD6" }
  ],
  "states": {
    "default": [
      { "pattern": "\\b(true)\\b", "styles": [1, "builtin"] }
    ]
  }
})";

  try {
    engine->compileSyntaxFromJson(syntax);
    FAIL("Expected SyntaxCompileError");
  } catch (const SyntaxCompileError& e) {
    CHECK(e.code() == SyntaxCompileError::ERR_INLINE_STYLE_REFERENCE_NOT_FOUND);
    CHECK(e.message().find("undefined inline style builtin") != U8String::npos);
  }
}

TEST_CASE("importSyntax with #ifdef is controlled by macros") {
  const U8String source_syntax = R"({
  "name": "sourceLang",
  "fileSuffixes": [".src"],
  "states": {
    "default": [
      { "pattern": "\\b(foo)\\b", "styles": [1, "keyword"] }
    ]
  }
})";
  const U8String host_syntax = R"({
  "name": "hostIfdef",
  "fileSuffixes": [".hid"],
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
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("hostIfdef");
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
    SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("hostIfdef");
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

  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName("tiecode");
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
