cd %~dp0..
sh build/version.sh .
if %ERRORLEVEL% NEQ 0 goto :fail
goto :eof

:fail
ECHO Aegisub requires that sh and git be on the windows command line path for version checking.
>  build\git_version.h echo #define BUILD_GIT_VERSION_NUMBER 0
>> build\git_version.h echo #define BUILD_GIT_VERSION_STR "unknown"
