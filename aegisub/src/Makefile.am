AUTOMAKE_OPTIONS = foreign
SUFFIXES = .c .cpp .rc
noinst_LIBRARIES= 
AM_CXXFLAGS = 

if BUILD_DARWIN
libosxutil_subdir = libosxutil
libosxutil_lib = libosxutil/libosxutil.a
libosxutil_ldflags = -framework CoreFoundation
endif

if PRECOMPILED_HEADER
BUILT_SOURCES = agi_pre.h.gch
precompiled_header = agi_pre.h
AM_CXXFLAGS += -Winvalid-pch -fpch-deps -fpch-preprocess -include agi_pre.h
nodist_aegisub_2_2_SOURCES = agi_pre.h.gch
endif

SUBDIRS = \
	bitmaps \
	libresrc \
	$(libosxutil_subdir)

AM_CXXFLAGS += -DAEGISUB -Iinclude -I../libffms/include @WX_CPPFLAGS@ @OPENMP_CXXFLAGS@ @LIBAVFORMAT_CFLAGS@ @LIBAVCODEC_CFLAGS@ @LIBSWSCALE_CFLAGS@ @LIBAVUTIL_CFLAGS@

bin_PROGRAMS = aegisub-2.2
aegisub_2_2_LDADD = libresrc/libresrc.a $(libosxutil_lib)
aegisub_2_2_CPPFLAGS = @FREETYPE_CFLAGS@
aegisub_2_2_LDFLAGS = @DEBUG_FLAGS@ @PROFILE_FLAGS@ @GL_LIBS@ @PTHREAD_LIBS@ @WX_LIBS@ @ICONV_LDFLAGS@ $(libosxutil_ldflags) @CCMALLOC_LDFLAGS@ @EFENCE_LDFLAGS@
LIBS += @FREETYPE_LIBS@ @FONTCONFIG_LIBS@ @CCMALLOC_LIBS@

if PRECOMPILED_HEADER
# This doesn't depend on Makefile on purpose, you should already know what you're doing when using this.
agi_pre.h.gch: agi_pre.h
	@CXX@ -include ../acconf.h $(DEFAULT_INCLUDES) @CXXFLAGS@ $(AM_CXXFLAGS) @DEBUG_FLAGS@ @PROFILE_FLAGS@ agi_pre.h
endif

if BUILD_DARWIN
aegisub_2_2_LDFLAGS += -L/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries
endif

noinst_LIBRARIES += libaudio_player.a
libaudio_player_a_SOURCES = audio_player.cpp
libaudio_player_a_CPPFLAGS = @ALSA_CFLAGS@ @PORTAUDIO_CFLAGS@ @LIBPULSE_CFLAGS@ @OPENAL_CFLAGS@
aegisub_2_2_LDADD += libaudio_player.a

if HAVE_ALSA
noinst_LIBRARIES += libaudio_alsa.a
libaudio_alsa_a_SOURCES = audio_player_alsa.cpp
aegisub_2_2_LDFLAGS += @ALSA_LDFLAGS@
aegisub_2_2_LDADD += libaudio_alsa.a
endif

if HAVE_PORTAUDIO
noinst_LIBRARIES += libaudio_portaudio.a
libaudio_portaudio_a_SOURCES = audio_player_portaudio.cpp
libaudio_portaudio_a_CPPFLAGS = @PORTAUDIO_CFLAGS@
aegisub_2_2_LDFLAGS += @PORTAUDIO_LIBS@
aegisub_2_2_LDADD += libaudio_portaudio.a
endif

if HAVE_PULSEAUDIO
noinst_LIBRARIES += libaudio_pulseaudio.a
libaudio_pulseaudio_a_SOURCES = audio_player_pulse.cpp
libaudio_pulseaudio_a_CPPFLAGS = @LIBPULSE_CFLAGS@
aegisub_2_2_LDFLAGS += @LIBPULSE_LIBS@
aegisub_2_2_LDADD += libaudio_pulseaudio.a
endif

if HAVE_OPENAL
noinst_LIBRARIES += libaudio_openal.a
libaudio_openal_a_SOURCES = audio_player_openal.cpp
libaudio_openal_a_CPPFLAGS = @OPENAL_CFLAGS@
aegisub_2_2_LDFLAGS += @OPENAL_LIBS@
aegisub_2_2_LDADD += libaudio_openal.a
endif

if HAVE_OSS
noinst_LIBRARIES += libaudio_oss.a
libaudio_oss_a_SOURCES = audio_player_oss.cpp
aegisub_2_2_LDADD += libaudio_oss.a
endif

if HAVE_FFMPEG
aegisub_2_2_LDFLAGS += @LIBAVFORMAT_LIBS@ @LIBAVCODEC_LIBS@ @LIBSWSCALE_LIBS@ @LIBAVUTIL_LIBS@
endif

if HAVE_PROVIDER_FFMPEGSOURCE
noinst_LIBRARIES += libaudiovideo_ffmpegsource.a
libaudiovideo_ffmpegsource_a_SOURCES = audio_provider_ffmpegsource.cpp video_provider_ffmpegsource.cpp ffmpegsource_common.cpp
libaudiovideo_ffmpegsource_a_CPPFLAGS = @LIBAVFORMAT_CFLAGS@ @LIBAVCODEC_CFLAGS@ @LIBSWSCALE_CFLAGS@ @LIBAVUTIL_CFLAGS@ @LIBPOSTPROC_CFLAGS@
aegisub_2_2_LDADD += libaudiovideo_ffmpegsource.a ../libffms/libffmpegsource_aegisub.a
aegisub_2_2_LDFLAGS += @LIBPOSTPROC_LIBS@
endif

noinst_LIBRARIES += libsubtitle_provider.a
libsubtitle_provider_a_SOURCES = subtitles_provider.cpp
libsubtitle_provider_a_CPPFLAGS = @CSRI_CFLAGS@
aegisub_2_2_LDADD += libsubtitle_provider.a

if WITH_LIBASS
noinst_LIBRARIES += libsubtitle_ass.a
libsubtitle_ass_a_SOURCES = subtitles_provider_libass.cpp
libsubtitle_ass_a_CPPFLAGS = @LIBASS_CFLAGS@ @ICONV_CFLAGS@
LIBS += @LIBASS_LIBS@
aegisub_2_2_LDADD += libsubtitle_ass.a
aegisub_2_2_LDFLAGS += @FONTCONFIG_LIBS@
endif

if WITH_CSRI
noinst_LIBRARIES += libsubtitle_csri.a
libsubtitle_csri_a_SOURCES = subtitles_provider_csri.cpp
libsubtitle_csri_a_CPPFLAGS = @ICONV_CFLAGS@ @CSRI_CFLAGS@
LIBS += @CSRI_LIBS@
aegisub_2_2_LDADD += libsubtitle_csri.a
aegisub_2_2_LDFLAGS += @FONTCONFIG_LIBS@
endif

if HAVE_AUTO4_LUA
noinst_LIBRARIES += libauto4_lua.a
libauto4_lua_a_SOURCES = auto4_lua.cpp auto4_lua_assfile.cpp auto4_lua_dialog.cpp auto4_lua_scriptreader.cpp
libauto4_lua_a_CPPFLAGS = @LUA_CFLAGS@
LIBS += @LUA_LDFLAGS@
aegisub_2_2_LDADD += libauto4_lua.a
endif

if HAVE_HUNSPELL
noinst_LIBRARIES += libmisc_hunspell.a
libmisc_hunspell_a_SOURCES = spellchecker_hunspell.cpp
aegisub_2_2_LDFLAGS += @HUNSPELL_LIBS@
aegisub_2_2_LDADD += libmisc_hunspell.a
endif

if HAVE_UNIVCHARDET
noinst_LIBRARIES += libmisc_universalchardet.a
libmisc_universalchardet_a_SOURCES = charset_detect.cpp text_file_reader.cpp
libmisc_universalchardet_a_CPPFLAGS = -D_X86_
aegisub_2_2_LDADD += libmisc_universalchardet.a ../universalchardet/libuniversalchardet.a
else
UNIVCHARSET = charset_detect.cpp text_file_reader.cpp
endif

if FONTLISTER_FREETYPE
  FONT_LISTER = font_file_lister_freetype.cpp
endif

if FONTLISTER_FONTCONFIG
  FONT_LISTER = font_file_lister_fontconfig.cpp
endif


## These aren't built, but are listed here so 'make dist' can always find all the sources
## This should also list all Win32 specific files
EXTRA_aegisub_2_2_SOURCES = \
	$(precompiled_header) \
	font_file_lister.cpp \
	$(FONT_LISTER) \
	audio_player_dsound.cpp \
	audio_player_dsound2.cpp \
	audio_player_portaudio.cpp \
	audio_player_pulse.cpp \
	audio_player_oss.cpp \
	audio_provider_avs.cpp \
	auto4_lua.cpp \
	auto4_lua_assfile.cpp \
	auto4_lua_dialog.cpp \
	auto4_lua_scriptreader.cpp \
	avisynth_wrap.cpp \
	dialog_associations.cpp \
	setup.cpp \
	spellchecker_hunspell.cpp \
	subtitle_format_prs.cpp \
	subtitles_provider_csri.cpp \
	subtitles_provider_libass.cpp \
	video_provider_avs.cpp


aegisub_2_2_SOURCES = \
	$(FFMPEG) \
	$(HUNSPELL) \
	$(LIBASS) \
	$(UNIVCHARSET) \
	font_file_lister.cpp \
	$(FONT_LISTER) \
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
	audio_display.cpp \
	audio_karaoke.cpp \
	audio_provider.cpp \
	audio_provider_convert.cpp \
	audio_provider_downmix.cpp \
	audio_provider_hd.cpp \
	audio_provider_pcm.cpp \
	audio_provider_quicktime.cpp \
	audio_provider_ram.cpp \
	audio_provider_stream.cpp \
	audio_renderer.cpp \
	audio_renderer_spectrum.cpp \
	auto4_base.cpp \
	avisynth_wrap.cpp \
	base_grid.cpp \
	browse_button.cpp \
	charset_conv.cpp \
	colorspace.cpp \
	colour_button.cpp \
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
	dialog_options.cpp \
	dialog_paste_over.cpp \
	dialog_progress.cpp \
	dialog_properties.cpp \
	dialog_resample.cpp \
	dialog_search_replace.cpp \
	dialog_selection.cpp \
	dialog_shift_times.cpp \
	dialog_spellchecker.cpp \
	dialog_splash.cpp \
	dialog_style_editor.cpp \
	dialog_style_manager.cpp \
	dialog_styling_assistant.cpp \
	dialog_text_import.cpp \
	dialog_timing_processor.cpp \
	dialog_tip.cpp \
	dialog_translation.cpp \
	dialog_version_check.cpp \
	dialog_video_details.cpp \
	drop.cpp \
	audio_provider_dummy.cpp \
	export_clean_info.cpp \
	export_fixstyle.cpp \
	export_framerate.cpp \
	export_visible_lines.cpp \
	fft.cpp \
	frame_main.cpp \
	frame_main_events.cpp \
	gl_text.cpp \
	gl_wrap.cpp \
	help_button.cpp \
	hilimod_textctrl.cpp \
	hotkeys.cpp \
	idle_field_event.cpp \
	kana_table.cpp \
	keyframe.cpp \
	main.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	options.cpp \
	plugin_manager.cpp \
	quicktime_common.cpp \
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
	text_file_writer.cpp \
	thesaurus.cpp \
	thesaurus_myspell.cpp \
	timeedit_ctrl.cpp \
	toggle_bitmap.cpp \
	tooltip_manager.cpp \
	utils.cpp \
	validators.cpp \
	variable_data.cpp \
	vector2d.cpp \
	version.cpp \
	vfr.cpp \
	video_box.cpp \
	video_context.cpp \
	video_display.cpp \
	video_frame.cpp \
	video_provider_cache.cpp \
	video_provider_dummy.cpp \
	video_provider_manager.cpp \
	video_provider_quicktime.cpp \
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


aegisub_2_2_SOURCES += \
	$(srcdir)/*.h \
	$(srcdir)/include/aegisub/*.h \
    $(srcdir)/config/*.h

noinst_HEADERS = \
	$(srcdir)/boost/*.hpp \
	gl/glext.h \
	$(srcdir)/*.hxx
