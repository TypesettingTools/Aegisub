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

deps = []
if get_option('internal_luajit')
    deps += subproject('luajit').get_variable('luajit_dep')
else
    deps += dependency('luajit')
endif

luabins = static_library('luabins', luabins_src, dependencies: deps)
luabins_dep = declare_dependency(link_with: luabins)
