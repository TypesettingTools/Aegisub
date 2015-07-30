rm -f make-dry
make -n > make-dry
grep '^cl' make-dry \
	| sed 's/.*\(lib[^ ]*.c\) .*/\1/' \
	| sed 's/\//\\/g' \
	| sed 's/\(.*\)/<ClCompile Include="$(FfmpegSrcDir)\\\1" \/>/' \
	| sort
echo
grep '^yasm' make-dry \
	| sed 's/.*\(lib[^ ]*.asm\) .*/\1/' \
	| sed 's/\//\\/g' \
	| sed 's/\(.*\)/<Yasm Include="$(FfmpegSrcDir)\\\1" \/>/' \
	| sort
