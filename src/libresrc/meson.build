fs = import('fs')

respack = find_program(meson.project_source_root() / 'tools/respack.py')

bitmap_manifest = '../bitmaps/manifest.respack'
bitmap_files = []
foreach bfile : fs.read(bitmap_manifest).strip().split('\n')
    bfile_s = bfile.strip()
    if bfile_s != ''
        bitmap_files += files('../bitmaps' / bfile_s)
    endif
endforeach

resrc = [
    custom_target('bitmap.{cpp,h}',
                  command: [respack, '@INPUT@', '@OUTPUT@'],
                  input: files(bitmap_manifest),
                  depend_files: bitmap_files,
                  output: ['bitmap.cpp', 'bitmap.h'])
]

conf_platform_json = configure_file(input: 'default_config_platform.json.in',
                                    output: '@BASENAME@',
                                    configuration: conf_platform)

if host_machine.system() == 'darwin'
    resmanifest = 'manifest_osx.respack'
else
    resmanifest = 'manifest.respack'
endif

resmanifest_files = [conf_platform_json]
# Filter out the files we've generated ourselves
foreach rfile : fs.read(resmanifest).strip().split('\n')
    rfile_s = rfile.strip()
    if fs.is_file(rfile_s)
        resmanifest_files += files(rfile_s)
    endif
endforeach

resrc += custom_target('default_config.{cpp,h}',
                        command: [respack, '@INPUT0@', '@OUTPUT@'],
                        input: [files(resmanifest)],
                        depend_files: resmanifest_files,
                        output: ['default_config.cpp', 'default_config.h'])


libresrc_inc = include_directories('.')
libresrc = static_library('resrc', 'libresrc.cpp', resrc, include_directories: deps_inc, dependencies: deps)
