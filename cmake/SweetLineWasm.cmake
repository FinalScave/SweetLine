file(GLOB_RECURSE SWEETLINE_WASM_EMBIND_FILES
        ${SWEETLINE_PROJECT_DIR}/platform/Emscripten/*.cpp
)

message(STATUS "add target: ${SWEETLINE_SHARED_LIB_NAME} for wasm embind")
add_executable(${SWEETLINE_SHARED_LIB_NAME}
        ${SWEETLINE_WASM_EMBIND_FILES}
        $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
)

target_compile_features(${SWEETLINE_SHARED_LIB_NAME} PRIVATE cxx_std_17)
target_include_directories(${SWEETLINE_SHARED_LIB_NAME} PRIVATE
        ${SWEETLINE_INCLUDE_DIRS}
        ${SWEETLINE_THIRD_PARTY_INCLUDE_DIRS}
)
target_compile_options(${SWEETLINE_SHARED_LIB_NAME} PRIVATE "-fexceptions")
target_compile_definitions(${SWEETLINE_SHARED_LIB_NAME} PRIVATE
        TESTS_DIR="${PROJECT_SOURCE_DIR}/tests"
        SYNTAX_DIR="${PROJECT_SOURCE_DIR}/syntaxes"
        SWEETLINE_DEBUG=1
        ONIG_STATIC=ON
        WASM
)

target_link_libraries(${SWEETLINE_SHARED_LIB_NAME} PRIVATE
        SweetLine3p::NlohmannJson
        SweetLine3p::UtfCpp
        ${SWEETLINE_LINK_LIB}
        "--bind"
        "-fexceptions"
        "-lNODEFS.js"
        "-s WASM=1"
        "-s MODULARIZE=1"
        "-s NO_DISABLE_EXCEPTION_CATCHING=1"
        "-s EXPORT_NAME='createSweetLine'"
        "-s EXPORT_ES6=1"
        "-s FORCE_FILESYSTEM=1"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s EXPORTED_RUNTIME_METHODS=[\"FS\"]"
)
