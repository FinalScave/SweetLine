set(TEST_PRODUCT_NAME unit_tests)

file(GLOB TEST_LANGUAGE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/languages/*.cpp"
)

add_executable(${TEST_PRODUCT_NAME}
        tests_main.cpp
        foundation_crlf_test.cpp
        patch_text.cpp
        highlight_test.cpp
        indent_test.cpp
        bracket_pair_test.cpp
        syntax_test.cpp
        ${TEST_LANGUAGE_SOURCES}
)

target_sources(${TEST_PRODUCT_NAME} PRIVATE
        $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
        ${SWEETLINE_C_API_OBJECTS}
)

target_include_directories(${TEST_PRODUCT_NAME} PRIVATE
        ${SWEETLINE_INCLUDE_DIRS}
)

target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE
        TESTS_DIR="${PROJECT_SOURCE_DIR}/tests"
        SYNTAX_DIR="${PROJECT_SOURCE_DIR}/syntaxes"
        SWEETLINE_DEBUG=1
        SWEETLINE_EXPORT=1
        ONIG_STATIC=ON
)
sweetline_apply_platform_target(${TEST_PRODUCT_NAME})

target_link_libraries(${TEST_PRODUCT_NAME} PRIVATE
        SweetLine3p::Catch2
)
sweetline_link_common_libraries(${TEST_PRODUCT_NAME})

add_test(NAME UnitTests COMMAND ${TEST_PRODUCT_NAME})
