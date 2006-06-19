/////////////////////////////////////////////////////////////////////////////
// Name:        bevelButton.cpp
// Purpose:     wxBevelButton, a button that looks like a toggle button in wxMac
// Author:      David Conrad
// Modified by:
// Created:     2006-06-16
// RCS-ID:      $Id: bevelButton.cpp,v 1.0 2006/06/16 23:29:20 SC Exp $
// Copyright:   (c) David Conrad
// Licence:       wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "button.h"
#endif

#include "wx/wxprec.h"

#include "bevelButton.h"
#include "wx/panel.h"
#include "wx/stockitem.h"

IMPLEMENT_DYNAMIC_CLASS(wxBevelButton, wxControl)

#include "wx/mac/uma.h"
#include "bevelButton.h"
// Button

static const int kMacOSXHorizontalBorder = 2 ;
static const int kMacOSXVerticalBorder = 4 ;

bool wxBevelButton::Create(wxWindow *parent, wxWindowID id, const wxString& lbl,
						   const wxPoint& pos,
						   const wxSize& size, long style,
						   const wxValidator& validator,
						   const wxString& name)
{
    wxString label(lbl);
    if (label.empty() && wxIsStockID(id))
        label = wxGetStockLabel(id);
    
    m_macIsUserPane = FALSE ;
    
    if ( !wxButtonBase::Create(parent, id, pos, size, style, validator, name) )
        return false;
    
    m_label = label ;
	
    Rect bounds = wxMacGetBoundsForControl( this , pos , size ) ;
    m_peer = new wxMacControl(this) ;

	verify_noerr ( CreateBevelButtonControl( MAC_WXHWND(parent->MacGetTopLevelWindowRef()) , &bounds , CFSTR("") , 
		 kControlBevelButtonNormalBevel , 0 , NULL , 0 , 0 , 0 , m_peer->GetControlRefAddr() ) );

    MacPostControlCreate(pos,size) ;
    
	return TRUE;
}

wxSize wxBevelButton::DoGetBestSize() const
{
    int wBtn = 70 ; 
    int hBtn = 20 ;
	
    int lBtn = m_label.Length() * 8 + 12 ;
    if (lBtn > wBtn) 
        wBtn = lBtn;
	
    return wxSize ( wBtn , hBtn ) ;
}
