cmake_minimum_required(VERSION 3.13)
project(ge211_examples CXX)

if(NOT GE211_VENDORED)
    find_package(Ge211 CONFIG)
endif()

set(target fireworks_example)

add_executable(${target} fireworks.cxx)
set_target_properties(${target} PROPERTIES
        CXX_STANDARD            14
        CXX_STANDARD_REQUIRED   On
        CXX_EXTENSIONS          Off)
target_link_libraries(${target} ge211)

if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
else(MSVC)
    target_compile_options(${target} PRIVATE -Wall -pedantic-errors)
endif(MSVC)
