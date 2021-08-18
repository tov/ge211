# Distributed under the OSI-approved BSD 3-Clause License. See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindSDL2_ttf
# -------------
#
# Locate SDL2_ttf library
#
# This module defines:
#
# ::
#
# SDL2_TTF_LIBRARIES, the name of the library to link against
# SDL2_TTF_INCLUDE_DIRS, where to find the headers
# SDL2_TTF_FOUND, if false, do not try to link against
# SDL2_TTF_VERSION_STRING - human-readable string containing the
# version of SDL2_ttf
# SDL2_TTF_COMPILE_FLAGS, extra flags to pass the compiler
# SDL2_TTF_LINK_FLAGS, extra flags to pass the linker
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
    message(STATUS "Using Emscripten port for SDL2_TTF")
    set(SDL2_TTF_FOUND 1)
    set(SDL2_TTF_VERSION_STRING "2.0.8")
    set(SDL2_TTF_COMPILE_FLAGS -sUSE_SDL_TTF=2)
    set(SDL2_TTF_LINK_FLAGS ${SDL2_TTF_COMPILE_FLAGS})
    set(SDL2_TTF_LIBRARIES)
    set(SDL2_TTF_INCLUDE_DIRS)
    return()
endif ()

include(FindPackageHandleStandardArgs)

find_path(SDL2_TTF_INCLUDE_DIR
  NAMES SDL_ttf.h
  HINTS
    ENV SDL2TTFDIR
    ENV SDL2DIR
    ${SDL2_DIR}
  # path suffixes to search inside the above
  PATH_SUFFIXES
    SDL2
    include/SDL2
    include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_TTF_LIBRARY
  NAMES SDL2_ttf
  HINTS
    ENV SDL2TTFDIR
    ENV SDL2DIR
    ${SDL2_DIR}
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX})

if(SDL2_TTF_INCLUDE_DIR AND EXISTS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_TTF_INCLUDE_DIR}/SDL_ttf.h" SDL2_TTF_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MAJOR "${SDL_TTF_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_MINOR "${SDL_TTF_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_TTF_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_TTF_VERSION_PATCH "${SDL_TTF_VERSION_PATCH_LINE}")
  set(SDL2_TTF_VERSION_STRING ${SDL2_TTF_VERSION_MAJOR}.${SDL2_TTF_VERSION_MINOR}.${SDL2_TTF_VERSION_PATCH})
  unset(SDL2_TTF_VERSION_MAJOR_LINE)
  unset(SDL2_TTF_VERSION_MINOR_LINE)
  unset(SDL2_TTF_VERSION_PATCH_LINE)
  unset(SDL2_TTF_VERSION_MAJOR)
  unset(SDL2_TTF_VERSION_MINOR)
  unset(SDL2_TTF_VERSION_PATCH)
endif()

set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR})
set(SDL2_TTF_COMPILE_FLAGS)
set(SDL2_TTF_LINK_FLAGS)

find_package_handle_standard_args(SDL2_ttf
        FOUND_VAR       SDL2_TTF_FOUND
        REQUIRED_VARS   SDL2_TTF_LIBRARIES SDL2_TTF_INCLUDE_DIRS
        VERSION_VAR     SDL2_TTF_VERSION_STRING)

mark_as_advanced(SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)

