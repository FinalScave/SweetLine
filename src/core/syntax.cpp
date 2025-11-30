#ifdef SWEETLINE_DEBUG
#include <iostream>
#endif

#include "internal_syntax.h"
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
#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenRule, pattern, styles, goto_state_str, group_count, group_offset_start, goto_state);
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

#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SyntaxRule, name, file_extensions_, variables_map_, state_rules_map_, state_id_map_);
  void SyntaxRule::dump() const {
    nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

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
#ifdef SWEETLINE_DEBUG
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
}