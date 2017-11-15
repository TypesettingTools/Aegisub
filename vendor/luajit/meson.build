project('luajit', 'c', version : '2.0.4', license : 'MIT')

cc = meson.get_compiler('c')

system_deps = [
    cc.find_library('dl', required : false),
    cc.find_library('m', required : false)
]

system_deps_native = [
    cc.find_library('dl', required : false, native : true),
    cc.find_library('m', required : false, native : true)
]

add_project_arguments('-DLUAJIT_ENABLE_LUA52COMPAT', language : 'c')
if host_machine.system() == 'linux'
    add_project_arguments('-DLUAJIT_USE_LINUX', language : 'c')
    readline_dep = cc.find_library('readline')
    ljvm_mode = 'elfasm'
    ljvm_bout = 'lj_vm.s'
elif host_machine.system() == 'darwin'
    add_project_arguments(['-DLUAJIT_USE_MACOSX'], language : 'c')
    readline_dep = cc.find_library('readline')
    ljvm_mode = 'machasm'
    ljvm_bout = 'lj_vm.s'
elif host_machine.system() == 'windows'
    add_project_arguments('-DLUAJIT_USE_WINDOWS', '-DWIN',
                          '-malign-double', language : 'c')
    readline_dep = []
    ljvm_mode = 'peobj'
    ljvm_bout = 'lj_vm.o'
else
    readline_dep = []
    error('Unsupported platform')
endif

incdir = include_directories('include')

subdir('src')

luajit_dep = declare_dependency(link_with : luajit_lib,
                                include_directories : incdir)
