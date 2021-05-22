@echo off

set SOURCE_ROOT=%1
set BUILD_ROOT=%1\build


echo Goto building dir
cd %BUILD_ROOT%

echo Removing old temp dir
rmdir /S /Q aegisub-portable
rmdir /S /Q install


echo Make install
meson install --no-rebuild --destdir %BUILD_ROOT%\install


echo Gathering files
xcopy %BUILD_ROOT%\install\bin\aegisub.exe  aegisub-portable\
@REM TODO: check and remove freetype later
xcopy %BUILD_ROOT%\install\bin\freetype-6.dll  aegisub-portable\

echo Copying - translations
xcopy %BUILD_ROOT%\install\share\locale  aegisub-portable\locale\  /s /e
xcopy %BUILD_ROOT%\install\share\locale  aegisub-portable\csri\  /s /e
echo Copying - codecs
echo Copying - codecs\Avisynth
xcopy %BUILD_ROOT%\installer-deps\AvisynthPlus64\x86-64\DevIL.dll  aegisub-portable\
xcopy %BUILD_ROOT%\installer-deps\AvisynthPlus64\x86-64\AviSynth.dll  aegisub-portable\
xcopy %BUILD_ROOT%\installer-deps\AvisynthPlus64\x86-64\plugins\DirectShowSource.dll  aegisub-portable\
echo Copying - codecs\VSFilter
xcopy %BUILD_ROOT%\installer-deps\VSFilter\x64\VSFilter.dll  aegisub-portable\csri\
echo Copying - runtimes\MS-CRT
xcopy %BUILD_ROOT%\installer-deps\VC_redist\VC_redist.x64.exe aegisub-portable\Microsoft.CRT\

echo Copying - automation
xcopy %BUILD_ROOT%\install\share\aegisub\automation  aegisub-portable\automation\  /s /e
echo Copying - automation\DEPCTRL
xcopy %BUILD_ROOT%\installer-deps\DependencyControl\modules  aegisub-portable\automation\include\l0\  /s /e
xcopy %BUILD_ROOT%\installer-deps\DependencyControl\macros  aegisub-portable\automation\autoload\  /s /e
xcopy %BUILD_ROOT%\installer-deps\Yutils\src\Yutils.lua  aegisub-portable\automation\include\
xcopy %BUILD_ROOT%\installer-deps\luajson\lua  aegisub-portable\automation\include\  /s /e

xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\requireffi\requireffi.lua  aegisub-portable\automation\include\requireffi\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\bad-mutex\BadMutex.dll  aegisub-portable\automation\include\BM\BadMutex\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\bad-mutex\BadMutex.lua  aegisub-portable\automation\include\BM\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\precise-timer\PreciseTimer.dll  aegisub-portable\automation\include\PT\PreciseTimer\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\precise-timer\PreciseTimer.lua  aegisub-portable\automation\include\PT\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\download-manager\DownloadManager.dll  aegisub-portable\automation\include\DM\DownloadManager\
xcopy %BUILD_ROOT%\installer-deps\ffi-experiments\build\download-manager\DownloadManager.lua  aegisub-portable\automation\include\DM\

echo Copying - portable-config
xcopy %SOURCE_ROOT%\packages\win_installer\portable\config.json  aegisub-portable\


echo Creating portable zip
del aegisub-portable-64.zip
7z a aegisub-portable-64.zip aegisub-portable\
