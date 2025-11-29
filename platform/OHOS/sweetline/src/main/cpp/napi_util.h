#ifndef SWEETLINE_NAPIUTIL_H
#define SWEETLINE_NAPIUTIL_H

#include "napi/native_api.h"
#include "c_wrapper.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

static int64_t getNapiInt64(napi_env env, napi_value value) {
  int64_t res = 0;
  napi_get_value_int64(env, value, &res);
  return res;
}

template<typename T>
std::shared_ptr<T> getNapiCPtrHolderValue(napi_env env, napi_value handle_value) {
  int64_t handle = getNapiInt64(env, handle_value);
  return getCPtrHolderValue<int64_t, T>(handle);
}

template<typename T>
void deleteNapiCPtrHolder(napi_env env, napi_value handle_value) {
  int64_t handle = getNapiInt64(env, handle_value);
  return deleteCPtrHolder<int64_t, T>(handle);
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
