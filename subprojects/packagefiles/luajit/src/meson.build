ljlib_src = files(
    'lib_base.c',
    'lib_bit.c',
    'lib_buffer.c',
    'lib_debug.c',
    'lib_ffi.c',
    'lib_io.c',
    'lib_jit.c',
    'lib_math.c',
    'lib_os.c',
    'lib_package.c',
    'lib_string.c',
    'lib_table.c',
)

ljcore_src = files(
    'lib_aux.c',
    'lib_init.c',
    'lj_alloc.c',
    'lj_api.c',
    'lj_asm.c',
    'lj_assert.c',
    'lj_bc.c',
    'lj_bcread.c',
    'lj_bcwrite.c',
    'lj_buf.c',
    'lj_carith.c',
    'lj_ccall.c',
    'lj_ccallback.c',
    'lj_cconv.c',
    'lj_cdata.c',
    'lj_char.c',
    'lj_clib.c',
    'lj_cparse.c',
    'lj_crecord.c',
    'lj_ctype.c',
    'lj_debug.c',
    'lj_dispatch.c',
    'lj_err.c',
    'lj_ffrecord.c',
    'lj_func.c',
    'lj_gc.c',
    'lj_gdbjit.c',
    'lj_ir.c',
    'lj_lex.c',
    'lj_lib.c',
    'lj_load.c',
    'lj_mcode.c',
    'lj_meta.c',
    'lj_obj.c',
    'lj_opt_dce.c',
    'lj_opt_fold.c',
    'lj_opt_loop.c',
    'lj_opt_mem.c',
    'lj_opt_narrow.c',
    'lj_opt_sink.c',
    'lj_opt_split.c',
    'lj_parse.c',
    'lj_prng.c',
    'lj_profile.c',
    'lj_record.c',
    'lj_serialize.c',
    'lj_snap.c',
    'lj_state.c',
    'lj_str.c',
    'lj_strfmt.c',
    'lj_strfmt_num.c',
    'lj_strscan.c',
    'lj_tab.c',
    'lj_trace.c',
    'lj_udata.c',
    'lj_vmevent.c',
    'lj_vmmath.c',
)

src_inc = include_directories('.')

subdir('host')

hdrgen = [
    'bcdef',
    'ffdef',
    'libdef',
    'recdef',
]

genheaders = []
foreach h: hdrgen
    genheaders += custom_target(h,
                                command: [buildvm, '-m', h, '-o', '@OUTPUT@', ljlib_src],
                                output: 'lj_@0@.h'.format(h))
endforeach

genheaders += custom_target('folddef',
                            command: [buildvm, '-m', 'folddef', '-o', '@OUTPUT@', files('lj_opt_fold.c')],
                            output: 'lj_folddef.h')

ljvm = custom_target(ljvm_bout,
                     command: [buildvm, '-m', ljvm_mode, '-o', '@OUTPUT@'],
                     output: ljvm_bout)

luajit_lib = static_library('luajit', ljlib_src, ljcore_src, genheaders, ljvm,
                            dependencies: system_deps)
