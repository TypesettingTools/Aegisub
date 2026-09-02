# Aegisub

For binaries and general information [see the homepage](http://aegisub.org).

The bug tracker can be found at https://github.com/TypesettingTools/Aegisub/issues.

Support is available on [Discord](https://discord.com/invite/AZaVyPr) or [IRC](irc://irc.rizon.net/aegisub).

## Building Aegisub

### Windows

Aegisub targets 64-bit builds only (Windows x86_64/ARM64 and Linux x86_64/ARM64). 32-bit/x86 builds are no longer supported.

Supported platforms (policy update):

- Windows: Windows 11 and newer (development and release builds target modern MSVC toolchains; Windows 10 is no longer supported).
- Linux: Supported on modern distributions only. Recommended: Ubuntu 22.04 LTS or newer, Fedora 36 or newer, or Arch Linux (rolling). Ancient distributions with outdated glibc or old system libraries are not supported.

Visual Studio 2026 (upstream target)

The repository is being prepared to upstream support for Visual Studio 2026. Developer-focused instructions and low-level build knobs are recorded in docs/developer.md — end users do not need to modify these settings.

If you need CI coverage for Visual Studio 2026 or a specific toolset, the GitHub Actions CI has a best-effort matrix entry that attempts to exercise the newer toolset when available on the runner.

If you need help with a specific Visual Studio 2026 issue (toolset name, msbuild flags, or SDK differences), tell me what errors you see and I will make targeted changes.

If you need longer compatibility (older distros or Windows 10), consider maintaining a separate compatibility branch or using containerized builds that pin older toolchains.

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

Building (x64):

1. Clone Aegisub's repository: `git clone https://github.com/TypesettingTools/Aegisub.git`
2. From the Visual Studio "x64 Native Tools Command Prompt", generate the build directory: `meson setup build -Ddefault_library=static` (if building for release, add `--buildtype=release`)
3. Build with `cd build` and `ninja`

You should now have a binary: `aegisub.exe`.

To build a Windows ARM64 binary from an x64 machine, install the Visual Studio ARM64
build tools, open an x64 Native Tools prompt, and select the ARM64 target when
initializing the developer environment:

```powershell
Enter-VsDevShell -VsInstallPath $env:VS -DevCmdArguments "-arch=arm64 -host_arch=x64"
meson setup arm64-build -Ddefault_library=static
meson compile -C arm64-build
```

For repeatable builds, the equivalent wrapper is:

```powershell
.\tools\build_windows.ps1 -Architecture x64
.\tools\build_windows.ps1 -Architecture arm64
```

Linux AppImage (x86_64/ARM64)

Aegisub supports creating a portable AppImage for x86_64 and ARM64 Linux hosts. A helper script is included at [tools/create_appimage.sh](C:/Users/Aura/Documents/GitHub/Aegisub/tools/create_appimage.sh).

Prerequisites:

- Meson, Ninja, and typical Linux build tools
- x86_64 or ARM64 Linux host

Build steps (recommended):

1. From the repository root, run:

   ./tools/create_appimage.sh build

   The script will configure and build the project (static default_library), install into an AppDir, and then use the matching native appimagetool to produce `Aegisub-v3.5.0-x86_64.AppImage` or `Aegisub-v3.5.0-arm64.AppImage`. Set `APP_VERSION` to override the release label.

AppImages must be built in Linux (natively or through WSL2, a virtual machine, or a container); a Windows host cannot execute the Linux build toolchain directly. The GitHub Actions workflow builds both architectures on native Linux runners.

The script also accepts `MESON_CROSS_FILE` for a Linux cross toolchain. For example,
from an x86_64 Linux host with an installed AArch64 compiler, sysroot, and
cross-pkg-config setup:

```bash
MESON_CROSS_FILE=cross/aarch64-linux-gnu.ini \
  tools/create_appimage.sh build-arm64
```

Cross-compiling Linux dependencies requires a complete AArch64 sysroot. Native
Linux runners are preferred for AppImages because they automatically provide the
correct system libraries and avoid accidentally packaging host-architecture
dependencies.

Notes:
- If you want to customize Meson options (for example to set -Dwx_version=3.3.0), pass them when running meson setup manually before running the script, e.g.:

  meson setup build --reconfigure -Dwx_version=3.3.0 -Ddefault_library=static
  ./tools/create_appimage.sh build

- The script will try to locate an icon in `packages/icons` or convert `packages/osx_bundle/Contents/Resources/Aegisub.icns` if conversion tools are available. If you have a preferred PNG icon, place it at `packages/icons/aegisub.png` before running the script.

Building with Visual Studio (MSVC)

If you prefer to use Visual Studio for iterative development and debugging, Meson can generate a Visual Studio solution. Open the "x64 Native Tools Command Prompt" for the Visual Studio installation you want to use, then run:

```
# Generate a Visual Studio solution (backend 'vs')
meson setup vsbuild --backend vs -Dwx_version=3.3.0 -Dwindows_target=0x0A00

# Open the generated solution in Visual Studio
# (the solution is at vsbuild\Aegisub.sln)
```

Notes:
- Open the solution in Visual Studio and select the desired configuration (Debug/Release) and platform (x64). Building inside Visual Studio uses the same toolchain as Meson.
- Ensure you run Meson from the same "x64 Native Tools Command Prompt" so the subprojects are configured with the same MSVC toolset (avoids CRT mismatch).
- To build from CLI using the generated solution/builddir:
```
meson compile -C vsbuild
```


Installer:

You can generate the x64 installer with the wrapper (after installing Inno Setup,
7-Zip, MoonScript, and gettext/msgfmt):

```powershell
.\tools\build_windows.ps1 -Architecture x64 -Installer
```

The installer is written to `build-x64\Aegisub-*.exe`. The lower-level equivalent
is `meson compile -C build-x64 win-installer`.

You can generate the portable zip with `ninja win-portable` after a successful build.

The current installer dependency payload is x64-specific (including VSFilter and
the bundled VC++ redistributable), so ARM64 binaries are not packaged by this
wrapper yet. ARM64 installer support requires ARM64-compatible third-party
payloads and an ARM64 redistributable.

### OS X

A vaguely recent version of Xcode and the corresponding command-line tools are required.

For personal usage, you can use pip and homebrew to install almost all of Aegisub's dependencies:

    pip3 install meson      # or brew install meson if you installed Python via brew
    brew install cmake ninja pkg-config  libass boost zlib ffms2 fftw hunspell uchardet
    export LDFLAGS="-L/usr/local/opt/icu4c/lib"
    export CPPFLAGS="-I/usr/local/opt/icu4c/include"
    export PKG_CONFIG_PATH="/usr/local/opt/icu4c/lib/pkgconfig"

When compiling on Apple Silicon, replace `/usr/local` with `/opt/homebrew`.

Once the dependencies are installed, build Aegisub with `meson build && meson compile -C build`.

#### Build dmg

```bash
meson build_static -Ddefault_library=static -Dbuildtype=debugoptimized -Dbuild_osx_bundle=true -Dlocal_boost=true
meson compile -C build_static
meson test -C build_static --verbose
meson compile osx-bundle -C build_static
meson compile osx-build-dmg -C build_static
```

### Linux or other

#### Build dependencies for Debian-based systems

```
compiler:    build-essential
pkgconfig:   pkg-config  or  pkgconf
meson:       meson ninja-build
gettext:     gettext intltool
fontconfig:  libfontconfig1-dev
libass:      libass-dev
boost:       libboost-chrono-dev libboost-locale-dev libboost-regex-dev libboost-system-dev libboost-thread-dev
zlib:        zlib1g-dev
WxWidgets:   wx3.2-headers libwxgtk3.2-dev
ICU:         icu-devtools libicu-dev
pulse-audio: libpulse-dev
ALSA:        libasound2-dev
OpenAL:      libopenal-dev
ffms2:       libffms2-dev
fftw3:       libfftw3-dev
hunspell:    libhunspell-dev
uchardet:    libuchardet-dev
libcurl:     libcurl4-openssl-dev  or  libcurl4-gnutls-dev
opengl:      libgl1-mesa-dev
gtest:       libgtest-dev
gmock:       libgmock-dev
libportal:   libportal-gtk3-dev
```

I.e. to install on Ubuntu 24.04 run this command:
``` bash
sudo apt install build-essential pkg-config meson ninja-build gettext intltool libfontconfig1-dev libass-dev libboost-chrono-dev libboost-locale-dev libboost-regex-dev libboost-system-dev libboost-thread-dev zlib1g-dev wx3.2-headers libwxgtk3.2-dev icu-devtools libicu-dev libpulse-dev libasound2-dev libopenal-dev libffms2-dev libfftw3-dev libhunspell-dev libuchardet-dev libcurl4-gnutls-dev libgl1-mesa-dev libgtest-dev libgmock-dev libportal-gtk3-dev
```

#### Build Aegisub

``` bash
meson setup build --prefix=/usr/local --buildtype=release --strip -Dsystem_luajit=false -Ddefault_library=static
meson compile -C build
meson install -C build --skip-subprojects luajit
```

#### Packaging
If you are packaging Aegisub for a Linux distribution, here are a few things you may need to know:
- Aegisub cannot be built with LTO (See: https://github.com/TypesettingTools/Aegisub/issues/290).
- Aegisub depends on LuaJIT and *requires* LuaJIT to be build with Lua 5.2 compatibility enabled.
  We are aware that most distributions do not compile LuaJIT with this flag, and that this complicates packaging for them, see https://github.com/TypesettingTools/Aegisub/issues/239 for a detailed discussion of the situation.

  Like for its other dependencies, Aegisub includes a meson subproject for LuaJIT that can be used to statically link a version of LuaJIT with 5.2 compatibility.
  For distributions that do not allow downloading additional sources at build time, the downloaded LuaJIT subproject is included in the source tarballs distributed with releases.
- When linked against libstdc++, Aegisub needs libstdc++ 6.0.32 or later due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95048.
  Aegisub's tests will detect this bug, but if you're not running tests on packaging you'll need to make sure the libstdc++ version is recent enough.
- Aegisub uses OpenGL through wxWidgets. For Aegisub to work directly on Wayland (as opposed to Xwayland), wxWidgets needs to be built with EGL enabled.
  Aegisub will automatically fall back to X11 when it detects missing EGL support.

The following commands are an example for how to build Aegisub with the goal of creating a distribution package:

```bash
meson subprojects download luajit              # Or use the tarball
meson subprojects packagefiles --apply luajit

meson setup builddir --wrap-mode=nodownload --prefix=/usr --buildtype=release -Dsystem_luajit=false -Ddefault_library=static -Dtests=false

meson compile -C builddir
meson install -C builddir --skip-subprojects luajit
```

## Developer Documenation
Some documentation for developers is available in [docs/developer_docs.md](docs/developer_docs.md).

## License

All files in this repository are licensed under various GPL-compatible BSD-style licenses; see LICENCE and the individual source files for more information.
The official Windows and OS X builds are GPLv2 due to including fftw3.
