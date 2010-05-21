chmod 664 data/*
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

echo '{"Valid" : []}' > data/mru_ok.json

echo '{"Invalid" : [1 3]}' > data/mru_invalid.json

touch data/rename_me

touch data/rename_me_overwrite
touch data/rename_me_overwrite_renamed


echo '{"String" : "This is a test"}' > data/option_string.json
echo '{"Integer" : 1}' > data/option_integer.json
echo '{"Double" : 2.1}' > data/option_double.json
echo '{"Bool" : true}' > data/option_bool.json
echo '{"Null" : null}' > data/option_null.json

echo '{"String" : [{"string" : "This is a test"}, {"string" : "This is a test"}]}' > data/option_array_string
echo '{"Integer" : [{"int" : 1}, {"int" : 1}]}' > data/option_array_integer
echo '{"Double" : [{"double" : 2.1}, {"double" : 2.1}]}' > data/option_array_double
echo '{"Bool" : [{"bool" : true}, {"bool" : true}]}' > data/option_array_bool
