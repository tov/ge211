find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    return()
endif()

option(BUILD_DOCUMENTATION
        "Create the HTML-based API documentation (requires Doxygen)"
        On)
option(DOWNLOAD_STDLIB_TAGS
        "Download tags for linking to standard library docs"
        On)
set(GIT_PUSH_DOCS_URI
        "https://github.com/tov/ge211.git"
        CACHE STRING "Repo to push documentation to.")
set(GIT_PUSH_DOCS_BRANCH
        "gh-pages"
        CACHE STRING "Branch to push documentation to.")

if(BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()
