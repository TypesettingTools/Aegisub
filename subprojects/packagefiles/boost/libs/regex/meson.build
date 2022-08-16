regex_sources = files([
    # 'src/posix_api.cpp',
    'src/regex.cpp',
    'src/static_mutex.cpp',
    # 'src/wide_posix_api.cpp',
])

regex_args = ['-DBOOST_HAS_ICU=1']

if not is_static
    regex_args += '-DBOOST_REGEX_DYN_LINK=1'
endif

inc = '../..'
boost_regex = library('boost_regex', regex_sources,
                      include_directories: inc,
                      cpp_args: regex_args,
                      dependencies: [thread_dep, icu_deps])

boost_regex_dep = declare_dependency(link_with: boost_regex, include_directories: inc,
                                     compile_args: '-DBOOST_ALL_NO_LIB=1')
