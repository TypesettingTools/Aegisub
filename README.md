# Aegisub - Advanced Subtitle Editor
## Quick Links
[Homepage](https://aegisub.org/)\
[Binaries](https://aegisub.org/downloads/)\
[Bug tracker](https://github.com/TypesettingTools/Aegisub/issues)\
[Discord](https://discord.com/invite/AZaVyPr)\
IRC: irc://irc.rizon.net/aegisub
<!-- Markdown only allows HTTP links, so the IRC isn't clickable. -->

Support is available on Discord or IRC.

## Building Aegisub
### Windows
#### Windows Prerequisites
1. Visual Studio (Community edition of any recent version is fine, needs the
   Windows SDK included)
2. Python 3
3. Meson
4. CMake

There are a few optional dependencies that must be installed and on your PATH:

1. msgfmt, to build the translations (installing from
   https://mlocati.github.io/articles/gettext-iconv-windows.html seems to be
   the easiest option)
2. InnoSetup, to build the regular installer (iscc.exe on your PATH)
3. 7zip, to build the regular installer (7z.exe on your PATH)
4. Moonscript, to build the regular installer (moonc.exe on your PATH)

All other dependencies are either stored in the repository or are included as
submodules.

#### Building on Windows
Clone Aegisub's repository:
```sh
git clone https://github.com/TypesettingTools/Aegisub
```
From the Visual Studio "x64 Native Tools Command Prompt," build Aegisub:
```sh
meson setup build -Ddefault_library=static && meson compile -C build
```
You should now have a binary at _build/aegisub.exe_.

#### Windows Installer
You can generate the installer after a successful build:
```sh
meson compile win-installer -C build
```
This assumes a working internet connection and installation of the
optional dependencies.

And to generate a portable zip after a successful build:
```sh
meson compile win-portable -C build
```

### macOS
#### macOS Prerequisites
A vaguely recent version of Xcode and its command-line tools is required.

For personal usage, you can use homebrew to install almost all of Aegisub's
dependencies:
```sh
brew install boost cmake ffms2 fftw hunspell libass meson ninja pkg-config zlib
export LDFLAGS="-L/usr/local/opt/icu4c/lib" \
   CPPFLAGS="-I/usr/local/opt/icu4c/include" \
   PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig"
```
When compiling on macOS 15.0 (Sequoia) or later, an additional `export` is
needed to make the color picker work:
```sh
export MACOS_X_DEPLOYMENT_TARGET=14.0
```
When compiling on Apple Silicon, replace `/usr/local` with `/opt/homebrew`.

#### Building for macOS
Once the dependencies are installed, build Aegisub:
```sh
meson setup build && meson compile -C build
```

#### macOS Installer
Create the macOS .dmg file as follows:
```sh
meson setup build_static -Ddefault_library=static \
   -Dbuild_osx_bundle=true -Dlocal_boost=true
meson test -C build_static --verbose
meson compile osx-bundle -C build_static
meson compile osx-build-dmg -C build_static
```

### Linux
#### Linux Prerequisites
Install the dependencies:

Alpine Linux edge:
```sh
apk add alsa-lib-dev boost-dev ffms2-dev fftw-dev fontconfig-dev gtest-dev \
   git hunspell-dev meson uchardet-dev wxwidgets-dev
```
Current stable versions of Alpine Linux don't have ffms2 packaged.

#### Building on Linux
```sh
meson setup build && meson compile -C build
```

## Updating Moonscript
From within the Moonscript repository:
```sh
bin/moon bin/splat.moon -l moonscript moonscript/ > bin/moonscript.lua
````
Open the newly created `bin/moonscript.lua` in a text editor, and make the
following changes:

1. Prepend the final line of the file, `package.preload["moonscript"]()`, with
   a `return`, producing `return package.preload["moonscript"]()`.
2. Within the function at `package.preload['moonscript.base']`, remove
   references to `moon_loader`, `insert_loader`, and `remove_loader`. This
   means removing their declarations, definitions, and entries in the returned
   table.
3. Within the function at `package.preload['moonscript']`, remove the line
   `_with_0.insert_loader()`.

The file is now ready for use, to be placed in `automation/include` within the
Aegisub repo.

## License
All files in this repository are licensed under various GPL-compatible
BSD-style licenses; see LICENCE and the individual source files for more
information. The official Windows and OS X builds are GPLv2 due to including
fftw3.
