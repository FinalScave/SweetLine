#include <nlohmann/json.hpp>
#include "internal_highlight.h"
#include "util.h"

#ifdef SWEETLINE_DEBUG
  #define DUMP_JSON_TO_RESULT(json, result) result = json.dump(2);
#else
  #define DUMP_JSON_TO_RESULT(json, result) result = json.dump();
#endif

namespace NS_SWEETLINE {
  // ===================================== TokenSpan ============================================
  bool TokenSpan::operator==(const TokenSpan& other) const {
    return range == other.range && style_id == other.style_id && state == other.state && goto_state == other.goto_state;
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
      if (last.range.end.column == span.range.start.column && last.style_id == span.style_id) {
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

  void LineHighlight::toJson(U8String& result) const {
    nlohmann::json json = *this;
    DUMP_JSON_TO_RESULT(json, result);
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

  void DocumentHighlight::toJson(U8String& result) const {
    nlohmann::json json = *this;
    DUMP_JSON_TO_RESULT(json, result);
  }

#ifdef SWEETLINE_DEBUG
  void DocumentHighlight::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== LineState ============================================
#ifdef SWEETLINE_DEBUG
  void CodeBlock::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== LineState ============================================
  bool LineBlockState::operator==(const LineBlockState& other) const {
    return nesting_level == other.nesting_level
      && block_state == other.block_state && block_column == other.block_column;
  }

  // ===================================== HighlightConfig ============================================
  HighlightConfig HighlightConfig::kDefault = {};

  // ===================================== HighlightConfig ============================================
  TextAnalyzer::TextAnalyzer(const SharedPtr<SyntaxRule>& rule, const HighlightConfig& config) {
    m_line_highlight_analyzer_ = makeUniquePtr<LineHighlightAnalyzer>(rule, config);
  }

  SharedPtr<DocumentHighlight> TextAnalyzer::analyzeText(const U8String& text) {
    SharedPtr<DocumentHighlight> highlight = makeSharedPtr<DocumentHighlight>();
    List<DocumentLine> lines;
    Document::splitTextIntoLines(text, lines);
    if (lines.empty()) {
      return highlight;
    }
    int32_t current_state = SyntaxRule::kDefaultStateId;
    size_t line_start_index = 0;
    for (size_t line_num = 0; line_num < lines.size(); ++line_num) {
      TextLineInfo info = {line_num, current_state, line_start_index};
      LineAnalyzeResult result;
      m_line_highlight_analyzer_->analyzeLine(lines[line_num].text, info, result);
      current_state = result.end_state;
      highlight->addLine(std::move(result.highlight));
      line_start_index += result.char_count + Document::getLineEndingWidth(lines[line_num].ending);
    }
    return highlight;
  }

  void TextAnalyzer::analyzeLine(const U8String& text, const TextLineInfo& line_info, LineAnalyzeResult& result) const {
    m_line_highlight_analyzer_->analyzeLine(text, line_info, result);
  }

  const HighlightConfig& TextAnalyzer::getHighlightConfig() const {
    return m_line_highlight_analyzer_->getHighlightConfig();
  }

  // ===================================== LineHighlightAnalyzer ============================================
  LineHighlightAnalyzer::LineHighlightAnalyzer(const SharedPtr<SyntaxRule>& syntax_rule, const HighlightConfig& config)
    : m_rule_(syntax_rule), m_config_(config) {
  }

  void LineHighlightAnalyzer::analyzeLine(const U8String& text, const TextLineInfo& info, LineAnalyzeResult& result) const {
    if (text.empty()) {
      result.end_state = info.start_state;
      result.char_count = 0;
      return;
    }

    size_t current_char_pos = 0;
    int32_t current_state = info.start_state;
    size_t line_char_count = Utf8Util::countChars(text);
    bool had_zero_width = false;
    // 一直匹配到当前行最后一个字符
    while (current_char_pos < line_char_count) {
      MatchResult match_result = matchAtPosition(text, current_char_pos, current_state);
      if (!match_result.matched) {
        current_char_pos++;
        had_zero_width = false;
        continue;
      }
      // 同一位置最多允许一次零宽匹配，防止死循环
      if (match_result.length == 0) {
        if (had_zero_width) {
          current_char_pos++;
          had_zero_width = false;
          continue;
        }
        had_zero_width = true;
      } else {
        had_zero_width = false;
      }
      if (match_result.length > 0) {
        addLineHighlightResult(result.highlight, info, current_state, match_result);
      }
      current_char_pos = match_result.start + match_result.length;
      if (match_result.goto_state >= 0) {
        current_state = match_result.goto_state;
      }
    }
    StateRule& state_rule = m_rule_->getStateRule(current_state);
    if (state_rule.line_end_state >= 0) { // 如果当前状态有行结束状态，则将当前状态切换到行结束状态
      current_state = state_rule.line_end_state;
    }
    result.end_state = current_state;
    result.char_count = line_char_count;
  }

  const HighlightConfig& LineHighlightAnalyzer::getHighlightConfig() const {
    return m_config_;
  }

  MatchResult LineHighlightAnalyzer::matchAtPosition(const U8String& text, size_t start_char_pos, int32_t syntax_state) const {
    MatchResult result;
    if (!m_rule_->containsRule(syntax_state)) {
      return result;
    }
    StateRule& state_rule = m_rule_->getStateRule(syntax_state);
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
      if (match_end_byte < match_start_byte) {
        return result;
      }
      size_t match_length_bytes = match_end_byte - match_start_byte;

      size_t match_start_char = Utf8Util::bytePosToCharPos(text, match_start_byte);
      size_t match_end_char = Utf8Util::bytePosToCharPos(text, match_end_byte);
      size_t match_length_chars = match_end_char - match_start_char;

      result.matched = true;
      result.start = match_start_char;
      result.length = match_length_chars;
      result.state = syntax_state;
      result.matched_text = Utf8Util::utf8Substr(text, match_start_char, match_length_chars);

      findMatchedRuleAndGroup(state_rule, region, text, match_start_byte, match_end_byte, result);
    }
    onig_region_free(region, 1);
    return result;
  }

  void LineHighlightAnalyzer::findMatchedRuleAndGroup(const StateRule& state_rule, const OnigRegion* region,
    const U8String& text, size_t match_start_byte, size_t match_end_byte, MatchResult& result) const {
    for (int32_t rule_idx = 0; rule_idx < static_cast<int32_t>(state_rule.token_rules.size()); ++rule_idx) {
      const TokenRule& token_rule = state_rule.token_rules[rule_idx];
      int32_t token_group_start = token_rule.group_offset_start;
      if (region->beg[token_group_start] != static_cast<int>(match_start_byte)
        || region->end[token_group_start] != static_cast<int>(match_end_byte)) {
        continue;
      }

      result.token_rule_idx = rule_idx;
      result.goto_state = token_rule.goto_state;
      result.style = token_rule.getGroupStyleId(0);
      result.matched_group = token_group_start;

      // group 0 有subState
      int32_t whole_sub_state = token_rule.getGroupSubState(0);
      if (whole_sub_state >= 0) {
        expandSubStateMatches(result.matched_text, whole_sub_state,
          result.start, 0, result.capture_groups);
        return;
      }
      buildCaptureGroups(token_rule, region, text, match_start_byte, match_end_byte, result);
      return;
    }
  }

  void LineHighlightAnalyzer::buildCaptureGroups(const TokenRule& token_rule, const OnigRegion* region,
    const U8String& text, size_t match_start_byte, size_t match_end_byte, MatchResult& result) const {
    int32_t token_group_start = token_rule.group_offset_start;
    for (int32_t group = 1; group <= token_rule.group_count; ++group) {
      int32_t absolute_group = group + token_group_start;
      int group_start_byte = region->beg[absolute_group];
      int group_end_byte = region->end[absolute_group];
      if (group_start_byte < static_cast<int>(match_start_byte)
        || group_end_byte > static_cast<int>(match_end_byte)) {
        continue;
      }
      size_t group_start_char = Utf8Util::bytePosToCharPos(text, group_start_byte);
      size_t group_end_char = Utf8Util::bytePosToCharPos(text, group_end_byte);
      size_t group_length_chars = group_end_char - group_start_char;

      int32_t sub_state = token_rule.getGroupSubState(group);
      if (sub_state >= 0) {
        // 有subState，递归匹配并展平
        U8String group_text = Utf8Util::utf8Substr(text, group_start_char, group_length_chars);
        expandSubStateMatches(group_text, sub_state, group_start_char, group, result.capture_groups);
      } else {
        // 无subState, 正常生成CaptureGroupMatch
        CaptureGroupMatch group_match;
        group_match.group = group;
        group_match.style = token_rule.getGroupStyleId(group);
        group_match.start = group_start_char;
        group_match.length = group_length_chars;
        result.capture_groups.push_back(group_match);
      }
    }
  }

  void LineHighlightAnalyzer::expandSubStateMatches(const U8String& sub_text, int32_t sub_state,
    size_t base_char_offset, int32_t group, List<CaptureGroupMatch>& capture_groups) const {
    size_t sub_text_len = Utf8Util::countChars(sub_text);
    size_t sub_pos = 0;
    int32_t current_state = sub_state;
    bool had_zero_width = false;
    while (sub_pos < sub_text_len) {
      MatchResult sub_result = matchAtPosition(sub_text, sub_pos, current_state);
      if (!sub_result.matched) {
        sub_pos++;
        had_zero_width = false;
        continue;
      }
      // 同一位置最多允许一次零宽匹配，防止死循环
      if (sub_result.length == 0) {
        if (had_zero_width) {
          sub_pos++;
          had_zero_width = false;
          continue;
        }
        had_zero_width = true;
      } else {
        had_zero_width = false;
      }
      if (sub_result.length > 0) {
        if (sub_result.capture_groups.empty()) {
          // 子匹配无捕获组，整体作为一个CaptureGroupMatch
          if (sub_result.style > 0) {
            CaptureGroupMatch gm;
            gm.group = group;
            gm.style = sub_result.style;
            gm.start = base_char_offset + sub_result.start;
            gm.length = sub_result.length;
            capture_groups.push_back(gm);
          }
        } else {
          // 子匹配有捕获组，逐个展平
          for (const CaptureGroupMatch& sub_gm : sub_result.capture_groups) {
            if (sub_gm.style > 0) {
              CaptureGroupMatch gm;
              gm.group = group;
              gm.style = sub_gm.style;
              gm.start = base_char_offset + sub_gm.start;
              gm.length = sub_gm.length;
              capture_groups.push_back(gm);
            }
          }
        }
      }
      sub_pos = sub_result.start + sub_result.length;
      if (sub_result.goto_state >= 0) {
        current_state = sub_result.goto_state;
      }
    }
  }

  void LineHighlightAnalyzer::addLineHighlightResult(LineHighlight& highlight, const TextLineInfo& info,
    int32_t syntax_state, const MatchResult& match_result) const {
    if (match_result.capture_groups.empty()) {
      TokenSpan span;
      span.range.start = {
        info.line,
        match_result.start,
        info.start_char_offset + match_result.start
      };
      span.range.end = {
        info.line,
        match_result.start + match_result.length,
        info.start_char_offset + match_result.start + match_result.length
      };
      span.state = syntax_state;
      span.matched_text = match_result.matched_text;
      span.style_id = match_result.style;
      if (m_config_.inline_style) {
        span.inline_style = m_rule_->getInlineStyle(match_result.style);
      }
      span.goto_state = match_result.goto_state;
      highlight.pushOrMergeSpan(std::move(span));
    } else {
      for (const CaptureGroupMatch& group_match : match_result.capture_groups) {
        TokenSpan span;
        span.range.start = {
          info.line,
          group_match.start,
          info.start_char_offset + group_match.start
        };
        span.range.end = {
          info.line,
          group_match.start + group_match.length,
          info.start_char_offset + group_match.start + group_match.length
        };
        span.state = syntax_state;
        span.style_id = group_match.style;
        if (m_config_.inline_style) {
          span.inline_style = m_rule_->getInlineStyle(group_match.style);
        }
        span.goto_state = match_result.goto_state;
        highlight.pushOrMergeSpan(std::move(span));
      }
    }
  }

  // ===================================== InternalDocumentAnalyzer ============================================
  InternalDocumentAnalyzer::InternalDocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
    const HighlightConfig& config): m_document_(document), m_rule_(rule), m_config_(config) {
    m_highlight_ = makeSharedPtr<DocumentHighlight>();
    m_line_highlight_analyzer_ = makeUniquePtr<LineHighlightAnalyzer>(m_rule_, config);
  }

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlight() {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    int32_t current_state = SyntaxRule::kDefaultStateId;
    const size_t line_count = m_document_->getLineCount();
    m_line_syntax_states_.resize(line_count, {});
    m_highlight_->reset();
    size_t line_start_index = 0;
    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      TextLineInfo info = {line_num, current_state, line_start_index};
      LineAnalyzeResult result;
      const DocumentLine& document_line = m_document_->getLine(line_num);
      m_line_highlight_analyzer_->analyzeLine(document_line.text, info, result);
      m_line_syntax_states_[line_num] = result.end_state;
      m_highlight_->addLine(std::move(result.highlight));
      current_state = result.end_state;
      line_start_index += result.char_count + Document::getLineEndingWidth(m_document_->getLine(line_num).ending);
    }
    return m_highlight_;
  }

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlightIncremental(const TextRange& range, const U8String& new_text) {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    int32_t line_change = m_document_->patch(range, new_text);
    size_t change_start_line = range.start.line;
    size_t change_end_line = static_cast<int32_t>(range.end.line) + line_change;
    m_line_syntax_states_[change_start_line] = change_start_line > 0 ? m_line_syntax_states_[change_start_line - 1] : SyntaxRule::kDefaultStateId;
    if (line_change < 0) {
      m_line_syntax_states_.erase(m_line_syntax_states_.begin() + range.end.line + line_change + 1,
        m_line_syntax_states_.begin() + range.end.line + 1);
      m_highlight_->lines.erase(m_highlight_->lines.begin() + range.end.line + line_change + 1,
        m_highlight_->lines.begin() + range.end.line + 1);
    } else if (line_change > 0) {
      m_line_syntax_states_.insert(m_line_syntax_states_.begin() + range.end.line + 1, line_change, {});
      m_highlight_->lines.insert(m_highlight_->lines.begin() + range.end.line + 1, line_change, {});
    }

    // 从patch的起始行开始分析，直到状态稳定
    int32_t current_state = m_line_syntax_states_[change_start_line];
    size_t total_line_count = m_document_->getLineCount();
    size_t line_start_index = m_document_->charIndexOfLine(change_start_line);
    size_t line = change_start_line;
    bool stable = false;
    for (; line < total_line_count; ++line) {
      if (stable) {
        break;
      }
      int32_t old_state = m_line_syntax_states_[line];
      TextLineInfo line_info = {line, current_state, line_start_index};
      LineAnalyzeResult result;
      const DocumentLine& document_line = m_document_->getLine(line);
      m_line_highlight_analyzer_->analyzeLine(document_line.text, line_info, result);
      m_line_syntax_states_[line] = result.end_state;
      current_state = result.end_state;

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
        const LineHighlight& old_line_highlight = m_highlight_->lines[line];
        if (old_line_highlight == result.highlight) {
          stable = true;
        }
      }
      m_highlight_->lines[line] = std::move(result.highlight);
      line_start_index += result.char_count + Document::getLineEndingWidth(m_document_->getLine(line).ending);
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

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlightIncremental(size_t start_index, size_t end_index, const U8String& new_text) {
    TextPosition start_pos = m_document_->charIndexToPosition(start_index);
    end_index = std::min(end_index, m_document_->totalChars());
    TextPosition end_pos = m_document_->charIndexToPosition(end_index);
    return analyzeHighlightIncremental(TextRange{start_pos, end_pos}, new_text);
  }

  SharedPtr<Document> InternalDocumentAnalyzer::getDocument() const {
    return m_document_;
  }

  const HighlightConfig& InternalDocumentAnalyzer::getHighlightConfig() const {
    return m_config_;
  }

  // ===================================== DocumentAnalyzer ============================================
  DocumentAnalyzer::DocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
    const HighlightConfig& config): analyzer_impl_(makeUniquePtr<InternalDocumentAnalyzer>(document, rule, config)) {
  }

  SharedPtr<DocumentHighlight> DocumentAnalyzer::analyze() const {
    return analyzer_impl_->analyzeHighlight();
  }

  SharedPtr<DocumentHighlight> DocumentAnalyzer::analyzeIncremental(const TextRange& range, const U8String& new_text) const {
    return analyzer_impl_->analyzeHighlightIncremental(range, new_text);
  }

  SharedPtr<DocumentHighlight> DocumentAnalyzer::analyzeIncremental(size_t start_index, size_t end_index, const U8String& new_text) const {
    return analyzer_impl_->analyzeHighlightIncremental(start_index, end_index, new_text);
  }

  SharedPtr<Document> DocumentAnalyzer::getDocument() const {
    return analyzer_impl_->getDocument();
  }

  const HighlightConfig& DocumentAnalyzer::getHighlightConfig() const {
    return analyzer_impl_->getHighlightConfig();
  }

  // ===================================== HighlightEngine ============================================
  HighlightEngine::HighlightEngine(const HighlightConfig& config): m_config_(config) {
    m_style_mapping_ = makeSharedPtr<StyleMapping>();
  }

  void HighlightEngine::defineMacro(const U8String& macro_name) {
    m_macros_.emplace(macro_name);
  }

  void HighlightEngine::undefineMacro(const U8String& macro_name) {
    m_macros_.erase(macro_name);
  }

  bool HighlightEngine::isMacroDefined(const U8String& macro_name) const {
    return m_macros_.find(macro_name) != m_macros_.end();
  }

  SharedPtr<SyntaxRule> HighlightEngine::compileSyntaxFromJson(const U8String& json) {
    UniquePtr<SyntaxRuleCompiler> compiler = makeUniquePtr<SyntaxRuleCompiler>(m_style_mapping_, m_config_.inline_style, this);
    SharedPtr<SyntaxRule> rule = compiler->compileSyntaxFromJson(json);
    m_syntax_rules_.emplace(rule);
    return rule;
  }

  SharedPtr<SyntaxRule> HighlightEngine::compileSyntaxFromFile(const U8String& file) {
    UniquePtr<SyntaxRuleCompiler> compiler = makeUniquePtr<SyntaxRuleCompiler>(m_style_mapping_, m_config_.inline_style, this);
    SharedPtr<SyntaxRule> rule = compiler->compileSyntaxFromFile(file);
    m_syntax_rules_.emplace(rule);
    return rule;
  }

  SharedPtr<SyntaxRule> HighlightEngine::getSyntaxRuleByName(const U8String& name) const {
    for (const SharedPtr<SyntaxRule>& rule : m_syntax_rules_) {
      if (rule->name == name) {
        return rule;
      }
    }
    return nullptr;
  }

  SharedPtr<SyntaxRule> HighlightEngine::getSyntaxRuleByExtension(const U8String& extension) const {
    if (extension.empty()) {
      return nullptr;
    }
    U8String fixed_extension = extension;
    if (fixed_extension[0] != '.') {
      fixed_extension.insert(0, ".");
    }
    for (const SharedPtr<SyntaxRule>& rule : m_syntax_rules_) {
      if (rule->file_extensions.find(fixed_extension) != rule->file_extensions.end()) {
        return rule;
      }
    }
    return nullptr;
  }

  void HighlightEngine::registerStyleName(const U8String& style_name, int32_t style_id) const {
    m_style_mapping_->registerStyleName(style_name, style_id);
  }

  const U8String& HighlightEngine::getStyleName(int32_t style_id) const {
    return m_style_mapping_->getStyleName(style_id);
  }

  SharedPtr<TextAnalyzer> HighlightEngine::createAnalyzerByName(const U8String& syntax_name) const {
    SharedPtr<SyntaxRule> rule = getSyntaxRuleByName(syntax_name);
    if (rule == nullptr) {
      return nullptr;
    }
    return makeSharedPtr<TextAnalyzer>(rule, m_config_);
  }

  SharedPtr<TextAnalyzer> HighlightEngine::createAnalyzerByExtension(const U8String& extension) const {
    SharedPtr<SyntaxRule> rule = getSyntaxRuleByExtension(extension);
    if (rule == nullptr) {
      return nullptr;
    }
    return makeSharedPtr<TextAnalyzer>(rule, m_config_);
  }

  SharedPtr<DocumentAnalyzer> HighlightEngine::loadDocument(const SharedPtr<Document>& document) {
    auto it = m_analyzer_map_.find(document->getUri());
    if (it == m_analyzer_map_.end()) {
      U8String uri = document->getUri();
      SharedPtr<SyntaxRule> rule = getSyntaxRuleByExtension(FileUtil::getExtension(uri));
      if (rule == nullptr) {
        return nullptr;
      }
      SharedPtr<DocumentAnalyzer> analyzer = SharedPtr<DocumentAnalyzer>(new DocumentAnalyzer(document, rule, m_config_));
      m_analyzer_map_.insert_or_assign(uri, analyzer);
      return analyzer;
    } else {
      return it->second;
    }
  }

  void HighlightEngine::removeDocument(const U8String& uri) {
    m_analyzer_map_.erase(uri);
  }
}
