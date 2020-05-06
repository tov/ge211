# Distributed under the OSI-approved BSD 3-Clause License. See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindSDL2_mixer
# -------------
#
# Locate SDL2_mixer library
#
# This module defines:
#
# ::
#
# SDL2_MIXER_LIBRARIES, the name of the library to link against
# SDL2_MIXER_INCLUDE_DIRS, where to find the headers
# SDL2_MIXER_FOUND, if false, do not try to link against
# SDL2_MIXER_VERSION_STRING - human-readable string containing the
# version of SDL2_mixer
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
#
# Mindlessly adapted from FindSDL2_ttf.cmake by Jesse Tov.

include(FindPackageHandleStandardArgs)

find_path(SDL2_MIXER_INCLUDE_DIR
  NAMES SDL_mixer.h
  HINTS
    ENV SDL2MIXERDIR
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

find_library(SDL2_MIXER_LIBRARY
  NAMES SDL2_mixer
  HINTS
    ENV SDL2MIXERDIR
    ENV SDL2DIR
    ${SDL2_DIR}
  PATH_SUFFIXES
    lib
    ${VC_LIB_PATH_SUFFIX})

if(SDL2_MIXER_INCLUDE_DIR AND EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL_mixer.h" SDL2_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MAJOR "${SDL_MIXER_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MINOR "${SDL_MIXER_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_PATCH "${SDL_MIXER_VERSION_PATCH_LINE}")
  set(SDL2_MIXER_VERSION_STRING ${SDL2_MIXER_VERSION_MAJOR}.${SDL2_MIXER_VERSION_MINOR}.${SDL2_MIXER_VERSION_PATCH})
  unset(SDL2_MIXER_VERSION_MAJOR_LINE)
  unset(SDL2_MIXER_VERSION_MINOR_LINE)
  unset(SDL2_MIXER_VERSION_PATCH_LINE)
  unset(SDL2_MIXER_VERSION_MAJOR)
  unset(SDL2_MIXER_VERSION_MINOR)
  unset(SDL2_MIXER_VERSION_PATCH)
endif()

set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

find_package_handle_standard_args(SDL2_mixer
        FOUND_VAR       SDL2_MIXER_FOUND
        REQUIRED_VARS   SDL2_MIXER_LIBRARIES SDL2_MIXER_INCLUDE_DIRS
        VERSION_VAR     SDL2_MIXER_VERSION_STRING)

mark_as_advanced(SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)

