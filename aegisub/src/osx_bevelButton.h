/////////////////////////////////////////////////////////////////////////////
// Name:        bevelButton.h
// Purpose:     wxBevelButton class, a button that looks like Toggle buttons in wxMac
// Author:      David Conrad
// Modified by:
// Created:     2006-06-16
// RCS-ID:      $Id: bevelButton.h,v 1.0 2006/06/16 23:29:20 VS Exp $
// Copyright:   (c) David Conrad
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __WXMAC__

#ifndef _WX_BEVEL_BUTTON_H_
#define _WX_BEVEL_BUTTON_H_

#include "wx/control.h"
#include "wx/gdicmn.h"

//WXDLLEXPORT_DATA(extern const wxChar*) wxButtonNameStr;

// Pushbutton
class WXDLLEXPORT wxBevelButton: public wxButton
{
	DECLARE_DYNAMIC_CLASS(wxButton)
public:
	inline wxBevelButton() {}
	inline wxBevelButton(wxWindow *parent, wxWindowID id,
						 const wxString& label = wxEmptyString,
						 const wxPoint& pos = wxDefaultPosition,
						 const wxSize& size = wxDefaultSize, long style = 0,
						 const wxValidator& validator = wxDefaultValidator,
						 const wxString& name = wxButtonNameStr)
{
		Create(parent, id, label, pos, size, style, validator, name);
}

bool Create(wxWindow *parent, wxWindowID id,
			const wxString& label = wxEmptyString,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize, long style = 0,
			const wxValidator& validator = wxDefaultValidator,
			const wxString& name = wxButtonNameStr);

protected:
virtual wxSize DoGetBestSize() const ;
};

#endif
// _WX_BUTTON_H_

#endif // __WXMAC__

