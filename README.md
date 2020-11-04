# GE211 — a game engine for CS 211

GE211 is a relatively simple game engine for beginning C++ programmers.

## Use

To use the framework, you need to derive your game class from
[ge211::Abstract_game]; so to get started you may want to go straight
there. Otherwise, all useful definitions are in the [ge211] namespace.

[ge211::Abstract_game]:
    https://tov.github.io/ge211/classge211_1_1_abstract__game.html

[ge211]:
    https://tov.github.io/ge211/namespacege211.html

## Requirements

GE211 depends on the SDL library version 2, along with SDL2 plugin
libraries SDL2_image, SDL2_mixer, and SDL2_ttf. You need to install the
development versions of these packages, as appropriate for your
operating system. They are easy to find on Google, but if you are in a
class, your instructor might have an easier way for you to install them.

## Setup

If you are using GE211 in a course (such as CS 211 at Northwestern),
your instructor will give you a CMake project that includes files and
configuration for GE211. You shouldn't have to do anything to set it
up. Otherwise, read on.

GE211 is configured and built using CMake. The easiest way to add the
library to your project is to add the whole repository as a subdirectory,
and then include it in your `CMakeLists.txt` via the `add_subdirectory`
command:

```CMake
add_subdirectory(3rdparty/ge211 EXCLUDE_FROM_ALL)
```

The `EXCLUDE_FROM_ALL` flag prevents extra CMake targets from GE211
from appearing in your IDE.

Adding the subdirectory creates a CMake library target that your program
target can be linked against using the `target_link_libraries` command:

```CMake
target_link_libraries(my_game ge211)
```

A minimal, complete `CMakeLists.txt` for using GE211 might look
something like this:

```CMake
cmake_minimum_required(VERSION 3.13)
project(my_game CXX)

add_subdirectory(3rdparty/ge211 EXCLUDE_FROM_ALL)

add_executable(my_game my_game.cxx)
target_link_libraries(my_game ge211)
set_target_properties(my_game PROPERTIES
        CXX_STANDARD            14
        CXX_STANDARD_REQUIRED   On
        CXX_EXTENSIONS          Off)
```

To see this in action, see the [ge211-vendored-example] repo.

[ge211-vendored-example]:
    https://github.com/tov/ge211-vendored-example
