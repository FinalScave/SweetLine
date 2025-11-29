#include <stdexcept>
#include <sstream>
#include "foundation.h"
#include "util.h"

namespace NS_SWEETLINE {
  // ===================================== TextPosition ============================================

  bool TextPosition::operator<(const TextPosition& other) const {
    if (line != other.line) return line < other.line;
    return column < other.column;
  }

  bool TextPosition::operator==(const TextPosition& other) const {
    return line == other.line && column == other.column;
  }

  // ===================================== TextRange ============================================
  bool TextRange::operator==(const TextRange& other) const {
    return start == other.start && end == other.end;
  }

  bool TextRange::contains(const TextPosition& pos) const {
    return !(pos < start) && (pos < end || pos == end);
  }

  // ===================================== Document ============================================
  Document::Document(const String& uri, const String& initial_text): uri_(uri) {
    setText(initial_text);
  }

  Document::Document(String&& uri, const String& initial_text): uri_(std::move(uri)) {
    setText(initial_text);
  }

  void Document::setText(const String& text) {
    splitTextIntoLines(text, lines);
  }

  String Document::getUri() const {
    return uri_;
  }

  String Document::getText() const {
    String result;
    for (size_t i = 0; i < lines.size(); ++i) {
      result += lines[i];
      if (i < lines.size() - 1) {
#ifdef _WIN32
        result += "\r\n";
#else
        result += '\n';
#endif
      }
    }
    return result;
  }

  const String& Document::getLine(size_t line) const {
    if (line >= lines.size()) {
      throw std::out_of_range("Line number out of range");
    }
    return lines[line];
  }

  size_t Document::totalChars() const {
    size_t count = 0;
    for (const auto& line : lines) {
      count += Utf8Util::countChars(line);
    }
    return count;
  }

  size_t Document::getLineCharCount(size_t line) const {
    if (line >= lines.size()) {
      throw std::out_of_range("getLineCharCount(): Invalid line: " + std::to_string(line));
    }
#ifdef _WIN32
    return Utf8Util::countChars(lines[line]) + 2;
#else
    return Utf8Util::countChars(lines[line]) + 1;
#endif
  }

  size_t Document::getLineCount() const {
    return lines.size();
  }
  
  int32_t Document::patch(const TextRange& range, const String& new_text) {
    if (range.start.line >= lines.size()) {
      // 追加到末尾
      return appendText(new_text);
    }
    // 将patch的文本按行分割
    std::vector<String> new_lines;
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
    std::vector<String> new_lines;
    splitTextIntoLines(text, new_lines);

    if (lines.empty()) {
      lines = std::move(new_lines);
    } else {
      // 第一行追加到现有的最后一行
      lines.back() += new_lines.empty() ? "" : new_lines[0];
      // 后续继续追加
      for (size_t i = 1; i < new_lines.size(); ++i) {
        lines.push_back(new_lines[i]);
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
    if (line >= lines.size()) {
      throw std::out_of_range("charIndexOfLine(): Invalid line: " + std::to_string(line));
    }
    size_t index = 0;
    for (size_t i = 0; i < line; ++i) {
#ifdef _WIN32
      index += Utf8Util::countChars(lines[i]) + 2;
#else
      index += Utf8Util::countChars(lines[i]) + 1;
#endif
    }
    return index;
  }

  TextPosition Document::charIndexToPosition(size_t char_index) const {
    TextPosition pos;
    size_t current = 0;
    for (pos.line = 0; pos.line < lines.size(); ++pos.line) {
      size_t lineLength = Utf8Util::countChars(lines[pos.line]);
      if (char_index <= current + lineLength) {
        pos.column = char_index - current;
        pos.index = char_index;
        return pos;
      }
#ifdef _WIN32
      current += lineLength + 2; // +2 for \r\n
#else
      current += lineLength + 1; // +1 for \n
#endif
    }
    throw std::out_of_range("Index out of range");
  }

  bool Document::isValidPosition(const TextPosition& pos) const {
    if (pos.line >= lines.size()) {
      return false;
    }
    return pos.column <= lines[pos.line].length();
  }

  size_t Document::positionToCharIndex(const TextPosition& pos) const {
    if (!isValidPosition(pos)) {
      throw std::out_of_range("Invalid text position");
    }
    size_t index = 0;
    for (size_t i = 0; i < pos.line; ++i) {
#ifdef _WIN32
      index += Utf8Util::countChars(lines[i]) + 2;
#else
      index += Utf8Util::countChars(lines[i]) + 1;
#endif
    }
    index += pos.column;
    return index;
  }

  void Document::splitTextIntoLines(const String& text, std::vector<String>& result) {
    result.clear();
    if (text.empty()) {
      return;
    }
    std::stringstream ss(text);
    String line;
    while (std::getline(ss, line)) {
      if (line.back() == '\r') {
        line = line.substr(0, line.length() - 1);
      }
      result.push_back(line);
    }
    // 如果最后以换行符结束，添加一个空行
    if (!text.empty() && text.back() == '\n') {
      result.push_back("");
    }
  }

  int32_t Document::patchSingleLine(const TextRange& range, const std::vector<String>& new_lines) {
    String& line = lines[range.start.line];
    // 转换为字节位置进行操作
    size_t start_byte = Utf8Util::charPosToBytePos(line, range.start.column);
    size_t end_byte = Utf8Util::charPosToBytePos(line, range.end.column);

    // patch的文本为空，则将range替换为""，即删除range内的文本
    if (new_lines.empty()) {
      line = line.substr(0, start_byte) + line.substr(end_byte);
      return 0;
    }

    if (new_lines.size() == 1) {
      // 将range中间直接替换为patch的内容
      line = line.substr(0, start_byte) + new_lines[0] + line.substr(end_byte);
      return 0;
    } else {
      // 第一行从range两边截断，左边拼接new_lines[0]
      String rest_of_line = line.substr(end_byte);
      line = line.substr(0, start_byte) + new_lines[0];

      // 插入新行
      for (size_t i = 1; i < new_lines.size(); ++i) {
        lines.insert(lines.begin() + range.start.line + i, new_lines[i]);
      }

      // 原来的第一行从range截断的右边拼接到插入文本的最后
      if (!rest_of_line.empty()) {
        lines[range.start.line + new_lines.size() - 1] += rest_of_line;
      }
      return static_cast<int32_t>(new_lines.size()) - 1;
    }
  }

  int32_t Document::patchMultipleLines(const TextRange& range, const std::vector<String>& new_lines) {
    // 第一行
    String& first_line = lines[range.start.line];
    size_t start_byte = Utf8Util::charPosToBytePos(first_line, range.start.column);

    // 最后一行
    String& last_line = lines[range.end.line];
    size_t end_byte = Utf8Util::charPosToBytePos(last_line, range.end.column);
    String rest_of_last_line = last_line.substr(end_byte);

    // patch的文本为空，则将range替换为""，即删除range内的文本
    if (new_lines.empty()) {
      first_line = first_line.substr(0, start_byte) + rest_of_last_line;
      // 删除range中间的行
      if (range.end.line > range.start.line) {
        size_t start_delete = range.start.line + 1;
        size_t end_delete = range.end.line + 1; // +1, erase 是 [first, last)
        if (end_delete > lines.size()) {
          end_delete = lines.size();
        }
        if (start_delete < end_delete) {
          lines.erase(lines.begin() + start_delete, lines.begin() + end_delete);
        }
      }
      return -static_cast<int32_t>(range.end.line - range.start.line);
    }

    // 在range截断的左边拼接第一行
    first_line = first_line.substr(0, start_byte) + new_lines[0];

    // 删除中间行
    size_t delete_count = range.end.line - range.start.line;
    if (delete_count > 0) {
      size_t delete_start = range.start.line + 1;
      size_t delete_end = delete_start + delete_count;
      if (delete_end > lines.size()) {
        delete_end = lines.size();
      }
      if (delete_start < delete_end) {
        lines.erase(lines.begin() + delete_start, lines.begin() + delete_end);
      }
    }

    // 插入新行
    for (size_t i = 1; i < new_lines.size(); ++i) {
      lines.insert(lines.begin() + range.start.line + i, new_lines[i]);
    }

    // range截断的右边剩余内容拼接到最后一行
    if (!rest_of_last_line.empty()) {
      size_t last_new_line_index = range.start.line + new_lines.size() - 1;
      if (last_new_line_index < lines.size()) {
        lines[last_new_line_index] += rest_of_last_line;
      } else {
        lines.push_back(rest_of_last_line);
      }
    }
    return static_cast<int32_t>(new_lines.size() - (range.end.line - range.start.line));
  }

}
