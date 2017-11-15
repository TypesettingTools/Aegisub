minilua = executable('minilua', 'minilua.c',
                     dependencies : system_deps_native,
                     native : true)

if host_machine.cpu_family() == 'x86_64'
    dynasm_arch = 'x86'
else
    dynasm_arch = host_machine.cpu_family()
endif

dynasm_dasc = files('../vm_@0@.dasc'.format(dynasm_arch))
dasm = [minilua, files('../../dynasm/dynasm.lua')]

# BUG: meson does not resolve paths correctly for subprojects
hpre = '#include "@0@/../lj_arch.h"'.format(meson.current_source_dir())

checkdefs = [
    ['LJ_ARCH_BITS', '64', ['-D', 'P64']],
    ['LJ_HASJIT', '1', ['-D', 'JIT']],
    ['LJ_HASFFI', '1', ['-D', 'FFI']],
    ['LJ_DUALNUM', '1', ['-D', 'DUALNUM']],
]

if host_machine.cpu_family() == 'x86'
    checkdefs += ['__SSE2__', '1', ['-D', 'SSE']]
endif

foreach def : checkdefs
    if cc.get_define(def[0], prefix : hpre) == def[1]
        dasm += def[2]
    endif
endforeach

buildvm_src = files(
    'buildvm.c',
    'buildvm_asm.c',
    'buildvm_peobj.c',
    'buildvm_lib.c',
    'buildvm_fold.c',
)

buildvm_arch = custom_target('buildvm_arch.h',
                             command : dasm + ['-o', '@OUTPUT@', '@INPUT@'],
                             input : dynasm_dasc,
                             output : 'buildvm_arch.h')

buildvm = executable('buildvm', buildvm_src, buildvm_arch,
                     dependencies : system_deps_native,
                     include_directories : src_inc,
                     native : true)
