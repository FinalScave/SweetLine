cmake_minimum_required(VERSION 3.10)
project(sweetline)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

if (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SWEETLINE_DEFAULT_BUILD_TESTS ON)
else ()
    set(SWEETLINE_DEFAULT_BUILD_TESTS OFF)
endif ()

option(SWEETLINE_BUILD_TESTS "Build SweetLine tests" ${SWEETLINE_DEFAULT_BUILD_TESTS})
option(SWEETLINE_BUILD_SHARED "Build SweetLine dynamic library" ON)
option(SWEETLINE_BUILD_STATIC "Build SweetLine static library" ON)
option(SWEETLINE_BUILD_WASM_EMBIND "Build SweetLine WebAssembly embind module" ON)
option(SWEETLINE_BUILD_APPLE_FRAMEWORK "Build SweetLine Apple framework" OFF)
option(SWEETLINE_APPLE_STATIC_FRAMEWORK "Build Apple framework target with a static Mach-O binary" OFF)

set(SWEETLINE_APPLE_FRAMEWORK_NAME "SweetLineCore")
set(SWEETLINE_APPLE_FRAMEWORK_IDENTIFIER "com.finalscave.sweetline.core")

if (EMSCRIPTEN)
    if (NOT SWEETLINE_BUILD_STATIC AND NOT SWEETLINE_BUILD_TESTS AND NOT SWEETLINE_BUILD_WASM_EMBIND)
        message(FATAL_ERROR "At least one of SWEETLINE_BUILD_STATIC, SWEETLINE_BUILD_TESTS, or SWEETLINE_BUILD_WASM_EMBIND must be enabled for Emscripten")
    endif ()
elseif (NOT SWEETLINE_BUILD_SHARED AND NOT SWEETLINE_BUILD_STATIC AND NOT SWEETLINE_BUILD_TESTS AND NOT SWEETLINE_BUILD_APPLE_FRAMEWORK)
    message(FATAL_ERROR "At least one of SWEETLINE_BUILD_SHARED, SWEETLINE_BUILD_STATIC, or SWEETLINE_BUILD_TESTS must be enabled")
endif ()

if (SWEETLINE_BUILD_APPLE_FRAMEWORK)
    if (NOT APPLE)
        message(FATAL_ERROR "SWEETLINE_BUILD_APPLE_FRAMEWORK is only supported on Apple platforms")
    endif ()
    if (NOT SWEETLINE_BUILD_SHARED)
        message(FATAL_ERROR "SWEETLINE_BUILD_APPLE_FRAMEWORK requires SWEETLINE_BUILD_SHARED=ON")
    endif ()
    if (SWEETLINE_BUILD_STATIC)
        message(FATAL_ERROR "SWEETLINE_BUILD_APPLE_FRAMEWORK requires SWEETLINE_BUILD_STATIC=OFF; build the static archive separately")
    endif ()
    if (SWEETLINE_APPLE_STATIC_FRAMEWORK AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        message(FATAL_ERROR "SWEETLINE_APPLE_STATIC_FRAMEWORK is only supported for iOS framework builds")
    endif ()
endif ()

set(SWEETLINE_LINK_LIB)
set(SWEETLINE_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(SWEETLINE_SRC_DIR ${SWEETLINE_PROJECT_DIR}/src)
set(SWEETLINE_PUBLIC_INCLUDE_DIR ${SWEETLINE_PROJECT_DIR}/include)
set(SWEETLINE_CORE_OBJECT_TARGET "${PROJECT_NAME}_core_objects")
set(SWEETLINE_C_API_OBJECT_TARGET "${PROJECT_NAME}_c_api_objects")
set(SWEETLINE_STATIC_LIB_NAME "${PROJECT_NAME}_static")
set(SWEETLINE_SHARED_LIB_NAME "${PROJECT_NAME}")

include(cmake/SweetLineThirdParty.cmake)
set(SWEETLINE_THIRD_PARTY_INCLUDE_DIRS
        $<TARGET_PROPERTY:SweetLine3p::NlohmannJson,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:SweetLine3p::UtfCpp,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:SweetLine3p::Oniguruma,INTERFACE_INCLUDE_DIRECTORIES>
)
list(APPEND SWEETLINE_LINK_LIB SweetLine3p::Oniguruma)

set(SWEETLINE_INCLUDE_DIRS
        ${SWEETLINE_PUBLIC_INCLUDE_DIR}
        ${SWEETLINE_SRC_DIR}
)

set(SWEETLINE_PLATFORM_FILES)
if (ANDROID)
    file(GLOB_RECURSE SWEETLINE_PLATFORM_FILES
            ${SWEETLINE_PROJECT_DIR}/platform/Android/sweetline/src/main/cpp/*.*)
    list(APPEND SWEETLINE_INCLUDE_DIRS ${SWEETLINE_PROJECT_DIR}/platform/Android/sweetline/src/main/cpp)
elseif (OHOS)
    file(GLOB_RECURSE SWEETLINE_PLATFORM_FILES
            ${SWEETLINE_PROJECT_DIR}/platform/OHOS/sweetline/src/main/cpp/*.*)
    list(APPEND SWEETLINE_INCLUDE_DIRS ${SWEETLINE_PROJECT_DIR}/platform/OHOS/sweetline/src/main/cpp)
endif ()

file(GLOB SWEETLINE_CORE_SOURCE_FILES ${SWEETLINE_SRC_DIR}/*.cpp)
set(SWEETLINE_C_API_SOURCE_FILES ${SWEETLINE_SRC_DIR}/c_sweetline.cpp)
list(REMOVE_ITEM SWEETLINE_CORE_SOURCE_FILES ${SWEETLINE_C_API_SOURCE_FILES})

set(SWEETLINE_C_API_OBJECTS)

function(sweetline_link_public_target target_name)
    target_include_directories(${target_name}
            PUBLIC
            $<BUILD_INTERFACE:${SWEETLINE_PUBLIC_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(${target_name} PRIVATE
            SweetLine3p::NlohmannJson
            SweetLine3p::UtfCpp
            ${SWEETLINE_LINK_LIB}
    )
endfunction()

function(sweetline_configure_compile_target target_name)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    target_include_directories(${target_name} PRIVATE
            ${SWEETLINE_INCLUDE_DIRS}
            ${SWEETLINE_THIRD_PARTY_INCLUDE_DIRS}
    )
    target_compile_definitions(${target_name} PRIVATE
            TESTS_DIR="${PROJECT_SOURCE_DIR}/tests"
            SYNTAX_DIR="${PROJECT_SOURCE_DIR}/syntaxes"
            SWEETLINE_DEBUG=1
            SWEETLINE_EXPORT=1
            ONIG_STATIC=ON
    )
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /Zc:__cplusplus /utf-8)
        target_compile_definitions(${target_name} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX=1)
    endif ()
    if (ANDROID)
        target_compile_definitions(${target_name} PRIVATE ANDROID)
    elseif (OHOS)
        target_compile_definitions(${target_name} PRIVATE OHOS)
    elseif (WIN32)
        target_compile_definitions(${target_name} PRIVATE WINDOWS _POSIX _POSIX_THREAD_SAFE_FUNCTIONS)
    elseif (APPLE)
        if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
            target_compile_definitions(${target_name} PRIVATE IOS)
        else ()
            target_compile_definitions(${target_name} PRIVATE OSX)
        endif ()
    elseif (EMSCRIPTEN)
        target_compile_options(${target_name} PRIVATE "-fexceptions")
        target_compile_definitions(${target_name} PRIVATE WASM)
    elseif (UNIX AND NOT APPLE)
        target_compile_definitions(${target_name} PRIVATE LINUX)
    endif ()
endfunction()

function(resolve_sweetline_apple_iconv_library out_var)
    if (CMAKE_OSX_SYSROOT AND EXISTS "${CMAKE_OSX_SYSROOT}/usr/lib/libiconv.tbd")
        set(${out_var} "${CMAKE_OSX_SYSROOT}/usr/lib/libiconv.tbd" PARENT_SCOPE)
    else ()
        set(${out_var} iconv PARENT_SCOPE)
    endif ()
endfunction()

if (ANDROID)
    find_library(SWEETLINE_ANDROID_LIB android)
    find_library(SWEETLINE_LOG_LIB log)
    list(APPEND SWEETLINE_LINK_LIB ${SWEETLINE_ANDROID_LIB} ${SWEETLINE_LOG_LIB})
elseif (OHOS)
    list(APPEND SWEETLINE_LINK_LIB libace_napi.z.so libhilog_ndk.z.so)
elseif (WIN32)
    list(APPEND SWEETLINE_LINK_LIB ws2_32)
elseif (APPLE)
    resolve_sweetline_apple_iconv_library(SWEETLINE_ICONV_LIB)
    list(APPEND SWEETLINE_LINK_LIB ${SWEETLINE_ICONV_LIB})
endif ()

add_library(${SWEETLINE_CORE_OBJECT_TARGET} OBJECT
        ${SWEETLINE_CORE_SOURCE_FILES}
        ${SWEETLINE_PLATFORM_FILES}
)
set_target_properties(${SWEETLINE_CORE_OBJECT_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
sweetline_configure_compile_target(${SWEETLINE_CORE_OBJECT_TARGET})

if (NOT EMSCRIPTEN AND (SWEETLINE_BUILD_STATIC OR SWEETLINE_BUILD_TESTS OR (SWEETLINE_BUILD_SHARED AND NOT APPLE)))
    add_library(${SWEETLINE_C_API_OBJECT_TARGET} OBJECT ${SWEETLINE_C_API_SOURCE_FILES})
    set_target_properties(${SWEETLINE_C_API_OBJECT_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    sweetline_configure_compile_target(${SWEETLINE_C_API_OBJECT_TARGET})
    list(APPEND SWEETLINE_C_API_OBJECTS $<TARGET_OBJECTS:${SWEETLINE_C_API_OBJECT_TARGET}>)
endif ()

if (SWEETLINE_BUILD_SHARED AND NOT EMSCRIPTEN AND NOT SWEETLINE_BUILD_APPLE_FRAMEWORK)
    message(STATUS "add target: ${SWEETLINE_SHARED_LIB_NAME} as dynamic lib")
    if (APPLE)
        add_library(${SWEETLINE_SHARED_LIB_NAME} SHARED
                ${SWEETLINE_CORE_SOURCE_FILES}
                ${SWEETLINE_C_API_SOURCE_FILES}
                ${SWEETLINE_PLATFORM_FILES}
        )
        sweetline_configure_compile_target(${SWEETLINE_SHARED_LIB_NAME})
    else ()
        add_library(${SWEETLINE_SHARED_LIB_NAME} SHARED
                $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
                ${SWEETLINE_C_API_OBJECTS}
        )
    endif ()
    sweetline_link_public_target(${SWEETLINE_SHARED_LIB_NAME})
    add_library(SweetLine::sweetline ALIAS ${SWEETLINE_SHARED_LIB_NAME})
elseif (SWEETLINE_BUILD_SHARED AND EMSCRIPTEN)
    message(STATUS "skip target: ${SWEETLINE_SHARED_LIB_NAME} dynamic lib is not used for Emscripten")
endif ()

if (SWEETLINE_BUILD_STATIC AND NOT SWEETLINE_BUILD_APPLE_FRAMEWORK)
    message(STATUS "add target: ${SWEETLINE_STATIC_LIB_NAME} as static lib")
    add_library(${SWEETLINE_STATIC_LIB_NAME} STATIC
            $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
            ${SWEETLINE_C_API_OBJECTS}
    )
    sweetline_link_public_target(${SWEETLINE_STATIC_LIB_NAME})
    target_compile_definitions(${SWEETLINE_STATIC_LIB_NAME} PUBLIC SWEETLINE_STATIC=1)
    add_library(SweetLine::sweetline_static ALIAS ${SWEETLINE_STATIC_LIB_NAME})

    include(cmake/MergeStaticLib.cmake)
    merge_static_libs(${SWEETLINE_STATIC_LIB_NAME} SweetLine3p::Oniguruma)
endif ()

if (APPLE AND SWEETLINE_BUILD_APPLE_FRAMEWORK)
    set(SWEETLINE_APPLE_FRAMEWORK_HEADER "${PROJECT_SOURCE_DIR}/cmake/apple/SweetLineCore.h")
    set(SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS
            "${SWEETLINE_APPLE_FRAMEWORK_HEADER}"
            "${SWEETLINE_PUBLIC_INCLUDE_DIR}/sweetline/c_sweetline.h"
    )
    set_source_files_properties(${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers")
    message(STATUS "add target: ${SWEETLINE_APPLE_FRAMEWORK_NAME} as Apple framework")
    add_library(${SWEETLINE_APPLE_FRAMEWORK_NAME} SHARED
            ${SWEETLINE_CORE_SOURCE_FILES}
            ${SWEETLINE_C_API_SOURCE_FILES}
            ${SWEETLINE_PLATFORM_FILES}
            ${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS}
    )

    sweetline_configure_compile_target(${SWEETLINE_APPLE_FRAMEWORK_NAME})
    sweetline_link_public_target(${SWEETLINE_APPLE_FRAMEWORK_NAME})
    set_target_properties(${SWEETLINE_APPLE_FRAMEWORK_NAME} PROPERTIES
            FRAMEWORK TRUE
            FRAMEWORK_VERSION A
            MACOSX_FRAMEWORK_IDENTIFIER "${SWEETLINE_APPLE_FRAMEWORK_IDENTIFIER}"
            OUTPUT_NAME "${SWEETLINE_APPLE_FRAMEWORK_NAME}"
            PUBLIC_HEADER "${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS}"
            XCODE_ATTRIBUTE_DEFINES_MODULE YES
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${SWEETLINE_APPLE_FRAMEWORK_IDENTIFIER}"
        )

    if (SWEETLINE_APPLE_STATIC_FRAMEWORK)
        set_target_properties(${SWEETLINE_APPLE_FRAMEWORK_NAME} PROPERTIES
                XCODE_ATTRIBUTE_MACH_O_TYPE staticlib
        )
    else ()
        set_target_properties(${SWEETLINE_APPLE_FRAMEWORK_NAME} PROPERTIES
                XCODE_ATTRIBUTE_MACH_O_TYPE mh_dylib
                BUILD_WITH_INSTALL_NAME_DIR TRUE
                INSTALL_NAME_DIR "@rpath"
        )
    endif ()
endif ()

if (EMSCRIPTEN AND SWEETLINE_BUILD_WASM_EMBIND)
    include(cmake/SweetLineWasm.cmake)
endif ()

if (SWEETLINE_BUILD_TESTS)
    enable_testing()
    message(STATUS "Unit tests enabled")
    if (NOT ANDROID AND NOT OHOS AND NOT EMSCRIPTEN)
        add_subdirectory(${SWEETLINE_PROJECT_DIR}/tests)
    endif ()
endif ()

include(GNUInstallDirs)
set(INSTALL_TARGETS)
if (TARGET ${SWEETLINE_SHARED_LIB_NAME})
    list(APPEND INSTALL_TARGETS ${SWEETLINE_SHARED_LIB_NAME})
endif ()
if (TARGET ${SWEETLINE_STATIC_LIB_NAME})
    list(APPEND INSTALL_TARGETS ${SWEETLINE_STATIC_LIB_NAME})
endif ()
if (INSTALL_TARGETS)
    install(TARGETS ${INSTALL_TARGETS}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    if (MSVC)
        foreach(TARGET_NAME ${INSTALL_TARGETS})
            get_target_property(tgt_type ${TARGET_NAME} TYPE)
            if (NOT tgt_type STREQUAL "STATIC_LIBRARY")
                install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}>
                        DESTINATION ${CMAKE_INSTALL_BINDIR}
                        OPTIONAL
                )
            endif ()
        endforeach ()
    endif ()
endif ()

install(DIRECTORY ${SWEETLINE_PUBLIC_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
