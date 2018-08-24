cmake_minimum_required(VERSION 3.3)
project(utf8-cpp CXX)

add_library(utf8-cpp INTERFACE)
set(UTF8_DIR "${PROJECT_SOURCE_DIR}")
target_include_directories(utf8-cpp INTERFACE "${UTF8_DIR}")
target_sources(utf8-cpp INTERFACE
        ${UTF8_DIR}/utf8.h
        ${UTF8_DIR}/utf8/core.h
        ${UTF8_DIR}/utf8/checked.h
        ${UTF8_DIR}/utf8/unchecked.h)
