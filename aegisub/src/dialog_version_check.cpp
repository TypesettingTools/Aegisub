// Copyright (c) 2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_version_check.cpp
/// @brief Version Checker dialogue box and logic
/// @ingroup configuration_ui
///


#include "config.h"

#include "dialog_version_check.h"

#ifndef AGI_PRE
#ifdef WIN32
// Congratulation wx, you forgot to include a header somewhere
#include <winsock2.h>
#endif

#include <wx/app.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/hyperlink.h>
#include <wx/intl.h>
#include <wx/platinfo.h>
#include <wx/protocol/http.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>

#include <algorithm>
#include <memory>
#include <set>
#include <tr1/functional>
#include <vector>
#endif

#include "compat.h"
#include "main.h"
#include "string_codec.h"
#include "version.h"

#include <libaegisub/exception.h>

/* *** Public API is implemented here *** */

// Allocate global lock mutex declared in header
wxMutex VersionCheckLock;

class AegisubVersionCheckerThread : public wxThread {
	bool interactive;
	void DoCheck();
	void PostErrorEvent(const wxString &error_text);
	ExitCode Entry();
public:
	AegisubVersionCheckerThread(bool interactive);
};

// Public API for version checker
void PerformVersionCheck(bool interactive)
{
	new AegisubVersionCheckerThread(interactive);
}

/* *** The actual implementation begins here *** */

struct AegisubUpdateDescription {
	wxString url;
	wxString friendly_name;
	wxString description;
};

class AegisubVersionCheckResultEvent : public wxEvent {
	wxString main_text;
	std::vector<AegisubUpdateDescription> updates;

public:
	AegisubVersionCheckResultEvent(wxString message = wxString());
	

	wxEvent *Clone() const
	{
		return new AegisubVersionCheckResultEvent(*this);
	}

	const wxString & GetMainText() const { return main_text; }

	// If there are no updates in the list, either none were found or an error occurred,
	// either way it means "failure" if it's empty
	const std::vector<AegisubUpdateDescription> & GetUpdates() const { return updates; }
	void AddUpdate(const wxString &url, const wxString &friendly_name, const wxString &description)
	{
		updates.push_back(AegisubUpdateDescription());
		AegisubUpdateDescription &desc = updates.back();
		desc.url = url;
		desc.friendly_name = friendly_name;
		desc.description = description;
	}
};

wxDEFINE_EVENT(AEGISUB_EVENT_VERSIONCHECK_RESULT, AegisubVersionCheckResultEvent);

AegisubVersionCheckResultEvent::AegisubVersionCheckResultEvent(wxString message)
: wxEvent(AEGISUB_EVENT_VERSIONCHECK_RESULT)
, main_text(message)
{
}


DEFINE_SIMPLE_EXCEPTION_NOINNER(VersionCheckError, agi::Exception, "versioncheck")

static void register_event_handler();

AegisubVersionCheckerThread::AegisubVersionCheckerThread(bool interactive)
: wxThread(wxTHREAD_DETACHED)
, interactive(interactive)
{
	register_event_handler();

	if (!wxSocketBase::IsInitialized())
		wxSocketBase::Initialize();

	Create();
	Run();
}

wxThread::ExitCode AegisubVersionCheckerThread::Entry()
{
	if (!interactive)
	{
		// Automatic checking enabled?
		if (!OPT_GET("App/Auto/Check For Updates")->GetBool())
			return 0;

		// Is it actually time for a check?
		time_t next_check = OPT_GET("Version/Next Check")->GetInt();
		if (next_check > wxDateTime::GetTimeNow())
			return 0;
	}

	if (VersionCheckLock.TryLock() != wxMUTEX_NO_ERROR) return 0;

	try {
		DoCheck();
	}
	catch (const agi::Exception &e) {
		PostErrorEvent(wxString::Format(
			_("There was an error checking for updates to Aegisub:\n%s\n\nIf other applications can access the Internet fine, this is probably a temporary server problem on our end."),
			e.GetMessage()));
	}
	catch (...) {
		PostErrorEvent(_("An unknown error occurred while checking for updates to Aegisub."));
	}

	VersionCheckLock.Unlock();

	// While Options isn't perfectly thread safe, this should still be okay.
	// Traversing the std::map to find the key-value pair doesn't modify any data as long as
	// the key already exists (which it does at this point), and modifying the value only
	// touches that specific key-value pair and will never cause a rebalancing of the tree,
	// because the tree only depends on the keys.
	// Lastly, writing options to disk only happens when Options.Save() is called.
	time_t new_next_check_time = wxDateTime::GetTimeNow() + 60*60; // in one hour
	OPT_SET("Version/Next Check")->SetInt((int)new_next_check_time);

	return 0;
}

void AegisubVersionCheckerThread::PostErrorEvent(const wxString &error_text)
{
	if (interactive)
		wxTheApp->AddPendingEvent(AegisubVersionCheckResultEvent(error_text));
}


static const char * GetOSShortName()
{
	int osver_maj, osver_min;
	wxOperatingSystemId osid = wxGetOsVersion(&osver_maj, &osver_min);

	if (osid & wxOS_WINDOWS_NT)
	{
		if (osver_maj == 5 && osver_min == 0)
			return "win2k";
		else if (osver_maj == 5 && osver_min == 1)
			return "winxp";
		else if (osver_maj == 5 && osver_min == 2)
			return "win2k3"; // this is also xp64
		else if (osver_maj == 6 && osver_min == 0)
			return "win60"; // vista and server 2008
		else if (osver_maj == 6 && osver_min == 1)
			return "win61"; // 7 and server 2008r2
		else if (osver_maj == 6 && osver_min == 2)
			return "win62"; // 8
		else
			return "windows"; // future proofing? I doubt we run on nt4
	}
	else if (osid & wxOS_MAC_OSX_DARWIN && osver_maj == 0x10) // yes 0x10, not decimal 10, don't ask me
	{
		// ugliest hack in the world? nah.
		static char osxstring[] = "osx00";
		char minor = osver_min >> 4;
		char patch = osver_min & 0x0F;
		osxstring[3] = minor + ((minor<=9) ? '0' : ('a'-1));
		osxstring[4] = patch + ((patch<=9) ? '0' : ('a'-1));
		return osxstring;
	}
	else if (osid & wxOS_UNIX_LINUX)
		return "linux";
	else if (osid & wxOS_UNIX_FREEBSD)
		return "freebsd";
	else if (osid & wxOS_UNIX_OPENBSD)
		return "openbsd";
	else if (osid & wxOS_UNIX_NETBSD)
		return "netbsd";
	else if (osid & wxOS_UNIX_SOLARIS)
		return "solaris";
	else if (osid & wxOS_UNIX_AIX)
		return "aix";
	else if (osid & wxOS_UNIX_HPUX)
		return "hpux";
	else if (osid & wxOS_UNIX)
		return "unix";
	else if (osid & wxOS_OS2)
		return "os2";
	else if (osid & wxOS_DOS)
		return "dos";
	else
		return "unknown";
}


#ifdef WIN32
typedef BOOL (WINAPI * PGetUserPreferredUILanguages)(DWORD dwFlags, PULONG pulNumLanguages, wchar_t *pwszLanguagesBuffer, PULONG pcchLanguagesBuffer);

// Try using Win 6+ functions if available
static wxString GetUILanguage()
{
	wxString res;

	HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
	if (!kernel32) return res;

	PGetUserPreferredUILanguages gupuil = (PGetUserPreferredUILanguages)GetProcAddress(kernel32, "GetUserPreferredUILanguages");
	if (!gupuil) goto error;

	ULONG numlang = 0, output_len = 0;
	if (gupuil(MUI_LANGUAGE_NAME, &numlang, 0, &output_len) == TRUE && output_len)
	{
		std::vector<wchar_t> output(output_len);
		if (gupuil(MUI_LANGUAGE_NAME, &numlang, &output[0], &output_len) && numlang >= 1)
		{
			// We got at least one language, just treat it as the only, and a null-terminated string
			res = &output[0];
		}
	}

error:
	FreeModule(kernel32);
	return res;
}
static wxString GetSystemLanguage()
{
	wxString res = GetUILanguage();
	if (!res)
	{
		// On an old version of Windows, let's just return the LANGID as a string
		res = wxString::Format("x-win%04x", GetUserDefaultUILanguage());
	}

	return res;
}
#else
static wxString GetSystemLanguage()
{
	return wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage())->CanonicalName;
}
#endif

template<class OutIter>
static void split_str(wxString const& str, wxString const& sep, bool empty, OutIter out)
{
	wxStringTokenizer tk(str, sep, empty ? wxTOKEN_DEFAULT : wxTOKEN_RET_EMPTY_ALL);
	while (tk.HasMoreTokens())
	{
		*out++ = tk.GetNextToken();
	}
}


void AegisubVersionCheckerThread::DoCheck()
{
	std::set<wxString> accept_tags;
#ifdef UPDATE_CHECKER_ACCEPT_TAGS
	split_str(wxString(UPDATE_CHECKER_ACCEPT_TAGS, wxConvUTF8), " ", false,
		inserter(accept_tags, accept_tags.end()));
#endif

	const wxString servername = "updates.aegisub.org";
	const wxString base_updates_path = "/trunk";

	wxString querystring = wxString::Format(
		"?rev=%d&rel=%d&os=%s&lang=%s",
		GetSVNRevision(),
		GetIsOfficialRelease()?1:0,
		GetOSShortName(),
		GetSystemLanguage());

	wxString path = base_updates_path + querystring;

	wxHTTP http;
	http.SetHeader("User-Agent", wxString("Aegisub ") + GetAegisubLongVersionString());
	http.SetHeader("Connection", "Close");
	http.SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);

	if (!http.Connect(servername))
		throw VersionCheckError(STD_STR(_("Could not connect to updates server.")));

	agi::scoped_ptr<wxInputStream> stream(http.GetInputStream(path));
	if (!stream) // check for null-pointer
		throw VersionCheckError(STD_STR(_("Could not download from updates server.")));

	if (http.GetResponse() < 200 || http.GetResponse() >= 300) {
		throw VersionCheckError(STD_STR(wxString::Format(_("HTTP request failed, got HTTP response %d."), http.GetResponse())));
	}

	wxTextInputStream text(*stream);

	AegisubVersionCheckResultEvent result_event;

	while (!stream->Eof() && stream->GetSize() > 0)
	{
		wxArrayString parsed;
		split_str(text.ReadLine(), "|", true, std::back_inserter(parsed));
		if (parsed.size() != 6) continue;

		wxString line_type = parsed[0];
		wxString line_revision = parsed[1];
		wxString line_tags_str = parsed[2];
		wxString line_url = inline_string_decode(parsed[3]);
		wxString line_friendlyname = inline_string_decode(parsed[4]);
		wxString line_description = inline_string_decode(parsed[5]);

		// stable runners don't want unstable, not interesting, skip
		if ((line_type == "branch" || line_type == "dev") && GetIsOfficialRelease())
			continue;

		// check if the tags match
		if (line_tags_str.empty() || line_tags_str == "all")
		{
			// looking good
		}
		else
		{
			std::set<wxString> tags;
			split_str(line_tags_str, " ", false, inserter(tags, tags.end()));
			if (!includes(accept_tags.begin(), accept_tags.end(), tags.begin(), tags.end()))
				continue;
		}

		if (line_type == "upgrade" || line_type == "bugfix")
		{
			// de facto interesting
		}
		else
		{
			// maybe interesting, check revision
			
			long new_revision = 0;
			if (!line_revision.ToLong(&new_revision)) continue;
			if (new_revision <= GetSVNRevision()) 
			{
				// too old, not interesting, skip
				continue;
			}
		}

		// it's interesting!
		result_event.AddUpdate(line_url, line_friendlyname, line_description);
	}

	if (result_event.GetUpdates().size() > 0 || interactive)
	{
		wxTheApp->AddPendingEvent(result_event);
	}
}

class VersionCheckerResultDialog : public wxDialog {
	void OnCloseButton(wxCommandEvent &evt);
	void OnRemindMeLater(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);

	wxCheckBox *automatic_check_checkbox;

public:
	VersionCheckerResultDialog(const wxString &main_text, const std::vector<AegisubUpdateDescription> &updates);

	bool ShouldPreventAppExit() const { return false; }
};

VersionCheckerResultDialog::VersionCheckerResultDialog(const wxString &main_text, const std::vector<AegisubUpdateDescription> &updates)
: wxDialog(0, -1, _("Version Checker"))
{
	const int controls_width = 500;

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText *text = new wxStaticText(this, -1, main_text);
	text->Wrap(controls_width);
	main_sizer->Add(text, 0, wxBOTTOM|wxEXPAND, 6);

	std::vector<AegisubUpdateDescription>::const_iterator upd_iterator = updates.begin();
	for (; upd_iterator != updates.end(); ++upd_iterator)
	{
		main_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 6);

		text = new wxStaticText(this, -1, upd_iterator->friendly_name);
		wxFont boldfont = text->GetFont();
		boldfont.SetWeight(wxFONTWEIGHT_BOLD);
		text->SetFont(boldfont);
		main_sizer->Add(text, 0, wxEXPAND|wxBOTTOM, 6);

		wxTextCtrl *descbox = new wxTextCtrl(this, -1, upd_iterator->description, wxDefaultPosition, wxSize(controls_width,60), wxTE_MULTILINE|wxTE_READONLY);
		main_sizer->Add(descbox, 0, wxEXPAND|wxBOTTOM, 6);

		main_sizer->Add(new wxHyperlinkCtrl(this, -1, upd_iterator->url, upd_iterator->url), 0, wxALIGN_LEFT|wxBOTTOM, 6);
	}

	automatic_check_checkbox = new wxCheckBox(this, -1, _("&Auto Check for Updates"));
	automatic_check_checkbox->SetValue(OPT_GET("App/Auto/Check For Updates")->GetBool());

	wxButton *remind_later_button = 0;
	if (updates.size() > 0)
		remind_later_button = new wxButton(this, wxID_NO, _("Remind me again in a &week"));

	wxButton *close_button = new wxButton(this, wxID_OK, _("&Close"));
	SetAffirmativeId(wxID_OK);
	SetEscapeId(wxID_OK);

	if (updates.size())
		main_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 6);
	main_sizer->Add(automatic_check_checkbox, 0, wxEXPAND|wxBOTTOM, 6);

	wxStdDialogButtonSizer *button_sizer = new wxStdDialogButtonSizer();
	button_sizer->AddButton(close_button);
	if (remind_later_button)
		button_sizer->AddButton(remind_later_button);
	button_sizer->Realize();
	main_sizer->Add(button_sizer, 0, wxEXPAND, 0);

	wxSizer *outer_sizer = new wxBoxSizer(wxVERTICAL);
	outer_sizer->Add(main_sizer, 0, wxALL|wxEXPAND, 12);

	SetSizerAndFit(outer_sizer);
	Centre();
	Show();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&VersionCheckerResultDialog::Close, this, false), wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &VersionCheckerResultDialog::OnRemindMeLater, this, wxID_NO);
	Bind(wxEVT_CLOSE_WINDOW, &VersionCheckerResultDialog::OnClose, this);
}
void VersionCheckerResultDialog::OnRemindMeLater(wxCommandEvent &)
{
	// In one week
	time_t new_next_check_time = wxDateTime::Today().GetTicks() + 7*24*60*60;
	OPT_SET("Version/Next Check")->SetInt((int)new_next_check_time);

	Close();
}

void VersionCheckerResultDialog::OnClose(wxCloseEvent &)
{
	OPT_SET("App/Auto/Check For Updates")->SetBool(automatic_check_checkbox->GetValue());
	Destroy();
}

static void on_update_result(AegisubVersionCheckResultEvent &evt)
{
	wxString text = evt.GetMainText();
	if (!text)
	{
		if (evt.GetUpdates().size() == 1)
		{
			text = _("An update to Aegisub was found.");
		}
		else if (evt.GetUpdates().size() > 1)
		{
			text = _("Several possible updates to Aegisub were found.");
		}
		else
		{
			text = _("There are no updates to Aegisub.");
		}
	}

	new VersionCheckerResultDialog(text, evt.GetUpdates());
}

static void register_event_handler()
{
	static bool is_registered = false;
	if (is_registered) return;

	wxTheApp->Bind(AEGISUB_EVENT_VERSIONCHECK_RESULT, on_update_result);
	is_registered = true;
}
