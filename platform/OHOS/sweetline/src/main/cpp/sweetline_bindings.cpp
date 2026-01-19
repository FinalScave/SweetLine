#include "napi/native_api.h"
#include "napi_util.h"
#include "highlight.h"
#include <cstdint>

using namespace NS_SWEETLINE;

static napi_value ConvertDocumentHighlightAsIntArray(napi_env env, const SharedPtr<DocumentHighlight>& highlight) {
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
  
  const size_t buffer_count = span_count * 7;
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  napi_value array_buffer;
  UniquePtr<int32_t[]> buffer = makeUniquePtr<int32_t[]>(buffer_count);
  if (!buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }

  size_t index = 0;
  for (const LineHighlight &line : highlight->lines) {
    for (const TokenSpan &span : line.spans) {
      buffer[index++] = static_cast<int32_t>(span.range.start.line);
      buffer[index++] = static_cast<int32_t>(span.range.start.column);
      buffer[index++] = static_cast<int32_t>(span.range.start.index);
      buffer[index++] = static_cast<int32_t>(span.range.end.line);
      buffer[index++] = static_cast<int32_t>(span.range.end.column);
      buffer[index++] = static_cast<int32_t>(span.range.end.index);
      buffer[index++] = static_cast<int32_t>(span.style);
    }
  }

  napi_status status = napi_create_external_arraybuffer(
      env,
      buffer.get(),
      buffer_size,
      [](napi_env env, void* data, void* hint) {
      },
      nullptr,
      &array_buffer
  );
  
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer");
    return nullptr;
  }

  buffer.release();

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, buffer_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray");
    return nullptr;
  }
  return typed_array;
}

static napi_value ConvertLineHighlightAsIntArray(napi_env env, const LineHighlight& line_highlight) {
  size_t span_count = line_highlight.spans.size();
  if (span_count == 0) {
    napi_value empty_array;
    napi_create_array_with_length(env, 0, &empty_array);
    return empty_array;
  }
  
  const size_t buffer_count = span_count * 7;
  const size_t buffer_size = buffer_count * sizeof(int32_t);

  napi_value array_buffer;
  UniquePtr<int32_t[]> buffer = makeUniquePtr<int32_t[]>(buffer_count);
  if (!buffer) {
    napi_throw_error(env, nullptr, "Failed to allocate buffer memory");
    return nullptr;
  }

  size_t index = 0;
  for (const TokenSpan &span : line_highlight.spans) {
    buffer[index++] = static_cast<int32_t>(span.range.start.line);
    buffer[index++] = static_cast<int32_t>(span.range.start.column);
    buffer[index++] = static_cast<int32_t>(span.range.start.index);
    buffer[index++] = static_cast<int32_t>(span.range.end.line);
    buffer[index++] = static_cast<int32_t>(span.range.end.column);
    buffer[index++] = static_cast<int32_t>(span.range.end.index);
    buffer[index++] = static_cast<int32_t>(span.style);
  }

  napi_status status = napi_create_external_arraybuffer(
      env,
      buffer.get(),
      buffer_size,
      [](napi_env env, void* data, void* hint) {
      },
      nullptr,
      &array_buffer
  );
  
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create external ArrayBuffer");
    return nullptr;
  }

  buffer.release();

  napi_value typed_array;
  status = napi_create_typedarray(env, napi_int32_array, buffer_count, array_buffer, 0, &typed_array);
  if (status != napi_ok) {
    napi_throw_error(env, nullptr, "Failed to create TypedArray");
    return nullptr;
  }
  return typed_array;
}

// ================================================ Document ====================================================
static napi_value Document_Create(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  String uri;
  bool status = getStdStringFromNapiValue(env, args[0], uri);
  if (!status) {
    return createNapiInt32(env, 0);
  }
  String text;
  status = getStdStringFromNapiValue(env, args[1], text);
  if (!status) {
    return createNapiInt32(env, 0);
  }
  intptr_t handle = makeCPtrHolderToIntPtr<Document>(uri, text);
  return createNapiInt64(env, handle);
}

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

static napi_value Document_GetLineCount(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return createNapiInt32(env, 0);
  }
  napi_value line_count;
  napi_create_int32(env, (int32_t)document->getLineCount(), &line_count);
  return line_count;
}

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
  const String& line_text = document->getLine(line).text;
  return createNapiString(env, line_text);
}

static napi_value Document_GetText(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[0]);
  if (document == nullptr) {
    return getNapiUndefined(env);
  }
  String text = document->getText();
  return createNapiString(env, text);
}

// ================================================ SyntaxRule ====================================================
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

static napi_value SyntaxRule_GetFileExtensions(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

  SharedPtr<SyntaxRule> rule = getNapiCPtrHolderValue<SyntaxRule>(env, args[0]);
  if (rule == nullptr) {
    return getNapiUndefined(env);
  }
  napi_value result;
  napi_create_array_with_length(env, rule->file_extensions_.size(), &result);
  size_t i = 0;
  for (const String& ext : rule->file_extensions_) {
    napi_set_element(env, result, i, createNapiString(env, ext));
    ++i;
  }
  return result;
}

// ================================================ DocumentAnalyzer ====================================================
static napi_value DocumentAnalyzer_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<DocumentAnalyzer>(env, args[0]);
  return getNapiUndefined(env);
}

static napi_value DocumentAnalyzer_Analyze(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  return ConvertDocumentHighlightAsIntArray(env, highlight);
}

static napi_value DocumentAnalyzer_AnalyzeChanges(napi_env env, napi_callback_info info) {
  size_t argc = 6;
  napi_value args[6] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  String new_text;
  if (!getStdStringFromNapiValue(env, args[5], new_text)) {
    return getNapiUndefined(env);
  }
  int32_t start_line = 0;
  napi_get_value_int32(env, args[1], &start_line);
  int32_t start_column = 0;
  napi_get_value_int32(env, args[2], &start_column);
  int32_t end_line = 0;
  napi_get_value_int32(env, args[3], &end_line);
  int32_t end_column = 0;
  napi_get_value_int32(env, args[4], &end_column);
  TextRange range = {
    {static_cast<size_t>(start_line), static_cast<size_t>(start_column)},
    {static_cast<size_t>(end_line), static_cast<size_t>(end_column)}
  };
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeChanges(range, new_text);
  return ConvertDocumentHighlightAsIntArray(env, highlight);
}

static napi_value DocumentAnalyzer_AnalyzeChanges2(napi_env env, napi_callback_info info) {
  size_t argc = 4;
  napi_value args[4] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  String new_text;
  if (!getStdStringFromNapiValue(env, args[3], new_text)) {
    return getNapiUndefined(env);
  }
  int32_t start_index = 0;
  napi_get_value_int32(env, args[1], &start_index);
  int32_t end_index = 0;
  napi_get_value_int32(env, args[2], &end_index);
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeChanges(start_index, end_index, new_text);
  return ConvertDocumentHighlightAsIntArray(env, highlight);
}

static napi_value DocumentAnalyzer_AnalyzeLine(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  int32_t line = 0;
  napi_get_value_int32(env, args[1], &line);
  LineHighlight line_highlight;
  analyzer->analyzeLine(line, line_highlight);
  return ConvertLineHighlightAsIntArray(env, line_highlight);
}

static napi_value DocumentAnalyzer_GetDocument(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<DocumentAnalyzer> analyzer = getNapiCPtrHolderValue<DocumentAnalyzer>(env, args[0]);
  if (analyzer == nullptr) {
    return getNapiUndefined(env);
  }
  intptr_t handle = toIntPtr(analyzer->getDocument());
  return createNapiInt64(env, handle);
}

// ================================================ HighlightEngine ====================================================
static napi_value HighlightEngine_Create(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  bool show_index;
  napi_get_value_bool(env, args[0], &show_index);
  HighlightConfig config = {show_index};
  intptr_t handle =  makeCPtrHolderToIntPtr<HighlightEngine>(config);
  return createNapiInt64(env, handle);
}

static napi_value HighlightEngine_Delete(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  deleteNapiCPtrHolder<HighlightEngine>(env, args[0]);
  return getNapiUndefined(env);
}

static napi_value HighlightEngine_RegisterStyleName(napi_env env, napi_callback_info info) {
  size_t argc = 3;
  napi_value args[3] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  String name;
  if (!getStdStringFromNapiValue(env, args[1], name)) {
    return getNapiBoolean(env, false);
  }
  int32_t style_id = 0;
  napi_get_value_int32(env, args[2], &style_id);
  engine->registerStyleName(name, style_id);
  return getNapiBoolean(env, true);
}

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
  const String& name = engine->getStyleName(style_id);
  return createNapiString(env, name);
}

static napi_value HighlightEngine_CompileSyntaxFromJson(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiUndefined(env);
  }
  String json;
  if (!getStdStringFromNapiValue(env, args[1], json)) {
    return getNapiUndefined(env);
  }
  SharedPtr<SyntaxRule> rule = engine->compileSyntaxFromJson(json);
  intptr_t handle = toIntPtr(rule);
  return createNapiInt64(env, handle);
}

static napi_value HighlightEngine_CompileSyntaxFromFile(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiUndefined(env);
  }
  String path;
  if (!getStdStringFromNapiValue(env, args[1], path)) {
    return getNapiUndefined(env);
  }
  SharedPtr<SyntaxRule> rule = engine->compileSyntaxFromFile(path);
  intptr_t handle = toIntPtr(rule);
  return createNapiInt64(env, handle);
}

static napi_value HighlightEngine_LoadDocument(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiUndefined(env);
  }
  SharedPtr<Document> document = getNapiCPtrHolderValue<Document>(env, args[1]);
  if (document == nullptr) {
    return getNapiUndefined(env);
  }
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  intptr_t handle = toIntPtr(analyzer);
  return createNapiInt64(env, handle);
}

static napi_value HighlightEngine_RemoveDocument(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  
  SharedPtr<HighlightEngine> engine = getNapiCPtrHolderValue<HighlightEngine>(env, args[0]);
  if (engine == nullptr) {
    return getNapiBoolean(env, false);
  }
  String uri;
  if (!getStdStringFromNapiValue(env, args[1], uri)) {
    return getNapiBoolean(env, false);
  }
  engine->removeDocument(uri);
  return getNapiBoolean(env, true);
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
    {"Document_Create", nullptr, Document_Create, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetUri", nullptr, Document_GetUri, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_TotalChars", nullptr, Document_TotalChars, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLineCharCount", nullptr, Document_GetLineCharCount, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_CharIndexOfLine", nullptr, Document_CharIndexOfLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_CharIndexToPosition", nullptr, Document_CharIndexToPosition, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLineCount", nullptr, Document_GetLineCount, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetLine", nullptr, Document_GetLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"Document_GetText", nullptr, Document_GetText, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SyntaxRule_GetName", nullptr, SyntaxRule_GetName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"SyntaxRule_GetFileExtensions", nullptr, SyntaxRule_GetFileExtensions, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_Delete", nullptr, DocumentAnalyzer_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_Analyze", nullptr, DocumentAnalyzer_Analyze, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges", nullptr, DocumentAnalyzer_AnalyzeChanges, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeChanges2", nullptr, DocumentAnalyzer_AnalyzeChanges2, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_AnalyzeLine", nullptr, DocumentAnalyzer_AnalyzeLine, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"DocumentAnalyzer_GetDocument", nullptr, DocumentAnalyzer_GetDocument, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_Create", nullptr, HighlightEngine_Create, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_Delete", nullptr, HighlightEngine_Delete, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_RegisterStyleName", nullptr, HighlightEngine_RegisterStyleName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_GetStyleName", nullptr, HighlightEngine_GetStyleName, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CompileSyntaxFromJson", nullptr, HighlightEngine_CompileSyntaxFromJson, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"HighlightEngine_CompileSyntaxFromFile", nullptr, HighlightEngine_CompileSyntaxFromFile, nullptr, nullptr, nullptr, napi_default, nullptr},
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
