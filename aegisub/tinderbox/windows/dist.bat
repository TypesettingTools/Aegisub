set dist_dir="aegisub-%1-%2-%3-r%4"
md %dist_dir%
xcopy aegisub\tinderbox\windows\bin\aegisub32.exe %dist_dir%
xcopy aegisub\tinderbox\windows\bin\aegisub32.pdb %dist_dir%
xcopy aegisub\tinderbox\windows\bin\libauto3_32.dll %dist_dir%
xcopy aegisub\tinderbox\windows\bin\libauto3_32.pdb %dist_dir%

"c:\Program Files (x86)\7-Zip\7z.exe" -bd a %dist_dir%.7z %dist_dir%/*
