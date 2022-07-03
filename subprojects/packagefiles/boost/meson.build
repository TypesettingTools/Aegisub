project('boost', 'cpp',
    version: '1.79.0',
    meson_version: '>=0.55.0'
    )

prj_version = meson.project_version() 

cpp = meson.get_compiler('cpp')
thread_dep = dependency('threads')
inc = include_directories('.')
is_static = get_option('default_library') == 'static'

cxx = meson.get_compiler('cpp')


code = '''#include<stdio.h>
void func() { printf("Compile me.\n"); }
int main(void){return 0;}
'''

has_stdcpp20 = cxx.links(code, args : '-std=c++20', name : 'std check')
cpp_20_std = '-std=c++20'
if not has_stdcpp20
   #  error('std=c++20 isn\'t supported by the compiler!')
   cpp_20_std = '--std=c++2a'
endif

# c++20' is needed for the newer features!

add_project_arguments([cpp_20_std], language : 'cpp')
add_project_link_arguments([cpp_20_std], language : 'cpp')
add_project_arguments('-DBOOST_ALL_NO_LIB=1', language: 'cpp')

if get_option('buildtype').startswith('release')
    add_project_arguments('-DNDEBUG', language: 'cpp')
endif

if host_machine.system() == 'windows'
    add_project_arguments('-DWIN32', language: 'cpp')
endif

icu_deps = [dependency('icu-uc'),
            dependency('icu-i18n')]

# regex is included in C++11  std libraries, so it isn't needed anymore!
modules = ['chrono', 'thread', 'filesystem', 'locale', 'program_options']
foreach module: modules
    subdir('libs/' + module)
endforeach