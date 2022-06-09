#!/bin/env bash


cd ${MESON_BUILD_ROOT}


if  ! [ -d "DependencyControl" ]; then
    mkdir "DependencyControl"
fi

cd "DependencyControl"

if  ! [ -d "DependencyControl" ]; then
    git clone https://github.com/TypesettingTools/DependencyControl.git &> /dev/null
    cd "DependencyControl"
    git checkout tags/v0.6.4-alpha &> /dev/null
    cd ..

fi

if  ! [ -d "YUtils" ]; then
    git clone https://github.com/TypesettingTools/YUtils.git &> /dev/null

fi

if  ! [ -d "luajson" ]; then
    git clone https://github.com/harningt/luajson.git &> /dev/null
    cd "luajson"
    git checkout tags/1.3.4 &> /dev/null
    cd ..

fi

if  ! [ -d "ffi-experiments" ]; then
    git clone https://github.com/TypesettingTools/ffi-experiments.git &> /dev/null
    cd "ffi-experiments"
    git checkout tags/DMv0.5.0-PTv0.1.6 &> /dev/null
    cd ..

fi








