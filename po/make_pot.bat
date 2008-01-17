del /s list.txt
dir /w /b ..\aegisub\*.cpp ..\aegisub\*.h >> list.txt
"c:\program files\gettext\bin\xgettext.exe" --files-from=list.txt --directory=../aegisub/ --output=aegisub.pot --c++ -k_
