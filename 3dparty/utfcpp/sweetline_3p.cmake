if (TARGET SweetLine3p::UtfCpp)
    return()
endif ()

add_library(sweetline_3p_utfcpp INTERFACE)

target_compile_features(sweetline_3p_utfcpp INTERFACE cxx_std_17)

target_include_directories(sweetline_3p_utfcpp
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(SweetLine3p::UtfCpp ALIAS sweetline_3p_utfcpp)
