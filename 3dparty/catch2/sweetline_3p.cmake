if (TARGET SweetLine3p::Catch2)
    return()
endif ()

add_library(sweetline_3p_catch2
        ${CMAKE_CURRENT_LIST_DIR}/src/catch_amalgamated.cpp
)

target_compile_features(sweetline_3p_catch2 PUBLIC cxx_std_17)
target_compile_definitions(sweetline_3p_catch2 PUBLIC CATCH_AMALGAMATED_CUSTOM_MAIN=ON)

target_include_directories(sweetline_3p_catch2
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include/catch2
)

add_library(SweetLine3p::Catch2 ALIAS sweetline_3p_catch2)
