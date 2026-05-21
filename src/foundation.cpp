#include <stdexcept>
#include <algorithm>
#include "foundation.h"
#include "util.h"

namespace NS_SWEETLINE {
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextPosition, line, column, index);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextRange, start, end);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PatchResult, line_delta, char_delta);

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
  Document::Document(const U8String& uri, const U8String& initial_text): m_uri_(uri) {
    setText(initial_text);
  }

  Document::Document(U8String&& uri, const U8String& initial_text): m_uri_(std::move(uri)) {
    setText(initial_text);
  }

  void Document::setText(const U8String& text) {
    splitTextIntoLines(text, m_lines_);
    rebuildLineMetrics();
  }

  U8String Document::getUri() const {
    return m_uri_;
  }

  U8String Document::getText() const {
    U8String result;
    for (const DocumentLine& line : m_lines_) {
      result += line.text;
      appendLineEnding(result, line.ending);
    }
    return result;
  }

  size_t Document::totalChars() const {
    if (m_lines_.empty()) {
      return 0;
    }
    return m_line_start_indices_.back() + m_line_total_widths_.back();
  }

  size_t Document::getLineCharCount(size_t line) const {
    if (line >= m_lines_.size()) {
      throw std::out_of_range("getLineCharCount(): Invalid line: " + std::to_string(line));
    }
    return m_line_total_widths_[line];
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

  U8String Document::getLineTextWithEnding(size_t line) const {
    const DocumentLine& doc_line = getLine(line);
    U8String result = doc_line.text;
    appendLineEnding(result, doc_line.ending);
    return result;
  }

  PatchResult Document::patch(const TextRange& range, const U8String& new_text) {
    if (range.start.line >= m_lines_.size()) {
      // Append to end
      return appendText(new_text);
    }

    const size_t old_total_chars = totalChars();
    const size_t old_line_count = m_lines_.size();

    // Split the patch text into lines
    List<DocumentLine> new_lines;
    splitTextIntoLines(new_text, new_lines);

    PatchResult result;
    if (range.start.line == range.end.line) {
      // Single line patch
      result = patchSingleLine(range, new_lines);
    } else {
      // Multi-line patch
      result = patchMultipleLines(range, new_lines);
    }
    rebuildLineMetricsFrom(range.start.line);
    result.line_delta = static_cast<int32_t>(m_lines_.size()) - static_cast<int32_t>(old_line_count);
    result.char_delta = static_cast<int32_t>(totalChars()) - static_cast<int32_t>(old_total_chars);
    return result;
  }

  PatchResult Document::appendText(const U8String& text) {
    const size_t old_total_chars = totalChars();
    const size_t old_line_count = m_lines_.size();
    List<DocumentLine> new_lines;
    splitTextIntoLines(text, new_lines);
    if (new_lines.empty()) {
      return {};
    }

    size_t rebuild_from_line = 0;
    if (m_lines_.empty()) {
      m_lines_ = std::move(new_lines);
      rebuildLineMetrics();
    } else {
      rebuild_from_line = m_lines_.size() - 1;
      const LineEnding appended_ending = new_lines[0].ending;
      m_lines_.back().text += new_lines.empty() ? "" : new_lines[0].text;
      m_lines_.back().ending = appended_ending;
      for (size_t i = 1; i < new_lines.size(); ++i) {
        m_lines_.push_back(new_lines[i]);
      }
      rebuildLineMetricsFrom(rebuild_from_line);
    }
    PatchResult result;
    result.line_delta = static_cast<int32_t>(m_lines_.size()) - static_cast<int32_t>(old_line_count);
    result.char_delta = static_cast<int32_t>(totalChars()) - static_cast<int32_t>(old_total_chars);
    return result;
  }

  void Document::insert(const TextPosition& position, const U8String& text) {
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
    return m_line_start_indices_[line];
  }

  TextPosition Document::charIndexToPosition(size_t char_index) const {
    if (m_lines_.empty()) {
      throw std::out_of_range("Index out of range");
    }

    const size_t total_chars = totalChars();
    if (char_index >= total_chars) {
      throw std::out_of_range("Index out of range");
    }

    auto it = std::upper_bound(m_line_start_indices_.begin(), m_line_start_indices_.end(), char_index);
    size_t line = it == m_line_start_indices_.begin()
      ? 0
      : static_cast<size_t>(std::distance(m_line_start_indices_.begin(), it) - 1);
    if (line >= m_lines_.size()) {
      line = m_lines_.size() - 1;
    }

    const size_t line_start_index = m_line_start_indices_[line];
    const size_t line_total_width = m_line_total_widths_[line];
    if (char_index < line_start_index + line_total_width) {
      return {line, char_index - line_start_index, char_index};
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
    return m_line_start_indices_[pos.line] + pos.column;
  }

  void Document::splitTextIntoLines(const U8String& text, List<DocumentLine>& result) {
    result.clear();
    if (text.empty()) {
      return;
    }
    size_t line_start = 0;
    for (size_t i = 0; i < text.length(); ++i) {
      if (text[i] != '\n' && text[i] != '\r') {
        continue;
      }
      const size_t line_end = i;
      LineEnding ending = LineEnding::LF;
      if (text[i] == '\r') {
        if (i + 1 < text.length() && text[i + 1] == '\n') {
          ending = LineEnding::CRLF;
          ++i;
        } else {
          ending = LineEnding::CR;
        }
      }
      result.push_back({text.substr(line_start, line_end - line_start), ending});
      line_start = i + 1;
    }
    if (line_start < text.length()) {
      result.push_back({text.substr(line_start), LineEnding::NONE});
    } else if (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
      result.push_back({"", LineEnding::NONE});
    }
  }

  void Document::rebuildLineMetrics() {
    m_line_total_widths_.resize(m_lines_.size());
    m_line_start_indices_.resize(m_lines_.size());
    if (m_lines_.empty()) {
      return;
    }
    rebuildLineMetricsFrom(0);
  }

  void Document::rebuildLineMetricsFrom(size_t start_line) {
    const size_t line_count = m_lines_.size();
    m_line_total_widths_.resize(line_count);
    m_line_start_indices_.resize(line_count);
    if (line_count == 0 || start_line >= line_count) {
      return;
    }

    for (size_t line = start_line; line < line_count; ++line) {
      m_line_total_widths_[line] = getLineTotalWidth(m_lines_[line]);
    }

    if (start_line == 0) {
      m_line_start_indices_[0] = 0;
      start_line = 1;
    }

    for (size_t line = start_line; line < line_count; ++line) {
      m_line_start_indices_[line] = m_line_start_indices_[line - 1] + m_line_total_widths_[line - 1];
    }
  }

  size_t Document::getLineTotalWidth(const DocumentLine& line) {
    return Utf8Util::countChars(line.text) + getLineEndingWidth(line.ending);
  }

  PatchResult Document::patchSingleLine(const TextRange& range, const List<DocumentLine>& new_lines) {
    DocumentLine& line = m_lines_[range.start.line];
    const LineEnding original_ending = line.ending;
    // Convert to byte positions for operation
    size_t start_byte = Utf8Util::charPosToBytePos(line.text, range.start.column);
    size_t end_byte = Utf8Util::charPosToBytePos(line.text, range.end.column);

    // If patch text is empty, replace range with "", i.e. delete text in range
    if (new_lines.empty()) {
      line.text = line.text.substr(0, start_byte) + line.text.substr(end_byte);
      return {};
    }

    if (new_lines.size() == 1) {
      line.text = line.text.substr(0, start_byte) + new_lines[0].text + line.text.substr(end_byte);
      line.ending = original_ending;
      return {};
    } else {
      U8String rest_of_line = line.text.substr(end_byte);
      line.text = line.text.substr(0, start_byte) + new_lines[0].text;
      line.ending = new_lines[0].ending;
      for (size_t i = 1; i < new_lines.size(); ++i) {
        m_lines_.insert(m_lines_.begin() + range.start.line + i, new_lines[i]);
      }
      size_t last_line_index = range.start.line + new_lines.size() - 1;
      m_lines_[last_line_index].text += rest_of_line;
      m_lines_[last_line_index].ending = original_ending;
      PatchResult result;
      result.line_delta = static_cast<int32_t>(new_lines.size()) - 1;
      return result;
    }
  }

  PatchResult Document::patchMultipleLines(const TextRange& range, const List<DocumentLine>& new_lines) {
    const size_t start_line = range.start.line;
    const size_t end_line = range.end.line;
    DocumentLine& first_line = m_lines_[start_line];
    const size_t start_byte = Utf8Util::charPosToBytePos(first_line.text, range.start.column);
    const U8String left_side = first_line.text.substr(0, start_byte);

    const DocumentLine& last_line = m_lines_[end_line];
    const size_t end_byte = Utf8Util::charPosToBytePos(last_line.text, range.end.column);
    const U8String rest_of_last_line = last_line.text.substr(end_byte);
    const LineEnding ending_of_last_line = last_line.ending;

    if (new_lines.empty()) {
      first_line.text = left_side + rest_of_last_line;
      first_line.ending = ending_of_last_line;
      size_t start_delete = start_line + 1;
      size_t end_delete = end_line + 1;
      if (start_delete < end_delete) {
        m_lines_.erase(m_lines_.begin() + start_delete, m_lines_.begin() + end_delete);
      }
      PatchResult result;
      result.line_delta = -static_cast<int32_t>(end_line - start_line);
      return result;
    }

    first_line.text = left_side + new_lines[0].text;
    first_line.ending = new_lines.size() == 1 ? ending_of_last_line : new_lines[0].ending;

    size_t delete_start = start_line + 1;
    size_t delete_end = end_line + 1;
    if (delete_start < delete_end) {
      m_lines_.erase(m_lines_.begin() + delete_start, m_lines_.begin() + delete_end);
    }

    for (size_t i = 1; i < new_lines.size(); ++i) {
      m_lines_.insert(m_lines_.begin() + start_line + i, new_lines[i]);
    }

    size_t last_new_line_index = start_line + new_lines.size() - 1;
    m_lines_[last_new_line_index].text += rest_of_last_line;
    m_lines_[last_new_line_index].ending = ending_of_last_line;
    PatchResult result;
    result.line_delta = static_cast<int32_t>(new_lines.size() - (end_line - start_line));
    return result;
  }

  inline void Document::appendLineEnding(U8String& text, LineEnding ending) {
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

  uint8_t Document::getLineEndingWidth(LineEnding ending) {
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
