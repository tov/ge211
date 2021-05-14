cmake_minimum_required(VERSION 3.13)
project(ge211
        VERSION 2021.5.1
        DESCRIPTION "A student game engine"
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

###
### DEPENDENCIES
###

# For Apple Silicon (ARM). (This can go away when CMake is updated to
# know about it.)
if(APPLE AND EXISTS /opt/homebrew)
    message(STATUS "GE211: Adding /opt/homebrew to system search path")
    list(PREPEND CMAKE_SYSTEM_PREFIX_PATH /opt/homebrew)
    list(REMOVE_DUPLICATES CMAKE_SYSTEM_PREFIX_PATH)
endif()

# Prefer Homebrew's /usr/local to Valve's /Library/Frameworks:
set(CMAKE_FIND_FRAMEWORK    LAST)

# Don't link in SDL2's main():
set(SDL2_BUILDING_LIBRARY   1)

add_subdirectory(3rdparty/utf8-cpp EXCLUDE_FROM_ALL)

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
    option(BUILD_DOCUMENTATION
            "Create the HTML-based API documentation (requires Doxygen)"
            On)
    option(DOWNLOAD_STDLIB_TAGS
            "Download tags for linking to standard library docs"
            On)
    set(GIT_PUSH_DOCS_URI
            "https://github.com/tov/ge211.git"
            CACHE STRING "Repo to push documentation to.")
    set(GIT_PUSH_DOCS_BRANCH
            "gh-pages"
            CACHE STRING "Branch to push documentation to.")

    if(BUILD_DOCUMENTATION)
        add_subdirectory(doc)
    endif()
endif()

###
### EXTRAS
###

# Support library installation
include(Ge211Installer)

# Example client program
set(GE211_INHERITED 1)
add_subdirectory(example/)
