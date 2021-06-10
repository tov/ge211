#!/usr/bin/env zsh

# Builds a GE211 project using Emscripten.
#
# Usage: emscripten_build.zsh -O2
#

# Parameters taken from environment:
: ${EMSDK:=/usr/local/emsdk}
: ${PREFIX:=/usr/local/asmjs-unknown-emscripten}
: ${GE211_DIR:=$PREFIX/src/dot-cs211/lib/ge211}
: ${TAG=}
: ${BUILD_DIR_FORMAT:=build.em%s}

set -e

main () {
    define_variables "$@"

    preprocess ge211_version.hxx
    compile $SRC_FILES
    link $OBJ_FILES
}

obj_name () {
    print -nr "${${${${1%/*}%/src}##*/}:-src}/${1##*/}" |
        tr -C /0-9A-Za-z_- _
}

compile () {
    local src obj

    for src
    do
        obj=$OBJ_DIR/$(obj_name $src).o
        OBJ_FILES+=($obj)

        if ! nt_all $obj $src $USER_HDR
        then
            mkparent $obj
            run $CXX -c $FLAGS $CPPFLAGS $CXXFLAGS $src -o $obj
        fi
    done
}

link () {
    if ! nt_all $OUT_JS $@
    then
        mkparent $OUT_JS
        run $CXX $FLAGS $LDFLAGS $@ -o $OUT_JS
    fi
}

define_variables () {
    FLAGS=()
    CPPFLAGS=()
    CXXFLAGS=()
    LDFLAGS=()

    local opts=()
    while (( $# ))
    do case $1 in
        (-?*) FLAGS+=($1); opts+=($1); shift;;
        (*)   break;;
    esac done
    TAG=${TAG-$(format_tag $opts)}
    unset opts

    if [[ $# = 0 ]]
    then set -- src/*.[Cc]*
    fi

    EM_BIN=$EMSDK/upstream/emscripten
    CXX=$EM_BIN/em++

    BUILD_DIR=$(printf $BUILD_DIR_FORMAT $TAG)
    OBJ_DIR=$BUILD_DIR/obj
    WEB_DIR=$BUILD_DIR/html
    GENERATED_DIR=$BUILD_DIR/generated
    GE211_INCLUDE_DIR=$GE211_DIR/include

    SHARED_OBJ_DIR=$GE211_DIR/example/$BUILD_DIR/obj/ge211
    if [ -d $SHARED_OBJ_DIR ]
    then
        mkdir -p $BUILD_DIR/obj
        ln -sf $SHARED_OBJ_DIR $BUILD_DIR/obj
    fi

    OUT_JS=$WEB_DIR/game.js

    FLAGS+=(--em-config $EMSDK/.emscripten)
    FLAGS+=(-fexceptions)
    FLAGS+=(
        -s USE_SDL=2
        -s USE_SDL_IMAGE=2
        -s SDL2_IMAGE_FORMATS=\[bmp,gif,jpg,png\]
        -s USE_SDL_MIXER=2
        -s SDL2_MIXER_FORMATS=\[mp3,ogg,wav\]
        -s USE_SDL_TTF=2
        -s ALLOW_MEMORY_GROWTH=1
    )

    CPPFLAGS+=(-DGE211_RESOURCES=\"/Resources/\")
    if [ -d Resources ]
    then
        LDFLAGS+=(--preload-file Resources@/Resources)
    fi
    LDFLAGS+=(--preload-file $GE211_DIR/Resources@/Resources)
    LDFLAGS+=(--use-preload-plugins)

    CPPFLAGS+=(-I$GE211_INCLUDE_DIR)
    CPPFLAGS+=(-I$GE211_DIR/3rdparty/utf8-cpp/include)
    CPPFLAGS+=(-I$GENERATED_DIR)

    USER_HDR=(src/*.[Hh]*)
    USER_SRC=($@)
    GE211_SRC=($GE211_DIR/src/*.cxx)
    SRC_FILES=($USER_SRC $GE211_SRC)

    CPPFLAGS+=(-I$PREFIX/include -I$PREFIX/include/SDL2)
    LDFLAGS+=(-L$PREFIX/lib)
}

format_tag () {
    printf '%s\n' |
        tr A-Z a-z |
        tr -sC '-0-9a-z_\n' _ |
        sort |
        uniq |
        tr -d '\n'
}

preprocess () {
    preprocess_version ge211_version.hxx
    preprocess_main
    preprocess_html
}

preprocess_version () {
    local src=$GE211_INCLUDE_DIR/$1.in
    local obj=$GENERATED_DIR/$1

    if ! [[ $obj -nt $src ]]
    then
        mkparent $obj
        sed '
            s/@ge211_VERSION_MAJOR@/2021/
            s/@ge211_VERSION_MINOR@/6/
            s/@ge211_VERSION_PATCH@/0/
        ' $src >$obj
    fi
}

preprocess_main () {
    if ! local main_in=$(grep -lE '\bmain[ \n\t]*\(' $USER_SRC)
    then
        return
    fi

    main_out=$GENERATED_DIR/${main_in##*/}

    if [[ $main_in -nt $main_out ]]
    then
        return
    fi

    USER_SRC=(
        $(printf '%s\n' $USER_SRC | grep -vF $main_in)
        $main_out
    )
    sed -E '
        /(Model|View|Controller)[ \n\t](\([^)]*\)|\{[^}]*\})\./ {
            s//static \1 tmp\1\2; tmp\1./
            n
        }
        s/(Model|View|Controller)[ \n\t]/static \1/
    ' <$main_in >$main_out
}

preprocess_html () {
    local css_in=$GE211_DIR/html/ge211.css
    local css_out=$WEB_DIR/ge211.css
    if ! [ $css_out -nt $css_in ]
    then
        mkparent $css_out
        cp $css_in $css_out
    fi

    local index_in=$GE211_DIR/html/ge211.html.in
    local index_out=$WEB_DIR/index.html
    if ! [ $index_out -nt $index_in ]
    then
        mkparent $index_out
        sed '
            s/@GE211_TITLE@/Ge211 Window/
            s/@GE211_STYLESHEET@/ge211.css/
            s/@GE211_JAVASCRIPT@/game.js/
        ' <$index_in >$index_out
    fi
}

nt_all () {
    local out=$1; shift

    local each
    for each
    do
        if ! [ $out -nt $each ]
        then
            return 1
        fi
    done

    return 0
}

run () {
    printf>&2 '%s\n' "$*"
    $@
}

mkparent () {
    local arg; for arg
    do mkdir -p ${arg%/*}
    done
}


###########
main "$@" #
###########

