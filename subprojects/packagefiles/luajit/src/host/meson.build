cpu_family = host_machine.cpu_family()
if cpu_family == 'x86_64'
    dynasm_arch = 'x64'
elif cpu_family == 'aarch64'
    dynasm_arch = 'arm64'
else
    dynasm_arch = cpu_family
endif

dynasm_dasc = files('../vm_@0@.dasc'.format(dynasm_arch))
dasm = [minilua, files('../../dynasm/dynasm.lua')]

# BUG: meson does not resolve paths correctly for subprojects
hpre = '#include "@0@/../lj_arch.h"'.format(meson.current_source_dir())

checkdefs = [
    ['LJ_LE', '1', ['-D', 'ENDIAN_LE']],
    ['LJ_ARCH_BITS', '64', ['-D', 'P64']],
    ['LJ_HASJIT', '1', ['-D', 'JIT']],
    ['LJ_HASFFI', '1', ['-D', 'FFI']],
    ['LJ_DUALNUM', '1', ['-D', 'DUALNUM']],
    ['LJ_ARCH_HASFPU', '1', ['-D', 'FPU']],
    ['LJ_ABI_SOFTFP', '0', ['-D', 'HFABI']],
    ['LJ_NO_UNWIND', '1', ['-D', 'NO_UNWIND']],
    ['LJ_ABI_PAUTH', '1', ['-D', 'PAUTH']],
]

foreach def: checkdefs
    if cc.get_define(def[0], prefix: hpre) == def[1]
        dasm += def[2]
    endif
endforeach

if host_machine.system() == 'windows'
  dasm += ['-D', 'WIN']
endif

dasm += ['-D', 'VER=' + cc.get_define('LJ_ARCH_VERSION', prefix: hpre)]

buildvm_src = files(
    'buildvm.c',
    'buildvm_asm.c',
    'buildvm_peobj.c',
    'buildvm_lib.c',
    'buildvm_fold.c',
)

buildvm_arch = custom_target('buildvm_arch.h',
                             command: dasm + ['-o', '@OUTPUT@', '@INPUT@'],
                             input: dynasm_dasc,
                             output: 'buildvm_arch.h')

buildvm = executable('buildvm', buildvm_src, buildvm_arch, luajit_h,
                     dependencies: system_deps,
                     include_directories: src_inc,
                     native: true)
