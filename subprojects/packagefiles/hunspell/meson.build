project('Hunspell', 'cpp', version: '1.7.0', meson_version: '>=0.57.0')

subdir('src/hunspell')

inc = include_directories('src')

hunspell_dep = declare_dependency(link_with: hunspell,
                                  include_directories: inc,
                                  compile_args: compile_args)
