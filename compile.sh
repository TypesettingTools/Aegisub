#!/bin/env bash

set -xe

ARG=$1

if  [ -z $1 ]; then 

ARG="release"


fi

export CC=gcc
export CCX=g++

if [ $ARG == "release" ]; then 
    COMMAND="meson build -Dbuildtype=release"
elif [ $ARG == "debug" ]; then
    COMMAND="meson build -Dbuildtype=debugoptimized"
elif [ $ARG == "runner" ]; then

    ACT=$(which act)

    sudo $ACT -P ubuntu-latest=ghcr.io/catthehacker/ubuntu:act-22.04
    exit 0
else

    echo "NOT supported option: $ARG"
    exit 1

fi


    # CONFIGURE

    bash -c "$COMMAND"

    # COOMPILE

    ## maybe this has to be done:  git config --global --add safe.directory $PWD

    sudo meson compile -C build

    ## run tests, these fail at the moment, for some file permission reasons :(

    # meson test -C build --verbose "gtest main"

    # PACK into DEB

    sudo meson compile -C build linux-dependency-control
    sudo meson compile -C build aegisub.desktop
    sudo meson compile -C build ubuntu-deb


    # INSTALL if no second parameter is given
    if  [ -z $2 ]; then
        sudo dpkg -i build/$(ls build/  | grep aegisub_.*deb)
        sudo dpkg -i build/$(ls build/  | grep aegisub-l10n_.*deb)
    else

        sudo meson install -C build

    fi


    #TODO fix crash of luafs.so and distribute and test that, python setup test (test .deb in livecd!!!!!!)
    #fix missing std lua scripts,
    #lua lfs.so missing lua_gettop,


    # TODO: event system, 
    #event system: gets each time a save point is made, so that it sees activity, pass in a object that persists the calls, so that the wakatime handler can be stored there, additionally a event string or array of string!!!
    # so that i can pass e.g ["history","save"] to set write=true fro wakatime
    # and third options is the function that is run and then executed with the event, and an object (table) with some additional data!

    #TODO make icons for ass work!
    #icons for .ass .ssa
    #mime type,

    #TODO create flatpak: https://docs.flatpak.org/en/latest/first-build.html