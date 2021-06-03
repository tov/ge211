set(GE211_DATADIR_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_INSTALL_FULL_DATADIR}/ge211
        CACHE STRING
        "Where to look for the GE211 data directory")

set(GE211_SYSTEM_RESOURCE_FILES
        LICENSE.sans.ttf.txt
        pop.ogg
        sans.ttf
        CACHE STRING
        "Files we expect to find in the GE211 Resources/ directory")

# Sets ${var} to ${defval} if ${var} is false.
macro(setdef var defval)
    if (${var})
    else ()
        set(${var} ${defval})
    endif ()
endmacro()

# If ${${dirvar}} isn’t the GE211 system data directory then
# check_get211_datadir unsets ${dirvar}.
macro(check_ge211_datadir dirvar)
    foreach (_file IN LISTS ${GE211_SYSTEM_RESOURCE_FILES})
        if (NOT EXISTS ${${dirvar}}/Resources/${_file})
            unset(${dirvar})
            break()
        endif ()
    endforeach ()
endmacro()

# Sets ${outvar} to the GE211 system data directory if found;
# errors otherwise.
function(find_ge211_datadir outvar)
    foreach (dir IN LISTS GE211_DATADIR_PATH)
        check_ge211_datadir(datadir)
        if (dir)
            message(STATUS "find_ge211_datadir: ${dir}")
            set(${outvar} ${dir}/ PARENT_SCOPE)
            return()
        endif ()
    endforeach ()

    message(SEND_ERROR "find_ge211_datadir: Couldn’t find it.")
endfunction()

# Creates an HTML page to host a Web Assembly program.
function(add_html_runtime basename)
    cmake_parse_arguments(PARSE_ARGV 1 ge211
            ""
            "DATADIR;OUTFILE;TITLE;STYLESHEET;JAVASCRIPT;"
            "")

    if (NOT ge211_DATADIR)
        find_ge211_datadir(ge211_DATADIR)
    endif ()

    setdef(ge211_OUTFILE ${basename}.html)
    setdef(ge211_TITLE ${basename})
    setdef(ge211_STYLESHEET ge211.css)
    setdef(ge211_JAVASCRIPT ${basename}.js)

    configure_file(
            ${ge211_DATADIR}/html/ge211.html.in
            ${ge211_OUTFILE}
            @ONLY)
    file(GENERATE
            OUTPUT ${ge211_STYLESHEET}
            INPUT ${ge211_DATADIR}/html/ge211.css)
endfunction(add_html_runtime)
