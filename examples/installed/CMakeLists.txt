cmake_minimum_required(VERSION 3.13)
project(ge211_examples CXX)

find_package(Ge211 CONFIG QUIET)

add_executable(fireworks fireworks.cxx)
target_link_libraries(fireworks ge211)
set_target_properties(fireworks PROPERTIES
        CXX_STANDARD            14
        CXX_STANDARD_REQUIRED   On
        CXX_EXTENSIONS          OFF)

if(MSVC)
    target_compile_options(fireworks PRIVATE /W4)
else(MSVC)
    target_compile_options(fireworks PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

include(GNUInstallDirs)
install(TARGETS fireworks
        RUNTIME DESTINATION bin)
