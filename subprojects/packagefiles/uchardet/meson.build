project('uchardet', 'cpp', version: '0.0.7', meson_version: '>=0.57.0')

inc = include_directories('src')

cpp = meson.get_compiler('cpp')

if cpp.has_argument('-msse2') and cpp.has_argument('-mfpmath=sse')
    add_project_arguments('-msse2', '-mfpmath=sse', language: 'cpp')
elif cpp.has_argument('-ffloat-store')
    add_project_arguments('-ffloat-store', language: 'cpp')
endif

subdir('src')

uchardet_dep = declare_dependency(link_with: uchardet,
                                  include_directories: inc)
