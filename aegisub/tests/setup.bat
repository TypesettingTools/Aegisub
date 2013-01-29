cd %1

icacls data /grant:r %USERNAME%:F /T
rd /s /q data
mkdir data

echo '' > data/file
mkdir data\dir

echo  > data/file_access_denied
icacls data\file_access_denied /deny %USERNAME%:F

echo  > data/file_read_only
icacls data\file_read_only /deny %USERNAME%:W


mkdir data\dir_access_denied
icacls data\dir_access_denied /deny %USERNAME%:F

mkdir data\dir_read_only
icacls data\dir_read_only /deny %USERNAME%:W

echo {"Valid" : ["Entry One", "Entry Two"]} > data/mru_ok.json

echo {"Invalid" : [1, 3]} > data/mru_invalid.json

echo > data/rename_me

echo > data/rename_me_overwrite
echo > data/rename_me_overwrite_renamed

mkdir data\options
xcopy "%~dp0\options" data\options

mkdir data\vfr
mkdir data\vfr\in
mkdir data\vfr\out

xcopy "%~dp0\vfr" data\vfr\in

mkdir data\keyframe
xcopy "%~dp0\keyframe" data\keyframe
