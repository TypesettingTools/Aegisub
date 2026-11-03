project('lpeg', 'c', version: '1.1.0', meson_version: '>=1.0.0')

# We can't easily pass the luajit dependency from Aegisub's meson.build to this subproject,
# so instead of building a separate library we just export extra sources.

inc = include_directories('.')

src = files(
  'lpcap.c',
  'lpcode.c',
  'lpcset.c',
  'lpprint.c',
  'lptree.c',
  'lpvm.c',
)

lpeg_dep = declare_dependency(include_directories: inc, sources: src)
