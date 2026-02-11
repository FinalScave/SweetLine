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

inline void writeDocumentHighlight(const SharedPtr<DocumentHighlight>& highlight, int32_t* buffer, const HighlightConfig& config) {
  size_t index = 0;
  for (const LineHighlight& line : highlight->lines) {
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

#endif //SWEETLINE_C_WRAPPER_HPP
