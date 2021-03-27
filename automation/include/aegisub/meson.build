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

foreach f: lua_files
    configure_file(input: f, output: '@PLAINNAME@', copy: true)
endforeach
