#ifdef SWEETLINE_DEBUG
#include <iostream>
#endif
#include <algorithm>

#include "internal_syntax.h"
#include "sweetline/highlight.h"
#include "sweetline/util.h"

namespace NS_SWEETLINE {
  struct SyntaxRuleRuntimeData {
    List<OnigRegex> file_name_pattern_regexes;
  };

  namespace {
    bool isScopeSkipStyleName(const U8String& style_name) {
      return style_name.find("string") != U8String::npos
        || style_name.find("comment") != U8String::npos
        || style_name.find("char") != U8String::npos;
    }

    OnigRegex compileRegexOrThrow(const U8String& pattern_text, const U8String& error_context) {
      OnigRegex regex = nullptr;
      OnigErrorInfo error_info;
      OnigUChar error_buf[ONIG_MAX_ERROR_MESSAGE_LEN];
      OnigUChar* pattern = reinterpret_cast<OnigUChar*>(const_cast<char*>(pattern_text.c_str()));
      OnigUChar* pattern_end = pattern + pattern_text.size();
      int status = onig_new(&regex, pattern, pattern_end,
        ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &error_info);
      if (status != ONIG_NORMAL) {
        onig_error_code_to_str(error_buf, status, &error_info);
        throw SyntaxCompileError(SyntaxCompileError::ERR_PATTERN_INVALID,
          error_context + " (" + reinterpret_cast<const char*>(error_buf) + ")");
      }
      return regex;
    }

    bool matchesRegex(OnigRegex regex, const U8String& text) {
      if (regex == nullptr) {
        return false;
      }
      OnigRegion* region = onig_region_new();
      OnigUChar* text_ptr = reinterpret_cast<OnigUChar*>(const_cast<char*>(text.c_str()));
      OnigUChar* text_end = text_ptr + text.size();
      int status = onig_search(regex, text_ptr, text_end, text_ptr, text_end, region, ONIG_OPTION_NONE);
      onig_region_free(region, 1);
      return status >= 0;
    }

    void freeRegex(OnigRegex regex) {
      if (regex != nullptr) {
        onig_free(regex);
      }
    }

    void resetCompiledTokenRuleRuntime(TokenRule& token_rule) {
      token_rule.group_count = 0;
      token_rule.group_offset_start = 0;
    }

    void clearCompiledStateRuntime(StateRule& state_rule) {
      state_rule.regex = nullptr;
      state_rule.group_count = 0;
      state_rule.merged_pattern.clear();
      for (TokenRule& token_rule : state_rule.token_rules) {
        resetCompiledTokenRuleRuntime(token_rule);
      }
    }

    int32_t resolveInlineStyleIdOrThrow(const SharedPtr<SyntaxRule>& rule,
      const U8String& style_name, const U8String& property_name) {
      auto it = rule->style_mapping->style_name_id_map.find(style_name);
      if (it == rule->style_mapping->style_name_id_map.end()
        || rule->inline_styles.find(it->second) == rule->inline_styles.end()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_INLINE_STYLE_REFERENCE_NOT_FOUND,
          property_name + ": undefined inline style " + style_name);
      }
      return it->second;
    }
  }

 // ===================================== SyntaxCompileError ============================================
  SyntaxCompileError::SyntaxCompileError(const int err_code): m_err_code_(err_code) {
  }

  SyntaxCompileError::SyntaxCompileError(const int err_code, const U8String& message): m_err_code_(err_code), m_message_(message) {
  }

  SyntaxCompileError::SyntaxCompileError(const int err_code, const char* message): m_err_code_(err_code), m_message_(message) {
  }

  const char* SyntaxCompileError::what() const noexcept {
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
    case ERR_FILE_NOT_EXISTS:
      return "File not exists";
    case ERR_FILE_INVALID:
      return "File invalid";
    case ERR_IMPORT_SYNTAX_NOT_FOUND:
      return "Import syntax not found";
    case ERR_STATE_REFERENCE_NOT_FOUND:
      return "State reference not found";
    case ERR_INLINE_STYLE_REFERENCE_NOT_FOUND:
      return "Inline style reference not found";
    default:
      return "Unknown error";
    }
  }

  const U8String& SyntaxCompileError::message() const noexcept {
    return m_message_;
  }

  int SyntaxCompileError::code() const noexcept {
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
    if (style_id <= 0) {
      return false;
    }
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

  bool SyntaxRule::matchesFileNamePattern(const U8String& file_name, size_t index) const {
    if (m_runtime_data_ == nullptr || index >= m_runtime_data_->file_name_pattern_regexes.size()) {
      return false;
    }
    return matchesRegex(m_runtime_data_->file_name_pattern_regexes[index], file_name);
  }

  SyntaxRule::SyntaxRule(): m_runtime_data_(makeUniquePtr<SyntaxRuleRuntimeData>()) {
    state_id_map.insert_or_assign(kDefaultStateName, kDefaultStateId);
  }

  SyntaxRule::~SyntaxRule() {
    for (auto& [state_id, state_rule] : state_rules_map) {
      freeRegex(state_rule.regex);
      state_rule.regex = nullptr;
    }
    if (m_runtime_data_ == nullptr) {
      return;
    }
    for (OnigRegex regex : m_runtime_data_->file_name_pattern_regexes) {
      freeRegex(regex);
    }
    m_runtime_data_->file_name_pattern_regexes.clear();
  }

#ifdef SWEETLINE_DEBUG
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SyntaxRule, name, file_names, file_suffixes, file_name_patterns,
    variables_map, state_rules_map, state_id_map);
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
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_INVALID, e.what());
    }
    parseSyntaxName(syntax_rule, root);
    parseFileNames(syntax_rule, root);
    parseFileSuffixes(syntax_rule, root);
    parseFileNamePatterns(syntax_rule, root);
    if (m_inline_style_) {
      syntax_rule->style_mapping = makeUniquePtr<StyleMapping>();
      parseInlineStyles(syntax_rule, root);
    }
    parseVariables(syntax_rule, root);
    parseStates(syntax_rule, root);
    // Process importSyntax requests (after state id parsing, before compiling merged patterns)
    processImportSyntaxRequests(syntax_rule);
    compileFileNamePatterns(syntax_rule);
    // Compile each state into a single merged regex pattern
    for (std::pair<const int32_t, StateRule>& pair : syntax_rule->state_rules_map) {
      compileStatePattern(pair.second);
    }
    // Parse scope rules for indent guides
    parseScopeRules(syntax_rule, root);
#ifdef SWEETLINE_DEBUG
    //syntax_rule->dump();
#endif
    return syntax_rule;
  }

  SharedPtr<SyntaxRule> SyntaxRuleCompiler::compileSyntaxFromFile(const U8String& file) {
    if (!FileUtil::isFile(file)) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_FILE_NOT_EXISTS, "File does not exist: " + file);
    }
    U8String content = FileUtil::readString(file);
    if (content.empty()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_FILE_INVALID, "File invalid: " + file);
    }
    return compileSyntaxFromJson(content);
  }

  void SyntaxRuleCompiler::parseSyntaxName(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("name")) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "name");
    }
    nlohmann::json name_json = root["name"];
    if (name_json.is_string()) {
      rule->name = name_json;
    } else {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "name");
    }
  }

  void SyntaxRuleCompiler::parseFileNames(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (root.contains("fileNames")) {
      nlohmann::json file_names_json = root["fileNames"];
      if (!file_names_json.is_array()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileNames");
      }
      for (const nlohmann::json& element_json : file_names_json) {
        if (element_json.is_string()) {
          rule->file_names.emplace(element_json);
        } else {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileNames");
        }
      }
    }
    if (root.contains("fileName")) {
      nlohmann::json file_name_json = root["fileName"];
      if (file_name_json.is_string()) {
        rule->file_names.emplace(file_name_json);
      } else {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileName");
      }
    }
  }

  void SyntaxRuleCompiler::parseFileSuffixes(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (root.contains("fileSuffixes")) {
      nlohmann::json suffixes_json = root["fileSuffixes"];
      if (!suffixes_json.is_array()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileSuffixes");
      }
      for (const nlohmann::json& element_json : suffixes_json) {
        if (!element_json.is_string()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileSuffixes");
        }
        U8String suffix = element_json;
        if (suffix.empty()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileSuffixes");
        }
        rule->file_suffixes.emplace(suffix);
      }
    }
    if (root.contains("fileSuffix")) {
      nlohmann::json suffix_json = root["fileSuffix"];
      if (!suffix_json.is_string()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileSuffix");
      }
      U8String suffix = suffix_json;
      if (suffix.empty()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileSuffix");
      }
      rule->file_suffixes.emplace(suffix);
    }
  }

  void SyntaxRuleCompiler::parseFileNamePatterns(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    auto append_pattern = [&rule](const U8String& property_name, const nlohmann::json& pattern_json) {
      if (!pattern_json.is_string()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, property_name);
      }
      U8String pattern_text = pattern_json;
      U8String error;
      PatternUtil::countCaptureGroups("\\A(?:" + pattern_text + ")\\z", error);
      if (!error.empty()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_PATTERN_INVALID,
          property_name + ": " + pattern_text + " (" + error + ")");
      }
      rule->file_name_patterns.emplace_back(pattern_text);
    };
    if (root.contains("fileNamePatterns")) {
      nlohmann::json patterns_json = root["fileNamePatterns"];
      if (!patterns_json.is_array()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fileNamePatterns");
      }
      for (const nlohmann::json& element_json : patterns_json) {
        append_pattern("fileNamePatterns", element_json);
      }
    }
    if (root.contains("fileNamePattern")) {
      append_pattern("fileNamePattern", root["fileNamePattern"]);
    }
    if (rule->file_names.empty() && rule->file_suffixes.empty() && rule->file_name_patterns.empty()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED,
        "fileName/fileNames or fileSuffix/fileSuffixes or fileNamePattern/fileNamePatterns");
    }
  }

  void SyntaxRuleCompiler::compileFileNamePatterns(const SharedPtr<SyntaxRule>& rule) {
    if (rule == nullptr || rule->m_runtime_data_ == nullptr) {
      return;
    }
    rule->m_runtime_data_->file_name_pattern_regexes.clear();
    for (const U8String& pattern_text : rule->file_name_patterns) {
      const U8String full_pattern = "\\A(?:" + pattern_text + ")\\z";
      rule->m_runtime_data_->file_name_pattern_regexes.push_back(
        compileRegexOrThrow(full_pattern, "fileNamePattern: " + pattern_text));
    }
  }

  void SyntaxRuleCompiler::parseVariables(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("variables")) {
      return;
    }
    nlohmann::json variables_json = root["variables"];
    if (!variables_json.is_object()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "variables");
    }
    for (const auto& item : variables_json.items()) {
      const U8String& key = item.key();
      const nlohmann::json& variable_json = item.value();
      if (!variable_json.is_string()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, key);
      }
      rule->variables_map.insert_or_assign(key, variable_json);
    }
    // Variables may reference other variables, so replace all variable references after iteration
    for (std::pair<const U8String, U8String>& pair : rule->variables_map) {
      replaceVariable(pair.second, rule->variables_map);
    }
  }

  void SyntaxRuleCompiler::parseInlineStyles(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("styles")) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "styles");
    }
    const nlohmann::json& styles_json = root["styles"];
    if (!styles_json.is_array()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles");
    }
    for (const nlohmann::json& style_json : styles_json) {
      if (!style_json.is_object()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles[i]");
      }
      if (!style_json.contains("name")) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "styles[i].name");
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
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles[i].tags");
        }
        for (const nlohmann::json& tag_json : tags_json) {
          if (!tag_json.is_string()) {
            throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles[i].tags[i]");
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

  U8String SyntaxRuleCompiler::makeJsonPath(const U8String& parent_path, const size_t index) {
    return parent_path + "[" + std::to_string(index) + "]";
  }

  void SyntaxRuleCompiler::appendFragmentEntries(const U8String& fragment_name, const U8String& reference_path,
    const FragmentMap& fragments, List<U8String>& include_stack, nlohmann::json& expanded_entries) {
    auto it = fragments.find(fragment_name);
    if (it == fragments.end()) {
      throw SyntaxCompileError(
        SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
        "fragment not found: " + fragment_name + " at " + reference_path);
    }
    auto stack_it = std::find(include_stack.begin(), include_stack.end(), fragment_name);
    if (stack_it != include_stack.end()) {
      U8String chain;
      for (auto iter = stack_it; iter != include_stack.end(); ++iter) {
        if (!chain.empty()) {
          chain += " -> ";
        }
        chain += *iter;
      }
      if (!chain.empty()) {
        chain += " -> ";
      }
      chain += fragment_name;
      throw SyntaxCompileError(
        SyntaxCompileError::ERR_STATE_INVALID,
        "circular fragments include: " + chain);
    }
    include_stack.push_back(fragment_name);
    resolveIncludesInEntries(it->second, "fragments." + fragment_name, fragments, include_stack, expanded_entries);
    include_stack.pop_back();
  }

  void SyntaxRuleCompiler::resolveIncludesInEntries(const nlohmann::json& entries_json, const U8String& source_path,
    const FragmentMap& fragments, List<U8String>& include_stack, nlohmann::json& expanded_entries) {
    if (!entries_json.is_array()) {
      throw SyntaxCompileError(
        SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
        source_path + " must be an array");
    }
    for (size_t i = 0; i < entries_json.size(); ++i) {
      const nlohmann::json& entry_json = entries_json[i];
      const U8String entry_path = makeJsonPath(source_path, i);
      if (!entry_json.is_object()) {
        throw SyntaxCompileError(
          SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
          entry_path + " must be an object");
      }
      const bool has_include = entry_json.contains("include");
      const bool has_includes = entry_json.contains("includes");
      if (!has_include && !has_includes) {
        expanded_entries.push_back(entry_json);
        continue;
      }
      if (has_include && has_includes) {
        throw SyntaxCompileError(
          SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
          entry_path + " cannot contain both include and includes");
      }
      if (entry_json.size() != 1) {
        throw SyntaxCompileError(
          SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
          entry_path + " include/includes rule must not contain other fields");
      }
      if (has_include) {
        const nlohmann::json& include_json = entry_json["include"];
        if (!include_json.is_string()) {
          throw SyntaxCompileError(
            SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
            entry_path + ".include must be a string");
        }
        appendFragmentEntries(include_json.get<U8String>(), entry_path + ".include", fragments, include_stack, expanded_entries);
        continue;
      }
      const nlohmann::json& includes_json = entry_json["includes"];
      if (!includes_json.is_array()) {
        throw SyntaxCompileError(
          SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
          entry_path + ".includes must be an array");
      }
      for (size_t j = 0; j < includes_json.size(); ++j) {
        if (!includes_json[j].is_string()) {
          throw SyntaxCompileError(
            SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
            makeJsonPath(entry_path + ".includes", j) + " must be a string");
        }
        appendFragmentEntries(includes_json[j].get<U8String>(),
          makeJsonPath(entry_path + ".includes", j), fragments, include_stack, expanded_entries);
      }
    }
  }

  SyntaxRuleCompiler::FragmentMap SyntaxRuleCompiler::collectFragments(nlohmann::json& root) {
    FragmentMap fragments;
    if (!root.contains("fragments")) {
      return fragments;
    }
    const nlohmann::json& fragments_json = root["fragments"];
    if (!fragments_json.is_object()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "fragments");
    }
    for (const auto& item : fragments_json.items()) {
      if (!item.value().is_array()) {
        throw SyntaxCompileError(
          SyntaxCompileError::ERR_JSON_PROPERTY_INVALID,
          "fragments." + item.key());
      }
      fragments.insert_or_assign(item.key(), item.value());
    }
    return fragments;
  }

  void SyntaxRuleCompiler::parseStates(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("states")) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "states");
    }
    FragmentMap fragments = collectFragments(root);
    const nlohmann::json& states_json = root["states"];
    if (!states_json.is_object()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "states");
    }
    for (const auto& item : states_json.items()) {
      const U8String& key = item.key();
      const nlohmann::json& state_json = item.value();
      if (!state_json.is_array()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, key);
      }
      StateRule state_rule;
      state_rule.name = key;
      nlohmann::json expanded_state_json = nlohmann::json::array();
      List<U8String> include_stack;
      resolveIncludesInEntries(state_json, "states." + key, fragments, include_stack, expanded_state_json);
      parseState(rule, state_rule, expanded_state_json);
      int32_t state_id = rule->getOrCreateStateId(state_rule.name);
      rule->state_rules_map.insert_or_assign(state_id, std::move(state_rule));
    }
    // After states parsing, replace goto state strings with state IDs for each token
    for (std::pair<const int32_t, StateRule>& pair : rule->state_rules_map) {
      StateRule& state_rule = pair.second;
      for (TokenRule& token_rule : state_rule.token_rules) {
        if (!token_rule.goto_state_str.empty()) {
          token_rule.goto_state = rule->getOrCreateStateId(token_rule.goto_state_str);
          if (!rule->containsRule(token_rule.goto_state)) {
            throw SyntaxCompileError(SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND,
              "state: " + token_rule.goto_state_str);
          }
        }
        // sub_state_strs -> sub_states
        for (const auto& [group, state_name] : token_rule.sub_state_strs) {
          int32_t sub_state_id = rule->getOrCreateStateId(state_name);
          if (!rule->containsRule(sub_state_id)) {
            throw SyntaxCompileError(SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND,
              "subState: " + state_name);
          }
          token_rule.sub_states.insert_or_assign(group, sub_state_id);
        }
      }
      if (!state_rule.line_end_state_str.empty()) {
        state_rule.line_end_state = rule->getOrCreateStateId(state_rule.line_end_state_str);
        if (!rule->containsRule(state_rule.line_end_state)) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_STATE_REFERENCE_NOT_FOUND,
            "onLineEndState: " + state_rule.line_end_state_str);
        }
      }
    }
  }

  void SyntaxRuleCompiler::parseState(const SharedPtr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json) {
    for (const nlohmann::json& token_json : state_json) {
      if (!token_json.is_object()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "state element");
      }
      if (token_json.contains("onLineEndState")) {
        const nlohmann::json& line_end_json = token_json["onLineEndState"];
        if (!line_end_json.is_string()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "onLineEndState");
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
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "pattern");
      }
      TokenRule token_rule;
      token_rule.pattern = token_json["pattern"];
      // Pattern may reference variables, perform variable substitution
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
          style_id = resolveInlineStyleIdOrThrow(rule, style_name, "style");
        } else {
          style_id = m_style_mapping_->getOrCreateStyleId(style_name);
        }
        token_rule.style_ids.insert_or_assign(0, style_id);
        if (isScopeSkipStyleName(style_name)) {
          rule->scope_skip_style_ids.emplace(style_id);
        }
      } else if (token_json.contains("styles")) {
        const nlohmann::json& styles_json = token_json["styles"];
        if (!styles_json.is_array()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles");
        }
        size_t size = styles_json.size();
        if (size % 2 != 0) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "styles elements count % 2 != 0");
        }
        for (size_t i = 0; i < size; i += 2) {
          U8String style_name = styles_json[i + 1];
          int32_t style_id;
          if (m_inline_style_) {
            style_id = resolveInlineStyleIdOrThrow(rule, style_name, "styles");
          } else {
            style_id = m_style_mapping_->getOrCreateStyleId(style_name);
          }
          token_rule.style_ids.insert_or_assign(styles_json[i], style_id);
          if (isScopeSkipStyleName(style_name)) {
            rule->scope_skip_style_ids.emplace(style_id);
          }
        }
      }
      // subState / subStates
      if (token_json.contains("subState")) {
        U8String sub_state_name = token_json["subState"];
        token_rule.sub_state_strs.insert_or_assign(0, sub_state_name);
      } else if (token_json.contains("subStates")) {
        const nlohmann::json& sub_states_json = token_json["subStates"];
        if (!sub_states_json.is_array()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "subStates");
        }
        size_t sub_size = sub_states_json.size();
        if (sub_size % 2 != 0) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "subStates elements count % 2 != 0");
        }
        for (size_t i = 0; i < sub_size; i += 2) {
          U8String sub_state_name = sub_states_json[i + 1];
          token_rule.sub_state_strs.insert_or_assign(static_cast<int32_t>(sub_states_json[i].get<int>()), sub_state_name);
        }
      }
      // Must have at least one of style or subState
      if (token_rule.style_ids.empty() && token_rule.sub_state_strs.empty()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "must have style/styles or subState/subStates");
      }
      state_rule.token_rules.push_back(std::move(token_rule));
    }
  }

  void SyntaxRuleCompiler::parseScopeRules(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root) {
    if (!root.contains("scopeRules")) {
      return;
    }
    const nlohmann::json& scope_rules_json = root["scopeRules"];
    if (!scope_rules_json.is_array()) {
      throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules");
    }
    int32_t rule_id_counter = 0;
    for (const nlohmann::json& scope_rule_json : scope_rules_json) {
      if (!scope_rule_json.is_object()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules[]");
      }
      ScopeRule scope_rule;
      if (!scope_rule_json.contains("start")) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "scopeRules[].start");
      }
      if (!scope_rule_json["start"].is_string()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules[].start");
      }
      scope_rule.start = scope_rule_json["start"];
      if (!scope_rule_json.contains("end")) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_MISSED, "scopeRules[].end");
      }
      if (!scope_rule_json["end"].is_string()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules[].end");
      }
      scope_rule.end = scope_rule_json["end"];
      if (scope_rule_json.contains("branches")) {
        if (!scope_rule_json["branches"].is_array()) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules[].branches");
        }
        for (const nlohmann::json& branch_json : scope_rule_json["branches"]) {
          if (!branch_json.is_string()) {
            throw SyntaxCompileError(SyntaxCompileError::ERR_JSON_PROPERTY_INVALID, "scopeRules[].branches[]");
          }
          scope_rule.branch_keywords.emplace(branch_json);
        }
      }
      scope_rule.rule_id = rule_id_counter;
      rule->scope_rules_map.insert_or_assign(rule_id_counter, std::move(scope_rule));
      rule_id_counter++;
    }
  }

  void SyntaxRuleCompiler::compileStatePattern(StateRule& state_rule) {
    freeRegex(state_rule.regex);
    state_rule.regex = nullptr;
    U8String merged_pattern;
    int32_t total_group_count {0};
    size_t token_size = state_rule.token_rules.size();
    // Merge all token patterns into one combined regex pattern
    for (size_t i = 0; i < token_size; ++i) {
      TokenRule& token_rule = state_rule.token_rules[i];
      // Get capture group count for each pattern and validate token patterns
      U8String err;
      int group_count = PatternUtil::countCaptureGroups(token_rule.pattern, err);
      if (!err.empty()) {
        throw SyntaxCompileError(SyntaxCompileError::ERR_PATTERN_INVALID, err + ": " + token_rule.pattern);
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
    state_rule.regex = compileRegexOrThrow(merged_pattern, merged_pattern);
    state_rule.merged_pattern = std::move(merged_pattern);
  }

  void SyntaxRuleCompiler::processImportSyntaxRequests(const SharedPtr<SyntaxRule>& rule) {
    struct PendingImportState {
      int32_t state_id {SyntaxRule::kDefaultStateId};
      List<ImportSyntaxRequest> requests;
    };

    List<PendingImportState> pending_states;
    pending_states.reserve(rule->state_rules_map.size());
    for (auto& [state_id, state_rule] : rule->state_rules_map) {
      if (state_rule.import_requests.empty()) {
        continue;
      }
      PendingImportState pending_state;
      pending_state.state_id = state_id;
      pending_state.requests = state_rule.import_requests;
      pending_states.push_back(std::move(pending_state));
      state_rule.import_requests.clear();
    }

    for (const PendingImportState& pending_state : pending_states) {
      for (const ImportSyntaxRequest& request : pending_state.requests) {
        // #ifdef
        if (!request.ifdef_macro.empty()) {
          if (m_engine_ == nullptr || !m_engine_->isMacroDefined(request.ifdef_macro)) {
            continue;
          }
        }
        if (m_engine_ == nullptr) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_STATE_INVALID,
            "importSyntax requires HighlightEngine, syntax: " + request.syntax_name);
        }
        SharedPtr<SyntaxRule> source_rule = m_engine_->getSyntaxRuleByName(request.syntax_name);
        if (source_rule == nullptr) {
          throw SyntaxCompileError(SyntaxCompileError::ERR_IMPORT_SYNTAX_NOT_FOUND,
            "importSyntax not found: " + request.syntax_name);
        }
        importSyntaxRule(rule, pending_state.state_id, source_rule);
      }
    }
  }

  void SyntaxRuleCompiler::importSyntaxRule(const SharedPtr<SyntaxRule>& target_rule, int32_t target_state_id,
    const SharedPtr<SyntaxRule>& source_rule) {
    // Offset is the current max state id + 1 in target_rule
    int32_t max_state_id = 0;
    for (const auto& [id, _] : target_rule->state_rules_map) {
      if (id > max_state_id) {
        max_state_id = id;
      }
    }
    // Also check pre-allocated IDs in state_id_map
    for (const auto& [_, id] : target_rule->state_id_map) {
      if (id > max_state_id) {
        max_state_id = id;
      }
    }
    int32_t state_id_offset = max_state_id + 1;

    // Max style id tracking
    int32_t max_style_id = 0;
    int32_t style_id_offset = 0;
    if (m_inline_style_) {
      for (const auto& [id, _] : target_rule->style_mapping->style_id_name_map) {
        if (id > max_state_id) {
          max_style_id = id;
        }
      }
      style_id_offset = max_style_id;
      for (auto& [id, inline_style] : source_rule->inline_styles) {
        target_rule->inline_styles.emplace(id + style_id_offset, inline_style);
      }
    }

    // Merge default state TokenRules from importSyntax into the current state
    auto source_default_it = source_rule->state_rules_map.find(SyntaxRule::kDefaultStateId);
    if (source_default_it != source_rule->state_rules_map.end()) {
      const StateRule& source_default = source_default_it->second;
      StateRule& target_state = target_rule->getStateRule(target_state_id);
      for (const TokenRule& src_token : source_default.token_rules) {
        TokenRule new_token = src_token;
        resetCompiledTokenRuleRuntime(new_token);
        // goto_state offset
        if (new_token.goto_state >= 0) {
          if (new_token.goto_state == SyntaxRule::kDefaultStateId) {
            new_token.goto_state = target_state_id;
          } else {
            new_token.goto_state += state_id_offset;
          }
        }
        // sub_states offset
        for (auto& [group, sub_state_id] : new_token.sub_states) {
          if (sub_state_id == SyntaxRule::kDefaultStateId) {
            sub_state_id = target_state_id;
          } else {
            sub_state_id += state_id_offset;
          }
        }
        // style id offset
        if (m_inline_style_) {
          for (auto& [_, style_id] : new_token.style_ids) {
            style_id += style_id_offset;
          }
        }
        // Clear goto_state_str and sub_state_strs (already compiled to IDs)
        new_token.goto_state_str.clear();
        new_token.sub_state_strs.clear();
        target_state.token_rules.push_back(std::move(new_token));
      }
    }

    // Move other states from importSyntax to target_rule with state_id offset
    for (const auto& [source_state_id, source_state_rule] : source_rule->state_rules_map) {
      if (source_state_id == SyntaxRule::kDefaultStateId) {
        continue;
      }
      int32_t new_state_id = source_state_id + state_id_offset;
      StateRule new_state = source_state_rule;
      clearCompiledStateRuntime(new_state);
      for (TokenRule& token_rule : new_state.token_rules) {
        // Fix goto_state for each TokenRule
        if (token_rule.goto_state >= 0) {
          if (token_rule.goto_state == SyntaxRule::kDefaultStateId) {
            token_rule.goto_state = target_state_id; // goto default -> goto current state
          } else {
            token_rule.goto_state += state_id_offset;
          }
        }
        // sub_states offset
        for (auto& [group, sub_state_id] : token_rule.sub_states) {
          if (sub_state_id == SyntaxRule::kDefaultStateId) {
            sub_state_id = target_state_id;
          } else {
            sub_state_id += state_id_offset;
          }
        }
        // style id offset
        if (m_inline_style_) {
          for (auto& [_, style_id] : token_rule.style_ids) {
            style_id += style_id_offset;
          }
        }
        token_rule.goto_state_str.clear();
        token_rule.sub_state_strs.clear();
      }
      // Fix line_end_state
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
    // Check if the entire string was parsed (end_ptr should point to end of string)
    if (*end_ptr != '\0') {
      // Parse failed, or contains non-hexadecimal characters
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
