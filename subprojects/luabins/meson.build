project('luabins', ['c'])
luabins_src = files(
    'src/fwrite.c',
    'src/load.c',
    'src/luabins.c',
    'src/luainternals.c',
    'src/savebuffer.c',
    'src/save.c',
    'src/write.c',
)

luabins = static_library('luabins', luabins_src, dependencies: dependency('luajit'))
luabins_dep = declare_dependency(link_with: luabins)
