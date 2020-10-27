project('ffms2', 'cpp', version: '2.40.0')

deps = []

deps += dependency('zlib')
deps += dependency('libavformat')
deps += dependency('libavcodec')
deps += dependency('libswscale')
deps += dependency('libavutil')
deps += dependency('libswresample')

args = ['-D_FILE_OFFSET_BITNS=64', '-DFFMS_EXPORTS', '-D__STDC_CONSTANT_MACROS']
usage_args = []

if get_option('default_library') == 'static'
    usage_args += '-DFFMS_STATIC'
endif

ffms2_sources = [
    'src/core/audiosource.cpp',
    #'src/core/audiosource.h',
    'src/core/ffms.cpp',
    'src/core/filehandle.cpp',
    #'src/core/filehandle.h',
    'src/core/indexing.cpp',
    #'src/core/indexing.h',
    'src/core/track.cpp',
    #'src/core/track.h',
    'src/core/utils.cpp',
    #'src/core/utils.h',
    'src/core/videosource.cpp',
    #'src/core/videosource.h',
    'src/core/videoutils.cpp',
    #'src/core/videoutils.h',
    'src/core/zipfile.cpp',
    #'src/core/zipfile.h',
]

deps_inc = include_directories('include')

ffms2 = library('ffms2', ffms2_sources, include_directories: deps_inc, dependencies: deps,
                cpp_args: args + usage_args)
ffms2_dep = declare_dependency(link_with: ffms2,
                               include_directories: deps_inc,
                               compile_args: usage_args)
