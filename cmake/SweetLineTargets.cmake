#[[
This file owns the common SweetLine target graph. Platform files under
cmake/platform are included after platform detection and may override the
default sweetline_platform_* hooks declared below.

Platform hooks may set these values from sweetline_platform_configure:
SWEETLINE_PLATFORM_SOURCE_FILES, SWEETLINE_PLATFORM_INCLUDE_DIRS,
SWEETLINE_PLATFORM_COMPILE_DEFINITIONS, SWEETLINE_PLATFORM_COMPILE_OPTIONS,
SWEETLINE_PLATFORM_LINK_LIBRARIES.

Platform files may override:
sweetline_platform_configure()
sweetline_platform_validate_build_options()
sweetline_platform_supports_tests(out_var)
sweetline_platform_should_build_c_api_objects(out_var)
sweetline_platform_add_shared_target(out_handled)
sweetline_platform_add_extra_targets()
sweetline_platform_install_target_debug_symbols(target_name)

Common library targets should keep their public include and link behavior
centralized through sweetline_link_public_target().
]]

set(SWEETLINE_TARGETS_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(sweetline_platform_configure)
endfunction()

function(sweetline_platform_supports_tests out_var)
    set(${out_var} ON PARENT_SCOPE)
endfunction()

function(sweetline_platform_should_build_c_api_objects out_var)
    if (SWEETLINE_BUILD_SHARED OR SWEETLINE_BUILD_STATIC OR SWEETLINE_BUILD_TESTS)
        set(${out_var} ON PARENT_SCOPE)
    else ()
        set(${out_var} OFF PARENT_SCOPE)
    endif ()
endfunction()

function(sweetline_platform_add_shared_target out_handled)
    set(${out_handled} OFF PARENT_SCOPE)
endfunction()

function(sweetline_platform_add_extra_targets)
endfunction()

function(sweetline_platform_install_target_debug_symbols target_name)
endfunction()

function(sweetline_validate_standard_build_options)
    if (NOT SWEETLINE_BUILD_SHARED
            AND NOT SWEETLINE_BUILD_STATIC
            AND NOT SWEETLINE_BUILD_TESTS
            AND NOT SWEETLINE_BUILD_APPLE_FRAMEWORK)
        message(FATAL_ERROR "At least one SweetLine target must be enabled for this platform")
    endif ()
endfunction()

function(sweetline_platform_validate_build_options)
    if (SWEETLINE_BUILD_APPLE_FRAMEWORK)
        message(FATAL_ERROR "SWEETLINE_BUILD_APPLE_FRAMEWORK is only supported on Apple platforms")
    endif ()
    sweetline_validate_standard_build_options()
endfunction()

function(sweetline_configure_platform)
    if (NOT SWEETLINE_PROJECT_DIR)
        get_filename_component(SWEETLINE_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
    endif ()

    set(SWEETLINE_PLATFORM_ID "generic")
    set(SWEETLINE_PLATFORM_FILE "")

    if (ANDROID)
        set(SWEETLINE_PLATFORM_ID "android")
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/Android.cmake")
    elseif (OHOS)
        set(SWEETLINE_PLATFORM_ID "ohos")
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/OHOS.cmake")
    elseif (WIN32)
        set(SWEETLINE_PLATFORM_ID "windows")
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/Windows.cmake")
    elseif (APPLE)
        if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
            set(SWEETLINE_PLATFORM_ID "ios")
        else ()
            set(SWEETLINE_PLATFORM_ID "osx")
        endif ()
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/Apple.cmake")
    elseif (EMSCRIPTEN)
        set(SWEETLINE_PLATFORM_ID "emscripten")
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/Emscripten.cmake")
    elseif (UNIX)
        set(SWEETLINE_PLATFORM_ID "linux")
        set(SWEETLINE_PLATFORM_FILE "${SWEETLINE_TARGETS_CMAKE_DIR}/platform/Linux.cmake")
    endif ()

    set(_sweetline_platform_id "${SWEETLINE_PLATFORM_ID}")
    set(_sweetline_platform_file "${SWEETLINE_PLATFORM_FILE}")
    if (_sweetline_platform_file)
        include("${_sweetline_platform_file}")
    endif ()

    set(SWEETLINE_PLATFORM_ID "${_sweetline_platform_id}")
    set(SWEETLINE_PLATFORM_SOURCE_FILES)
    set(SWEETLINE_PLATFORM_INCLUDE_DIRS)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS)
    set(SWEETLINE_PLATFORM_COMPILE_OPTIONS)
    set(SWEETLINE_PLATFORM_LINK_LIBRARIES)

    sweetline_platform_configure()

    set(SWEETLINE_PLATFORM_ID "${SWEETLINE_PLATFORM_ID}" PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_SOURCE_FILES ${SWEETLINE_PLATFORM_SOURCE_FILES} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_INCLUDE_DIRS ${SWEETLINE_PLATFORM_INCLUDE_DIRS} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS ${SWEETLINE_PLATFORM_COMPILE_DEFINITIONS} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_OPTIONS ${SWEETLINE_PLATFORM_COMPILE_OPTIONS} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_LINK_LIBRARIES ${SWEETLINE_PLATFORM_LINK_LIBRARIES} PARENT_SCOPE)
endfunction()

function(sweetline_apply_platform_target target_name)
    if (SWEETLINE_PLATFORM_COMPILE_OPTIONS)
        target_compile_options(${target_name} PRIVATE ${SWEETLINE_PLATFORM_COMPILE_OPTIONS})
    endif ()
    if (SWEETLINE_PLATFORM_COMPILE_DEFINITIONS)
        target_compile_definitions(${target_name} PRIVATE ${SWEETLINE_PLATFORM_COMPILE_DEFINITIONS})
    endif ()
endfunction()

function(sweetline_link_common_libraries target_name)
    target_link_libraries(${target_name} PRIVATE
            SweetLine3p::NlohmannJson
            SweetLine3p::UtfCpp
            ${SWEETLINE_LINK_LIB}
    )
endfunction()

function(sweetline_link_public_target target_name)
    target_include_directories(${target_name}
            PUBLIC
            $<BUILD_INTERFACE:${SWEETLINE_PUBLIC_INCLUDE_DIR}>
            $<INSTALL_INTERFACE:include>
    )
    sweetline_link_common_libraries(${target_name})
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
    sweetline_apply_platform_target(${target_name})
endfunction()

function(sweetline_configure_targets)
    set(SWEETLINE_LINK_LIB)
    set(SWEETLINE_SRC_DIR ${SWEETLINE_PROJECT_DIR}/src)
    set(SWEETLINE_CORE_OBJECT_TARGET "${PROJECT_NAME}_core_objects")
    set(SWEETLINE_C_API_OBJECT_TARGET "${PROJECT_NAME}_c_api_objects")

    sweetline_configure_platform()
    sweetline_platform_validate_build_options()

    include("${SWEETLINE_TARGETS_CMAKE_DIR}/SweetLineThirdParty.cmake")
    set(SWEETLINE_THIRD_PARTY_INCLUDE_DIRS
            $<TARGET_PROPERTY:SweetLine3p::NlohmannJson,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:SweetLine3p::UtfCpp,INTERFACE_INCLUDE_DIRECTORIES>
            $<TARGET_PROPERTY:SweetLine3p::Oniguruma,INTERFACE_INCLUDE_DIRECTORIES>
    )
    set(SWEETLINE_LINK_LIB SweetLine3p::Oniguruma ${SWEETLINE_PLATFORM_LINK_LIBRARIES})

    set(SWEETLINE_INCLUDE_DIRS
            ${SWEETLINE_PUBLIC_INCLUDE_DIR}
            ${SWEETLINE_SRC_DIR}
            ${SWEETLINE_PLATFORM_INCLUDE_DIRS}
    )
    set(SWEETLINE_PLATFORM_FILES ${SWEETLINE_PLATFORM_SOURCE_FILES})

    file(GLOB SWEETLINE_CORE_SOURCE_FILES ${SWEETLINE_SRC_DIR}/*.cpp)
    set(SWEETLINE_C_API_SOURCE_FILES ${SWEETLINE_SRC_DIR}/c_sweetline.cpp)
    list(REMOVE_ITEM SWEETLINE_CORE_SOURCE_FILES ${SWEETLINE_C_API_SOURCE_FILES})

    sweetline_platform_supports_tests(SWEETLINE_TARGETS_ENABLE_TESTS)

    add_library(${SWEETLINE_CORE_OBJECT_TARGET} OBJECT
            ${SWEETLINE_CORE_SOURCE_FILES}
            ${SWEETLINE_PLATFORM_FILES}
    )
    set_target_properties(${SWEETLINE_CORE_OBJECT_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    sweetline_configure_compile_target(${SWEETLINE_CORE_OBJECT_TARGET})

    sweetline_platform_should_build_c_api_objects(_sweetline_build_c_api_objects)
    set(SWEETLINE_C_API_OBJECTS)
    if (_sweetline_build_c_api_objects)
        add_library(${SWEETLINE_C_API_OBJECT_TARGET} OBJECT ${SWEETLINE_C_API_SOURCE_FILES})
        set_target_properties(${SWEETLINE_C_API_OBJECT_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        sweetline_configure_compile_target(${SWEETLINE_C_API_OBJECT_TARGET})
        list(APPEND SWEETLINE_C_API_OBJECTS $<TARGET_OBJECTS:${SWEETLINE_C_API_OBJECT_TARGET}>)
    endif ()

    sweetline_platform_add_shared_target(_sweetline_platform_handled)
    if (NOT _sweetline_platform_handled AND SWEETLINE_BUILD_SHARED)
        message(STATUS "add target: ${SWEETLINE_SHARED_LIB_NAME} as dynamic lib")
        add_library(${SWEETLINE_SHARED_LIB_NAME} SHARED
                $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
                ${SWEETLINE_C_API_OBJECTS}
        )
        sweetline_link_public_target(${SWEETLINE_SHARED_LIB_NAME})
        add_library(SweetLine::sweetline ALIAS ${SWEETLINE_SHARED_LIB_NAME})
    endif ()

    if (SWEETLINE_BUILD_STATIC)
        message(STATUS "add target: ${SWEETLINE_STATIC_LIB_NAME} as static lib")
        add_library(${SWEETLINE_STATIC_LIB_NAME} STATIC
                $<TARGET_OBJECTS:${SWEETLINE_CORE_OBJECT_TARGET}>
                ${SWEETLINE_C_API_OBJECTS}
        )
        sweetline_link_public_target(${SWEETLINE_STATIC_LIB_NAME})
        target_compile_definitions(${SWEETLINE_STATIC_LIB_NAME} PUBLIC SWEETLINE_STATIC=1)
        add_library(SweetLine::sweetline_static ALIAS ${SWEETLINE_STATIC_LIB_NAME})

        include("${SWEETLINE_TARGETS_CMAKE_DIR}/MergeStaticLib.cmake")
        merge_static_libs(${SWEETLINE_STATIC_LIB_NAME} SweetLine3p::Oniguruma)
    endif ()

    sweetline_platform_add_extra_targets()

    set(SWEETLINE_PLATFORM_ID "${SWEETLINE_PLATFORM_ID}" PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS ${SWEETLINE_PLATFORM_COMPILE_DEFINITIONS} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_OPTIONS ${SWEETLINE_PLATFORM_COMPILE_OPTIONS} PARENT_SCOPE)
    set(SWEETLINE_CORE_OBJECT_TARGET ${SWEETLINE_CORE_OBJECT_TARGET} PARENT_SCOPE)
    set(SWEETLINE_C_API_OBJECT_TARGET ${SWEETLINE_C_API_OBJECT_TARGET} PARENT_SCOPE)
    set(SWEETLINE_INCLUDE_DIRS ${SWEETLINE_INCLUDE_DIRS} PARENT_SCOPE)
    set(SWEETLINE_LINK_LIB ${SWEETLINE_LINK_LIB} PARENT_SCOPE)
    set(SWEETLINE_THIRD_PARTY_INCLUDE_DIRS ${SWEETLINE_THIRD_PARTY_INCLUDE_DIRS} PARENT_SCOPE)
    set(SWEETLINE_C_API_OBJECTS ${SWEETLINE_C_API_OBJECTS} PARENT_SCOPE)
    set(SWEETLINE_TARGETS_ENABLE_TESTS ${SWEETLINE_TARGETS_ENABLE_TESTS} PARENT_SCOPE)
endfunction()
