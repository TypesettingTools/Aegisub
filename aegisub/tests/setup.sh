chmod 777 data/*
chmod -R 777 data/

rm -rf data
mkdir -p data

touch data/file
mkdir data/dir

touch data/file_access_denied
chmod 000 data/file_access_denied

touch data/file_read_only
chmod 444 data/file_read_only


mkdir data/dir_access_denied
chmod 000 data/dir_access_denied

mkdir data/dir_read_only
chmod 444 data/dir_read_only

echo '{"Valid" : ["Entry One", "Entry Two"]}' > data/mru_ok.json

echo '{"Invalid" : [1, 3]}' > data/mru_invalid.json

touch data/rename_me

touch data/rename_me_overwrite
touch data/rename_me_overwrite_renamed

mkdir data/options
cp options/* data/options

mkdir data/vfr
mkdir data/vfr/in
mkdir data/vfr/out
cp vfr/* data/vfr/in/

mkdir data/keyframe
cp keyframe/* data/keyframe
