python3 = import('python').find_installation('python3')

incdir = include_directories('common', 'i18n')
toolinc = include_directories('tools/toolutil')
ctestinc = include_directories('tools/ctestfw')

subdir('stubdata')

# get sources and dependencies for required libraries
subdir('common')
subdir('i18n')
subdir('io')
subdir('tools/toolutil')

# create [library name]_lib_stub artifacts linking against stub data
icu_libs = ['uc', 'i18n', 'io', 'tu']
foreach lib: icu_libs
    liblinks = []
    foreach link: get_variable(lib + '_link')
        liblinks += get_variable(link + '_lib_stub')
    endforeach
    set_variable(lib + '_lib_stub', library(
        'icu' + lib + '_stub',
        get_variable(lib + '_sources'),
        include_directories: incdir,
        c_args: get_variable(lib + '_args'),
        cpp_args: get_variable(lib + '_args'),
        link_with: liblinks + [stubdata_lib],
        dependencies: get_variable(lib + '_deps')
    ))
endforeach

# build tools and full data library using the libraries linked with the stub library
subdir('tools/icupkg')
subdir('tools/pkgdata')
subdir('data')

# rebuild libraries linking with the full data library
foreach lib: icu_libs
    liblinks = []
    foreach link: get_variable(lib + '_link')
        liblinks += get_variable(link + '_lib')
    endforeach
    set_variable(lib + '_lib', library(
        'icu' + lib,
        get_variable(lib + '_sources'),
        include_directories: incdir,
        c_args: get_variable(lib + '_args'),
        cpp_args: get_variable(lib + '_args'),
        link_with: liblinks + [data_lib],
        dependencies: get_variable(lib + '_deps')
    ))
endforeach

# icutest not needed for bootstrapping, link with non-boostrap libs
subdir('tools/ctestfw')

icudata_dep = declare_dependency(link_with: [data_lib],
    compile_args: usage_args,
    include_directories: incdir)

icuuc_dep = declare_dependency(link_with: [uc_lib],
    compile_args: usage_args,
    include_directories: incdir)

icuio_dep = declare_dependency(link_with: [io_lib],
    compile_args: usage_args,
    include_directories: incdir)

icui18n_dep = declare_dependency(link_with: [i18n_lib],
    compile_args: usage_args,
    include_directories: incdir)

icutu_dep = declare_dependency(link_with: [tu_lib],
    compile_args: usage_args,
    include_directories: incdir)

icutest_dep = declare_dependency(link_with: [test_lib],
    compile_args: usage_args,
    include_directories: [incdir, ctestinc])
