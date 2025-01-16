fs = import('fs')

respack = find_program(meson.project_source_root() / 'tools/respack.py')
combine_config = find_program(meson.project_source_root() / 'tools/combine-config.py')

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
                  command: [respack, '@INPUT@', '@OUTPUT@', meson.current_source_dir() / '../bitmaps'],
                  input: files(bitmap_manifest),
                  depend_files: bitmap_files,
                  output: ['bitmap.cpp', 'bitmap.h'])
]

conf_platform = ''
hotkey_platform = ''
menu_platform = 'default_menu_platform.json'
if host_machine.system() == 'windows'
    conf_platform = 'win/default_config.json'
elif host_machine.system() == 'darwin'
    conf_platform = 'osx/default_config.json'
    hotkey_platform = 'osx/default_hotkey.json'
    menu_platform = 'osx/default_menu.json'
endif

config_resources = [
    ['default_config.json', conf_platform],
    ['default_hotkey.json', hotkey_platform],
    ['default_menu.json', menu_platform],
]

merged_resources = {}

foreach res : config_resources
    base = res[0]
    basename = base.split('.')[0]
    outname = basename + '_platform_merged.json'

    if res[1] == ''
        platform_json = [base, '']
    else
        target = custom_target(outname,
                               command: [combine_config, '@INPUT@', '@OUTPUT@'],
                               input: [base, res[1]],
                               output: outname)

        platform_json = [target, target.full_path()]
    endif

    merged_resources += {basename: platform_json}
endforeach

conf_build_json = configure_file(input: 'default_config_build.json.in',
                                 output: '@BASENAME@',
                                 configuration: conf_meson)

conf_full_json = custom_target('default_config.json',
                               command: [combine_config, '@INPUT@', '@OUTPUT@'],
                               input: [merged_resources['default_config'][0], conf_build_json],
                               output: 'default_config.json')


merged_resources += {'default_config': [conf_full_json, conf_full_json.full_path()]}

resmanifest_files = []
foreach name, target : merged_resources
    resmanifest_files += target[0]
endforeach

resmanifest_base = files('manifest.respack.in')

foreach rfile : fs.read(resmanifest_base).strip().split('\n')
    rfile_s = rfile.strip()
    # Filter out the files we've generated ourselves
    if not rfile_s.contains('|') and fs.is_file(rfile_s)
        resmanifest_files += files(rfile_s)
    endif
endforeach

resmanifest = configure_file(input: resmanifest_base,
                             output: '@BASENAME@',
                             configuration: {
                                 'DEFAULT_CONFIG_PATH': merged_resources['default_config'][1],
                                 'DEFAULT_HOTKEY_PATH': merged_resources['default_hotkey'][1],
                                 'DEFAULT_MENU_PATH': merged_resources['default_menu'][1],
                             })

resrc += custom_target('default_config.{cpp,h}',
                        command: [respack, '@INPUT0@', '@OUTPUT@', meson.current_source_dir()],
                        input: [resmanifest] + resmanifest_files,
                        output: ['default_config.cpp', 'default_config.h'])


libresrc_inc = include_directories('.')
libresrc = static_library('resrc', 'libresrc.cpp', resrc, include_directories: deps_inc, dependencies: deps)
