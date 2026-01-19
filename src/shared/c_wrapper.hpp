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
  std::vector<String> vector_;
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

inline void convertHighlightsToBuffer(const SharedPtr<DocumentHighlight>& highlight, int32_t* buffer, bool with_index) {
  size_t index = 0;
  if (with_index) {
    for (const LineHighlight& line : highlight->lines) {
      for (const TokenSpan& span : line.spans) {
        buffer[index++] = static_cast<int32_t>(span.range.start.line);
        buffer[index++] = static_cast<int32_t>(span.range.start.column);
        buffer[index++] = static_cast<int32_t>(span.range.start.index);
        buffer[index++] = static_cast<int32_t>(span.range.end.line);
        buffer[index++] = static_cast<int32_t>(span.range.end.column);
        buffer[index++] = static_cast<int32_t>(span.range.end.index);
        buffer[index++] = static_cast<int32_t>(span.style);
      }
    }
  } else {
    for (const LineHighlight& line : highlight->lines) {
      for (const TokenSpan& span : line.spans) {
        buffer[index++] = static_cast<int32_t>(span.range.start.line);
        buffer[index++] = static_cast<int32_t>(span.range.start.column);
        buffer[index++] = static_cast<int32_t>(span.range.end.line);
        buffer[index++] = static_cast<int32_t>(span.range.end.column);
        buffer[index++] = static_cast<int32_t>(span.style);
      }
    }
  }
}

#endif //SWEETLINE_C_WRAPPER_HPP
