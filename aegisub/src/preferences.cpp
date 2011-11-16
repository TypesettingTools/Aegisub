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


#ifndef AGI_PRE
#include <iterator>

#include <wx/any.h>
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

#include "colour_button.h"
#include "command/command.h"
#include "compat.h"
#include "hotkey_data_view_model.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "include/aegisub/hotkey.h"
#include "include/aegisub/subtitles_provider.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "preferences_base.h"
#include "video_provider_manager.h"

#define CLASS_PAGE(name)                             \
class name: public OptionPage {                  \
public:                                          \
	name(wxTreebook *book, Preferences *parent); \
};

CLASS_PAGE(General)
CLASS_PAGE(Subtitles)
CLASS_PAGE(Audio)
CLASS_PAGE(Video)
CLASS_PAGE(Interface)
CLASS_PAGE(Interface_Colours)
CLASS_PAGE(Paths)
CLASS_PAGE(File_Associations)
CLASS_PAGE(Backup)
CLASS_PAGE(Automation)
CLASS_PAGE(Advanced)
CLASS_PAGE(Advanced_Interface)
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

	wxFlexGridSizer *startup = PageSizer(_("Startup"));
	OptionAdd(startup, _("Check for updates"), "App/Auto/Check For Updates");

	wxFlexGridSizer *recent = PageSizer(_("Recently Used Lists"));
	OptionAdd(recent, _("Files"), "Limits/MRU");
	OptionAdd(recent, _("Find/Replace"), "Limits/Find Replace");
	sizer->AddSpacer(15);

	wxFlexGridSizer *undo = PageSizer(_("Undo / Redo Settings"));
	OptionAdd(undo, _("Undo Levels"), "Limits/Undo Levels");

	wxFlexGridSizer *toolbar = PageSizer(_("Toolbar Settings"));
	OptionAdd(toolbar, _("Toolbar Icon Size"), "App/Toolbar Icon Size");

	SetSizerAndFit(sizer);
}


/// Subtitles preferences page
Subtitles::Subtitles(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Subtitles")) {

	wxFlexGridSizer *general = PageSizer(_("Options"));
	OptionAdd(general, _("Enable call tips"), "App/Call Tips");
	OptionAdd(general, _("Enable syntax highlighting"), "Subtitle/Highlight/Syntax");
	OptionAdd(general, _("Link commiting of times"), "Subtitle/Edit Box/Link Time Boxes Commit");
	OptionAdd(general, _("Overwrite-Insertion in time boxes"), "Subtitle/Time Edit/Insert Mode");

	wxFlexGridSizer *grid = PageSizer(_("Grid"));
	OptionAdd(grid, _("Allow grid to take focus"), "Subtitle/Grid/Focus Allow");
	OptionAdd(grid, _("Highlight visible subtitles"), "Subtitle/Grid/Highlight Subtitles in Frame");

	SetSizerAndFit(sizer);
}


/// Audio preferences page
Audio::Audio(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Audio")) {

	wxFlexGridSizer *general = PageSizer(_("Options"));
	OptionAdd(general, _("Grab times from line upon selection"), "Audio/Grab Times on Select");
	OptionAdd(general, _("Default mouse wheel to zoom"), "Audio/Wheel Default to Zoom");
	OptionAdd(general, _("Lock scroll on cursor"), "Audio/Lock Scroll on Cursor");
	OptionAdd(general, _("Snap to keyframes"), "Audio/Display/Snap/Keyframes");
	OptionAdd(general, _("Snap to adjacent lines"), "Audio/Display/Snap/Other Lines");
	OptionAdd(general, _("Auto-focus on mouse over"), "Audio/Auto/Focus");
	OptionAdd(general, _("Play audio when stepping in video"), "Audio/Plays When Stepping Video");
	CellSkip(general);
	OptionAdd(general, _("Default timing length"), "Timing/Default Duration", 0, 36000);
	OptionAdd(general, _("Default lead-in length"), "Audio/Lead/IN", 0, 36000);
	OptionAdd(general, _("Default lead-out length"), "Audio/Lead/OUT", 0, 36000);

	const wxString dtl_arr[3] = { _("Don't show"), _("Show previous"), _("Show all") };
	wxArrayString choice_dtl(3, dtl_arr);
	OptionChoice(general, _("Show inactive lines"), choice_dtl, "Audio/Inactive Lines Display Mode");
	OptionAdd(general, _("Start-marker drag sensitivity"), "Audio/Start Drag Sensitivity", 1, 15);

	wxFlexGridSizer *display = PageSizer(_("Display Visual Options"));
	OptionAdd(display, _("Secondary lines"), "Audio/Display/Draw/Secondary Lines");
	OptionAdd(display, _("Selection background"), "Audio/Display/Draw/Selection Background");
	OptionAdd(display, _("Timeline"), "Audio/Display/Draw/Timeline");
	OptionAdd(display, _("Cursor time"), "Audio/Display/Draw/Cursor Time");
	OptionAdd(display, _("Keyframes"), "Audio/Display/Draw/Keyframes in Dialogue Mode");
	OptionAdd(display, _("Karaoke keyframes"), "Audio/Display/Draw/Keyframes in Karaoke Mode");
	OptionAdd(display, _("Video position"), "Audio/Display/Draw/Video Position");

	SetSizerAndFit(sizer);
}


/// Video preferences page
Video::Video(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Video")) {

	wxFlexGridSizer *general = PageSizer(_("Options"));
	OptionAdd(general, _("Show keyframes in slider"), "Video/Slider/Show Keyframes");
	OptionAdd(general, _("Always show visual tools"), "Tool/Visual/Always Show");

	const wxString cres_arr[3] = { _("Never"), _("Ask"), _("Always") };
	wxArrayString choice_res(3, cres_arr);
	OptionChoice(general, _("Match video resolution on open"), choice_res, "Video/Check Script Res");

	const wxString czoom_arr[24] = { "12.5%", "25%", "37.5%", "50%", "62.5%", "75%", "87.5%", "100%", "112.5%", "125%", "137.5%", "150%", "162.5%", "175%", "187.5%", "200%", "212.5%", "225%", "237.5%", "250%", "262.5%", "275%", "287.5%", "300%" };
	wxArrayString choice_zoom(24, czoom_arr);
	OptionChoice(general, _("Default Zoom"), choice_zoom, "Video/Default Zoom");

	OptionAdd(general, _("Fast jump step in frames"), "Video/Slider/Fast Jump Step");

	const wxString cscr_arr[3] = { _("?video"), _("?script"), _(".") };
	wxArrayString scr_res(3, cscr_arr);
	OptionChoice(general, _("Screenshot save path"), scr_res, "Path/Screenshot");

	SetSizerAndFit(sizer);
}


/// Interface preferences page
Interface::Interface(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Interface")) {
	wxFlexGridSizer *grid = PageSizer(_("Subtitle Grid"));
	OptionFont(grid, "Subtitle/Grid/");

	OptionAdd(grid, _("Hide overrides symbol"), "Subtitle/Grid/Hide Overrides Char");

	wxFlexGridSizer *edit_box = PageSizer(_("Edit Box"));
	OptionFont(edit_box, "Subtitle/Edit Box/");

	SetSizerAndFit(sizer);
}


/// Interface Colours preferences subpage
Interface_Colours::Interface_Colours(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Colours"), PAGE_SCROLL|PAGE_SUB) {
	delete sizer;
	wxSizer *main_sizer = sizer = new wxBoxSizer(wxHORIZONTAL);

	wxFlexGridSizer *audio = PageSizer(_("Audio Display"));
	OptionAdd(audio, _("Play cursor"), "Colour/Audio Display/Play Cursor");
	OptionAdd(audio, _("Background"), "Colour/Audio Display/Background/Background");
	OptionAdd(audio, _("Selection background"), "Colour/Audio Display/Background/Selection");
	OptionAdd(audio, _("Selection background modified"), "Colour/Audio Display/Background/Selection Modified");
	OptionAdd(audio, _("Seconds boundaries"), "Colour/Audio Display/Seconds Boundaries");
	OptionAdd(audio, _("Waveform"), "Colour/Audio Display/Waveform");
	OptionAdd(audio, _("Waveform selected"), "Colour/Audio Display/Waveform Selected");
	OptionAdd(audio, _("Waveform Modified"), "Colour/Audio Display/Waveform Modified");
	OptionAdd(audio, _("Waveform Inactive"), "Colour/Audio Display/Waveform Inactive");
	OptionAdd(audio, _("Line boundary start"), "Colour/Audio Display/Line boundary Start");
	OptionAdd(audio, _("Line boundary end"), "Colour/Audio Display/Line boundary End");
	OptionAdd(audio, _("Line boundary inactive line"), "Colour/Audio Display/Line Boundary Inactive Line");
	OptionAdd(audio, _("Syllable text"), "Colour/Audio Display/Syllable Text");
	OptionAdd(audio, _("Syllable boundaries"), "Colour/Audio Display/Syllable Boundaries");

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
class CommandRenderer : public wxDataViewIconTextRenderer {
	wxArrayString autocomplete;
public:
	CommandRenderer()
	: wxDataViewIconTextRenderer("wxDataViewIconText", wxDATAVIEW_CELL_EDITABLE)
	{
		std::vector<std::string> cmd_names = cmd::get_registered_commands();
		autocomplete.reserve(cmd_names.size());
		copy(cmd_names.begin(), cmd_names.end(), std::back_inserter(autocomplete));
	}

	wxWindow *CreateEditorCtrl(wxWindow *parent, wxRect label_rect, wxVariant const& value) {
		wxTextCtrl *ctrl = static_cast<wxTextCtrl*>(wxDataViewIconTextRenderer::CreateEditorCtrl(parent, label_rect, value));
		ctrl->AutoComplete(autocomplete);
		return ctrl;
	}
};

class HotkeyRenderer : public wxDataViewTextRenderer {
	wxTextCtrl *ctrl;
public:
	HotkeyRenderer() : wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE) { }

	wxWindow *CreateEditorCtrl(wxWindow *parent, wxRect label_rect, wxVariant const& value) {
		ctrl = static_cast<wxTextCtrl*>(wxDataViewTextRenderer::CreateEditorCtrl(parent, label_rect, value));
		ctrl->Bind(wxEVT_KEY_DOWN, &HotkeyRenderer::OnKeyDown, this);
		return ctrl;
	}

	void OnKeyDown(wxKeyEvent &evt) {
		ctrl->ChangeValue(lagi_wxString(hotkey::keypress_to_str(evt.GetKeyCode(), evt.GetUnicodeKey(), evt.GetModifiers())));
	}
};

/// Interface Hotkeys preferences subpage
Interface_Hotkeys::Interface_Hotkeys(wxTreebook *book, Preferences *parent)
: OptionPage(book, parent, _("Hotkeys"), PAGE_SUB)
, model(new HotkeyDataViewModel(parent))
{
	quick_search = new wxSearchCtrl(this, -1);
	wxButton *new_button = new wxButton(this, -1, _("New"));
	wxButton *edit_button = new wxButton(this, -1, _("Edit"));
	wxButton *delete_button = new wxButton(this, -1, _("Delete"));

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

void Interface_Hotkeys::OnNewButton(wxCommandEvent&) {
	model->New(dvc->GetSelection());
}

void Interface_Hotkeys::OnEditButton(wxCommandEvent&) {
	dvc->StartEditor(dvc->GetSelection(), 0);
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

void Interface_Hotkeys::OnClearFilter(wxCommandEvent &evt) {
	quick_search->SetValue("");
}

/// Paths preferences page
Paths::Paths(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Paths")) {
	wxFlexGridSizer *general = PageSizer(_("General"));
	general->Add(new wxStaticText(this, wxID_ANY, "TBD..."), 0, wxALL, 5);

	SetSizerAndFit(sizer);
}

/// File Associations preferences page
File_Associations::File_Associations(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("File Associations")) {
	wxFlexGridSizer *assoc = PageSizer(_("General"));
	assoc->Add(new wxStaticText(this, wxID_ANY, "TBD..."), 0, wxALL, 5);

	SetSizerAndFit(sizer);
}


/// Backup preferences page
Backup::Backup(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Backup")) {
	wxFlexGridSizer *save = PageSizer(_("Automatic Save"));
	OptionAdd(save, _("Enable"), "App/Auto/Backup");
	CellSkip(save);
	OptionAdd(save, _("Interval in seconds"), "App/Auto/Save Every Seconds");
	OptionBrowse(save, _("Path"), "Path/Auto/Save");

	wxFlexGridSizer *backup = PageSizer(_("Automatic Backup"));
	OptionAdd(backup, _("Enable"), "App/Auto/Backup");
	CellSkip(backup);
	OptionBrowse(backup, _("Path"), "Path/Auto/Backup");

	SetSizerAndFit(sizer);
}

/// Automation preferences page
Automation::Automation(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Automation")) {
	wxFlexGridSizer *general = PageSizer(_("General"));

	OptionAdd(general, _("Base path"), "Path/Automation/Base");
	OptionAdd(general, _("Include path"), "Path/Automation/Include");
	OptionAdd(general, _("Auto-load path"), "Path/Automation/Autoload");

	const wxString tl_arr[6] = { _("Fatal"), _("Error"), _("Warning"), _("Hint"), _("Debug"), _("Trace") };
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


/// Advanced Interface preferences subpage
Advanced_Interface::Advanced_Interface(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Backup"), PAGE_SUB) {
	wxFlexGridSizer *interface_ = PageSizer(_("Interface"));

	interface_->Add(new wxStaticText(this, wxID_ANY, "TBD..."), 0, wxALL, 5);

	SetSizerAndFit(sizer);
}

static wxArrayString vec_to_arrstr(std::vector<std::string> const& vec) {
	wxArrayString arrstr;
	std::copy(vec.begin(), vec.end(), std::back_inserter(arrstr));
	return arrstr;
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

	OptionAdd(spectrum, _("Cutoff"), "Audio/Renderer/Spectrum/Cutoff");

	const wxString sq_arr[4] = { _("Regular quality"), _("Better quality"), _("High quality"), _("Insane quality") };
	wxArrayString sq_choice(4, sq_arr);
	OptionChoice(spectrum, _("Quality"), sq_choice, "Audio/Renderer/Spectrum/Quality");

	OptionAdd(spectrum, _("Cache memory max (MB)"), "Audio/Renderer/Spectrum/Memory Max", 2, 1024);

#if defined(WIN32) || defined(_DEBUG)
	wxFlexGridSizer *windows = PageSizer(_("Windows Specific"));
	const wxString adm_arr[3] = { "ConvertToMono", "GetLeftChannel", "GetRightChannel" };
	wxArrayString adm_choice(3, adm_arr);
	OptionChoice(windows, _("Avisynth down-mixer"), adm_choice, "Audio/Downmixer");
#endif

	SetSizerAndFit(sizer);
}


/// Advanced Video preferences subpage
Advanced_Video::Advanced_Video(wxTreebook *book, Preferences *parent): OptionPage(book, parent, _("Video"), PAGE_SUB) {
	wxFlexGridSizer *expert = PageSizer(_("Expert"));

	wxArrayString vp_choice = vec_to_arrstr(VideoProviderFactory::GetClasses());
	OptionChoice(expert, _("Video provider"), vp_choice, "Video/Provider");

	wxArrayString sp_choice = vec_to_arrstr(SubtitlesProviderFactory::GetClasses());
	OptionChoice(expert, _("Subtitle provider"), sp_choice, "Subtitle/Provider");


#if defined(WIN32) || defined(_DEBUG)
	wxFlexGridSizer *windows = PageSizer(_("Windows Specific"));
	OptionAdd(windows, _("Allow pre-2.56a Avisynth"), "Provider/Avisynth/Allow Ancient");
	CellSkip(windows);
	OptionAdd(windows, _("Avisynth memory limit"), "Provider/Avisynth/Memory Max");
#endif

	SetSizerAndFit(sizer);
}

void Preferences::SetOption(std::string const& name, wxAny value) {
	pending_changes[name] = value;
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

void Preferences::OnApply(wxCommandEvent &event) {
	for (std::map<std::string, wxAny>::iterator cur = pending_changes.begin(); cur != pending_changes.end(); ++cur) {
		agi::OptionValue *opt = OPT_SET(cur->first);
		switch (opt->GetType()) {
			case agi::OptionValue::Type_Bool:
				opt->SetBool(cur->second.As<bool>());
				break;
			case agi::OptionValue::Type_Colour:
				opt->SetColour(cur->second.As<agi::Colour>());
				break;
			case agi::OptionValue::Type_Double:
				opt->SetDouble(cur->second.As<double>());
				break;
			case agi::OptionValue::Type_Int:
				opt->SetInt(cur->second.As<int>());
				break;
			case agi::OptionValue::Type_String:
				opt->SetString(cur->second.As<std::string>());
				break;
			default:
				throw PreferenceNotSupported("Unsupported type");
		}
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
	//	SetIcon(BitmapToIcon(GETIMAGE(options_button_24)));

	book = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	new General(book, this);
	new Subtitles(book, this);
	new Audio(book, this);
	new Video(book, this);
	new Interface(book, this);
	new Interface_Colours(book, this);
	new Interface_Hotkeys(book, this);
	new Paths(book, this);
	new File_Associations(book, this);
	new Backup(book, this);
	new Automation(book, this);
	new Advanced(book, this);
	new Advanced_Interface(book, this);
	new Advanced_Audio(book, this);
	new Advanced_Video(book, this);

	book->Fit();

	book->ChangeSelection(OPT_GET("Tool/Preferences/Page")->GetInt());
	book->Bind(wxEVT_COMMAND_TREEBOOK_PAGE_CHANGED, &PageChanged);

	// Bottom Buttons
	wxStdDialogButtonSizer *stdButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxAPPLY);
	applyButton = stdButtonSizer->GetApplyButton();
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *defaultButton = new wxButton(this, -1, _("Restore Defaults"));
	buttonSizer->Add(defaultButton, wxSizerFlags(0).Expand());
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(stdButtonSizer, wxSizerFlags(0).Expand());

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, wxSizerFlags(1).Expand().Border());
	mainSizer->Add(buttonSizer, wxSizerFlags(0).Expand().Border(wxALL & ~wxTOP));

	SetSizerAndFit(mainSizer);
	SetMinSize(wxSize(-1, 500));
	SetMaxSize(wxSize(-1, 500));
	CenterOnParent();

	applyButton->Enable(false);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnApply, this, wxID_APPLY);
}

Preferences::~Preferences() {
}
