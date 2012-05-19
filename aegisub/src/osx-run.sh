#!/bin/sh
case "$1" in
	"build")
		cd ..
		make
		make osx-bundle
		cd src
		open ../*.app
	;;
	"build-gdb")
		cd ..
		make
		make osx-bundle
		cd src
		gdb ../*.app/Contents/MacOS/aegisub
	;;
	"gdb")
		gdb ../*.app/Contents/MacOS/aegisub
	;;
	*)
		open ../*.app
	;;
esac

