#ifndef SWEETLINE_INTERNAL_HIGHLIGHT_H
#define SWEETLINE_INTERNAL_HIGHLIGHT_H

#include "highlight.h"
#include "internal_syntax.h"

namespace NS_SWEETLINE {
  struct CaptureGroupMatch {
    /// 匹配到的捕获组索引
    int32_t group {-1};
    /// 高亮样式
    int32_t style;
    /// 匹配到的起始字符位置
    size_t start {0};
    /// 匹配到的字符长度
    size_t length {0};
  };

  /// 正则匹配结果
  struct MatchResult {
    /// 是否匹配到了
    bool matched {false};
    /// 匹配到的起始字符位置
    size_t start {0};
    /// 匹配到的字符长度
    size_t length {0};
    /// 当前所处的state
    int32_t state {-1};
    /// 匹配到的所属token rule
    int32_t token_rule_idx {-1};
    /// 匹配到的捕获组
    int32_t matched_group {-1};
    /// 高亮样式
    int32_t style {0};
    /// 要切换的state
    int32_t goto_state {-1};
    /// 匹配到的文本内容
    String matched_text;
    /// 所有匹配的捕获组
    List<CaptureGroupMatch> capture_groups;
  };

  class InternalDocumentAnalyzer {
  public:
    explicit InternalDocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
      const HighlightConfig& config = HighlightConfig::kDefault);

    SharedPtr<DocumentHighlight> analyze();

    SharedPtr<DocumentHighlight> analyzeChanges(const TextRange& range, const String& new_text);

    SharedPtr<DocumentHighlight> analyzeChanges(size_t start_index, size_t end_index, const String& new_text);

    void analyzeLine(size_t line, LineHighlight& line_highlight);

    SharedPtr<Document> getDocument() const;
  private:
    SharedPtr<Document> m_document_;
    SharedPtr<DocumentHighlight> m_highlight_;
    SharedPtr<SyntaxRule> m_rule_;
    HighlightConfig m_config_;
    List<int32_t> m_line_states_;

    size_t analyzeLineWithState(size_t line,
      size_t line_start_index, int32_t start_state, LineHighlight& line_highlight);
    void addLineMatchResult(LineHighlight& highlight, size_t line_num, size_t line_char_pos,
      size_t line_start_index, int32_t state, const MatchResult& match_result);
    MatchResult matchAtPosition(const String& text, size_t start_char_pos, int32_t state);
    void findMatchedRuleAndGroup(const StateRule& state_rule, OnigRegion* region, const String& text,
      size_t match_start_byte, size_t match_end_byte, MatchResult& result);
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextPosition, line, column, index);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRange, start, end);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenSpan, range, style, state, goto_state);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LineHighlight, spans);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentHighlight, lines)
}

#endif //SWEETLINE_INTERNAL_HIGHLIGHT_H