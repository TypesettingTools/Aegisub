#!/bin/sh
set -e

echo Running autoreconf...
autoreconf -ivf "$@"

echo Now run ./configure and then make to build Aegisub
