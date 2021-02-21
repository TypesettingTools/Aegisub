project('luajit', 'c', version: '2.0.4', license: 'MIT')

cc = meson.get_compiler('c')

system_deps = [
    cc.find_library('dl', required: false),
    cc.find_library('m', required: false)
]

# compat flag is needed for both the buildvm code generator (compiled natively) and luajit itself
add_project_arguments('-DLUAJIT_ENABLE_LUA52COMPAT', language: 'c', native: true)
add_project_arguments('-DLUAJIT_ENABLE_LUA52COMPAT', language: 'c', native: false)
if host_machine.system() == 'linux'
    add_project_arguments('-DLUAJIT_OS=LUAJIT_OS_LINUX', language: 'c', native: true)
    readline_dep = cc.find_library('readline')
    ljvm_mode = 'elfasm'
    ljvm_bout = 'lj_vm.s'
elif host_machine.system() == 'darwin'
    add_project_arguments('-DLUAJIT_OS=LUAJIT_OS_OSX', language: 'c', native: true)
    readline_dep = cc.find_library('readline')
    ljvm_mode = 'machasm'
    ljvm_bout = 'lj_vm.s'
elif host_machine.system() == 'windows'
    add_project_arguments('-DLUAJIT_OS=LUAJIT_OS_WINDOWS', language: 'c', native: true)
    if cc.get_id() != 'msvc'
        add_project_arguments('-malign-double', language: 'c', native: true)
    endif
    readline_dep = []
    ljvm_mode = 'peobj'
    ljvm_bout = 'lj_vm.o'
else
    readline_dep = []
    error('Unsupported platform')
endif

incdir = include_directories('include')

subdir('src')

luajit_link_args = []
if host_machine.system() == 'darwin'
	luajit_link_args = ['pagezero_size', '10000', '-image_base', '100000000']
endif

luajit_dep = declare_dependency(link_with: luajit_lib,
                                link_args: luajit_link_args,
                                include_directories: incdir)
