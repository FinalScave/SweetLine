if (TARGET SweetLine3p::NlohmannJson)
    return()
endif ()

add_library(sweetline_3p_nlohmann_json INTERFACE)

target_compile_features(sweetline_3p_nlohmann_json INTERFACE cxx_std_17)

target_include_directories(sweetline_3p_nlohmann_json
        INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(SweetLine3p::NlohmannJson ALIAS sweetline_3p_nlohmann_json)
