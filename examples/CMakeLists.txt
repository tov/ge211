cmake_minimum_required(VERSION 3.3)
project(ge211_examples CXX)

add_executable(fireworks fireworks.cpp)
target_link_libraries(fireworks ge211)
set_property(TARGET fireworks PROPERTY CXX_STANDARD 14)
set_property(TARGET fireworks PROPERTY CXX_STANDARD_REQUIRED On)