// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file preferences.cpp
/// @brief Preferences dialogue
/// @ingroup configuration_ui

#include "config.h"

#ifndef AGI_PRE
#include <iterator>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/event.h>
#include <wx/filefn.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/treebook.h>
#include <wx/treebook.h>
#endif

#include <libaegisub/exception.h>

#include "preferences.h"

#include "audio_renderer_waveform.h"
#include "colour_button.h"
#include "command/command.h"
#include "compat.h"
#include "help_button.h"
#include "hotkey_data_view_model.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/hotkey.h"
#include "include/aegisub/subtitles_provider.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "preferences_base.h"
#include "video_provider_manager.h"

#ifdef WITH_PORTAUDIO
#include "audio_player_portaudio.h"
#endif

#ifdef WITH_FFMS2
#include <ffms.h>
#endif

static wxArrayString vec_to_arrstr(std::vector<std::string> const& vec) {
	wxArrayString arrstr;
	transform(vec.begin(), vec.end(), std::back_inserter(arrstr), &lagi_wxString);
	return arrstr;
}

#define CLASS_PAGE(name)                             \
class name: public OptionPage {                  \
public:                                          \
	name(wxTreebook *book, Preferences *parent); \
};

CLASS_PAGE(General)
CLASS_PAGE(Audio)
CLASS_PAGE(Video)
CLASS_PAGE(Interface)
CLASS_PAGE(Interface_Colours)
CLASS_PAGE(Backup)
CLASS_PAGE(Automation)
CLASS_PAGE(Advanced)
CLASS_PAGE(Advanced_Audio)
CLASS_PAGE(Advanced_Video)

class Interface_Hotkeys : public OptionPage {
	wxDataViewCtrl *dvc;
	wxObjectDataPtr<HotkeyDataViewModel> model;
	wxSearchCtrl *quick_search;

	void OnNewButton(wxCommandEvent&);
	void OnEditButton(wxCommandEvent&);
	void OnDeleteButton(wxCommandEvent&);
	void OnUpdateFilter(wxCommandEvent&);
	void OnClearFilter(wxCommandEvent&);
public:
	Interface_Hotkeys(wxTreebook *book, Preferences *parent);
};

/// General preferences page
General::General(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("General")) {
	wxFlexGridSizer *general = PageSizer(_("General"));
	OptionAdd(general, _("Check for updates on startup"), "App/Auto/Check For Updates");
	OptionAdd(general, _("Show main toolbar"), "App/Show Toolbar");

	OptionAdd(general, _("Toolbar Icon Size"), "App/Toolbar Icon Size");
	wxString autoload_modes[] = { _("Never"), _("Always"), _("Ask") };
	wxArrayString autoload_modes_arr(3, autoload_modes);
	OptionChoice(general, _("Automatically load linked files"), autoload_modes_arr, "App/Auto/Load Linked Files");
	OptionAdd(general, _("Undo Levels"), "Limits/Undo Levels", 2);

	wxFlexGridSizer *recent = PageSizer(_("Recently Used Lists"));
	OptionAdd(recent, _("Files"), "Limits/MRU");
	OptionAdd(recent, _("Find/Replace"), "Limits/Find Replace");

	SetSizerAndFit(sizer);
}

/// Audio preferences page
Audio::Audio(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Audio")) {
	wxFlexGridSizer *general = PageSizer(_("Options"));
	OptionAdd(general, _("Default mouse wheel to zoom"), "Audio/Wheel Default to Zoom");
	OptionAdd(general, _("Lock scroll on cursor"), "Audio/Lock Scroll on Cursor");
	OptionAdd(general, _("Snap markers by default"), "Audio/Snap/Enable");
	OptionAdd(general, _("Auto-focus on mouse over"), "Audio/Auto/Focus");
	OptionAdd(general, _("Play audio when stepping in video"), "Audio/Plays When Stepping Video");
	CellSkip(general);
	OptionAdd(general, _("Default timing length (ms)"), "Timing/Default Duration", 0, 36000);
	OptionAdd(general, _("Default lead-in length (ms)"), "Audio/Lead/IN", 0, 36000);
	OptionAdd(general, _("Default lead-out length (ms)"), "Audio/Lead/OUT", 0, 36000);

	OptionAdd(general, _("Start-marker drag sensitivity (px)"), "Audio/Start Drag Sensitivity", 1, 15);
	OptionAdd(general, _("Line boundary thickness (px)"), "Audio/Line Boundaries Thickness", 1, 5);
	OptionAdd(general, _("Maximum snap distance (px)"), "Audio/Snap/Distance", 0, 25);

	const wxString dtl_arr[] = { _("Don't show"), _("Show previous"), _("Show previous and next"), _("Show all") };
	wxArrayString choice_dtl(4, dtl_arr);
	OptionChoice(general, _("Show inactive lines"), choice_dtl, "Audio/Inactive Lines Display Mode");
	CellSkip(general);
	OptionAdd(general, _("Include commented inactive lines"), "Audio/Display/Draw/Inactive Comments");

	wxFlexGridSizer *display = PageSizer(_("Display Visual Options"));
	OptionAdd(display, _("Keyframes in dialogue mode"), "Audio/Display/Draw/Keyframes in Dialogue Mode");
	OptionAdd(display, _("Keyframes in karaoke mode"), "Audio/Display/Draw/Keyframes in Karaoke Mode");
	OptionAdd(display, _("Cursor time"), "Audio/Display/Draw/Cursor Time");
	OptionAdd(display, _("Video position"), "Audio/Display/Draw/Video Position");
	OptionChoice(display, _("Waveform Style"), AudioWaveformRenderer::GetWaveformStyles(), "Audio/Display/Waveform Style");

	wxFlexGridSizer *label = PageSizer(_("Audio labels"));
	OptionFont(label, "Audio/Karaoke/");

	SetSizerAndFit(sizer);
}

/// Video preferences page
Video::Video(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Video")) {
	wxFlexGridSizer *general = PageSizer(_("Options"));
	OptionAdd(general, _("Show keyframes in slider"), "Video/Slider/Show Keyframes");
	OptionAdd(general, _("Seek video to line start on selection change"), "Video/Subtitle Sync");
	OptionAdd(general, _("Always show visual tools"), "Tool/Visual/Always Show");
	OptionAdd(general, _("Automatically open audio when opening video"), "Video/Open Audio");

	const wxString czoom_arr[24] = { "12.5%", "25%", "37.5%", "50%", "62.5%", "75%", "87.5%", "100%", "112.5%", "125%", "137.5%", "150%", "162.5%", "175%", "187.5%", "200%", "212.5%", "225%", "237.5%", "250%", "262.5%", "275%", "287.5%", "300%" };
	wxArrayString choice_zoom(24, czoom_arr);
	OptionChoice(general, _("Default Zoom"), choice_zoom, "Video/Default Zoom");

	OptionAdd(general, _("Fast jump step in frames"), "Video/Slider/Fast Jump Step");

	const wxString cscr_arr[3] = { "?video", "?script", "." };
	wxArrayString scr_res(3, cscr_arr);
	OptionChoice(general, _("Screenshot save path"), scr_res, "Path/Screenshot");

	wxFlexGridSizer *resolution = PageSizer(_("Script Resolution"));
	wxControl *autocb = OptionAdd(resolution, _("Use resolution of first video opened"), "Subtitle/Default Resolution/Auto");
	CellSkip(resolution);
	DisableIfChecked(autocb,
		OptionAdd(resolution, _("Default width"), "Subtitle/Default Resolution/Width"));
	DisableIfChecked(autocb,
		OptionAdd(resolution, _("Default height"), "Subtitle/Default Resolution/Height"));

	const wxString cres_arr[3] = { _("Never"), _("Ask"), _("Always") };
	wxArrayString choice_res(3, cres_arr);
	OptionChoice(resolution, _("Match video resolution on open"), choice_res, "Video/Check Script Res");

	SetSizerAndFit(sizer);
}

/// Interface preferences page
Interface::Interface(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Interface")) {
	wxFlexGridSizer *edit_box = PageSizer(_("Edit Box"));
	OptionAdd(edit_box, _("Enable call tips"), "App/Call Tips");
	OptionAdd(edit_box, _("Overwrite in time boxes"), "Subtitle/Time Edit/Insert Mode");
	CellSkip(edit_box);
	OptionAdd(edit_box, _("Enable syntax highlighting"), "Subtitle/Highlight/Syntax");
	OptionBrowse(edit_box, _("Dictionaries path"), "Path/Dictionary");
	OptionFont(edit_box, "Subtitle/Edit Box/");

	wxFlexGridSizer *grid = PageSizer(_("Grid"));
	OptionAdd(grid, _("Allow grid to take focus"), "Subtitle/Grid/Focus Allow");
	OptionAdd(grid, _("Highlight visible subtitles"), "Subtitle/Grid/Highlight Subtitles in Frame");
	OptionAdd(grid, _("Hide overrides symbol"), "Subtitle/Grid/Hide Overrides Char");
	OptionFont(grid, "Subtitle/Grid/");

	SetSizerAndFit(sizer);
}

/// Interface Colours preferences subpage
Interface_Colours::Interface_Colours(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Colors"), PAGE_SCROLL|PAGE_SUB) {
	delete sizer;
	wxSizer *main_sizer = new wxBoxSizer(wxHORIZONTAL);

	sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(sizer);

	wxFlexGridSizer *audio = PageSizer(_("Audio Display"));
	OptionAdd(audio, _("Play cursor"), "Colour/Audio Display/Play Cursor");
	OptionAdd(audio, _("Line boundary start"), "Colour/Audio Display/Line boundary Start");
	OptionAdd(audio, _("Line boundary end"), "Colour/Audio Display/Line boundary End");
	OptionAdd(audio, _("Line boundary inactive line"), "Colour/Audio Display/Line Boundary Inactive Line");
	OptionAdd(audio, _("Syllable boundaries"), "Colour/Audio Display/Syllable Boundaries");

	wxFlexGridSizer *color_schemes = PageSizer(_("Audio Color Schemes"));
	wxArrayString schemes = vec_to_arrstr(OPT_GET("Audio/Colour Schemes")->GetListString());
	OptionChoice(color_schemes, _("Spectrum"), schemes, "Colour/Audio Display/Spectrum");
	OptionChoice(color_schemes, _("Waveform"), schemes, "Colour/Audio Display/Waveform");

	sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(sizer);

	wxFlexGridSizer *syntax = PageSizer(_("Syntax Highlighting"));
	OptionAdd(syntax, _("Normal"), "Colour/Subtitle/Syntax/Normal");
	OptionAdd(syntax, _("Brackets"), "Colour/Subtitle/Syntax/Brackets");
	OptionAdd(syntax, _("Slashes and Parentheses"), "Colour/Subtitle/Syntax/Slashes");
	OptionAdd(syntax, _("Tags"), "Colour/Subtitle/Syntax/Tags");
	OptionAdd(syntax, _("Parameters"), "Colour/Subtitle/Syntax/Parameters");
	OptionAdd(syntax, _("Error"), "Colour/Subtitle/Syntax/Error");
	OptionAdd(syntax, _("Error Background"), "Colour/Subtitle/Syntax/Background/Error");
	OptionAdd(syntax, _("Line Break"), "Colour/Subtitle/Syntax/Line Break");
	OptionAdd(syntax, _("Karaoke templates"), "Colour/Subtitle/Syntax/Karaoke Template");

	wxFlexGridSizer *grid = PageSizer(_("Subtitle Grid"));
	OptionAdd(grid, _("Standard foreground"), "Colour/Subtitle Grid/Standard");
	OptionAdd(grid, _("Standard background"), "Colour/Subtitle Grid/Background/Background");
	OptionAdd(grid, _("Selection foreground"), "Colour/Subtitle Grid/Selection");
	OptionAdd(grid, _("Selection background"), "Colour/Subtitle Grid/Background/Selection");
	OptionAdd(grid, _("Collision foreground"), "Colour/Subtitle Grid/Collision");
	OptionAdd(grid, _("In frame background"), "Colour/Subtitle Grid/Background/Inframe");
	OptionAdd(grid, _("Comment background"), "Colour/Subtitle Grid/Background/Comment");
	OptionAdd(grid, _("Selected comment background"), "Colour/Subtitle Grid/Background/Selected Comment");
	OptionAdd(grid, _("Left Column"), "Colour/Subtitle Grid/Left Column");
	OptionAdd(grid, _("Active Line Border"), "Colour/Subtitle Grid/Active Border");
	OptionAdd(grid, _("Lines"), "Colour/Subtitle Grid/Lines");

	sizer = main_sizer;

	SetSizerAndFit(sizer);
}

/// wxDataViewIconTextRenderer with command name autocompletion
class CommandRenderer : public wxDataViewCustomRenderer {
	wxArrayString autocomplete;
	wxDataViewIconText value;
	static const int icon_width = 20;

public:
	CommandRenderer()
	: wxDataViewCustomRenderer("wxDataViewIconText", wxDATAVIEW_CELL_EDITABLE)
	{
		std::vector<std::string> cmd_names = cmd::get_registered_commands();
		autocomplete.reserve(cmd_names.size());
		copy(cmd_names.begin(), cmd_names.end(), std::back_inserter(autocomplete));
	}

	wxWindow *CreateEditorCtrl(wxWindow *parent, wxRect label_rect, wxVariant const& value) {
		wxDataViewIconText iconText;
		iconText << value;

		wxString text = iconText.GetText();

		// adjust the label rect to take the width of the icon into account
		label_rect.x += icon_width;
		label_rect.width -= icon_width;

		wxTextCtrl* ctrl = new wxTextCtrl(parent, -1, text, label_rect.GetPosition(), label_rect.GetSize(), wxTE_PROCESS_ENTER);
		ctrl->SetInsertionPointEnd();
		ctrl->SelectAll();
		ctrl->AutoComplete(autocomplete);
		return ctrl;
	}

	bool SetValue(wxVariant const& var) {
		value << var;
		return true;
	}

	bool Render(wxRect rect, wxDC *dc, int state) {
		wxIcon const& icon = value.GetIcon();
		if (icon.IsOk())
			dc->DrawIcon(icon, rect.x, rect.y + (rect.height - icon.GetHeight()) / 2);

		RenderText(value.GetText(), icon_width, rect, dc, state);

		return true;
	}

	wxSize GetSize() const {
		if (!value.GetText().empty()) {
			wxSize size = GetTextExtent(value.GetText());
			size.x += icon_width;
			return size;
		}
		return wxSize(80,20);
	}

	bool GetValueFromEditorCtrl(wxWindow* editor, wxVariant &var) {
		wxTextCtrl *text = static_cast<wxTextCtrl*>(editor);
		wxDataViewIconText iconText(text->GetValue(), value.GetIcon());
		var << iconText;
		return true;
	}

	bool GetValue(wxVariant &) const { return false; }
	bool HasEditorCtrl() const { return true; }
};

class HotkeyRenderer : public wxDataViewCustomRenderer {
	wxString value;
	wxTextCtrl *ctrl;

public:
	HotkeyRenderer() : wxDataViewCustomRenderer("string", wxDATAVIEW_CELL_EDITABLE) { }

	wxWindow *CreateEditorCtrl(wxWindow *parent, wxRect label_rect, wxVariant const& var) {
		ctrl = new wxTextCtrl(parent, -1, var.GetString(), label_rect.GetPosition(), label_rect.GetSize(), wxTE_PROCESS_ENTER);
		ctrl->SetInsertionPointEnd();
		ctrl->SelectAll();
		ctrl->Bind(wxEVT_KEY_DOWN, &HotkeyRenderer::OnKeyDown, this);
		return ctrl;
	}

	void OnKeyDown(wxKeyEvent &evt) {
		ctrl->ChangeValue(lagi_wxString(hotkey::keypress_to_str(evt.GetKeyCode(), evt.GetUnicodeKey(), evt.GetModifiers())));
	}

	bool SetValue(wxVariant const& var) {
		value = var.GetString();
		return true;
	}

	bool Render(wxRect rect, wxDC *dc, int state) {
		RenderText(value, 0, rect, dc, state);
		return true;
	}

	bool GetValueFromEditorCtrl(wxWindow*, wxVariant &var) {
		var = ctrl->GetValue();
		return true;
	}

	bool GetValue(wxVariant &) const { return false; }
	wxSize GetSize() const { return !value ? wxSize(80, 20) : GetTextExtent(value); }
	bool HasEditorCtrl() const { return true; }
};

/// Interface Hotkeys preferences subpage
Interface_Hotkeys::Interface_Hotkeys(wxTreebook *book, Preferences *parent)
: OptionPage(book, parent, _("Hotkeys"), PAGE_SUB)
, model(new HotkeyDataViewModel(parent))
{
	quick_search = new wxSearchCtrl(this, -1);
	wxButton *new_button = new wxButton(this, -1, _("&New"));
	wxButton *edit_button = new wxButton(this, -1, _("&Edit"));
	wxButton *delete_button = new wxButton(this, -1, _("&Delete"));

	new_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Interface_Hotkeys::OnNewButton, this);
	edit_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Interface_Hotkeys::OnEditButton, this);
	delete_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Interface_Hotkeys::OnDeleteButton, this);

	quick_search->Bind(wxEVT_COMMAND_TEXT_UPDATED, &Interface_Hotkeys::OnUpdateFilter, this);
	quick_search->Bind(wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, &Interface_Hotkeys::OnClearFilter, this);

	dvc = new wxDataViewCtrl(this, -1);
	dvc->AssociateModel(model.get());
	dvc->AppendColumn(new wxDataViewColumn("Hotkey", new HotkeyRenderer, 0, 125, wxALIGN_LEFT, wxCOL_SORTABLE | wxCOL_RESIZABLE));
	dvc->AppendColumn(new wxDataViewColumn("Command", new CommandRenderer, 1, 250, wxALIGN_LEFT, wxCOL_SORTABLE | wxCOL_RESIZABLE));
	dvc->AppendTextColumn("Description", 2, wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT, wxCOL_SORTABLE | wxCOL_RESIZABLE);

	wxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
	buttons->Add(quick_search, wxSizerFlags().Border());
	buttons->AddStretchSpacer(1);
	buttons->Add(new_button, wxSizerFlags().Border().Right());
	buttons->Add(edit_button, wxSizerFlags().Border().Right());
	buttons->Add(delete_button, wxSizerFlags().Border().Right());

	sizer->Add(buttons, wxSizerFlags().Expand());
	sizer->Add(dvc, wxSizerFlags(1).Expand().Border(wxLEFT | wxRIGHT));

	SetSizerAndFit(sizer);
}

static void edit_item(wxDataViewCtrl *dvc, wxDataViewItem item) {
#if wxCHECK_VERSION(2, 9, 4)
	dvc->EditItem(item, dvc->GetColumn(0));
#else
	dvc->StartEditor(item, 0);
#endif
}

void Interface_Hotkeys::OnNewButton(wxCommandEvent&) {
	wxDataViewItem sel = dvc->GetSelection();
	dvc->ExpandAncestors(sel);
	dvc->Expand(sel);

	wxDataViewItem new_item = model->New(sel);
	if (new_item.IsOk()) {
		dvc->Select(new_item);
		dvc->EnsureVisible(new_item);
		edit_item(dvc, new_item);
	}
}

void Interface_Hotkeys::OnEditButton(wxCommandEvent&) {
	edit_item(dvc, dvc->GetSelection());
}

void Interface_Hotkeys::OnDeleteButton(wxCommandEvent&) {
	model->Delete(dvc->GetSelection());
}

void Interface_Hotkeys::OnUpdateFilter(wxCommandEvent&) {
	model->SetFilter(quick_search->GetValue());

	if (!quick_search->GetValue().empty()) {
		wxDataViewItemArray contexts;
		model->GetChildren(wxDataViewItem(0), contexts);
		for (size_t i = 0; i < contexts.size(); ++i)
			dvc->Expand(contexts[i]);
	}
}

void Interface_Hotkeys::OnClearFilter(wxCommandEvent &) {
	quick_search->SetValue("");
}

/// Backup preferences page
Backup::Backup(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Backup")) {
	wxFlexGridSizer *save = PageSizer(_("Automatic Save"));
	wxControl *cb = OptionAdd(save, _("Enable"), "App/Auto/Save");
	CellSkip(save);
	EnableIfChecked(cb,
		OptionAdd(save, _("Interval in seconds"), "App/Auto/Save Every Seconds", 1));
	OptionBrowse(save, _("Path"), "Path/Auto/Save", cb, true);
	OptionAdd(save, _("Autosave after every change"), "App/Auto/Save on Every Change");

	wxFlexGridSizer *backup = PageSizer(_("Automatic Backup"));
	cb = OptionAdd(backup, _("Enable"), "App/Auto/Backup");
	CellSkip(backup);
	OptionBrowse(backup, _("Path"), "Path/Auto/Backup", cb, true);

	SetSizerAndFit(sizer);
}

/// Automation preferences page
Automation::Automation(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Automation")) {
	wxFlexGridSizer *general = PageSizer(_("General"));

	OptionAdd(general, _("Base path"), "Path/Automation/Base");
	OptionAdd(general, _("Include path"), "Path/Automation/Include");
	OptionAdd(general, _("Auto-load path"), "Path/Automation/Autoload");

	const wxString tl_arr[6] = { _("0: Fatal"), _("1: Error"), _("2: Warning"), _("3: Hint"), _("4: Debug"), _("5: Trace") };
	wxArrayString tl_choice(6, tl_arr);
	OptionChoice(general, _("Trace level"), tl_choice, "Automation/Trace Level");

	const wxString tp_arr[3] = { _("Normal"), _("Below Normal (recommended)"), _("Lowest") };
	wxArrayString tp_choice(3, tp_arr);
	OptionChoice(general, _("Thread priority"), tp_choice, "Automation/Thread Priority");

	const wxString ar_arr[4] = { _("No scripts"), _("Subtitle-local scripts"), _("Global autoload scripts"), _("All scripts") };
	wxArrayString ar_choice(4, ar_arr);
	OptionChoice(general, _("Autoreload on Export"), ar_choice, "Automation/Autoreload Mode");

	SetSizerAndFit(sizer);
}


/// Advanced preferences page
Advanced::Advanced(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Advanced")) {
	wxFlexGridSizer *general = PageSizer(_("General"));

	wxStaticText *warning = new wxStaticText(this, wxID_ANY ,_("Changing these settings might result in bugs and/or crashes.  Do not touch these unless you know what you're doing."));
	warning->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	sizer->Fit(this);
	warning->Wrap(400);
	general->Add(warning, 0, wxALL, 5);

	SetSizerAndFit(sizer);
}

/// Advanced Audio preferences subpage
Advanced_Audio::Advanced_Audio(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Audio"), PAGE_SUB) {
	wxFlexGridSizer *expert = PageSizer(_("Expert"));

	wxArrayString ap_choice = vec_to_arrstr(AudioProviderFactory::GetClasses());
	OptionChoice(expert, _("Audio provider"), ap_choice, "Audio/Provider");

	wxArrayString apl_choice = vec_to_arrstr(AudioPlayerFactory::GetClasses());
	OptionChoice(expert, _("Audio player"), apl_choice, "Audio/Player");

	wxFlexGridSizer *cache = PageSizer(_("Cache"));
	const wxString ct_arr[3] = { _("None (NOT RECOMMENDED)"), _("RAM"), _("Hard Disk") };
	wxArrayString ct_choice(3, ct_arr);
	OptionChoice(cache, _("Cache type"), ct_choice, "Audio/Cache/Type");

	OptionBrowse(cache, _("Path"), "Audio/Cache/HD/Location");
	OptionAdd(cache, _("File name"), "Audio/Cache/HD/Name");

	wxFlexGridSizer *spectrum = PageSizer(_("Spectrum"));

	const wxString sq_arr[4] = { _("Regular quality"), _("Better quality"), _("High quality"), _("Insane quality") };
	wxArrayString sq_choice(4, sq_arr);
	OptionChoice(spectrum, _("Quality"), sq_choice, "Audio/Renderer/Spectrum/Quality");

	OptionAdd(spectrum, _("Cache memory max (MB)"), "Audio/Renderer/Spectrum/Memory Max", 2, 1024);

#ifdef WITH_AVISYNTH
	wxFlexGridSizer *avisynth = PageSizer("Avisynth");
	const wxString adm_arr[3] = { "ConvertToMono", "GetLeftChannel", "GetRightChannel" };
	wxArrayString adm_choice(3, adm_arr);
	OptionChoice(avisynth, _("Avisynth down-mixer"), adm_choice, "Audio/Downmixer");
	OptionAdd(avisynth, _("Force sample rate"), "Provider/Audio/AVS/Sample Rate");
#endif

#ifdef WITH_FFMS2
	wxFlexGridSizer *ffms = PageSizer("FFmpegSource");

	const wxString error_modes[] = { _("Ignore"), _("Clear"), _("Stop"), _("Abort") };
	wxArrayString error_modes_choice(4, error_modes);
	OptionChoice(ffms, _("Audio indexing error handling mode"), error_modes_choice, "Provider/Audio/FFmpegSource/Decode Error Handling");

	OptionAdd(ffms, _("Always index all audio tracks"), "Provider/FFmpegSource/Index All Tracks");
#endif

#ifdef WITH_PORTAUDIO
	wxFlexGridSizer *portaudio = PageSizer("Portaudio");
	OptionChoice(portaudio, _("Portaudio device"), PortAudioPlayer::GetOutputDevices(), "Player/Audio/PortAudio/Device Name");
#endif

#ifdef WITH_OSS
	wxFlexGridSizer *oss = PageSizer("OSS");
	OptionBrowse(oss, _("OSS Device"), "Player/Audio/OSS/Device");
#endif

#ifdef WITH_DIRECTSOUND
	wxFlexGridSizer *dsound = PageSizer("DirectSound");
	OptionAdd(dsound, _("Buffer latency"), "Player/Audio/DirectSound/Buffer Latency", 1, 1000);
	OptionAdd(dsound, _("Buffer length"), "Player/Audio/DirectSound/Buffer Length", 1, 100);
#endif

	SetSizerAndFit(sizer);
}

/// Advanced Video preferences subpage
Advanced_Video::Advanced_Video(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Video"), PAGE_SUB) {
	wxFlexGridSizer *expert = PageSizer(_("Expert"));

	wxArrayString vp_choice = vec_to_arrstr(VideoProviderFactory::GetClasses());
	OptionChoice(expert, _("Video provider"), vp_choice, "Video/Provider");

	wxArrayString sp_choice = vec_to_arrstr(SubtitlesProviderFactory::GetClasses());
	OptionChoice(expert, _("Subtitles provider"), sp_choice, "Subtitle/Provider");


#ifdef WITH_AVISYNTH
	wxFlexGridSizer *avisynth = PageSizer("Avisynth");
	OptionAdd(avisynth, _("Allow pre-2.56a Avisynth"), "Provider/Avisynth/Allow Ancient");
	CellSkip(avisynth);
	OptionAdd(avisynth, _("Avisynth memory limit"), "Provider/Avisynth/Memory Max");
#endif

#ifdef WITH_FFMS2
	wxFlexGridSizer *ffms = PageSizer("FFmpegSource");

	const wxString log_levels[] = { "Quiet", "Panic", "Fatal", "Error", "Warning", "Info", "Verbose", "Debug" };
	wxArrayString log_levels_choice(8, log_levels);
	OptionChoice(ffms, _("Debug log verbosity"), log_levels_choice, "Provider/FFmpegSource/Log Level");

	OptionAdd(ffms, _("Decoding threads"), "Provider/Video/FFmpegSource/Decoding Threads", -1);
	OptionAdd(ffms, _("Enable unsafe seeking"), "Provider/Video/FFmpegSource/Unsafe Seeking");
#if FFMS_VERSION >= ((2 << 24) | (17 << 16) | (1 << 8) | 0)
	OptionAdd(ffms, _("Force BT.601"), "Provider/Video/FFmpegSource/Force BT.601");
#endif
#endif

	SetSizerAndFit(sizer);
}

void Preferences::SetOption(agi::OptionValue *new_value) {
	std::string name = new_value->GetName();
	if (pending_changes.count(name))
		delete pending_changes[name];
	pending_changes[name] = new_value;
	if (IsEnabled())
		applyButton->Enable(true);
}

void Preferences::AddPendingChange(Thunk const& callback) {
	pending_callbacks.push_back(callback);
	if (IsEnabled())
		applyButton->Enable(true);
}

void Preferences::OnOK(wxCommandEvent &event) {
	OnApply(event);
	EndModal(0);
}

void Preferences::OnApply(wxCommandEvent &) {
	for (std::map<std::string, agi::OptionValue*>::iterator cur = pending_changes.begin(); cur != pending_changes.end(); ++cur) {
		agi::OptionValue *opt = OPT_SET(cur->first);
		switch (opt->GetType()) {
			case agi::OptionValue::Type_Bool:
				opt->SetBool(cur->second->GetBool());
				break;
			case agi::OptionValue::Type_Colour:
				opt->SetColour(cur->second->GetColour());
				break;
			case agi::OptionValue::Type_Double:
				opt->SetDouble(cur->second->GetDouble());
				break;
			case agi::OptionValue::Type_Int:
				opt->SetInt(cur->second->GetInt());
				break;
			case agi::OptionValue::Type_String:
				opt->SetString(cur->second->GetString());
				break;
			default:
				throw PreferenceNotSupported("Unsupported type");
		}
		delete cur->second;
	}
	pending_changes.clear();

	for (std::deque<Thunk>::iterator it = pending_callbacks.begin(); it != pending_callbacks.end(); ++it)
		(*it)();
	pending_callbacks.clear();

	applyButton->Enable(false);
	config::opt->Flush();
}

static void PageChanged(wxBookCtrlEvent& evt) {
	OPT_SET("Tool/Preferences/Page")->SetInt(evt.GetSelection());
}

Preferences::Preferences(wxWindow *parent): wxDialog(parent, -1, _("Preferences"), wxDefaultPosition, wxSize(-1, 500)) {
	SetIcon(GETICON(options_button_16));

	book = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	new General(book, this);
	new Audio(book, this);
	new Video(book, this);
	new Interface(book, this);
	new Interface_Colours(book, this);
	new Interface_Hotkeys(book, this);
	new Backup(book, this);
	new Automation(book, this);
	new Advanced(book, this);
	new Advanced_Audio(book, this);
	new Advanced_Video(book, this);

	book->Fit();

	book->ChangeSelection(OPT_GET("Tool/Preferences/Page")->GetInt());
	book->Bind(wxEVT_COMMAND_TREEBOOK_PAGE_CHANGED, &PageChanged);

	// Bottom Buttons
	wxStdDialogButtonSizer *stdButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxAPPLY | wxHELP);
	applyButton = stdButtonSizer->GetApplyButton();

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, wxSizerFlags(1).Expand().Border());
	mainSizer->Add(stdButtonSizer, wxSizerFlags(0).Expand().Border(wxALL & ~wxTOP));

	SetSizerAndFit(mainSizer);
	SetMinSize(wxSize(-1, 500));
	SetMaxSize(wxSize(-1, 500));
	CenterOnParent();

	applyButton->Enable(false);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnApply, this, wxID_APPLY);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&HelpButton::OpenPage, "Options"), wxID_HELP);
}

Preferences::~Preferences() {
	for (std::map<std::string, agi::OptionValue*>::iterator cur = pending_changes.begin(); cur != pending_changes.end(); ++cur) {
		delete cur->second;
	}
}
