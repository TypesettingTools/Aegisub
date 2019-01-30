respack = find_program('../../tools/respack.py')

resrc = [
    custom_target('bitmap.{cpp,h}',
                  command: [respack, '@INPUT@', '@OUTPUT@'],
                  input: files('../bitmaps/manifest.respack'),
                  output: ['bitmap.cpp', 'bitmap.h'])
]

conf_platform_json = configure_file(input: 'default_config_platform.json.in',
                                    output: '@BASENAME@',
                                    configuration: conf_platform)

if host_machine.system() == 'darwin'
    resrc += custom_target('default_config.{cpp,h}',
                           command: [respack, '@INPUT0@', '@OUTPUT@'],
                           input: [files('manifest_osx.respack'), conf_platform_json],
                           output: ['default_config.cpp', 'default_config.h'])
else
    resrc += custom_target('default_config.{cpp,h}',
                           command: [respack, '@INPUT0@', '@OUTPUT@'],
                           input: [files('manifest.respack'), conf_platform_json],
                           output: ['default_config.cpp', 'default_config.h'])
endif


libresrc_inc = include_directories('.')
libresrc = static_library('resrc', 'libresrc.cpp', resrc, dependencies: deps)
