EXTRA_DIST = res.rc $(srcdir)/*.h
SUFFIXES = .c .cpp .rc

SUBDIRS = bitmaps posix

bin_PROGRAMS = aegisub

SVNREV := $(strip $(shell grep -m 1 revision= .svn/entries 2>/dev/null | cut -d \" -f 2))
DARCSREV := $(strip $(shell head -n 1 ../_darcs/inventory 2>/dev/null))
REVISION := $(if $(SVNREV),-DBUILD_SVN_REVISION=$(SVNREV)) $(if $(DARCSREV),-DBUILD_DARCS)
BUILDINFO := -DBUILD_CREDIT="\"$(shell whoami)\"" $(REVISION)

AM_CPPFLAGS = -Iposix -include posix/defines.h $(BUILDINFO)
LDADD = posix/libposix.a ../lua51/src/liblua.a
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
	audio_player.cpp \
	audio_player_portaudio.cpp \
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
	avisynth_wrap.cpp \
	base_grid.cpp \
	colorspace.cpp \
	colour_button.cpp \
	dialog_about.cpp \
	dialog_attachments.cpp \
	dialog_automation.cpp \
	dialog_colorpicker.cpp \
	dialog_export.cpp \
	dialog_fonts_collector.cpp \
	dialog_hotkeys.cpp \
	dialog_jumpto.cpp \
	dialog_options.cpp \
	dialog_paste_over.cpp \
	dialog_progress.cpp \
	dialog_properties.cpp \
	dialog_resample.cpp \
	dialog_search_replace.cpp \
	dialog_selection.cpp \
	dialog_shift_times.cpp \
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
	hilimod_textctrl.cpp \
	hotkeys.cpp \
	idle_field_event.cpp \
	lavc_file.cpp \
	main.cpp \
	md5.c \
	mkv_wrap.cpp \
	mythes.cxx \
	options.cpp \
	spellchecker.cpp \
	spellchecker_hunspell.cpp \
	static_bmp.cpp \
	string_codec.cpp \
	subs_edit_box.cpp \
	subs_edit_ctrl.cpp \
	subs_grid.cpp \
	subtitle_format.cpp \
	subtitle_format_ass.cpp \
	subtitle_format_mkv.cpp \
	subtitle_format_prs.cpp \
	subtitle_format_srt.cpp \
	subtitle_format_txt.cpp \
	subtitle_provider.cpp \
	subtitle_provider_asa.cpp \
	subtitle_provider_libass.cpp \
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
	video_display.cpp \
	video_display_visual.cpp \
	video_provider.cpp \
	video_provider_lavc.cpp \
	video_slider.cpp \
	video_zoom.cpp \
	MatroskaParser.c


