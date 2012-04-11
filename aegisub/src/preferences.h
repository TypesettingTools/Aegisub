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

/// @file preferences.h
/// @brief Preferences dialogue
/// @see preferences.cpp
/// @ingroup configuration_ui

#ifndef AGI_PRE
#include <deque>
#include <tr1/functional>
#include <map>

#include <wx/dialog.h>
#endif

#include <libaegisub/exception.h>

class wxButton;
class wxTreebook;
namespace agi { class OptionValue; }

DEFINE_BASE_EXCEPTION_NOINNER(PreferencesError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceIncorrectType, PreferencesError, "preferences/incorrect_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceNotSupported, PreferencesError, "preferences/not_supported")

class Preferences : public wxDialog {
public:
	typedef std::tr1::function<void ()> Thunk;
private:
	wxTreebook *book;
	wxButton *applyButton;

	std::map<std::string, agi::OptionValue*> pending_changes;
	std::deque<Thunk> pending_callbacks;
	std::deque<std::string> option_names;

	void OnOK(wxCommandEvent &);
	void OnCancel(wxCommandEvent &);
	void OnApply(wxCommandEvent &);
	void OnResetDefault(wxCommandEvent&);

public:
	Preferences(wxWindow *parent);
	~Preferences();

	/// Add an option to be set when the OK or Apply button is clicked
	/// @param new_value Clone of the option with the new value to copy over
	void SetOption(agi::OptionValue *new_value);

	/// All a function to call when the OK or Apply button is clicked
	/// @param callback Function to call
	void AddPendingChange(Thunk const& callback);

	/// Add an option which can be changed via the dialog
	/// @param name Name of the option
	///
	/// This is used for resetting options to the defaults. We don't want to
	/// simply revert to the default config file as a bunch of things other than
	/// user options are stored in it. Perhaps that should change in the future.
	void AddChangeableOption(std::string const& name);
};
