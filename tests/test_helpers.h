#pragma once

#include <catch2/catch_amalgamated.hpp>
#include "sweetline/highlight.h"

namespace NS_SWEETLINE_TEST {
  using namespace NS_SWEETLINE;

  inline SharedPtr<HighlightEngine> makeTestHighlightEngine(
    const HighlightConfig& config = HighlightConfig::kDefault) {
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

  inline int32_t styleAtColumn(const LineHighlight& line, size_t column) {
    for (const TokenSpan& span : line.spans) {
      if (column >= span.range.start.column && column < span.range.end.column) {
        return span.style_id;
      }
    }
    return -1;
  }

  inline const IndentGuideLine* findGuideByColumn(const IndentGuideResult& result, int32_t column) {
    for (const IndentGuideLine& guide : result.guide_lines) {
      if (guide.column == column) {
        return &guide;
      }
    }
    return nullptr;
  }

  inline const IndentGuideLine* findGuideByPosition(
    const IndentGuideResult& result, int32_t column, int32_t start_line, int32_t end_line) {
    for (const IndentGuideLine& guide : result.guide_lines) {
      if (guide.column == column && guide.start_line == start_line && guide.end_line == end_line) {
        return &guide;
      }
    }
    return nullptr;
  }

  inline void requireNoTransparentSpans(const SharedPtr<DocumentHighlight>& highlight, const U8String& file_name) {
    REQUIRE(highlight != nullptr);
    for (size_t line_index = 0; line_index < highlight->lines.size(); ++line_index) {
      const LineHighlight& line = highlight->lines[line_index];
      for (const TokenSpan& span : line.spans) {
        CAPTURE(file_name, line_index, span.range.start.column, span.range.end.column, span.style_id);
        REQUIRE(span.style_id > 0);
      }
    }
  }
}
