# Copy files to build directory for testing purposes
lua_files = files(
    'argcheck.moon',
    'clipboard.lua',
    'ffi.moon',
    'lfs.moon',
    're.moon',
    'unicode.moon',
    'util.moon',
)

foreach file: lua_files
    fs.copyfile(file)
endforeach
