#ifndef SWEETLINE_INTERNAL_SYNTAX_H
#define SWEETLINE_INTERNAL_SYNTAX_H

#include <cstdint>
#include <oniguruma/oniguruma.h>
#include <nlohmann/json.hpp>
#include "syntax.h"

namespace NS_SWEETLINE {
  class HighlightEngine;
}

namespace NS_SWEETLINE {
  /// 匹配分析的最小单元(token)的规则
  struct TokenRule {
    /// 正则表达式
    U8String pattern;
    /// 按捕获组区分的高亮样式ID (没有指定inlineStyle时使用)
    HashMap<int32_t, int32_t> style_ids;
    /// Json解析到的跳转state文本
    U8String goto_state_str;
    /// token包含的正则表达式捕获组数量
    int32_t group_count {0};
    /// token的捕获组在大表达式中的group偏移
    int32_t group_offset_start {0};
    /// 要跳转的state
    int32_t goto_state {-1};

    /// 按捕获组指定subState名称
    HashMap<int32_t, U8String> sub_state_strs;
    /// 按捕获组指定subState ID
    HashMap<int32_t, int32_t> sub_states;

    int32_t getGroupStyleId(int32_t group) const;
    int32_t getGroupSubState(int32_t group) const;

    static int32_t kDefaultStyleId;
    static TokenRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// importSyntax请求（解析阶段记录，编译完成后处理）
  struct ImportSyntaxRequest {
    /// 要导入的语法规则名称
    U8String syntax_name;
    /// #ifdef 宏条件（为空表示无条件导入）
    U8String ifdef_macro;
  };

  /// 每个state的规则
  struct StateRule {
    /// state名称
    U8String name;
    /// 每个token的规则
    List<TokenRule> token_rules;
    /// 一行结束后要跳转的状态名称
    U8String line_end_state_str;
    /// 一行结束后要跳转的状态
    int32_t line_end_state {-1};
    /// 每个token的表达式合并的大表达式
    U8String merged_pattern;
    /// 编译后的正则表达式指针
    OnigRegex regex;
    /// 合并后大表达式的总捕获组数量
    int32_t group_count {0};
    /// importSyntax请求列表
    List<ImportSyntaxRequest> import_requests;

    static StateRule kEmpty;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 每个block匹配对的规则
  struct BlockRule {
    /// 符号对开始文本(如 {, @begin)
    U8String start;
    /// 符号对结束文本(如 }, @end)
    U8String end;
    /// 分支关键字
    std::unordered_set<U8String> branch_keywords;
    /// ID，解析后生成
    int32_t rule_id {0};

#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 语法规则编译器
  class SyntaxRuleCompiler {
  public:
    explicit SyntaxRuleCompiler(const SharedPtr<StyleMapping>& style_mapping, bool inline_style, HighlightEngine* engine = nullptr);

    /// 通过json编译语法规则
    /// @param json 语法规则文件的json
    SharedPtr<SyntaxRule> compileSyntaxFromJson(const U8String& json);

    /// 编译语法规则
    /// @param file 语法规则定义文件(json)
    SharedPtr<SyntaxRule> compileSyntaxFromFile(const U8String& file);
  private:
    SharedPtr<StyleMapping> m_style_mapping_;
    bool m_inline_style_ {false};
    HighlightEngine* m_engine_ {nullptr};

    static void parseSyntaxName(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseFileExtensions(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseVariables(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void parseInlineStyles(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    void parseStates(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    void parseState(const SharedPtr<SyntaxRule>& rule, StateRule& state_rule, const nlohmann::json& state_json);
		void parseBlockPairs(const SharedPtr<SyntaxRule>& rule, nlohmann::json& root);
    static void compileStatePattern(StateRule& state_rule);
    void processImportSyntaxRequests(const SharedPtr<SyntaxRule>& rule);
    static void importSyntaxRule(const SharedPtr<SyntaxRule>& target_rule, int32_t target_state_id,
      const SharedPtr<SyntaxRule>& source_rule);
    static void replaceVariable(U8String& text, HashMap<U8String, U8String>& variables_map);

    static int32_t parseColorSafe(const U8String& color_str);
  };
}

#endif //SWEETLINE_INTERNAL_SYNTAX_H