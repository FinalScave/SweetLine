#include "c_wrapper.hpp"

StringKeepAlive& StringKeepAlive::getInstance() {
  static thread_local StringKeepAlive string_pool;
  return string_pool;
}

void StringKeepAlive::clear() {
  vector_.clear();
}

extern "C" {

sl_document_handle_t sl_create_document(const char* uri, const char* text) {
  return makeCPtrHolderToHandle<sl_document_handle_t, Document>(uri, text);
}

sl_error_t sl_free_document(sl_document_handle_t document_handle) {
  deleteCPtrHolder<sl_document_handle_t, Document>(document_handle);
  return SL_OK;
}

sl_engine_handle_t sl_create_engine(bool show_index, bool inline_style) {
  HighlightConfig config = {show_index, inline_style};
  return makeCPtrHolderToHandle<sl_engine_handle_t, HighlightEngine>(config);
}

sl_error_t sl_free_engine(sl_engine_handle_t engine_handle) {
  deleteCPtrHolder<sl_engine_handle_t, HighlightEngine>(engine_handle);
  return SL_OK;
}

sl_error_t sl_engine_define_macro(sl_engine_handle_t engine_handle, const char* macro_name) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return SL_HANDLE_INVALID;
  }
  engine->defineMacro(macro_name);
  return SL_OK;
}

sl_error_t sl_engine_undefine_macro(sl_engine_handle_t engine_handle, const char* macro_name) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return SL_HANDLE_INVALID;
  }
  engine->undefineMacro(macro_name);
  return SL_OK;
}

sl_syntax_error_t sl_engine_compile_json(sl_engine_handle_t engine_handle, const char* syntax_json) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return {SL_HANDLE_INVALID};
  }
  try {
    engine->compileSyntaxFromJson(syntax_json);
    return {SL_OK};
  } catch (SyntaxRuleParseError& err) {
    StringKeepAlive::getInstance().clear();
    return {static_cast<sl_error_t>(err.code()), StringKeepAlive::getInstance().getAliveCString(err.message())};
  }
}

sl_syntax_error_t sl_engine_compile_file(sl_engine_handle_t engine_handle, const char* syntax_file) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return {SL_HANDLE_INVALID};
  }
  try {
    engine->compileSyntaxFromFile(syntax_file);
    return {SL_OK};
  } catch (SyntaxRuleParseError& err) {
    StringKeepAlive::getInstance().clear();
    return {static_cast<sl_error_t>(err.code()), StringKeepAlive::getInstance().getAliveCString(err.message())};
  }
}

sl_error_t sl_engine_register_style_name(sl_engine_handle_t engine_handle, const char* style_name, int32_t style_id) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return SL_HANDLE_INVALID;
  }
  engine->registerStyleName(style_name, style_id);
  return SL_OK;
}

const char* sl_engine_get_style_name(sl_engine_handle_t engine_handle, int32_t style_id) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return nullptr;
  }
  return engine->getStyleName(style_id).c_str();
}

sl_analyzer_handle_t sl_engine_create_text_analyzer(sl_engine_handle_t engine_handle, const char* syntax_name) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return nullptr;
  }
  return asCHandle<sl_analyzer_handle_t>(engine->createAnalyzerByName(syntax_name));
}

sl_analyzer_handle_t sl_engine_create_text_analyzer2(sl_engine_handle_t engine_handle, const char* extension) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return nullptr;
  }
  return asCHandle<sl_analyzer_handle_t>(engine->createAnalyzerByExtension(extension));
}

int32_t* sl_text_analyze(sl_analyzer_handle_t analyzer_handle, const char* text) {
  SharedPtr<TextAnalyzer> analyzer = getCPtrHolderValue<sl_analyzer_handle_t, TextAnalyzer>(analyzer_handle);
  if (analyzer == nullptr) {
    return nullptr;
  }
  const HighlightConfig& config = analyzer->getHighlightConfig();
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeText(text);
  int32_t span_count = static_cast<int32_t>(highlight->spanCount());
  int32_t stride = computeSpanBufferStride(config);
  int32_t* buffer = new int32_t[2 + span_count * stride];
  buffer[0] = span_count;
  buffer[1] = stride;
  writeDocumentHighlight(highlight, buffer + 2, config);
  return buffer;
}

int32_t* sl_text_analyze_line(sl_analyzer_handle_t analyzer_handle, const char* text, int32_t* line_info) {
  SharedPtr<TextAnalyzer> analyzer = getCPtrHolderValue<sl_analyzer_handle_t, TextAnalyzer>(analyzer_handle);
  if (analyzer == nullptr || line_info == nullptr) {
    return nullptr;
  }
  const HighlightConfig& config = analyzer->getHighlightConfig();
  TextLineInfo info_struct = {static_cast<size_t>(line_info[0]), line_info[1], static_cast<size_t>(line_info[2])};
  LineAnalyzeResult result;
  analyzer->analyzeLine(text, info_struct, result);
  int32_t span_count = static_cast<int32_t>(result.highlight.spans.size());
  int32_t stride = computeSpanBufferStride(config);
  int32_t* buffer = new int32_t[4 + span_count * stride];
  buffer[0] = span_count;
  buffer[1] = stride;
  buffer[2] = result.end_state;
  buffer[3] = static_cast<int32_t>(result.char_count);
  writeLineHighlight(result.highlight, buffer + 4, config);
  return buffer;
}

sl_analyzer_handle_t sl_engine_load_document(sl_engine_handle_t engine_handle, sl_document_handle_t document_handle) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<sl_engine_handle_t, HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return nullptr;
  }
  SharedPtr<Document> document = getCPtrHolderValue<sl_document_handle_t, Document>(document_handle);
  if (document == nullptr) {
    return nullptr;
  }
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  return asCHandle<sl_analyzer_handle_t>(analyzer);
}

int32_t* sl_document_analyze(sl_analyzer_handle_t analyzer_handle) {
  SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<sl_analyzer_handle_t, DocumentAnalyzer>(analyzer_handle);
  if (analyzer == nullptr) {
    return nullptr;
  }
  const HighlightConfig& config = analyzer->getHighlightConfig();
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  int32_t span_count = static_cast<int32_t>(highlight->spanCount());
  int32_t stride = computeSpanBufferStride(config);
  int32_t* buffer = new int32_t[2 + span_count * stride];
  buffer[0] = span_count;
  buffer[1] = stride;
  writeDocumentHighlight(highlight, buffer + 2, config);
  return buffer;
}

int32_t* sl_document_analyze_incremental(sl_analyzer_handle_t analyzer_handle, int32_t* changes_range, const char* new_text) {
  SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<sl_analyzer_handle_t, DocumentAnalyzer>(analyzer_handle);
  if (analyzer == nullptr || changes_range == nullptr) {
    return nullptr;
  }
  const HighlightConfig& config = analyzer->getHighlightConfig();
  TextPosition start = {static_cast<size_t>(changes_range[0]), static_cast<size_t>(changes_range[1])};
  TextPosition end = {static_cast<size_t>(changes_range[2]), static_cast<size_t>(changes_range[3])};
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeIncremental({start, end}, new_text);
  int32_t span_count = static_cast<int32_t>(highlight->spanCount());
  int32_t stride = computeSpanBufferStride(config);
  int32_t* buffer = new int32_t[2 + span_count * stride];
  buffer[0] = span_count;
  buffer[1] = stride;
  writeDocumentHighlight(highlight, buffer + 2, config);
  return buffer;
}

void sl_free_buffer(int32_t* result) {
  delete[] result;
}

}