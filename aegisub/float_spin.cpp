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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/sizer.h>
#include "float_spin.h"
#include "utils.h"
#include "validators.h"


///////
// IDs
enum {
	SPIN_BUTTON = 1100,
	TEXT_EDIT
};


///////////////
// Constructor
FloatSpinCtrl::FloatSpinCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size,
							 long style, double _min, double _max, double initial, double _step, const wxString& name)
: wxPanel(parent,id,pos,size,style,name)
{
	// Set data
	value = initial;

	// Create sub-controls
	text = new FloatSpinText(this,TEXT_EDIT);
	button = new wxSpinButton(this,SPIN_BUTTON,wxDefaultPosition,wxSize(-1,20),wxSP_VERTICAL);
	SetRange(_min,_max,_step);

	// Set sizer
	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(text,1,wxEXPAND,0);
	sizer->Add(button,0,wxEXPAND,0);
	SetSizer(sizer);
	//sizer->SetSizeHints(this);

	// Update text
	UpdateText();
}


/////////////
// Set value
void FloatSpinCtrl::SetValue(double _value) {
	value = _value;
	button->SetValue(int(value/step));
	text->SetValue(PrettyFloatD(value));
}


/////////////
// Set range
void FloatSpinCtrl::SetRange(double _min,double _max,double _step) {
	min = _min;
	max = _max;
	step = _step;
	button->SetRange(int(min/step),int(max/step));
}


///////////////
// Update text
void FloatSpinCtrl::UpdateText() {
	text->SetValue(PrettyFloatD(value));
}


///////////////
// Event table
BEGIN_EVENT_TABLE(FloatSpinCtrl,wxPanel)
	EVT_SPIN(SPIN_BUTTON, FloatSpinCtrl::OnSpin)
END_EVENT_TABLE()


///////////
// On spin
void FloatSpinCtrl::OnSpin(wxSpinEvent &event) {
	value = double(event.GetPosition())*step;
	UpdateText();
}


////////////////////////// EDIT BOX ////////////////////////////

///////////////
// Constructor
FloatSpinText::FloatSpinText(FloatSpinCtrl *_parent,int id)
: wxTextCtrl(_parent,id,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator(NULL,true,true))
{
	parent = _parent;
}


//////////////
// Focus lost
void FloatSpinText::OnKillFocus(wxFocusEvent &event) {
	SendValue();
}


///////////////
// Key pressed
void FloatSpinText::OnKeyPress(wxKeyEvent &event) {
	int code = event.GetKeyCode();
	if (code == WXK_UP || code == WXK_DOWN) {
		// Update first
		SendValue();

		// Get delta
		int sign = -1;
		if (code == WXK_UP) sign = 1;

		// Change value
		int value = parent->button->GetValue()+sign;
		parent->button->SetValue(value);
		parent->value = parent->button->GetValue()*parent->step;
		parent->UpdateText();

		return;
	}
	event.Skip();
}


//////////////
// Send value
void FloatSpinText::SendValue() {
	try {
		double value = 0.0;
		GetValue().ToDouble(&value);
		int pos = int(value/parent->step);
		parent->button->SetValue(pos);
		parent->UpdateText();
	}
	catch (...) {
		parent->UpdateText();
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(FloatSpinText,wxTextCtrl)
	EVT_KEY_DOWN(FloatSpinText::OnKeyPress)
	EVT_KILL_FOCUS(FloatSpinText::OnKillFocus)
END_EVENT_TABLE()
