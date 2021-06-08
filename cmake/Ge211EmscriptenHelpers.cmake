# Emscripten setup

# Sets ${var} to ${defval} if ${var} is false.
macro(_setdef var defval)
    if (NOT ${var})
        set(${var} ${defval})
    endif ()
endmacro()


# Creates an HTML page to host a Web Assembly program.
function(emscripten_add_target name)
    set(args1 DATADIR OUTDIR OUTFILE TITLE STYLESHEET JAVASCRIPT)
    cmake_parse_arguments(PARSE_ARGV 1 GE211 "" "${args1}" "")

    _setdef(GE211_DATADIR    /usr/local/asmjs-unknown-emscripten/share/ge211)
    _setdef(GE211_OUTDIR     ${CMAKE_CURRENT_BINARY_DIR})
    _setdef(GE211_OUTFILE    ${name}.html)
    _setdef(GE211_TITLE      ${name})
    _setdef(GE211_STYLESHEET ${name}.css)
    _setdef(GE211_JAVASCRIPT ${name}.js)

    cmake_policy(CMP0070 NEW)

    configure_file(
            ${GE211_DATADIR}/html/ge211.html.in
            ${GE211_OUTDIR}/${GE211_OUTFILE}
            @ONLY)

    file(GENERATE
            OUTPUT ${GE211_OUTDIR}/${GE211_STYLESHEET}
            INPUT ${GE211_DATADIR}/html/ge211.css)
endfunction()
