set(DOXYGEN_OUTPUT_DIRECTORY       doc)
set(DOXYGEN_JAVADOC_AUTOBRIEF      YES)
set(DOXYGEN_ALIASES                [=[preconditions=#### Preconditions: ]=])
set(DOXYGEN_HIDE_FRIEND_COMPOUNDS  YES)
set(DOXYGEN_HIDE_SCOPE_NAMES       YES)
set(DOXYGEN_SHOW_INCLUDE_FILES     NO)
set(DOXYGEN_LAYOUT_FILE            doxygen/DoxygenLayout.xml)
set(DOXYGEN_EXCLUDE_SYMBOLS        detail)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE doxygen/FRONTMATTER.md)
set(DOXYGEN_SOURCE_BROWSER         YES)
set(DOXYGEN_HTML_DYNAMIC_SECTIONS  YES)
set(DOXYGEN_DISABLE_INDEX          YES)
set(DOXYGEN_GENERATE_TREEVIEW      YES)
set(DOXYGEN_TAGFILES
        "doxygen/cppreference.xml=http://en.cppreference.com/w/")
set(DOXYGEN_ENABLE_PREPROCESSING   YES)
set(DOXYGEN_PREDEFINED             NO_NOEXCEPT)
set(DOXYGEN_MACRO_EXPANSION        YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF     YES)
set(DOXYGEN_EXPAND_AS_DEFINED      NOEXCEPT NOEXCEPT_)

doxygen_add_docs(ge211_docs
        doxygen/FRONTMATTER.md
        include/
        src/)

