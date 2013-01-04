// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file validators.h
/// @see validators.cpp
/// @ingroup custom_control utility
///

#include <libaegisub/exception.h>

#include <string>

#include <wx/radiobox.h>
#include <wx/validate.h>

/// A wx validator that only allows valid numbers
class NumValidator : public wxValidator {
	double fValue; ///< Value if isFloat is true
	int iValue;    ///< Value if isFloat is false
	bool isFloat;  ///< Should decimals be allowed?
	bool isSigned; ///< Can the number be negative?

	/// Polymorphic copy
	wxObject* Clone() const;
	/// Check if the value in the passed window is valid
	bool Validate(wxWindow* parent);
	/// Copy the currently stored value to the associated window
	bool TransferToWindow();
	/// Read the value in the associated window and validate it
	bool TransferFromWindow();

	/// Check a single character
	/// @param chr Character to check
	/// @param isFirst Is this the first character in the string?
	/// @param canSign Can this character be a sign?
	/// @param gotDecimal[in,out] Has a decimal been found? Set to true if a chr is a decimal
	/// @return Is this character valid?
	bool CheckCharacter(int chr,bool isFirst,bool canSign,bool &gotDecimal);

	/// wx character event handler
	void OnChar(wxKeyEvent& event);

public:
	/// Constructor
	/// @param val Initial value to set the associated control to
	/// @param isfloat Allow floats, or just ints?
	/// @param issigned Allow negative numbers?
	explicit NumValidator(wxString val = wxString(), bool isfloat=false, bool issigned=false);

	/// Constructor
	/// @param val Initial value to set the associated control to
	/// @param issigned Allow negative numbers?
	explicit NumValidator(int val, bool issigned=false);

	/// Constructor
	/// @param val Initial value to set the associated control to
	/// @param issigned Allow negative numbers?
	explicit NumValidator(int64_t val, bool issigned=false);

	/// Constructor
	/// @param val Initial value to set the associated control to
	/// @param issigned Allow negative numbers?
	explicit NumValidator(double val, bool issigned=false);

	/// Copy constructor
	NumValidator(const NumValidator& from);

	DECLARE_EVENT_TABLE()
};

template<typename T>
class EnumBinder : public wxValidator {
	T *value;

	wxObject *Clone() const override { return new EnumBinder<T>(value); }

	bool TransferFromWindow() override {
		if (wxRadioBox *rb = dynamic_cast<wxRadioBox*>(GetWindow()))
			*value = static_cast<T>(rb->GetSelection());
		else
			throw agi::InternalError("Control type not supported by EnumBinder", 0);
		return true;
	}

	bool TransferToWindow() override {
		if (wxRadioBox *rb = dynamic_cast<wxRadioBox*>(GetWindow()))
			rb->SetSelection(static_cast<int>(*value));
		else
			throw agi::InternalError("Control type not supported by EnumBinder", 0);
		return true;
	}

public:
	explicit EnumBinder(T *value) : value(value) { }
	EnumBinder(EnumBinder const& rhs) : value(rhs.value) { }
};

template<typename T>
EnumBinder<T> MakeEnumBinder(T *value) {
	return EnumBinder<T>(value);
}

class StringBinder : public wxValidator {
	std::string *value;

	wxObject* Clone() const { return new StringBinder(value); }
	bool Validate(wxWindow*) { return true;}
	bool TransferToWindow();
	bool TransferFromWindow();

public:
	explicit StringBinder(std::string *value) : value(value) { }
};
