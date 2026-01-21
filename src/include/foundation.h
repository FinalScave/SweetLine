#ifndef SWEETLINE_FOUNDATION_H
#define SWEETLINE_FOUNDATION_H

#ifdef SWEETLINE_DEBUG
#include <nlohmann/json.hpp>
#include <iostream>
#endif

#include "macro.h"

namespace NS_SWEETLINE {
  /// 文本位置描述
  struct TextPosition {
    /// 文字所处行，起始为0
    size_t line {0};
    /// 文字所处列，起始为0
    size_t column {0};
    /// 文字在全文中的索引，起始为0
    size_t index {0};

    bool operator<(const TextPosition& other) const;
    bool operator==(const TextPosition& other) const;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 文字的范围区间描述
  struct TextRange {
    TextPosition start;
    TextPosition end;

    bool operator==(const TextRange& other) const;
    bool contains(const TextPosition& pos) const;
#ifdef SWEETLINE_DEBUG
    void dump() const;
#endif
  };

  /// 换行符类型
  enum struct LineEnding {
    NONE, // 最后一行没有换行符
    LF, // \n
    CRLF, // \r\n
    CR // \r
  };

  /// 文本行结构定义
  struct DocumentLine {
    /// 当前行文本内容 (不包含换行符)
    U8String text;
    /// 当前行换行符类型
    LineEnding ending {LineEnding::NONE};
  };

  /// 支持增量更新的文本
  class Document {
  public:
    explicit Document(const U8String& uri, const U8String& initial_text = "");
    explicit Document(U8String&& uri, const U8String& initial_text = "");

    /// 设置完整的文本内容，设置后会按行分割
    /// @param text 文本内容
    void setText(const U8String& text);

    /// 获取当前文档的Uri
    U8String getUri() const;

    /// 获取完整文本
    U8String getText() const;

    /// 取总字符数
    size_t totalChars() const;

    /// 获取指定行的字符总数
    /// @param line 行号索引
    size_t getLineCharCount(size_t line) const;

    /// 获取总行数
    size_t getLineCount() const;

    /// 获取指定行的文本信息
    const DocumentLine& getLine(size_t line) const;

    /// 根据指定的行列范围进行增量更新
    /// @param range 更新的范围区间
    /// @param new_text 更新后的文本
    /// @return 返回行数变更的情况，假设返回结果为n，如果为负数说明删除了n行，如果为正数说明新增了n行，如果为0，说明只改变了当前行
    int32_t patch(const TextRange& range, const U8String& new_text);

    /// 追加文本
    /// @param text 要追加的文本
    int32_t appendText(const U8String& text);

    /// 在指定位置插入文本
    /// @param position 要插入的位置
    /// @param text 要插入的文本
    void insert(const TextPosition& position, const U8String& text);

    /// 删除指定范围的文本
    /// @param range 范围
    void remove(const TextRange& range);

    /// 计算指定行在全文中的起始索引
    /// @param line 行号
    size_t charIndexOfLine(size_t line) const;

    /// 将字符索引转换位行列位置
    /// @param char_index 字符索引
    /// @return 行列位置
    TextPosition charIndexToPosition(size_t char_index) const;

    /// 获取结尾换行符的宽度
    /// @param ending 换行符类型
    /// @return 对应的换行符宽度
    static uint8_t getLineEndingWidth(LineEnding ending);
  private:
    U8String m_uri_;
    std::vector<DocumentLine> m_lines_;
    bool isValidPosition(const TextPosition& pos) const;
    size_t positionToCharIndex(const TextPosition& pos) const;
    void splitTextIntoLines(const U8String& text, std::vector<DocumentLine>& result);
    int32_t patchSingleLine(const TextRange& range, const std::vector<DocumentLine>& new_lines);
    int32_t patchMultipleLines(const TextRange& range, const std::vector<DocumentLine>& new_lines);
    static void appendLineEnding(U8String& text, LineEnding ending);
  };
}

#endif //SWEETLINE_FOUNDATION_H
