#include "c_wrapper.hpp"

StringKeepAlive& StringKeepAlive::getInstance() {
  static thread_local StringKeepAlive string_pool;
  return string_pool;
}

void StringKeepAlive::clear() {
  vector_.clear();
}

extern "C" {

intptr_t sl_create_document(const char* uri, const char* text) {
  return makeCPtrHolderToIntPtr<Document>(uri, text);
}

sl_error_t sl_free_document(intptr_t document_handle) {
  deleteCPtrHolder<Document>(document_handle);
  return SL_OK;
}

intptr_t sl_create_engine(bool show_index) {
  HighlightConfig config = {show_index};
  return makeCPtrHolderToIntPtr<HighlightEngine>(config);
}

sl_error_t sl_free_engine(intptr_t engine_handle) {
  deleteCPtrHolder<HighlightEngine>(engine_handle);
  return SL_OK;
}

sl_syntax_error_t sl_engine_compile_json(intptr_t engine_handle, const char* syntax_json) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(engine_handle);
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

sl_syntax_error_t sl_engine_compile_file(intptr_t engine_handle, const char* syntax_file) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(engine_handle);
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

sl_error_t sl_engine_register_style_name(intptr_t engine_handle, const char* style_name, int32_t style_id) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return SL_HANDLE_INVALID;
  }
  engine->registerStyleName(style_name, style_id);
  return SL_OK;
}

const char* sl_engine_get_style_name(intptr_t engine_handle, int32_t style_id) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return nullptr;
  }
  return engine->getStyleName(style_id).c_str();
}

intptr_t sl_engine_load_document(intptr_t engine_handle, intptr_t document_handle) {
  SharedPtr<HighlightEngine> engine = getCPtrHolderValue<HighlightEngine>(engine_handle);
  if (engine == nullptr) {
    return SL_HANDLE_INVALID;
  }
  SharedPtr<Document> document = getCPtrHolderValue<Document>(document_handle);
  if (document == nullptr) {
    return SL_HANDLE_INVALID;
  }
  SharedPtr<DocumentAnalyzer> analyzer = engine->loadDocument(document);
  return toIntPtr(analyzer);
}

int32_t* sl_document_analyze(intptr_t analyzer_handle, int32_t* data_size) {
  SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(analyzer_handle);
  if (analyzer == nullptr) {
    *data_size = 0;
    return nullptr;
  }
  SharedPtr<DocumentHighlight> highlight = analyzer->analyze();
  *data_size = highlight->spanCount();
  int32_t* result = static_cast<int32_t*>(malloc(sizeof(int32_t) * *data_size));
  writeDocumentHighlight(highlight, result, analyzer->getHighlightConfig());
  return result;
}

int32_t* sl_document_analyze_changes(intptr_t analyzer_handle, size_t* changes_range, const char* new_text, int32_t* data_size) {
  SharedPtr<DocumentAnalyzer> analyzer = getCPtrHolderValue<DocumentAnalyzer>(analyzer_handle);
  if (analyzer == nullptr) {
    *data_size = 0;
    return nullptr;
  }
  TextPosition start = {changes_range[0], changes_range[1]};
  TextPosition end = {changes_range[2], changes_range[3]};
  SharedPtr<DocumentHighlight> highlight = analyzer->analyzeChanges({start, end}, new_text);
  *data_size = highlight->spanCount();
  int32_t* result = static_cast<int32_t*>(malloc(sizeof(int32_t) * *data_size));
  writeDocumentHighlight(highlight, result, analyzer->getHighlightConfig());
  return result;
}

}