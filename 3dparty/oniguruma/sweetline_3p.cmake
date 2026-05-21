if (TARGET SweetLine3p::Oniguruma)
    return()
endif ()

set(SWEETLINE_3P_ONIGURUMA_LIBRARY)

if (WIN32 AND MSVC)
    set(SWEETLINE_3P_ONIGURUMA_LIBRARY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/lib/windows/x64/debug/onig.lib)
    set(SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/lib/windows/x64/release/onig.lib)
    if (NOT EXISTS ${SWEETLINE_3P_ONIGURUMA_LIBRARY_DEBUG})
        message(FATAL_ERROR "Missing Oniguruma debug prebuilt library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY_DEBUG}")
    endif ()
    if (NOT EXISTS ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE})
        message(FATAL_ERROR "Missing Oniguruma release prebuilt library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE}")
    endif ()

    message(STATUS "Add third-party library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY_DEBUG}")
    message(STATUS "Add third-party library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE}")
    add_library(sweetline_3p_oniguruma STATIC IMPORTED)
    set_target_properties(sweetline_3p_oniguruma PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
            IMPORTED_LOCATION_DEBUG ${SWEETLINE_3P_ONIGURUMA_LIBRARY_DEBUG}
            IMPORTED_LOCATION_RELEASE ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE}
            IMPORTED_LOCATION_RELWITHDEBINFO ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE}
            IMPORTED_LOCATION_MINSIZEREL ${SWEETLINE_3P_ONIGURUMA_LIBRARY_RELEASE}
    )
else ()
    if (ANDROID)
        set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/android/${CMAKE_ANDROID_ARCH_ABI}/release/libonig.a)
    elseif (OHOS)
        set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/ohos/${OHOS_ARCH}/release/libonig.a)
    elseif (WIN32)
        set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/windows/x64/release/libonig.a)
    elseif (APPLE)
        if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
            if (CMAKE_OSX_SYSROOT MATCHES ".*simulator.*" OR CMAKE_OSX_SYSROOT STREQUAL "iphonesimulator")
                set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/ios/simulator-${CMAKE_OSX_ARCHITECTURES}/release/libonig.a)
            else ()
                set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/ios/${CMAKE_OSX_ARCHITECTURES}/release/libonig.a)
            endif ()
        else ()
            set(_SWEETLINE_3P_MACOS_ARCH ${CMAKE_OSX_ARCHITECTURES})
            if (NOT _SWEETLINE_3P_MACOS_ARCH OR _SWEETLINE_3P_MACOS_ARCH STREQUAL "")
                execute_process(COMMAND uname -m OUTPUT_VARIABLE _SWEETLINE_3P_MACOS_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
            endif ()
            set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/macos/${_SWEETLINE_3P_MACOS_ARCH}/release/libonig.a)
        endif ()
    elseif (EMSCRIPTEN)
        set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/emscripten/release/libonig.a)
    elseif (UNIX AND NOT APPLE)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
            set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/linux/aarch64/release/libonig.a)
        elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
            set(SWEETLINE_3P_ONIGURUMA_LIBRARY ${CMAKE_CURRENT_LIST_DIR}/lib/linux/x86_64/release/libonig.a)
        else ()
            message(FATAL_ERROR "Unsupported Linux processor for prebuilt Oniguruma: ${CMAKE_SYSTEM_PROCESSOR}")
        endif ()
    endif ()

    if (NOT SWEETLINE_3P_ONIGURUMA_LIBRARY OR NOT EXISTS ${SWEETLINE_3P_ONIGURUMA_LIBRARY})
        message(FATAL_ERROR "Unsupported or missing Oniguruma prebuilt library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY}")
    endif ()

    message(STATUS "Add third-party library: ${SWEETLINE_3P_ONIGURUMA_LIBRARY}")
    add_library(sweetline_3p_oniguruma STATIC IMPORTED)
    set_target_properties(sweetline_3p_oniguruma PROPERTIES IMPORTED_LOCATION ${SWEETLINE_3P_ONIGURUMA_LIBRARY})
endif ()

target_compile_definitions(sweetline_3p_oniguruma
        INTERFACE
        ONIG_STATIC=ON
)

target_include_directories(sweetline_3p_oniguruma
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(SweetLine3p::Oniguruma ALIAS sweetline_3p_oniguruma)
