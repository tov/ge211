set(STDLIB_DOC_ROOT  "http://en.cppreference.com/w/")
set(STDLIB_TAG_FILE  "cppreference.xml")
set(STDLIB_TAG_URL   "http://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml")
set(STDLIB_TAG_PATH  "${CMAKE_CURRENT_BINARY_DIR}/src/${STDLIB_TAG_FILE}")

set(readme_md        ${CMAKE_CURRENT_SOURCE_DIR}/../README.md)
set(frontmatter_md   ${CMAKE_CURRENT_BINARY_DIR}/FRONTMATTER.md)

# Convert Markdown format:
SET(sed_command sed -E "/^\\[.*\\]: *$$/,/^ *$$/d\;s/[][]//g")
execute_process(COMMAND ${sed_command}
        INPUT_FILE   ${readme_md}
        OUTPUT_FILE  ${frontmatter_md})

set(DOXYGEN_OUTPUT_DIRECTORY       doc)
set(DOXYGEN_JAVADOC_AUTOBRIEF      YES)
set(DOXYGEN_HIDE_FRIEND_COMPOUNDS  YES)
set(DOXYGEN_HIDE_SCOPE_NAMES       YES)
set(DOXYGEN_SHOW_INCLUDE_FILES     NO)
set(DOXYGEN_LAYOUT_FILE            DoxygenLayout.xml)
set(DOXYGEN_HTML_EXTRA_STYLESHEET  extradoxy.css)
set(DOXYGEN_EXCLUDE_SYMBOLS        detail)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${frontmatter_md})
set(DOXYGEN_SOURCE_BROWSER         YES)
set(DOXYGEN_HTML_DYNAMIC_SECTIONS  YES)
set(DOXYGEN_DISABLE_INDEX          YES)
set(DOXYGEN_GENERATE_TREEVIEW      YES)
set(DOXYGEN_ENABLE_PREPROCESSING   YES)
set(DOXYGEN_PREDEFINED             GE211_DOXYGEN)
set(DOXYGEN_MACRO_EXPANSION        YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF     YES)
set(DOXYGEN_EXPAND_AS_DEFINED      NOEXCEPT NOEXCEPT_ IF_DOXYGEN IF_COMPILER)

set(DOXYGEN_ALIASES
        "ownership=#### Ownership"
        "example=#### Example"
        "examples=#### Examples"
        "precondition=#### Precondition"
        "preconditions=#### Preconditions")

if (DOWNLOAD_STDLIB_TAGS)
    include(ExternalProject)
    ExternalProject_Add(Fetch-StdLib-Tags
            PREFIX                ${CMAKE_CURRENT_BINARY_DIR}
            URL                   ${STDLIB_TAG_URL}
            DOWNLOAD_NAME         ${STDLIB_TAG_FILE}
            DOWNLOAD_NO_EXTRACT   True
            TIMEOUT               10
            DOWNLOAD_NO_PROGRESS  True
            CONFIGURE_COMMAND     ""
            BUILD_COMMAND         ""
            INSTALL_COMMAND       "")
    set(DOXYGEN_TAGFILES "${STDLIB_TAG_PATH}=${STDLIB_DOC_ROOT}")
endif ()

file(GLOB_RECURSE ge211_srcs
        LIST_DIRECTORIES false
        ../include/*.hxx
        ../src/*.cxx)
doxygen_add_docs(Build-GE211-Docs
        ${ge211_srcs}
        ${frontmatter_md}
        USE_STAMP_FILE)

if (DOWNLOAD_STDLIB_TAGS)
    add_dependencies(Build-GE211-Docs Fetch-StdLib-Tags)
endif()

add_custom_target(Upload-GE211-Docs
        COMMAND ghp-import -n doc/html
        COMMAND git push --force ${GIT_PUSH_DOCS_URI} ${GIT_PUSH_DOCS_BRANCH}
        COMMENT "Publishing to GitHub Pages")
add_dependencies(Upload-GE211-Docs Build-GE211-Docs)

