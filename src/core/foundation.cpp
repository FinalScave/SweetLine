#include <stdexcept>
#include <sstream>
#include "foundation.h"
#include "util.h"

namespace NS_SWEETLINE {
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextPosition, line, column, index);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRange, start, end);

  // ===================================== TextPosition ============================================
  bool TextPosition::operator<(const TextPosition& other) const {
    if (line != other.line) return line < other.line;
    return column < other.column;
  }

  bool TextPosition::operator==(const TextPosition& other) const {
    return line == other.line && column == other.column;
  }

#ifdef SWEETLINE_DEBUG
  void TextPosition::dump() const {
    const nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== TextRange ============================================
  bool TextRange::operator==(const TextRange& other) const {
    return start == other.start && end == other.end;
  }

  bool TextRange::contains(const TextPosition& pos) const {
    return !(pos < start) && (pos < end || pos == end);
  }

#ifdef SWEETLINE_DEBUG
  void TextRange::dump() const {
    const nlohmann::json json = *this;
    std::cout << json.dump(2) << std::endl;
  }
#endif

  // ===================================== Document ============================================
  Document::Document(const String& uri, const String& initial_text): m_uri_(uri) {
    setText(initial_text);
  }

  Document::Document(String&& uri, const String& initial_text): m_uri_(std::move(uri)) {
    setText(initial_text);
  }

  void Document::setText(const String& text) {
    splitTextIntoLines(text, m_lines_);
  }

  String Document::getUri() const {
    return m_uri_;
  }

  String Document::getText() const {
    String result;
    for (const DocumentLine& line : m_lines_) {
      result += line.text;
      appendLineEnding(result, line.ending);
    }
    return result;
  }

  size_t Document::totalChars() const {
    size_t count = 0;
    for (const DocumentLine& line : m_lines_) {
      count += Utf8Util::countChars(line.text) + getLineEndingWidth(line.ending);
    }
    return count;
  }

  size_t Document::getLineCharCount(size_t line) const {
    if (line >= m_lines_.size()) {
      throw std::out_of_range("getLineCharCount(): Invalid line: " + std::to_string(line));
    }
    return Utf8Util::countChars(m_lines_[line].text) + getLineEndingWidth(m_lines_[line].ending);
  }

  size_t Document::getLineCount() const {
    return m_lines_.size();
  }

  const DocumentLine& Document::getLine(size_t line) const {
    if (line >= m_lines_.size()) {
      throw std::out_of_range("Line number out of range");
    }
    return m_lines_[line];
  }

  int32_t Document::patch(const TextRange& range, const String& new_text) {
    if (range.start.line >= m_lines_.size()) {
      // 追加到末尾
      return appendText(new_text);
    }
    // 将patch的文本按行分割
    std::vector<DocumentLine> new_lines;
    splitTextIntoLines(new_text, new_lines);

    if (range.start.line == range.end.line) {
      // 单行patch
      return patchSingleLine(range, new_lines);
    } else {
      // 多行patch
      return patchMultipleLines(range, new_lines);
    }
  }

  int32_t Document::appendText(const String& text) {
    std::vector<DocumentLine> new_lines;
    splitTextIntoLines(text, new_lines);

    if (m_lines_.empty()) {
      m_lines_ = std::move(new_lines);
    } else {
      // 第一行追加到现有的最后一行
      m_lines_.back().text += new_lines.empty() ? "" : new_lines[0].text;
      // 后续继续追加
      for (size_t i = 1; i < new_lines.size(); ++i) {
        m_lines_.push_back(new_lines[i]);
      }
    }
    return static_cast<int32_t>(new_lines.size());
  }

  void Document::insert(const TextPosition& position, const String& text) {
    if (!isValidPosition(position)) {
      throw std::out_of_range("Invalid insert position");
    }
    patch({position, position}, text);
  }

  void Document::remove(const TextRange& range) {
    patch(range, "");
  }

  size_t Document::charIndexOfLine(size_t line) const {
    if (line >= m_lines_.size()) {
      throw std::out_of_range("charIndexOfLine(): Invalid line: " + std::to_string(line));
    }
    size_t index = 0;
    for (size_t i = 0; i < line; ++i) {
      index += Utf8Util::countChars(m_lines_[i].text) + getLineEndingWidth(m_lines_[i].ending);
    }
    return index;
  }

  TextPosition Document::charIndexToPosition(size_t char_index) const {
    TextPosition pos;
    size_t current = 0;
    for (pos.line = 0; pos.line < m_lines_.size(); ++pos.line) {
      size_t lineLength = Utf8Util::countChars(m_lines_[pos.line].text);
      if (char_index <= current + lineLength) {
        pos.column = char_index - current;
        pos.index = char_index;
        return pos;
      }
      current += lineLength + getLineEndingWidth(m_lines_[pos.line].ending);
    }
    throw std::out_of_range("Index out of range");
  }

  bool Document::isValidPosition(const TextPosition& pos) const {
    if (pos.line >= m_lines_.size()) {
      return false;
    }
    return pos.column < getLineCharCount(pos.line);
  }

  size_t Document::positionToCharIndex(const TextPosition& pos) const {
    if (!isValidPosition(pos)) {
      throw std::out_of_range("Invalid text position");
    }
    size_t index = 0;
    for (size_t i = 0; i < pos.line; ++i) {
      index += Utf8Util::countChars(m_lines_[i].text) + getLineEndingWidth(m_lines_[i].ending);
    }
    index += pos.column;
    return index;
  }

  void Document::splitTextIntoLines(const String& text, std::vector<DocumentLine>& result) {
    result.clear();
    if (text.empty()) {
      return;
    }
    std::stringstream ss(text);
    String line;
    while (std::getline(ss, line)) {
      LineEnding ending = LineEnding::LF;
      if (!line.empty() && line.back() == '\r') {
        line = line.substr(0, line.length() - 1);
        ending = LineEnding::CRLF;
      }
      result.push_back({line, ending});
    }
    // 如果最后以换行符结束，添加一个空行
    if (!text.empty() && text.back() == '\n') {
      result.push_back({"", LineEnding::NONE});
    }
  }

  int32_t Document::patchSingleLine(const TextRange& range, const std::vector<DocumentLine>& new_lines) {
    DocumentLine& line = m_lines_[range.start.line];
    // 转换为字节位置进行操作
    size_t start_byte = Utf8Util::charPosToBytePos(line.text, range.start.column);
    size_t end_byte = Utf8Util::charPosToBytePos(line.text, range.end.column);

    // patch的文本为空，则将range替换为""，即删除range内的文本
    if (new_lines.empty()) {
      line.text = line.text.substr(0, start_byte) + line.text.substr(end_byte);
      return 0;
    }

    if (new_lines.size() == 1) {
      // 将range中间直接替换为patch的内容
      line.text = line.text.substr(0, start_byte) + new_lines[0].text + line.text.substr(end_byte);
      return 0;
    } else {
      // 第一行从range两边截断，左边拼接new_lines[0]
      String rest_of_line = line.text.substr(end_byte);
      line.text = line.text.substr(0, start_byte) + new_lines[0].text;

      // 插入新行
      for (size_t i = 1; i < new_lines.size(); ++i) {
        m_lines_.insert(m_lines_.begin() + range.start.line + i, new_lines[i]);
      }

      // 原来的第一行从range截断的右边拼接到插入文本的最后
      if (!rest_of_line.empty()) {
        m_lines_[range.start.line + new_lines.size() - 1].text += rest_of_line;
      }
      return static_cast<int32_t>(new_lines.size()) - 1;
    }
  }

  int32_t Document::patchMultipleLines(const TextRange& range, const std::vector<DocumentLine>& new_lines) {
    // 第一行
    DocumentLine& first_line = m_lines_[range.start.line];
    size_t start_byte = Utf8Util::charPosToBytePos(first_line.text, range.start.column);

    // 最后一行
    DocumentLine& last_line = m_lines_[range.end.line];
    size_t end_byte = Utf8Util::charPosToBytePos(last_line.text, range.end.column);
    String rest_of_last_line = last_line.text.substr(end_byte);
    LineEnding ending_of_last_line = last_line.ending;

    // patch的文本为空，则将range替换为""，即删除range内的文本
    if (new_lines.empty()) {
      first_line.text = first_line.text.substr(0, start_byte) + rest_of_last_line;
      // 删除range中间的行
      if (range.end.line > range.start.line) {
        size_t start_delete = range.start.line + 1;
        size_t end_delete = range.end.line + 1; // +1, erase 是 [first, last)
        if (end_delete > m_lines_.size()) {
          end_delete = m_lines_.size();
        }
        if (start_delete < end_delete) {
          m_lines_.erase(m_lines_.begin() + start_delete, m_lines_.begin() + end_delete);
        }
      }
      return -static_cast<int32_t>(range.end.line - range.start.line);
    }

    // 在range截断的左边拼接第一行
    first_line.text = first_line.text.substr(0, start_byte) + new_lines[0].text;

    // 删除中间行
    size_t delete_count = range.end.line - range.start.line;
    if (delete_count > 0) {
      size_t delete_start = range.start.line + 1;
      size_t delete_end = delete_start + delete_count;
      if (delete_end > m_lines_.size()) {
        delete_end = m_lines_.size();
      }
      if (delete_start < delete_end) {
        m_lines_.erase(m_lines_.begin() + delete_start, m_lines_.begin() + delete_end);
      }
    }

    // 插入新行
    for (size_t i = 1; i < new_lines.size(); ++i) {
      m_lines_.insert(m_lines_.begin() + range.start.line + i, new_lines[i]);
    }

    // range截断的右边剩余内容拼接到最后一行
    if (!rest_of_last_line.empty()) {
      size_t last_new_line_index = range.start.line + new_lines.size() - 1;
      if (last_new_line_index < m_lines_.size()) {
        m_lines_[last_new_line_index].text += rest_of_last_line;
      } else {
        m_lines_.push_back({rest_of_last_line, ending_of_last_line});
      }
    }
    return static_cast<int32_t>(new_lines.size() - (range.end.line - range.start.line));
  }

  inline void Document::appendLineEnding(String& text, LineEnding ending) {
    switch (ending) {
    case LineEnding::LF:
      text += "\n";
      break;
    case LineEnding::CRLF:
      text += "\r\n";
      break;
    case LineEnding::CR:
      text += "\r";
      break;
    default: break;
    }
  }

  inline uint8_t Document::getLineEndingWidth(LineEnding ending) {
    switch (ending) {
    case LineEnding::LF:
    case LineEnding::CR:
      return 1;
    case LineEnding::CRLF:
      return 2;
    default: return 0;
    }
  }
}
