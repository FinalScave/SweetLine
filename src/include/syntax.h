#ifndef SWEETLINE_SYNTAX_H
#define SWEETLINE_SYNTAX_H

#include "macro.h"

namespace NS_SWEETLINE {
  /// 语法规则Json解析时的错误
  class SyntaxRuleParseError : public std::exception {
  public:
    /// 缺少属性
    static constexpr int ERR_JSON_PROPERTY_MISSED = -1;
    /// 属性内容错误
    static constexpr int ERR_JSON_PROPERTY_INVALID = -2;
    /// 正则表达式错误
    static constexpr int ERR_PATTERN_INVALID = -3;
    /// state错误
    static constexpr int ERR_STATE_INVALID = -4;
    /// json存在语法错误
    static constexpr int ERR_JSON_INVALID = -5;
    /// 文件不存在
    static constexpr int ERR_FILE_NOT_EXISTS = -6;
    /// 文件内容为空
    static constexpr int ERR_FILE_INVALID = -7;

    explicit SyntaxRuleParseError(int err_code);
    explicit SyntaxRuleParseError(int err_code, const U8String& message);
    explicit SyntaxRuleParseError(int err_code, const char* message);

    const char* what() const noexcept override;
    const U8String& message() const noexcept;
    int code() const noexcept;
  private:
    int m_err_code_;
    U8String m_message_;
  };

  /// 高亮样式tags枚举
  enum struct InlineStyleTag : int16_t {
    /// 加粗显示
    BOLD = 1,
    /// 斜体显示
    ITALIC = 1 << 1,
  };

  /// 语法规则中直接包含的样式定义
  struct InlineStyle {
    /// 前景色(ARGB)
    int32_t foreground {0};
    /// 背景色(ARGB)
    int32_t background {0};
    /// tags
    int16_t tags {0};
  };

  /// 高亮样式id和名称的映射
  struct StyleMapping {
    StyleMapping();

    /// style 名称 到 id的映射
    HashMap<U8String, int32_t> style_name_id_map;
    /// style id 到 名称的映射
    HashMap<int32_t, U8String> style_id_name_map;

    void registerStyleName(const U8String& style_name, int32_t style_id);
    int32_t getStyleId(const U8String& style_name);
    int32_t getOrCreateStyleId(const U8String& style_name);
    const U8String& getStyleName(int32_t style_id);
  private:
    int32_t m_style_id_counter_ {1};
    static int32_t kDefaultStyleId;
    static U8String kDefaultStyleName;
  };

  struct StateRule;
  struct BlockRule;
  /// 语法规则
  struct SyntaxRule {
    /// 语法规则的名称
    U8String name;
    /// 支持的文件扩展名
    HashSet<U8String> file_extensions;
    /// 语法规则中定义的styles(可空),样式ID->InlineStyle
    HashMap<int32_t, InlineStyle> inline_styles;
    /// 语法规则内联样式定义表 (inline_style模式)
    UniquePtr<StyleMapping> style_mapping;
    /// variables
    HashMap<U8String, U8String> variables_map;
    /// state id 到 StateRule 的映射
    HashMap<int32_t, StateRule> state_rules_map;
    /// state名称 到 id 的映射
    HashMap<U8String, int32_t> state_id_map;

    bool containsInlineStyle(int32_t style_id);
    InlineStyle& getInlineStyle(int32_t style_id);

    int32_t getOrCreateStateId(const U8String& state_name);
    bool containsRule(int32_t state_id) const;
    StateRule& getStateRule(int32_t state_id);
    SyntaxRule();

    constexpr static int32_t kDefaultStateId = 0;
    constexpr static const char* kDefaultStateName = "default";
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  private:
    int32_t m_state_id_counter_ {1};
  };
}

#endif //SWEETLINE_SYNTAX_H