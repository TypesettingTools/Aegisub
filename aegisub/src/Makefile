include ../Makefile.inc

PROGRAM = aegisub-3.0
PROGRAM_INSTALL = yes

PRECOMPILED_HEADER_NAME=agi_pre.h

SUBDIRS = command libresrc

CXXFLAGS += -DAEGISUB -I. -I..  -Iinclude -I../libaegisub/include $(CFLAGS_PTHREAD) $(CPPFLAGS_WX)

LDFLAGS += -L../libaegisub -laegisub-3.0
LDFLAGS += $(CFLAGS_DEBUG) $(CFLAGS_PROFILE) $(LDFLAGS_CCMALLOC) $(LDFLAGS_EFENCE)
LDFLAGS += $(LDFLAGS_GL) $(LDFLAGS_PTHREAD) $(LDFLAGS_WX) $(LDFLAGS_FREETYPE) $(LDFLAGS_FONTCONFIG)
LDFLAGS_POST += libresrc/libresrc.a $(LDFLAGS_UCHARDET) command/aegisub_command.a

ifdef BUILD_DARWIN
LDFLAGS += -lz
endif


###############
# AUDIO PLAYERS
###############

ifeq (yes, $(HAVE_ALSA))
SRC_OPT += audio_player_alsa.cpp
audio_player_alsa.o: CXXFLAGS += $(CFLAGS_ALSA)
LDFLAGS += $(LDFLAGS_ALSA)
endif

ifeq (yes, $(HAVE_PORTAUDIO))
SRC_OPT += audio_player_portaudio.cpp
audio_player_portaudio.o: CXXFLAGS += $(CFLAGS_PORTAUDIO)
LDFLAGS += $(LDFLAGS_PORTAUDIO)
endif

ifeq (yes, $(HAVE_PULSEAUDIO))
SRC_OPT += audio_player_pulse.cpp
audio_player_pulse.o: CXXFLAGS += $(CFLAGS_LIBPULSE)
LDFLAGS += $(LDFLAGS_LIBPULSE)
endif

ifeq (yes, $(HAVE_OPENAL))
SRC_OPT += audio_player_openal.cpp
audio_player_openal.o: CXXFLAGS += $(CFLAGS_OPENAL)
LDFLAGS += $(LDFLAGS_OPENAL)
endif

ifeq (yes, $(HAVE_OSS))
SRC_OPT += audio_player_oss.cpp
endif


#######################
# AUDIO / VIDEO SUPPORT
#######################
ifeq (yes, $(HAVE_PROVIDER_FFMPEGSOURCE))
SRC_OPT += audio_provider_ffmpegsource.cpp video_provider_ffmpegsource.cpp ffmpegsource_common.cpp
audio_provider_ffmpegsource.o video_provider_ffmpegsource.o ffmpegsource_common.o: \
	CXXFLAGS += $(CFLAGS_FFMPEGSOURCE) $(CFLAGS_LIBAVFORMAT) $(CFLAGS_LIBAVCODEC) $(CFLAGS_LIBSWSCALE) $(CFLAGS_LIBAVUTIL) $(CFLAGS_LIBPOSTPROC)
ffmpegsource_common.o: CXXFLAGS += -D__STDC_FORMAT_MACROS
LDFLAGS += $(LDFLAGS_FFMPEGSOURCE) $(LDFLAGS_LIBAVFORMAT) $(LDFLAGS_LIBAVCODEC) $(LDFLAGS_LIBSWSCALE) $(LDFLAGS_LIBAVUTIL) $(LDFLAGS_LIBPOSTPROC)
LDFLAGS_POST += $(LDFLAGS_FFMPEGSOURCE)
endif


###########
# SUBTITLES
###########
ifeq (yes, $(WITH_LIBASS))
SRC_OPT += subtitles_provider_libass.cpp
subtitles_provider_libass.o: CXXFLAGS += $(CFLAGS_LIBASS)  $(CFLAGS_ICONV)
LDFLAGS += $(LDFLAGS_LIBASS) $(LDFLAGS_ICONV) $(LDFLAGS_FONTCONFIG)
LDFLAGS_POST += $(LDFLAGS_LIBASS)
endif


############
# AUTOMATION
############
ifeq (yes, $(HAVE_AUTO4_LUA))
SRC_OPT += auto4_lua.cpp auto4_lua_assfile.cpp auto4_lua_dialog.cpp auto4_lua_scriptreader.cpp
auto4_lua.o auto4_lua_assfile.o auto4_lua_dialog.o auto4_lua_scriptreader.o: CXXFLAGS += $(CFLAGS_LUA)
LDFLAGS += $(LDFLAGS_LUA)
endif


##############
# MISCELLANOUS
##############
ifeq (yes, $(HAVE_HUNSPELL))
SRC_OPT += spellchecker_hunspell.cpp
spellchecker_hunspell.o: CXXFLAGS += $(CFLAGS_HUNSPELL)
LDFLAGS += $(LDFLAGS_HUNSPELL)
endif


#####################
# SOURCE-LEVEL CFLAGS
#####################
audio_player.o:					CXXFLAGS += $(CFLAGS_ALSA) $(CFLAGS_PORTAUDIO) $(CFLAGS_LIBPULSE) $(CFLAGS_OPENAL)
audio_provider.o:				CXXFLAGS += $(CFLAGS_FFMPEGSOURCE)
auto4_base.o:					CXXFLAGS += $(CFLAGS_FREETYPE)
charset_detect.o:				CXXFLAGS += -D_X86_
font_file_lister_fontconfig.o:	CXXFLAGS += $(CFLAGS_FONTCONFIG) $(CFLAGS_FREETYPE)
font_file_lister.o:				CXXFLAGS += $(CFLAGS_FREETYPE)
text_file_reader.o:				CXXFLAGS += -D_X86_
video_provider_manager.o:		CXXFLAGS += $(CFLAGS_FFMPEGSOURCE)


# Ensure any optional source files above are added for compilation.
SRC += $(SRC_OPT)

SRC += \
	MatroskaParser.c \
	aegisublocale.cpp \
	ass_attachment.cpp \
	ass_dialogue.cpp \
	ass_entry.cpp \
	ass_export_filter.cpp \
	ass_exporter.cpp \
	ass_file.cpp \
	ass_karaoke.cpp \
	ass_override.cpp \
	ass_style.cpp \
	ass_style_storage.cpp \
	ass_time.cpp \
	audio_box.cpp \
	audio_controller.cpp \
	audio_colorscheme.cpp \
	audio_display.cpp \
	audio_karaoke.cpp \
	audio_player.cpp \
	audio_provider.cpp \
	audio_provider_convert.cpp \
	audio_provider_downmix.cpp \
	audio_provider_hd.cpp \
	audio_provider_pcm.cpp \
	audio_provider_ram.cpp \
	audio_renderer.cpp \
	audio_renderer_spectrum.cpp \
	audio_renderer_waveform.cpp \
	audio_timing_dialogue.cpp \
	auto4_base.cpp \
	avisynth_wrap.cpp \
	base_grid.cpp \
	browse_button.cpp \
	charset_conv.cpp \
	charset_detect.cpp \
	colorspace.cpp \
	colour_button.cpp \
	compat.cpp \
	dialog_about.cpp \
	dialog_attachments.cpp \
	dialog_automation.cpp \
	dialog_colorpicker.cpp \
	dialog_detached_video.cpp \
	dialog_dummy_video.cpp \
	dialog_export.cpp \
	dialog_fonts_collector.cpp \
	dialog_jumpto.cpp \
	dialog_kara_timing_copy.cpp \
	dialog_log.cpp \
	dialog_paste_over.cpp \
	dialog_progress.cpp \
	dialog_properties.cpp \
	dialog_resample.cpp \
	dialog_search_replace.cpp \
	dialog_selected_choices.cpp \
	dialog_selection.cpp \
	dialog_shift_times.cpp \
	dialog_spellchecker.cpp \
	dialog_style_editor.cpp \
	dialog_style_manager.cpp \
	dialog_styling_assistant.cpp \
	dialog_text_import.cpp \
	dialog_timing_processor.cpp \
	dialog_translation.cpp \
	dialog_version_check.cpp \
	dialog_video_details.cpp \
	drop.cpp \
	audio_provider_dummy.cpp \
	export_clean_info.cpp \
	export_fixstyle.cpp \
	export_framerate.cpp \
	fft.cpp \
	font_file_lister.cpp \
	font_file_lister_fontconfig.cpp \
	frame_main.cpp \
	gl_text.cpp \
	gl_wrap.cpp \
	help_button.cpp \
	hotkey.cpp \
	kana_table.cpp \
	main.cpp \
	menu.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	plugin_manager.cpp \
	preferences.cpp \
	preferences_base.cpp \
	scintilla_text_ctrl.cpp \
	spellchecker.cpp \
	spline.cpp \
	spline_curve.cpp \
	standard_paths.cpp \
	static_bmp.cpp \
	string_codec.cpp \
	subs_edit_box.cpp \
	subs_edit_ctrl.cpp \
	subs_grid.cpp \
	subs_preview.cpp \
	subtitle_format.cpp \
	subtitle_format_ass.cpp \
	subtitle_format_dvd.cpp \
	subtitle_format_encore.cpp \
	subtitle_format_microdvd.cpp \
	subtitle_format_mkv.cpp \
	subtitle_format_srt.cpp \
	subtitle_format_transtation.cpp \
	subtitle_format_ttxt.cpp \
	subtitle_format_txt.cpp \
	subtitles_provider.cpp \
	text_file_reader.cpp \
	text_file_writer.cpp \
	thesaurus.cpp \
	thesaurus_myspell.cpp \
	timeedit_ctrl.cpp \
	threaded_frame_source.cpp \
	toggle_bitmap.cpp \
	toolbar.cpp \
	tooltip_manager.cpp \
	utils.cpp \
	validators.cpp \
	variable_data.cpp \
	vector2d.cpp \
	version.cpp \
	video_box.cpp \
	video_context.cpp \
	video_display.cpp \
	video_frame.cpp \
	video_out_gl.cpp \
	video_provider_cache.cpp \
	video_provider_dummy.cpp \
	video_provider_manager.cpp \
	video_provider_yuv4mpeg.cpp \
	video_slider.cpp \
	visual_feature.cpp \
	visual_tool.cpp \
	visual_tool_clip.cpp \
	visual_tool_cross.cpp \
	visual_tool_drag.cpp \
	visual_tool_rotatexy.cpp \
	visual_tool_rotatez.cpp \
	visual_tool_scale.cpp \
	visual_tool_vector_clip.cpp


HEADER += \
	*.h \
	include/aegisub/*.h \
    config/*.h


EXTRA_DIST = \
	$(SRC_OPT) \
	*.hxx

EXTRA_DIST= \
	bitmaps/16/*.png \
	bitmaps/24/*.png \
	bitmaps/misc/*.png \
	bitmaps/windows/eyedropper.cur \
	bitmaps/windows/icon.ico

include ../Makefile.target
-include *.d
