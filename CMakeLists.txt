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
option(SWEETLINE_BUILD_ANDROID_JNI "Build SweetLine Android Java/JNI bridge" ON)
option(SWEETLINE_BUILD_WASM_EMBIND "Build SweetLine WebAssembly embind module" ON)
option(SWEETLINE_BUILD_APPLE_FRAMEWORK "Build SweetLine Apple framework" OFF)
option(SWEETLINE_APPLE_STATIC_FRAMEWORK "Build Apple framework target with a static Mach-O binary" OFF)

set(SWEETLINE_PROJECT_DIR ${PROJECT_SOURCE_DIR})
set(SWEETLINE_PUBLIC_INCLUDE_DIR ${SWEETLINE_PROJECT_DIR}/include)
set(SWEETLINE_STATIC_LIB_NAME "${PROJECT_NAME}_static")
set(SWEETLINE_SHARED_LIB_NAME "${PROJECT_NAME}")

include(cmake/SweetLineTargets.cmake)
sweetline_configure_targets()

if (SWEETLINE_BUILD_TESTS)
    enable_testing()
    message(STATUS "Unit tests enabled")
    if (SWEETLINE_TARGETS_ENABLE_TESTS)
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
    foreach(TARGET_NAME ${INSTALL_TARGETS})
        sweetline_platform_install_target_debug_symbols(${TARGET_NAME})
    endforeach ()
endif ()

install(DIRECTORY ${SWEETLINE_PUBLIC_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
