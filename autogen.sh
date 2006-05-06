#!/bin/sh

aclocal && autoheader && automake -a && autoconf && echo "'configure' script created successfully."

