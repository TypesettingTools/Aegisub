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

    sudo meson compile -C build

    # PACK into DEB

    sudo meson compile -C build linux-dependency-control
    sudo meson compile -C build aegisub.desktop
    sudo meson compile -C build ubuntu-deb


    # INSTALL if no second parameter is given
    if  [ -z $2 ]; then
        sudo dpkg -i build/$(ls build/  | grep aegisub_.*deb)
    else

        sudo meson install -C build

    fi

    # TODO: event system, 
    # + fix crash of luafs.so and distribute and test that, python setup test (test .deb in livecd!!!!!!)

    #event system: gets each time a ave point is made, so that it sees activity, pass in a object that persists the calls, so that teh waktaim handler can be stored there, additionally a ebent string or array of string!!!
    # so that i can pass e.g ["histyor","save"] um auch write=true bei wakatime umzusezne, also todoo, better execve!!!!
    # and third options is te function that is run and then executed with the event, the object and some additional data!

    # while changing styles, make ersetze alle available, install also snippet /Vorlage) and icons for ass!



    #icons for .ass .ssa
    #mime type,
    #dependency control compile  in that sh! script, 
    #also probaly lua with luarocks and shared .so ?!
    #fix missing std lua scripts,
    #lua lfs.so missing lua_gettop,
    #create own trasnlation deb(build / generate  them!)