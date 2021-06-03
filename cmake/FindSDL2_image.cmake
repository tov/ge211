# Distributed under the OSI-approved BSD 3-Clause License. See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindSDL2_image
# -------------
#
# Locate SDL2_image library
#
# This module defines:
#
# ::
#
# SDL2_IMAGE_LIBRARIES, the name of the library to link against
# SDL2_IMAGE_INCLUDE_DIRS, where to find the headers
# SDL2_IMAGE_FOUND, if false, do not try to link against
# SDL2_IMAGE_VERSION_STRING - human-readable string containing the
# version of SDL2_image
# SDL2_IMAGE_COMPILE_FLAGS, extra flags to pass the compiler
# SDL2_IMAGE_LINK_FLAGS, extra flags to pass the linker
#
#
#
#
#
# $SDL2DIR is an environment variable that would correspond to the
# ./configure --prefix=$SDL2DIR used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

if (EMSCRIPTEN)
    message(STATUS "Using Emscripten port for SDL2_IMAGE")
    set(SDL2_IMAGE_FORMATS
            bmp gif jpg png svg
            CACHE STRING
            "list of image formats to support under emscripten")

    set(SDL2_IMAGE_FOUND 1)
    set(SDL2_IMAGE_VERSION_STRING emcc-port)
    set(SDL2_IMAGE_COMPILE_FLAGS
            -sUSE_SDL_IMAGE=2
            -sSDL2_IMAGE_FORMATS=[$<JOIN:${SDL2_IMAGE_FORMATS},$<COMMA>>])
    set(SDL2_IMAGE_LINK_FLAGS ${SDL2_IMAGE_COMPILE_FLAGS})
    set(SDL2_IMAGE_LIBRARIES)
    set(SDL2_IMAGE_INCLUDE_DIRS)
    return()
endif ()

include(FindPackageHandleStandardArgs)

find_path(SDL2_IMAGE_INCLUDE_DIR
  NAMES SDL_image.h
  HINTS
    ENV SDL2IMAGEDIR
    ENV SDL2DIR
    ${SDL2_DIR}
  # path suffixes to search inside ENV{SDL2DIR}
  PATH_SUFFIXES
    SDL2
    include/SDL2
    include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_IMAGE_LIBRARY
  NAMES SDL2_image
  HINTS
    ENV SDL2IMAGEDIR
    ENV SDL2DIR
    ${SDL2_DIR}
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX})

if(SDL2_IMAGE_INCLUDE_DIR AND EXISTS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_MAJOR "${SDL_IMAGE_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_MINOR "${SDL_IMAGE_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_PATCH "${SDL_IMAGE_VERSION_PATCH_LINE}")
  set(SDL2_IMAGE_VERSION_STRING ${SDL2_IMAGE_VERSION_MAJOR}.${SDL2_IMAGE_VERSION_MINOR}.${SDL2_IMAGE_VERSION_PATCH})
  unset(SDL2_IMAGE_VERSION_MAJOR_LINE)
  unset(SDL2_IMAGE_VERSION_MINOR_LINE)
  unset(SDL2_IMAGE_VERSION_PATCH_LINE)
  unset(SDL2_IMAGE_VERSION_MAJOR)
  unset(SDL2_IMAGE_VERSION_MINOR)
  unset(SDL2_IMAGE_VERSION_PATCH)
endif()

set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR})
set(SDL2_COMPILE_FLAGS)
set(SDL2_LINK_FLAGS)

find_package_handle_standard_args(SDL2_image
        FOUND_VAR       SDL2_IMAGE_FOUND
        REQUIRED_VARS   SDL2_IMAGE_LIBRARIES SDL2_IMAGE_INCLUDE_DIRS
        VERSION_VAR     SDL2_IMAGE_VERSION_STRING)

mark_as_advanced(SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)

