set dist_dir="aegisub-%1-%2-%3-r%4"
md %dist_dir%
xcopy aegisub\tinderbox\windows\bin\aegisub??.exe %dist_dir%
xcopy aegisub\tinderbox\windows\bin\aegisub??.pdb %dist_dir%

"c:\Program Files (x86)\7-Zip\7z.exe" -bd a dist.7z %dist_dir%/*
