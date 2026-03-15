#include "napi/native_api.h"
#include "napi_util.h"
#include "highlight.h"
#include "internal_highlight.h"
#include <cstdint>

using namespace NS_SWEETLINE;

/// 将文档高亮结果转换为 Int32Array 返回给 ArkTS 层
/// 返回的数组结构:
/// buffer[0] = 高亮块数量
/// buffer[1] = 每个高亮块包含的整数字段数量(stride)
/// 后续数据包含 buffer[0] * buffer[1] 个整数字段
static napi_value ConvertDocumentHighlightAsIntArray(napi_env env, const HighlightConfig& config,
                                                     const SharedPtr<DocumentHighlight>& highlight) {
  if (!highlight) {
    napi_throw_error(env, nullptr, "Highlight pointer is null");
    return nullptr;
  }
  
  size_t span_count = highlight->spanCount();
  if (span_count == 0) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  
  int32_t stride = computeSpanBufferStride(config);
  const size_t buffer_count = 2 + span_count * stride;
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  int32_t* raw_buffer = new(std::nothrow) int32_t[buffer_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }
  raw_buffer[0] = static_cast<int32_t>(span_count);
  raw_buffer[1] = stride;
  writeDocumentHighlight(highlight, raw_buffer + 2, config);

  napi_value array_buffer;
  napi_status status = napi_create_external_arraybuffer(
      env,
      raw_buffer,
      buffer_size,
      [](napi_env, void* data, void*) {
        delete[] static_cast<int32_t*>(data);
      },
      nullptr,
      &array_buffer
  );
  
  if (status != napi_ok) {
    delete[] raw_buffer;
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer");
    return nullptr;
  }

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, buffer_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray");
    return nullptr;
  }
  return typed_array;
}

/// 将指定行区域高亮切片转换为 Int32Array 返回给 ArkTS 层
/// 返回的数组结构:
/// buffer[0] = 切片起始行 start_line
/// buffer[1] = patch 后文档总行数 total_line_count
/// buffer[2] = 切片行数 line_count
/// buffer[3] = 高亮块数量 span_count
/// buffer[4] = 每个高亮块包含的整数字段数量(stride)
/// 后续数据包含 span_count * stride 个整数字段
static napi_value ConvertDocumentHighlightSliceAsIntArray(napi_env env, const HighlightConfig& config,
                                                           const SharedPtr<DocumentHighlightSlice>& slice) {
  if (!slice) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  int32_t* raw_buffer = writeDocumentHighlightSlice(slice, config);
  if (!raw_buffer) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  size_t span_count = static_cast<size_t>(raw_buffer[3]);
  size_t stride = static_cast<size_t>(raw_buffer[4]);
  size_t buffer_count = 5 + span_count * stride;
  size_t buffer_size = buffer_count * sizeof(int32_t);

  napi_value array_buffer;
  napi_status status = napi_create_external_arraybuffer(
      env,
      raw_buffer,
      buffer_size,
      [](napi_env, void* data, void*) {
        delete[] static_cast<int32_t*>(data);
      },
      nullptr,
      &array_buffer
  );
  if (status != napi_ok) {
    delete[] raw_buffer;
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer for DocumentHighlightSlice");
    return nullptr;
  }

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, buffer_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray for DocumentHighlightSlice");
    return nullptr;
  }
  return typed_array;
}

/// 将单行高亮分析结果转换为 Int32Array 返回给 ArkTS 层
/// 返回的数组结构:
/// buffer[0] = 高亮块数量
/// buffer[1] = 每个高亮块包含的整数字段数量(stride)
/// buffer[2] = 当前行分析结束时的状态ID
/// buffer[3] = 当前行分析的字符总数(不包含换行符)
/// 后续数据包含 buffer[0] * buffer[1] 个整数字段
static napi_value ConvertLineAnalyzeResultAsIntArray(napi_env env, const HighlightConfig& config,
                                                      const LineAnalyzeResult& result) {
  size_t span_count = result.highlight.spans.size();
  int32_t stride = computeSpanBufferStride(config);
  const size_t buffer_count = 4 + span_count * stride;
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  int32_t* raw_buffer = new(std::nothrow) int32_t[buffer_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }
  raw_buffer[0] = static_cast<int32_t>(span_count);
  raw_buffer[1] = stride;
  raw_buffer[2] = result.end_state;
  raw_buffer[3] = static_cast<int32_t>(result.char_count);
  writeLineHighlight(result.highlight, raw_buffer + 4, config);

  napi_value array_buffer;
  napi_status status = napi_create_external_arraybuffer(
      env,
      raw_buffer,
      buffer_size,
      [](napi_env, void* data, void*) {
        delete[] static_cast<int32_t*>(data);
      },
      nullptr,
      &array_buffer
  );

  if (status != napi_ok) {
    delete[] raw_buffer;
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer");
    return nullptr;
  }

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, buffer_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray");
    return nullptr;
  }
  return typed_array;
}

// ================================================ Document ====================================================

/// 创建引擎托管文档(Document)
static napi_value Document_Create(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  U8String uri;
  bool status = getStdStringFromNapiValue(env, args[0], uri);
  if (!status) {
    return createNapiInt64(env, 0);
  }
  U8String text;
  status = getStdStringFromNapiValue(env, args[1], text);
  if (!status) {
    return createNapiInt64(env, 0);
  }
  int64_t handle = makeCPtrHolderToHandle<int64_t, Document>(uri, text);
  return createNapiInt64(env, handle);
}

/// 销毁托管文档
static napi_value Document_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  deleteNapiCPtrHolder<Document>(env, args[0]);
  return getNapiUndefined(env);
}

/// 获取托管文档的Uri
static napi_value Document_GetUri(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return getNapiUndefined(env);
  }
  return createNapiString(env, document->getUri());
}

/// 文档字符总数
static napi_value Document_TotalChars(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  return createNapiInt32(env, static_cast<int32_t>(document->totalChars()));
}

/// 获取指定行的字符总数
static napi_value Document_GetLineCharCount(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  int32_t line = 0;
  napi_get_value_int32(env, args[1], &line);
  return createNapiInt32(env, static_cast<int32_t>(document->getLineCharCount(line)));
}

/// 计算指定行在全文中的起始索引
static napi_value Document_CharIndexOfLine(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  int32_t line = 0;
  napi_get_value_int32(env, args[1], &line);
  return createNapiInt32(env, static_cast<int32_t>(document->charIndexOfLine(line)));
}

/// 将字符索引转换为行列位置
static napi_value Document_CharIndexToPosition(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  int32_t index = 0;
  napi_get_value_int32(env, args[1], &index);
  TextPosition position = document->charIndexToPosition(index);
  napi_value array;
  napi_create_array_with_length(env, 2, &array);
  napi_set_element(env, array, 0, createNapiInt32(env, static_cast<int32_t>(position.line)));
  napi_set_element(env, array, 1, createNapiInt32(env, static_cast<int32_t>(position.column)));
  return array;
}

/// 获取总行数
static napi_value Document_GetLineCount(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  return createNapiInt32(env, static_cast<int32_t>(document->getLineCount()));
}

/// 获取指定行的文本信息
static napi_value Document_GetLine(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t line;
  napi_get_value_int32(env, args[1], &line);
  const U8String& line_text = document->getLine(line).text;
  return createNapiString(env, line_text);
}

/// 获取完整文本
static napi_value Document_GetText(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return getNapiUndefined(env);
  }
  U8String text = document->getText();
  return createNapiString(env, text);
}

// ================================================ SyntaxRule ====================================================

/// 获取语法规则的名称
static napi_value SyntaxRule_GetName(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<SyntaxRule> rule = getNapiCPtrHolderValue<SyntaxRule>(env, args[0]);
  if (rule == nullptr) {
    return getNapiUndefined(env);
  }
  return createNapiString(env, rule->name);
}

/// 获取语法规则支持的文件扩展名
static napi_value SyntaxRule_GetFileExtensions(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<SyntaxRule> rule = getNapiCPtrHolderValue<SyntaxRule>(env, args[0]);
  if (rule == nullptr) {
    return getNapiUndefined(env);
  }
  napi_value result;
  napi_create_array_with_length(env, rule->file_extensions.size(), &result);
  size_t i = 0;
  for (const U8String& ext : rule->file_extensions) {
    napi_set_element(env, result, i, createNapiString(env, ext));
    ++i;
  }
  return result;
}

// ================================================ TextAnalyzer ====================================================

/// 将 IndentGuideResult 转换为 Int32Array 返回给 ArkTS 层
static napi_value ConvertIndentGuideResultAsIntArray(napi_env env, const SharedPtr<IndentGuideResult>& result) {
  if (!result) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  int32_t* raw_buffer = writeIndentGuideResult(result);
  if (!raw_buffer) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  // 计算缓冲区总大小
  size_t guide_data_size = 0;
  for (const IndentGuideLine& g : result->guide_lines) {
    guide_data_size += 6 + g.branches.size() * 2;
  }
  size_t total_count = 4 + guide_data_size + result->line_states.size() * 4;
  size_t total_size = total_count * sizeof(int32_t);

  napi_value array_buffer;
  napi_status status = napi_create_external_arraybuffer(
      env,
      raw_buffer,
      total_size,
      [](napi_env, void* data, void*) {
        delete[] static_cast<int32_t*>(data);
      },
      nullptr,
      &array_buffer
  );
  if (status != napi_ok) {
    delete[] raw_buffer;
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer for IndentGuideResult");
    return nullptr;
  }

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, total_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray for IndentGuideResult");
    return nullptr;
  }
  return typed_array;
}

// ================================================ TextAnalyzer ====================================================

/// 销毁纯文本高亮分析器
static napi_value TextAnalyzer_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  deleteNapiCPtrHolder<TextAnalyzer>(env, args[0]);
  return getNapiUndefined(env);
}

/// 分析一段文本内容，并返回整段文本的高亮结果
static napi_value TextAnalyzer_AnalyzeText(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<TextAnalyzer> analyzer = getNapiCPtrHolderValue<TextAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String text;
  if (!getStdStringFromNapiValue(env, args[1], text)) {
    return getNapiUndefined(env);
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(text);
  return ConvertDocumentHighlightAsIntArray(env, analyzer->getHighlightConfig(), highlight);
}

/// 分析单行文本
/// args: [handle, text, line, startState, startCharOffset]
static napi_value TextAnalyzer_AnalyzeLine(napi_env env, napi_callback_info info) {
  size_t argc = 5;
  napi_value args[5] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<TextAnalyzer> analyzer = getNapiCPtrHolderValue<TextAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String text;
  if (!getStdStringFromNapiValue(env, args[1], text)) {
    return getNapiUndefined(env);
  }
  int32_t line = 0, start_state = 0, start_char_offset = 0;
  napi_get_value_int32(env, args[2], &line);
  napi_get_value_int32(env, args[3], &start_state);
  napi_get_value_int32(env, args[4], &start_char_offset);

  TextLineInfo line_info = {static_cast<size_t>(line), start_state, static_cast<size_t>(start_char_offset)};
  LineAnalyzeResult result;
  analyzer->analyzeLine(text, line_info, result);
  return ConvertLineAnalyzeResultAsIntArray(env, analyzer->getHighlightConfig(), result);
}

/// 对纯文本进行缩进划线分析（内部会先进行高亮分析）
/// args: [handle, text]
static napi_value TextAnalyzer_AnalyzeIndentGuides(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<TextAnalyzer> analyzer = getNapiCPtrHolderValue<TextAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String text;
  if (!getStdStringFromNapiValue(env, args[1], text)) {
    return getNapiUndefined(env);
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(text);
  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides(text, highlight);
  return ConvertIndentGuideResultAsIntArray(env, result);
}

// ================================================ DocumentAnalyzer ====================================================

/// 销毁文档高亮分析器
static napi_value DocumentAnalyzer_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<DocumentAnalyzer>(env, args[0]);
  return getNapiUndefined(env);
}

/// 对整个文本进行高亮分析
static napi_value DocumentAnalyzer_Analyze(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  return ConvertDocumentHighlightAsIntArray(env, analyzer->getHighlightConfig(), highlight);
}

/// 根据patch内容重新分析整个文本的高亮结果(行列范围)
static napi_value DocumentAnalyzer_AnalyzeChanges(napi_env env, napi_callback_info info) {
  size_t argc = 6;
  napi_value args[6] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String new_text;
  if (!getStdStringFromNapiValue(env, args[5], new_text)) {
    return getNapiUndefined(env);
  }
  int32_t start_line = 0, start_column = 0, end_line = 0, end_column = 0;
  napi_get_value_int32(env, args[1], &start_line);
  napi_get_value_int32(env, args[2], &start_column);
  napi_get_value_int32(env, args[3], &end_line);
  napi_get_value_int32(env, args[4], &end_column);
  TextRange range = {
    {static_cast<size_t>(start_line), static_cast<size_t>(start_column)},
    {static_cast<size_t>(end_line), static_cast<size_t>(end_column)}
  };
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeIncremental(range, new_text);
  return ConvertDocumentHighlightAsIntArray(env, analyzer->getHighlightConfig(), highlight);
}

/// 根据patch内容重新分析文本，并返回指定可见行区域高亮切片
/// args: [handle, startLine, startColumn, endLine, endColumn, newText, visibleStartLine, visibleLineCount]
static napi_value DocumentAnalyzer_AnalyzeChangesInLineRange(napi_env env, napi_callback_info info) {
  size_t argc = 8;
  napi_value args[8] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String new_text;
  if (!getStdStringFromNapiValue(env, args[5], new_text)) {
    return getNapiUndefined(env);
  }
  int32_t start_line = 0, start_column = 0, end_line = 0, end_column = 0;
  napi_get_value_int32(env, args[1], &start_line);
  napi_get_value_int32(env, args[2], &start_column);
  napi_get_value_int32(env, args[3], &end_line);
  napi_get_value_int32(env, args[4], &end_column);
  int32_t visible_start_line = 0, visible_line_count = 0;
  napi_get_value_int32(env, args[6], &visible_start_line);
  napi_get_value_int32(env, args[7], &visible_line_count);

  TextRange range = {
      {static_cast<size_t>(start_line), static_cast<size_t>(start_column)},
      {static_cast<size_t>(end_line), static_cast<size_t>(end_column)}
  };
  LineRange visible_range = {static_cast<size_t>(visible_start_line), static_cast<size_t>(visible_line_count)};
  SharedPtr<DocumentHighlightSlice> slice = analyzer->analyzeIncrementalInLineRange(range, new_text, visible_range);
  return ConvertDocumentHighlightSliceAsIntArray(env, analyzer->getHighlightConfig(), slice);
}

/// 根据patch内容重新分析整个文本的高亮结果(字符索引范围)
static napi_value DocumentAnalyzer_AnalyzeChanges2(napi_env env, napi_callback_info info) {
  size_t argc = 4;
  napi_value args[4] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  U8String new_text;
  if (!getStdStringFromNapiValue(env, args[3], new_text)) {
    return getNapiUndefined(env);
  }
  int32_t start_index = 0, end_index = 0;
  napi_get_value_int32(env, args[1], &start_index);
  napi_get_value_int32(env, args[2], &end_index);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeIncremental(
    static_cast<size_t>(start_index), static_cast<size_t>(end_index), new_text);
  return ConvertDocumentHighlightAsIntArray(env, analyzer->getHighlightConfig(), highlight);
}

/// 对托管文档进行缩进划线分析
static napi_value DocumentAnalyzer_AnalyzeIndentGuides(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides();
  return ConvertIndentGuideResultAsIntArray(env, result);
}

/// 获取托管文档对象
static napi_value DocumentAnalyzer_GetDocument(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  int64_t handle = asCHandle<int64_t>(analyzer->getDocument());
  return createNapiInt64(env, handle);
}

// ================================================ HighlightEngine ====================================================

/// 创建SweetLine高亮引擎
/// configBits: bit0=showIndex, bit1=inlineStyle
static napi_value HighlightEngine_Create(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  int32_t config_bits = 0;
  napi_get_value_int32(env, args[0], &config_bits);
  HighlightConfig config = unpackHighlightConfig(config_bits);
  int64_t handle = makeCPtrHolderToHandle<int64_t, HighlightEngine>(config);
  return createNapiInt64(env, handle);
}

/// 销毁高亮引擎
static napi_value HighlightEngine_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<HighlightEngine>(env, args[0]);
  return getNapiUndefined(env);
}

/// 注册一个高亮样式，用于名称映射
static napi_value HighlightEngine_RegisterStyleName(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  U8String name;
  if (!getStdStringFromNapiValue(env, args[1], name)) {
    return getNapiBoolean(env, false);
  }
  int32_t style_id = 0;
  napi_get_value_int32(env, args[2], &style_id);
  engine->registerStyleName(name, style_id);
  return getNapiBoolean(env, true);
}

/// 通过样式id获取注册的样式名称
static napi_value HighlightEngine_GetStyleName(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t style_id = 0;
  napi_get_value_int32(env, args[1], &style_id);
  const U8String& name = engine->getStyleName(style_id);
  return createNapiString(env, name);
}

/// 定义一个宏，用于控制importSyntax的#ifdef条件编译
static napi_value HighlightEngine_DefineMacro(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  U8String macro_name;
  if (!getStdStringFromNapiValue(env, args[1], macro_name)) {
    return getNapiBoolean(env, false);
  }
  engine->defineMacro(macro_name);
  return getNapiBoolean(env, true);
}

/// 取消定义宏
static napi_value HighlightEngine_UndefineMacro(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  U8String macro_name;
  if (!getStdStringFromNapiValue(env, args[1], macro_name)) {
    return getNapiBoolean(env, false);
  }
  engine->undefineMacro(macro_name);
  return getNapiBoolean(env, true);
}

/// 通过json编译语法规则
static napi_value HighlightEngine_CompileSyntaxFromJson(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  U8String json;
  if (!getStdStringFromNapiValue(env, args[1], json)) {
    return createNapiInt64(env, 0);
  }
  try {
    SharedPtr<SyntaxRule> rule = engine->compileSyntaxFromJson(json);
    int64_t handle = asCHandle<int64_t>(rule);
    return createNapiInt64(env, handle);
  } catch (SyntaxRuleParseError& error) {
    U8String msg = U8String(error.what()) + ": " + error.message();
    napi_throw_error(env, nullptr, msg.c_str());
    return createNapiInt64(env, 0);
  }
}

/// 编译语法规则(读取json配置文件)
static napi_value HighlightEngine_CompileSyntaxFromFile(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  U8String path;
  if (!getStdStringFromNapiValue(env, args[1], path)) {
    return createNapiInt64(env, 0);
  }
  try {
    SharedPtr<SyntaxRule> rule = engine->compileSyntaxFromFile(path);
    int64_t handle = asCHandle<int64_t>(rule);
    return createNapiInt64(env, handle);
  } catch (SyntaxRuleParseError& error) {
    U8String msg = U8String(error.what()) + ": " + error.message();
    napi_throw_error(env, nullptr, msg.c_str());
    return createNapiInt64(env, 0);
  }
}

/// 根据语法规则名称创建一个文本高亮分析器(不支持增量分析)
static napi_value HighlightEngine_CreateAnalyzerByName(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  U8String syntax_name;
  if (!getStdStringFromNapiValue(env, args[1], syntax_name)) {
    return createNapiInt64(env, 0);
  }
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByName(syntax_name);
  int64_t handle = asCHandle<int64_t>(analyzer);
  return createNapiInt64(env, handle);
}

/// 根据文件后缀名创建一个文本高亮分析器(不支持增量分析)
static napi_value HighlightEngine_CreateAnalyzerByExtension(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  U8String extension;
  if (!getStdStringFromNapiValue(env, args[1], extension)) {
    return createNapiInt64(env, 0);
  }
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByExtension(extension);
  int64_t handle = asCHandle<int64_t>(analyzer);
  return createNapiInt64(env, handle);
}

/// 加载托管文档对象获得文档高亮分析器
static napi_value HighlightEngine_LoadDocument(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[1]);
  if (document == nullptr) {
    return createNapiInt64(env, 0);
  }
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  int64_t handle = asCHandle<int64_t>(analyzer);
  return createNapiInt64(env, handle);
}

/// 移除托管文档
static napi_value HighlightEngine_RemoveDocument(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  U8String uri;
  if (!getStdStringFromNapiValue(env, args[1], uri)) {
    return getNapiBoolean(env, false);
  }
  engine->removeDocument(uri);
  return getNapiBoolean(env, true);
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
    // Document
    {"Document_Create", nullptr, Document_Create, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_Delete", nullptr, Document_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetUri", nullptr, Document_GetUri, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_TotalChars", nullptr, Document_TotalChars, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLineCharCount", nullptr, Document_GetLineCharCount, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_CharIndexOfLine", nullptr, Document_CharIndexOfLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_CharIndexToPosition", nullptr, Document_CharIndexToPosition, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLineCount", nullptr, Document_GetLineCount, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLine", nullptr, Document_GetLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetText", nullptr, Document_GetText, nullptr, nullptr, nullptr, napi_default, nullptr},
    // SyntaxRule
    {"SyntaxRule_GetName", nullptr, SyntaxRule_GetName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SyntaxRule_GetFileExtensions", nullptr, SyntaxRule_GetFileExtensions, nullptr, nullptr, nullptr, napi_default, nullptr},
    // TextAnalyzer
    {"TextAnalyzer_Delete", nullptr, TextAnalyzer_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeText", nullptr, TextAnalyzer_AnalyzeText, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeLine", nullptr, TextAnalyzer_AnalyzeLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeIndentGuides", nullptr, TextAnalyzer_AnalyzeIndentGuides, nullptr, nullptr, nullptr, napi_default, nullptr},
    // DocumentAnalyzer
    {"DocumentAnalyzer_Delete", nullptr, DocumentAnalyzer_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_Analyze", nullptr, DocumentAnalyzer_Analyze, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges", nullptr, DocumentAnalyzer_AnalyzeChanges, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChangesInLineRange", nullptr, DocumentAnalyzer_AnalyzeChangesInLineRange, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges2", nullptr, DocumentAnalyzer_AnalyzeChanges2, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeIndentGuides", nullptr, DocumentAnalyzer_AnalyzeIndentGuides, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_GetDocument", nullptr, DocumentAnalyzer_GetDocument, nullptr, nullptr, nullptr, napi_default, nullptr},
    // HighlightEngine
    {"HighlightEngine_Create", nullptr, HighlightEngine_Create, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_Delete", nullptr, HighlightEngine_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_RegisterStyleName", nullptr, HighlightEngine_RegisterStyleName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_GetStyleName", nullptr, HighlightEngine_GetStyleName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_DefineMacro", nullptr, HighlightEngine_DefineMacro, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_UndefineMacro", nullptr, HighlightEngine_UndefineMacro, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CompileSyntaxFromJson", nullptr, HighlightEngine_CompileSyntaxFromJson, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CompileSyntaxFromFile", nullptr, HighlightEngine_CompileSyntaxFromFile, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CreateAnalyzerByName", nullptr, HighlightEngine_CreateAnalyzerByName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CreateAnalyzerByExtension", nullptr, HighlightEngine_CreateAnalyzerByExtension, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_LoadDocument", nullptr, HighlightEngine_LoadDocument, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_RemoveDocument", nullptr, HighlightEngine_RemoveDocument, nullptr, nullptr, nullptr, napi_default, nullptr},
  };
  napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
  return exports;
}
EXTERN_C_END

static napi_module libModule = {
  .nm_version = 1,
  .nm_flags = 0,
  .nm_filename = nullptr,
  .nm_register_func = Init,
  .nm_modname = "sweetline",
  .nm_priv = ((void *)0),
  .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterSweetlineModule(void) { 
  napi_module_register(&libModule);
}
