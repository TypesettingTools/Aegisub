#!/bin/env bash

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



    # TODO: event system, 
    #event system: gets each time a save point is made, so that it sees activity, pass in a object that persists the calls, so that the wakatime handler can be stored there, additionally a event string or array of string!!!
    # so that i can pass e.g ["history","save"] to set write=true fro wakatime
    # and third options is the function that is run and then executed with the event, and an object (table) with some additional data!

    #TODO make icons for ass work!
    #icons for .ass .ssa
    #mime type,

    #TODO create flatpak: https://docs.flatpak.org/en/latest/first-build.html



    # TODO : git single file update system, use event system for making regularly git updates, if enabled, for better saving, delete automatically, after some time!
    # git doesn't support single iles, so make directory with only that file ?!