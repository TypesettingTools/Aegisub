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

Prerequisites:

1. Visual Studio (Community edition of any recent version is fine, needs the Windows SDK included)
2. Python 3
3. Meson
4. CMake

There are a few optional dependencies that must be installed and on your PATH:

1. msgfmt, to build the translations (installing from https://mlocati.github.io/articles/gettext-iconv-windows.html seems to be the easiest option)
2. InnoSetup, to build the regular installer (iscc.exe on your PATH)
3. 7zip, to build the regular installer (7z.exe on your PATH)
4. Moonscript, to build the regular installer (moonc.exe on your PATH)

All other dependencies are either stored in the repository or are included as submodules.

Building:

1. Clone Aegisub's repository: `git clone https://github.com/TypesettingTools/Aegisub.git`
2. From the Visual Studio "x64 Native Tools Command Prompt", generate the build directory: `meson build -Ddefault_library=static` (if building for release, add `--buildtype=release`)
3. Build with `cd build` and `ninja`

You should now have a binary: `aegisub.exe`.

Installer:

You can generate the installer with `ninja win-installer` after a successful build. This assumes a working internet connection and installation of the optional dependencies.

You can generate the portable zip with `ninja win-portable` after a successful build.

### OS X

A vaguely recent version of Xcode and the corresponding command-line tools are required.

For personal usage, you can use pip and homebrew to install almost all of Aegisub's dependencies:

    pip3 install meson      # or brew install meson if you installed Python via brew
    brew install cmake ninja pkg-config  libass boost zlib ffms2 fftw hunspell uchardet
    export LDFLAGS="-L/usr/local/opt/icu4c/lib"
    export CPPFLAGS="-I/usr/local/opt/icu4c/include"
    export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig"

When compiling on Apple Silicon, replace `/usr/local` with `/opt/homebrew`.
When compiling on macOS 15.0 (Sequoia) or later, you may also want to `export MACOS_X_DEPLOYMENT_TARGET=14.0` to make the color picker work.

Once the dependencies are installed, build Aegisub with `meson build && meson compile -C build`.

#### Build dmg

```bash
meson build_static -Ddefault_library=static -Dbuildtype=debugoptimized -Dbuild_osx_bundle=true -Dlocal_boost=true
meson compile -C build_static
meson test -C build_static --verbose
meson compile osx-bundle -C build_static
meson compile osx-build-dmg -C build_static
```

## Updating Moonscript

From within the Moonscript repository, run `bin/moon bin/splat.moon -l moonscript moonscript/ > bin/moonscript.lua`.
Open the newly created `bin/moonscript.lua`, and within it make the following changes:

1. Prepend the final line of the file, `package.preload["moonscript"]()`, with a `return`, producing `return package.preload["moonscript"]()`.
2. Within the function at `package.preload['moonscript.base']`, remove references to `moon_loader`, `insert_loader`, and `remove_loader`. This means removing their declarations, definitions, and entries in the returned table.
3. Within the function at `package.preload['moonscript']`, remove the line `_with_0.insert_loader()`.

The file is now ready for use, to be placed in `automation/include` within the Aegisub repo.

## License

All files in this repository are licensed under various GPL-compatible BSD-style licenses; see LICENCE and the individual source files for more information.
The official Windows and OS X builds are GPLv2 due to including fftw3.
