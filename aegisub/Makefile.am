EXTRA_DIST = res.rc $(srcdir)/*.h \
	subtitles_provider_libass.cpp
SUFFIXES = .c .cpp .rc

SUBDIRS = bitmaps posix

bin_PROGRAMS = aegisub

SVNREV := $(strip $(shell grep -m 1 revision= .svn/entries 2>/dev/null | cut -d \" -f 2))
DARCSREV := $(strip $(shell head -n 1 ../_darcs/inventory 2>/dev/null))
REVISION := $(if $(SVNREV),-DBUILD_SVN_REVISION=$(SVNREV)) $(if $(DARCSREV),-DBUILD_DARCS)
BUILDINFO := -DBUILD_CREDIT="\"$(shell whoami)\"" $(REVISION)

AM_CPPFLAGST = -DAEGISUB -Iposix -include posix/defines.h $(BUILDINFO)
LDADD = posix/libposix.a ../lua51/src/liblua.a ../FexTrackerSource/libfex.a ../csri/lib/.libs/libcsri.a -lGL -lGLU

if USE_LIBASS
LIBASS=subtitles_provider_libass.cpp
else
LIBASS=
endif

AUDIO_PLAYER=audio_player.cpp 
if HAVE_PORTAUDIO
AUDIO_PLAYER += audio_player_portaudio.cpp 
LDADD += -lportaudio
endif
if HAVE_ALSA
AUDIO_PLAYER += audio_player_alsa.cpp 
LDADD += -lasound
endif
if HAVE_PULSEAUDIO
AUDIO_PLAYER += audio_player_pulse.cpp 
LDADD += -lpulse -lpulse-simple
endif

if HAVE_RUBY
AUTO4RUBY=auto4_ruby_assfile.cpp auto4_ruby.cpp auto4_ruby_dialog.cpp
AM_CPPFLAGS = $(shell ruby -r rbconfig -e "p '-I' + Config::CONFIG['rubylibdir'] + '/' + Config::CONFIG['arch'] ") $(AM_CPPFLAGST)
else
AUTO4RUBY=
AM_CPPFLAGS = $(AM_CPPFLAGST)
endif

if HAVE_HUNSPELL
HUNSPELL=spellchecker_hunspell.cpp
else
HUNSPELL=
endif

aegisub_SOURCES = \
	aegisublocale.cpp \
	ass_attachment.cpp \
	ass_dialogue.cpp \
	ass_entry.cpp \
	ass_export_filter.cpp \
	ass_exporter.cpp \
	ass_file.cpp \
	ass_override.cpp \
	ass_style.cpp \
	ass_style_storage.cpp \
	ass_time.cpp \
	audio_box.cpp \
	audio_display.cpp \
	audio_karaoke.cpp \
	$(AUDIO_PLAYER) \
	audio_provider.cpp \
	audio_provider_hd.cpp \
	audio_provider_lavc.cpp \
	audio_provider_ram.cpp \
	audio_provider_stream.cpp \
	audio_spectrum.cpp \
	auto4_base.cpp \
	auto4_lua.cpp \
	auto4_lua_assfile.cpp \
	auto4_lua_dialog.cpp \
	auto4_auto3.cpp \
	$(AUTO4RUBY) \
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
	dialog_fextracker.cpp \
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
	dialog_timing_processor.cpp \
	dialog_tip.cpp \
	dialog_translation.cpp \
	dialog_version_check.cpp \
	drop.cpp \
	export_clean_info.cpp \
	export_fixstyle.cpp \
	export_framerate.cpp \
	export_visible_lines.cpp \
	fft.cpp \
	frame_main.cpp \
	frame_main_events.cpp \
	gl_text.cpp \
	gl_wrap.cpp \
	hilimod_textctrl.cpp \
	hotkeys.cpp \
	idle_field_event.cpp \
	kana_table.cpp \
	lavc_file.cpp \
	main.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	options.cpp \
	spellchecker.cpp \
	$(HUNSPELL) \
	static_bmp.cpp \
	string_codec.cpp \
	subs_edit_box.cpp \
	subs_edit_ctrl.cpp \
	subs_grid.cpp \
	subs_preview.cpp \
	subtitle_format.cpp \
	subtitle_format_ass.cpp \
	subtitle_format_mkv.cpp \
	subtitle_format_prs.cpp \
	subtitle_format_srt.cpp \
	subtitle_format_txt.cpp \
	subtitles_provider.cpp \
	subtitles_provider_csri.cpp \
	$(LIBASS) \
	text_file_reader.cpp \
	text_file_writer.cpp \
	thesaurus.cpp \
	thesaurus_myspell.cpp \
	timeedit_ctrl.cpp \
	toggle_bitmap.cpp \
	utils.cpp \
	validators.cpp \
	variable_data.cpp \
	version.cpp \
	vfr.cpp \
	video_box.cpp \
	video_context.cpp \
	video_display.cpp \
	video_display_fextracker.cpp \
	video_display_visual.cpp \
	video_frame.cpp \
	video_provider.cpp \
	video_provider_dummy.cpp \
	video_provider_lavc.cpp \
	video_slider.cpp \
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
