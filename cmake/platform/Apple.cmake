function(sweetline_resolve_apple_iconv_library out_var)
    if (CMAKE_OSX_SYSROOT AND EXISTS "${CMAKE_OSX_SYSROOT}/usr/lib/libiconv.tbd")
        set(${out_var} "${CMAKE_OSX_SYSROOT}/usr/lib/libiconv.tbd" PARENT_SCOPE)
    else ()
        set(${out_var} iconv PARENT_SCOPE)
    endif ()
endfunction()


function(sweetline_platform_configure)
    sweetline_resolve_apple_iconv_library(_sweetline_iconv_lib)

    set(SWEETLINE_PLATFORM_LINK_LIBRARIES ${_sweetline_iconv_lib} PARENT_SCOPE)

    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(SWEETLINE_PLATFORM_ID "ios" PARENT_SCOPE)
        set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS IOS PARENT_SCOPE)
    else ()
        set(SWEETLINE_PLATFORM_ID "osx" PARENT_SCOPE)
        set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS OSX PARENT_SCOPE)
    endif ()
endfunction()

function(sweetline_platform_validate_build_options)
    sweetline_validate_standard_build_options()
    if (SWEETLINE_BUILD_APPLE_FRAMEWORK)
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
endfunction()

function(sweetline_platform_should_build_c_api_objects out_var)
    if (SWEETLINE_BUILD_STATIC OR SWEETLINE_BUILD_TESTS)
        set(${out_var} ON PARENT_SCOPE)
    else ()
        set(${out_var} OFF PARENT_SCOPE)
    endif ()
endfunction()

function(sweetline_platform_add_shared_target out_handled)
    if (NOT SWEETLINE_BUILD_SHARED)
        set(${out_handled} OFF PARENT_SCOPE)
        return()
    endif ()
    if (SWEETLINE_BUILD_APPLE_FRAMEWORK)
        set(${out_handled} ON PARENT_SCOPE)
        return()
    endif ()

    message(STATUS "add target: ${SWEETLINE_SHARED_LIB_NAME} as dynamic lib")
    add_library(${SWEETLINE_SHARED_LIB_NAME} SHARED
            ${SWEETLINE_CORE_SOURCE_FILES}
            ${SWEETLINE_C_API_SOURCE_FILES}
            ${SWEETLINE_PLATFORM_FILES}
    )
    sweetline_configure_compile_target(${SWEETLINE_SHARED_LIB_NAME})
    sweetline_link_public_target(${SWEETLINE_SHARED_LIB_NAME})
    add_library(SweetLine::sweetline ALIAS ${SWEETLINE_SHARED_LIB_NAME})
    set(${out_handled} ON PARENT_SCOPE)
endfunction()

function(sweetline_platform_add_extra_targets)
    if (NOT SWEETLINE_BUILD_APPLE_FRAMEWORK)
        return()
    endif ()

    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(_sweetline_framework_name "SweetLineCoreIOS")
        set(_sweetline_framework_identifier "com.qiplat.sweetline.core.ios")
    else ()
        set(_sweetline_framework_name "SweetLineCoreMacOS")
        set(_sweetline_framework_identifier "com.qiplat.sweetline.core.macos")
    endif ()

    set(SWEETLINE_APPLE_FRAMEWORK_HEADER_DIR
            "${PROJECT_BINARY_DIR}/cmake/platform/apple/${_sweetline_framework_name}"
    )
    set(SWEETLINE_APPLE_FRAMEWORK_HEADER
            "${SWEETLINE_APPLE_FRAMEWORK_HEADER_DIR}/${_sweetline_framework_name}.h"
    )
    file(MAKE_DIRECTORY "${SWEETLINE_APPLE_FRAMEWORK_HEADER_DIR}")
    file(WRITE "${SWEETLINE_APPLE_FRAMEWORK_HEADER}"
            "#pragma once\n\n#include <${_sweetline_framework_name}/c_sweetline.h>\n"
    )
    set(SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS
            "${SWEETLINE_APPLE_FRAMEWORK_HEADER}"
            "${SWEETLINE_PUBLIC_INCLUDE_DIR}/sweetline/c_sweetline.h"
    )
    set_source_files_properties(${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS} PROPERTIES MACOSX_PACKAGE_LOCATION "Headers")
    message(STATUS "add target: ${_sweetline_framework_name} as Apple framework")
    add_library(${_sweetline_framework_name} SHARED
            ${SWEETLINE_CORE_SOURCE_FILES}
            ${SWEETLINE_C_API_SOURCE_FILES}
            ${SWEETLINE_PLATFORM_FILES}
            ${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS}
    )

    sweetline_configure_compile_target(${_sweetline_framework_name})
    sweetline_link_public_target(${_sweetline_framework_name})
    set_target_properties(${_sweetline_framework_name} PROPERTIES
            FRAMEWORK TRUE
            FRAMEWORK_VERSION A
            MACOSX_FRAMEWORK_IDENTIFIER "${_sweetline_framework_identifier}"
            OUTPUT_NAME "${_sweetline_framework_name}"
            PUBLIC_HEADER "${SWEETLINE_APPLE_FRAMEWORK_PUBLIC_HEADERS}"
            XCODE_ATTRIBUTE_DEFINES_MODULE YES
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${_sweetline_framework_identifier}"
    )

    if (SWEETLINE_APPLE_STATIC_FRAMEWORK)
        set_target_properties(${_sweetline_framework_name} PROPERTIES
                XCODE_ATTRIBUTE_MACH_O_TYPE staticlib
        )
    else ()
        set_target_properties(${_sweetline_framework_name} PROPERTIES
                XCODE_ATTRIBUTE_MACH_O_TYPE mh_dylib
                BUILD_WITH_INSTALL_NAME_DIR TRUE
                INSTALL_NAME_DIR "@rpath"
        )
    endif ()
endfunction()
