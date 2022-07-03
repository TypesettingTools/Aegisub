#!/bin/env bash



cd ${MESON_BUILD_ROOT} || exit 5

## TODO get these numbers dynamically

DEB_NAME="aegisub_3.2.2+dpctrl-ubuntu_amd64"

# create deb directroy, later this will be bundled into the deb

mkdir $DEB_NAME
cd $DEB_NAME || exit 5

# now create the pseudo file system and copy all relevant systems in there

mkdir -p usr/bin

cp ../aegisub usr/bin/

mkdir -p usr/local/share/aegisub/automation

cp -r ../../automation/autoload/ usr/local/share/aegisub/automation/


cp -r ../../automation/demos/ usr/local/share/aegisub/automation/

cp -r ../../automation/include/ usr/local/share/aegisub/automation/


# remove the meson build files, that where copied over

rm usr/local/share/aegisub/automation/include/meson.build

rm usr/local/share/aegisub/automation/include/aegisub/meson.build




mkdir -p usr/lib

## since local bioost is used, the libraries have to be used!!

mkdir -p lib/x86_64-linux-gnu/

declare -a modules=('chrono' 'thread' 'filesystem' 'locale' 'program_options')
boost_version='1.79.0'
    for module in  ${modules[@]}; do
        cp "../subprojects/boost_1_79_0/libs/$module/libboost_$module.$boost_version.so" "usr/lib/"
    done  


## Only including DependencyControl, if it was build!

if  [ -d "../DependencyControl" ]; then

    mkdir -p tmp/DependencyControl/autoload/

    cp ../DependencyControl/DependencyControl/macros/l0.DependencyControl.Toolbox.moon tmp/DependencyControl/autoload/

    mkdir -p tmp/DependencyControl/include

    mkdir -p tmp/DependencyControl/include/l0/

    cp -r ../DependencyControl/DependencyControl/modules/* tmp/DependencyControl/include/l0/

    cp -r ../DependencyControl/luajson/lua/* tmp/DependencyControl/include/

    cp ../DependencyControl/YUtils/src/Yutils.lua tmp/DependencyControl/include/



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

     #post or preinst ???
    touch DEBIAN/postinst
cat >> DEBIAN/postinst << 'EOF'
#!/bin/bash
set -e

## check if executed correctly:



if [ -z "$SUDO_USER" ]; then

echo  "DO NOT call the installation from the root user, but rather use 'sudo <installation command>' to install it, otheriwse the files can't be moved to the correct folder!!"
exit 5
rm -r /tmp/DependencyControl/

fi

HOME_DIR=$(getent passwd "$SUDO_USER" | cut -d: -f6 )


## now copy the file from tmp to the target!
## better not do that here, but why not xD

mkdir -p "$HOME_DIR/.aegisub/automation/"

sudo chown -R "$SUDO_USER" "$HOME_DIR/.aegisub/"

if [ -d "/tmp/DependencyControl" ]; then

    cp -r /tmp/DependencyControl/*  "$HOME_DIR/.aegisub/automation/"

    rm -r /tmp/DependencyControl/
fi


## generate mimetypes: see https://wiki.ubuntu.com.cn/UbuntuHelp:AddingMimeTypes

if [ -d "/tmp/Aegisub" ]; then

    EXISTS_SSA=$(grep 'text/x-ssa' /etc/mime.types || echo "")
    EXISTS_ASS=$(grep 'text/x-ass' /etc/mime.types || echo "")

    if [ -z "$EXISTS_SSA" ]; then 
        echo "text/x-ssa ssa"  >> /etc/mime.types
    fi

    if [ -z "$EXISTS_ASS" ]; then 
        echo "text/x-ass ass"  >> /etc/mime.types
    fi


    mkdir -p "/usr/share/icons/hicolor/scalable/mimetypes/"

    cp "/tmp/Aegisub/scaleable.svg" "/usr/share/icons/hicolor/scalable/mimetypes/text-x-ssa.svg"
    cp "/tmp/Aegisub/scaleable.svg" "/usr/share/icons/hicolor/scalable/mimetypes/text-x-ass.svg"


    EXISTS_MIME_TYPE_ASS=$(grep -rn 'text/x-ass' /usr/share/mime/packages/ || echo "")
    EXISTS_MIME_TYPE_SSA=$(grep -rn 'text/x-ssa' /usr/share/mime/packages/ || echo "")

    if [ -z "$EXISTS_MIME_TYPE_ASS" ] || [ -z "$EXISTS_MIME_TYPE_SSA" ]; then 
        
        # should exist, but fdor safty
        mkdir -p "/usr/share/mime/packages/"

        touch "/usr/share/mime/packages/text.xml"
        echo '<?xml version="1.0" encoding="UTF-8"?>'                                       >> "/usr/share/mime/packages/text.xml"
        echo '<mime-info xmlns='\''http://www.freedesktop.org/standards/shared-mime-info'\''>'  >> "/usr/share/mime/packages/text.xml"
        echo '        <mime-type type="text/x-ass">'                                        >> "/usr/share/mime/packages/text.xml"
        echo '                <glob pattern="*.ass"/>'                                      >> "/usr/share/mime/packages/text.xml"
        echo '                <magic priority="100">'                                       >> "/usr/share/mime/packages/text.xml"
        echo '                    <match value="0xEF" type="byte" offset="0"/>'             >> "/usr/share/mime/packages/text.xml"
        echo '                    <match value="0xBB" type="byte" offset="1"/>'             >> "/usr/share/mime/packages/text.xml"
        echo '                    <match value="0xBF" type="byte" offset="2"/>'             >> "/usr/share/mime/packages/text.xml"
        echo '                    <match value="[Script Info]" type="string" offset="3"/>'  >> "/usr/share/mime/packages/text.xml"
        echo '                </magic>'                                                     >> "/usr/share/mime/packages/text.xml"
        echo '        </mime-type>'                                                         >> "/usr/share/mime/packages/text.xml"
        echo '        <mime-type type="text/x-ssa">'                                        >> "/usr/share/mime/packages/text.xml"
        echo '                <glob pattern="*.ssa"/>'                                      >> "/usr/share/mime/packages/text.xml"
        echo '        </mime-type>'                                                         >> "/usr/share/mime/packages/text.xml"
        echo '</mime-info>'                                                                 >> "/usr/share/mime/packages/text.xml"

        sudo update-mime-database /usr/share/mime
        sudo update-icon-caches /usr/share/icons/*
        sudo gtk-update-icon-cache /usr/share/icons/hicolor -f
    fi
        
    rm -r /tmp/Aegisub/

    sudo chmod -R 776 /usr/local/share/aegisub
    sudo chown -R "root:$SUDO_USER" "/usr/local/share/aegisub"

fi


if ! command -v "luarocks" &> /dev/null
    then
    echo  "luarocks is missing, the installer should install that, but if not, then you have to to that manually: 'sudo apt-get install luarocks'"
exit 6
fi

## for sqlite version of dependency-control
# sudo luarocks install lsqlite3    > /dev/null
sudo luarocks install moonscript  > /dev/null



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
    cd ../..  || exit 5
    meson -C build aegisub.desktop
    cd "${MESON_BUILD_ROOT}/$DEB_NAME"  || exit 5

fi


cp ../packages/aegisub.desktop usr/share/applications/


## TODO changelog.Debian.gz and copyright, see original deb

# mkdir -p usr/share/doc/aegisub


mkdir -p usr/share/icons/hicolor
mkdir -p usr/share/icons/Humanity/mimes


declare -a aegisub_logos=('16x16.png' '22x22.png' '24x24.png' '32x32.png' '48x48.png' '64x64.png' 'scalable.svg')

    for logo in ${aegisub_logos[@]}; do

        declare -a parts=(`echo $logo | tr "." " "`)  
        dir=${parts[0]}
        ext=${parts[1]}
        size=${dir:0:2}

        mkdir -p "usr/share/icons/hicolor/$dir/apps/"
        cp "../../packages/desktop/$dir/aegisub.$ext" "usr/share/icons/hicolor/$dir/apps/"

        # if it's not the scalable file, but rather the 16x16 etc.
        if ! [ "$size" = "sc" ]; then 
        ## TODO: better icons suppoort, and this doesn't even work, maybe run "sudo gtk-update-icon-cache /usr/share/icons/Humanity" afterwards
            mkdir -p "usr/share/icons/Humanity/mimes/$size"
            cp "../../packages/desktop/scalable/aegisub.svg" "usr/share/icons/Humanity/mimes/$size/text-x-ass.svg"
            cp "../../packages/desktop/scalable/aegisub.svg" "usr/share/icons/Humanity/mimes/$size/text-x-ssa.svg"
        else 
            mkdir -p tmp/Aegisub/
            cp "../../packages/desktop/scalable/aegisub.svg" "tmp/Aegisub/scaleable.svg"
        fi
    done




##changing the permissions of the added files

chmod +r -R usr/


mkdir -p usr/share/man/man1

touch usr/share/man/man1/aegisub.1


DATE=$(date +"%B %d, %Y")

echo ".TH aegisub-3.2.2 \"$DATE\"" >> usr/share/man/man1/aegisub.1

cat >> usr/share/man/man1/aegisub.1 << 'EOF'
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
https://aegi.vmoe.info/docs/3.0
.SH AUTHOR
aegisub-3.2.2 was written by the Aegisub Project <http://www.aegisub.org/>.
.PP
This manual page was written by Sebastian Reichel <sre@debian.org>
for the Debian project (but may be used by others).
EOF


gzip  usr/share/man/man1/aegisub.1

mkdir -p usr/share/pixmaps

cp ../../packages/desktop/pixmaps/aegisub.xpm usr/share/pixmaps/



# now creating the debian control files

mkdir -p DEBIAN/
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

# adding luarocks, that is also needed for dependency control!
echo "Depends: $DEPENDECY_LIST ,luarocks (>= 3.8.0+dfsg1-1)"  >> DEBIAN/control

rm debian/control

rm -r debian



# create md5sums

touch DEBIAN/md5sums

md5sum $(find * -type f -not -path 'DEBIAN/*') > DEBIAN/md5sums



cd ..  || exit 5

dpkg-deb --build -Zxz  --root-owner-group $DEB_NAME

rm -r $DEB_NAME



## NOW generate locales!


LOCALE_DEB_NAME="aegisub-l10n_3.2.2+dpctrl-ubuntu_amd64"

# create deb directroy, later this will be bundled into the deb

mkdir $LOCALE_DEB_NAME
cd $LOCALE_DEB_NAME || exit 5


# now create the pseudo file system and copy all relevant systems in there

mkdir -p usr/share/locale


cp -r ../po/* usr/share/locale/




# now creating the debian control files

mkdir -p DEBIAN/
touch DEBIAN/control

## TODO use dpkg-gencontrol


cat > DEBIAN/control << 'EOF'
Package: aegisub-l10n
Source: aegisub
Version: 3.2.2+dpctrl-ubuntu
Architecture: all
Maintainer: None
Installed-Size: 3230
Depends: aegisub (>= 3.2.2+dpctrl-ubuntu)
Section: localization
Priority: optional
Original-Maintainer: Aniol Marti <amarti@caliu.cat>
Description: aegisub language packages
 Originally created as tool to make typesetting, particularly in anime
 fansubs, a less painful experience, Aegisub has grown into a fully
 fledged, highly customizable subtitle editor.
 .
 It features a lot of convenient tools to help you with timing, typesetting,
 editing and translating subtitles, as well as a powerful scripting environment
 called Automation (originally mostly intended for creating karaoke effects,
 Automation can now be used much else, including creating macros and various
 other convenient tools).
 .
 This package contains language packages for the following languages:
 ca, cs, da, de, el, es, fa, fi, fr, hu, id, it, ja, ko, pl, pt_BR, pt_PT, ru,
 sr_RS, vi, zh_CN, zh_TW
EOF


# create md5sums

touch DEBIAN/md5sums

md5sum $(find * -type f -not -path 'DEBIAN/*') > DEBIAN/md5sums



cd ..  || exit 5

dpkg-deb --build -Zxz  --root-owner-group $LOCALE_DEB_NAME

rm -r $LOCALE_DEB_NAME
