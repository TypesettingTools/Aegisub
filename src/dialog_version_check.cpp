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

/// @file dialog_version_check.cpp
/// @brief Version Checker dialogue box and logic
/// @ingroup configuration_ui
///

#include "config.h"

#ifdef WITH_UPDATE_CHECKER

#include "dialog_version_check.h"

#ifdef _MSC_VER
#pragma warning(disable : 4250) // 'boost::asio::basic_socket_iostream<Protocol>' : inherits 'std::basic_ostream<_Elem,_Traits>::std::basic_ostream<_Elem,_Traits>::_Add_vtordisp2' via dominance
#endif

#include "compat.h"
#include "options.h"
#include "string_codec.h"
#include "version.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/exception.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/scoped_ptr.h>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/hyperlink.h>
#include <wx/intl.h>
#include <wx/platinfo.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>

#include <algorithm>
#include <ctime>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/format.hpp>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

// Allocate global lock mutex declared in header
std::mutex VersionCheckLock;

namespace {
struct AegisubUpdateDescription {
	std::string url;
	std::string friendly_name;
	std::string description;

	AegisubUpdateDescription(std::string url, std::string friendly_name, std::string description)
	: url(std::move(url)), friendly_name(std::move(friendly_name)), description(std::move(description)) { }
};

class VersionCheckerResultDialog final : public wxDialog {
	void OnCloseButton(wxCommandEvent &evt);
	void OnRemindMeLater(wxCommandEvent &evt);
	void OnClose(wxCloseEvent &evt);

	wxCheckBox *automatic_check_checkbox;

public:
	VersionCheckerResultDialog(wxString const& main_text, const std::vector<AegisubUpdateDescription> &updates);

	bool ShouldPreventAppExit() const override { return false; }
};

VersionCheckerResultDialog::VersionCheckerResultDialog(wxString const& main_text, const std::vector<AegisubUpdateDescription> &updates)
: wxDialog(nullptr, -1, _("Version Checker"))
{
	const int controls_width = 500;

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText *text = new wxStaticText(this, -1, main_text);
	text->Wrap(controls_width);
	main_sizer->Add(text, 0, wxBOTTOM|wxEXPAND, 6);

	for (auto const& update : updates) {
		main_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 6);

		text = new wxStaticText(this, -1, to_wx(update.friendly_name));
		wxFont boldfont = text->GetFont();
		boldfont.SetWeight(wxFONTWEIGHT_BOLD);
		text->SetFont(boldfont);
		main_sizer->Add(text, 0, wxEXPAND|wxBOTTOM, 6);

		wxTextCtrl *descbox = new wxTextCtrl(this, -1, to_wx(update.description), wxDefaultPosition, wxSize(controls_width,60), wxTE_MULTILINE|wxTE_READONLY);
		main_sizer->Add(descbox, 0, wxEXPAND|wxBOTTOM, 6);

		main_sizer->Add(new wxHyperlinkCtrl(this, -1, to_wx(update.url), to_wx(update.url)), 0, wxALIGN_LEFT|wxBOTTOM, 6);
	}

	automatic_check_checkbox = new wxCheckBox(this, -1, _("&Auto Check for Updates"));
	automatic_check_checkbox->SetValue(OPT_GET("App/Auto/Check For Updates")->GetBool());

	wxButton *remind_later_button = nullptr;
	if (updates.size() > 0)
		remind_later_button = new wxButton(this, wxID_NO, _("Remind me again in a &week"));

	wxButton *close_button = new wxButton(this, wxID_OK, _("&Close"));
	SetAffirmativeId(wxID_OK);
	SetEscapeId(wxID_OK);

	if (updates.size())
		main_sizer->Add(new wxStaticLine(this), 0, wxEXPAND|wxALL, 6);
	main_sizer->Add(automatic_check_checkbox, 0, wxEXPAND|wxBOTTOM, 6);

	auto button_sizer = new wxStdDialogButtonSizer();
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

	Bind(wxEVT_BUTTON, std::bind(&VersionCheckerResultDialog::Close, this, false), wxID_OK);
	Bind(wxEVT_BUTTON, &VersionCheckerResultDialog::OnRemindMeLater, this, wxID_NO);
	Bind(wxEVT_CLOSE_WINDOW, &VersionCheckerResultDialog::OnClose, this);
}

void VersionCheckerResultDialog::OnRemindMeLater(wxCommandEvent &) {
	// In one week
	time_t new_next_check_time = time(nullptr) + 7*24*60*60;
	OPT_SET("Version/Next Check")->SetInt(new_next_check_time);

	Close();
}

void VersionCheckerResultDialog::OnClose(wxCloseEvent &) {
	OPT_SET("App/Auto/Check For Updates")->SetBool(automatic_check_checkbox->GetValue());
	Destroy();
}

DEFINE_SIMPLE_EXCEPTION_NOINNER(VersionCheckError, agi::Exception, "versioncheck")

void PostErrorEvent(bool interactive, wxString const& error_text) {
	if (interactive) {
		agi::dispatch::Main().Async([=]{
			new VersionCheckerResultDialog(error_text, {});
		});
	}
}

static const char * GetOSShortName() {
	int osver_maj, osver_min;
	wxOperatingSystemId osid = wxGetOsVersion(&osver_maj, &osver_min);

	if (osid & wxOS_WINDOWS_NT) {
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
	// CF returns 0x10 for some reason, which wx has recently started
	// turning into 10
	else if (osid & wxOS_MAC_OSX_DARWIN && (osver_maj == 0x10 || osver_maj == 10)) {
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
static wxString GetUILanguage() {
	agi::scoped_holder<HMODULE, BOOL (__stdcall *)(HMODULE)> kernel32(LoadLibraryW(L"kernel32.dll"), FreeLibrary);
	if (!kernel32) return "";

	PGetUserPreferredUILanguages gupuil = (PGetUserPreferredUILanguages)GetProcAddress(kernel32, "GetUserPreferredUILanguages");
	if (!gupuil) return "";

	ULONG numlang = 0, output_len = 0;
	if (gupuil(MUI_LANGUAGE_NAME, &numlang, 0, &output_len) != TRUE || !output_len)
		return "";

	std::vector<wchar_t> output(output_len);
	if (!gupuil(MUI_LANGUAGE_NAME, &numlang, &output[0], &output_len) || numlang < 1)
		return "";

	// We got at least one language, just treat it as the only, and a null-terminated string
	return &output[0];
}

static wxString GetSystemLanguage() {
	wxString res = GetUILanguage();
	if (!res)
		// On an old version of Windows, let's just return the LANGID as a string
		res = wxString::Format("x-win%04x", GetUserDefaultUILanguage());

	return res;
}
#elif __APPLE__
static wxString GetSystemLanguage() {
	CFLocaleRef locale = CFLocaleCopyCurrent();
	CFStringRef localeName = (CFStringRef)CFLocaleGetValue(locale, kCFLocaleIdentifier);

	char buf[128] = { 0 };
	CFStringGetCString(localeName, buf, sizeof buf, kCFStringEncodingUTF8);
	CFRelease(locale);

	return wxString::FromUTF8(buf);

}
#else
static wxString GetSystemLanguage() {
	return wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage())->CanonicalName;
}
#endif

static wxString GetAegisubLanguage() {
	return to_wx(OPT_GET("App/Language")->GetString());
}

void DoCheck(bool interactive) {
	boost::asio::ip::tcp::iostream stream;
	stream.connect(UPDATE_CHECKER_SERVER, "http");
	if (!stream)
		throw VersionCheckError(from_wx(_("Could not connect to updates server.")));

	stream << boost::format(
		"GET %s?rev=%d&rel=%d&os=%s&lang=%s&aegilang=%s HTTP/1.0\r\n"
		"User-Agent: Aegisub %s\r\n"
		"Host: %s\r\n"
		"Accept: */*\r\n"
		"Connection: close\r\n\r\n")
		% UPDATE_CHECKER_BASE_URL
		% GetSVNRevision()
		% (GetIsOfficialRelease() ? 1 : 0)
		% GetOSShortName()
		% GetSystemLanguage()
		% GetAegisubLanguage()
		% GetAegisubLongVersionString()
		% UPDATE_CHECKER_SERVER
		;

	std::string http_version;
	stream >> http_version;
	int status_code;
	stream >> status_code;
	if (!stream || http_version.substr(0, 5) != "HTTP/")
		throw VersionCheckError(from_wx(_("Could not download from updates server.")));
	if (status_code != 200)
		throw VersionCheckError(from_wx(wxString::Format(_("HTTP request failed, got HTTP response %d."), status_code)));

	stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Skip the headers since we don't care about them
	for (auto const& header : agi::line_iterator<std::string>(stream))
		if (header.empty()) break;

	std::vector<AegisubUpdateDescription> results;
	for (auto const& line : agi::line_iterator<std::string>(stream)) {
		if (line.empty()) continue;

		std::vector<std::string> parsed;
		boost::split(parsed, line, boost::is_any_of("|"));
		if (parsed.size() != 6) continue;

		// 0 and 2 being things that never got used
		std::string revision = parsed[1];
		std::string url = inline_string_decode(parsed[3]);
		std::string friendlyname = inline_string_decode(parsed[4]);
		std::string description = inline_string_decode(parsed[5]);

		if (atoi(revision.c_str()) <= GetSVNRevision())
			continue;

		results.emplace_back(url, friendlyname, description);
	}

	if (!results.empty() || interactive) {
		agi::dispatch::Main().Async([=]{
			wxString text;
			if (results.size() == 1)
				text = _("An update to Aegisub was found.");
			else if (results.size() > 1)
				text = _("Several possible updates to Aegisub were found.");
			else
				text = _("There are no updates to Aegisub.");

			new VersionCheckerResultDialog(text, results);
		});
	}
}

}

void PerformVersionCheck(bool interactive) {
	agi::dispatch::Background().Async([=]{
		if (!interactive) {
			// Automatic checking enabled?
			if (!OPT_GET("App/Auto/Check For Updates")->GetBool())
				return;

			// Is it actually time for a check?
			time_t next_check = OPT_GET("Version/Next Check")->GetInt();
			if (next_check > time(nullptr))
				return;
		}

		if (!VersionCheckLock.try_lock()) return;

		try {
			DoCheck(interactive);
		}
		catch (const agi::Exception &e) {
			PostErrorEvent(interactive, wxString::Format(
				_("There was an error checking for updates to Aegisub:\n%s\n\nIf other applications can access the Internet fine, this is probably a temporary server problem on our end."),
				e.GetMessage()));
		}
		catch (...) {
			PostErrorEvent(interactive, _("An unknown error occurred while checking for updates to Aegisub."));
		}

		VersionCheckLock.unlock();

		agi::dispatch::Main().Async([]{
			time_t new_next_check_time = time(nullptr) + 60*60; // in one hour
			OPT_SET("Version/Next Check")->SetInt(new_next_check_time);
		});
	});
}

#endif
