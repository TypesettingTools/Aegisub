#!/bin/sh

# Help buildbot find the doxygen.log
ln -s docs/doxygen/doxygen.log

cd docs/doxygen
sh -x ./gen.sh /usr/www/docs.aegisub.org/source

chmod 0644 /usr/www/docs.aegisub.org/source/*
