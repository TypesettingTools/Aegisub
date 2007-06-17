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
#include "float_spin.h"


///////////////
// Constructor
FloatSpinCtrl::FloatSpinCtrl(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size,
							 long style, double _min, double _max, double initial, double _step, const wxString& name)
: wxPanel(parent,id,pos,size,style,name)
{
	// Set data
	min = _min;
	max = _max;
	step = _step;
	value = initial;

	// Create sub-controls
	text = new wxTextCtrl(this,-1,wxString::Format(_T("%f"),initial));
	button = new wxSpinButton(this,-1,wxDefaultPosition,wxSize(-1,20),wxSP_VERTICAL);

	// Set sizer
	wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(text,1,wxEXPAND,0);
	sizer->Add(button,0,wxEXPAND,0);
	SetSizer(sizer);
	sizer->SetSizeHints(this);
}


/////////////
// Set value
void FloatSpinCtrl::SetValue(double value) {
}


/////////////
// Set range
void FloatSpinCtrl::SetRange(double min,double max) {
}


////////////
// Set step
void FloatSpinCtrl::SetStep(double step) {
}
