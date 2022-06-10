#!/bin/env bash



cd ${MESON_BUILD_ROOT}

## TODO get these numbers dynamically

DEB_NAME="aegisub_3.2.2+dpctrl-ubuntu_amd64"

# create deb directroy, later this will be bundled into the deb

mkdir $DEB_NAME
cd $DEB_NAME

# now create the pseudo file system and copy all relevant systems in there

mkdir -p usr/bin

cp ../aegisub usr/bin/

mkdir -p usr/share/aegisub/automation

cp -r ../../automation/autoload/ usr/share/aegisub/automation/

cp -r ../../automation/demos/ usr/share/aegisub/automation/

cp -r ../../automation/include/ usr/share/aegisub/automation/



## TODO dependecy control

mkdir -p usr/lib


## Only including DependencyControl, if it was build!

if  [ -d "../DependencyControl" ]; then

    mkdir -p tmp/DependencyControl/autoload/

    cp ../DependencyControl/DependencyControl/macros/l0.DependencyControl.Toolbox.moon tmp/DependencyControl/autoload/

    mkdir -p tmp/DependencyControl/include

    mkdir -p tmp/DependencyControl/include/l0/

    cp -r ../DependencyControl/DependencyControl/modules/* tmp/DependencyControl/include/l0/

    cp -r ../DependencyControl/luajson/lua/* tmp/DependencyControl/include/

    cp ../DependencyControl/YUtils/src/Yutils.lua tmp/DependencyControl/include/

    cd ../DependencyControl/ffi-experiments/

    if ! command -v "moonc" &> /dev/null
    then
        if command -v "apt-get" &> /dev/null
        then
            echo "Trying to install mooscript automatically"
            sudo apt-get install luarocks lua -y
            sudo luarocks install moonscript

        else
            echo "You don't have moonscript installed, please install it"
            exit 1

        fi
    fi



    make lua

    make all

    cd "../../$DEB_NAME"

    mkdir -p tmp/DependencyControl/include/requireffi/

    cp ../DependencyControl/ffi-experiments/build/requireffi.lua tmp/DependencyControl/include/requireffi/

    mkdir -p tmp/DependencyControl/include/BM/

    cp ../DependencyControl/ffi-experiments/build/BadMutex.lua tmp/DependencyControl/include/BM/

    mkdir -p tmp/DependencyControl/include/DM/

    cp ../DependencyControl/ffi-experiments/build/DownloadManager.lua tmp/DependencyControl/include/DM/

    mkdir -p tmp/DependencyControl/include/PT/

    cp ../DependencyControl/ffi-experiments/build/PreciseTimer.lua tmp/DependencyControl/include/PT/

    cp ../DependencyControl/ffi-experiments/build/lib*.so usr/lib/


    mkdir -p DEBIAN/
    touch DEBIAN/postinst
cat >> DEBIAN/postinst << 'EOF'
#!/bin/sh
set -e

## check if executed correctly:



if [ -z "$SUDO_USER" ]; then

echo  "DO NOT call the installation from the root user, but rather use 'sudo <installation command>' to install it, otheriwse the files can't be moved to the correct folder!!"

rm -r /tmp/DependencyControl/

exit 1

fi

HOME_DIR=$( getent passwd "$SUDO_USER" | cut -d: -f6 )


## now copy the file from tmp to the target!

mkdir -p "$HOME_DIR/.aegisub/automation/"

sudo chown -R $SUDO_USER "$HOME_DIR/.aegisub/"

cp -r /tmp/DependencyControl/*  "$HOME_DIR/.aegisub/automation/"

rm -r /tmp/DependencyControl/


## better not do that here, but why not xD





EOF

chmod 555 DEBIAN/postinst

fi


    # all locally not available libraries are now in subprojects, 
    # they either have to be included, or with options forced to be used form the system, so that
    # 'dpkg-shlibdeps' can read them all

    

    declare -a  ALL_SO=$(find ../subprojects/ -type f -regex ".*\.so")

    for SO in  ${ALL_SO[@]}; do
        cp $SO usr/lib/

    done  



mkdir -p usr/share/applications



if ! [ -f "../packages/aegisub.desktop" ]; then
    cd ../..
    meson -C build aegisub.desktop
    cd "${MESON_BUILD_ROOT}/$DEB_NAME"

fi


cp ../packages/aegisub.desktop usr/share/applications/


## TODO changelog.Debian.gz and copyright, see original deb

# mkdir -p usr/share/doc/aegisub


mkdir -p usr/share/icons/hicolor


declare -a aegisub_logos=('16x16.png' '22x22.png' '24x24.png' '32x32.png' '48x48.png' '64x64.png' 'scalable.svg')

    for logo in ${aegisub_logos[@]}; do

        declare -a parts=(`echo $logo | tr "." " "`)  
        dir=${parts[0]}
        ext=${parts[1]}

        mkdir -p "usr/share/icons/hicolor/$dir/apps/"
        cp "../../packages/desktop/$dir/aegisub.$ext" "usr/share/icons/hicolor/$dir/apps/"

    done




mkdir -p usr/share/man

touch usr/share/man/aegisub-3.2.2


DATE=$(date +"%B %d, %Y")

echo ".TH aegisub-3.2.2 \"$DATE\"" >> usr/share/man/aegisub-3.2.2

cat >> usr/share/man/aegisub-3.2.2 << 'EOF'
.SH NAME
aegisub-3.2.2 \- advanced subtitle editor
.SH SYNOPSIS
.B aegisub-3.2.2
.IR "files" "..."
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
http://docs.aegisub.org/manual/
.SH AUTHOR
aegisub-3.2.2 was written by the Aegisub Project <http://www.aegisub.org/>.
.PP
This manual page was written by Sebastian Reichel <sre@debian.org>
for the Debian project (but may be used by others).
EOF


gzip  usr/share/man/aegisub-3.2.2 

mkdir -p usr/share/pixmaps

cp ../../packages/desktop/pixmaps/aegisub.xpm usr/share/pixmaps/


# now creating the debian control files


touch DEBIAN/control

## TODO use dpkg-gencontrol


cat > DEBIAN/control << 'EOF'

Package: aegisub
Version: 3.2.2+dpctrl-ubuntu
Architecture: amd64
Maintainer: None
Suggests: aegisub-l10n
Section: video
Priority: optional
Description: advanced subtitle editor
 Originally created as tool to make typesetting, particularly in anime
 fansubs, a less painful experience, Aegisub has grown into a fully
 fledged, highly customizable subtitle editor.
 .
 It features a lot of convenient tools to help you with timing, typesetting,
 editing and translating subtitles, as well as a powerful scripting environment
 called Automation (originally mostly intended for creating karaoke effects,
 Automation can now be used much else, including creating macros and various
 other convenient tools).
Original-Maintainer: Aniol Marti <amarti@caliu.cat>
EOF


mkdir debian

touch debian/control

DEPENDECIES=$(dpkg-shlibdeps -O ../aegisub 2>/dev/null) 

# substring, removes the first 15 charcaters

DEPENDECY_LIST=${DEPENDECIES:15}


echo "Depends: $DEPENDECY_LIST"  >> DEBIAN/control


rm debian/control

rm -r debian



# create md5sums

touch DEBIAN/md5sums

md5sum $(find * -type f -not -path 'DEBIAN/*') > DEBIAN/md5sums



cd ..

dpkg-deb --build -Zxz  --root-owner-group $DEB_NAME


rm -r $DEB_NAME



## NOW generate locales!


#TODO generate aegisub-l10n (but also old deb can be used, if the version is adjusted, or even without that?)





