function(sweetline_platform_configure)
    set(SWEETLINE_PLATFORM_ID "ohos" PARENT_SCOPE)

    file(GLOB_RECURSE _sweetline_platform_files
            ${SWEETLINE_PROJECT_DIR}/platform/OHOS/sweetline/src/main/cpp/*.*)
    set(SWEETLINE_PLATFORM_SOURCE_FILES ${_sweetline_platform_files} PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_INCLUDE_DIRS
            ${SWEETLINE_PROJECT_DIR}/platform/OHOS/sweetline/src/main/cpp
            PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_COMPILE_DEFINITIONS OHOS PARENT_SCOPE)
    set(SWEETLINE_PLATFORM_LINK_LIBRARIES libace_napi.z.so libhilog_ndk.z.so PARENT_SCOPE)
endfunction()

function(sweetline_platform_supports_tests out_var)
    set(${out_var} OFF PARENT_SCOPE)
endfunction()
