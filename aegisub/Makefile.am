AUTOMAKE_OPTIONS = foreign
SUFFIXES = .c .cpp .rc
noinst_LIBRARIES= 

SUBDIRS = bitmaps posix

AM_CXXFLAGS = -DAEGISUB -Iposix -include posix/defines.h -Iinclude @WX_CPPFLAGS@ @OPENMP_CXXFLAGS@

bin_PROGRAMS = aegisub
aegisub_LDADD = posix/libposix.a
aegisub_CPPFLAGS = @FREETYPE_CFLAGS@
aegisub_LDFLAGS = @GL_LIBS@ @PTHREAD_LIBS@ @WX_LIBS@
LIBS += @FREETYPE_LIBS@ @FONTCONFIG_LIBS@

if HAVE_ALSA
noinst_LIBRARIES += libaudio_alsa.a
libaudio_alsa_a_SOURCES = audio_player_alsa.cpp
aegisub_LDFLAGS += @ALSA_LDFLAGS@
aegisub_LDADD += libaudio_alsa.a
endif

if HAVE_PORTAUDIO
noinst_LIBRARIES += libaudio_portaudio.a
libaudio_portaudio_a_SOURCES = audio_player_portaudio.cpp
aegisub_LDFLAGS += @PORTAUDIO_LDFLAGS@
aegisub_LDADD += libaudio_portaudio.a
endif

if HAVE_PULSEAUDIO
noinst_LIBRARIES += libaudio_pulseaudio.a
libaudio_pulseaudio_a_SOURCES = audio_player_pulse.cpp
libaudio_pulseaudio_a_CPPFLAGS = @LIBPULSE_CFLAGS@
aegisub_LDFLAGS += @LIBPULSE_LIBS@
aegisub_LDADD += libaudio_pulseaudio.a
endif

if HAVE_OPENAL
noinst_LIBRARIES += libaudio_openal.a
libaudio_openal_a_SOURCES = audio_player_openal.cpp
libaudio_openal_a_CPPFLAGS = @OPENAL_CFLAGS@
aegisub_LDFLAGS += @OPENAL_LIBS@
aegisub_LDADD += libaudio_openal.a
endif

if HAVE_FFMPEG
noinst_LIBRARIES += libaudiovideo_ffmpeg.a
libaudiovideo_ffmpeg_a_SOURCES = audio_provider_lavc.cpp lavc_file.cpp video_provider_lavc.cpp lavc_keyframes.cpp
aegisub_LDFLAGS += @AVFORMAT_LDFLAGS@ @AVCODEC_LDFLAGS@ @SWSCALE_LDFLAGS@
aegisub_LDADD += libaudiovideo_ffmpeg.a
endif

if WITH_INTERNAL_LIBASS
aegisub_LDFLAGS += @FONTCONFIG_LIBS@
endif

if HAVE_LIBASS
noinst_LIBRARIES += libsubtitle_ass.a
libsubtitle_ass_a_SOURCES = subtitles_provider_libass.cpp
libsubtitle_ass_a_CPPFLAGS = @LIBASS_CFLAGS@
LIBS += @LIBASS_LIBS@
aegisub_LDADD += libsubtitle_ass.a
endif

if HAVE_AUTO4_LUA
noinst_LIBRARIES += libauto4_lua.a
libauto4_lua_a_SOURCES = auto4_lua.cpp auto4_lua_assfile.cpp auto4_lua_dialog.cpp auto4_lua_scriptreader.cpp
libauto4_lua_a_CPPFLAGS = @LUA_CFLAGS@
LIBS += @LUA_LDFLAGS@
aegisub_LDADD += libauto4_lua.a
endif

if HAVE_AUTO4_PERL
noinst_LIBRARIES += libauto4_perl.a
libauto4_perl_a_SOURCES = auto4_perl.cpp auto4_perl_script.cpp auto4_perl_dialogs.cpp auto4_perl_ass.cpp auto4_perl_console.cpp auto4_perl.h auto4_perl_console.h
libauto4_perl_a_CPPFLAGS = @PERL_CFLAGS@
LIBS += @PERL_LDFLAGS@
aegisub_LDADD += libauto4_perl.a
endif

if HAVE_AUTO4_RUBY
noinst_LIBRARIES += libauto4_ruby.a
libauto4_ruby_a_SOURCES = auto4_ruby_assfile.cpp auto4_ruby.cpp auto4_ruby_dialog.cpp
libauto4_ruby_a_CPPFLAGS = @RUBY_CFLAGS@
aegisub_LDFLAGS += @RUBY_LDFLAGS@
aegisub_LDADD += libauto4_ruby.a
endif

if HAVE_AUTO3_LUA
AUTO3_LUA = auto4_auto3.cpp
aegisub_LDADD += -L$(srcdir)/../auto3 -laegisub-auto3
endif

if HAVE_HUNSPELL
noinst_LIBRARIES += libmisc_hunspell.a
libmisc_hunspell_a_SOURCES = spellchecker_hunspell.cpp
aegisub_LDFLAGS += @HUNSPELL_LIBS@
aegisub_LDADD += libmisc_hunspell.a
endif

if HAVE_UNIVCHARDET
noinst_LIBRARIES += libmisc_universalchardet.a
libmisc_universalchardet_a_SOURCES = charset_detect.cpp text_file_reader.cpp
libmisc_universalchardet_a_CPPFLAGS = -D_X86_
aegisub_LDADD += libmisc_universalchardet.a ../universalchardet/libuniversalchardet.a
else
UNIVCHARSET = charset_detect.cpp text_file_reader.cpp
endif



## These aren't built, but are listed here so 'make dist' can always find all the sources
## This should also list all Win32 specific files
EXTRA_aegisub_SOURCES = \
	audio_player_dsound.cpp \
	audio_player_portaudio.cpp \
	audio_player_pulse.cpp \
	audio_provider_avs.cpp \
	audio_provider_lavc.cpp \
	auto4_auto3.cpp \
	auto4_lua.cpp \
	auto4_lua_assfile.cpp \
	auto4_lua_dialog.cpp \
	auto4_lua_scriptreader.cpp \
	auto4_ruby.cpp \
	auto4_ruby_assfile.cpp \
	auto4_ruby_dialog.cpp \
	avisynth_wrap.cpp \
	dialog_associations.cpp \
	lavc_file.cpp \
	lavc_keyframes.cpp \
	setup.cpp \
	spellchecker_hunspell.cpp \
	stdwx.cpp \
	subtitle_format_prs.cpp \
	subtitles_provider_libass.cpp \
	video_provider_avs.cpp \
	video_provider_dshow.cpp \
	video_provider_lavc.cpp

aegisub_SOURCES = \
	$(AUTO3_LUA) \
	$(FFMPEG) \
	$(HUNSPELL) \
	$(LIBASS) \
	$(UNIVCHARSET) \
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
	audio_player.cpp \
	audio_provider.cpp \
	audio_provider_convert.cpp \
	audio_provider_hd.cpp \
	audio_provider_pcm.cpp \
	audio_provider_ram.cpp \
	audio_provider_stream.cpp \
	audio_spectrum.cpp \
	auto4_base.cpp \
	avisynth_wrap.cpp \
	base_grid.cpp \
	browse_button.cpp \
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
	dialog_kanji_timer.cpp \
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
	export_clean_info.cpp \
	export_fixstyle.cpp \
	export_framerate.cpp \
	export_visible_lines.cpp \
	fft.cpp \
	font_file_lister.cpp \
	font_file_lister_fontconfig.cpp \
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
	macosx/bevelButton.cpp \
	main.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	options.cpp \
	plugin_manager.cpp \
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
	subtitle_format_ttxt.cpp \
	subtitle_format_txt.cpp \
	subtitles_provider.cpp \
	subtitles_provider_csri.cpp \
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


aegisub_SOURCES += \
	$(srcdir)/*.h \
	$(srcdir)/include/aegisub/*.h

noinst_HEADERS = \
	$(srcdir)/boost/*.hpp \
	gl/glext.h \
	$(srcdir)/*.hxx

EXTRA_DIST = \
	auto4_perldata.inc \
	win32/stdint.h
