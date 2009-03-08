del /s list.txt
dir /w /b ..\aegisub\*.cpp ..\aegisub\*.h >> list.txt
"C:\Program Files (x86)\GnuWin32\bin\xgettext.exe" --files-from=list.txt --directory=../aegisub/ --output=aegisub.pot --c++ -k_
pause