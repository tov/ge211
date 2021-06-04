# Emscripten setup

function(_show_var var)
    message(STATUS "${ARGN} ${var}: ${${var}}")
endfunction()

set(GE211_PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}
    CACHE STRING
    "Where to find the GE211 sources")

# Sets ${outvar} to the GE211 system data directory if found;
# errors otherwise.
function(_find_ge211_datadir outvar)
    foreach (dir IN ITEMS
            ${GE211_PROJECT_SOURCE_DIR}
            ${CMAKE_INSTALL_FULL_DATADIR}/ge211)

        if (EXISTS ${dir}/html/ge211.html.in)
            message(STATUS "Found GE211 DATADIR: ${dir}")
            set(${outvar} ${dir}/ PARENT_SCOPE)
            return()
        else ()
            message(DEBUG "Skipped: ${dir}")
        endif ()
    endforeach ()

    message(FATAL_ERROR "GE211 DATADIR not found")
endfunction()


# Sets ${var} to ${defval} if ${var} is false.
macro(_setdef var defval)
    if (NOT ${var})
        set(${var} ${defval})
    endif ()
endmacro()


# Creates an HTML page to host a Web Assembly program.
function(emscripten_add_target name)
    set(args1 DATADIR OUTFILE TITLE STYLESHEET JAVASCRIPT)
    cmake_parse_arguments(PARSE_ARGV 1 GE211 "" "${args1}" "")

    if (NOT GE211_DATADIR)
        _find_ge211_datadir(GE211_DATADIR)
    endif ()

    target_preload_resources(${name} PUBLIC)

    _setdef(GE211_OUTFILE        ${name}.html)
    _setdef(GE211_TITLE          ${name})
    _setdef(GE211_STYLESHEET     ${name}.css)
    _setdef(GE211_JAVASCRIPT     ${name}.js)

    configure_file(
            ${GE211_DATADIR}/html/ge211.html.in
            ${GE211_OUTFILE}
            @ONLY)

    file(GENERATE
            OUTPUT ${GE211_STYLESHEET}
            INPUT ${GE211_DATADIR}/html/ge211.css)
endfunction()

function(target_preload_resources name)
    set(args0 PUBLIC PRIVATE INTERFACE REQUIRED)
    set(args1 PATH)
    cmake_parse_arguments(PARSE_ARGV 3 pa "${args0}" "${args1}" "")

    if (pa_INTERFACE)
        set(scope INTERFACE)
    elseif (pa_PUBLIC)
        set(scope PUBLIC)
    else ()
        set(scope PRIVATE)
    endif ()

    if (NOT pa_PATH)
        set(pa_PATH ${PROJECT_SOURCE_DIR}/Resources)
    endif ()

    if (EXISTS ${path})
        target_link_options(${name}
                ${scope}
                "SHELL:--preload-file ${pa_PATH}@/Resources")
    elseif (pa_REQUIRED)
        message(SEND_ERROR
                "Could not preload resources from ${path} for target ${name}")
    endif ()
endfunction()
