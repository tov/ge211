cmake_minimum_required(VERSION 3.13)
project(ge211
        VERSION 2020.2.0
        DESCRIPTION "A student game engine"
        LANGUAGES CXX)

###
### DEPENDENCIES
###

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(3rdparty/utf8-cpp)

find_package(SDL2       REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf   REQUIRED)

###
### MAIN LIBRARY SETUP
###

add_library(ge211
        include/ge211.hxx
        include/ge211_forward.hxx
        include/ge211_time.hxx
        include/ge211_util.hxx
        src/ge211_base.cxx
        src/ge211_color.cxx
        src/ge211_engine.cxx
        src/ge211_event.cxx
        src/ge211_error.cxx
        src/ge211_geometry.cxx
        src/ge211_audio.cxx
        src/ge211_random.cxx
        src/ge211_render.cxx
        src/ge211_resource.cxx
        src/ge211_session.cxx
        src/ge211_sprites.cxx
        src/ge211_window.cxx)

set_target_properties(ge211
    PROPERTIES
        CXX_STANDARD                    14
        CXX_STANDARD_REQUIRED           On
        CXX_EXTENSIONS                  Off
        VERSION                         ${PROJECT_VERSION}
        INTERFACE_INCLUDE_DIRECTORIES   "${SDL2_INCLUDE_DIRS}")

if(MSVC)
    target_compile_options(ge211 PRIVATE /W4)
else(MSVC)
    target_compile_options(ge211 PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

target_include_directories(ge211
    SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${SDL2_INCLUDE_DIRS})

target_include_directories(ge211
    PRIVATE
        include
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS})

target_link_libraries(ge211
    PUBLIC
        ${SDL2_LIBRARIES}
    PRIVATE
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        utf8-cpp)

SET(GE211_RESOURCE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/Resources/"
        "${CMAKE_INSTALL_FULL_DATADIR}/ge211/"
        CACHE STRING
        "Where to search for GE211 resource files.")

target_compile_definitions(ge211 PRIVATE
    -DGE211_RESOURCES="$<JOIN:${GE211_RESOURCE_PATH},\"$<COMMA>\">")

add_subdirectory(examples/installed/)

###
### LIBRARY INSTALLATION
###
include(Ge211Installer)

###
### CLIENT PROGRAM INSTALLER
###
include(Ge211ClientInstaller)
