#!/bin/env bash

cd ${MESON_BUILD_ROOT} || exit 5

if ! [ -d "DependencyControl" ]; then
    mkdir "DependencyControl"
fi

cd "DependencyControl" || exit 5

if ! [ -d "DependencyControl" ]; then
    # git clone https://github.com/TypesettingTools/DependencyControl.git --branch sqlite
    # git clone https://github.com/Totto16/DependencyControl.git &> /dev/null
    git clone https://github.com/TypesettingTools/DependencyControl.git &>/dev/null
    cd "DependencyControl" || exit 5
    git checkout tags/v0.6.4-alpha &>/dev/null
    cd ..
fi

if ! [ -d "YUtils" ]; then
    git clone https://github.com/TypesettingTools/YUtils.git &>/dev/null
fi

if ! [ -d "luajson" ]; then
    git clone https://github.com/harningt/luajson.git &>/dev/null
fi

if ! [ -d "ffi-experiments" ]; then
    # a fork to get a script working, that doesn't work on ubuntu (where /bin/sh doesn't support [[)
    git clone https://github.com/Totto16/ffi-experiments.git &>/dev/null
fi

cd "ffi-experiments" || exit 5
# weird but necessary, since executning in root :(,
#TODO FIX THAT
sudo chmod 777 -R .
if ! command -v "moonc" &>/dev/null; then
    if command -v "apt-get" &>/dev/null; then
        echo "Trying to install mooscript automatically"
        sudo apt-get install luarocks lua -y
        sudo luarocks install moonscript

    else
        echo "You don't have moonscript installed, please install it"
        exit 1

    fi
fi

make -B lua

make -B all

cd ..
