// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#include "font_file_lister.h"
#include "font_file_lister_fontconfig.h"

#include "ass_file.h"
#include "compat.h"
#include "dialog_manager.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "scintilla_text_ctrl.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/make_unique.h>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

namespace {
class DialogFontsCollector final : public wxDialog {
	AssFile *subs;

	ScintillaTextCtrl *collection_log;
	wxButton *close_btn;
	wxButton *dest_browse_button;
	wxButton *start_btn;
	wxRadioBox *collection_mode;
	wxStaticText *dest_label;
	wxTextCtrl *dest_ctrl;

	void OnStart(wxCommandEvent &);
	void OnBrowse(wxCommandEvent &);
	void OnRadio(wxCommandEvent &e);

	/// Append text to log message from worker thread
	void OnAddText(wxThreadEvent &event);
	/// Collection complete notification from the worker thread to reenable buttons
	void OnCollectionComplete(wxThreadEvent &);

public:
	DialogFontsCollector(agi::Context *c);
};

enum FcMode {
	CheckFontsOnly = 0,
	CopyToFolder = 1,
	CopyToScriptFolder = 2,
	CopyToZip = 3,
	SymlinkToFolder = 4
};

wxDEFINE_EVENT(EVT_ADD_TEXT, wxThreadEvent);
wxDEFINE_EVENT(EVT_COLLECTION_DONE, wxThreadEvent);

void FontsCollectorThread(AssFile *subs, agi::fs::path const& destination, FcMode oper, wxEvtHandler *collector) {
	agi::dispatch::Background().Async([=]{
		auto AppendText = [&](wxString text, int colour) {
			wxThreadEvent event(EVT_ADD_TEXT);
			event.SetPayload(std::make_pair(colour, text));
			collector->AddPendingEvent(event);
		};

		FontConfigFontFileLister lister(AppendText);
		auto paths = FontCollector(AppendText, lister).GetFontPaths(subs);
		if (paths.empty()) {
			collector->AddPendingEvent(wxThreadEvent(EVT_COLLECTION_DONE));
			return;
		}

		// Copy fonts
		switch (oper) {
			case CheckFontsOnly:
				collector->AddPendingEvent(wxThreadEvent(EVT_COLLECTION_DONE));
				return;
			case SymlinkToFolder:
				AppendText(_("Symlinking fonts to folder...\n"), 0);
				break;
			case CopyToScriptFolder:
			case CopyToFolder:
				AppendText(_("Copying fonts to folder...\n"), 0);
				break;
			case CopyToZip:
				AppendText(_("Copying fonts to archive...\n"), 0);
				break;
		}

		// Open zip stream if saving to compressed archive
		std::unique_ptr<wxFFileOutputStream> out;
		std::unique_ptr<wxZipOutputStream> zip;
		if (oper == CopyToZip) {
			try {
				agi::fs::CreateDirectory(destination.parent_path());
			}
			catch (agi::fs::FileSystemError const& e) {
				AppendText(wxString::Format(_("* Failed to create directory '%s': %s.\n"),
					destination.parent_path().wstring(), to_wx(e.GetChainedMessage())), 2);
				collector->AddPendingEvent(wxThreadEvent(EVT_COLLECTION_DONE));
				return;
			}

			out = agi::make_unique<wxFFileOutputStream>(destination.wstring());
			if (out->IsOk())
				zip = agi::make_unique<wxZipOutputStream>(*out);

			if (!out->IsOk() || !zip || !zip->IsOk()) {
				AppendText(wxString::Format(_("* Failed to open %s.\n"), destination.wstring()), 2);
				collector->AddPendingEvent(wxThreadEvent(EVT_COLLECTION_DONE));
				return;
			}
		}

		int64_t total_size = 0;
		bool allOk = true;
		for (auto path : paths) {
			path.make_preferred();

			int ret = 0;
			total_size += agi::fs::Size(path);

			switch (oper) {
				case SymlinkToFolder:
				case CopyToScriptFolder:
				case CopyToFolder: {
					auto dest = destination/path.filename();
					if (agi::fs::FileExists(dest))
						ret = 2;
#ifndef _WIN32
					else if (oper == SymlinkToFolder) {
						// returns 0 on success, -1 on error...
						if (symlink(path.c_str(), dest.c_str()))
							ret = 0;
						else
							ret = 3;
					}
#endif
					else {
						try {
							agi::fs::Copy(path, dest);
							ret = true;
						}
						catch (...) {
							ret = false;
						}
					}
				}
				break;

				case CopyToZip: {
					wxFFileInputStream in(path.wstring());
					if (!in.IsOk())
						ret = false;
					else {
						ret = zip->PutNextEntry(path.filename().wstring());
						zip->Write(in);
					}
				}
				default: break;
			}

			if (ret == 1)
				AppendText(wxString::Format(_("* Copied %s.\n"), path.wstring()), 1);
			else if (ret == 2)
				AppendText(wxString::Format(_("* %s already exists on destination.\n"), path.filename().wstring()), 3);
			else if (ret == 3)
				AppendText(wxString::Format(_("* Symlinked %s.\n"), path.wstring()), 1);
			else {
				AppendText(wxString::Format(_("* Failed to copy %s.\n"), path.wstring()), 2);
				allOk = false;
			}
		}

		if (allOk)
			AppendText(_("Done. All fonts copied."), 1);
		else
			AppendText(_("Done. Some fonts could not be copied."), 2);

		if (total_size > 32 * 1024 * 1024)
			AppendText(_("\nOver 32 MB of fonts were copied. Some of the fonts may not be loaded by the player if they are all attached to a Matroska file."), 2);

		AppendText("\n", 0);

		collector->AddPendingEvent(wxThreadEvent(EVT_COLLECTION_DONE));
	});
}

DialogFontsCollector::DialogFontsCollector(agi::Context *c)
: wxDialog(c->parent, -1, _("Fonts Collector"))
, subs(c->ass.get())
{
	SetIcon(GETICON(font_collector_button_16));

	wxString modes[] = {
		 _("Check fonts for availability")
		,_("Copy fonts to folder")
		,_("Copy fonts to subtitle file's folder")
		,_("Copy fonts to zipped archive")
#ifndef _WIN32
		,_("Symlink fonts to folder")
#endif
	};
	collection_mode = new wxRadioBox(this, -1, _("Action"), wxDefaultPosition, wxDefaultSize, countof(modes), modes, 1);
	collection_mode->SetSelection(mid<int>(0, OPT_GET("Tool/Fonts Collector/Action")->GetInt(), 4));

	if (config::path->Decode("?script") == "?script")
		collection_mode->Enable(2, false);

	wxStaticBoxSizer *destination_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Destination"));

	dest_label = new wxStaticText(this, -1, " ");
	dest_ctrl = new wxTextCtrl(this, -1, config::path->Decode(OPT_GET("Path/Fonts Collector Destination")->GetString()).wstring());
	dest_browse_button = new wxButton(this, -1, _("&Browse..."));

	wxSizer *dest_browse_sizer = new wxBoxSizer(wxHORIZONTAL);
	dest_browse_sizer->Add(dest_ctrl, wxSizerFlags(1).Border(wxRIGHT).Align(wxALIGN_CENTER_VERTICAL));
	dest_browse_sizer->Add(dest_browse_button, wxSizerFlags());

	destination_box->Add(dest_label, wxSizerFlags().Border(wxBOTTOM));
	destination_box->Add(dest_browse_sizer, wxSizerFlags().Expand());

	wxStaticBoxSizer *log_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Log"));
	collection_log = new ScintillaTextCtrl(this, -1, "", wxDefaultPosition, wxSize(600, 300));
	collection_log->SetWrapMode(wxSTC_WRAP_WORD);
	collection_log->SetMarginWidth(1, 0);
	collection_log->SetReadOnly(true);
	collection_log->StyleSetForeground(1, wxColour(0, 200, 0));
	collection_log->StyleSetForeground(2, wxColour(200, 0, 0));
	collection_log->StyleSetForeground(3, wxColour(200, 100, 0));
	log_box->Add(collection_log, wxSizerFlags().Border());

	wxStdDialogButtonSizer *button_sizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	start_btn = button_sizer->GetAffirmativeButton();
	close_btn = button_sizer->GetCancelButton();
	start_btn->SetLabel(_("&Start!"));
	start_btn->SetDefault();

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(collection_mode, wxSizerFlags().Expand().Border());
	main_sizer->Add(destination_box, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	main_sizer->Add(log_box, wxSizerFlags().Border(wxALL & ~wxTOP));
	main_sizer->Add(button_sizer, wxSizerFlags().Right().Border(wxALL & ~wxTOP));

	SetSizerAndFit(main_sizer);
	CenterOnParent();

	// Update the browse button and label
	wxCommandEvent evt;
	OnRadio(evt);

	start_btn->Bind(wxEVT_BUTTON, &DialogFontsCollector::OnStart, this);
	dest_browse_button->Bind(wxEVT_BUTTON, &DialogFontsCollector::OnBrowse, this);
	collection_mode->Bind(wxEVT_RADIOBOX, &DialogFontsCollector::OnRadio, this);
	button_sizer->GetHelpButton()->Bind(wxEVT_BUTTON, std::bind(&HelpButton::OpenPage, "Fonts Collector"));
	Bind(EVT_ADD_TEXT, &DialogFontsCollector::OnAddText, this);
	Bind(EVT_COLLECTION_DONE, &DialogFontsCollector::OnCollectionComplete, this);
}

void DialogFontsCollector::OnStart(wxCommandEvent &) {
	collection_log->SetReadOnly(false);
	collection_log->ClearAll();
	collection_log->SetReadOnly(true);

	agi::fs::path dest;
	int action = collection_mode->GetSelection();
	OPT_SET("Tool/Fonts Collector/Action")->SetInt(action);
	if (action != CheckFontsOnly) {
		dest = config::path->Decode(action == CopyToScriptFolder ? "?script/" : from_wx(dest_ctrl->GetValue()));

		if (action != CopyToZip) {
			if (agi::fs::FileExists(dest))
				wxMessageBox(_("Invalid destination."), _("Error"), wxOK | wxICON_ERROR | wxCENTER, this);
			try {
				agi::fs::CreateDirectory(dest);
			}
			catch (agi::Exception const&) {
				wxMessageBox(_("Could not create destination folder."), _("Error"), wxOK | wxICON_ERROR | wxCENTER, this);
				return;
			}
		}
		else if (agi::fs::DirectoryExists(dest) || dest.filename().empty()) {
			wxMessageBox(_("Invalid path for .zip file."), _("Error"), wxOK | wxICON_ERROR | wxCENTER, this);
			return;
		}
	}

	if (action != CheckFontsOnly)
		OPT_SET("Path/Fonts Collector Destination")->SetString(dest.string());

	// Disable the UI while it runs as we don't support canceling
	EnableCloseButton(false);
	start_btn->Enable(false);
	dest_browse_button->Enable(false);
	dest_ctrl->Enable(false);
	close_btn->Enable(false);
	collection_mode->Enable(false);
	dest_label->Enable(false);

	FontsCollectorThread(subs, dest, static_cast<FcMode>(action), GetEventHandler());
}

void DialogFontsCollector::OnBrowse(wxCommandEvent &) {
	wxString dest;
	if (collection_mode->GetSelection() == CopyToZip) {
		dest = wxFileSelector(
			_("Select archive file name"),
			dest_ctrl->GetValue(),
			wxFileName(dest_ctrl->GetValue()).GetFullName(),
			".zip", "Zip Archives (*.zip)|*.zip",
			wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	}
	else
		dest = wxDirSelector(_("Select folder to save fonts on"), dest_ctrl->GetValue(), 0);

	if (!dest.empty())
		dest_ctrl->SetValue(dest);
}

void DialogFontsCollector::OnRadio(wxCommandEvent &) {
	int value = collection_mode->GetSelection();
	wxString dst = dest_ctrl->GetValue();

	if (value == CheckFontsOnly || value == CopyToScriptFolder) {
		dest_ctrl->Enable(false);
		dest_browse_button->Enable(false);
		dest_label->Enable(false);
		dest_label->SetLabel(_("N/A"));
	}
	else {
		dest_ctrl->Enable(true);
		dest_browse_button->Enable(true);
		dest_label->Enable(true);

		if (value == CopyToFolder || value == SymlinkToFolder) {
			dest_label->SetLabel(_("Choose the folder where the fonts will be collected to. It will be created if it doesn't exist."));

			// Remove filename from browse box
			if (dst.Right(4) == ".zip")
				dest_ctrl->SetValue(wxFileName(dst).GetPath());
		}
		else {
			dest_label->SetLabel(_("Enter the name of the destination zip file to collect the fonts to. If a folder is entered, a default name will be used."));

			// Add filename to browse box
			if (!dst.EndsWith(".zip")) {
				wxFileName fn(dst + "//");
				fn.SetFullName("fonts.zip");
				dest_ctrl->SetValue(fn.GetFullPath());
			}
		}
	}
}

void DialogFontsCollector::OnAddText(wxThreadEvent &event) {
	std::pair<int, wxString> str = event.GetPayload<std::pair<int, wxString>>();
	collection_log->SetReadOnly(false);
	int pos = collection_log->GetReverseUnicodePosition(collection_log->GetLength());
	collection_log->AppendText(str.second);
	if (str.first) {
		collection_log->StartUnicodeStyling(pos, 31);
		collection_log->SetUnicodeStyling(pos, str.second.size(), str.first);
	}
	collection_log->GotoPos(pos);
	collection_log->SetReadOnly(true);
}

void DialogFontsCollector::OnCollectionComplete(wxThreadEvent &) {
	EnableCloseButton(true);
	start_btn->Enable();
	close_btn->Enable();
	collection_mode->Enable();
	if (config::path->Decode("?script") == "?script")
		collection_mode->Enable(2, false);

	wxCommandEvent evt;
	OnRadio(evt);
}
}

void ShowFontsCollectorDialog(agi::Context *c) {
	c->dialog->Show<DialogFontsCollector>(c);
}
