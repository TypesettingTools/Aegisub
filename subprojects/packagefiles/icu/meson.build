project('icu', 'c', 'cpp',
  version: '67.1',
  meson_version: '>=0.55.0',
  default_options: 'cpp_std=c++17')

cpp = meson.get_compiler('cpp')

if host_machine.system() == 'windows'
   add_project_arguments('-DWIN32', '-DWIN64', '-D_MBCS', '/utf-8', language : 'cpp')
   add_project_arguments('-DWIN32', '-DWIN64', '-D_MBCS', '/utf-8', language : 'c')
endif

if cpp.has_header('elf.h')
   add_project_arguments('-DU_HAVE_ELF_H=1', language: 'c')
   add_project_arguments('-DU_HAVE_ELF_H=1', language: 'cpp')
endif

dl_dep = cpp.find_library('dl', required: false)
thread_dep = dependency('threads')

# Compiler flags the users of this library must use.
usage_args = []

if get_option('default_library') == 'static'
    add_project_arguments('-DU_STATIC_IMPLEMENTATION', language : 'c')
    add_project_arguments('-DU_STATIC_IMPLEMENTATION', language : 'cpp')
    usage_args = ['-DU_STATIC_IMPLEMENTATION']
elif host_machine.system() == 'windows'
    error('Shared library build not supported on Windows. Set default_library to static.')
endif

subdir('source')
