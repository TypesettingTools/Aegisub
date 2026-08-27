project('libmatroska', 'cpp', version: '1.7.1', default_options: ['cpp_std=c++14'])

libebml_dep = dependency('libebml', version: '>=1.4.3')
sources = files(
  'src/FileKax.cpp', 'src/KaxAttached.cpp', 'src/KaxAttachments.cpp', 'src/KaxBlock.cpp',
  'src/KaxBlockData.cpp', 'src/KaxCluster.cpp', 'src/KaxContexts.cpp', 'src/KaxCues.cpp',
  'src/KaxCuesData.cpp', 'src/KaxInfoData.cpp', 'src/KaxSeekHead.cpp', 'src/KaxSegment.cpp',
  'src/KaxSemantic.cpp', 'src/KaxTracks.cpp', 'src/KaxVersion.cpp',
)
matroska = static_library('matroska', sources,
  include_directories: include_directories('.'), dependencies: libebml_dep,
  cpp_args: host_machine.system() == 'windows' ? ['-D_CRT_SECURE_NO_WARNINGS'] : [],
  install: false)
libmatroska_dep = declare_dependency(link_with: matroska,
  include_directories: include_directories('.'), dependencies: libebml_dep)
