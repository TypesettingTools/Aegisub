subdir('libresrc')

aegisub_src = files(
    'MatroskaParser.c',
    'aegisublocale.cpp',
    'ass_attachment.cpp',
    'ass_dialogue.cpp',
    'ass_entry.cpp',
    'ass_export_filter.cpp',
    'ass_exporter.cpp',
    'ass_file.cpp',
    'ass_karaoke.cpp',
    'ass_override.cpp',
    'ass_parser.cpp',
    'ass_style.cpp',
    'ass_style_storage.cpp',
    'async_video_provider.cpp',
    'audio_box.cpp',
    'audio_colorscheme.cpp',
    'audio_controller.cpp',
    'audio_display.cpp',
    'audio_karaoke.cpp',
    'audio_marker.cpp',
    'audio_player.cpp',
    'audio_provider_factory.cpp',
    'audio_renderer.cpp',
    'audio_renderer_spectrum.cpp',
    'audio_renderer_waveform.cpp',
    'audio_timing_dialogue.cpp',
    'audio_timing_karaoke.cpp',
    'auto4_base.cpp',
    'auto4_lua.cpp',
    'auto4_lua_assfile.cpp',
    'auto4_lua_dialog.cpp',
    'auto4_lua_progresssink.cpp',
    'base_grid.cpp',
    'charset_detect.cpp',
    'colorspace.cpp',
    'colour_button.cpp',
    'command/app.cpp',
    'command/audio.cpp',
    'command/automation.cpp',
    'command/command.cpp',
    'command/edit.cpp',
    'command/grid.cpp',
    'command/help.cpp',
    'command/keyframe.cpp',
    'command/recent.cpp',
    'command/subtitle.cpp',
    'command/time.cpp',
    'command/timecode.cpp',
    'command/tool.cpp',
    'command/video.cpp',
    'command/vis_tool.cpp',
    'compat.cpp',
    'context.cpp',
    'crash_writer.cpp',
    'dialog_about.cpp',
    'dialog_attachments.cpp',
    'dialog_automation.cpp',
    'dialog_autosave.cpp',
    'dialog_colorpicker.cpp',
    'dialog_detached_video.cpp',
    'dialog_dummy_video.cpp',
    'dialog_export.cpp',
    'dialog_export_ebu3264.cpp',
    'dialog_fonts_collector.cpp',
    'dialog_jumpto.cpp',
    'dialog_kara_timing_copy.cpp',
    'dialog_log.cpp',
    'dialog_paste_over.cpp',
    'dialog_progress.cpp',
    'dialog_properties.cpp',
    'dialog_resample.cpp',
    'dialog_search_replace.cpp',
    'dialog_selected_choices.cpp',
    'dialog_selection.cpp',
    'dialog_shift_times.cpp',
    'dialog_spellchecker.cpp',
    'dialog_style_editor.cpp',
    'dialog_style_manager.cpp',
    'dialog_styling_assistant.cpp',
    'dialog_text_import.cpp',
    'dialog_timing_processor.cpp',
    'dialog_translation.cpp',
    'dialog_version_check.cpp',
    'dialog_video_details.cpp',
    'dialog_video_properties.cpp',
    'export_fixstyle.cpp',
    'export_framerate.cpp',
    'fft.cpp',
    'font_file_lister.cpp',
    'frame_main.cpp',
    'gl_text.cpp',
    'gl_wrap.cpp',
    'grid_column.cpp',
    'help_button.cpp',
    'hotkey.cpp',
    'hotkey_data_view_model.cpp',
    'initial_line_state.cpp',
    'main.cpp',
    'menu.cpp',
    'mkv_wrap.cpp',
    'pen.cpp',
    'persist_location.cpp',
    'preferences.cpp',
    'preferences_base.cpp',
    'project.cpp',
    'resolution_resampler.cpp',
    'search_replace_engine.cpp',
    'selection_controller.cpp',
    'spellchecker.cpp',
    'spline.cpp',
    'spline_curve.cpp',
    'string_codec.cpp',
    'subs_controller.cpp',
    'subs_edit_box.cpp',
    'subs_edit_ctrl.cpp',
    'subs_preview.cpp',
    'subtitle_format.cpp',
    'subtitle_format_ass.cpp',
    'subtitle_format_ebu3264.cpp',
    'subtitle_format_encore.cpp',
    'subtitle_format_microdvd.cpp',
    'subtitle_format_mkv.cpp',
    'subtitle_format_srt.cpp',
    'subtitle_format_ssa.cpp',
    'subtitle_format_transtation.cpp',
    'subtitle_format_ttxt.cpp',
    'subtitle_format_txt.cpp',
    'subtitles_provider.cpp',
    'subtitles_provider_libass.cpp',
    'text_file_reader.cpp',
    'text_file_writer.cpp',
    'text_selection_controller.cpp',
    'thesaurus.cpp',
    'timeedit_ctrl.cpp',
    'toggle_bitmap.cpp',
    'toolbar.cpp',
    'tooltip_manager.cpp',
    'utils.cpp',
    'validators.cpp',
    'vector2d.cpp',
    'version.cpp',
    'video_box.cpp',
    'video_controller.cpp',
    'video_display.cpp',
    'video_frame.cpp',
    'video_out_gl.cpp',
    'video_provider_cache.cpp',
    'video_provider_dummy.cpp',
    'video_provider_manager.cpp',
    'video_provider_yuv4mpeg.cpp',
    'video_slider.cpp',
    'visual_feature.cpp',
    'visual_tool.cpp',
    'visual_tool_clip.cpp',
    'visual_tool_cross.cpp',
    'visual_tool_drag.cpp',
    'visual_tool_rotatexy.cpp',
    'visual_tool_rotatez.cpp',
    'visual_tool_scale.cpp',
    'visual_tool_vector_clip.cpp',
)

if host_machine.system() == 'darwin'
    aegisub_src += [
        'font_file_lister_coretext.mm',
        'osx/osx_utils.mm',
        'osx/retina_helper.mm',
    ]
elif host_machine.system() == 'windows'
    aegisub_src += [
        'avisynth_wrap.cpp',
        'font_file_lister_gdi.cpp',
        'libass_gdi_fontselect.cpp',
        'video_provider_avs.cpp',
    ]

    if cc.has_header('wingdi.h')
        deps += cc.find_library('gdi32', required: true)
    else
        error('Missing Windows SDK GDI Library (wingdi.h / gdi32.lib)')
    endif
        if cc.has_header('usp10.h')
        deps += cc.find_library('usp10', required: true)
    else
        error('Missing Windows SDK Uniscribe Library (usp10.h / usp10.lib)')
    endif

    res_inc = include_directories('bitmaps/windows')
    res_dep_files = files(
        'bitmaps/windows/icon.ico',
        'bitmaps/windows/eyedropper.cur',
    )
    windows = import('windows')
    if wx_dep.type_name() != 'not-found' # system wx
        raw_wx_windres_args = wx_dep.get_variable(configtool: 'rescomp').split()
        wx_windres_args = []
        foreach arg: raw_wx_windres_args
            if arg != 'windres'
                wx_windres_args += arg
            endif
        endforeach
        aegisub_src += windows.compile_resources('res/res.rc',
                                                 args: wx_windres_args,
                                                 depend_files: res_dep_files,
                                                 depends: version_h,
                                                 include_directories: [res_inc, version_inc])
    else # subproject
        wx_inc = wx.include_directories('wxmono')
        aegisub_src += windows.compile_resources('res/res.rc',
                                                 depend_files: res_dep_files,
                                                 depends: version_h,
                                                 include_directories: [res_inc, version_inc, wx_inc])
    endif
endif

if conf.has('WITH_FONTCONFIG')
    aegisub_src += 'font_file_lister_fontconfig.cpp'
endif

if conf.has('WITH_CSRI')
    aegisub_src += 'subtitles_provider_csri.cpp'
endif

opt_src = [
    ['ALSA', 'audio_player_alsa.cpp'],
    ['PortAudio', 'audio_player_portaudio.cpp'],
    ['PulseAudio', 'audio_player_pulse.cpp'],
    ['OpenAL', 'audio_player_openal.cpp'],
    ['OSS', 'audio_player_oss.cpp'],
    ['DirectSound', ['audio_player_dsound.cpp',
                     'audio_player_dsound2.cpp']],

    ['FFMS2', ['audio_provider_ffmpegsource.cpp',
               'video_provider_ffmpegsource.cpp',
               'ffmpegsource_common.cpp']],

    ['Hunspell', 'spellchecker_hunspell.cpp'],
]

foreach opt: opt_src
    if dep_avail.contains(opt[0])
        aegisub_src += opt[1]
    endif
endforeach

aegisub_cpp_pch = ['include/agi_pre.h']
aegisub_c_pch = ['include/agi_pre_c.h']

aegisub = executable('aegisub', aegisub_src, version_h, acconf,
                     link_with: [libresrc, libluabins, libaegisub],
                     include_directories: [libaegisub_inc, libresrc_inc, version_inc, deps_inc],
                     cpp_pch: aegisub_cpp_pch,
                     c_pch: aegisub_c_pch,
                     install: true,
                     install_dir: bindir,
                     dependencies: deps,
                     gui_app: true)
