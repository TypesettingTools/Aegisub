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


#pragma once


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>


//////////////
// Prototypes
class FloatSpinCtrl;


///////////////////
// Float spin text
class FloatSpinText : public wxTextCtrl {
	friend class FloatSpinCtrl;
private:
	FloatSpinCtrl *parent;

	FloatSpinText(FloatSpinCtrl *parent,int id);

	void OnKillFocus(wxFocusEvent &event);
	void OnKeyPress(wxKeyEvent &event);
	void SendValue();

	DECLARE_EVENT_TABLE()
};


//////////////////////
// Float spin control
class FloatSpinCtrl : public wxPanel {
	friend class FloatSpinText;
private:
	wxTextCtrl *text;
	wxSpinButton *button;

	double min;
	double max;
	double step;
	double value;

	void UpdateText();

	void OnSpin(wxSpinEvent &event);

public:
	FloatSpinCtrl(wxWindow* parent,wxWindowID id=-1,const wxPoint& pos=wxDefaultPosition,const wxSize& size = wxDefaultSize, long style = wxSP_ARROW_KEYS, double min = 0.0, double max = 100.0, double initial = 0.0, double step = 1.0, const wxString& name = _T("wxSpinCtrl"));

	void SetValue(double value);
	void SetRange(double min,double max,double step);

	double GetValue() { return value; }
	double GetMin() { return min; }
	double GetMax() { return max; }

	DECLARE_EVENT_TABLE()
};
