filesystem_sources = files([
    'src/codecvt_error_category.cpp',
    'src/exception.cpp',
    'src/directory.cpp',
    'src/operations.cpp',
    'src/path.cpp',
    'src/path_traits.cpp',
    'src/portability.cpp',
    'src/unique_path.cpp',
    'src/utf8_codecvt_facet.cpp',
])

filesystem_args = ['-DBOOST_FILESYSTEM_SOURCE',
                   # normally involves a check for std::atomic_ref
                   '-DBOOST_FILESYSTEM_NO_CXX20_ATOMIC_REF=1',
                   is_static ? '-DBOOST_FILESYSTEM_STATIC_LINK=1' : '-DBOOST_FILESYSTEM_DYN_LINK=1']

filesystem_deps = []
if host_machine.system() == 'windows'
    filesystem_sources += ['src/windows_file_codecvt.cpp']
    filesystem_args += ['-DBOOST_USE_WINDOWS_H', '-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX']
    filesystem_args += ['-D_SCL_SECURE_NO_WARNINGS', '-D_SCL_SECURE_NO_DEPRECATE', '-D_CRT_SECURE_NO_WARNINGS', '-D_CRT_SECURE_NO_DEPRECATE']

    bcrypt_dep = dependency('bcrypt', required: false)
    if bcrypt_dep.found()
        filesystem_deps += bcrypt_dep
        filesystem_args += '-DBOOST_FILESYSTEM_HAS_BCRYPT'
    else
        filesystem_deps += cpp.find_library('advapi32')
        filesystem_args += '-DBOOST_FILESYSTEM_HAS_WINCRYPT'
    endif
endif

boost_filesystem = library('boost_filesystem', filesystem_sources,
                           include_directories: inc,
                           cpp_args: filesystem_args,
                           dependencies: [thread_dep, filesystem_deps])

boost_filesystem_dep = declare_dependency(link_with: boost_filesystem, include_directories: inc,
                                          compile_args: [is_static ? '-DBOOST_FILESYSTEM_STATIC_LINK=1'
                                                                   : '-DBOOST_FILESYSTEM_DYN_LINK=1',
                                                         '-DBOOST_ALL_NO_LIB=1'])
