import json
import sys

host = sys.argv[1]
cpu_family = sys.argv[2]
outfile = sys.argv[3]

with open('meson-info/intro-targets.json', 'r') as f:
    targets = json.load(f)

def get_arguments(name):
    target = next(target for target in targets
                  if target['name'] == name)
    sources = next(sources for sources in target['target_sources']
                   if sources['language'] == 'c')
    return sources['compiler'] + sources['parameters']

def quote(arguments):
    return " ".join(json.dumps(arg) for arg in arguments)

obj_arguments = get_arguments("pkgdata") + ['-c']
lib_arguments = get_arguments("icutest") + ['-Wl,-Bsymbolic']

config = {
    'A': 'a',
    'LIBPREFIX': 'lib',
    'LIB_EXT_ORDER': '.',
    'COMPILE': quote(obj_arguments),
    'LIBFLAGS': '', # already included in COMPILE
    'LDICUDTFLAGS': '-nodefaultlibs -nostdlib',
    'RPATH_FLAGS': '',
    'BIR_LDFLAGS': '-Wl,-Bsymbolic',
    'AR': 'ar',
    'ARFLAGS': 'r',
    'RANLIB': 'ranlib',
    'LD_SONAME': '',
    'INSTALL_CMD': ''
}

if host == 'darwin':
    config['GENCCODE_ASSEMBLY_TYPE'] = '-a gcc-darwin'
    config['SO'] = 'dylib'
    config['SOBJ'] = 'dylib'
    config['GENLIB'] = quote(lib_arguments + ['-dynamiclib', '-dynamic'])
else:
    config['GENCCODE_ASSEMBLY_TYPE'] = '-a gcc'
    config['SO'] = 'so'
    config['SOBJ'] = 'so'
    config['GENLIB'] = quote(lib_arguments + ['-shared'])

with open(outfile, 'w') as f:
    for k, v in config.items():
        f.write("{}={}\n".format(k, v))
