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
        ${SWEETLINE_PUBLIC_INCLUDE_DIR}
        ${SWEETLINE_SRC_DIR}
)

target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE
        TESTS_DIR="${PROJECT_SOURCE_DIR}/tests"
        SYNTAX_DIR="${PROJECT_SOURCE_DIR}/syntaxes"
        SWEETLINE_DEBUG=1
        SWEETLINE_EXPORT=1
        ONIG_STATIC=ON
)
if (MSVC)
    target_compile_options(${TEST_PRODUCT_NAME} PRIVATE /Zc:__cplusplus /utf-8)
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX=1)
endif ()
if (ANDROID)
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE ANDROID)
elseif (OHOS)
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE OHOS)
elseif (WIN32)
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE WINDOWS _POSIX _POSIX_THREAD_SAFE_FUNCTIONS)
elseif (APPLE)
    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE IOS)
    else ()
        target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE OSX)
    endif ()
elseif (EMSCRIPTEN)
    target_compile_options(${TEST_PRODUCT_NAME} PRIVATE "-fexceptions")
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE WASM)
elseif (UNIX AND NOT APPLE)
    target_compile_definitions(${TEST_PRODUCT_NAME} PRIVATE LINUX)
endif ()

target_link_libraries(${TEST_PRODUCT_NAME} PRIVATE
        SweetLine3p::Catch2
        SweetLine3p::NlohmannJson
        SweetLine3p::UtfCpp
        ${SWEETLINE_LINK_LIB}
)

add_test(NAME UnitTests COMMAND ${TEST_PRODUCT_NAME})
