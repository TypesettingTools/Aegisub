#!/bin/bash

test -f aegisub/aegisub && test -x aegisub/aegisub || ( exit "Make sure you're in the right dir"; exit 1 )
test -e $1 && ( echo "$1 already exists, will not overwrite."; exit 1 )

echo "Making directory structure..."
mkdir $1 || ( echo "Failed creating directory $1"; exit 1 )
mkdir $1/Contents mkdir $1/Contents/MacOS
mkdir $1/Contents/Resources

echo "Copying files into package..."
cp aegisub/macosx/Info.plist $1/Contents
cp aegisub/aegisub $1/Contents/MacOS
cp aegisub/macosx/*.icns $1/Contents/Resources

echo "Here we should be finding and fixing up libraries"

echo "Done creating $1!"

