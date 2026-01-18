#include <nlohmann/json.hpp>
#include "internal_highlight.h"
#include "util.h"

namespace NS_SWEETLINE {
  // ===================================== TokenSpan ============================================
  bool TokenSpan::operator==(const TokenSpan& other) const {
    return range == other.range && style == other.style && state == other.state && goto_state == other.goto_state;
  }

  bool TokenSpan::operator!=(const TokenSpan& other) const {
    return !this->operator==(other);
  }

#ifdef SWEETLINE_DEBUG
  void TokenSpan::dump() const {
    const nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== LineHighlight ============================================
  void LineHighlight::pushOrMergeSpan(TokenSpan&& span) {
    if (spans.empty()) {
      spans.push_back(std::move(span));
    } else {
      TokenSpan& last = spans.back();
      if (last.range.end.column == span.range.start.column && last.style == span.style) {
        last.range.end.column = span.range.end.column;
        last.range.end.index = span.range.end.index;
      } else {
        spans.push_back(std::move(span));
      }
    }
  }

  bool LineHighlight::operator==(const LineHighlight& other) const {
    if (spans.size() != other.spans.size()) {
      return false;
    }
    const size_t size = spans.size();
    for (size_t i = 0; i < size; ++i) {
      if (spans[i] != other.spans[i]) {
        return false;
      }
    }
    return true;
  }

  void LineHighlight::toJson(String& result) const {
    nlohmann::json json = *this;
#ifdef SWEETLINE_DEBUG
    result = json.dump(2);
#else
    result = json.dump();
#endif
  }

#ifdef SWEETLINE_DEBUG
  void LineHighlight::dump() const {
    const nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== DocumentHighlight ============================================
  void DocumentHighlight::addLine(LineHighlight&& line) {
    lines.push_back(std::move(line));
  }

  size_t DocumentHighlight::spanCount() const {
    size_t count = 0;
    for (const LineHighlight& line: lines) {
      count += line.spans.size();
    }
    return count;
  }

  void DocumentHighlight::reset() {
    lines.clear();
  }

  void DocumentHighlight::toJson(String& result) const {
    nlohmann::json json = *this;
#ifdef SWEETLINE_DEBUG
    result = json.dump(2);
#else
    result = json.dump();
#endif
  }

#ifdef SWEETLINE_DEBUG
  void DocumentHighlight::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== HighlightConfig ============================================
  HighlightConfig HighlightConfig::kDefault = {};

  // ===================================== InternalDocumentAnalyzer ============================================
  InternalDocumentAnalyzer::InternalDocumentAnalyzer(const Ptr<Document>& document, const Ptr<SyntaxRule>& rule,
    const HighlightConfig& config): m_document_(document), m_rule_(rule), m_config_(config) {
    m_highlight_ = MAKE_PTR<DocumentHighlight>();
  }

  Ptr<DocumentHighlight> InternalDocumentAnalyzer::analyze() {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    int32_t current_state = SyntaxRule::kDefaultStateId;
    const size_t line_count = m_document_->getLineCount();
    m_line_states_.resize(line_count, SyntaxRule::kDefaultStateId);
    m_highlight_->reset();
    size_t line_start_index = 0;
    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      LineHighlight line_highlight;
      size_t char_count = analyzeLineWithState(line_num, line_start_index, current_state, line_highlight);
      m_highlight_->addLine(std::move(line_highlight));
      current_state = m_line_states_[line_num];
      line_start_index += char_count + Document::getLineEndingWidth(m_document_->getLine(line_num).ending);
    }
    return m_highlight_;
  }

  Ptr<DocumentHighlight> InternalDocumentAnalyzer::analyzeChanges(const TextRange& range, const String& new_text) {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    int32_t line_change = m_document_->patch(range, new_text);
    size_t change_start_line = range.start.line;
    size_t change_end_line = static_cast<int32_t>(range.end.line) + line_change;
    m_line_states_[change_start_line] = change_start_line > 0 ? m_line_states_[change_start_line - 1] : SyntaxRule::kDefaultStateId;
    if (line_change < 0) {
      m_line_states_.erase(m_line_states_.begin() + range.end.line + line_change + 1,
        m_line_states_.begin() + range.end.line + 1);
      m_highlight_->lines.erase(m_highlight_->lines.begin() + range.end.line + line_change + 1,
        m_highlight_->lines.begin() + range.end.line + 1);
    } else if (line_change > 0) {
      m_line_states_.insert(m_line_states_.begin() + range.end.line + 1, line_change, SyntaxRule::kDefaultStateId);
      m_highlight_->lines.insert(m_highlight_->lines.begin() + range.end.line + 1, line_change, {});
    }

    // 从patch的起始行开始分析，直到状态稳定
    int32_t current_state = m_line_states_[change_start_line];
    size_t total_line_count = m_document_->getLineCount();
    size_t line_start_index = m_document_->charIndexOfLine(change_start_line);
    size_t line = change_start_line;
    bool stable = false;
    for (; line < total_line_count; ++line) {
      if (stable) {
        break;
      }
      int32_t old_state = m_line_states_[line];
      LineHighlight new_line_highlight;
      size_t char_count = analyzeLineWithState(line, line_start_index, current_state, new_line_highlight);
      current_state = m_line_states_[line];

      // 已经将patch range末尾后一行分析完毕，检查状态是否已经稳定
      if (line > change_end_line && old_state == current_state) {
        /*stable = true;
        for (size_t check_line = line + 1; check_line < total_line_count; ++check_line) {
          if (line_states_[check_line] != highlight_->lines[check_line].spans.back().state) {
            stable = false;
            break;
          }
        }*/
        // 或许不需要遍历后续所有行比对状态？只需要这一行的状态和高亮信息与patch前一致就可以判定为稳定
        LineHighlight old_line_highlight = m_highlight_->lines[line];
        if (old_line_highlight == new_line_highlight) {
          stable = true;
        }
      }
      m_highlight_->lines[line] = new_line_highlight;
      line_start_index += char_count + Document::getLineEndingWidth(m_document_->getLine(line).ending);
    }
    // 更新后续行的索引
    if (m_config_.show_index) {
      for (; line < total_line_count; ++line) {
        LineHighlight& line_highlight = m_highlight_->lines[line];
        for (TokenSpan& span : line_highlight.spans) {
          span.range.start.index = line_start_index + span.range.start.column;
          span.range.end.index = line_start_index + span.range.end.column;
        }
        line_start_index += m_document_->getLineCharCount(line);
      }
    }
    return m_highlight_;
  }

  Ptr<DocumentHighlight> InternalDocumentAnalyzer::analyzeChanges(size_t start_index, size_t end_index, const String& new_text) {
    TextPosition start_pos = m_document_->charIndexToPosition(start_index);
    end_index = std::min(end_index, m_document_->totalChars());
    TextPosition end_pos = m_document_->charIndexToPosition(end_index);
    return analyzeChanges(TextRange{start_pos, end_pos}, new_text);
  }

  void InternalDocumentAnalyzer::analyzeLine(size_t line, LineHighlight& line_highlight) {
    if (m_rule_ == nullptr) {
      return;
    }
    int32_t start_state = (line > 0) ? m_line_states_[line - 1] : SyntaxRule::kDefaultStateId;
    size_t line_start_index = m_document_->charIndexOfLine(line);
    analyzeLineWithState(line, line_start_index, start_state, line_highlight);
  }

  Ptr<Document> InternalDocumentAnalyzer::getDocument() const {
    return m_document_;
  }

  size_t InternalDocumentAnalyzer::analyzeLineWithState(size_t line,
    size_t line_start_index, int32_t start_state, LineHighlight& line_highlight) {
    const DocumentLine& document_line = m_document_->getLine(line);
    const String& line_text = document_line.text;

    if (line_text.empty()) {
      m_line_states_[line] = start_state;
      return 0;
    }

    size_t current_char_pos = 0;
    int32_t current_state = start_state;
    size_t line_char_count = Utf8Util::countChars(line_text);
    // 一直匹配到当前行最后一个字符
    while (current_char_pos < line_char_count) {
      MatchResult match_result = matchAtPosition(line_text, current_char_pos, current_state);
      if (!match_result.matched) {
        current_char_pos++;
        continue;
      }
      addLineMatchResult(line_highlight, line,
        current_char_pos, line_start_index, current_state, match_result);
      current_char_pos = match_result.start + match_result.length;
      if (match_result.goto_state >= 0) {
        current_state = match_result.goto_state;
      }
    }

    m_line_states_[line] = current_state;
    return line_char_count;
  }

  void InternalDocumentAnalyzer::addLineMatchResult(LineHighlight& highlight, size_t line_num,
    size_t line_char_pos, size_t line_start_index, int32_t state, const MatchResult& match_result) {
    if (match_result.capture_groups.empty()) {
      TokenSpan span;
      span.range.start = {line_num, match_result.start, line_start_index + match_result.start};
      span.range.end = {line_num, match_result.start + match_result.length, line_start_index + match_result.start + match_result.length};
      span.state = state;
      span.matched_text = match_result.matched_text;
      span.style = match_result.style;
      span.goto_state = match_result.goto_state;
      highlight.pushOrMergeSpan(std::move(span));
    } else {
      for (const CaptureGroupMatch& group_match : match_result.capture_groups) {
        TokenSpan span;
        span.range.start = {line_num, group_match.start, line_start_index + group_match.start};
        span.range.end = {line_num, group_match.start + group_match.length, line_start_index + group_match.start + group_match.length};
        span.state = state;
        span.style = group_match.style;
        span.goto_state = match_result.goto_state;
        highlight.pushOrMergeSpan(std::move(span));
      }
    }
  }

  MatchResult InternalDocumentAnalyzer::matchAtPosition(const String& text, size_t start_char_pos, int32_t state) {
    MatchResult result;
    if (!m_rule_->containsRule(state)) {
      return result;
    }
    StateRule& state_rule = m_rule_->getStateRule(state);
    size_t start_byte_pos = Utf8Util::charPosToBytePos(text, start_char_pos);

    OnigRegion* region = onig_region_new();
    const OnigUChar* start = (const OnigUChar*)(text.c_str() + start_byte_pos);
    const OnigUChar* end = (const OnigUChar*)(text.c_str() + text.length());
    const OnigUChar* range_end = end;

    int match_byte_pos = onig_search(state_rule.regex, (OnigUChar*)text.c_str(),
      end, start, range_end, region, ONIG_OPTION_NONE);
    if (match_byte_pos >= 0) {
      size_t match_start_byte = match_byte_pos;
      size_t match_end_byte = region->end[0];
      if (match_end_byte <= match_start_byte) {
        return result;
      }
      size_t match_length_bytes = match_end_byte - match_start_byte;

      size_t match_start_char = Utf8Util::bytePosToCharPos(text, match_start_byte);
      size_t match_end_char = Utf8Util::bytePosToCharPos(text, match_end_byte);
      size_t match_length_chars = match_end_char - match_start_char;

      result.matched = true;
      result.start = match_start_char;
      result.length = match_length_chars;
      result.state = state;
      result.matched_text = Utf8Util::utf8Substr(text, match_start_char, match_length_chars);

      findMatchedRuleAndGroup(state_rule, region, text, match_start_byte, match_end_byte, result);
    }
    onig_region_free(region, 1);
    return result;
  }

  void InternalDocumentAnalyzer::findMatchedRuleAndGroup(const StateRule& state_rule, OnigRegion* region,
    const String& text, size_t match_start_byte, size_t match_end_byte, MatchResult& result) {
    for (int32_t rule_idx = 0; rule_idx < static_cast<int32_t>(state_rule.token_rules.size()); ++rule_idx) {
      const TokenRule& token_rule = state_rule.token_rules[rule_idx];
      int32_t token_group_start = token_rule.group_offset_start;

      if (region->beg[token_group_start] == static_cast<int>(match_start_byte) &&
          region->end[token_group_start] == static_cast<int>(match_end_byte)) {
        result.token_rule_idx = rule_idx;
        result.goto_state = token_rule.goto_state;
        result.style = token_rule.getGroupStyle(0);
        result.matched_group = token_group_start;

        for (int32_t group = 1;group <= token_rule.group_count;++group) {
          int32_t absolute_group = group + token_group_start;
          int group_start_byte = region->beg[absolute_group];
          int group_end_byte = region->end[absolute_group];
          if (group_start_byte >= static_cast<int>(match_start_byte) && group_end_byte <= static_cast<int>(match_end_byte)) {
            CaptureGroupMatch group_match;
            group_match.group = group;
            group_match.style = token_rule.getGroupStyle(group);
            size_t match_start_char = Utf8Util::bytePosToCharPos(text, group_start_byte);
            size_t match_end_char = Utf8Util::bytePosToCharPos(text, group_end_byte);
            size_t match_length_chars = match_end_char - match_start_char;
            group_match.start = match_start_char;
            group_match.length = match_length_chars;
            result.capture_groups.push_back(group_match);
          }
        }
        return;
      }
    }
  }

  // ===================================== DocumentAnalyzer ============================================
  DocumentAnalyzer::DocumentAnalyzer(const Ptr<Document>& document, const Ptr<SyntaxRule>& rule,
    const HighlightConfig& config): analyzer_impl_(MAKE_UPTR<InternalDocumentAnalyzer>(document, rule, config)) {
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyze() const {
    return analyzer_impl_->analyze();
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyzeChanges(const TextRange& range, const String& new_text) const {
    return analyzer_impl_->analyzeChanges(range, new_text);
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyzeChanges(size_t start_index, size_t end_index, const String& new_text) const {
    return analyzer_impl_->analyzeChanges(start_index, end_index, new_text);
  }

  void DocumentAnalyzer::analyzeLine(size_t line, LineHighlight& line_highlight) const {
    analyzer_impl_->analyzeLine(line, line_highlight);
  }

  Ptr<Document> DocumentAnalyzer::getDocument() const {
    return analyzer_impl_->getDocument();
  }

  // ===================================== HighlightEngine ============================================
  HighlightEngine::HighlightEngine(const HighlightConfig& config): config_(config) {
    style_mapping_ = MAKE_PTR<StyleMapping>();
  }

  Ptr<SyntaxRule> HighlightEngine::compileSyntaxFromJson(const String& json) {
    UPtr<SyntaxRuleCompiler> compiler = MAKE_UPTR<SyntaxRuleCompiler>(style_mapping_);
    Ptr<SyntaxRule> rule = compiler->compileSyntaxFromJson(json);
    syntax_rules_.emplace(rule);
    return rule;
  }

  Ptr<SyntaxRule> HighlightEngine::compileSyntaxFromFile(const String& file) {
    UPtr<SyntaxRuleCompiler> compiler = MAKE_UPTR<SyntaxRuleCompiler>(style_mapping_);
    Ptr<SyntaxRule> rule = compiler->compileSyntaxFromFile(file);
    syntax_rules_.emplace(rule);
    return rule;
  }

  Ptr<SyntaxRule> HighlightEngine::getSyntaxRuleByName(const String& name) const {
    for (const Ptr<SyntaxRule>& rule : syntax_rules_) {
      if (rule->name == name) {
        return rule;
      }
    }
    return nullptr;
  }

  Ptr<SyntaxRule> HighlightEngine::getSyntaxRuleByExtension(const String& extension) const {
    if (extension.empty()) {
      return nullptr;
    }
    String fixed_extension = extension;
    if (fixed_extension[0] != '.') {
      fixed_extension.insert(0, ".");
    }
    for (const Ptr<SyntaxRule>& rule : syntax_rules_) {
      if (rule->file_extensions_.find(fixed_extension) != rule->file_extensions_.end()) {
        return rule;
      }
    }
    return nullptr;
  }

  void HighlightEngine::registerStyleName(const String& style_name, int32_t style_id) const {
    style_mapping_->registerStyleName(style_name, style_id);
  }

  const String& HighlightEngine::getStyleName(int32_t style_id) const {
    return style_mapping_->getStyleName(style_id);
  }

  Ptr<DocumentAnalyzer> HighlightEngine::loadDocument(const Ptr<Document>& document) {
    auto it = analyzer_map_.find(document->getUri());
    if (it == analyzer_map_.end()) {
      String uri = document->getUri();
      Ptr<SyntaxRule> rule = getSyntaxRuleByExtension(FileUtil::getExtension(uri));
      if (rule == nullptr) {
        return nullptr;
      }
      Ptr<DocumentAnalyzer> analyzer = Ptr<DocumentAnalyzer>(new DocumentAnalyzer(document, rule, config_));
      analyzer_map_.insert_or_assign(uri, analyzer);
      return analyzer;
    } else {
      return it->second;
    }
  }

  void HighlightEngine::removeDocument(const String& uri) {
    analyzer_map_.erase(uri);
  }
}
