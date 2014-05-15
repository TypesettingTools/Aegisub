#!/bin/sh

intltool-merge --quiet --keys-style ../../po fragment_strings.iss.in fragment_strings.iss

# None of the intltool formats quite match InnoSetup's, so munge one that's close
sed -i '' 's/^\[\(.*\)\]\(.\)/\1.\2/' fragment_strings.iss
