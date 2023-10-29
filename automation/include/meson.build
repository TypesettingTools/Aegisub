# Copy files to build directory for testing purposes
lua_files = files(
    'cleantags.lua',
    'clipboard.lua',
    'karaskel-auto4.lua',
    'karaskel.lua',
    'lfs.lua',
    'moonscript.lua',
    're.lua',
    'unicode.lua',
    'unicode-monkeypatch.lua',
    'utils-auto4.lua',
    'utils.lua',
)

fs = import('fs')

foreach file: lua_files
    fs.copyfile(file)
endforeach

subdir('aegisub')
