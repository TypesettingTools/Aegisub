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
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/filefn.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/treebook.h>
#endif

#include <libaegisub/exception.h>

#include "colour_button.h"
#include "compat.h"
#include "libresrc/libresrc.h"
#include "preferences.h"
#include "main.h"
#include "subtitles_provider_manager.h"
#include "video_provider_manager.h"
#include "audio_player_manager.h"
#include "audio_provider_manager.h"

/// Define make all platform-specific options visible in a single view.
#define SHOW_ALL 1

DEFINE_BASE_EXCEPTION_NOINNER(PreferencesError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceIncorrectType, PreferencesError, "preferences/incorrect_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceNotSupported, PreferencesError, "preferences/not_supported")



void Preferences::OptionChoice(wxPanel *parent, wxFlexGridSizer *flex, const wxString &name, const wxArrayString &choices, const char *opt_name) {
	agi::OptionValue *opt = OPT_GET(opt_name);

	int type = opt->GetType();
	wxString selection;

	switch (type) {
		case agi::OptionValue::Type_Int: {
			selection = choices.Item(opt->GetInt());
			break;
		}
		case agi::OptionValue::Type_String: {
			selection.assign(opt->GetString());
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}

	flex->Add(new wxStaticText(parent, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
	wxComboBox *cb = new wxComboBox(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY | wxCB_DROPDOWN);
	cb->SetValue(selection);
	flex->Add(cb, 1, wxEXPAND, 0);
}


void Preferences::OptionBrowse(wxPanel *parent, wxFlexGridSizer *flex, const wxString &name, BrowseType browse_type, const char *opt_name) {

	agi::OptionValue *opt = OPT_GET(opt_name);

	if (opt->GetType() != agi::OptionValue::Type_String)
		throw PreferenceIncorrectType("Option must be agi::OptionValue::Type_String for BrowseButton.");

	flex->Add(new wxStaticText(parent, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);

	wxFlexGridSizer *button_flex = new wxFlexGridSizer(2,5,5);
	button_flex->AddGrowableCol(0,1);
	flex->Add(button_flex, 1, wxEXPAND, 5);

	wxTextCtrl *text = new wxTextCtrl(parent, wxID_ANY , opt->GetString(), wxDefaultPosition, wxDefaultSize);
	button_flex->Add(text, 1, wxEXPAND);
	BrowseButton *browse = new BrowseButton(parent, wxID_ANY, wxEmptyString, browse_type);
	browse->Bind(text);
	button_flex->Add(browse, 1, wxEXPAND);

}



void Preferences::OptionAdd(wxPanel *parent, wxFlexGridSizer *flex, const wxString &name, const char *opt_name, double min, double max, double inc) {

	agi::OptionValue *opt = OPT_GET(opt_name);

	int type = opt->GetType();

	switch (type) {

		case agi::OptionValue::Type_Bool: {
			wxCheckBox *cb = new wxCheckBox(parent, wxID_ANY, name);
			flex->Add(cb, 1, wxEXPAND, 0);
			cb->SetValue(opt->GetBool());
			break;
		}

		case agi::OptionValue::Type_Int:
		case agi::OptionValue::Type_Double: {
			flex->Add(new wxStaticText(parent, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt(), inc);
			flex->Add(scd);

			break;
		}

		case agi::OptionValue::Type_String: {
			flex->Add(new wxStaticText(parent, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxTextCtrl *text = new wxTextCtrl(parent, wxID_ANY , lagi_wxString(opt->GetString()), wxDefaultPosition, wxDefaultSize);
			flex->Add(text, 1, wxEXPAND);
			break;
		}

		case agi::OptionValue::Type_Colour: {
			flex->Add(new wxStaticText(parent, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			flex->Add(new ColourButton(parent, wxID_ANY, wxSize(40,10), lagi_wxColour(opt->GetColour())));
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}


class OptionPage: public wxPanel {
public:
	wxSizer *sizer;

	OptionPage(wxTreebook *book, wxString name): wxPanel(book, -1) {
		book->AddPage(this, name, true);
		sizer = new wxBoxSizer(wxVERTICAL);
	}

	~OptionPage() {}

	void OptionAdd(wxFlexGridSizer *&flex, const wxString &name, const char *opt_name, double min=0, double max=100, double inc=1) {

		agi::OptionValue *opt = OPT_GET(opt_name);

		int type = opt->GetType();

		switch (type) {

			case agi::OptionValue::Type_Bool: {
				wxCheckBox *cb = new wxCheckBox(this, wxID_ANY, name);
				flex->Add(cb, 1, wxEXPAND, 0);
				cb->SetValue(opt->GetBool());
				break;
			}

			case agi::OptionValue::Type_Int:
			case agi::OptionValue::Type_Double: {
				flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
				wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt(), inc);
				flex->Add(scd);

				break;
			}

			case agi::OptionValue::Type_String: {
				flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
				wxTextCtrl *text = new wxTextCtrl(this, wxID_ANY , lagi_wxString(opt->GetString()), wxDefaultPosition, wxDefaultSize);
				flex->Add(text, 1, wxEXPAND);
				break;
			}

			case agi::OptionValue::Type_Colour: {
				flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
				flex->Add(new ColourButton(this, wxID_ANY, wxSize(40,10), lagi_wxColour(opt->GetColour())));
				break;
			}

			default:
				throw PreferenceNotSupported("Unsupported type");
		}
	}

	wxFlexGridSizer* PageSizer(wxString name) {
		wxSizer *tmp_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, name);
		sizer->Add(tmp_sizer, 0,wxEXPAND, 5);
		wxFlexGridSizer *flex = new wxFlexGridSizer(2,5,5);
		flex->AddGrowableCol(0,1);
		tmp_sizer->Add(flex, 1, wxEXPAND, 5);
		sizer->AddSpacer(8);
		return flex;
	}

};


void Preferences::OnOK(wxCommandEvent &event) {
	EndModal(0);
}


void Preferences::OnApply(wxCommandEvent &event) {
}


void Preferences::OnCancel(wxCommandEvent &event) {
	EndModal(0);
}


#define PAGE_CREATE(name)                           \
	wxPanel *panel = new wxPanel(book, -1);         \
	book->AddPage(panel, name, true);               \
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);    \

#define SUBPAGE_CREATE(name)                        \
	wxPanel *panel = new wxPanel(book, -1);         \
	book->AddSubPage(panel, name, true);            \
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);    \


#define PAGE_SIZER(name, name_value)                                                           \
	wxSizer *name_value##_sizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, name);             \
	sizer->Add(name_value##_sizer, 0,wxEXPAND, 5);                                             \
	wxFlexGridSizer *name_value##_flex = new wxFlexGridSizer(2,5,5);                           \
	name_value##_flex->AddGrowableCol(0,1);                                                    \
	name_value##_sizer->Add(name_value##_flex, 1, wxEXPAND, 5);                                \
	sizer->AddSpacer(8);

// name_value##_flex->SetFlexibleDirection(wxVERTICAL);

#define PAGE_END() \
	panel->SetSizerAndFit(sizer);

/// Skip a cell in a FlexGridSizer -- there's probably a better way to do this.
#define CELL_SKIP(flex) \
	flex->Add(new wxStaticText(panel, wxID_ANY , wxEmptyString), 0, wxALL, 5);



class General: public OptionPage {
public:
	General(wxTreebook *book): OptionPage(book, _("General")) {

		wxFlexGridSizer *startup = PageSizer(_("Startup"));
		OptionAdd(startup, _("Check for updates"), "App/Splash");
		OptionAdd(startup, _("Show Splash Screen"), "App/Splash");

		wxFlexGridSizer *recent = PageSizer(_("Recently Used Lists"));
		OptionAdd(recent, _("Files"), "Limits/MRU");
		OptionAdd(recent, _("Find/Replace"), "Limits/Find Replace");
		sizer->AddSpacer(15);

		wxFlexGridSizer *undo = PageSizer(_("Undo / Redo Settings"));
		OptionAdd(undo, _("Undo Levels"), "Limits/MRU");

		SetSizerAndFit(sizer);
	}
};


void Preferences::Subtitles(wxTreebook *book) {
	PAGE_CREATE(_("Subtitles"))

	PAGE_SIZER(_("Options"), general)

	OptionAdd(panel, general_flex, _("Enable call tips"), "App/Call Tips");
	OptionAdd(panel, general_flex, _("Enable syntax highlighting"), "Subtitle/Highlight/Syntax");
	OptionAdd(panel, general_flex, _("Link commiting of times"), "Subtitle/Edit Box/Link Time Boxes Commit");
	OptionAdd(panel, general_flex, _("Overwrite-Insertion in time boxes"), "Subtitle/Time Edit/Insert Mode");

	PAGE_SIZER(_("Grid"), grid)
	OptionAdd(panel, grid_flex, _("Allow grid to take focus"), "Subtitle/Grid/Focus Allow");
	OptionAdd(panel, grid_flex, _("Highlight visible subtitles"), "Subtitle/Grid/Highlight Subtitles in Frame");

	PAGE_END()
}


void Preferences::Audio(wxTreebook *book) {
	PAGE_CREATE(_("Audio"))

	PAGE_SIZER(_("Options"), general)
	OptionAdd(panel, general_flex, _("Grab times from line upon selection"), "Audio/Grab Times on Select");
	OptionAdd(panel, general_flex, _("Default mouse wheel to zoom"), "Audio/Wheel Default to Zoom");
	OptionAdd(panel, general_flex, _("Lock scroll on cursor"), "Audio/Lock Scroll on Cursor");
	OptionAdd(panel, general_flex, _("Snap to keyframes"), "Audio/Display/Snap/Keyframes");
	OptionAdd(panel, general_flex, _("Snap to adjacent lines"), "Audio/Display/Snap/Other Lines");
	OptionAdd(panel, general_flex, _("Auto-focus on mouse over"), "Audio/Auto/Focus");
	OptionAdd(panel, general_flex, _("Play audio when stepping in video"), "Audio/Plays When Stepping Video");

	CELL_SKIP(general_flex)

	OptionAdd(panel, general_flex, _("Default timing length"), "Timing/Default Duration", 0, 36000);
	OptionAdd(panel, general_flex, _("Default lead-in length"), "Audio/Lead/IN", 0, 36000);
	OptionAdd(panel, general_flex, _("Default lead-out length"), "Audio/Lead/OUT", 0, 36000);

	const wxString dtl_arr[3] = { _("Don't show"), _("Show previous"), _("Show all") };
	wxArrayString choice_dtl(3, dtl_arr);
	OptionChoice(panel, general_flex, _("Show inactive lines"), choice_dtl, "Audio/Inactive Lines Display Mode");

	OptionAdd(panel, general_flex, _("Start-marker drag sensitivity"), "Audio/Start Drag Sensitivity", 1, 15);

	PAGE_SIZER(_("Display Visual Options"), display)
	OptionAdd(panel, display_flex, _("Secondary lines"), "Audio/Display/Draw/Secondary Lines");
	OptionAdd(panel, display_flex, _("Selection background"), "Audio/Display/Draw/Selection Background");
	OptionAdd(panel, display_flex, _("Timeline"), "Audio/Display/Draw/Timeline");
	OptionAdd(panel, display_flex, _("Cursor time"), "Audio/Display/Draw/Cursor Time");
	OptionAdd(panel, display_flex, _("Keyframes"), "Audio/Display/Draw/Keyframes");
	OptionAdd(panel, display_flex, _("Video position"), "Audio/Display/Draw/Video Position");

	PAGE_END()
}


void Preferences::Video(wxTreebook *book) {

	PAGE_CREATE(_("Video"))

	PAGE_SIZER(_("Options"), general)

	OptionAdd(panel, general_flex, _("Show keyframes in slider"), "Video/Slider/Show Keyframes");
	OptionAdd(panel, general_flex, _("Always show visual tools"), "Tool/Visual/Always Show");

	const wxString cres_arr[3] = { _("Never"), _("Ask"), _("Always") };
	wxArrayString choice_res(3, cres_arr);
	OptionChoice(panel, general_flex, _("Match video resolution on open"), choice_res, "Video/Check Script Res");

	const wxString czoom_arr[24] = { _T("12.5%"), _T("25%"), _T("37.5%"), _T("50%"), _T("62.5%"), _T("75%"), _T("87.5%"), _T("100%"), _T("112.5%"), _T("125%"), _T("137.5%"), _T("150%"), _T("162.5%"), _T("175%"), _T("187.5%"), _T("200%"), _T("212.5%"), _T("225%"), _T("237.5%"), _T("250%"), _T("262.5%"), _T("275%"), _T("287.5%"), _T("300%") };
	wxArrayString choice_zoom(24, czoom_arr);
	OptionChoice(panel, general_flex, _("Default Zoom"), choice_zoom, "Video/Default Zoom");

	OptionAdd(panel, general_flex, _("Fast jump step in frames"), "Video/Slider/Fast Jump Step");

	const wxString cscr_arr[3] = { _("?video"), _("?script"), _(".") };
	wxArrayString scr_res(3, cscr_arr);
	OptionChoice(panel, general_flex, _("Screenshot save path"), scr_res, "Path/Screenshot");



	panel->SetSizerAndFit(sizer);


}


void Preferences::Interface(wxTreebook *book) {
	PAGE_CREATE(_("Interface"))

	PAGE_SIZER(_("Subtitle Grid"), grid)
	OptionBrowse(panel, grid_flex, _("Font face"), BROWSE_FONT, "Subtitle/Grid/Font Face");
	OptionAdd(panel, grid_flex, _("Font size"), "Subtitle/Grid/Font Size", 3, 42);

	OptionAdd(panel, grid_flex, _("Hide overrides symbol"), "Subtitle/Grid/Hide Overrides Char");

	PAGE_END()
}

void Preferences::Interface_Colours(wxTreebook *book) {

	wxScrolled<wxPanel> *panel = new wxScrolled<wxPanel>(book, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
	panel->SetScrollbars(0, 20, 0, 50);
	book->AddSubPage(panel, _("Colours"), true);
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

	PAGE_SIZER(_("General"), general)
	OptionAdd(panel, general_flex, _("Modified Background"), "Colour/Background/Modified");

	PAGE_SIZER(_("Audio Display"), audio)
	OptionAdd(panel, audio_flex, _("Play cursor"), "Colour/Audio Display/Play Cursor");
	OptionAdd(panel, audio_flex, _("Background"), "Colour/Audio Display/Background/Background");
	OptionAdd(panel, audio_flex, _("Selection background"), "Colour/Audio Display/Background/Selection");
	OptionAdd(panel, audio_flex, _("Selection background modified"), "Colour/Audio Display/Background/Selection Modified");
	OptionAdd(panel, audio_flex, _("Seconds boundaries"), "Colour/Audio Display/Seconds Boundaries");
	OptionAdd(panel, audio_flex, _("Waveform"), "Colour/Audio Display/Waveform");
	OptionAdd(panel, audio_flex, _("Waveform selected"), "Colour/Audio Display/Waveform Selected");
	OptionAdd(panel, audio_flex, _("Waveform Modified"), "Colour/Audio Display/Waveform Modified");
	OptionAdd(panel, audio_flex, _("Waveform Inactive"), "Colour/Audio Display/Waveform Inactive");
	OptionAdd(panel, audio_flex, _("Line boundary start"), "Colour/Audio Display/Line boundary Start");
	OptionAdd(panel, audio_flex, _("Line boundary end"), "Colour/Audio Display/Line boundary End");
	OptionAdd(panel, audio_flex, _("Line boundary inactive line"), "Colour/Audio Display/Line Boundary Inactive Line");
	OptionAdd(panel, audio_flex, _("Syllable text"), "Colour/Audio Display/Syllable Text");
	OptionAdd(panel, audio_flex, _("Syllable boundaries"), "Colour/Audio Display/Syllable Boundaries");

	PAGE_SIZER(_("Syntax Highlighting"), syntax)
	OptionAdd(panel, syntax_flex, _("Normal"), "Colour/Subtitle/Syntax/Normal");
	OptionAdd(panel, syntax_flex, _("Brackets"), "Colour/Subtitle/Syntax/Brackets");
	OptionAdd(panel, syntax_flex, _("Slashes and Parentheses"), "Colour/Subtitle/Syntax/Slashes");
	OptionAdd(panel, syntax_flex, _("Tags"), "Colour/Subtitle/Syntax/Highlight Tags");
	OptionAdd(panel, syntax_flex, _("Parameters"), "Colour/Subtitle/Syntax/Parameters");
	OptionAdd(panel, syntax_flex, _("Error"), "Colour/Subtitle/Syntax/Error");
	OptionAdd(panel, syntax_flex, _("Error Background"), "Colour/Subtitle/Syntax/Background/Error");
	OptionAdd(panel, syntax_flex, _("Line Break"), "Colour/Subtitle/Syntax/Line Break");
	OptionAdd(panel, syntax_flex, _("Karaoke templates"), "Colour/Subtitle/Syntax/Karaoke Template");

	PAGE_SIZER(_("Subtitle Grid"), grid)
	OptionAdd(panel, grid_flex, _("Standard foreground"), "Colour/Subtitle Grid/Standard");
	OptionAdd(panel, grid_flex, _("Standard background"), "Colour/Subtitle Grid/Background/Background");
	OptionAdd(panel, grid_flex, _("Selection foreground"), "Colour/Subtitle Grid/Selection");
	OptionAdd(panel, grid_flex, _("Selection background"), "Colour/Subtitle Grid/Background/Selection");
	OptionAdd(panel, grid_flex, _("Comment background"), "Colour/Subtitle Grid/Background/Comment");
	OptionAdd(panel, grid_flex, _("Selected comment background"), "Colour/Subtitle Grid/Background/Selected Comment");
	OptionAdd(panel, grid_flex, _("Left Column"), "Colour/Subtitle Grid/Left Column");
	OptionAdd(panel, grid_flex, _("Active Line Border"), "Colour/Subtitle Grid/Active Border");
	OptionAdd(panel, grid_flex, _("Lines"), "Colour/Subtitle Grid/Lines");
	PAGE_END()
}

void Preferences::Interface_Hotkeys(wxTreebook *book) {
	SUBPAGE_CREATE(_("Hotkeys"))

	PAGE_SIZER(_("Hotkeys"), hotkey)
	hotkey_flex->Add(new wxStaticText(panel, wxID_ANY , _T("To be added after hotkey rewrite.")), 0, wxALL, 5);

	PAGE_END()
}

void Preferences::Paths(wxTreebook *book) {
	PAGE_CREATE(_("Paths"))

//	OptionBrowse(panel, general_flex, _("Dictionaries path"), BROWSE_FOLDER, "Path/Dictionary")

	PAGE_END()
}

void Preferences::File_Associations(wxTreebook *book) {
	PAGE_CREATE(_("File Assoc."))
	PAGE_END()
}

void Preferences::Backup(wxTreebook *book) {
	PAGE_CREATE(_("Backup"))


	PAGE_SIZER(_("Automatic Save"), save)
	OptionAdd(panel, save_flex, _("Enable"), "App/Auto/Backup");
	CELL_SKIP(save_flex)
	OptionAdd(panel, save_flex, _("Interval in seconds."), "App/Auto/Save Every Seconds");
	OptionBrowse(panel, save_flex, _("Path"), BROWSE_FOLDER, "Path/Auto/Save");

	PAGE_SIZER(_("Automatic Backup"), backup)
	CELL_SKIP(backup_flex)
	OptionAdd(panel, backup_flex, _("Enable"), "App/Auto/Backup");
	OptionBrowse(panel, backup_flex, _("Path"), BROWSE_FOLDER, "Path/Auto/Backup");

	PAGE_END()
}

void Preferences::Automation(wxTreebook *book) {
	PAGE_CREATE(_("Automation"))

	PAGE_SIZER(_("Options"), general)

	OptionAdd(panel, general_flex, _("Base path"), "Path/Automation/Base");
	OptionAdd(panel, general_flex, _("Include path"), "Path/Automation/Include");
	OptionAdd(panel, general_flex, _("Auto-load path"), "Path/Automation/Autoload");

	const wxString tl_arr[6] = { _("Fatal"), _("Error"), _("Warning"), _("Hint"), _("Debug"), _("Trace") };
	wxArrayString tl_choice(6, tl_arr);
	OptionChoice(panel, general_flex, _("Trace level"), tl_choice, "Automation/Trace Level");

	const wxString tp_arr[3] = { _("Normal"), _("Below Normal (recommended)"), _("Lowest") };
	wxArrayString tp_choice(3, tp_arr);
	OptionChoice(panel, general_flex, _("Thread priority"), tp_choice, "Automation/Lua/Thread Priority");

	const wxString ar_arr[4] = { _("No scripts"), _("Subtitle-local scripts"), _("Global autoload scripts"), _("All scripts") };
	wxArrayString ar_choice(4, ar_arr);
	OptionChoice(panel, general_flex, _("Autoreload on Export"), ar_choice, "Automation/Autoreload Mode");


	PAGE_END()
}


void Preferences::Advanced(wxTreebook *book) {
	PAGE_CREATE(_("Advanced"))

	wxStaticText *warning = new wxStaticText(panel, wxID_ANY ,_("Changing these settings might result in bugs and/or crashes.  Do not touch these unless you know what you're doing."));
	warning->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	sizer->Fit(panel);
	warning->Wrap(400);
	sizer->Add(warning, 0, wxALL, 5);

	PAGE_END()
}

void Preferences::Advanced_Interface(wxTreebook *book) {
	SUBPAGE_CREATE(_("Interface"))
	PAGE_END()
}


void Preferences::Advanced_Audio(wxTreebook *book) {
	SUBPAGE_CREATE(_("Audio"))

	PAGE_SIZER(_("Options"), expert)

	wxArrayString ap_choice = AudioProviderFactoryManager::GetFactoryList();
	OptionChoice(panel, expert_flex, _("Audio provider"), ap_choice, "Audio/Provider");

	wxArrayString apl_choice = AudioPlayerFactoryManager::GetFactoryList();
	OptionChoice(panel, expert_flex, _("Audio player"), apl_choice, "Audio/Player");

	PAGE_SIZER(_("Cache"), cache)
	const wxString ct_arr[3] = { _("None (NOT RECOMMENDED)"), _("RAM"), _("Hard Disk") };
	wxArrayString ct_choice(3, ct_arr);
	OptionChoice(panel, cache_flex, _("Cache type"), ct_choice, "Audio/Cache/Type");

	OptionBrowse(panel, cache_flex, _("Path"), BROWSE_FOLDER, "Audio/Cache/HD/Location");
	OptionAdd(panel, cache_flex, _("File name"), "Audio/Cache/HD/Name");


	PAGE_SIZER(_("Spectrum"), spectrum)

	OptionAdd(panel, spectrum_flex, _("Cutoff"), "Audio/Renderer/Spectrum/Cutoff");

	const wxString sq_arr[4] = { _("Regular quality"), _("Better quality"), _("High quality"), _("Insane quality") };
	wxArrayString sq_choice(4, sq_arr);
	OptionChoice(panel, spectrum_flex, _("Quality"), sq_choice, "Audio/Renderer/Spectrum/Quality");
	OptionAdd(panel, spectrum_flex, _("Cache memory max (MB)"), "Audio/Renderer/Spectrum/Memory Max", 2, 1024);

#if defined(WIN32) || defined(SHOW_ALL)
	PAGE_SIZER(_("Windows Only"), windows);
	const wxString adm_arr[3] = { _T("ConvertToMono"), _T("GetLeftChannel"), _T("GetRightChannel") };
	wxArrayString adm_choice(3, adm_arr);
	OptionChoice(panel, windows_flex, _("Avisynth down-mixer"), adm_choice, "Audio/Downmixer");
#endif

	PAGE_END()
}


void Preferences::Advanced_Video(wxTreebook *book) {
	SUBPAGE_CREATE(_("Video"))

	PAGE_SIZER(_("Options"), expert)
	wxArrayString vp_choice = VideoProviderFactoryManager::GetFactoryList();
	OptionChoice(panel, expert_flex, _("Video provider"), vp_choice, "Video/Provider");

	wxArrayString sp_choice = SubtitlesProviderFactoryManager::GetFactoryList();
	OptionChoice(panel, expert_flex, _("Subtitle provider"), sp_choice, "Subtitle/Provider");



#if defined(WIN32) || defined(SHOW_ALL)
	PAGE_SIZER(_("Windows Only"), windows);

	OptionAdd(panel, windows_flex, _("Allow pre-2.56a Avisynth"), "Provider/Avisynth/Allow Ancient");
	CELL_SKIP(windows_flex)
	OptionAdd(panel, windows_flex, _("Avisynth memory limit"), "Provider/Avisynth/Memory Max");
#endif

	PAGE_END()
}


Preferences::Preferences(wxWindow *parent): wxDialog(parent, -1, _("Preferences"), wxDefaultPosition, wxSize(-1, 500)) {
//	SetIcon(BitmapToIcon(GETIMAGE(options_button_24)));

	book = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	general = new General(book);

	Subtitles(book);
	Audio(book);
	Video(book);
	Interface(book);
	Interface_Colours(book);
	Interface_Hotkeys(book);
	Paths(book);
	File_Associations(book);
	Backup(book);
	Automation(book);
	Advanced(book);
	Advanced_Interface(book);
	Advanced_Audio(book);
	Advanced_Video(book);

	book->Fit();

	/// @todo Save the last page and start with that page on next launch.
	book->ChangeSelection(0);

	// Bottom Buttons
	wxStdDialogButtonSizer *stdButtonSizer = new wxStdDialogButtonSizer();
	stdButtonSizer->AddButton(new wxButton(this,wxID_OK));
	stdButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	stdButtonSizer->AddButton(new wxButton(this,wxID_APPLY));
	stdButtonSizer->Realize();
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *defaultButton = new wxButton(this,2342,_("Restore Defaults"));
	buttonSizer->Add(defaultButton,0,wxEXPAND);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(stdButtonSizer,0,wxEXPAND);


	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, 1 ,wxEXPAND | wxALL, 5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	SetSizerAndFit(mainSizer);
	this->SetMinSize(wxSize(-1, 500));
	this->SetMaxSize(wxSize(-1, 500));
	CenterOnParent();



}

Preferences::~Preferences() {

}


BEGIN_EVENT_TABLE(Preferences, wxDialog)
    EVT_BUTTON(wxID_OK, Preferences::OnOK)
    EVT_BUTTON(wxID_CANCEL, Preferences::OnCancel)
    EVT_BUTTON(wxID_APPLY, Preferences::OnApply)
END_EVENT_TABLE()
