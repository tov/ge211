add_library(utf8-cpp INTERFACE)
set(UTF8_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(utf8-cpp INTERFACE "${UTF8_DIR}")
target_sources(utf8-cpp INTERFACE
        ${UTF8_DIR}/utf8.h
        ${UTF8_DIR}/utf8/core.h
        ${UTF8_DIR}/utf8/checked.h
        ${UTF8_DIR}/utf8/unchecked.h)
