function(sweetline_platform_configure)
    set(SWEETLINE_PLATFORM_ID "emscripten" PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS WASM PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_OPTIONS "-fexceptions" PARENT_SCOPE)
endfunction()

function(sweetline_platform_validate_build_options)
    if (SWEETLINE_BUILD_APPLE_FRAMEWORK)
        message(FATAL_ERROR "SWEETLINE_BUILD_APPLE_FRAMEWORK is only supported on Apple platforms")
    endif ()
    if (NOT SWEETLINE_BUILD_STATIC AND NOT SWEETLINE_BUILD_TESTS AND NOT SWEETLINE_BUILD_WASM_EMBIND)
        message(FATAL_ERROR "At least one of SWEETLINE_BUILD_STATIC, SWEETLINE_BUILD_TESTS, or SWEETLINE_BUILD_WASM_EMBIND must be enabled for Emscripten")
    endif ()
endfunction()

function(sweetline_platform_supports_tests out_var)
    set(${out_var} OFF PARENT_SCOPE)
endfunction()

function(sweetline_platform_should_build_c_api_objects out_var)
    set(${out_var} OFF PARENT_SCOPE)
endfunction()

function(sweetline_platform_add_shared_target out_handled)
    if (SWEETLINE_BUILD_SHARED)
        message(STATUS "skip target: ${SWEETLINE_SHARED_LIB_NAME} dynamic lib is not used for Emscripten")
        set(${out_handled} ON PARENT_SCOPE)
    else ()
        set(${out_handled} OFF PARENT_SCOPE)
    endif ()
endfunction()

function(sweetline_platform_add_extra_targets)
    if (NOT SWEETLINE_BUILD_WASM_EMBIND)
        return()
    endif ()

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

    sweetline_link_common_libraries(${SWEETLINE_SHARED_LIB_NAME})
    target_link_libraries(${SWEETLINE_SHARED_LIB_NAME} PRIVATE
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
endfunction()
