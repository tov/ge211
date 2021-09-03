# For building GE211 tests.

file(GLOB ALL_TEST_SRC test*.cxx)
add_executable(test_ge211 ${ALL_TEST_SRC})

target_link_libraries(test_ge211
        PRIVATE doctest_with_main
        PRIVATE ge211)

set_target_properties(test_ge211 PROPERTIES
        CXX_STANDARD            14
        CXX_STANDARD_REQUIRED   On
        CXX_EXTENSIONS          Off)
