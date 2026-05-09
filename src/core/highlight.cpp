#include <algorithm>
#include <nlohmann/json.hpp>
#include "internal_highlight.h"
#include "util.h"

#ifdef SWEETLINE_DEBUG
  #define DUMP_JSON_TO_RESULT(json, result) result = json.dump(2);
#else
  #define DUMP_JSON_TO_RESULT(json, result) result = json.dump();
#endif

namespace NS_SWEETLINE {
  namespace {
    enum class SyntaxRouteStatus {
      matched,
      not_found
    };

    struct SyntaxRouteResult {
      SyntaxRouteStatus status {SyntaxRouteStatus::not_found};
      SharedPtr<SyntaxRule> rule {nullptr};
    };

    struct PatternSpecificity {
      size_t literal_count {0};
      size_t generic_count {0};
      size_t pattern_length {0};
    };

    bool isAsciiWordChar(char ch) {
      return (ch >= 'a' && ch <= 'z')
        || (ch >= 'A' && ch <= 'Z')
        || (ch >= '0' && ch <= '9')
        || ch == '_' || ch == '-';
    }

    bool isGenericRegexEscape(char ch) {
      switch (ch) {
      case 'A':
      case 'z':
      case 'b':
      case 'B':
      case 'd':
      case 'D':
      case 's':
      case 'S':
      case 'w':
      case 'W':
      case 'p':
      case 'P':
      case 'h':
      case 'H':
      case 'v':
      case 'V':
      case 'R':
      case 'X':
      case 'Q':
      case 'E':
        return true;
      default:
        return false;
      }
    }

    PatternSpecificity computePatternSpecificity(const U8String& pattern_text) {
      PatternSpecificity result;
      result.pattern_length = pattern_text.size();
      for (size_t i = 0; i < pattern_text.size(); ++i) {
        const char ch = pattern_text[i];
        if (ch == '\\' && i + 1 < pattern_text.size()) {
          const char escaped = pattern_text[++i];
          if (isGenericRegexEscape(escaped)) {
            result.generic_count++;
          } else {
            result.literal_count++;
          }
          continue;
        }
        if (ch == '[') {
          result.generic_count++;
          while (++i < pattern_text.size()) {
            if (pattern_text[i] == '\\' && i + 1 < pattern_text.size()) {
              ++i;
              continue;
            }
            if (pattern_text[i] == ']') {
              break;
            }
          }
          continue;
        }
        if (isAsciiWordChar(ch)) {
          result.literal_count++;
          continue;
        }
        switch (ch) {
        case '.':
        case '*':
        case '+':
        case '?':
        case '|':
        case '(':
        case ')':
        case '{':
        case '}':
        case '^':
        case '$':
          result.generic_count++;
          break;
        default:
          result.literal_count++;
          break;
        }
      }
      return result;
    }

    bool isBetterRuleNameMatch(const SharedPtr<SyntaxRule>& candidate, const SharedPtr<SyntaxRule>& current) {
      return current == nullptr || candidate->name < current->name;
    }

    bool isBetterSuffixMatch(const SharedPtr<SyntaxRule>& candidate, size_t candidate_length,
      const SharedPtr<SyntaxRule>& current, size_t current_length) {
      if (current == nullptr) {
        return true;
      }
      if (candidate_length != current_length) {
        return candidate_length > current_length;
      }
      return candidate->name < current->name;
    }

    bool isBetterPatternSpecificity(const PatternSpecificity& candidate, const PatternSpecificity& current) {
      if (candidate.literal_count != current.literal_count) {
        return candidate.literal_count > current.literal_count;
      }
      if (candidate.generic_count != current.generic_count) {
        return candidate.generic_count < current.generic_count;
      }
      return candidate.pattern_length > current.pattern_length;
    }

    bool isBetterPatternMatch(const SharedPtr<SyntaxRule>& candidate_rule, const U8String& candidate_pattern,
      const PatternSpecificity& candidate_specificity, const SharedPtr<SyntaxRule>& current_rule,
      const U8String& current_pattern, const PatternSpecificity& current_specificity) {
      if (current_rule == nullptr) {
        return true;
      }
      if (isBetterPatternSpecificity(candidate_specificity, current_specificity)) {
        return true;
      }
      if (isBetterPatternSpecificity(current_specificity, candidate_specificity)) {
        return false;
      }
      if (candidate_rule->name != current_rule->name) {
        return candidate_rule->name < current_rule->name;
      }
      return candidate_pattern < current_pattern;
    }

    SyntaxRouteResult resolveSyntaxByFileName(
      const HashSet<SharedPtr<SyntaxRule>>& syntax_rules,
      const U8String& file_name) {
      if (file_name.empty()) {
        return {};
      }
      const U8String base_name = FileUtil::getPathName(file_name);
      if (base_name.empty()) {
        return {};
      }
      SharedPtr<SyntaxRule> exact_match = nullptr;
      for (const SharedPtr<SyntaxRule>& rule : syntax_rules) {
        if (rule->file_names.find(base_name) != rule->file_names.end()) {
          if (isBetterRuleNameMatch(rule, exact_match)) {
            exact_match = rule;
          }
        }
      }
      if (exact_match != nullptr) {
        SyntaxRouteResult result;
        result.status = SyntaxRouteStatus::matched;
        result.rule = exact_match;
        return result;
      }

      SharedPtr<SyntaxRule> suffix_match = nullptr;
      size_t suffix_length = 0;
      for (const SharedPtr<SyntaxRule>& rule : syntax_rules) {
        for (const U8String& suffix : rule->file_suffixes) {
          if (!StrUtil::endsWith(base_name, suffix)) {
            continue;
          }
          if (isBetterSuffixMatch(rule, suffix.size(), suffix_match, suffix_length)) {
            suffix_match = rule;
            suffix_length = suffix.size();
          }
        }
      }
      if (suffix_match != nullptr) {
        SyntaxRouteResult result;
        result.status = SyntaxRouteStatus::matched;
        result.rule = suffix_match;
        return result;
      }

      SharedPtr<SyntaxRule> pattern_match = nullptr;
      PatternSpecificity pattern_specificity;
      U8String matched_pattern;
      for (const SharedPtr<SyntaxRule>& rule : syntax_rules) {
        for (size_t i = 0; i < rule->file_name_patterns.size(); ++i) {
          if (!rule->matchesFileNamePattern(base_name, i)) {
            continue;
          }
          const U8String& pattern_text = rule->file_name_patterns[i];
          const PatternSpecificity candidate_specificity = computePatternSpecificity(pattern_text);
          if (isBetterPatternMatch(rule, pattern_text, candidate_specificity,
            pattern_match, matched_pattern, pattern_specificity)) {
            pattern_match = rule;
            pattern_specificity = candidate_specificity;
            matched_pattern = pattern_text;
          }
        }
      }
      if (pattern_match != nullptr) {
        SyntaxRouteResult result;
        result.status = SyntaxRouteStatus::matched;
        result.rule = pattern_match;
        return result;
      }
      return {};
    }

    SharedPtr<DocumentHighlightSlice> buildHighlightSlice(
      const SharedPtr<Document>& document,
      const SharedPtr<DocumentHighlight>& highlight,
      const LineRange& visible_range) {
      auto slice = makeSharedPtr<DocumentHighlightSlice>();
      if (document == nullptr) {
        return slice;
      }
      slice->total_line_count = document->getLineCount();
      slice->start_line = std::min(visible_range.start_line, slice->total_line_count);
      if (highlight == nullptr || visible_range.line_count == 0 || slice->start_line >= slice->total_line_count) {
        return slice;
      }
      size_t available_count = slice->total_line_count - slice->start_line;
      size_t slice_line_count = std::min(visible_range.line_count, available_count);
      slice->lines.reserve(slice_line_count);
      for (size_t i = 0; i < slice_line_count; ++i) {
        size_t line = slice->start_line + i;
        if (line < highlight->lines.size()) {
          slice->lines.push_back(highlight->lines[line]);
        } else {
          slice->lines.push_back({});
        }
      }
      return slice;
    }
  }

  // ===================================== TokenSpan ============================================
  bool TokenSpan::operator==(const TokenSpan& other) const {
    return range == other.range && style_id == other.style_id && state == other.state && goto_state == other.goto_state;
  }

  bool TokenSpan::operator!=(const TokenSpan& other) const {
    return !this->operator==(other);
  }

  bool TokenSpan::isReusableWith(const TokenSpan& other) const {
    return range.start.column == other.range.start.column
      && range.end.column == other.range.end.column
      && style_id == other.style_id
      && state == other.state
      && goto_state == other.goto_state;
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
  void ScopeBlock::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== LineScopeState ============================================
  bool LineScopeState::operator==(const LineScopeState& other) const {
    return nesting_level == other.nesting_level
      && scope_state == other.scope_state && scope_column == other.scope_column
      && indent_level == other.indent_level;
  }

  bool IndentGuideLine::BranchPoint::operator==(const BranchPoint &other) const {
    return line == other.line && column == other.column;
  }

  bool IndentGuideLine::operator==(const IndentGuideLine &other) const {
    return start_line == other.start_line && end_line == other.end_line
      && column == other.column && scope_rule_id == other.scope_rule_id;
  }

  // ===================================== HighlightConfig ============================================
  HighlightConfig HighlightConfig::kDefault = {};

  // ===================================== TextAnalyzer ============================================
  TextAnalyzer::TextAnalyzer(const SharedPtr<SyntaxRule>& rule, const HighlightConfig& config)
    : m_rule_(rule), m_config_(config), m_cached_highlight_(nullptr) {
    m_line_highlight_analyzer_ = makeUniquePtr<LineHighlightAnalyzer>(rule, config);
  }

  SharedPtr<DocumentHighlight> TextAnalyzer::analyzeText(const U8String& text) {
    SharedPtr<DocumentHighlight> highlight = makeSharedPtr<DocumentHighlight>();
    List<DocumentLine> lines;
    Document::splitTextIntoLines(text, lines);
    if (!lines.empty()) {
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
    }
    m_cached_highlight_ = highlight;
    return highlight;
  }

  void TextAnalyzer::analyzeLine(const U8String& text, const TextLineInfo& line_info, LineAnalyzeResult& result) const {
    m_line_highlight_analyzer_->analyzeLine(text, line_info, result);
  }

  const HighlightConfig& TextAnalyzer::getHighlightConfig() const {
    return m_line_highlight_analyzer_->getHighlightConfig();
  }

  SharedPtr<IndentGuideResult> TextAnalyzer::analyzeIndentGuides(const U8String& text, const SharedPtr<DocumentHighlight>& highlight) {
    auto result = makeSharedPtr<IndentGuideResult>();
    auto temp_doc = makeSharedPtr<Document>("", text);
    if (temp_doc == nullptr) {
      return result;
    }
    SharedPtr<DocumentHighlight> active_highlight = highlight;
    if (active_highlight == nullptr && m_cached_highlight_ != nullptr) {
      active_highlight = m_cached_highlight_;
    }
    if (active_highlight != nullptr && active_highlight->lines.size() != temp_doc->getLineCount()) {
      active_highlight = nullptr;
    }
    bool can_use_scope_rules = m_rule_ != nullptr && !m_rule_->scope_rules_map.empty() && active_highlight != nullptr;
    if (can_use_scope_rules) {
    // Check if all ScopeRule are indent-based (end == "")
      bool all_indent_based = true;
      bool has_indent_based = false;
      for (const auto& [id, br] : m_rule_->scope_rules_map) {
        if (br.end.empty()) {
          has_indent_based = true;
        } else {
          all_indent_based = false;
        }
      }
      if (all_indent_based && has_indent_based) {
        IndentGuideAnalyzer::analyzeByIndentationWithStart(m_rule_, temp_doc, active_highlight, m_config_.tab_size, result);
      } else {
        IndentGuideAnalyzer::analyzeByScopeRules(m_rule_, temp_doc, active_highlight, result);
      }
    } else {
      IndentGuideAnalyzer::analyzeByIndentation(temp_doc, m_config_.tab_size, result);
    }
    return result;
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
    // Keep matching until the last character of the current line
    while (current_char_pos < line_char_count) {
      MatchResult match_result = matchAtPosition(text, current_char_pos, current_state);
      if (!match_result.matched) {
        current_char_pos++;
        had_zero_width = false;
        continue;
      }
      // Allow at most one zero-width match at the same position to prevent infinite loop
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
    if (state_rule.line_end_state >= 0) { // If current state has a line-end state, switch to it
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

      // group 0 has subState
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
        // Has subState, recursively match and flatten
        U8String group_text = Utf8Util::utf8Substr(text, group_start_char, group_length_chars);
        expandSubStateMatches(group_text, sub_state, group_start_char, group, result.capture_groups);
      } else {
        // No subState, generate normal CaptureGroupMatch
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
      // Allow at most one zero-width match at the same position to prevent infinite loop
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
          // Sub-match has no capture groups, treat the whole match as one CaptureGroupMatch
          if (sub_result.style > 0) {
            CaptureGroupMatch gm;
            gm.group = group;
            gm.style = sub_result.style;
            gm.start = base_char_offset + sub_result.start;
            gm.length = sub_result.length;
            capture_groups.push_back(gm);
          }
        } else {
          // Sub-match has capture groups, flatten each one
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
      if (m_config_.inline_style && match_result.style > 0) {
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
        if (m_config_.inline_style && group_match.style > 0) {
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

  void InternalDocumentAnalyzer::resetAnalysisCache() {
    if (m_highlight_ != nullptr) {
      m_highlight_->reset();
    }
    m_line_syntax_states_.clear();
    m_valid_line_count_ = 0;
    m_reusable_tail_start_ = 0;
    m_reusable_tail_lines_dirty_ = false;
    m_reusable_tail_indices_dirty_ = false;
  }

  void InternalDocumentAnalyzer::invalidateAnalysisFrom(size_t line) {
    m_valid_line_count_ = std::min(m_valid_line_count_, line);
  }

  void InternalDocumentAnalyzer::ensureCacheSize(size_t line_count) {
    if (m_highlight_ == nullptr) {
      m_highlight_ = makeSharedPtr<DocumentHighlight>();
    }
    if (m_highlight_->lines.size() < line_count) {
      m_highlight_->lines.resize(line_count);
    }
    if (m_line_syntax_states_.size() < line_count) {
      m_line_syntax_states_.resize(line_count, SyntaxRule::kDefaultStateId);
    }
  }

  void InternalDocumentAnalyzer::syncCachedLinesAfterPatch(
    size_t change_start_line, size_t old_end_line, int32_t line_delta, int32_t char_delta) {
    if (m_highlight_ == nullptr) {
      return;
    }
    size_t cached_line_count = m_highlight_->lines.size();
    if (change_start_line >= cached_line_count) {
      return;
    }

    size_t old_tail_begin = old_end_line + 1;
    if (old_tail_begin >= cached_line_count) {
      m_highlight_->lines.resize(change_start_line);
      m_line_syntax_states_.resize(change_start_line);
      m_valid_line_count_ = std::min(m_valid_line_count_, change_start_line);
      m_reusable_tail_start_ = m_highlight_->lines.size();
      m_reusable_tail_lines_dirty_ = false;
      m_reusable_tail_indices_dirty_ = false;
      return;
    }

    size_t new_tail_begin = old_tail_begin;
    if (line_delta >= 0) {
      new_tail_begin += static_cast<size_t>(line_delta);
    } else {
      new_tail_begin -= static_cast<size_t>(-line_delta);
    }
    if (line_delta > 0) {
      m_highlight_->lines.insert(
        m_highlight_->lines.begin() + static_cast<ptrdiff_t>(old_tail_begin),
        static_cast<size_t>(line_delta), {});
      m_line_syntax_states_.insert(
        m_line_syntax_states_.begin() + static_cast<ptrdiff_t>(old_tail_begin),
        static_cast<size_t>(line_delta), SyntaxRule::kDefaultStateId);
    } else if (line_delta < 0) {
      m_highlight_->lines.erase(
        m_highlight_->lines.begin() + static_cast<ptrdiff_t>(new_tail_begin),
        m_highlight_->lines.begin() + static_cast<ptrdiff_t>(old_tail_begin));
      m_line_syntax_states_.erase(
        m_line_syntax_states_.begin() + static_cast<ptrdiff_t>(new_tail_begin),
        m_line_syntax_states_.begin() + static_cast<ptrdiff_t>(old_tail_begin));
    }

    m_valid_line_count_ = std::min(m_valid_line_count_, change_start_line);
    m_reusable_tail_start_ = new_tail_begin;
    m_reusable_tail_lines_dirty_ = line_delta != 0;
    m_reusable_tail_indices_dirty_ = m_config_.show_index && char_delta != 0;
  }

  bool LineHighlight::isReusableWith(const LineHighlight& other) const {
    if (spans.size() != other.spans.size()) {
      return false;
    }
    const size_t size = spans.size();
    for (size_t i = 0; i < size; ++i) {
      if (!spans[i].isReusableWith(other.spans[i])) {
        return false;
      }
    }
    return true;
  }

  void InternalDocumentAnalyzer::rebaseReusableTailFrom(size_t start_line, size_t end_line_exclusive) {
    if (m_document_ == nullptr
      || m_highlight_ == nullptr
      || start_line >= end_line_exclusive
      || (!m_reusable_tail_lines_dirty_ && !m_reusable_tail_indices_dirty_)) {
      return;
    }
    size_t total_line_count = std::min({end_line_exclusive, m_highlight_->lines.size(), m_document_->getLineCount()});
    if (start_line >= total_line_count) {
      return;
    }
    size_t line_start_index = m_reusable_tail_indices_dirty_ ? m_document_->charIndexOfLine(start_line) : 0;
    for (size_t line = start_line; line < total_line_count; ++line) {
      LineHighlight& line_highlight = m_highlight_->lines[line];
      for (TokenSpan& span : line_highlight.spans) {
        if (m_reusable_tail_lines_dirty_) {
          span.range.start.line = line;
          span.range.end.line = line;
        }
        if (m_reusable_tail_indices_dirty_) {
          span.range.start.index = line_start_index + span.range.start.column;
          span.range.end.index = line_start_index + span.range.end.column;
        }
      }
      if (m_reusable_tail_indices_dirty_) {
        line_start_index += m_document_->getLineCharCount(line);
      }
    }
  }

  SharedPtr<DocumentHighlightSlice> InternalDocumentAnalyzer::buildValidSlice(const LineRange& visible_range) const {
    auto slice = makeSharedPtr<DocumentHighlightSlice>();
    if (m_document_ == nullptr) {
      return slice;
    }
    slice->total_line_count = m_document_->getLineCount();
    slice->start_line = std::min(visible_range.start_line, slice->total_line_count);
    if (visible_range.line_count == 0 || slice->start_line >= slice->total_line_count) {
      return slice;
    }
    size_t available_count = slice->total_line_count - slice->start_line;
    size_t slice_line_count = std::min(visible_range.line_count, available_count);
    slice->lines.reserve(slice_line_count);
    for (size_t i = 0; i < slice_line_count; ++i) {
      size_t line = slice->start_line + i;
      if (m_highlight_ == nullptr || line >= m_valid_line_count_ || line >= m_highlight_->lines.size()) {
        break;
      }
      slice->lines.push_back(m_highlight_->lines[line]);
    }
    return slice;
  }

  void InternalDocumentAnalyzer::ensureAnalyzedThrough(size_t inclusive_end_line) {
    if (m_rule_ == nullptr || m_document_ == nullptr) {
      return;
    }
    const size_t line_count = m_document_->getLineCount();
    if (line_count == 0) {
      resetAnalysisCache();
      return;
    }
    size_t target_line = std::min(inclusive_end_line, line_count - 1);
    if (m_valid_line_count_ > target_line) {
      return;
    }

    size_t comparable_cached_end = m_highlight_ == nullptr ? 0 : m_highlight_->lines.size();
    size_t comparable_reusable_start = std::min(m_reusable_tail_start_, comparable_cached_end);
    ensureCacheSize(target_line + 1);
    size_t line_start_index = m_document_->charIndexOfLine(m_valid_line_count_);

    while (m_valid_line_count_ <= target_line) {
      size_t line = m_valid_line_count_;
      int32_t current_state = line == 0 ? SyntaxRule::kDefaultStateId : m_line_syntax_states_[line - 1];
      const DocumentLine& document_line = m_document_->getLine(line);
      TextLineInfo info = {line, current_state, line_start_index};
      LineAnalyzeResult result;
      m_line_highlight_analyzer_->analyzeLine(document_line.text, info, result);

      bool comparable_old = line >= comparable_reusable_start && line < comparable_cached_end;
      int32_t old_state = comparable_old ? m_line_syntax_states_[line] : SyntaxRule::kDefaultStateId;
      bool stable = comparable_old
        && old_state == result.end_state
        && result.highlight.isReusableWith(m_highlight_->lines[line]);

      m_line_syntax_states_[line] = result.end_state;
      m_highlight_->lines[line] = std::move(result.highlight);
      m_valid_line_count_ = line + 1;
      line_start_index += result.char_count + Document::getLineEndingWidth(document_line.ending);

      if (stable) {
        if (line + 1 < comparable_cached_end) {
          rebaseReusableTailFrom(line + 1, comparable_cached_end);
        }
        m_valid_line_count_ = comparable_cached_end;
        m_reusable_tail_start_ = comparable_cached_end;
        m_reusable_tail_lines_dirty_ = false;
        m_reusable_tail_indices_dirty_ = false;
        if (m_valid_line_count_ <= target_line) {
          line_start_index = m_document_->charIndexOfLine(m_valid_line_count_);
        }
      }
    }

    if (m_highlight_ != nullptr && m_valid_line_count_ >= m_highlight_->lines.size()) {
      m_reusable_tail_start_ = m_highlight_->lines.size();
      m_reusable_tail_lines_dirty_ = false;
      m_reusable_tail_indices_dirty_ = false;
    }
  }

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlight() {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    resetAnalysisCache();
    if (m_document_ != nullptr && m_document_->getLineCount() > 0) {
      ensureAnalyzedThrough(m_document_->getLineCount() - 1);
    }
    return m_highlight_;
  }

  SharedPtr<DocumentHighlightSlice> InternalDocumentAnalyzer::analyzeHighlightLineRange(const LineRange& visible_range) {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    if (m_document_ != nullptr
      && visible_range.line_count > 0
      && visible_range.start_line < m_document_->getLineCount()) {
      size_t end_line = visible_range.start_line + visible_range.line_count - 1;
      ensureAnalyzedThrough(end_line);
    }
    return buildValidSlice(visible_range);
  }

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlightIncremental(const TextRange& range, const U8String& new_text) {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    size_t old_end_line = range.end.line;
    PatchResult patch_result = m_document_->patch(range, new_text);
    size_t change_start_line = range.start.line;
    syncCachedLinesAfterPatch(change_start_line, old_end_line, patch_result.line_delta, patch_result.char_delta);
    invalidateAnalysisFrom(change_start_line);
    if (m_document_ != nullptr && m_document_->getLineCount() > 0) {
      ensureAnalyzedThrough(m_document_->getLineCount() - 1);
    }
    return m_highlight_;
  }

  SharedPtr<DocumentHighlight> InternalDocumentAnalyzer::analyzeHighlightIncremental(size_t start_index, size_t end_index, const U8String& new_text) {
    TextPosition start_pos = m_document_->charIndexToPosition(start_index);
    end_index = std::min(end_index, m_document_->totalChars());
    TextPosition end_pos = m_document_->charIndexToPosition(end_index);
    return analyzeHighlightIncremental(TextRange{start_pos, end_pos}, new_text);
  }

  SharedPtr<DocumentHighlightSlice> InternalDocumentAnalyzer::analyzeHighlightIncrementalInLineRange(
    const TextRange& range, const U8String& new_text, const LineRange& visible_range) {
    if (m_rule_ == nullptr) {
      return nullptr;
    }
    size_t old_end_line = range.end.line;
    PatchResult patch_result = m_document_->patch(range, new_text);
    size_t change_start_line = range.start.line;
    syncCachedLinesAfterPatch(change_start_line, old_end_line, patch_result.line_delta, patch_result.char_delta);
    invalidateAnalysisFrom(change_start_line);
    if (m_document_ != nullptr
      && visible_range.line_count > 0
      && visible_range.start_line < m_document_->getLineCount()) {
      size_t end_line = visible_range.start_line + visible_range.line_count - 1;
      ensureAnalyzedThrough(end_line);
    }
    return buildValidSlice(visible_range);
  }

  SharedPtr<DocumentHighlightSlice> InternalDocumentAnalyzer::getHighlightSlice(const LineRange& visible_range) const {
    return buildValidSlice(visible_range);
  }

  SharedPtr<Document> InternalDocumentAnalyzer::getDocument() const {
    return m_document_;
  }

  const HighlightConfig& InternalDocumentAnalyzer::getHighlightConfig() const {
    return m_config_;
  }

  SharedPtr<IndentGuideResult> InternalDocumentAnalyzer::analyzeIndentGuides() {
    auto result = makeSharedPtr<IndentGuideResult>();
    if (m_rule_ == nullptr || m_highlight_ == nullptr || m_document_ == nullptr) {
      return result;
    }
    if (m_valid_line_count_ != m_document_->getLineCount()) {
      return result;
    }
    if (!m_rule_->scope_rules_map.empty()) {
      bool all_indent_based = true;
      bool has_indent_based = false;
      for (const auto& [id, br] : m_rule_->scope_rules_map) {
        if (br.end.empty()) {
          has_indent_based = true;
        } else {
          all_indent_based = false;
        }
      }
      if (all_indent_based && has_indent_based) {
        IndentGuideAnalyzer::analyzeByIndentationWithStart(m_rule_, m_document_, m_highlight_, m_config_.tab_size, result);
      } else {
        IndentGuideAnalyzer::analyzeByScopeRules(m_rule_, m_document_, m_highlight_, result);
      }
    } else {
      IndentGuideAnalyzer::analyzeByIndentation(m_document_, m_config_.tab_size, result);
    }
    return result;
  }

  // ===================================== DocumentAnalyzer ============================================
  DocumentAnalyzer::DocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
    const HighlightConfig& config): analyzer_impl_(makeUniquePtr<InternalDocumentAnalyzer>(document, rule, config)) {
  }

  SharedPtr<DocumentHighlight> DocumentAnalyzer::analyze() const {
    return analyzer_impl_->analyzeHighlight();
  }

  SharedPtr<DocumentHighlightSlice> DocumentAnalyzer::analyzeLineRange(const LineRange& visible_range) const {
    return analyzer_impl_->analyzeHighlightLineRange(visible_range);
  }

  SharedPtr<DocumentHighlight> DocumentAnalyzer::analyzeIncremental(const TextRange& range, const U8String& new_text) const {
    return analyzer_impl_->analyzeHighlightIncremental(range, new_text);
  }

  SharedPtr<DocumentHighlightSlice> DocumentAnalyzer::analyzeIncrementalInLineRange(
    const TextRange& range, const U8String& new_text, const LineRange& visible_range) const {
    return analyzer_impl_->analyzeHighlightIncrementalInLineRange(range, new_text, visible_range);
  }

  SharedPtr<DocumentHighlightSlice> DocumentAnalyzer::getHighlightSlice(const LineRange& visible_range) const {
    return analyzer_impl_->getHighlightSlice(visible_range);
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

  SharedPtr<IndentGuideResult> DocumentAnalyzer::analyzeIndentGuides() const {
    return analyzer_impl_->analyzeIndentGuides();
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

  SharedPtr<SyntaxRule> HighlightEngine::getSyntaxRuleByFileName(const U8String& file_name) const {
    SyntaxRouteResult result = resolveSyntaxByFileName(m_syntax_rules_, file_name);
    return result.status == SyntaxRouteStatus::matched ? result.rule : nullptr;
  }

  void HighlightEngine::registerStyleName(const U8String& style_name, int32_t style_id) const {
    m_style_mapping_->registerStyleName(style_name, style_id);
  }

  const U8String& HighlightEngine::getStyleName(int32_t style_id) const {
    return m_style_mapping_->getStyleName(style_id);
  }

  SharedPtr<TextAnalyzer> HighlightEngine::createAnalyzerBySyntaxName(const U8String& syntax_name) const {
    SharedPtr<SyntaxRule> rule = getSyntaxRuleByName(syntax_name);
    if (rule == nullptr) {
      return nullptr;
    }
    return makeSharedPtr<TextAnalyzer>(rule, m_config_);
  }

  SharedPtr<TextAnalyzer> HighlightEngine::createAnalyzerByFileName(const U8String& file_name) const {
    SharedPtr<SyntaxRule> rule = getSyntaxRuleByFileName(file_name);
    if (rule == nullptr) {
      return nullptr;
    }
    return makeSharedPtr<TextAnalyzer>(rule, m_config_);
  }

  SharedPtr<DocumentAnalyzer> HighlightEngine::loadDocument(const SharedPtr<Document>& document) {
    auto it = m_analyzer_map_.find(document->getUri());
    if (it == m_analyzer_map_.end()) {
      U8String uri = document->getUri();
      SharedPtr<SyntaxRule> rule = getSyntaxRuleByFileName(uri);
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
