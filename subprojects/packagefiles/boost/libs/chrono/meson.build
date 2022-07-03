chrono_sources = files([
    'src/chrono.cpp',
    'src/process_cpu_clocks.cpp',
    'src/thread_clock.cpp'
])

boost_chrono = library('boost_chrono' +'.'+ prj_version, chrono_sources,
                       include_directories: inc,
                       cpp_args: is_static ? '-DBOOST_All_STATIC_LINK=1' : '-DBOOST_All_DYN_LINK=1',
                       dependencies: thread_dep)

boost_chrono_dep = declare_dependency(link_with: boost_chrono, include_directories: inc,
                                      compile_args: [is_static ? '-DBOOST_CHRONO_STATIC_LINK=1'
                                                               : '-DBOOST_CHRONO_DYN_LINK=1',
                                                     '-DBOOST_ALL_NO_LIB=1'])
