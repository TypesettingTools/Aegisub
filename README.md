# Aegisub

For binaries and general information [see the homepage](http://www.aegisub.org).

The bug tracker can be found at http://devel.aegisub.org.

Support is available on [the forums](http://forum.aegisub.org) or [on IRC](irc://irc.rizon.net/aegisub).

## Building Aegisub

### Windows

Prerequisites:

1. Visual Studio 2013. Express edition might work.
2. A recent Windows SDK
3. A recent DirectX SDK
4. A MSYS install with git and c99conv. Note that mingw is not required.

There are a few optional dependencies:

1. msgfmt, to build the translations
2. WinRAR, to build the portable installer
3. InnoSetup, to build the regular installer

All other dependencies are either stored in the repository or are included as submodules.

Building:

1. Clone Aegisub's repository recursively to fetch it and all submodules: `git clone --recursive git@github.com:Aegisub/Aegisub.git`
2. Disable autocrlf for ffmpeg, as its build system manages to not support Windows newlines: `cd vendor/ffmpeg && git config --local core.autocrlf && git rm --cached -r . && git reset --hard`
3. Open Visual Studio from the VS2013 Native Tools Command Promp using devenv.exe /useenv (required for the build system to be able to find nmake.exe for building wxWidgets)
4. Open Aegisub.sln
5. Open the properties for the Aegisub project and set the location of MSYS in Configuration Properties > Aegisub > Library paths
6. Build Aegisub
7. Copy the contents of an existing Aegisub install into the aegisub/aegisub/bin directory (not strictly required, but you'll be missing a lot of functionality otherwise).

There's a pile of other files needed at runtime such as dictionaries, VSFilter and avisynth. The simplest way to get them is to copy all of the files from the Aegisub installer to the bin directory.

For actual development work you will probably want to mostly use the
"Debug-MinDep" configuration (which disables building most of the projects), as
the dependency checking is pretty slow.

## License

All source files in this repository are licensed under either 3-clause BSD or
ISC licenses. In practice, Aegisub binaries are usually GPL licensed due to the
dependencies.
