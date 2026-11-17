#!/bin/env bash

set -e

MANIFEST="manifest.respack"

while IFS="" read -r line || [ -n "$p" ]; do
    if [[ "$line" =~ ^button/(.*)_([0-9]+)\.png$ ]]; then
        name="${BASH_REMATCH[1]}"
        size="${BASH_REMATCH[2]}"

        infile="../../docs/art-sources/buttons/${name}.svg"
        outfile="${line}"

        echo $outfile

        inkscape "${infile}" -o "${outfile}" -w "${size}" -h "${size}"
    fi
done < "${MANIFEST}"
