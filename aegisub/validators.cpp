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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "validators.h"
#include "utils.h"


///////////////
// Constructor
NumValidator::NumValidator(wxString* _valPtr,bool isfloat,bool issigned) {
	isFloat = isfloat;
	isSigned = issigned;
	valPtr = _valPtr;

	// Get value
	if (valPtr) {
		if (isFloat) {
			valPtr->ToDouble(&fValue);
		}
		else {
			long tLong;
			valPtr->ToLong(&tLong);
			iValue = tLong;
		}
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(NumValidator, wxValidator)
    EVT_CHAR(NumValidator::OnChar)
END_EVENT_TABLE()


/////////
// Clone
wxObject* NumValidator::Clone() const {
	NumValidator *clone = new NumValidator(valPtr,isFloat,isSigned);
	return clone;
}


////////////
// Validate
bool NumValidator::Validate(wxWindow* parent) {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Is the control enabled?
    if (!ctrl->IsEnabled()) return true;

	// Check length
	if (value.Length() < 1) return false;

	// Check each character
	bool gotDecimal = false;
	for (size_t i=0;i<value.Length();i++) {
		if (!CheckCharacter(value[i],i==0,gotDecimal)) return false;
	}

	// All clear
	return true;
}


///////////////////////////////////////
// Check if a given character is valid
bool NumValidator::CheckCharacter(int chr,bool isFirst,bool &gotDecimal) {
	// Check sign
	if (chr == _T('-') || chr == _T('+')) {
		if (!isFirst || !isSigned) return false;
		else return true;
	}

	// Check decimal point
	if (chr == _T('.') || chr == _T(',')) {
		if (!isFloat || gotDecimal) return false;
		else {
			gotDecimal = true;
			return true;
		}
	}

	// Check digit
	if (chr < _T('0') || chr > _T('9')) return false;
	return true;
}


/////////////////////
// Filter keypresses
void NumValidator::OnChar(wxKeyEvent& event) {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();
	int chr = event.GetKeyCode();

	// Special keys
	if (chr < WXK_SPACE || chr == WXK_DELETE || chr > WXK_START) {
		event.Skip();
		return;
	}

	// Get selection
	long from,to;
	ctrl->GetSelection(&from,&to);

	// Count decimal points and signs outside selection
	int decimals = 0;
	int signs = 0;
	wxChar curchr;
	for (size_t i=0;i<value.Length();i++) {
		if (i >= (unsigned)from && i < (unsigned)to) continue;
		curchr = value[i];
		if (curchr == _T('.') || curchr == _T(',')) decimals++;
		if (curchr == _T('+') || curchr == _T('-')) signs++;
	}
	bool gotDecimal = decimals > 0;
	bool canSign = from == 0 && signs == 0;

	// Check character
	if (!CheckCharacter(chr,canSign,gotDecimal)) {
		if (!wxValidator::IsSilent()) wxBell();
		return;
	}

	// OK
	event.Skip();
	return;
}


//////////////////////
// Transfer to window
bool NumValidator::TransferToWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	if (isFloat) ctrl->SetValue(PrettyFloatD(fValue));
	else ctrl->SetValue(wxString::Format(_T("%d"),iValue));

	return true;
}


///////////////////////
// Receive from window
bool NumValidator::TransferFromWindow() {
	wxTextCtrl *ctrl = (wxTextCtrl*) GetWindow();
	wxString value = ctrl->GetValue();

	// Validate
	bool ok = Validate(ctrl);
	if (!ok) return false;

	// Transfer
	if (isFloat) {
		value.ToDouble(&fValue);
	}
	else {
		long tLong;
		value.ToLong(&tLong);
		iValue = tLong;
	}

	return true;
}
