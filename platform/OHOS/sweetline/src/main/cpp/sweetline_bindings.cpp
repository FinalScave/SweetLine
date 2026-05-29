#include "napi/native_api.h"
#include "napi_util.h"
#include "sweetline/highlight.h"
#include "internal_highlight.h"
#include <cstdint>

using namespace NS_SWEETLINE;

static void ThrowSyntaxCompileError(napi_env env, const SyntaxCompileError& error) {
  napi_value message;
  napi_create_string_utf8(env, error.message().c_str(), NAPI_AUTO_LENGTH, &message);

  napi_value js_error;
  napi_create_error(env, nullptr, message, &js_error);

  napi_value name;
  napi_create_string_utf8(env, "SyntaxCompileError", NAPI_AUTO_LENGTH, &name);
  napi_set_named_property(env, js_error, "name", name);

  napi_value code;
  napi_create_int32(env, error.code(), &code);
  napi_set_named_property(env, js_error, "errorCode", code);

  napi_throw(env, js_error);
}

/// Convert document highlight result to Int32Array for ArkTS layer
/// Returned array structure:
/// buffer[0] = span payload flags(bit0: hasStartIndex, bit1: inlineStyle)
/// buffer[1] = integer field count per token span(stride)
/// buffer[2] = line count
/// Followed by line entries:
///   line_entry[0] = token span count of current line
///   followed by span payloads
static napi_value ConvertDocumentHighlightAsIntArray(napi_env env, const HighlightConfig& config,
                                                     const SharedPtr<DocumentHighlight>& highlight) {
  if (!highlight) {
    napi_throw_error(env, nullptr, "Highlight pointer is null");
    return nullptr;
  }

  const size_t buffer_count = computeDocumentHighlightBufferSize(highlight, config);
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  int32_t* raw_buffer = new(std::nothrow) int32_t[buffer_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }
  writeDocumentHighlight(highlight, raw_buffer, config);

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

/// Convert highlight slice for specified line range to Int32Array for ArkTS layer
/// Returned array structure:
/// buffer[0] = span payload flags(bit0: hasStartIndex, bit1: inlineStyle)
/// buffer[1] = integer field count per token span(stride)
/// buffer[2] = slice start line start_line
/// buffer[3] = total line count after patch total_line_count
/// buffer[4] = slice line count line_count
/// Followed by line entries:
///   line_entry[0] = token span count of current line
///   followed by span payloads
static napi_value ConvertDocumentHighlightSliceAsIntArray(napi_env env, const HighlightConfig& config,
                                                           const SharedPtr<DocumentHighlightSlice>& slice) {
  if (!slice) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  const size_t buffer_count = computeDocumentHighlightSliceBufferSize(slice, config);
  const size_t buffer_size = buffer_count * sizeof(int32_t);
  int32_t* raw_buffer = new(std::nothrow) int32_t[buffer_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory for DocumentHighlightSlice");
    return nullptr;
  }
  writeDocumentHighlightSlice(slice, raw_buffer, config);

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

/// Convert single line analysis result to Int32Array for ArkTS layer
/// Returned array structure:
/// buffer[0] = span payload flags(bit0: hasStartIndex, bit1: inlineStyle)
/// buffer[1] = integer field count per token span(stride)
/// buffer[2] = token span count
/// buffer[3] = state ID at end of current line analysis
/// buffer[4] = total characters analyzed in current line (excluding line ending)
/// Followed by the corresponding number of integer fields
static napi_value ConvertLineAnalyzeResultAsIntArray(napi_env env, const HighlightConfig& config,
                                                      const LineAnalyzeResult& result) {
  size_t span_count = result.highlight.spans.size();
  int32_t stride = computeSpanBufferStride(config);
  const size_t buffer_count = 5 + span_count * stride;
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  int32_t* raw_buffer = new(std::nothrow) int32_t[buffer_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }
  raw_buffer[0] = packSpanPayloadFlags(config);
  raw_buffer[1] = stride;
  raw_buffer[2] = static_cast<int32_t>(span_count);
  raw_buffer[3] = result.end_state;
  raw_buffer[4] = static_cast<int32_t>(result.char_count);
  writeLineHighlight(result.highlight, raw_buffer + 5, config);

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

/// Create a managed document (Document)
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

/// Destroy the managed document
static napi_value Document_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  deleteNapiCPtrHolder<Document>(env, args[0]);
  return getNapiUndefined(env);
}

/// Get the URI of the managed document
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

/// Total character count of the document
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

/// Get the total character count of a specific line
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

/// Calculate the start character index of a specific line
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

/// Convert a character index to line/column position
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

/// Get the total line count
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

/// Get the text content of a specific line
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

/// Get the full text content
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

/// Get the name of the syntax rule
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

/// Get the exact file names supported by the syntax rule
static napi_value SyntaxRule_GetFileNames(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<SyntaxRule> rule = getNapiCPtrHolderValue<SyntaxRule>(env, args[0]);
  if (rule == nullptr) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  napi_value result;
  napi_create_array_with_length(env, rule->file_names.size(), &result);
  size_t i = 0;
  for (const U8String& file_name : rule->file_names) {
    napi_set_element(env, result, i, createNapiString(env, file_name));
    ++i;
  }
  return result;
}

/// Get the file suffixes supported by the syntax rule
static napi_value SyntaxRule_GetFileSuffixes(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<SyntaxRule> rule = getNapiCPtrHolderValue<SyntaxRule>(env, args[0]);
  if (rule == nullptr) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  napi_value result;
  napi_create_array_with_length(env, rule->file_suffixes.size(), &result);
  size_t i = 0;
  for (const U8String& suffix : rule->file_suffixes) {
    napi_set_element(env, result, i, createNapiString(env, suffix));
    ++i;
  }
  return result;
}

// ================================================ TextAnalyzer ====================================================

/// Convert IndentGuideResult to Int32Array for ArkTS layer
static napi_value ConvertIndentGuideResultAsIntArray(napi_env env, const SharedPtr<IndentGuideResult>& result) {
  if (!result) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  const size_t total_count = computeIndentGuideResultBufferSize(result);
  const size_t total_size = total_count * sizeof(int32_t);
  int32_t* raw_buffer = new(std::nothrow) int32_t[total_count];
  if (!raw_buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory for IndentGuideResult");
    return nullptr;
  }
  writeIndentGuideResult(result, raw_buffer);

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

/// Destroy the plain text highlight analyzer
static napi_value TextAnalyzer_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  deleteNapiCPtrHolder<TextAnalyzer>(env, args[0]);
  return getNapiUndefined(env);
}

/// Analyze a text and return the highlight result
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

/// Analyze a single line of text
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

/// Perform indent guide analysis on plain text
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
  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuides(text);
  return ConvertIndentGuideResultAsIntArray(env, result);
}

// ================================================ DocumentAnalyzer ====================================================

/// Destroy the document highlight analyzer
static napi_value DocumentAnalyzer_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<DocumentAnalyzer>(env, args[0]);
  return getNapiUndefined(env);
}

/// Perform full highlight analysis on the text
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

/// Analyze enough lines to cover the specified visible line range
/// args: [handle, visibleStartLine, visibleLineCount]
static napi_value DocumentAnalyzer_AnalyzeLineRange(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t visible_start_line = 0, visible_line_count = 0;
  napi_get_value_int32(env, args[1], &visible_start_line);
  napi_get_value_int32(env, args[2], &visible_line_count);
  LineRange visible_range = {static_cast<size_t>(visible_start_line), static_cast<size_t>(visible_line_count)};
  SharedPtr<DocumentHighlightSlice> slice = analyzer->analyzeLineRange(visible_range);
  return ConvertDocumentHighlightSliceAsIntArray(env, analyzer->getHighlightConfig(), slice);
}

/// Incrementally re-analyze the text based on patch content (line/column range)
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

/// Incrementally re-analyze the text based on patch content (character index range)
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

/// Incrementally re-analyze and return highlight slice for specified visible line range
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

/// Get highlight slice from the current cached result
/// args: [handle, visibleStartLine, visibleLineCount]
static napi_value DocumentAnalyzer_GetHighlightSlice(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t visible_start_line = 0, visible_line_count = 0;
  napi_get_value_int32(env, args[1], &visible_start_line);
  napi_get_value_int32(env, args[2], &visible_line_count);
  LineRange visible_range = {static_cast<size_t>(visible_start_line), static_cast<size_t>(visible_line_count)};
  SharedPtr<DocumentHighlightSlice> slice = analyzer->getHighlightSlice(visible_range);
  return ConvertDocumentHighlightSliceAsIntArray(env, analyzer->getHighlightConfig(), slice);
}

/// Perform indent guide analysis on the managed document
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

/// Perform indent guide analysis for the requested visible line range
/// args: [handle, visibleStartLine, visibleLineCount]
static napi_value DocumentAnalyzer_AnalyzeIndentGuidesInLineRange(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t visible_start_line = 0, visible_line_count = 0;
  napi_get_value_int32(env, args[1], &visible_start_line);
  napi_get_value_int32(env, args[2], &visible_line_count);
  LineRange visible_range = {static_cast<size_t>(visible_start_line), static_cast<size_t>(visible_line_count)};
  SharedPtr<IndentGuideResult> result = analyzer->analyzeIndentGuidesInLineRange(visible_range);
  return ConvertIndentGuideResultAsIntArray(env, result);
}

/// Get the managed document
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

/// Create a SweetLine highlight engine
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

/// Destroy the highlight engine
static napi_value HighlightEngine_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<HighlightEngine>(env, args[0]);
  return getNapiUndefined(env);
}

/// Register a highlight style for name mapping
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

/// Get the registered style name by style ID
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

/// Define a macro for controlling #ifdef conditional compilation in importSyntax
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

/// Undefine a macro
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

/// Compile syntax rule from JSON
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
  } catch (const SyntaxCompileError& error) {
    ThrowSyntaxCompileError(env, error);
    return createNapiInt64(env, 0);
  }
}

/// Compile syntax rule (from JSON config file)
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
  } catch (const SyntaxCompileError& error) {
    ThrowSyntaxCompileError(env, error);
    return createNapiInt64(env, 0);
  }
}

/// Create a text highlight analyzer by syntax rule name (no incremental analysis)
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
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerBySyntaxName(syntax_name);
  int64_t handle = asCHandle<int64_t>(analyzer);
  return createNapiInt64(env, handle);
}

/// Create a text highlight analyzer by file name (no incremental analysis)
static napi_value HighlightEngine_CreateAnalyzerByFileName(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return createNapiInt64(env, 0);
  }
  U8String file_name;
  if (!getStdStringFromNapiValue(env, args[1], file_name)) {
    return createNapiInt64(env, 0);
  }
  SharedPtr<TextAnalyzer> analyzer = engine->createAnalyzerByFileName(file_name);
  int64_t handle = asCHandle<int64_t>(analyzer);
  return createNapiInt64(env, handle);
}

/// Load a managed document and get a document highlight analyzer
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

/// Remove a managed document
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
    {"SyntaxRule_GetFileNames", nullptr, SyntaxRule_GetFileNames, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SyntaxRule_GetFileSuffixes", nullptr, SyntaxRule_GetFileSuffixes, nullptr, nullptr, nullptr, napi_default, nullptr},
    // TextAnalyzer
    {"TextAnalyzer_Delete", nullptr, TextAnalyzer_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeText", nullptr, TextAnalyzer_AnalyzeText, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeLine", nullptr, TextAnalyzer_AnalyzeLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"TextAnalyzer_AnalyzeIndentGuides", nullptr, TextAnalyzer_AnalyzeIndentGuides, nullptr, nullptr, nullptr, napi_default, nullptr},
    // DocumentAnalyzer
    {"DocumentAnalyzer_Delete", nullptr, DocumentAnalyzer_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_Analyze", nullptr, DocumentAnalyzer_Analyze, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeLineRange", nullptr, DocumentAnalyzer_AnalyzeLineRange, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges", nullptr, DocumentAnalyzer_AnalyzeChanges, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges2", nullptr, DocumentAnalyzer_AnalyzeChanges2, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChangesInLineRange", nullptr, DocumentAnalyzer_AnalyzeChangesInLineRange, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_GetHighlightSlice", nullptr, DocumentAnalyzer_GetHighlightSlice, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeIndentGuides", nullptr, DocumentAnalyzer_AnalyzeIndentGuides, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeIndentGuidesInLineRange", nullptr, DocumentAnalyzer_AnalyzeIndentGuidesInLineRange, nullptr, nullptr, nullptr, napi_default, nullptr},
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
    {"HighlightEngine_CreateAnalyzerByFileName", nullptr, HighlightEngine_CreateAnalyzerByFileName, nullptr, nullptr, nullptr, napi_default, nullptr},
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
