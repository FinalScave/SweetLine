function(sweetline_platform_configure)
    set(SWEETLINE_PLATFORM_ID "android" PARENT_SCOPE)

    set(_sweetline_platform_files)
    if (SWEETLINE_BUILD_ANDROID_JNI)
        file(GLOB_RECURSE _sweetline_platform_files
                ${SWEETLINE_PROJECT_DIR}/platform/Android/sweetline/src/main/cpp/*.*)
    endif ()
    set(SWEETLINE_PLATFORM_SOURCE_FILES ${_sweetline_platform_files} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_INCLUDE_DIRS
            ${SWEETLINE_PROJECT_DIR}/platform/Android/sweetline/src/main/cpp
            PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS ANDROID PARENT_SCOPE)

    set(_sweetline_platform_link_libraries)
    if (SWEETLINE_BUILD_ANDROID_JNI)
        find_library(_sweetline_android_lib android)
        find_library(_sweetline_log_lib log)
        list(APPEND _sweetline_platform_link_libraries
                ${_sweetline_android_lib}
                ${_sweetline_log_lib})
    endif ()
    set(SWEETLINE_PLATFORM_LINK_LIBRARIES ${_sweetline_platform_link_libraries} PARENT_SCOPE)
endfunction()

function(sweetline_platform_supports_tests out_var)
    set(${out_var} OFF PARENT_SCOPE)
endfunction()
