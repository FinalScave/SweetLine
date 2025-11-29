#ifndef SWEETLINE_NAPIUTIL_H
#define SWEETLINE_NAPIUTIL_H

#include "napi/native_api.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

template<typename T>
class NapiPtrHolder {
public:
  explicit NapiPtrHolder(const std::shared_ptr<T>& ptr): m_ptr_(ptr) {
  }

  std::shared_ptr<T>& get() {
    return m_ptr_;
  }
  
private:
  std::shared_ptr<T> m_ptr_;
};

static napi_value createNapiString(napi_env env, const std::string& str) {
  napi_value result;
  napi_create_string_utf8(env, str.c_str(), str.length(), &result);
  return result;
}

static napi_value createNapiInt32(napi_env env, int32_t value) {
  napi_value result;
  napi_create_int32(env, value, &result);
  return result;
}

static napi_value createNapiInt64(napi_env env, int64_t value) {
  napi_value result;
  napi_create_int64(env, value, &result);
  return result;
}

static napi_value getNapiBoolean(napi_env env, bool value) {
  napi_value result;
  napi_get_boolean(env, value, &result);
  return result;
}

static napi_value getNapiUndefined(napi_env env) {
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

template<typename T, typename... Args>
napi_value makePtrHolderToNapiHandle(napi_env env, Args... args) {
  std::shared_ptr<T> ptr = std::make_shared<T>((args)...);
  NapiPtrHolder<T>* holder = new NapiPtrHolder<T>(ptr);
  return createNapiInt64(env, reinterpret_cast<int64_t>(holder));
}

template<typename T>
napi_value toNapiHandle(napi_env env, T* raw_ptr) {
  if (raw_ptr == nullptr) {
    return createNapiInt64(env, 0);
  }
  return createNapiInt64(env, reinterpret_cast<int64_t>(raw_ptr));
}

template<typename T>
napi_value toNapiHandle(napi_env env, NapiPtrHolder<T>* holder) {
  return createNapiInt64(env, reinterpret_cast<int64_t>(holder));
}

template<typename T>
napi_value toNapiHandle(napi_env env, const std::shared_ptr<T>& ptr) {
  if (ptr == nullptr) {
    return createNapiInt64(env, 0);
  }
  NapiPtrHolder<T>* holder = new NapiPtrHolder<T>(ptr);
  return toNapiHandle<T>(env, holder);
}

template<typename T>
NapiPtrHolder<T>* toNativePtrHolder(napi_env env, napi_value handle) {
  int64_t native_handle;
  napi_status status = napi_get_value_int64(env, handle, &native_handle);
  if (status != napi_ok || native_handle == 0) {
    return nullptr;
  }
  return reinterpret_cast<NapiPtrHolder<T>*>(native_handle);
}

template<typename T>
std::shared_ptr<T> getNativePtrHolderValue(napi_env env, napi_value handle) {
  NapiPtrHolder<T>* holder = toNativePtrHolder<T>(env, handle);
  if (holder != nullptr) {
    return holder->get();
  } else {
    return nullptr;
  }
}

template<typename T>
napi_value releasePtrHolder(napi_env env, napi_value handle) {
  NapiPtrHolder<T>* holder = toNativePtrHolder<T>(env, handle);
  if (holder != nullptr) {
    delete holder;
    napi_value result;
    napi_create_int64(env, 0, &result);
    return result;
  }
  return handle;
}

static bool getStdStringFromNapiValue(napi_env env, napi_value value, std::string& result, size_t max_length = 0) {
  if (env == nullptr || value == nullptr) {
    return false;
  }
  
  napi_valuetype value_type;
  napi_status status = napi_typeof(env, value, &value_type);
  if (status != napi_ok || value_type != napi_string) {
    return false;
  }
  
  size_t str_length = 0;
  status = napi_get_value_string_utf8(env, value, nullptr, 0, &str_length);
  if (status != napi_ok) {
    return false;
  }
  
  if (str_length > max_length && max_length > 0) {
    str_length = max_length;
  }
  
  std::vector<char> buffer(str_length + 1);
  size_t copied_length = 0;
  status = napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &copied_length);
  if (status != napi_ok) {
    return false;
  }

  result.assign(buffer.data(), copied_length);
  return true;
}

#endif //SWEETLINE_NAPIUTIL_H
