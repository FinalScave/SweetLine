#ifndef SWEETLINE_HIGHLIGHT_H
#define SWEETLINE_HIGHLIGHT_H

#include <cstdint>
#include "foundation.h"
#include "syntax.h"

namespace NS_SWEETLINE {
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

#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 每一行的高亮块序列
  struct LineHighlight {
    List<TokenSpan> spans;
    void pushOrMergeSpan(TokenSpan&& span);
    bool operator==(const LineHighlight& other) const;
    void toJson(String& result) const;

#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 整个文本内容的高亮
  struct DocumentHighlight {
    List<LineHighlight> lines;

    void addLine(LineHighlight&& line);
    size_t spanCount() const;
    void reset();
    void toJson(String& result) const;

#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 高亮配置
  struct HighlightConfig {
    /// 分析的高亮信息是否携带index，不携带的情况下每个TokenSpan只有line和column
    bool show_index {false};

    static HighlightConfig kDefault;
  };

  class InternalDocumentAnalyzer;
  /// 高亮分析器
  class DocumentAnalyzer {
  public:
    /// 对整个文本进行高亮分析
    /// @return 整个文本的高亮结果
    SharedPtr<DocumentHighlight> analyze() const;

    /// 根据patch内容重新分析整个文本的高亮结果
    /// @param range patch的变更范围
    /// @param new_text patch的文本
    /// @return 整个文本的高亮结果
    SharedPtr<DocumentHighlight> analyzeChanges(const TextRange& range, const String& new_text) const;

    /// 根据patch内容重新分析整个文本的高亮结果
    /// @param start_index patch变更的起始字符索引
    /// @param end_index patch变更的结束字符索引
    /// @param new_text patch的文本
    /// @return 整个文本的高亮结果
    SharedPtr<DocumentHighlight> analyzeChanges(size_t start_index, size_t end_index, const String& new_text) const;

    /// 分析一行的高亮结果
    /// @param line 行号
    /// @param line_highlight LineHighlight
    /// @return 一行的高亮结果
    void analyzeLine(size_t line, LineHighlight& line_highlight) const;

    /// 获取当前高亮分析器持有的文本内容
    /// @return std::shared_ptr<Document>
    SharedPtr<Document> getDocument() const;
  private:
    friend class HighlightEngine;
    DocumentAnalyzer(const SharedPtr<Document>& document, const SharedPtr<SyntaxRule>& rule,
      const HighlightConfig& config = HighlightConfig::kDefault);
    UniquePtr<InternalDocumentAnalyzer> analyzer_impl_;
  };

  /// 高亮引擎
  class HighlightEngine {
  public:
    explicit HighlightEngine(const HighlightConfig& config = HighlightConfig::kDefault);

    /// 通过json编译语法规则
    /// @param json 语法规则文件的json
    /// @throws SyntaxRuleParseError 编译错误时会抛出 SyntaxRuleParseError
    SharedPtr<SyntaxRule> compileSyntaxFromJson(const String& json);

    /// 编译语法规则
    /// @param file 语法规则定义文件(json)
    /// @throws SyntaxRuleParseError 编译错误时会抛出 SyntaxRuleParseError
    SharedPtr<SyntaxRule> compileSyntaxFromFile(const String& file);

    /// 获取指定名称的语法规则(如 java)
    /// @param name 语法规则名称
    SharedPtr<SyntaxRule> getSyntaxRuleByName(const String& name) const;

    /// 获取指定后缀名匹配的的语法规则(如 .t)
    /// @param extension 后缀名
    SharedPtr<SyntaxRule> getSyntaxRuleByExtension(const String& extension) const;

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
    SharedPtr<DocumentAnalyzer> loadDocument(const SharedPtr<Document>& document);

    /// 移除加载过的文本
    /// @param uri 文本内容的Uri
    void removeDocument(const String& uri);
  private:
    HighlightConfig config_;
    HashSet<SharedPtr<SyntaxRule>> syntax_rules_;
    HashMap<String, SharedPtr<DocumentAnalyzer>> analyzer_map_;
    SharedPtr<StyleMapping> style_mapping_;
  };
}

#endif //SWEETLINE_HIGHLIGHT_H