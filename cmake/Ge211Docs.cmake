set(DOXYGEN_ALIASES                [=[preconditions=#### Preconditions:\n\n]=])
set(DOXYGEN_DISABLE_INDEX          YES)
set(DOXYGEN_EXCLUDE_SYMBOLS        detail)
set(DOXYGEN_GENERATE_TREEVIEW      YES)
set(DOXYGEN_HIDE_FRIEND_COMPOUNDS  YES)
set(DOXYGEN_HIDE_SCOPE_NAMES       YES)
set(DOXYGEN_HTML_DYNAMIC_SECTIONS  YES)
set(DOXYGEN_JAVADOC_AUTOBRIEF      YES)
set(DOXYGEN_LAYOUT_FILE            doxygen/DoxygenLayout.xml)
set(DOXYGEN_OUTPUT_DIRECTORY       doc)
set(DOXYGEN_SHOW_INCLUDE_FILES     NO)
set(DOXYGEN_SOURCE_BROWSER         YES)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE doxygen/FRONTMATTER.md)

doxygen_add_docs(ge211_documentation
        doxygen/FRONTMATTER.md
        include/
        src/)

