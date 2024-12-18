# Packaging

## Windows installer

Prerequisites:

1. 7zip (7z.exe on your PATH)
2. Moonscript (moonc.exe on your PATH)
3. Gettext (installing from https://mlocati.github.io/articles/gettext-iconv-windows.html seems to be the easiest option)
4. InnoSetup (needs to be a recent version, iscc.exe on your PATH which does not happen by default on installs)

If these are all available before you built aegisub, running `ninja win-installer` should download and build the dependencies, then create the installer.

## MacOS

TODO
