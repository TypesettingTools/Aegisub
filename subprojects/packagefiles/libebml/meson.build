project('libebml', 'cpp', version: '1.4.5', default_options: ['cpp_std=c++14'])

sources = files(
  'src/Debug.cpp', 'src/EbmlBinary.cpp', 'src/EbmlContexts.cpp', 'src/EbmlCrc32.cpp',
  'src/EbmlDate.cpp', 'src/EbmlDummy.cpp', 'src/EbmlElement.cpp', 'src/EbmlFloat.cpp',
  'src/EbmlHead.cpp', 'src/EbmlMaster.cpp', 'src/EbmlSInteger.cpp', 'src/EbmlStream.cpp',
  'src/EbmlString.cpp', 'src/EbmlSubHead.cpp', 'src/EbmlUInteger.cpp',
  'src/EbmlUnicodeString.cpp', 'src/EbmlVersion.cpp', 'src/EbmlVoid.cpp',
  'src/IOCallback.cpp', 'src/MemIOCallback.cpp', 'src/MemReadIOCallback.cpp',
  'src/SafeReadIOCallback.cpp', 'src/StdIOCallback.cpp',
)

ebml = static_library('ebml', sources,
  include_directories: include_directories('.'),
  cpp_args: host_machine.system() == 'windows' ? ['-D_CRT_SECURE_NO_WARNINGS'] : [],
  install: false)
libebml_dep = declare_dependency(link_with: ebml, include_directories: include_directories('.'))
