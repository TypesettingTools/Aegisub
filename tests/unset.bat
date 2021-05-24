@echo off

IF [%1]==[] GOTO :help


:main
@REM Switch to the build directory
cd %1

@REM Restores all permissions to the four read-only and access denied files
icacls data\file_access_denied /grant %USERNAME%:F
icacls data\file_read_only /grant %USERNAME%:W
icacls data\dir_access_denied /grant %USERNAME%:F
icacls data\dir_read_only /grant %USERNAME%:W

@REM Delete the entire test data folder
rmdir /s/q data


:help
@REM Show help information
ECHO Restore access and delete the test data folder.
ECHO    Usage:  unset PATH_TO_BUILD_DIR
