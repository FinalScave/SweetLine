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

template<typename T, typename... Args>
intptr_t makeCPtrHolderToIntPtr(Args... args) {
  SharedPtr<T> ptr = makeSharedPtr<T>((args)...);
  CPtrHolder<T>* holder = new CPtrHolder<T>(ptr);
  return reinterpret_cast<intptr_t>(holder);
}

template<typename T>
intptr_t toIntPtr(CPtrHolder<T>* holder) {
  return reinterpret_cast<intptr_t>(holder);
}

template<typename T>
intptr_t toIntPtr(const SharedPtr<T>& ptr) {
  if (ptr == nullptr) {
    return 0;
  }
  CPtrHolder<T>* holder = new CPtrHolder<T>(ptr);
  return reinterpret_cast<intptr_t>(holder);
}

template<typename T>
CPtrHolder<T>* toCPtrHolder(intptr_t handle) {
  if (handle == 0) {
    return nullptr;
  }
  return reinterpret_cast<CPtrHolder<T>*>(handle);
}

template<typename T>
SharedPtr<T> getCPtrHolderValue(intptr_t handle) {
  if (handle == 0) {
    return nullptr;
  }
  CPtrHolder<T>* holder = reinterpret_cast<CPtrHolder<T>*>(handle);
  if (holder != nullptr) {
    return holder->get();
  } else {
    return nullptr;
  }
}

template<typename T>
void deleteCPtrHolder(intptr_t handle) {
  if (handle == 0) {
    return;
  }
  CPtrHolder<T>* holder = reinterpret_cast<CPtrHolder<T>*>(handle);
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

inline size_t computeSpanBufferSize(const HighlightConfig& config) {
  size_t base_count = 6; //起始和结束行列，以及索引
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
        buffer[index++] = static_cast<int32_t>(span.inline_style.tags);
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
      buffer[index++] = static_cast<int32_t>(span.inline_style.tags);
    } else {
      buffer[index++] = static_cast<int32_t>(span.style_id);
    }
  }
}

#endif //SWEETLINE_C_WRAPPER_HPP
