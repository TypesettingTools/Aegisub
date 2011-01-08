@echo off

set OS=Windows_NT
set PROCESSOR_ARCHITECTURE=AMD64
set PROCESSOR_IDENTIFIER=Intel64 Family 6 Model 15 Stepping 7, GenuineIntel
set PROCESSOR_LEVEL=6
set PROCESSOR_REVISION=0f07
set TRACE_FORMAT_SEARCH_PATH=\\NTREL202.ntdev.corp.microsoft.com\34FB5F65-FFEB-4B61-BF0E-A6A76C450FAA\TraceFormat
set DFSTRACINGON=FALSE
set FP_NO_HOST_CHECK=NO

set CommonProgramFiles=C:\Program Files\Common Files
set CommonProgramFiles(x86)=C:\Program Files (x86)\Common Files
set ProgramFiles=C:\Program Files
set ProgramFiles(x86)=C:\Program Files (x86)

set VSINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 9.0
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC
set VS90COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\
set DXSDK_DIR=C:\Program Files (x86)\Microsoft DirectX SDK (March 2009)\
set WindowsSdkDir=C:\Program Files\Microsoft SDKs\Windows\v6.0A\

set HOME=C:\Users\Aegisub
set LOCALAPPDATA=%HOME%\AppData\Local
set TEMP=%LOCALAPPDATA%\Temp
set TMP=%LOCALAPPDATA%\Temp

set INCLUDE=%VCINSTALLDIR%\ATLMFC\INCLUDE;%VCINSTALLDIR%\INCLUDE
set INCLUDE=%INCLUDE%;%WindowsSdkDir%\include
set INCLUDE=%INCLUDE%;%DXSDK_DIR%\Include
set INCLUDE=%INCLUDE%;%HOME%\Dev\wxlib\include
set INCLUDE=%INCLUDE%;%HOME%\Dev\src\freetype-2.3.9\include
set INCLUDE=%INCLUDE%;%HOME%\Dev\src\portaudio\include

set Framework35Version=v3.5
set FrameworkVersion=v2.0.50727
set PATHEXT=.COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC;.PY

if /i "%2"=="Win32" goto set32
if /i "%2"=="x64" goto set64

:set32
set LIB=%VCINSTALLDIR%\ATLMFC\LIB;%VCINSTALLDIR%\LIB
set LIB=%LIB%;%WindowsSdkDir%\lib
set LIB=%LIB%;%DXSDK_DIR%\Lib\x86
set LIB=%LIB%;%HOME%\Dev\wxlib\lib32
set LIB=%LIB%;%HOME%\Dev\src\freetype-2.3.9\objs\win32\vc2008
set LIB=%LIB%;%HOME%\Dev\src\portaudio\lib
set LIB=%LIB%;%HOME%\Dev\src\ffms2\lib
set FrameworkDir=C:\Windows\Microsoft.NET\Framework
set LIBPATH=C:\Windows\Microsoft.NET\Framework\v3.5;C:\Windows\Microsoft.NET\Framework\v2.0.50727;C:\Windows\Microsoft.NET\Framework\v3.5;C:\Windows\Microsoft.NET\Framework\v2.0.50727;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\ATLMFC\LIB;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\LIB;
set Path=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\BIN;C:\Windows\Microsoft.NET\Framework\v3.5;C:\Windows\Microsoft.NET\Framework\v3.5\Microsoft .NET Framework 3.5 (Pre-Release Version);C:\Windows\Microsoft.NET\Framework\v2.0.50727;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\VCPackages;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\bin;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin;C:\Program Files (x86)\CollabNet Subversion Server;C:\cygwin\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Program Files\TortoiseSVN\bin;C:\Python25;C:\Python25\Scripts
goto startbuild

:set64
set LIB=%VCINSTALLDIR%\ATLMFC\LIB\amd64;%VCINSTALLDIR%\LIB\amd64
set LIB=%LIB%;%WindowsSdkDir%\lib\x64
set LIB=%LIB%;%DXSDK_DIR%\Lib\x64
set LIB=%LIB%;%HOME%\Dev\wxlib\lib64
set LIB=%LIB%;%HOME%\Dev\src\freetype-2.3.9\objs\win32\vc2008_64
set LIB=%LIB%;%HOME%\Dev\src\portaudio\lib
set FrameworkDir=C:\Windows\Microsoft.NET\Framework64
set LIBPATH=C:\Windows\Microsoft.NET\Framework64\v3.5;C:\Windows\Microsoft.NET\Framework64\v2.0.50727;C:\Windows\Microsoft.NET\Framework64\v3.5;C:\Windows\Microsoft.NET\Framework64\v2.0.50727;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\ATLMFC\LIB\amd64;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\LIB\amd64;
set Path=C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\BIN\amd64;C:\Windows\Microsoft.NET\Framework64\v3.5;C:\Windows\Microsoft.NET\Framework64\v3.5\Microsoft .NET Framework 3.5 (Pre-Release Version);C:\Windows\Microsoft.NET\Framework64\v2.0.50727;C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\VCPackages;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools;C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\bin;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\x64;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\win64\x64;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin;C:\Program Files (x86)\CollabNet Subversion Server;C:\cygwin\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Program Files\TortoiseSVN\bin;C:\Python25;C:\Python25\Scripts
goto startbuild

:startbuild

vcbuild /nologo /platform:%2 /r /useenv aegisub\tinderbox\windows\aegisub_vs2008.sln %1^|%2
