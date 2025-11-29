#include <nlohmann/json.hpp>
#include "highlight.h"
#include "util.h"

namespace NS_SWEETLINE {
  // ===================================== SyntaxRuleParseError ============================================
  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code): err_code_(err_code) {
  }

  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code, const String& message): err_code_(err_code), message_(message) {
  }

  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code, const char* message): err_code_(err_code), message_(message) {
  }

  const char* SyntaxRuleParseError::what() const noexcept {
    switch (err_code_) {
    case kErrCodePropertyExpected:
      return "Miss property";
    case kErrCodePropertyInvalid:
      return "Property invalid";
    case kErrCodePatternInvalid:
      return "Pattern invalid";
    case kErrCodeStateInvalid:
      return "State invalid";
    case kErrCodeJsonInvalid:
      return "Json invalid";
    default:
      return "Unknown error";
    }
  }

  const String& SyntaxRuleParseError::message() const noexcept {
    return message_;
  }

  // ===================================== TokenRule ============================================
  int32_t TokenRule::getGroupStyle(const int32_t group) const {
    auto it = styles.find(group);
    if (it == styles.end()) {
      return kDefaultStyle;
    }
    return it->second;
  }

  int32_t TokenRule::kDefaultStyle = 0;
  TokenRule TokenRule::kEmpty;
  StateRule StateRule::kEmpty;
  // ===================================== SyntaxRule ============================================
  int32_t SyntaxRule::getOrCreateStateId(const String& state_name) {
    auto it = state_id_map_.find(state_name);
    if (it == state_id_map_.end()) {
      int32_t new_state_id_ = state_id_counter_++;
      state_id_map_.insert_or_assign(state_name, new_state_id_);
      return new_state_id_;
    } else {
      return it->second;
    }
  }

  bool SyntaxRule::containsRule(int32_t state_id) const {
    return state_rules_map_.find(state_id) != state_rules_map_.end();
  }

  StateRule& SyntaxRule::getStateRule(int32_t state_id) {
    return state_rules_map_[state_id];
  }

  SyntaxRule::SyntaxRule() {
    state_id_map_.insert_or_assign(kDefaultStateName, kDefaultStateId);
  }

  // ===================================== StyleMapping ============================================
  int32_t StyleMapping::kDefaultStyleId = 0;
  String StyleMapping::kDefaultStyleName = "default";

  StyleMapping::StyleMapping() {
    registerStyleName(kDefaultStyleName, kDefaultStyleId);
  }

  void StyleMapping::registerStyleName(const String& style_name, int32_t style_id) {
    style_id_name_map_.insert_or_assign(style_id, style_name);
    style_name_id_map_.insert_or_assign(style_name, style_id);
  }

  int32_t StyleMapping::getStyleId(const String& style_name) {
    auto it = style_name_id_map_.find(style_name);
    if (it == style_name_id_map_.end()) {
      return kDefaultStyleId;
    }
    return it->second;
  }

  int32_t StyleMapping::getOrCreateStyleId(const String& style_name) {
    if (auto it = style_name_id_map_.find(style_name); it != style_name_id_map_.end()) {
      return it->second;
    }
    while (style_id_name_map_.find(style_id_counter_) != style_id_name_map_.end()) {
      style_id_counter_++;
    }
    style_id_name_map_.insert_or_assign(style_id_counter_, style_name);
    style_name_id_map_.insert_or_assign(style_name, style_id_counter_);
    return style_id_counter_;
  }

  const String& StyleMapping::getStyleName(int32_t style_id) {
    auto it = style_id_name_map_.find(style_id);
    if (it == style_id_name_map_.end()) {
      return kDefaultStyleName;
    }
    return it->second;
  }

  // ===================================== SyntaxRuleCompiler ============================================
  SyntaxRuleCompiler::SyntaxRuleCompiler(const Ptr<StyleMapping>& style_mapping): style_mapping_(style_mapping) {
  }

  Ptr<SyntaxRule> SyntaxRuleCompiler::compileSyntaxFromJson(const String& json) {
    Ptr<SyntaxRule> syntax_rule = MAKE_PTR<SyntaxRule>();
    nlohmann::json root;
    try {
      root = nlohmann::json::parse(json);
    } catch (nlohmann::json::parse_error& e) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodeJsonInvalid, e.what());
    }
    parseSyntaxName(syntax_rule, root);
    parseFileExtensions(syntax_rule, root);
    parseVariables(syntax_rule, root);
    parseStates(syntax_rule, root);
    // 每个state都编译成一个大表达式
    for (std::pair<const int32_t, StateRule>& pair : syntax_rule->state_rules_map_) {
      compileStatePattern(pair.second);
    }
#ifdef FH_DEBUG
    //syntax_rule->dump();
#endif
    return syntax_rule;
  }

  Ptr<SyntaxRule> SyntaxRuleCompiler::compileSyntaxFromFile(const String& file) {
    if (!FileUtil::isFile(file)) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodeFileNotExists, "File does not exist: " + file);
    }
    String content = FileUtil::readString(file);
    if (content.empty()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodeFileInvalid, "File invalid: " + file);
    }
    return compileSyntaxFromJson(content);
  }

  void SyntaxRuleCompiler::parseSyntaxName(const Ptr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("name")) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyExpected, "name");
    }
    nlohmann::json name_json = root["name"];
    if (name_json.is_string()) {
      rule->name = name_json;
    } else {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "name");
    }
  }

  void SyntaxRuleCompiler::parseFileExtensions(const Ptr<SyntaxRule>& rule, nlohmann::json& root) {
    if (root.contains("fileExtensions")) {
      nlohmann::json extensions_json = root["fileExtensions"];
      if (!extensions_json.is_array()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "fileExtensions");
      }
      for (const nlohmann::json& element_json : extensions_json) {
        if (element_json.is_string()) {
          rule->file_extensions_.emplace(element_json);
        } else {
          throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "fileExtensions");
        }
      }
    } else {
      if (root.contains("fileExtension")) {
        nlohmann::json extensions_json = root["fileExtension"];
        if (extensions_json.is_string()) {
          rule->file_extensions_.emplace(extensions_json);
        } else {
          throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "fileExtension");
        }
      } else {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyExpected, "fileExtensions or fileExtension");
      }
    }
  }

  void SyntaxRuleCompiler::parseVariables(const Ptr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("variables")) {
      return;
    }
    nlohmann::json variables_json = root["variables"];
    if (!variables_json.is_object()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "variables");
    }
    for (const auto& item : variables_json.items()) {
      const String& key = item.key();
      const nlohmann::json& variable_json = item.value();
      if (!variable_json.is_string()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, key);
      }
      rule->variables_map_.insert_or_assign(key, variable_json);
    }
    // 变量有可能引用别的变量，所以全部遍历完后替换变量引用
    for (std::pair<const String, String>& pair : rule->variables_map_) {
      replaceVariable(pair.second, rule->variables_map_);
    }
  }

  void SyntaxRuleCompiler::parseStates(const Ptr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("states")) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyExpected, "states");
    }
    nlohmann::json states_json = root["states"];
    if (!states_json.is_object()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "states");
    }
    for (const auto& item : states_json.items()) {
      const String& key = item.key();
      const nlohmann::json& state_json = item.value();
      if (!state_json.is_array()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, key);
      }
      StateRule state_rule;
      state_rule.name = key;
      parseState(rule, state_rule, state_json);
      int32_t state_id = rule->getOrCreateStateId(state_rule.name);
      rule->state_rules_map_.insert_or_assign(state_id, std::move(state_rule));
    }
    // states解析完之后将每个token要跳转的state替换成state id
    for (std::pair<const int32_t, StateRule>& pair : rule->state_rules_map_) {
      for (TokenRule& token_rule : pair.second.token_rules) {
        if (token_rule.goto_state_str.empty()) {
          continue;
        }
        token_rule.goto_state = rule->getOrCreateStateId(token_rule.goto_state_str);
        if (!rule->containsRule(token_rule.goto_state)) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid,
            "state: " + token_rule.goto_state_str);
        }
      }
    }
  }

  void SyntaxRuleCompiler::parseState(const Ptr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json) {
    for (const nlohmann::json& token_json : state_json) {
      if (!token_json.is_object()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "state element");
      }
      if (!token_json.contains("pattern")) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "pattern");
      }
      TokenRule token_rule;
      token_rule.pattern = token_json["pattern"];
      // pattern有可能引用变量，进行变量替换
      replaceVariable(token_rule.pattern, rule->variables_map_);
      // state
      if (token_json.contains("state")) {
        token_rule.goto_state_str = token_json["state"];
      }
      // style / styles
      if (token_json.contains("style")) {
        String style_name = token_json["style"];
        int32_t style_id = style_mapping_->getOrCreateStyleId(style_name);
        token_rule.styles.insert_or_assign(0, style_id);
      } else if (token_json.contains("styles")) {
        const nlohmann::json& styles_json = token_json["styles"];
        if (!styles_json.is_array()) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "styles");
        }
        size_t size = styles_json.size();
        if (size % 2 != 0) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "styles elements count % 2 != 0");
        }
        for (size_t i = 0; i < size; i += 2) {
          String style_name = styles_json[i + 1];
          int32_t style_id = style_mapping_->getOrCreateStyleId(style_name);
          token_rule.styles.insert_or_assign(styles_json[i], style_id);
        }
      } else {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePropertyInvalid, "style/styles");
      }
      state_rule.token_rules.push_back(std::move(token_rule));
    }
  }

  void SyntaxRuleCompiler::compileStatePattern(StateRule& state_rule) {
    String merged_pattern;
    int32_t total_group_count {0};
    size_t token_size = state_rule.token_rules.size();
    // 将所有token的表达式合成一个大表达式
    for (size_t i = 0; i < token_size; ++i) {
      TokenRule& token_rule = state_rule.token_rules[i];
      // 获取每个pattern的捕获组数量，并检测每个token的pattern是否有错误
      String err;
      int group_count = PatternUtil::countCaptureGroups(token_rule.pattern, err);
      if (!err.empty()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePatternInvalid, err + ": " + token_rule.pattern);
      }
      token_rule.group_count = group_count;
      token_rule.group_offset_start = 1 + total_group_count;
      total_group_count += 1 + token_rule.group_count;
      if (i > 0) {
        merged_pattern += "|";
      }
      merged_pattern += "(";
      merged_pattern += token_rule.pattern;
      merged_pattern += ")";
    }
    state_rule.group_count = total_group_count;
    // 编译合并的大表达
    OnigErrorInfo error;
    OnigRegion* region = onig_region_new();
    int status = onig_new(&state_rule.regex,
      (OnigUChar*)merged_pattern.c_str(),
      (OnigUChar*)(merged_pattern.c_str() + merged_pattern.length()),
      ONIG_OPTION_DEFAULT,
      ONIG_ENCODING_UTF8,
      ONIG_SYNTAX_DEFAULT,
      &error);
    if (status != ONIG_NORMAL) {
      onig_region_free(region, 1);
      throw SyntaxRuleParseError(SyntaxRuleParseError::kErrCodePatternInvalid, merged_pattern);
    }
    onig_region_free(region, 1);
    state_rule.merged_pattern = std::move(merged_pattern);
  }

  void SyntaxRuleCompiler::replaceVariable(String& text, HashMap<String, String>& variables_map) {
    for (const std::pair<const String, String>& pair : variables_map) {
      text = StrUtil::replaceAll(text, "${" + pair.first + "}", pair.second);
    }
  }

  // ===================================== TokenSpan ============================================
  bool TokenSpan::operator==(const TokenSpan& other) const {
    return range == other.range && style == other.style && state == other.state && goto_state == other.goto_state;
  }

  bool TokenSpan::operator!=(const TokenSpan& other) const {
    return !this->operator==(other);
  }

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
    result = json.dump(2);
  }

  // ===================================== HighlightConfig ============================================
  HighlightConfig HighlightConfig::kDefault = {};

  // ===================================== DocumentAnalyzer ============================================
  DocumentAnalyzer::DocumentAnalyzer(const Ptr<Document>& document, const Ptr<SyntaxRule>& rule,
    const HighlightConfig& config): document_(document), rule_(rule), config_(config) {
    highlight_ = MAKE_PTR<DocumentHighlight>();
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyze() {
    if (rule_ == nullptr) {
      return nullptr;
    }
    int32_t current_state = SyntaxRule::kDefaultStateId;
    const size_t line_count = document_->getLineCount();
    line_states_.resize(line_count, SyntaxRule::kDefaultStateId);
    highlight_->reset();
    size_t line_start_index = 0;
    for (size_t line_num = 0; line_num < line_count; ++line_num) {
      LineHighlight line_highlight;
      size_t char_count = analyzeLineWithState(line_num, line_start_index, current_state, line_highlight);
      highlight_->addLine(std::move(line_highlight));
      current_state = line_states_[line_num];
#ifdef _WIN32
      line_start_index += char_count + 2; // \r\n + 2
#else
      line_start_index += char_count + 1; // \n + 1
#endif
    }
    return highlight_;
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyzeChanges(const TextRange& range, const String& new_text) {
    if (rule_ == nullptr) {
      return nullptr;
    }
    int32_t line_change = document_->patch(range, new_text);
    size_t change_start_line = range.start.line;
    size_t change_end_line = static_cast<int32_t>(range.end.line) + line_change;
    line_states_[change_start_line] = change_start_line > 0 ? line_states_[change_start_line - 1] : SyntaxRule::kDefaultStateId;
    if (line_change < 0) {
      line_states_.erase(line_states_.begin() + range.end.line + line_change + 1,
        line_states_.begin() + range.end.line + 1);
      highlight_->lines.erase(highlight_->lines.begin() + range.end.line + line_change + 1,
        highlight_->lines.begin() + range.end.line + 1);
    } else if (line_change > 0) {
      line_states_.insert(line_states_.begin() + range.end.line + 1, line_change, SyntaxRule::kDefaultStateId);
      highlight_->lines.insert(highlight_->lines.begin() + range.end.line + 1, line_change, {});
    }

    // 从patch的起始行开始分析，直到状态稳定
    int32_t current_state = line_states_[change_start_line];
    size_t total_line_count = document_->getLineCount();
    size_t line_start_index = document_->charIndexOfLine(change_start_line);
    size_t line = change_start_line;
    bool stable = false;
    for (; line < total_line_count; ++line) {
      if (stable) {
        break;
      }
      int32_t old_state = line_states_[line];
      LineHighlight new_line_highlight;
      size_t char_count = analyzeLineWithState(line, line_start_index, current_state, new_line_highlight);
      current_state = line_states_[line];

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
        LineHighlight old_line_highlight = highlight_->lines[line];
        if (old_line_highlight == new_line_highlight) {
          stable = true;
        }
      }
      highlight_->lines[line] = new_line_highlight;
#ifdef _WIN32
      line_start_index += char_count + 2;
#else
      line_start_index += char_count + 1;
#endif
    }
    // 更新后续行的索引
    if (config_.show_index) {
      for (; line < total_line_count; ++line) {
        LineHighlight& line_highlight = highlight_->lines[line];
        for (TokenSpan& span : line_highlight.spans) {
          span.range.start.index = line_start_index + span.range.start.column;
          span.range.end.index = line_start_index + span.range.end.column;
        }
        line_start_index += document_->getLineCharCount(line);
      }
    }
    return highlight_;
  }

  Ptr<DocumentHighlight> DocumentAnalyzer::analyzeChanges(size_t start_index, size_t end_index, const String& new_text) {
    TextPosition start_pos = document_->charIndexToPosition(start_index);
    end_index = std::min(end_index, document_->totalChars());
    TextPosition end_pos = document_->charIndexToPosition(end_index);
    return analyzeChanges(TextRange{start_pos, end_pos}, new_text);
  }

  void DocumentAnalyzer::analyzeLine(size_t line, LineHighlight& line_highlight) {
    if (rule_ == nullptr) {
      return;
    }
    int32_t start_state = (line > 0) ? line_states_[line - 1] : SyntaxRule::kDefaultStateId;
    size_t line_start_index = document_->charIndexOfLine(line);
    analyzeLineWithState(line, line_start_index, start_state, line_highlight);
  }

  Ptr<Document> DocumentAnalyzer::getDocument() const {
    return document_;
  }

  size_t DocumentAnalyzer::analyzeLineWithState(size_t line,
                                                size_t line_start_index, int32_t start_state, LineHighlight& line_highlight) {
    const String& line_text = document_->getLine(line);

    if (line_text.empty()) {
      line_states_[line] = start_state;
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

    line_states_[line] = current_state;
    return line_char_count;
  }

  void DocumentAnalyzer::addLineMatchResult(LineHighlight& highlight, size_t line_num,
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

  MatchResult DocumentAnalyzer::matchAtPosition(const String& text, size_t start_char_pos, int32_t state) {
    MatchResult result;
    if (!rule_->containsRule(state)) {
      return result;
    }
    StateRule& state_rule = rule_->getStateRule(state);
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

  void DocumentAnalyzer::findMatchedRuleAndGroup(const StateRule& state_rule, OnigRegion* region,
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

  // ===================================== HighlightEngine ============================================
  HighlightEngine::HighlightEngine(const HighlightConfig& config): config_(config) {
    style_mapping_ = MAKE_PTR<StyleMapping>();
    syntax_rule_compiler_ = MAKE_UPTR<SyntaxRuleCompiler>(style_mapping_);
  }

  Ptr<SyntaxRule> HighlightEngine::compileSyntaxFromJson(const String& json) {
    Ptr<SyntaxRule> rule = syntax_rule_compiler_->compileSyntaxFromJson(json);
    syntax_rules_.emplace(rule);
    return rule;
  }

  Ptr<SyntaxRule> HighlightEngine::compileSyntaxFromFile(const String& file) {
    Ptr<SyntaxRule> rule = syntax_rule_compiler_->compileSyntaxFromFile(file);
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
      Ptr<DocumentAnalyzer> analyzer = MAKE_PTR<DocumentAnalyzer>(document, rule, config_);
      analyzer_map_.insert_or_assign(uri, analyzer);
      return analyzer;
    } else {
      return it->second;
    }
  }
}
