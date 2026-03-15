#ifndef SWEETLINE_C_WRAPPER_HPP
#define SWEETLINE_C_WRAPPER_HPP

#include "../include/c_sweetline.h"
#include "highlight.h"

class StringKeepAlive {
public:
  static StringKeepAlive& getInstance();
  void clear();

  template<typename U>
  const char* getAliveCString(U&& str) {
    vector_.push_back(std::forward<U>(str));
    return vector_.back().c_str();
  }
private:
  std::vector<U8String> vector_;
};

template<typename T>
class CPtrHolder {
public:
  explicit CPtrHolder(const SharedPtr<T>& ptr): m_ptr_(ptr) {
  }

  SharedPtr<T>& get() {
    return m_ptr_;
  }
private:
  SharedPtr<T> m_ptr_;
};

template<typename HandleT, typename ClassT, typename... Args>
HandleT makeCPtrHolderToHandle(Args&&... args) {
  SharedPtr<ClassT> ptr = makeSharedPtr<ClassT>((std::forward<Args>(args))...);
  CPtrHolder<ClassT>* holder = new CPtrHolder<ClassT>(ptr);
  return reinterpret_cast<HandleT>(holder);
}

template<typename HandleT, typename ClassT>
HandleT asCHandle(const SharedPtr<ClassT>& ptr_v) {
  CPtrHolder<ClassT>* holder = new CPtrHolder<ClassT>(ptr_v);
  return reinterpret_cast<HandleT>(holder);
}

template<typename HandleT, typename ClassT>
SharedPtr<ClassT> getCPtrHolderValue(HandleT handle) {
  if (handle == 0) {
    return nullptr;
  }
  CPtrHolder<ClassT>* holder = reinterpret_cast<CPtrHolder<ClassT>*>(handle);
  if (holder != nullptr) {
    return holder->get();
  } else {
    return nullptr;
  }
}

template<typename HandleT, typename ClassT>
void deleteCPtrHolder(HandleT handle) {
  if (handle == 0) {
    return;
  }
  CPtrHolder<ClassT>* holder = reinterpret_cast<CPtrHolder<ClassT>*>(handle);
  if (holder != nullptr) {
    delete holder;
  }
}

using namespace NS_SWEETLINE;

inline int32_t packHighlightConfig(const HighlightConfig& config) {
  int32_t bits = 0;
  if (config.show_index) {
    bits |= 1;
  }
  if (config.inline_style) {
    bits |= 1 << 1;
  }
  // tab_size 编码到 bit8~bit15 (8位，支持0~255)
  bits |= (config.tab_size & 0xFF) << 8;
  return bits;
}

inline HighlightConfig unpackHighlightConfig(int32_t bits) {
  HighlightConfig config;
  if ((bits & 1) != 0) {
    config.show_index = true;
  }
  if ((bits & (1 << 1)) != 0) {
    config.inline_style = true;
  }
  int32_t tab_size = (bits >> 8) & 0xFF;
  if (tab_size > 0) {
    config.tab_size = tab_size;
  }
  return config;
}

inline int32_t packInlineStyleTags(const InlineStyle& style) {
  int32_t bits = 0;
  if (style.is_bold) {
    bits |= kStyleBold;
  }
  if (style.is_italic) {
    bits |= kStyleItalic;
  }
  if (style.is_strikethrough) {
    bits |= kStyleStrikeThrough;
  }
  return bits;
}

inline int64_t packTextPosition(const TextPosition& position) {
  int64_t line = (int64_t)position.line;
  int64_t column = (int64_t)position.column;
  return (line << 32) | (column & 0XFFFFFFFFLL);
}

inline TextPosition unpackTextPosition(int64_t bits) {
  size_t start_line = (size_t)(int32_t)(bits >> 32);
  size_t start_column = (size_t)(int32_t)(bits & 0XFFFFFFFF);
  return {start_line, start_column};
}

inline TextLineInfo unpackTextLineInfo(int32_t* arr) {
  return {static_cast<size_t>(arr[0]), arr[1], static_cast<size_t>(arr[2])};
}

inline int32_t computeSpanBufferStride(const HighlightConfig& config) {
  int32_t base_count = 6; //起始和结束行列，以及索引
  if (config.inline_style) {
    base_count += 3; //前景色、背景色、tags各占一个int32
  } else {
    base_count += 1; //styleId占一个
  }
  return base_count;
}

inline size_t computeLineHighlightsSpanCount(const List<LineHighlight>& lines) {
  size_t span_count = 0;
  for (const LineHighlight& line : lines) {
    span_count += line.spans.size();
  }
  return span_count;
}

inline void writeLineHighlights(const List<LineHighlight>& lines, int32_t* buffer, const HighlightConfig& config) {
  size_t index = 0;
  for (const LineHighlight& line : lines) {
    for (const TokenSpan& span : line.spans) {
      buffer[index++] = static_cast<int32_t>(span.range.start.line);
      buffer[index++] = static_cast<int32_t>(span.range.start.column);
      buffer[index++] = static_cast<int32_t>(span.range.start.index);
      buffer[index++] = static_cast<int32_t>(span.range.end.line);
      buffer[index++] = static_cast<int32_t>(span.range.end.column);
      buffer[index++] = static_cast<int32_t>(span.range.end.index);
      if (config.inline_style) {
        buffer[index++] = static_cast<int32_t>(span.inline_style.foreground);
        buffer[index++] = static_cast<int32_t>(span.inline_style.background);
        buffer[index++] = packInlineStyleTags(span.inline_style);
      } else {
        buffer[index++] = static_cast<int32_t>(span.style_id);
      }
    }
  }
}

inline void writeDocumentHighlight(const SharedPtr<DocumentHighlight>& highlight, int32_t* buffer, const HighlightConfig& config) {
  writeLineHighlights(highlight->lines, buffer, config);
}

inline void writeLineHighlight(const LineHighlight& highlight, int32_t* buffer, const HighlightConfig& config) {
  size_t index = 0;
  for (const TokenSpan& span : highlight.spans) {
    buffer[index++] = static_cast<int32_t>(span.range.start.line);
    buffer[index++] = static_cast<int32_t>(span.range.start.column);
    buffer[index++] = static_cast<int32_t>(span.range.start.index);
    buffer[index++] = static_cast<int32_t>(span.range.end.line);
    buffer[index++] = static_cast<int32_t>(span.range.end.column);
    buffer[index++] = static_cast<int32_t>(span.range.end.index);
    if (config.inline_style) {
      buffer[index++] = static_cast<int32_t>(span.inline_style.foreground);
      buffer[index++] = static_cast<int32_t>(span.inline_style.background);
      buffer[index++] = packInlineStyleTags(span.inline_style);
    } else {
      buffer[index++] = static_cast<int32_t>(span.style_id);
    }
  }
}

/// 将 DocumentHighlightSlice 序列化为 int32_t 缓冲区
/// 布局:
/// buffer[0] = 切片起始行 start_line
/// buffer[1] = patch 后文档总行数 total_line_count
/// buffer[2] = 切片行数 line_count
/// buffer[3] = 高亮块数量 span_count
/// buffer[4] = 每个高亮块包含的整数字段数量(stride)
/// 后续数据包含 span_count * stride 个整数字段
inline int32_t* writeDocumentHighlightSlice(const SharedPtr<DocumentHighlightSlice>& slice, const HighlightConfig& config) {
  if (slice == nullptr) {
    return nullptr;
  }
  size_t span_count = computeLineHighlightsSpanCount(slice->lines);
  size_t stride = static_cast<size_t>(computeSpanBufferStride(config));
  size_t total_size = 5 + span_count * stride;
  int32_t* buffer = new int32_t[total_size];
  buffer[0] = static_cast<int32_t>(slice->start_line);
  buffer[1] = static_cast<int32_t>(slice->total_line_count);
  buffer[2] = static_cast<int32_t>(slice->lines.size());
  buffer[3] = static_cast<int32_t>(span_count);
  buffer[4] = static_cast<int32_t>(stride);
  writeLineHighlights(slice->lines, buffer + 5, config);
  return buffer;
}

/// 将 IndentGuideResult 序列化为 int32_t 缓冲区
/// 布局:
/// buffer[0] = guide_lines 数量
/// buffer[1] = 每条 guide_line 的固定字段数 (stride, 不含 branches)
/// buffer[2] = line_states 数量（即行数）
/// buffer[3] = 每行 line_state 的字段数 (4)
/// 之后依次写入:
///   guide_lines: [column, start_line, end_line, nesting_level, scope_rule_id, branch_count, branch_line_0, branch_column_0, ...]
///   line_states: [nesting_level, scope_state, scope_column, indent_level] * line_count
inline int32_t* writeIndentGuideResult(const SharedPtr<IndentGuideResult>& result) {
  if (result == nullptr) {
    return nullptr;
  }
  // 计算 guide_lines 部分的总 int 数
  size_t guide_data_size = 0;
  for (const IndentGuideLine& g : result->guide_lines) {
    guide_data_size += 6 + g.branches.size() * 2; // 5 fixed fields + branch_count + (line, column) per branch
  }
  size_t line_state_count = result->line_states.size();
  constexpr int32_t line_state_stride = 4;
  size_t total_size = 4 + guide_data_size + line_state_count * line_state_stride;
  int32_t* buffer = new int32_t[total_size];

  buffer[0] = static_cast<int32_t>(result->guide_lines.size());
  buffer[1] = 6; // stride (fixed fields per guide_line, excluding variable branches)
  buffer[2] = static_cast<int32_t>(line_state_count);
  buffer[3] = line_state_stride;

  size_t idx = 4;
  for (const IndentGuideLine& g : result->guide_lines) {
    buffer[idx++] = g.column;
    buffer[idx++] = g.start_line;
    buffer[idx++] = g.end_line;
    buffer[idx++] = g.nesting_level;
    buffer[idx++] = g.scope_rule_id;
    buffer[idx++] = static_cast<int32_t>(g.branches.size());
    for (const auto& bp : g.branches) {
      buffer[idx++] = bp.line;
      buffer[idx++] = bp.column;
    }
  }
  for (const LineScopeState& ls : result->line_states) {
    buffer[idx++] = ls.nesting_level;
    buffer[idx++] = static_cast<int32_t>(ls.scope_state);
    buffer[idx++] = ls.scope_column;
    buffer[idx++] = ls.indent_level;
  }
  return buffer;
}

#endif //SWEETLINE_C_WRAPPER_HPP
