thread_args = [is_static ? '-DBOOST_THREAD_BUILD_LIB=1' : '-DBOOST_THREAD_BUILD_DLL=1']

if host_machine.system() == 'windows'
    thread_sources = files([
        'src/win32/thread.cpp',
        'src/win32/tss_dll.cpp',
        'src/win32/tss_pe.cpp',
        'src/win32/thread_primitives.cpp',
        'src/future.cpp'
    ])
    thread_args += ['-DWIN32_LEAN_AND_MEAN', '-DBOOST_USE_WINDOWS_H', '-DBOOST_THREAD_WIN32',
                    '-DBOOST_THREAD_USES_CHRONO']
else
    thread_sources = files([
        'src/pthread/thread.cpp',
        'src/pthread/once.cpp',
        'src/future.cpp'
    ])
    thread_args += ['-DBOOST_THREAD_POSIX', '-DBOOST_THREAD_DONT_USE_CHRONO']
endif

boost_thread = library('boost_thread' +'.'+ prj_version, thread_sources,
                       include_directories: inc,
                       cpp_args: thread_args,
                       dependencies: [thread_dep, boost_chrono_dep])

boost_thread_dep = declare_dependency(link_with: boost_thread, include_directories: inc,
                                      compile_args: [is_static ? '-DBOOST_THREAD_BUILD_LIB=1'
                                                               : '-DBOOST_THREAD_BUILD_DLL=1',
                                                     '-DBOOST_ALL_NO_LIB=1'])
