project(
    'boost',
    'cpp',
    version: '1.83.0',
    meson_version: '>=0.55.0',
    default_options: ['cpp_std=c++20'],
)

warning('NOTE: after manually porting to boost 1.83 some things stopped working, this port is not working ATM!')

prj_version = meson.project_version()

cpp = meson.get_compiler('cpp')
thread_dep = dependency('threads')
inc = include_directories('.')
is_static = get_option('default_library') == 'static'


add_project_arguments('-DBOOST_ALL_NO_LIB=1', language: 'cpp')

if get_option('buildtype').startswith('release')
    add_project_arguments('-DNDEBUG', language: 'cpp')
endif

if host_machine.system() == 'windows'
    add_project_arguments('-DWIN32', language: 'cpp')
endif

icu_deps = [dependency('icu-uc'), dependency('icu-i18n')]

# regex is included in C++11  std libraries, so it isn't needed anymore!
modules = ['chrono', 'thread', 'filesystem', 'locale', 'program_options']
foreach module : modules
    subdir('libs/' + module)
endforeach

