#!/bin/sh
if meson compile -C build ; then
    ./build/platformer
fi
