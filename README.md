# GE211 — a game engine for EECS 211

GE211 is a relatively simple game engine for beginning C++ programmers.

## Use

To use the framework, you need to derive your game class from
ge211::Abstract_game; so to get started you may want to go straight
there. Otherwise, all useful definitions are in the ge211 namespace.

## Setup

If you are using GE211 in a course (such as EECS 211 at Northwestern),
your instructor will give you a CMake project that includes files and
configuration for GE211. You shouldn't have to do anything to set it
up. Otherwise, read on.

GE211 is configured and built using CMake. The easiest way to add this
whole respository to your project is to add it as a subdirectory, and
then include it in your `CMakeLists.txt` via the `add_subdirectory`
command:

```CMake
    add_subdirectory(3rdparty/ge211 EXCLUDE_FROM_ALL)
```

The `EXCLUDE_FROM_ALL` flag prevents extra CMake targets from GE211
from appearing in your IDE.

Adding the subdirectory a CMake library target that your program target
can be linked against using the `target_link_libraries` command:

```CMake
    target_link_libraries(my_game ge211)
```

A minimal, complete `CMakeLists.txt` for using GE211 might look
something like this:

```CMake
    cmake_minimum_required(VERSION 3.3)
    project(my_game CXX)

    add_subdirectory(3rdparty/ge211 EXCLUDE_FROM_ALL)

    add_executable(my_game my_game.cpp)
    target_link_libraries(my_game ge211)
    set_property(TARGET my_game PROPERTY CXX_STANDARD 14)
    set_property(TARGET my_game PROPERTY CXX_STANDARD_REQUIRED On)
```
