#ifndef SWEETLINE_INTERNAL_HIGHLIGHT_H
#define SWEETLINE_INTERNAL_HIGHLIGHT_H

#include "highlight.h"
#include "internal_syntax.h"

namespace NS_SWEETLINE {
  struct CaptureGroupMatch {
    /// Matched capture group index
    int32_t group {-1};
    /// Highlight style
    int32_t style;
    /// Matched start character position
    size_t start {0};
    /// Matched character length
    size_t length {0};
  };

  /// Regex match result
  struct MatchResult {
    /// Whether a match was found
    bool matched {false};
    /// Matched start character position
    size_t start {0};
    /// Matched character length
    size_t length {0};
    /// Current state
    int32_t state {-1};
    /// Index of the matched token rule
    int32_t token_rule_idx {-1};
    /// Matched capture group
    int32_t matched_group {-1};
    /// Highlight style
    int32_t style {0};
    /// Target state to transition to
    int32_t goto_state {-1};
    /// Matched text content
    U8String matched_text;
    /// All matched capture groups
    List<CaptureGroupMatch> capture_groups;
  };

  /// Single line text syntax analysis
  class LineHighlightAnalyzer {
  public:
    LineHighlightAnalyzer(const SharedPtr<SyntaxRule>& syntax_rule,
      const HighlightConfig& config = HighlightConfig::kDefault);

    /// Analyze a line by passing the line number and corresponding text
    /// @param text Line text content
    /// @param info Metadata including start highlight state and line number
    /// @param result Highlight result, receives analysis output
    /// @return Some information after analysis for subsequent use
    void analyzeLine(const U8String& text, const TextLineInfo& info, LineAnalyzeResult& result) const;

    /// Get the currently configured highlight options
    const HighlightConfig& getHighlightConfig() const;
  private:
    SharedPtr<SyntaxRule> m_rule_;
    HighlightConfig m_config_;

    MatchResult matchAtPosition(const U8String& text, size_t start_char_pos, int32_t syntax_state) const;

    void findMatchedRuleAndGroup(const StateRule& state_rule, const OnigRegion* region,
      const U8String& text, size_t match_start_byte, size_t match_end_byte, MatchResult& result) const;

    void buildCaptureGroups(const TokenRule& token_rule, const OnigRegion* region,
      const U8String& text, size_t match_start_byte, size_t match_end_byte, MatchResult& result) const;

    void expandSubStateMatches(const U8String& sub_text, int32_t sub_state,
      size_t base_char_offset, int32_t group, List<CaptureGroupMatch>& capture_groups) const;

    void addLineHighlightResult(LineHighlight& highlight, const TextLineInfo& info,
      int32_t syntax_state, const MatchResult& match_result) const;
  };

  class InternalDocumentAnalyzer {
  public:
    explicit InternalDocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
      const HighlightConfig& config = HighlightConfig::kDefault);

    SharedPtr<DocumentHighlight> analyzeHighlight();

    SharedPtr<DocumentHighlight> analyzeHighlightIncremental(const TextRange& range, const U8String& new_text);

    SharedPtr<DocumentHighlight> analyzeHighlightIncremental(size_t start_index, size_t end_index, const U8String& new_text);

    SharedPtr<DocumentHighlightSlice> analyzeHighlightIncrementalInLineRange(const TextRange& range, const U8String& new_text,
      const LineRange& visible_range);

    SharedPtr<DocumentHighlightSlice> getHighlightSlice(const LineRange& visible_range) const;

    SharedPtr<IndentGuideResult> analyzeIndentGuides();

    SharedPtr<Document> getDocument() const;

    const HighlightConfig& getHighlightConfig() const;
  private:
    SharedPtr<Document> m_document_;
    SharedPtr<DocumentHighlight> m_highlight_;
    SharedPtr<SyntaxRule> m_rule_;
    UniquePtr<LineHighlightAnalyzer> m_line_highlight_analyzer_;
    HighlightConfig m_config_;
    List<int32_t> m_line_syntax_states_;
    List<LineScopeState> m_line_scope_states_;
  };

  /// Indent guide analyzer (post-processing phase independent of highlight analysis)
  class IndentGuideAnalyzer {
  public:
    /// Scope-based analysis using scopeRules (Strategy A)
    static void analyzeByScopeRules(
      const SharedPtr<SyntaxRule>& rule,
      const SharedPtr<Document>& document,
      const SharedPtr<DocumentHighlight>& highlight,
      SharedPtr<IndentGuideResult>& result);

    /// Indentation-based block analysis (Strategy B)
    static void analyzeByIndentation(
      const SharedPtr<Document>& document,
      int32_t tab_size,
      SharedPtr<IndentGuideResult>& result);

    /// Indentation-based block analysis with start markers (Python hybrid mode)
    static void analyzeByIndentationWithStart(
      const SharedPtr<SyntaxRule>& rule,
      const SharedPtr<Document>& document,
      const SharedPtr<DocumentHighlight>& highlight,
      int32_t tab_size,
      SharedPtr<IndentGuideResult>& result);

  private:
    /// Compute the leading whitespace column count of a line (with tab expansion)
    static int32_t computeLeadingWhitespace(const U8String& text, int32_t tab_size);
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextPosition, line, column, index);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRange, start, end);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InlineStyle, foreground, background, is_bold, is_italic, is_strikethrough);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenSpan, range, style_id, inline_style, state, goto_state);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LineHighlight, spans);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentHighlight, lines)
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScopeBlock, start, end, branches)
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IndentGuideLine::BranchPoint, line, column)
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IndentGuideLine, column, start_line, end_line, nesting_level, scope_rule_id, branches)
}

#endif //SWEETLINE_INTERNAL_HIGHLIGHT_H
