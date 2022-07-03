project('boost', 'cpp',
    version: '1.79.0',
    meson_version: '>=0.55.0'
    )


prj_version = meson.project_version() 

cpp = meson.get_compiler('cpp')
thread_dep = dependency('threads')
inc = include_directories('.')
is_static = get_option('default_library') == 'static'


# c++20' is needed for the newer features!!
add_project_arguments(['-std=c++20'], language : 'cpp')
add_project_link_arguments(['-std=c++20'], language : 'cpp')
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