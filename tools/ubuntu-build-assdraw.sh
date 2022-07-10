#!/bin/env bash


cd ${MESON_BUILD_ROOT}



if  ! [ -d "assdraw" ]; then
    # git clone https://github.com/TypesettingTools/DependencyControl.git --branch sqlite
    # git clone https://github.com/Totto16/DependencyControl.git &> /dev/null
    git clone https://github.com/Totto16/assdraw.git &> /dev/null
fi

cd "assdraw"


./autogen.sh

./configure

make

cd ..

# now generate DEB!!

DEB_NAME="assdraw_3.0.0-ubuntu_amd64"


# create deb directroy, later this will be bundled into the deb

mkdir $DEB_NAME
cd $DEB_NAME || exit 5

# now create the pseudo file system and copy all relevant systems in there

mkdir -p usr/bin

cp ../assdraw/src/assdraw usr/bin/

# maybe add libs, are some additional needed?

mkdir -p usr/lib


mkdir -p lib/x86_64-linux-gnu/

mkdir -p DEBIAN/


mkdir -p usr/share/applications



cat > ../packages/assdraw.desktop << 'EOF'
[Desktop Entry]
Version=1.0
Type=Application
Name=Assdraw
Comment=Create and edit vectors for aegisub.
Exec=assdraw
TryExec=assdraw
Icon=/usr/share/assdraw/launcher.png
Terminal=false
Categories=AudioVideo;AudioVideoEditing;GTK;
StartupNotify=false
StartupWMClass=assdraw
EOF


cp ../packages/assdraw.desktop usr/share/applications/

mkdir -p usr/share/assdraw/

cp ../assdraw/src/assdraw usr/share/assdraw/

cp ../assdraw/src/bitmaps/assdraw.png usr/share/assdraw/launcher.png

##changing the permissions of the added files

chmod +r -R usr/


mkdir -p usr/share/man/man1

touch usr/share/man/man1/assdraw.1



## TODO make it properly

DATE=$(date +"%B %d, %Y")

echo ".TH assdraw-3.0.0 \"$DATE\"" >> usr/share/man/man1/assdraw.1

cat >> usr/share/man/man1/assdraw.1 << 'EOF'
.SH NAME
assdraw-3.0.0 \- advanced subtitle editor
.SH SYNOPSIS
.B assdraw
.SH DESCRIPTION
Originally created as tool to make typesetting, particularly in anime
fansubs, a less painful experience, Aegisub has grown into a fully
fledged, highly customizable subtitle editor.
.PP
It features a lot of convenient tools to help you with timing, typesetting,
editing and translating subtitles, as well as a powerful scripting environment
called Automation (originally mostly intended for creating karaoke effects,
Automation can now be used much else, including creating macros and various
other convenient tools).
.SH SEE ALSO
You can find more documentation on the following website:
https://aegi.vmoe.info/docs/3.0
.SH AUTHOR
assdraw-3.9.0 was written by the Aegisub Project <http://www.aegisub.org/>.
.PP
This manual page was written by Sebastian Reichel <sre@debian.org>
for the Debian project (but may be used by others).
EOF


gzip  usr/share/man/man1/assdraw.1


# now creating the debian control files

mkdir -p DEBIAN/
touch DEBIAN/control

## TODO use dpkg-gencontrol

## TODO make it properly
cat > DEBIAN/control << 'EOF'


Package: assdraw
Version: 3.0.0-ubuntu
Architecture: amd64
Maintainer: None
Section: video
Priority: optional
Description: todo
EOF


mkdir debian

touch debian/control

DEPENDECIES=$(dpkg-shlibdeps -O ../assdraw/src/assdraw 2>/dev/null) 

# substring, removes the first 15 charcaters

DEPENDECY_LIST=${DEPENDECIES:15}

# adding luarocks, that is also needed for dependency control!
echo "Depends: $DEPENDECY_LIST"  >> DEBIAN/control

rm debian/control

rm -r debian


# create md5sums

touch DEBIAN/md5sums

md5sum $(find * -type f -not -path 'DEBIAN/*') > DEBIAN/md5sums


cd ..  || exit 5

dpkg-deb --build -Zxz  --root-owner-group $DEB_NAME

rm -r $DEB_NAME
