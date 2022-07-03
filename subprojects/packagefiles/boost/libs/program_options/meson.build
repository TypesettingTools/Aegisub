program_options_sources = files([
    'src/cmdline.cpp',
    'src/config_file.cpp', 
    'src/convert.cpp',
    'src/options_description.cpp',
    'src/parsers.cpp', 
    'src/positional_options.cpp', 
    'src/split.cpp', 
    'src/utf8_codecvt_facet.cpp',
    'src/value_semantic.cpp',
    'src/variables_map.cpp',
    'src/winmain.cpp'
])

program_options_args = []
if not is_static
    program_options_args += '-DBOOST_PROGRAM_OPTIONS_DYN_LINK=1'
endif

boost_program_options = library('boost_program_options' +'.'+ prj_version, program_options_sources,
                                cpp_args: program_options_args,
                                include_directories: inc,
                                dependencies: thread_dep)

boost_program_options_dep = declare_dependency(link_with: boost_program_options,
                                               include_directories: inc,
                                               compile_args: program_options_args +
                                                             ['-DBOOST_ALL_NO_LIB=1'])
