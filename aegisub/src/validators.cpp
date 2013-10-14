// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "config.h"

#include "validators.h"

#include "compat.h"
#include "utils.h"

#include <libaegisub/exception.h>
#include <libaegisub/util.h>

#include <wx/combobox.h>
#include <wx/textctrl.h>

namespace {
std::string new_value(wxTextCtrl *ctrl, int chr) {
	long from, to;
	ctrl->GetSelection(&from, &to);
	auto value = ctrl->GetValue();
	return from_wx(value.substr(0, from) + (wxChar)chr + value.substr(to));
}

wxChar decimal_separator() {
	auto sep = wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER);
	return sep.empty() ? '.' : sep[0];
}
}

IntValidator::IntValidator(std::string const& initial)
: allow_negative(false)
{
	agi::util::try_parse(initial, &value);
	Bind(wxEVT_CHAR, &IntValidator::OnChar, this);
}

IntValidator::IntValidator(int val, bool allow_negative)
: value(val)
, allow_negative(allow_negative)
{
	Bind(wxEVT_CHAR, &IntValidator::OnChar, this);
}

IntValidator::IntValidator(IntValidator const& rgt)
: allow_negative(rgt.allow_negative)
{
	SetWindow(rgt.GetWindow());
	Bind(wxEVT_CHAR, &IntValidator::OnChar, this);
}

bool IntValidator::TransferToWindow() {
	static_cast<wxTextCtrl *>(GetWindow())->SetValue(std::to_wstring(value));
	return true;
}

void IntValidator::OnChar(wxKeyEvent& event) {
	int chr = event.GetKeyCode();
	if (chr < WXK_SPACE || chr == WXK_DELETE || chr > WXK_START) {
		event.Skip();
		return;
	}

	auto ctrl = static_cast<wxTextCtrl *>(GetWindow());
	auto str = new_value(ctrl, chr);
	int parsed;
	if (allow_negative && str == '-')
		event.Skip();
	else if (agi::util::try_parse(str, &parsed) && (allow_negative || parsed >= 0))
		event.Skip();
	else if (!wxValidator::IsSilent())
		wxBell();
}

DoubleValidator::DoubleValidator(double *val, bool allow_negative)
: value(val)
, min(allow_negative ? std::numeric_limits<double>::lowest() : 0)
, max(std::numeric_limits<double>::max())
, decimal_sep(decimal_separator())
{
	Bind(wxEVT_CHAR, &DoubleValidator::OnChar, this);
}

DoubleValidator::DoubleValidator(double *val, double min, double max)
: value(val)
, min(min)
, max(max)
, decimal_sep(decimal_separator())
{
	Bind(wxEVT_CHAR, &DoubleValidator::OnChar, this);
}

DoubleValidator::DoubleValidator(DoubleValidator const& rgt)
: value(rgt.value)
, min(rgt.min)
, max(rgt.max)
, decimal_sep(rgt.decimal_sep)
{
	Bind(wxEVT_CHAR, &DoubleValidator::OnChar, this);
	SetWindow(rgt.GetWindow());
}

void DoubleValidator::OnChar(wxKeyEvent& event) {
	int chr = event.GetKeyCode();
	if (chr < WXK_SPACE || chr == WXK_DELETE || chr > WXK_START) {
		event.Skip();
		return;
	}

	if (chr == decimal_sep)
		chr = '.';

	auto str = new_value(static_cast<wxTextCtrl *>(GetWindow()), chr);
	if (decimal_sep != '.')
		replace(begin(str), end(str), (char)decimal_sep, '.');

	double parsed;
	bool can_parse = agi::util::try_parse(str, &parsed);
	if ((min < 0 && str == '-') || str == '.')
		event.Skip();
	else if (can_parse && parsed >= min && parsed <= max)
		event.Skip();
	else if (can_parse && min < 0 && chr == '-') // allow negating an existing value even if it results in being out of range
		event.Skip();
	else if (!wxValidator::IsSilent())
		wxBell();
}

bool DoubleValidator::TransferToWindow() {
	auto str = wxString::Format("%g", *value);
	if (decimal_sep != '.')
		std::replace(str.begin(), str.end(), wxS('.'), decimal_sep);
	if (str.find(decimal_sep) != str.npos) {
		while (str.Last() == '0')
			str.RemoveLast();
	}
	static_cast<wxTextCtrl *>(GetWindow())->SetValue(str);
	return true;
}

bool DoubleValidator::TransferFromWindow() {
	auto ctrl = static_cast<wxTextCtrl *>(GetWindow());
	if (!Validate(ctrl)) return false;
	auto str = from_wx(ctrl->GetValue());
	if (decimal_sep != '.')
		replace(begin(str), end(str), (char)decimal_sep, '.');
	agi::util::try_parse(str, value);
	return true;
}

bool StringBinder::TransferFromWindow() {
	wxWindow *window = GetWindow();
	if (wxTextCtrl *ctrl = dynamic_cast<wxTextCtrl*>(window))
		*value = from_wx(ctrl->GetValue());
	else if (wxComboBox *ctrl = dynamic_cast<wxComboBox*>(window))
		*value = from_wx(ctrl->GetValue());
	else
		throw agi::InternalError("Unsupported control type", 0);
	return true;
}

bool StringBinder::TransferToWindow() {
	wxWindow *window = GetWindow();
	if (wxTextCtrl *ctrl = dynamic_cast<wxTextCtrl*>(window))
		ctrl->SetValue(to_wx(*value));
	else if (wxComboBox *ctrl = dynamic_cast<wxComboBox*>(window))
		ctrl->SetValue(to_wx(*value));
	else
		throw agi::InternalError("Unsupported control type", 0);
	return true;
}
