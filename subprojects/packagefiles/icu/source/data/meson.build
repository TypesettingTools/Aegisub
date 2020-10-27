icudata_lst = custom_target('icudata.lst',
                            # not sure how to ensure that @PRIVATE_DIR@ exists
                            #command: [icupkg, '-d', '@PRIVATE_DIR@', '--list', '-x', '*', '@INPUT@',  '-o', '@OUTPUT@'],
                            command: [icupkg, '-d', '@OUTDIR@', '--list', '-x', '*', '@INPUT@',  '-o', '@OUTPUT@'],
                            input: files('in/icudt67l.dat'),
                            output: 'icudata.lst')

is_static = get_option('default_library') == 'static'

if host_machine.system() == 'windows' # only msvc supported
    data_lib_static = custom_target(
        'icudata_static', command: [pkgdata, '-q', '-c', '-e', 'icudt67', '-L', 'icudata_static',
                                    '-d', '@OUTDIR@', '-s', '@OUTDIR@', '-p', 'icudt67l',
                                    '-m', 'static', '@INPUT@'],
        input: icudata_lst, output: 'icudata_static.lib')
else
    if host_machine.system() == 'darwin'
        suffix = is_static ? '.a' : '.dylib'
    else
        suffix = is_static ? '.a' : '.so'
    endif
    suffix = '.a'

    icupkg_inc = custom_target('icupkg.inc',
                               command: [python3, '@INPUT@', host_machine.system(),
                                         host_machine.cpu_family(), '@OUTPUT@'],
                               input: 'generate_icupkg_inc.py',
                               output: 'icupkg.inc')
    data_lib_static = custom_target(
        'icudata_static', command: [pkgdata, '-q', '-c', '-e', 'icudt67', '-L', 'icudata_static',
                                    '-d', '@OUTDIR@', '-s', '@OUTDIR@',
                                    '-p', 'icudt67l', '-O', icupkg_inc,
                                    '-m', 'static', '@INPUT@'],
        input: icudata_lst, output: 'libicudata_static' + suffix)
endif

if is_static
    data_lib = data_lib_static
else
    # convert to proper library object to make sure rpath gets set properly
    data_lib = library('icudata', link_whole: data_lib_static)
endif

# -nodefaultlibs -nostdlib  for stubdata
