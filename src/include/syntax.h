#ifndef SWEETLINE_SYNTAX_H
#define SWEETLINE_SYNTAX_H

#include "macro.h"

namespace NS_SWEETLINE {
  /// 语法规则Json解析时的错误
  class SyntaxRuleParseError : public std::exception {
  public:
    /// 缺少属性
    static constexpr int kErrCodePropertyExpected = -1;
    /// 属性内容错误
    static constexpr int kErrCodePropertyInvalid = -2;
    /// 正则表达式错误
    static constexpr int kErrCodePatternInvalid = -3;
    /// state错误
    static constexpr int kErrCodeStateInvalid = -4;
    /// json存在语法错误
    static constexpr int kErrCodeJsonInvalid = -5;
    /// 文件不存在
    static constexpr int kErrCodeFileNotExists = -6;
    /// 文件内容为空
    static constexpr int kErrCodeFileInvalid = -7;

    explicit SyntaxRuleParseError(int err_code);
    explicit SyntaxRuleParseError(int err_code, const String& message);
    explicit SyntaxRuleParseError(int err_code, const char* message);

    const char* what() const noexcept override;
    const String& message() const noexcept;
  private:
    int err_code_;
    String message_;
  };

  struct StateRule;
  /// 语法规则
  struct SyntaxRule {
    /// 语法规则的名称
    String name;
    /// 支持的文件扩展名
    HashSet<String> file_extensions_;
    /// variables
    HashMap<String, String> variables_map_;
    /// state id 到 StateRule 的映射
    HashMap<int32_t, StateRule> state_rules_map_;
    /// state名称 到 id 的映射
    HashMap<String, int32_t> state_id_map_;

    int32_t getOrCreateStateId(const String& state_name);
    bool containsRule(int32_t state_id) const;
    StateRule& getStateRule(int32_t state_id);
    SyntaxRule();

    constexpr static int32_t kDefaultStateId = 0;
    constexpr static const char* kDefaultStateName = "default";
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  private:
    int32_t state_id_counter_ {1};
  };

  /// 高亮样式id和名称的映射
  struct StyleMapping {
    StyleMapping();

    /// style 名称 到 id的映射
    HashMap<String, int32_t> style_name_id_map_;
    /// style id 到 名称的映射
    HashMap<int32_t, String> style_id_name_map_;

    void registerStyleName(const String& style_name, int32_t style_id);
    int32_t getStyleId(const String& style_name);
    int32_t getOrCreateStyleId(const String& style_name);
    const String& getStyleName(int32_t style_id);
  private:
    int32_t style_id_counter_ {4};
    static int32_t kDefaultStyleId;
    static String kDefaultStyleName;
  };
}

#endif //SWEETLINE_SYNTAX_H