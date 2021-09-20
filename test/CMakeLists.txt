# For building GE211 tests.

# add_test_suite(TARGET SRCFILE...) creates an executable target named
# TARGET that builds from the listed SRCFILEs and the Doctest run-time.
function(add_test_suite target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target}
            PRIVATE doctest_with_main
            PRIVATE ge211)
    set_target_properties(${target} PROPERTIES
            CXX_STANDARD            14
            CXX_STANDARD_REQUIRED   On
            CXX_EXTENSIONS          Off)
endfunction()

file(GLOB ALL_TEST_SRC test*.cxx)

add_test_suite(test_ge211 ${ALL_TEST_SRC})

foreach(test_suite ${ALL_TEST_SRC})
    string(REGEX REPLACE
            ".*/test_?(.*)[.]cxx" "\\1"
            test_name ${test_suite})
    add_test_suite("test_suite_${test_name}" ${test_suite})
endforeach()
