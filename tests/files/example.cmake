cmake_minimum_required(VERSION 3.26...3.31 FATAL_ERROR)
project(SweetLineDemo VERSION 1.4.2 DESCRIPTION "Example CMake file for SweetLine" HOMEPAGE_URL "https://example.com/cmake" LANGUAGES C CXX)

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(APP_DOCS_URL "https://example.com/cmake")
set(APP_BINARY_DIR "${CMAKE_BINARY_DIR}/bin")
set(APP_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(APP_TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(APP_FEATURES syntax url comments fragments)
set(APP_WARNINGS -Wall -Wextra -Wpedantic)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_DEMO "Build demo target" ON)
option(ENABLE_TESTS "Build unit tests" ON)
option(ENABLE_SANITIZERS "Enable sanitizer flags in Debug" OFF)
cmake_dependent_option(
  ENABLE_PACKAGING
  "Build install and package targets"
  ON
  "ENABLE_DEMO"
  OFF
)

set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Build type" FORCE)
endif()

check_cxx_compiler_flag("-fno-exceptions" HAS_NO_EXCEPTIONS)

function(sweetline_configure_target target_name)
  set(options ENABLE_IPO)
  set(one_value_args OUTPUT_NAME FOLDER)
  set(multi_value_args SOURCES DEFINITIONS INCLUDES FEATURES)
  cmake_parse_arguments(SCFG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

  add_library(${target_name} STATIC ${SCFG_SOURCES})
  target_include_directories(
    ${target_name}
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${APP_GENERATED_DIR}>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
      ${SCFG_INCLUDES}
  )
  target_compile_definitions(
    ${target_name}
    PRIVATE
      APP_DOCS_URL="${APP_DOCS_URL}"
      $<$<BOOL:${WIN32}>:SL_PLATFORM_WINDOWS>
      $<$<CONFIG:Debug>:SL_DEBUG_BUILD>
      ${SCFG_DEFINITIONS}
  )
  target_compile_features(${target_name} PUBLIC cxx_std_17 ${SCFG_FEATURES})
  target_compile_options(
    ${target_name}
    PRIVATE
      "$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall;-Wextra;-Wconversion>"
      "$<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->"
      "$<$<AND:$<CONFIG:Debug>,$<BOOL:${ENABLE_SANITIZERS}>>:-fsanitize=address,undefined>"
  )
  set_target_properties(
    ${target_name}
    PROPERTIES
      OUTPUT_NAME "${SCFG_OUTPUT_NAME}"
      FOLDER "${SCFG_FOLDER}"
      POSITION_INDEPENDENT_CODE ON
      CXX_VISIBILITY_PRESET hidden
      VISIBILITY_INLINES_HIDDEN YES
  )

  if(SCFG_ENABLE_IPO)
    set_property(TARGET ${target_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endfunction()

file(MAKE_DIRECTORY "${APP_GENERATED_DIR}")
configure_file(
  "${APP_TEMPLATE_DIR}/sweetline_config.hpp.in"
  "${APP_GENERATED_DIR}/sweetline_config.hpp"
  @ONLY
)

set(core_sources
  src/core/document.cpp
  src/core/highlight.cpp
  src/core/parser.cpp
  src/core/syntax.cpp
)

set(tool_sources
  src/tools/dump_tokens.cpp
  src/tools/render_preview.cpp
)

sweetline_configure_target(
  sweetline_core
  OUTPUT_NAME sweetline-core
  FOLDER libraries
  SOURCES ${core_sources}
  DEFINITIONS SL_DOCS_URL="${APP_DOCS_URL}"
  FEATURES cxx_std_20
)

add_executable(sweetline_cli ${tool_sources})
target_link_libraries(
  sweetline_cli
  PRIVATE
    sweetline_core
    "$<$<PLATFORM_ID:Darwin>:-framework CoreFoundation>"
)
target_compile_definitions(
  sweetline_cli
  PRIVATE
    "$<$<BOOL:${HAS_NO_EXCEPTIONS}>:SL_CAN_DISABLE_EXCEPTIONS>"
)
set_property(TARGET sweetline_cli PROPERTY RUNTIME_OUTPUT_DIRECTORY "${APP_BINARY_DIR}")

if(ENABLE_DEMO AND EXISTS "${CMAKE_SOURCE_DIR}/demo")
  add_subdirectory(demo EXCLUDE_FROM_ALL)
endif()

if(ENABLE_TESTS)
  enable_testing()
  add_executable(sweetline_tests tests/test_main.cpp tests/test_parser.cpp)
  target_link_libraries(sweetline_tests PRIVATE sweetline_core)
  add_test(NAME syntax.parser COMMAND sweetline_tests --suite parser)
  add_test(NAME syntax.fragments COMMAND sweetline_tests --suite fragments)
endif()

if(ENABLE_PACKAGING)
  install(TARGETS sweetline_core sweetline_cli EXPORT SweetLineTargets)
  install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  install(FILES "${APP_GENERATED_DIR}/sweetline_config.hpp" LICENSE README.md DESTINATION "${CMAKE_INSTALL_DATADIR}/sweetline")
endif()

message(STATUS "SweetLine docs: ${APP_DOCS_URL}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Enabled features: ${APP_FEATURES}")
