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

#include <libaegisub/exception.h>

#include <string>

#include <wx/radiobox.h>
#include <wx/validate.h>

class IntValidator final : public wxValidator {
	int value;
	bool allow_negative;

	bool CheckCharacter(int chr, bool is_first) const;
	void OnChar(wxKeyEvent& event);

	bool Validate(wxWindow *) override { return true; }
	wxObject* Clone() const override { return new IntValidator(*this); }
	bool TransferToWindow() override;
	bool TransferFromWindow() override { return true; }

	IntValidator(IntValidator const& rgt);

public:
	explicit IntValidator(int val=0, bool allow_negative=false);
};

class DoubleValidator final : public wxValidator {
	double *value;
	double min;
	double max;
	wxChar decimal_sep;

	bool Validate(wxWindow* parent) override { return true; }
	bool CheckCharacter(int chr, bool is_first, bool *got_decimal) const;
	void OnChar(wxKeyEvent& event);

	DoubleValidator(DoubleValidator const& rgt);

	wxObject* Clone() const override { return new DoubleValidator(*this); }
	bool TransferToWindow() override;
	bool TransferFromWindow() override;

public:
	explicit DoubleValidator(double *val, bool allow_negative=false);
	explicit DoubleValidator(double *val, double min, double max);
};

class DoubleSpinValidator final : public wxValidator {
	double *value;
	wxValidator *Clone() const override { return new DoubleSpinValidator(value); }
	bool Validate(wxWindow*) override { return true; }
	bool TransferToWindow() override;
	bool TransferFromWindow() override;

public:
	DoubleSpinValidator(double *value) : value(value) { }
};

template<typename T>
class EnumBinder final : public wxValidator {
	T *value;

	wxObject *Clone() const override { return new EnumBinder<T>(value); }
	bool Validate(wxWindow *) override { return true; }

	bool TransferFromWindow() override {
		if (wxRadioBox *rb = dynamic_cast<wxRadioBox*>(GetWindow()))
			*value = static_cast<T>(rb->GetSelection());
		else
			throw agi::InternalError("Control type not supported by EnumBinder", nullptr);
		return true;
	}

	bool TransferToWindow() override {
		if (wxRadioBox *rb = dynamic_cast<wxRadioBox*>(GetWindow()))
			rb->SetSelection(static_cast<int>(*value));
		else
			throw agi::InternalError("Control type not supported by EnumBinder", nullptr);
		return true;
	}

public:
	explicit EnumBinder(T *value) : value(value) { }
	EnumBinder(EnumBinder const& rhs) : wxValidator(rhs), value(rhs.value) { }
};

template<typename T>
EnumBinder<T> MakeEnumBinder(T *value) {
	return EnumBinder<T>(value);
}

class StringBinder final : public wxValidator {
	std::string *value;

	wxObject* Clone() const override { return new StringBinder(value); }
	bool Validate(wxWindow*) override { return true;}
	bool TransferToWindow() override;
	bool TransferFromWindow() override;

public:
	explicit StringBinder(std::string *value) : value(value) { }
};
