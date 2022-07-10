#!/bin/env bash
set -e

ARG=$1

if  [ -z $1 ]; then 

ARG="release"


fi

export CC=gcc
export CXX=g++

buildtype=""
DEBUG="false"
if [ $ARG == "release" ]; then 
    buildtype="release"
elif [ $ARG == "debug" ]; then
    buildtype="debugoptimized"
elif [ $ARG == "clean" ]; then
    sudo rm -rf build/
    exit 0
elif [ $ARG == "format" ]; then


    # TODO, use clang-tidy
    # sudo find src/ -name '*.h' -o -name '*.cpp' -exec clang-tidy -checks='*'  -fix  {}  -- -std=c++14  -I${vcpkgRoot}/packages/ -I/{vcpkgRoot}/packages/wxwidgets_x64-linux/include/ -I{vcpkgRoot}/packages/wxwidgets_x64-linux/include/wx-3.1/;

    sudo find src/ -name '*.h' -o -name '*.c' -o -name '*.cpp'  -name '*.hpp' | xargs clang-format -style=file -i 
    sudo find libaegisub/ -name '*.h' -o -name '*.c' -o -name '*.cpp'  -name '*.hpp' | xargs clang-format -style=file -i 

    exit 0
elif [ $ARG == "dev" ]; then
    buildtype="debugoptimized"
    DEBUG="true"
elif [ $ARG == "runner" ]; then

    ACT=$(which act)

    sudo $ACT -P ubuntu-latest=ghcr.io/catthehacker/ubuntu:act-22.04
    exit 0
else

    echo "NOT supported option: $ARG"
    exit 1

fi


    # CONFIGURE

    bash -c "meson build -Dbuildtype=$buildtype -Dlocal_boost=true -Dwx_version=3.1.7"

    if [ $DEBUG == "true" ]; then
        nodemon --watch src/ -e .cpp,.h --exec "sudo meson compile -C build && ./build/aegisub || exit 1"
        exit 0
    fi


    # COMPILE

    ## maybe this has to be done:  git config --global --add safe.directory $PWD

    sudo meson compile -C build

    ## run tests, these fail at the moment, for some file permission reasons :(

    # meson test -C build --verbose "gtest main"

    # PACK into DEB

    if [ ! -f "build/aegisub" ]; then
        echo "Failed to build aegiusb. Aborting"
        exit 4
    fi

    sudo meson compile -C build linux-dependency-control
    sudo meson compile -C build aegisub.desktop
    sudo meson compile -C build ubuntu-deb
    sudo meson compile -C build ubuntu.assdraw-deb


    # INSTALL if no second parameter is given
    if  [ -z $2 ]; then
        sudo dpkg -i build/$(ls build/  | grep aegisub_.*deb) || sudo apt-get -f install
        sudo dpkg -i build/$(ls build/  | grep aegisub-l10n_.*deb)  || sudo apt-get -f install
        sudo dpkg -i build/$(ls build/  | grep assdraw.*deb)  || sudo apt-get -f install
    else

        sudo meson install -C build

    fi