@echo off

echo Gathering files
xcopy ..\..\..\automation\autoload\*.lua aegisub-portable\automation\autoload\ > NUL
xcopy ..\..\..\automation\demos\*.lua aegisub-portable\automation\demos\ > NUL
xcopy ..\..\..\automation\include\*.lua aegisub-portable\automation\include\ > NUL
xcopy ..\..\..\bin\aegisub32.exe aegisub-portable\ > NUL
xcopy ..\..\..\bin\ffms2.dll aegisub-portable\ > NUL
xcopy ..\src\ASSDraw3.chm aegisub-portable\ > NUL
xcopy ..\src\ASSDraw3.exe aegisub-portable\ > NUL
xcopy ..\src\DirectShowSource.dll aegisub-portable\ > NUL
xcopy ..\src\avisynth.dll aegisub-portable\ > NUL
xcopy ..\src\devil.dll aegisub-portable\ > NUL
xcopy ..\src\dictionaries\en_US.aff aegisub-portable\dictionaries\ > NUL
xcopy ..\src\dictionaries\en_US.dic aegisub-portable\dictionaries\ > NUL
xcopy ..\src\vsfilter-aegisub32.dll aegisub-portable\csri\ > NUL
xcopy ..\src\xy-vsfilter-aegisub.dll aegisub-portable\csri\ > NUL
xcopy ..\src\x86\Microsoft.VC90.CRT\* aegisub-portable\Microsoft.VC90.CRT\ > NUL
xcopy ..\src\x86\OpenAL32.dll aegisub-portable\ > NUL
xcopy config.json aegisub-portable\ > NUL

echo Creating SFX
del ..\output\aegisub-3.0.0-portable.exe
WinRAR a -sfx -s -m5 -ep1 -r -zportable-comment.txt -iimgside-logo.bmp -iiconicon.ico ..\output\aegisub-3.0.0-portable.exe aegisub-portable\

echo Removing temp dir
rmdir /S /Q aegisub-portable > NUL

