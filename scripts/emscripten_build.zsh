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

set -e -onullglob -opipefail
unset REPORTTIME

main () {
    define_variables "$@"

    link_dir $GE211_DIR/example/$OBJ_DIR/ge211 $OBJ_DIR/ge211

    preprocess
    compile $GE211_SRC $USER_SRC
    link $OBJ_FILES
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
            run $CXX -o $obj -c $src $FLAGS $CPPFLAGS $CXXFLAGS
        fi
    done
}

link () {
    if ! nt_all $OUT_JS $@
    then
        mkparent $OUT_JS
        run $CXX -o $OUT_JS $@ $FLAGS $LDFLAGS
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
    OUT_JS=$WEB_DIR/game.js

    FLAGS+=(-fexceptions)
    FLAGS+=(
        -sUSE_SDL=2
        -sUSE_SDL_IMAGE=2
        -sSDL2_IMAGE_FORMATS=\[bmp,gif,jpg,png\]
        -sUSE_SDL_MIXER=2
        -sSDL2_MIXER_FORMATS=\[mp3,ogg,wav\]
        -sUSE_SDL_TTF=2
        -sALLOW_MEMORY_GROWTH=1
    )

    CPPFLAGS+=(-DGE211_RESOURCES=\"/Resources/\")
    if [[ -d Resources ]]
    then
        LDFLAGS+=(--preload-file Resources@/Resources)
    fi
    LDFLAGS+=(--preload-file $GE211_DIR/Resources@/Resources)
    LDFLAGS+=(--use-preload-plugins)

    CPPFLAGS+=(-Isrc)
    CPPFLAGS+=(-I$GE211_INCLUDE_DIR)
    CPPFLAGS+=(-I$GE211_DIR/3rdparty/utf8-cpp/include)
    CPPFLAGS+=(-I$GENERATED_DIR)

    USER_HDR=(src/*.[Hh]*)
    USER_SRC=($@)
    GE211_SRC=($GE211_DIR/src/*.cxx)

    CPPFLAGS+=(-I$PREFIX/include -I$PREFIX/include/SDL2)
    LDFLAGS+=(-L$PREFIX/lib)
}

obj_name () {
    local path=$(print -n $1 | sed 's|src/||g')
    path_take $path 2 src |
        /usr/bin/tr -C /0-9A-Za-z_- _
}

# Trims the path $1 to its final $2 components.
path_take () {
    local path=$1 n=$2; shift 2
    local res=

    while (( n ))
    do
        if [[ $path = . || -z $path ]]
        then
            if [[ -n $1 ]]
            then path=$1; shift
            else break
            fi
        fi

        res=${path:t}${res:+/}$res
        path=${path:h}
        (( n-- ))
    done

    print -nr $res
    return $n
}

format_tag () {
    printf '%s\n' ${@:l} |
        tr -sC '-0-9a-z_\n' _ |
        sort |
        uniq |
        tr -d '\n'
}

link_dir () {
    local src=$1 dst=$2

    if [[ -d $src ]]
    then
        if [[ -e $dst ]]
        then
            local real_src=$(readlink $src)
            local real_dst=$(readlink $dst)
            if [[ $real_src = $real_dst ]]
            then
                return
            else
                rm -R $dst
            fi
        else
            mkparent $dst
        fi

        ln -s $src $dst
    fi
}

preprocess () {
    preprocess_version ge211_version.hxx
    preprocess_main
    preprocess_html
}

preprocess_version () {
    local src=$GE211_INCLUDE_DIR/$1.in
    local obj=$GENERATED_DIR/$1

    if nt_all $obj $src
    then
        return
    fi

    file_message preprocess $obj $_T_CAP_YEL

    mkparent $obj
    sed '
        s/@ge211_VERSION_MAJOR@/2021/
        s/@ge211_VERSION_MINOR@/6/
        s/@ge211_VERSION_PATCH@/0/
    ' $src >$obj
}

preprocess_main () {
    local main_in main_out

    if ! main_in=$(
            grep -lE '\bmain[ \n\t]*\(' $USER_SRC | head -1
        ) || grep -q '#NO_STACK' $main_in
    then
        return
    fi

    main_out=$GENERATED_DIR/${main_in:t}

    if nt_all $main_in $main_out
    then
        return
    fi

    USER_SRC=(
        $main_out
        $(printf '%s\n' $USER_SRC | grep -vF $main_in || true)
    )

    file_message destackify $main_out $_T_CAP_YEL

    local classes='Model|View|Controller'
    sed -E "
        /\b($classes)([ \t]*\([^)]*\)|\{[^}]*\})[ \t]*\./ {
            s//(*new \1\2)./
            n
        }
        s/\b($classes)[ \t]/static \1 /
    " <$main_in >$main_out
}

preprocess_html () {
    local css_in=$GE211_DIR/html/ge211.css
    local css_out=$WEB_DIR/ge211.css
    if ! nt_all $css_out $css_in
    then
        file_message preprocess $css_out $_T_CAP_YEL
        mkparent $css_out
        cp $css_in $css_out
    fi

    local index_in=$GE211_DIR/html/ge211.html.in
    local index_out=$WEB_DIR/index.html
    if ! nt_all $index_out $index_in
    then
        file_message preprocess $index_out $_T_CAP_YEL
        mkparent $index_out
        sed '
            s/@GE211_TITLE@/Ge211 Window/
            s/@GE211_STYLESHEET@/ge211.css/
            s/@GE211_JAVASCRIPT@/game.js/
        ' <$index_in >$index_out
    fi
}

alias tput='tput 2>/dev/null'
_COLS=$(tput cols)
_COLS=$(( _COLS ? _COLS : 80 ))

_T_CAP_0=$(tput sgr0)
_T_CAP_OP=$(tput op)
_T_CAP_ITA=$(tput sitm)
_T_CAP_ROM=$(tput ritm)
_T_CAP_GRN=$(tput setaf 2 || tput setf 2)
_T_CAP_YEL=$(tput setaf 3 || tput setf 3)
_T_CAP_MAG=$(tput setaf 5 || tput setf 5)
_T_CAP_CYA=$(tput setaf 6 || tput setf 6)
_T_CAP_DIM=$(tput dim)
unalias tput

file_message () {
    local t_fresh= t_colon= t_body=

    if tty -s
    then
        t_fresh=$_T_CAP_0$_T_CAP_ITA${3-$_T_CAP_GRN}
        t_colon=$_T_CAP_OP$_T_CAP_ROM
        t_body=${4-$_T_CAP_CYA}
    fi

    local fmt="$t_fresh%s$t_colon:$t_body %s$_T_CAP_0\n"

    printf $fmt $1 "$(path_take $2 2)"
}

nt_all () {
    local out=$1; shift

    local each
    for each
    do
        if ! [[ $out -nt $each ]]
        then
            return 1
        fi
    done

    file_message up-to-date $out
    return 0
}

run () {
    printf '%s\n' "$*"
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

