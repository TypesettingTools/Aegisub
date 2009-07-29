/////////////////////////////////////////////////////////////////////////////
// Name:        bevelButton.cpp
// Purpose:     wxBevelButton, a button that looks like a toggle button in wxMac
// Author:      David Conrad
// Modified by:
// Created:     2006-06-16
// RCS-ID:      $Id$
// Copyright:   (c) David Conrad
// Licence:       wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "config.h"

#ifdef __WXMAC__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "osx_bevelButton.h"
#endif

#include "wx/wxprec.h"

#include "osx_bevelButton.h"
#include "wx/panel.h"
#include "wx/stockitem.h"

IMPLEMENT_DYNAMIC_CLASS(wxBevelButton, wxControl)

#include "wx/mac/uma.h"
// Button


/// DOCME
static const int kMacOSXHorizontalBorder = 2 ;

/// DOCME
static const int kMacOSXVerticalBorder = 4 ;


/// @brief DOCME
/// @param parent    
/// @param id        
/// @param lbl       
/// @param pos       
/// @param size      
/// @param style     
/// @param validator 
/// @param name      
/// @return 
///
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


/// @brief DOCME
///
wxSize wxBevelButton::DoGetBestSize() const
{
    int wBtn = 70 ; 
    int hBtn = 20 ;
	
    int lBtn = m_label.Length() * 8 + 12 ;
    if (lBtn > wBtn) 
        wBtn = lBtn;
	
    return wxSize ( wBtn , hBtn ) ;
}

#endif // __WXMAC__


