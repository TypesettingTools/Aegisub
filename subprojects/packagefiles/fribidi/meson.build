project('fribidi', 'c', version: '1.0.10',
  meson_version : '>= 0.48')

# New release:
#   interface_age++
#   micro version++
#
# If any functions have been added:
#   interface_age = 0
#   interface_version++
#
# If binary backwards compatibility has been broken:
#   panic!

interface_age = 0
interface_version = 4

soversion = 0
libversion = '@0@.@1@.0'.format(soversion, interface_version, interface_age)

# C compiler. This is the cross compiler if we're cross-compiling
cc = meson.get_compiler('c')

if cc.get_id() == 'gcc' and cc.has_argument('-ansi')
  add_project_arguments('-ansi', language: 'c')
endif

# Symbol visibility
have_visibility_hidden = false
if host_machine.system() != 'windows'
  have_visibility_hidden = cc.has_argument('-fvisibility=hidden')
  if have_visibility_hidden
    add_project_arguments('-fvisibility=hidden', language: 'c')
  endif
endif

# Must explicitly make symbols public if default visibility is hidden
visibility_args = []
fribidi_static_cargs = []
if have_visibility_hidden
  visibility_args = ['-DFRIBIDI_ENTRY=__attribute__ ((visibility ("default")))']
else
  if host_machine.system() == 'windows'
    if get_option('default_library') == 'static'
      fribidi_static_cargs = ['-DFRIBIDI_LIB_STATIC']
    endif
  endif
endif

cdata = configuration_data()

# Checks for library functions
foreach f : ['memmove', 'memset', 'strdup']
  cdata.set('HAVE_' + f.to_upper(), cc.has_function(f))
endforeach

# Checks for header files
# Some HAVE_FOO defines need to be defined to either 1 or 0, others need to
# be defined or undefined. The code base is a bit inconsistent there.
foreach h : ['stdlib.h', 'string.h', 'memory.h']
  cdata.set10('HAVE_' + h.underscorify().to_upper(), cc.has_header(h))
endforeach
foreach h : ['strings.h', 'sys/times.h']
  cdata.set('HAVE_' + h.underscorify().to_upper(), cc.has_header(h))
endforeach

# Not entirely correct, but sufficient for us. Should move away from this
# ancient define and just include individual headers based on individual defs
if cc.has_header('strings.h')
  # define to 1 or leave undefined otherwise, don't simplify
  cdata.set('STDC_HEADERS', 1)
endif

# This is available pretty much everywhere
cdata.set('HAVE_STRINGIZE', 1)

if get_option('debug')
  cdata.set('DEBUG', 1)
endif

no_deprecated = not get_option('deprecated')
cdata.set('FRIBIDI_NO_DEPRECATED', no_deprecated)

# write config.h
config_h = configure_file(output: 'config.h', configuration: cdata)

incs = include_directories('.', 'lib', 'gen.tab')

subdir('gen.tab')
subdir('lib')
if get_option('bin')
  subdir('bin')
endif
if get_option('tests')
   subdir('test')
endif
if get_option('docs')
  subdir('doc')
endif

# fribidi.pc
pkg = import('pkgconfig')
pkg.generate(name: 'GNU FriBidi',
  filebase: 'fribidi',
  description: 'Unicode Bidirectional Algorithm Library',
  libraries: libfribidi, 
  subdirs: 'fribidi',
  version: meson.project_version())
