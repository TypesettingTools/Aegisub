cd %1

icacls data /grant:r %USERNAME%:F /T
rd /s /q data
mkdir data

copy nul data\file
mkdir data\dir

echo {"Video" : ["Entry One", "Entry Two"]} > data/mru_ok.json

echo {"Video" : [1, 3]} > data/mru_invalid.json

< nul set /p ="1234567890" > data\ten_bytes
copy nul data\touch_mod_time

mkdir data\dir_iterator
copy nul data\dir_iterator\1.a
copy nul data\dir_iterator\2.a
copy nul data\dir_iterator\1.b
copy nul data\dir_iterator\2.b

mkdir data\options
xcopy "%~dp0\options" data\options

mkdir data\vfr
mkdir data\vfr\in
mkdir data\vfr\out

xcopy "%~dp0\vfr" data\vfr\in

mkdir data\keyframe
xcopy "%~dp0\keyframe" data\keyframe

ping -n 2 127.0.0.1 > nul
