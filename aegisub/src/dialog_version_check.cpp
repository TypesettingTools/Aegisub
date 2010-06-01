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

#ifndef AGI_PRE
#ifdef WIN32
// Congratulation wx, you forgot to include a header somewhere
#include <winsock2.h>
#endif

#include <wx/string.h>
#include <wx/event.h>
#include <wx/thread.h>
#include <wx/protocol/http.h>
#include <wx/txtstrm.h>
#include <wx/platinfo.h>
#include <wx/tokenzr.h>
#include <wx/hyperlink.h>
#include <wx/intl.h>
#include <wx/checkbox.h>
#include <wx/app.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <memory>
#include <vector>
#endif

#include "compat.h"
#include "dialog_version_check.h"
#include "main.h"
#include "options.h"
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
public:
	AegisubVersionCheckerThread(bool interactive);
	virtual ExitCode Entry();
};

// Public API for version checker
void PerformVersionCheck(bool interactive)
{
	new AegisubVersionCheckerThread(interactive);
}



/* *** The actual implementation begins here *** */



// A new event class for reporting the result of checking

DEFINE_EVENT_TYPE(AEGISUB_EVENT_VERSIONCHECK_RESULT)

struct AegisubUpdateDescription {
	wxString url;
	wxString friendly_name;
	wxString description;
};

class AegisubVersionCheckResultEvent : public wxEvent {
	wxString main_text;
	std::vector<AegisubUpdateDescription> updates;

public:
	AegisubVersionCheckResultEvent()
		: wxEvent(0, AEGISUB_EVENT_VERSIONCHECK_RESULT)
	{ }

	virtual wxEvent *Clone() const
	{
		return new AegisubVersionCheckResultEvent(*this);
	}

	const wxString & GetMainText() const { return main_text; }
	void SetMainText(const wxString &new_text) { main_text = new_text; }

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

DEFINE_SIMPLE_EXCEPTION_NOINNER(VersionCheckError, agi::Exception, "versioncheck")


class AegisubVersionCheckEventHandler : public wxEvtHandler {
public:
	void OnUpdateResult(AegisubVersionCheckResultEvent &evt);

	static void EnsureHandlerIsRegistered();
};



AegisubVersionCheckerThread::AegisubVersionCheckerThread(bool interactive)
: wxThread(wxTHREAD_DETACHED)
, interactive(interactive)
{
	AegisubVersionCheckEventHandler::EnsureHandlerIsRegistered();

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
		if (!OPT_GET("App/Auto/Check For Updates")->GetInt())
			return 0;

		// Is it actually time for a check?
		time_t next_check = OPT_GET("Version/Next Check")->GetInt();
		if ((time_t)next_check > wxDateTime::GetTimeNow())
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
	if (!interactive) return;

	AegisubVersionCheckResultEvent evt;
	evt.SetMainText(error_text);
	wxTheApp->AddPendingEvent(evt);
}


static const wxChar * GetOSShortName()
{
	int osver_maj, osver_min;
	wxOperatingSystemId osid = wxGetOsVersion(&osver_maj, &osver_min);

	if (osid & wxOS_WINDOWS_NT)
	{
		if (osver_maj == 5 && osver_min == 0)
			return _T("win2k");
		else if (osver_maj == 5 && osver_min == 1)
			return _T("winxp");
		else if (osver_maj == 5 && osver_min == 2)
			return _T("win2k3"); // this is also xp64
		else if (osver_maj == 6 && osver_min == 0)
			return _T("win60"); // vista and server 2008
		else if (osver_maj == 6 && osver_min == 1)
			return _T("win61"); // 7 and server 2008r2
		else
			return _T("windows"); // future proofing? I doubt we run on nt4
	}
	else if (osid & wxOS_MAC_OSX_DARWIN && osver_maj == 0x10) // yes 0x10, not decimal 10, don't ask me
	{
		// ugliest hack in the world? nah.
		static wxChar osxstring[] = _T("osx00");
		char minor = osver_min >> 4;
		char patch = osver_min & 0x0F;
		osxstring[3] = minor + ((minor<=9) ? '0' : ('a'-1));
		osxstring[4] = patch + ((patch<=9) ? '0' : ('a'-1));
		return osxstring;
	}
	else if (osid & wxOS_UNIX_LINUX)
		return _T("linux");
	else if (osid & wxOS_UNIX_FREEBSD)
		return _T("freebsd");
	else if (osid & wxOS_UNIX_OPENBSD)
		return _T("openbsd");
	else if (osid & wxOS_UNIX_NETBSD)
		return _T("netbsd");
	else if (osid & wxOS_UNIX_SOLARIS)
		return _T("solaris");
	else if (osid & wxOS_UNIX_AIX)
		return _T("aix");
	else if (osid & wxOS_UNIX_HPUX)
		return _T("hpux");
	else if (osid & wxOS_UNIX)
		return _T("unix");
	else if (osid & wxOS_OS2)
		return _T("os2");
	else if (osid & wxOS_DOS)
		return _T("dos");
	else
		return _T("unknown");
}


#ifdef WIN32
typedef BOOL (WINAPI * PGetUserPreferredUILanguages)(DWORD dwFlags, PULONG pulNumLanguages, wchar_t *pwszLanguagesBuffer, PULONG pcchLanguagesBuffer);
static wxString GetSystemLanguage()
{
	wxString res;

	// Try using Win 6+ functions if available
	HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
	PGetUserPreferredUILanguages gupuil = (PGetUserPreferredUILanguages)GetProcAddress(kernel32, "GetUserPreferredUILanguages");
	if (gupuil)
	{
		ULONG numlang = 0, output_len = 0;
		if (gupuil(MUI_LANGUAGE_NAME, &numlang, 0, &output_len) != TRUE)
			goto getsyslang_fallback;
		wchar_t *output = new wchar_t[output_len];
		if (gupuil(MUI_LANGUAGE_NAME, &numlang, output, &output_len) && numlang >= 1)
		{
			// We got at least one language, just treat it as the only, and a null-terminated string
			res = wxString(output);
			delete[] output;
		}
		else
		{
			delete[] output;
			goto getsyslang_fallback;
		}
	}
	else
	{
getsyslang_fallback:
		// On an old version of Windows, let's just return the LANGID as a string
		LANGID langid = GetUserDefaultUILanguage();
		res = wxString::Format(_T("x-win%04x"), langid);

	}
	FreeModule(kernel32);

	return res;
}
#else
static wxString GetSystemLanguage()
{
	return wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage())->CanonicalName;
}
#endif


void AegisubVersionCheckerThread::DoCheck()
{
	const wxString servername = _T("updates.aegisub.org");
	const wxString base_updates_path = _T("/trunk");

	wxString querystring = wxString::Format(
		_T("?rev=%d&rel=%d&os=%s&lang=%s"),
		GetSVNRevision(),
		GetIsOfficialRelease()?1:0,
		GetOSShortName(),
		GetSystemLanguage().c_str());

	wxString path = base_updates_path + querystring;

	wxHTTP http;
	http.SetHeader(_T("Connection"), _T("Close"));
	http.SetFlags(wxSOCKET_WAITALL|wxSOCKET_BLOCK);

	if (!http.Connect(servername))
		throw VersionCheckError(STD_STR(_("Could not connect to updates server.")));

	std::auto_ptr<wxInputStream> stream(http.GetInputStream(path));
	if (stream.get() == 0) // check for null-pointer
		throw VersionCheckError(_("Could not connect to updates server."));

	if (http.GetResponse() < 200 || http.GetResponse() >= 300) {
		const std::string str_err = STD_STR(wxString::Format(_("HTTP request failed, got HTTP response %d."), http.GetResponse()));
		throw VersionCheckError(str_err);
	}

	wxTextInputStream text(*stream);

	AegisubVersionCheckResultEvent result_event;

	while (!stream->Eof() && stream->GetSize() > 0)
	{
		wxString line = text.ReadLine();
		wxStringTokenizer tkn(line, _T("|"), wxTOKEN_RET_EMPTY_ALL);
		wxArrayString parsed;
		while (tkn.HasMoreTokens()) {
			parsed.Add(tkn.GetNextToken());
		}
		if (parsed.Count() != 6) continue;

		wxString line_type = parsed[0];
		wxString line_revision = parsed[1];
		wxString line_platform = parsed[2];
		wxString line_url = inline_string_decode(parsed[3]);
		wxString line_friendlyname = inline_string_decode(parsed[4]);
		wxString line_description = inline_string_decode(parsed[5]);

		if ((line_type == _T("branch") || line_type == _T("dev")) && GetIsOfficialRelease())
		{
			// stable runners don't want unstable, not interesting, skip
			continue;
		}

		// check if the OS is right
		wxOperatingSystemId osid = wxGetOsVersion();
		if (line_platform == _T("windows") && !(osid & wxOS_WINDOWS_NT))
		{
			continue;
		}
		if (line_platform == _T("mac") && !(osid & wxOS_MAC_OSX_DARWIN))
		{
			continue;
		}
		if (line_platform == _T("source") && (osid & wxOS_WINDOWS_NT || osid & wxOS_MAC_OSX_DARWIN))
		{
			// TODO: support interested-in-source-releases flag
			continue;
		}
		if (!(line_platform  == _T("windows") || line_platform == _T("mac") || line_platform == _T("source") || line_platform == _T("all")))
		{
			continue;
		}

		if (line_type == _T("upgrade") || line_type == _T("bugfix"))
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

	if (result_event.GetUpdates().size() == 1)
	{
		result_event.SetMainText(_("An update to Aegisub was found."));
	}
	else if (result_event.GetUpdates().size() > 1)
	{
		result_event.SetMainText(_("Several possible updates to Aegisub were found."));
	}
	else
	{
		result_event.SetMainText(_("There are no updates to Aegisub."));
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

	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(VersionCheckerResultDialog, wxDialog)
	EVT_BUTTON(wxID_OK, VersionCheckerResultDialog::OnCloseButton)
	EVT_BUTTON(wxID_NO, VersionCheckerResultDialog::OnRemindMeLater)
	EVT_CLOSE(VersionCheckerResultDialog::OnClose)
END_EVENT_TABLE()


VersionCheckerResultDialog::VersionCheckerResultDialog(const wxString &main_text, const std::vector<AegisubUpdateDescription> &updates)
: wxDialog(0, -1, _("Version Checker"))
{
	const int controls_width = 350;

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

	automatic_check_checkbox = new wxCheckBox(this, -1, _("Auto Check for Updates"));
	automatic_check_checkbox->SetValue(!!OPT_GET("App/Auto/Check For Updates")->GetInt());

	wxButton *remind_later_button = 0;
	if (updates.size() > 0)
		remind_later_button = new wxButton(this, wxID_NO, _("Remind me again in a &week"));

	wxButton *close_button = new wxButton(this, wxID_OK, _("&Close"));
	SetAffirmativeId(wxID_OK);
	SetEscapeId(wxID_OK);

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
}


void VersionCheckerResultDialog::OnCloseButton(wxCommandEvent &evt)
{
	Close();
}


void VersionCheckerResultDialog::OnRemindMeLater(wxCommandEvent &evt)
{
	// In one week
	time_t new_next_check_time = wxDateTime::Today().GetTicks() + 7*24*60*60;
	OPT_SET("Version/Next Check")->SetInt((int)new_next_check_time);

	Close();
}


void VersionCheckerResultDialog::OnClose(wxCloseEvent &evt)
{
	OPT_SET("App/Auto/Check For Updates")->SetBool(automatic_check_checkbox->GetValue());
	Destroy();
}



void AegisubVersionCheckEventHandler::OnUpdateResult(AegisubVersionCheckResultEvent &evt)
{
	VersionCheckerResultDialog *dlg = new VersionCheckerResultDialog(evt.GetMainText(), evt.GetUpdates());
	dlg->Show();
}

void AegisubVersionCheckEventHandler::EnsureHandlerIsRegistered()
{
	static bool is_registered = false;
	static AegisubVersionCheckEventHandler evt_handler_object;

	if (is_registered) return;

	wxTheApp->Connect(
		-1, -1,
		AEGISUB_EVENT_VERSIONCHECK_RESULT,
		(wxObjectEventFunction)&AegisubVersionCheckEventHandler::OnUpdateResult,
		0,
		&evt_handler_object);

	is_registered = true;
}

