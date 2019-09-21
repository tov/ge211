#!/bin/sh

set -e

if [ -z "$PREFIX" ]; then
    echo >&2 "$0: \$PREFIX must be set"
    exit 1
fi

if [ $# != 1 ]; then
    echo >&2 "Usage: $0 SRCURL"
    exit 2
fi

srcurl="$1"; shift

retry () {
    "$@" && return

    local attempt
    for attempt in 2 3; do
        echo >&2 "Command failed: $*"
        echo >&2 "Retrying in 5 seconds"
        sleep 5
        "$@" && return
    done

    false
}

file="/tmp/$$-$(basename "$srcurl")"
retry wget --quiet -O "$file" "$srcurl"

dir="/tmp/$(basename "$srcurl").d"
mkdir -p "$dir"
tar -xf "$file" --strip-components=1 -C "$dir"

cd "$dir"
./configure --prefix "$PREFIX"
make -j 2
make install

