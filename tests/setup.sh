d=$(dirname $0)/
if test -d data; then
	chmod 777 data/*
	chmod -R 777 data/
fi

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

printf %s '1234567890' > data/ten_bytes
touch -r $0 data/touch_mod_time

mkdir data/options
cp $d/options/* data/options

mkdir data/dir_iterator
touch data/dir_iterator/1.a
touch data/dir_iterator/2.a
touch data/dir_iterator/1.b
touch data/dir_iterator/2.b

mkdir data/vfr
mkdir data/vfr/in
mkdir data/vfr/out
cp $d/vfr/* data/vfr/in/

mkdir data/keyframe
cp $d/keyframe/* data/keyframe
