#include <catch2/catch_amalgamated.hpp>
#include "sweetline/c_sweetline.h"

namespace {
  constexpr const char* kDocumentSyntax = R"({
    "name": "cApiDocument",
    "fileSuffix": ".remove",
    "states": {
      "default": [
        { "pattern": "\\bfirst\\b", "style": "keyword" }
      ]
    }
  })";
}

TEST_CASE("C API returns null handles when analyzer creation fails") {
  sl_engine_handle_t engine = sl_create_engine(false, false, 4);
  REQUIRE(engine != nullptr);

  CHECK(sl_engine_create_text_analyzer(engine, "missing") == nullptr);
  CHECK(sl_engine_create_text_analyzer_by_file_name(engine, "missing.unknown") == nullptr);

  sl_document_handle_t document = sl_create_document("missing.unknown", "text");
  REQUIRE(document != nullptr);
  CHECK(sl_engine_load_document(engine, document) == nullptr);

  CHECK(sl_free_document(document) == SL_OK);
  CHECK(sl_free_engine(engine) == SL_OK);
}

TEST_CASE("C API frees text and document analyzer handles") {
  sl_engine_handle_t engine = sl_create_engine(false, false, 4);
  REQUIRE(engine != nullptr);
  REQUIRE(sl_engine_compile_json(engine, kDocumentSyntax).err_code == SL_OK);

  sl_analyzer_handle_t text_analyzer = sl_engine_create_text_analyzer(engine, "cApiDocument");
  REQUIRE(text_analyzer != nullptr);

  sl_document_handle_t document = sl_create_document("sample.remove", "first");
  REQUIRE(document != nullptr);
  sl_analyzer_handle_t document_analyzer = sl_engine_load_document(engine, document);
  REQUIRE(document_analyzer != nullptr);

  CHECK(sl_free_text_analyzer(text_analyzer) == SL_OK);
  CHECK(sl_free_document_analyzer(document_analyzer) == SL_OK);
  CHECK(sl_free_text_analyzer(nullptr) == SL_OK);
  CHECK(sl_free_document_analyzer(nullptr) == SL_OK);
  CHECK(sl_free_document(document) == SL_OK);
  CHECK(sl_free_engine(engine) == SL_OK);
}

TEST_CASE("C API removes loaded documents by URI") {
  sl_engine_handle_t engine = sl_create_engine(false, false, 4);
  REQUIRE(engine != nullptr);
  REQUIRE(sl_engine_compile_json(engine, kDocumentSyntax).err_code == SL_OK);

  sl_document_handle_t first_document = sl_create_document("same.remove", "first");
  sl_document_handle_t second_document = sl_create_document("same.remove", "second");
  REQUIRE(first_document != nullptr);
  REQUIRE(second_document != nullptr);

  sl_analyzer_handle_t first_analyzer = sl_engine_load_document(engine, first_document);
  sl_analyzer_handle_t cached_analyzer = sl_engine_load_document(engine, second_document);
  REQUIRE(first_analyzer != nullptr);
  REQUIRE(cached_analyzer != nullptr);

  int32_t* cached_result = sl_document_analyze(cached_analyzer);
  REQUIRE(cached_result != nullptr);
  REQUIRE(cached_result[2] == 1);
  CHECK(cached_result[3] == 1);
  sl_free_buffer(cached_result);

  CHECK(sl_engine_remove_document(engine, "same.remove") == SL_OK);
  sl_analyzer_handle_t replacement_analyzer = sl_engine_load_document(engine, second_document);
  REQUIRE(replacement_analyzer != nullptr);

  int32_t* replacement_result = sl_document_analyze(replacement_analyzer);
  REQUIRE(replacement_result != nullptr);
  REQUIRE(replacement_result[2] == 1);
  CHECK(replacement_result[3] == 0);
  sl_free_buffer(replacement_result);

  CHECK(sl_free_document_analyzer(first_analyzer) == SL_OK);
  CHECK(sl_free_document_analyzer(cached_analyzer) == SL_OK);
  CHECK(sl_free_document_analyzer(replacement_analyzer) == SL_OK);
  CHECK(sl_free_document(first_document) == SL_OK);
  CHECK(sl_free_document(second_document) == SL_OK);
  CHECK(sl_free_engine(engine) == SL_OK);
}

TEST_CASE("C API passes tab size to indent guide analysis") {
  sl_engine_handle_t engine = sl_create_engine(false, false, 8);
  REQUIRE(engine != nullptr);
  REQUIRE(sl_engine_compile_json(engine, kDocumentSyntax).err_code == SL_OK);

  sl_analyzer_handle_t analyzer = sl_engine_create_text_analyzer(engine, "cApiDocument");
  REQUIRE(analyzer != nullptr);

  int32_t* result = sl_text_analyze_indent_guides(analyzer, "\t    first");
  REQUIRE(result != nullptr);
  REQUIRE(result[1] == 1);
  size_t line_state_offset = 3;
  for (int32_t guide = 0; guide < result[2]; ++guide) {
    int32_t branch_count = result[line_state_offset + 4];
    line_state_offset += 5 + static_cast<size_t>(branch_count) * 2;
  }
  CHECK(result[line_state_offset + 3] == 1);

  sl_free_buffer(result);
  CHECK(sl_free_text_analyzer(analyzer) == SL_OK);
  CHECK(sl_free_engine(engine) == SL_OK);
}
