cmake_minimum_required(VERSION 3.11)
project(utf8-cpp CXX)

add_library(utf8-cpp INTERFACE)

target_include_directories(utf8-cpp SYSTEM INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

SET(HEADER_FILES
        include/utf8.h
        include/utf8/core.h
        include/utf8/checked.h
        include/utf8/unchecked.h)

list(TRANSFORM "${HEADER_FILES}"
        PREPEND "${PROJECT_SOURCE_DIR}"
        OUTPUT_VARIABLE ABSOLUTE_HEADER_FILES)

target_sources(utf8-cpp INTERFACE
        ${ABSOLUTE_HEADER_FILES})

include(GNUInstallDirs)
export(TARGETS utf8-cpp FILE Utf8Cpp.cmake)

