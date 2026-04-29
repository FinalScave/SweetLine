set(TEST_PRODUCT_NAME unit_tests)
add_definitions(-DCATCH_AMALGAMATED_CUSTOM_MAIN=ON)
add_executable(${TEST_PRODUCT_NAME}
        ${3DPARTY_DIR}/include/catch2/catch_amalgamated.cpp
        tests_main.cpp
        foundation_crlf_test.cpp
        patch_text.cpp
        highlight_test.cpp
        indent_test.cpp
        syntax_test.cpp
)
target_sources(${TEST_PRODUCT_NAME} PRIVATE $<TARGET_OBJECTS:${CORE_OBJECT_TARGET}>)

target_include_directories(${TEST_PRODUCT_NAME} PRIVATE
        ${3DPARTY_DIR}/include
        ${SRC_DIR}/include
)

target_link_libraries(${TEST_PRODUCT_NAME} PRIVATE
        ${LINK_LIB}
)

add_test(NAME UnitTests COMMAND ${TEST_PRODUCT_NAME})
