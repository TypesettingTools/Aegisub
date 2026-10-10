cmake_minimum_required(VERSION 3.20)
project(aegisub_clang_tidy_checks LANGUAGES CXX)

find_package(Clang REQUIRED CONFIG)

add_library(aegisub-clang-tidy MODULE
    AegisubTidyModule.cpp
    FilesystemPathCheck.cpp)
target_compile_features(aegisub-clang-tidy PRIVATE cxx_std_20)
target_include_directories(aegisub-clang-tidy PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(aegisub-clang-tidy PRIVATE clangTidy)
set_target_properties(aegisub-clang-tidy PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    PREFIX "")
