#ifdef SWEETLINE_DEBUG
#include <iostream>
#endif

#include "internal_syntax.h"
#include "highlight.h"
#include "util.h"

namespace NS_SWEETLINE {
 // ===================================== SyntaxRuleParseError ============================================
  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code): m_err_code_(err_code) {
  }

  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code, const U8String& message): m_err_code_(err_code), m_message_(message) {
  }

  SyntaxRuleParseError::SyntaxRuleParseError(const int err_code, const char* message): m_err_code_(err_code), m_message_(message) {
  }

  const char* SyntaxRuleParseError::what() const noexcept {
    switch (m_err_code_) {
    case ERR_JSON_PROPERTY_MISSED:
      return "Miss property";
    case ERR_JSON_PROPERTY_INVALID:
      return "Property invalid";
    case ERR_PATTERN_INVALID:
      return "Pattern invalid";
    case ERR_STATE_INVALID:
      return "State invalid";
    case ERR_JSON_INVALID:
      return "Json invalid";
    default:
      return "Unknown error";
    }
  }

  const U8String& SyntaxRuleParseError::message() const noexcept {
    return m_message_;
  }

  int SyntaxRuleParseError::code() const noexcept {
    return m_err_code_;
  }

  // ===================================== TokenRule ============================================
  int32_t TokenRule::getGroupStyleId(const int32_t group) const {
    auto it = style_ids.find(group);
    if (it == style_ids.end()) {
      return kDefaultStyleId;
    }
    return it->second;
  }

  int32_t TokenRule::getGroupSubState(const int32_t group) const {
    auto it = sub_states.find(group);
    if (it == sub_states.end()) {
      return -1;
    }
    return it->second;
  }

  int32_t TokenRule::kDefaultStyleId = 0;
  TokenRule TokenRule::kEmpty;
#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenRule, pattern, style_ids, goto_state_str, group_count, group_offset_start, goto_state);
  void TokenRule::dump() const {
    const nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== StateRule ============================================
  StateRule StateRule::kEmpty;
#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StateRule, name, token_rules, merged_pattern, group_count);
  void StateRule::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== SyntaxRule ============================================
  bool SyntaxRule::containsInlineStyle(int32_t style_id) {
    return inline_styles.find(style_id) != inline_styles.end();
  }

  InlineStyle& SyntaxRule::getInlineStyle(int32_t style_id) {
    return inline_styles.find(style_id)->second;
  }

  int32_t SyntaxRule::getOrCreateStateId(const U8String& state_name) {
    auto it = state_id_map.find(state_name);
    if (it == state_id_map.end()) {
      int32_t new_state_id_ = m_state_id_counter_++;
      state_id_map.insert_or_assign(state_name, new_state_id_);
      return new_state_id_;
    } else {
      return it->second;
    }
  }

  bool SyntaxRule::containsRule(int32_t state_id) const {
    return state_rules_map.find(state_id) != state_rules_map.end();
  }

  StateRule& SyntaxRule::getStateRule(int32_t state_id) {
    return state_rules_map[state_id];
  }

  SyntaxRule::SyntaxRule() {
    state_id_map.insert_or_assign(kDefaultStateName, kDefaultStateId);
  }

#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SyntaxRule, name, file_extensions, variables_map, state_rules_map, state_id_map);
  void SyntaxRule::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== StyleMapping ============================================
  int32_t StyleMapping::kDefaultStyleId = 0;
  U8String StyleMapping::kDefaultStyleName = "default";

  StyleMapping::StyleMapping() {
    registerStyleName(kDefaultStyleName, kDefaultStyleId);
  }

  void StyleMapping::registerStyleName(const U8String& style_name, int32_t style_id) {
    style_id_name_map.insert_or_assign(style_id, style_name);
    style_name_id_map.insert_or_assign(style_name, style_id);
  }

  int32_t StyleMapping::getStyleId(const U8String& style_name) {
    auto it = style_name_id_map.find(style_name);
    if (it == style_name_id_map.end()) {
      return kDefaultStyleId;
    }
    return it->second;
  }

  int32_t StyleMapping::getOrCreateStyleId(const U8String& style_name) {
    if (auto it = style_name_id_map.find(style_name); it != style_name_id_map.end()) {
      return it->second;
    }
    while (style_id_name_map.find(m_style_id_counter_) != style_id_name_map.end()) {
      m_style_id_counter_++;
    }
    style_id_name_map.insert_or_assign(m_style_id_counter_, style_name);
    style_name_id_map.insert_or_assign(style_name, m_style_id_counter_);
    return m_style_id_counter_;
  }

  const U8String& StyleMapping::getStyleName(int32_t style_id) {
    auto it = style_id_name_map.find(style_id);
    if (it == style_id_name_map.end()) {
      return kDefaultStyleName;
    }
    return it->second;
  }

  // ===================================== SyntaxRuleCompiler ============================================
  SyntaxRuleCompiler::SyntaxRuleCompiler(const SharedPtr<StyleMapping>& style_mapping, bool inline_style, HighlightEngine* engine)
    : m_style_mapping_(style_mapping), m_inline_style_(inline_style), m_engine_(engine) {
  }

  SharedPtr<SyntaxRule> SyntaxRuleCompiler::compileSyntaxFromJson(const U8String& json) {
    SharedPtr<SyntaxRule> syntax_rule = makeSharedPtr<SyntaxRule>();
    nlohmann::json root;
    try {
      root = nlohmann::json::parse(json);
    } catch (nlohmann::json::parse_error& e) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_INVALID, e.what());
    }
    parseSyntaxName(syntax_rule, root);
    parseFileExtensions(syntax_rule, root);
    if (m_inline_style_) {
      syntax_rule->style_mapping = makeUniquePtr<StyleMapping>();
      parseInlineStyles(syntax_rule, root);
    }
    parseVariables(syntax_rule, root);
    parseStates(syntax_rule, root);
    // 处理importSyntax请求（在state id解析完成后、编译大表达式之前）
    processImportSyntaxRequests(syntax_rule);
    // 每个state都编译成一个大表达式
    for (std::pair<const int32_t, StateRule>& pair : syntax_rule->state_rules_map) {
      compileStatePattern(pair.second);
    }
    // 解析作用域划线规则
    parseBlockPairs(syntax_rule, root);
#ifdef SWEETLINE_DEBUG
    //syntax_rule->dump();
#endif
    return syntax_rule;
  }

  SharedPtr<SyntaxRule> SyntaxRuleCompiler::compileSyntaxFromFile(const U8String& file) {
    if (!FileUtil::isFile(file)) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_FILE_NOT_EXISTS, "File does not exist: " + file);
    }
    U8String content = FileUtil::readString(file);
    if (content.empty()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_FILE_INVALID, "File invalid: " + file);
    }
    return compileSyntaxFromJson(content);
  }

  void SyntaxRuleCompiler::parseSyntaxName(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("name")) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "name");
    }
    nlohmann::json name_json = root["name"];
    if (name_json.is_string()) {
      rule->name = name_json;
    } else {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "name");
    }
  }

  void SyntaxRuleCompiler::parseFileExtensions(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (root.contains("fileExtensions")) {
      nlohmann::json extensions_json = root["fileExtensions"];
      if (!extensions_json.is_array()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "fileExtensions");
      }
      for (const nlohmann::json& element_json : extensions_json) {
        if (element_json.is_string()) {
          rule->file_extensions.emplace(element_json);
        } else {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "fileExtensions");
        }
      }
    } else {
      if (root.contains("fileExtension")) {
        nlohmann::json extensions_json = root["fileExtension"];
        if (extensions_json.is_string()) {
          rule->file_extensions.emplace(extensions_json);
        } else {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "fileExtension");
        }
      } else {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "fileExtensions or fileExtension");
      }
    }
  }

  void SyntaxRuleCompiler::parseVariables(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("variables")) {
      return;
    }
    nlohmann::json variables_json = root["variables"];
    if (!variables_json.is_object()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "variables");
    }
    for (const auto& item : variables_json.items()) {
      const U8String& key = item.key();
      const nlohmann::json& variable_json = item.value();
      if (!variable_json.is_string()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, key);
      }
      rule->variables_map.insert_or_assign(key, variable_json);
    }
    // 变量有可能引用别的变量，所以全部遍历完后替换变量引用
    for (std::pair<const U8String, U8String>& pair : rule->variables_map) {
      replaceVariable(pair.second, rule->variables_map);
    }
  }

  void SyntaxRuleCompiler::parseInlineStyles(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("styles")) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "styles");
    }
    const nlohmann::json& styles_json = root["styles"];
    if (!styles_json.is_array()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles");
    }
    for (const nlohmann::json& style_json : styles_json) {
      if (!style_json.is_object()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles[i]");
      }
      if (!style_json.contains("name")) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "styles[i].name");
      }
      U8String style_name = style_json["name"];
      InlineStyle style;
      if (style_json.contains("foreground")) {
        U8String color_str = style_json["foreground"];
        style.foreground = parseColorSafe(color_str);
      }
      if (style_json.contains("background")) {
        U8String color_str = style_json["background"];
        style.background = parseColorSafe(color_str);
      }
      if (style_json.contains("tags")) {
        const nlohmann::json& tags_json = style_json["tags"];
        if (!tags_json.is_array()) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles[i].tags");
        }
        for (const nlohmann::json& tag_json : tags_json) {
          if (!tag_json.is_string()) {
            throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles[i].tags[i]");
          }
          if (tag_json == "bold") {
            style.is_bold = true;
          }
          if (tag_json == "italic") {
            style.is_italic = true;
          }
          if (tag_json == "strikethrough") {
            style.is_strikethrough = true;
          }
        }
      }
      int32_t style_id = rule->style_mapping->getOrCreateStyleId(style_name);
      rule->inline_styles.insert_or_assign(style_id, std::move(style));
    }
  }

  void SyntaxRuleCompiler::parseStates(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("states")) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "states");
    }
    const nlohmann::json& states_json = root["states"];
    if (!states_json.is_object()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "states");
    }
    for (const auto& item : states_json.items()) {
      const U8String& key = item.key();
      const nlohmann::json& state_json = item.value();
      if (!state_json.is_array()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, key);
      }
      StateRule state_rule;
      state_rule.name = key;
      parseState(rule, state_rule, state_json);
      int32_t state_id = rule->getOrCreateStateId(state_rule.name);
      rule->state_rules_map.insert_or_assign(state_id, std::move(state_rule));
    }
    // states解析完之后将每个token要跳转的state替换成state id
    for (std::pair<const int32_t, StateRule>& pair : rule->state_rules_map) {
      StateRule& state_rule = pair.second;
      for (TokenRule& token_rule : state_rule.token_rules) {
        if (!token_rule.goto_state_str.empty()) {
          token_rule.goto_state = rule->getOrCreateStateId(token_rule.goto_state_str);
          if (!rule->containsRule(token_rule.goto_state)) {
            throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID,
              "state: " + token_rule.goto_state_str);
          }
        }
        // sub_state_strs -> sub_states
        for (const auto& [group, state_name] : token_rule.sub_state_strs) {
          int32_t sub_state_id = rule->getOrCreateStateId(state_name);
          if (!rule->containsRule(sub_state_id)) {
            throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID,
              "subState: " + state_name);
          }
          token_rule.sub_states.insert_or_assign(group, sub_state_id);
        }
      }
      if (!state_rule.line_end_state_str.empty()) {
        state_rule.line_end_state = rule->getOrCreateStateId(state_rule.line_end_state_str);
        if (!rule->containsRule(state_rule.line_end_state)) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID,
            "onLineEndState: " + state_rule.line_end_state_str);
        }
      }
    }
  }

  void SyntaxRuleCompiler::parseState(const SharedPtr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json) {
    for (const nlohmann::json& token_json : state_json) {
      if (!token_json.is_object()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "state element");
      }
      if (token_json.contains("onLineEndState")) {
        const nlohmann::json& line_end_json = token_json["onLineEndState"];
        if (!line_end_json.is_string()) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "onLineEndState");
        }
        state_rule.line_end_state_str = token_json["onLineEndState"];
        continue;
      }
      // importSyntax
      if (token_json.contains("importSyntax")) {
        ImportSyntaxRequest request;
        request.syntax_name = token_json["importSyntax"];
        if (token_json.contains("#ifdef")) {
          request.ifdef_macro = token_json["#ifdef"];
        }
        state_rule.import_requests.push_back(std::move(request));
        continue;
      }
      if (!token_json.contains("pattern")) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "pattern");
      }
      TokenRule token_rule;
      token_rule.pattern = token_json["pattern"];
      // pattern有可能引用变量，进行变量替换
      replaceVariable(token_rule.pattern, rule->variables_map);
      // state
      if (token_json.contains("state")) {
        token_rule.goto_state_str = token_json["state"];
      }
      // style / styles
      if (token_json.contains("style")) {
        U8String style_name = token_json["style"];
        int32_t style_id;
        if (m_inline_style_) {
          style_id = rule->style_mapping->getOrCreateStyleId(style_name);
        } else {
          style_id = m_style_mapping_->getOrCreateStyleId(style_name);
        }
        token_rule.style_ids.insert_or_assign(0, style_id);
      } else if (token_json.contains("styles")) {
        const nlohmann::json& styles_json = token_json["styles"];
        if (!styles_json.is_array()) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles");
        }
        size_t size = styles_json.size();
        if (size % 2 != 0) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "styles elements count % 2 != 0");
        }
        for (size_t i = 0; i < size; i += 2) {
          U8String style_name = styles_json[i + 1];
          int32_t style_id;
          if (m_inline_style_) {
            style_id = rule->style_mapping->getOrCreateStyleId(style_name);
          } else {
            style_id = m_style_mapping_->getOrCreateStyleId(style_name);
          }
          token_rule.style_ids.insert_or_assign(styles_json[i], style_id);
        }
      }
      // subState / subStates
      if (token_json.contains("subState")) {
        U8String sub_state_name = token_json["subState"];
        token_rule.sub_state_strs.insert_or_assign(0, sub_state_name);
      } else if (token_json.contains("subStates")) {
        const nlohmann::json& sub_states_json = token_json["subStates"];
        if (!sub_states_json.is_array()) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "subStates");
        }
        size_t sub_size = sub_states_json.size();
        if (sub_size % 2 != 0) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "subStates elements count % 2 != 0");
        }
        for (size_t i = 0; i < sub_size; i += 2) {
          U8String sub_state_name = sub_states_json[i + 1];
          token_rule.sub_state_strs.insert_or_assign(static_cast<int32_t>(sub_states_json[i].get<int>()), sub_state_name);
        }
      }
      // 至少要有 style 或 subState 其中之一
      if (token_rule.style_ids.empty() && token_rule.sub_state_strs.empty()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "must have style/styles or subState/subStates");
      }
      state_rule.token_rules.push_back(std::move(token_rule));
    }
  }

  void SyntaxRuleCompiler::parseBlockPairs(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("blockPairs")) {
      return;
    }
    const nlohmann::json& block_pairs_json = root["blockPairs"];
    if (!block_pairs_json.is_array()) {
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_INVALID, "blockPairs");
    }
    BlockRule block_rule;
    for (const nlohmann::json& block_pair_json : block_pairs_json) {
      if (!block_pair_json.contains("start")) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "start");
      }
      block_rule.start = block_pair_json["start"];
      if (!block_pair_json.contains("end")) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_JSON_PROPERTY_MISSED, "end");
      }
      block_rule.end = block_pair_json["end"];
      if (block_pair_json.contains("branches")) {
        for (const nlohmann::json& branch_json : block_pair_json["branches"]) {
          block_rule.branch_keywords.emplace(branch_json);
        }
      }
    }
    //rule->first_byte_block_table.insert_or_assign(block_rule.start[0], std::move(block_rule));
    //rule->first_byte_block_table.insert_or_assign(block_rule.end[0], std::move(block_rule));
  }

  void SyntaxRuleCompiler::compileStatePattern(StateRule& state_rule) {
    U8String merged_pattern;
    int32_t total_group_count {0};
    size_t token_size = state_rule.token_rules.size();
    // 将所有token的表达式合成一个大表达式
    for (size_t i = 0; i < token_size; ++i) {
      TokenRule& token_rule = state_rule.token_rules[i];
      // 获取每个pattern的捕获组数量，并检测每个token的pattern是否有错误
      U8String err;
      int group_count = PatternUtil::countCaptureGroups(token_rule.pattern, err);
      if (!err.empty()) {
        throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_PATTERN_INVALID, err + ": " + token_rule.pattern);
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
      throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_PATTERN_INVALID, merged_pattern);
    }
    onig_region_free(region, 1);
    state_rule.merged_pattern = std::move(merged_pattern);
  }

  void SyntaxRuleCompiler::processImportSyntaxRequests(const SharedPtr<SyntaxRule>& rule) {
    for (auto& [state_id, state_rule] : rule->state_rules_map) {
      if (state_rule.import_requests.empty()) {
        continue;
      }
      for (const ImportSyntaxRequest& request : state_rule.import_requests) {
        // #ifdef
        if (!request.ifdef_macro.empty()) {
          if (m_engine_ == nullptr || !m_engine_->isMacroDefined(request.ifdef_macro)) {
            continue;
          }
        }
        if (m_engine_ == nullptr) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_STATE_INVALID,
            "importSyntax requires HighlightEngine, syntax: " + request.syntax_name);
        }
        SharedPtr<SyntaxRule> source_rule = m_engine_->getSyntaxRuleByName(request.syntax_name);
        if (source_rule == nullptr) {
          throw SyntaxRuleParseError(SyntaxRuleParseError::ERR_STATE_INVALID,
            "importSyntax not found: " + request.syntax_name);
        }
        importSyntaxRule(rule, state_id, source_rule);
      }
      state_rule.import_requests.clear();
    }
  }

  void SyntaxRuleCompiler::importSyntaxRule(const SharedPtr<SyntaxRule>& target_rule, int32_t target_state_id,
    const SharedPtr<SyntaxRule>& source_rule) {
    // offset为当前target_rule中最大的state id + 1
    int32_t max_state_id = 0;
    for (const auto& [id, _] : target_rule->state_rules_map) {
      if (id > max_state_id) {
        max_state_id = id;
      }
    }
    // 也检查state_id_map中预分配的id
    for (const auto& [_, id] : target_rule->state_id_map) {
      if (id > max_state_id) {
        max_state_id = id;
      }
    }
    int32_t state_id_offset = max_state_id + 1;

    // importSyntax中default state的TokenRule合并到当前state
    auto source_default_it = source_rule->state_rules_map.find(SyntaxRule::kDefaultStateId);
    if (source_default_it != source_rule->state_rules_map.end()) {
      const StateRule& source_default = source_default_it->second;
      StateRule& target_state = target_rule->getStateRule(target_state_id);
      for (const TokenRule& src_token : source_default.token_rules) {
        TokenRule new_token = src_token;
        // goto_state偏移
        if (new_token.goto_state >= 0) {
          if (new_token.goto_state == SyntaxRule::kDefaultStateId) {
            new_token.goto_state = target_state_id;
          } else {
            new_token.goto_state += state_id_offset;
          }
        }
        // sub_states偏移
        for (auto& [group, sub_state_id] : new_token.sub_states) {
          if (sub_state_id == SyntaxRule::kDefaultStateId) {
            sub_state_id = target_state_id;
          } else {
            sub_state_id += state_id_offset;
          }
        }
        // 清除goto_state_str和sub_state_strs（已经是编译后的id）
        new_token.goto_state_str.clear();
        new_token.sub_state_strs.clear();
        target_state.token_rules.push_back(std::move(new_token));
      }
    }

    // importSyntax其他state搬到target_rule，state_id加偏移
    for (const auto& [source_state_id, source_state_rule] : source_rule->state_rules_map) {
      if (source_state_id == SyntaxRule::kDefaultStateId) {
        continue;
      }
      int32_t new_state_id = source_state_id + state_id_offset;
      StateRule new_state = source_state_rule;
      // 修正每个TokenRule的goto_state
      for (TokenRule& token_rule : new_state.token_rules) {
        if (token_rule.goto_state >= 0) {
          if (token_rule.goto_state == SyntaxRule::kDefaultStateId) {
            token_rule.goto_state = target_state_id; // goto default → goto当前state
          } else {
            token_rule.goto_state += state_id_offset;
          }
        }
        // sub_states偏移
        for (auto& [group, sub_state_id] : token_rule.sub_states) {
          if (sub_state_id == SyntaxRule::kDefaultStateId) {
            sub_state_id = target_state_id;
          } else {
            sub_state_id += state_id_offset;
          }
        }
        token_rule.goto_state_str.clear();
        token_rule.sub_state_strs.clear();
      }
      // 修正line_end_state
      if (new_state.line_end_state >= 0) {
        if (new_state.line_end_state == SyntaxRule::kDefaultStateId) {
          new_state.line_end_state = target_state_id;
        } else {
          new_state.line_end_state += state_id_offset;
        }
      }
      new_state.line_end_state_str.clear();

      U8String new_state_name = "__imported_" + source_rule->name + "_" + source_state_rule.name;
      target_rule->state_id_map.insert_or_assign(new_state_name, new_state_id);
      target_rule->state_rules_map.insert_or_assign(new_state_id, std::move(new_state));
    }
  }

  void SyntaxRuleCompiler::replaceVariable(U8String& text, HashMap<U8String, U8String>& variables_map) {
    for (const std::pair<const U8String, U8String>& pair : variables_map) {
      text = StrUtil::replaceAll(text, "${" + pair.first + "}", pair.second);
    }
  }

  int32_t SyntaxRuleCompiler::parseColorSafe(const U8String& color_str) {
    if (color_str.empty()) {
      return 0;
    }
    const char* str = color_str.c_str();
    if (*str == '#') {
      str++;
    }
    size_t len = 0;
    const char* ptr = str;
    while(*ptr) {
      len++;
      ptr++;
    }
    char* end_ptr;
    uint32_t color = strtoul(str, &end_ptr, 16);
    // 检查是否解析了整个字符串（end_ptr 应该指向字符串结尾）
    if (*end_ptr != '\0') {
      // 解析失败，或者包含非十六进制字符
      return 0;
    }
    if (len == 6) {
      color |= 0xFF000000;
    } else if (len != 8) {
      return 0;
    }
    return static_cast<int32_t>(color);
  }
}
