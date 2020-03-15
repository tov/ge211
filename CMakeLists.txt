cmake_minimum_required(VERSION 3.13)
project(ge211
        VERSION 2020.4.0
        DESCRIPTION "A student game engine"
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

###
### DEPENDENCIES
###

add_subdirectory(3rdparty/utf8-cpp)

find_package(SDL2       REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf   REQUIRED)

###
### MAIN LIBRARY SETUP
###

configure_file(include/ge211_version.hxx.in
               include/ge211_version.hxx)
add_subdirectory(src)

###
### DOCUMENTATION
###

find_package(Doxygen)
if(DOXYGEN_FOUND)
    include(Ge211Docs)
endif()

###
### EXTRAS
###

# Library installation
include(Ge211Installer)

# Client program installer
include(Ge211ClientInstaller)

# Example client program
add_subdirectory(examples/installed/)

