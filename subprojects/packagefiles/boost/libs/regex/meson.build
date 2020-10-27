regex_sources = files([
    'src/c_regex_traits.cpp',
    'src/cpp_regex_traits.cpp',
    'src/cregex.cpp',
    'src/fileiter.cpp',
    'src/icu.cpp',
    'src/instances.cpp',
    'src/posix_api.cpp',
    'src/regex.cpp',
    'src/regex_debug.cpp',
    'src/regex_raw_buffer.cpp',
    'src/regex_traits_defaults.cpp',
    'src/static_mutex.cpp',
    'src/w32_regex_traits.cpp',
    'src/wc_regex_traits.cpp',
    'src/wide_posix_api.cpp',
    'src/winstances.cpp',
    'src/usinstances.cpp',
])

regex_args = ['-DBOOST_HAS_ICU=1']

if not is_static
    regex_args += '-DBOOST_REGEX_DYN_LINK=1'
endif

boost_regex = library('boost_regex', regex_sources,
                      include_directories: inc,
                      cpp_args: regex_args,
                      dependencies: [thread_dep, icu_deps])

boost_regex_dep = declare_dependency(link_with: boost_regex, include_directories: inc,
                                     compile_args: '-DBOOST_ALL_NO_LIB=1')
