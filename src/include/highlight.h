#ifndef SWEETLINE_HIGHLIGHT_H
#define SWEETLINE_HIGHLIGHT_H

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <oniguruma/oniguruma.h>
#include "foundation.h"

namespace NS_SWEETLINE {
  template<typename T>
  using List = std::vector<T>;
  template<typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqualTo = std::equal_to<K>>
  using HashMap = std::unordered_map<K, V, KeyHash, KeyEqualTo>;
  template<typename T, typename Hash = std::hash<T>, typename EqualTo = std::equal_to<T>>
  using HashSet = std::unordered_set<T, Hash, EqualTo>;

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

  /// 匹配分析的最小单元(token)的规则
  struct TokenRule {
    /// 正则表达式
    String pattern;
    /// 按捕获组区分的高亮样式
    HashMap<int32_t, int32_t> styles;
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
#ifdef FH_DEBUG
    void dump() const {
      const nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TokenRule, pattern, styles, goto_state_str, group_count, group_offset_start, goto_state);
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
#ifdef FH_DEBUG
    void dump() const {
      const nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StateRule, name, token_rules, merged_pattern, group_count);
#endif
  };

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
#ifdef FH_DEBUG
    void dump() const {
      const nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SyntaxRule, name, file_extensions_, variables_map_, state_rules_map_, state_id_map_);
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

  /// 匹配的每一个高亮块
  struct TokenSpan {
    /// 高亮块的范围
    TextRange range;
    /// 匹配到的文本
    String matched_text;
    /// 高亮块所匹配的style
    int32_t style;
    /// 高亮块被匹配时所处的状态
    int32_t state {0};
    /// 高亮块要跳转的别的state
    int32_t goto_state {-1};

    bool operator==(const TokenSpan& other) const;
    bool operator!=(const TokenSpan& other) const;

#ifdef FH_DEBUG
    void dump() const {
      const nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TokenSpan, range, style, state, goto_state);
#endif
  };

  /// 每一行的高亮块序列
  struct LineHighlight {
    List<TokenSpan> spans;
    void pushOrMergeSpan(TokenSpan&& span);
    bool operator==(const LineHighlight& other) const;

#ifdef FH_DEBUG
    void dump() const {
      const nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LineHighlight, spans);
#endif
  };

  /// 整个文本内容的高亮
  struct DocumentHighlight {
    List<LineHighlight> lines;

    void addLine(LineHighlight&& line);
    size_t spanCount() const;
    void reset();
    void toJson(String& result) const;

#ifdef FH_DEBUG
    void dump() const {
      nlohmann::json json = *this;
      std::cout << json.dump(2) << std::endl;
    }
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DocumentHighlight, lines)
#endif
  };

  struct CaptureGroupMatch {
    /// 匹配到的捕获组索引
    int32_t group {-1};
    /// 高亮样式
    int32_t style;
    /// 匹配到的起始字符位置
    size_t start {0};
    /// 匹配到的字符长度
    size_t length {0};
  };

  /// 正则匹配结果
  struct MatchResult {
    /// 是否匹配到了
    bool matched {false};
    /// 匹配到的起始字符位置
    size_t start {0};
    /// 匹配到的字符长度
    size_t length {0};
    /// 当前所处的state
    int32_t state {-1};
    /// 匹配到的所属token rule
    int32_t token_rule_idx {-1};
    /// 匹配到的捕获组
    int32_t matched_group {-1};
    /// 高亮样式
    int32_t style {0};
    /// 要切换的state
    int32_t goto_state {-1};
    /// 匹配到的文本内容
    String matched_text;
    /// 所有匹配的捕获组
    List<CaptureGroupMatch> capture_groups;
  };

  /// 高亮配置
  struct HighlightConfig {
    /// 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
    bool show_index {false};

    static HighlightConfig kDefault;
  };

  /// 高亮分析器
  class DocumentAnalyzer {
  public:
    explicit DocumentAnalyzer(const Ptr<Document>& document, const Ptr<SyntaxRule>& rule,
      const HighlightConfig& config = HighlightConfig::kDefault);

    /// 对整个文本进行高亮分析
    /// @return 整个文本的高亮结果
    Ptr<DocumentHighlight> analyze();

    /// 根据patch内容重新分析整个文本的高亮结果
    /// @param range patch的变更范围
    /// @param new_text patch的文本
    /// @return 整个文本的高亮结果
    Ptr<DocumentHighlight> analyzeChanges(const TextRange& range, const String& new_text);

    /// 根据patch内容重新分析整个文本的高亮结果
    /// @param start_index patch变更的起始字符索引
    /// @param end_index patch变更的结束字符索引
    /// @param new_text patch的文本
    /// @return 整个文本的高亮结果
    Ptr<DocumentHighlight> analyzeChanges(size_t start_index, size_t end_index, const String& new_text);

    /// 分析一行的高亮结果
    /// @param line 行号
    /// @param line_highlight LineHighlight
    /// @return 一行的高亮结果
    void analyzeLine(size_t line, LineHighlight& line_highlight);

    /// 获取当前高亮分析器持有的文本内容
    /// @return std::shared_ptr<Document>
    Ptr<Document> getDocument() const;
  private:
    Ptr<Document> document_;
    Ptr<DocumentHighlight> highlight_;
    Ptr<SyntaxRule> rule_;
    HighlightConfig config_;
    List<int32_t> line_states_;

    size_t analyzeLineWithState(size_t line,
      size_t line_start_index, int32_t start_state, LineHighlight& line_highlight);
    void addLineMatchResult(LineHighlight& highlight, size_t line_num, size_t line_char_pos,
      size_t line_start_index, int32_t state, const MatchResult& match_result);
    MatchResult matchAtPosition(const String& text, size_t start_char_pos, int32_t state);
    void findMatchedRuleAndGroup(const StateRule& state_rule, OnigRegion* region, const String& text,
      size_t match_start_byte, size_t match_end_byte, MatchResult& result);
  };

  /// 高亮引擎
  class HighlightEngine {
  public:
    explicit HighlightEngine(const HighlightConfig& config = HighlightConfig::kDefault);

    /// 通过json编译语法规则
    /// @param json 语法规则文件的json
    Ptr<SyntaxRule> compileSyntaxFromJson(const String& json);

    /// 编译语法规则
    /// @param file 语法规则定义文件(json)
    Ptr<SyntaxRule> compileSyntaxFromFile(const String& file);

    /// 获取指定名称的语法规则(如 java)
    /// @param name 语法规则名称
    Ptr<SyntaxRule> getSyntaxRuleByName(const String& name) const;

    /// 获取指定后缀名匹配的的语法规则(如 .t)
    /// @param extension 后缀名
    Ptr<SyntaxRule> getSyntaxRuleByExtension(const String& extension) const;

    /// 注册一个高亮样式，用于名称映射
    /// @param style_name 样式名称
    /// @param style_id 样式id
    void registerStyleName(const String& style_name, int32_t style_id) const;

    /// 通过样式id获取注册的样式名称
    /// @param style_id 样式id
    const String& getStyleName(int32_t style_id) const;

    /// 加载文本并进行首次分析
    /// @param document 文本内容
    /// @return 整个文本的高亮结果
    Ptr<DocumentAnalyzer> loadDocument(const Ptr<Document>& document);
  private:
    HighlightConfig config_;
    HashSet<Ptr<SyntaxRule>> syntax_rules_;
    HashMap<String, Ptr<DocumentAnalyzer>> analyzer_map_;
    Ptr<StyleMapping> style_mapping_;
    UPtr<SyntaxRuleCompiler> syntax_rule_compiler_;
  };
}

#endif //SWEETLINE_HIGHLIGHT_H