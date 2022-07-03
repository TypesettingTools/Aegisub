#!/bin/env bash
set -e

ARG=$1

if  [ -z $1 ]; then 

ARG="release"


fi

export CC=gcc
export CXX=g++

buildtype=""
if [ $ARG == "release" ]; then 
    buildtype="release"
elif [ $ARG == "debug" ]; then
    buildtype="debugoptimized"
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


    # INSTALL if no second parameter is given
    if  [ -z $2 ]; then
        sudo dpkg -i build/$(ls build/  | grep aegisub_.*deb) || sudo apt-get -f install
        sudo dpkg -i build/$(ls build/  | grep aegisub-l10n_.*deb)  || sudo apt-get -f install
    else

        sudo meson install -C build

    fi