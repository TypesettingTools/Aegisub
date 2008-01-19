AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = res.rc $(srcdir)/*.h beautify

SUFFIXES = .c .cpp .rc

SUBDIRS = bitmaps posix

bin_PROGRAMS = aegisub

SVNREV := $(shell svnversion | sed 's/[^0-9].*//')
DARCSREV := $(strip $(shell head -n 1 ../_darcs/inventory 2>/dev/null))
REVISION := $(if $(SVNREV),-DBUILD_SVN_REVISION=$(SVNREV)) $(if $(DARCSREV),-DBUILD_DARCS)
BUILDINFO := -DBUILD_CREDIT="\"$(shell whoami)\"" $(REVISION)

AM_CPPFLAGS = -DAEGISUB -Iposix -include posix/defines.h -include posix/config.h $(BUILDINFO)
aegisub_LDADD = posix/libposix.a ../csri/lib/.libs/libcsri.a
aegisub_LDFLAGS = 

if USE_LIBASS
LIBASS=subtitles_provider_libass.cpp
else
LIBASS=
endif

AUDIO_PLAYER=audio_player.cpp 
if HAVE_PORTAUDIO
AUDIO_PLAYER += audio_player_portaudio.cpp 
endif
if HAVE_ALSA
AUDIO_PLAYER += audio_player_alsa.cpp 
endif
if HAVE_PULSEAUDIO
AUDIO_PLAYER += audio_player_pulse.cpp 
endif

AUTOMATION = auto4_base.cpp
if WITH_AUTO4_LUA
AUTOMATION +=  auto4_lua.cpp auto4_lua_assfile.cpp auto4_lua_dialog.cpp auto4_lua_scriptreader.cpp
aegisub_LDADD += ../lua51/src/liblua.a
endif
if WITH_AUTO3
AUTOMATION += auto4_auto3.cpp
aegisub_LDADD += ../auto3/libaegisub-auto3.la
endif
if WITH_AUTO4_RUBY
AUTOMATION += auto4_ruby_assfile.cpp auto4_ruby.cpp auto4_ruby_dialog.cpp
AM_CPPFLAGS += $(shell ruby -r rbconfig -e "p '-I' + Config::CONFIG['rubylibdir'] + '/' + Config::CONFIG['arch'] ") $(AM_CPPFLAGST)
endif

if HAVE_HUNSPELL
HUNSPELL=spellchecker_hunspell.cpp
else
HUNSPELL=
endif

if HAVE_FFMPEG
FFMPEG=audio_provider_lavc.cpp lavc_file.cpp video_provider_lavc.cpp lavc_keyframes.cpp
else
FFMPEG=
endif


## These aren't built, but are listed here so 'make dist' can always find all the sources
## This should also list all Win32 specific files
EXTRA_aegisub_SOURCES = \
	audio_player_alsa.cpp \
	audio_player_dsound.cpp \
	audio_player_portaudio.cpp \
	audio_player_pulse.cpp \
	audio_provider_avs.cpp \
	audio_provider_lavc.cpp \
	auto4_lua.cpp \
	auto4_lua_assfile.cpp \
	auto4_lua_dialog.cpp \
	auto4_lua_scriptreader.cpp \
	auto4_auto3.cpp \
	auto4_ruby.cpp \
	auto4_ruby_assfile.cpp \
	auto4_ruby_dialog.cpp \
	avisynth.h \
	avisynth_wrap.cpp \
	dialog_associations.cpp \
	factory.h \
	lavc_file.cpp \
	lavc_keyframes.cpp \
	setup.cpp \
	spellchecker_hunspell.cpp \
	stdwx.cpp \
	stdwx.h \
	subtitle_format_prs.cpp \
	subtitles_provider_libass.cpp \
	video_provider_avs.cpp \
	video_provider_dshow.cpp \
	video_provider_lavc.cpp \
	boost/*.hpp \
	csri/*.h \
	gl/*.h \
	win32/stdint.h

aegisub_SOURCES = \
	$(AUDIO_PLAYER) \
	$(AUTOMATION) \
	$(HUNSPELL) \
	$(LIBASS) \
	$(FFMPEG) \
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
	audio_provider_hd.cpp \
	audio_provider_ram.cpp \
	audio_provider_pcm.cpp \
	audio_provider_stream.cpp \
	audio_spectrum.cpp \
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
	main.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	options.cpp \
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
	subtitle_format_encore.cpp \
	subtitle_format_microdvd.cpp \
	subtitle_format_mkv.cpp \
	subtitle_format_srt.cpp \
	subtitle_format_txt.cpp \
	subtitle_format_ttxt.cpp \
	subtitles_provider.cpp \
	subtitles_provider_csri.cpp \
	text_file_reader.cpp \
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
	video_provider.cpp \
	video_provider_dummy.cpp \
	video_slider.cpp \
	visual_feature.cpp \
	visual_tool.cpp \
	visual_tool_clip.cpp \
	visual_tool_cross.cpp \
	visual_tool_drag.cpp \
	visual_tool_rotatexy.cpp \
	visual_tool_rotatez.cpp \
	visual_tool_scale.cpp \
	visual_tool_vector_clip.cpp \
	MatroskaParser.c

noinst_HEADERS = \
	boost/assert.hpp \
	boost/checked_delete.hpp \
	boost/shared_count.hpp \
	boost/shared_ptr.hpp \
	boost/throw_exception.hpp \
	boost/workaround.hpp \
	gl/glext.h \
	csri/csri.h \
	csri/logging.h \
	csri/openerr.h \
	mythes.hxx
