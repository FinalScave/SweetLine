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
    U8String matched_text;
    /// 所有匹配的捕获组
    List<CaptureGroupMatch> capture_groups;
  };

  /// 单行文本语法分析
  class LineHighlightAnalyzer {
  public:
    LineHighlightAnalyzer(const SharedPtr<SyntaxRule>& syntax_rule,
      const HighlightConfig& config = HighlightConfig::kDefault);

    /// 传入行号和对应行文本进行语法分析
    /// @param text 对应行文本
    /// @param info 行起始高亮状态信息、行号等元数据信息
    /// @param result 高亮结果，用于接收分析时结果
    /// @return 分析完毕后的一些信息，供后续使用
    void analyzeLine(const U8String& text, const TextLineInfo& info, LineAnalyzeResult& result) const;

    /// 获取当前配置的高亮选项
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

    SharedPtr<Document> getDocument() const;

    const HighlightConfig& getHighlightConfig() const;
  private:
    SharedPtr<Document> m_document_;
    SharedPtr<DocumentHighlight> m_highlight_;
    SharedPtr<SyntaxRule> m_rule_;
    UniquePtr<LineHighlightAnalyzer> m_line_highlight_analyzer_;
    HighlightConfig m_config_;
    List<int32_t> m_line_syntax_states_;
    List<LineBlockState> m_line_block_states_;
  };

  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextPosition, line, column, index);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRange, start, end);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InlineStyle, foreground, background, is_bold, is_italic, is_strikethrough);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenSpan, range, style_id, inline_style, state, goto_state);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LineHighlight, spans);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DocumentHighlight, lines)
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CodeBlock, start, end, branches)
}

#endif //SWEETLINE_INTERNAL_HIGHLIGHT_H