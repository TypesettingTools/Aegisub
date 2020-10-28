minilua = executable('minilua', 'minilua.c',
                     dependencies: system_deps,
                     native: true)

if host_machine.cpu_family() == 'x86_64'
    dynasm_arch = 'x86'
else
    dynasm_arch = host_machine.cpu_family()
endif

dynasm_dasc = files('../vm_@0@.dasc'.format(dynasm_arch))
dasm = [minilua, files('../../dynasm/dynasm.lua')]

if cc.get_id() == 'msvc'
    # the cl.exe preprocessor seemingly removes/expands macros, so hardcode flags
    dasm += ['-D', 'WIN', '-D', 'JIT', '-D', 'FFI']
    if host_machine.cpu_family() == 'x86_64'
        dasm += ['-D', 'P64']
    endif
else
    # BUG: meson does not resolve paths correctly for subprojects
    hpre = '#include "@0@/../lj_arch.h"'.format(meson.current_source_dir())

    checkdefs = [
        ['LJ_ARCH_BITS', '64', ['-D', 'P64']],
        ['LJ_HASJIT', '1', ['-D', 'JIT']],
        ['LJ_HASFFI', '1', ['-D', 'FFI']],
    ]

    if host_machine.cpu_family() == 'x86'
        checkdefs += ['__SSE2__', '1', ['-D', 'SSE']]
    endif

    foreach def: checkdefs
        if cc.get_define(def[0], prefix: hpre) == def[1]
            dasm += def[2]
        endif
    endforeach
endif

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

buildvm = executable('buildvm', buildvm_src, buildvm_arch,
                     dependencies: system_deps,
                     include_directories: src_inc,
                     native: true)
