project('boost', 'cpp',
  version: '1.74.0',
  meson_version: '>=0.55.0')

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

icu_deps = [dependency('icu-uc'),
            dependency('icu-i18n')]

modules = ['chrono', 'thread', 'filesystem', 'locale', 'regex', 'program_options']
foreach module: modules
    subdir('libs/' + module)
endforeach
