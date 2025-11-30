#ifndef SWEETLINE_INTERNAL_SYNTAX_H
#define SWEETLINE_INTERNAL_SYNTAX_H

#include <cstdint>
#include <oniguruma/oniguruma.h>
#include <nlohmann/json.hpp>
#include "syntax.h"

namespace NS_SWEETLINE {
  /// 匹配分析的最小单元(token)的规则
  struct TokenRule {
    /// 正则表达式
    String pattern;
    /// 按捕获组区分的高亮样式
    HashMap<std::int32_t, int32_t> styles;
    /// Json解析到的跳转state文本
    String goto_state_str;
    /// token包含的正则表达式捕获组数量
    int32_t group_count {0};
    /// token的捕获组在大表达式中的group偏移
    int32_t group_offset_start {0};
    /// 要跳转的state
    int32_t goto_state {-1};

    int32_t getGroupStyle(int32_t group) const;

    static int32_t kDefaultStyle;
    static TokenRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 每个state的规则
  struct StateRule {
    /// state名称
    String name;
    /// 每个token的规则
    List<TokenRule> token_rules;
    /// 每个token的表达式合并的大表达式
    String merged_pattern;
    /// 编译后的正则表达式指针
    OnigRegex regex;
    /// 合并后大表达式的总捕获组数量
    int32_t group_count {0};

    static StateRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 语法规则编译器
  class SyntaxRuleCompiler {
  public:
    explicit SyntaxRuleCompiler(const Ptr<StyleMapping>& style_mapping);

    /// 通过json编译语法规则
    /// @param json 语法规则文件的json
    Ptr<SyntaxRule> compileSyntaxFromJson(const String& json);

    /// 编译语法规则
    /// @param file 语法规则定义文件(json)
    Ptr<SyntaxRule> compileSyntaxFromFile(const String& file);
  private:
    Ptr<StyleMapping> style_mapping_;
    static void parseSyntaxName(const Ptr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseFileExtensions(const Ptr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseVariables(const Ptr<SyntaxRule>& rule, nlohmann::json& root);
    void parseStates(const Ptr<SyntaxRule>& rule, nlohmann::json& root);
    void parseState(const Ptr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json);
    static void compileStatePattern(StateRule& state_rule);
    static void replaceVariable(String& text, HashMap<String, String>& variables_map);
  };
}

#endif //SWEETLINE_INTERNAL_SYNTAX_H