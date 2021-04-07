hunspell_sources = files([
    'affentry.cxx',
    'affixmgr.cxx',
    'csutil.cxx',
    'filemgr.cxx',
    'hashmgr.cxx',
    'hunspell.cxx',
    'hunzip.cxx',
    'phonet.cxx',
    'replist.cxx',
    'suggestmgr.cxx',
])

if get_option('default_library') == 'static'
    cpp_args = []
    compile_args = ['-DHUNSPELL_STATIC']
else
    cpp_args = ['-DBUILDING_LIBHUNSPELL']
    compile_args = []
endif

hunspell = library('hunspell', hunspell_sources,
                   cpp_args: cpp_args + compile_args)
