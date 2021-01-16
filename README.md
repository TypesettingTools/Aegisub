# Aegisub

For binaries and general information [see the homepage](http://www.aegisub.org).

The bug tracker can be found at https://github.com/Aegisub/Aegisub/issues.

Support is available on [Discord](https://discord.com/invite/AZaVyPr) or [IRC](irc://irc.rizon.net/aegisub).

## Building Aegisub

### Windows

Prerequisites:

1. Visual Studio (Community edition of any recent version is fine)
2. The June 2010 DirectX SDK (the final release before DirectSound was dropped)
3. Python 3
4. Meson
5. CMake
6. Powershell execution policy set to Unrestricted

There are a few optional dependencies that must be installed and on your PATH:

1. msgfmt, to build the translations
2. WinRAR, to build the portable installer
3. InnoSetup, to build the regular installer
4. 7zip, to build the regular installer
5. Moonscript, to build the regular installer

All other dependencies are either stored in the repository or are included as submodules.

Building:

1. Clone Aegisub's repository: `git clone https://github.com/TypesettingTools/Aegisub.git`
2. From the Visual Studio "x64 Native Tools Command Prompt", generate the build directory: `meson build -Ddefault_library=static`
3. Build with `cd build` and `meson compile`

You should now have a binary at `src/aegisub.exe`.

Installer:

You can generate the installer with `ninja win-installer` after a successful build. This assumes a working internet connection and installation of the optional dependencies.

### OS X

A vaguely recent version of Xcode and the corresponding command-line tools are required.

For personal usage, you can use pip and homebrew to install almost all of Aegisub's dependencies:

	brew install libass nasm ninja boost zlib icu4c pkg-config ffms2 fftw hunspell gettext cmake
	brew link --force gettext
	export LDFLAGS="-L/usr/local/opt/icu4c/lib"
	export CPPFLAGS="-I/usr/local/opt/icu4c/include"
	export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig"
	pip install meson

Once the dependencies are installed, build Aegisub with `meson build && meson compile -C build`.

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
