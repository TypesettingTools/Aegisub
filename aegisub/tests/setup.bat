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

echo {"Valid" : []} > data/mru_ok.json

echo {"Invalid" : [1 3]} > data/mru_invalid.json

echo > data/rename_me

echo > data/rename_me_overwrite
echo > data/rename_me_overwrite_renamed


echo {"String" : "This is a test"} > data/option_string.json
echo {"Integer" : 1} > data/option_integer.json
echo {"Double" : 2.1} > data/option_double.json
echo {"Bool" : true} > data/option_bool.json
echo {"Null" : null} > data/option_null.json

echo {"String" : [{"string" : "This is a test"}, {"string" : "This is a test"}]} > data/option_array_string
echo {"Integer" : [{"int" : 1}, {"int" : 1}]} > data/option_array_integer
echo {"Double" : [{"double" : 2.1}, {"double" : 2.1}]} > data/option_array_double
echo {"Bool" : [{"bool" : true}, {"bool" : true}]} > data/option_array_bool

mkdir data\vfr
mkdir data\vfr\in
mkdir data\vfr\out

xcopy "%~dp0\vfr" data\vfr\in

mkdir data\keyframe
xcopy "%~dp0\keyframe" data\keyframe
