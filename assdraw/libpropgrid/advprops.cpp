/////////////////////////////////////////////////////////////////////////////
// Name:        advprops.cpp
// Purpose:     wxPropertyGrid Advanced Properties (font, colour, etc.)
// Author:      Jaakko Salli
// Modified by:
// Created:     Sep-25-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "advprops.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/object.h"
    #include "wx/hash.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/window.h"
    #include "wx/panel.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/button.h"
    #include "wx/pen.h"
    #include "wx/brush.h"
    #include "wx/cursor.h"
    #include "wx/dialog.h"
    #include "wx/settings.h"
    #include "wx/msgdlg.h"
    #include "wx/choice.h"
    #include "wx/stattext.h"
    #include "wx/textctrl.h"
    #include "wx/scrolwin.h"
    #include "wx/dirdlg.h"
    #include "wx/combobox.h"
    #include "wx/layout.h"
    #include "wx/sizer.h"
    #include "wx/textdlg.h"
    #include "wx/filedlg.h"
    #include "wx/intl.h"
#endif

#define __wxPG_SOURCE_FILE__

#include <wx/propgrid/propgrid.h>

#if wxPG_INCLUDE_ADVPROPS

#include <wx/propgrid/propdev.h>

#include <wx/propgrid/advprops.h>

#ifdef __WXMSW__
    #include <wx/msw/private.h>
#endif

// -----------------------------------------------------------------------

#if defined(__WXMSW__)
    #define wxPG_CAN_DRAW_CURSOR           1
#elif defined(__WXGTK__)
    #define wxPG_CAN_DRAW_CURSOR           0
#elif defined(__WXMAC__)
    #define wxPG_CAN_DRAW_CURSOR           0
#else
    #define wxPG_CAN_DRAW_CURSOR           0
#endif


#if !defined(wxPG_ALLOW_WXADV)
    #undef wxUSE_DATEPICKCTRL
    #define wxUSE_DATEPICKCTRL 0
#endif

// -----------------------------------------------------------------------
// Value type related
// -----------------------------------------------------------------------


bool operator == (const wxFont&, const wxFont&)
{
    return false;
}

WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ(wxFont,wxFontProperty,(const wxFont*)NULL)

// Implement dynamic class for type value.
IMPLEMENT_DYNAMIC_CLASS(wxColourPropertyValue,wxObject)

bool operator == (const wxColourPropertyValue& a, const wxColourPropertyValue& b)
{
    return ( ( a.m_colour == b.m_colour ) && (a.m_type == b.m_type) );
}

WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ(wxColourPropertyValue,wxSystemColourProperty,
                                 (const wxColourPropertyValue*)NULL)

WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ(wxColour,wxColourProperty,
                                 (const wxColour*)wxBLACK)

bool operator == (const wxArrayInt& array1, const wxArrayInt& array2)
{
    if ( array1.GetCount() != array2.GetCount() )
        return false;
    size_t i;
    for ( i=0; i<array1.GetCount(); i++ )
    {
        if ( array1[i] != array2[i] )
            return false;
    }
    return true;
}

WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP(wxArrayInt,
                                 wxMultiChoiceProperty,
                                 wxArrayInt())

#if wxUSE_DATETIME

// This macro can be used for values that have built-in support in wxVariant.
WX_PG_IMPLEMENT_VALUE_TYPE(wxDateTime,wxDateProperty,wxT("datetime"),GetDateTime,wxDateTime())

#endif

// -----------------------------------------------------------------------
// wxSpinCtrl-based property editor
// -----------------------------------------------------------------------

#if wxUSE_SPINBTN

//
// Implement an editor control that allows using wxSpinCtrl (actually,
// a combination of wxTextCtrl and wxSpinButton) to edit value of
// wxIntProperty and wxFloatProperty (and similar).
//
// Note that new editor classes needs to be registered before use.
// This can be accomplished using wxPGRegisterEditorClass macro, which
// is used for SpinCtrl in wxPropertyContainerMethods::RegisterAdditionalEditors
// (see below). Registeration can also be performed in a constructor of a
// property that is likely to require the editor in question.
//


#include <wx/spinbutt.h>


// NOTE: Regardless that this class inherits from a working editor, it has
//   all necessary methods to work independently. wxTextCtrl stuff is only
//   used for event handling here.
class wxPGSpinCtrlEditor : public wxPGTextCtrlEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    virtual ~wxPGSpinCtrlEditor();

    // See below for short explanations of what these are suppposed to do.
    wxPG_DECLARE_CREATECONTROLS

    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd, wxEvent& event ) const;
};


// This macro also defines global wxPGEditor_SpinCtrl for storing
// the singleton class instance.
WX_PG_IMPLEMENT_EDITOR_CLASS(SpinCtrl,wxPGSpinCtrlEditor,wxPGEditor)


// Trivial destructor.
wxPGSpinCtrlEditor::~wxPGSpinCtrlEditor()
{
}


// Create controls and initialize event handling.
#ifndef __WXPYTHON__
wxWindow* wxPGSpinCtrlEditor::CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
                                              const wxPoint& pos, const wxSize& sz, wxWindow** pSecondary ) const
#else
wxPGWindowPair wxPGSpinCtrlEditor::CreateControls( wxPropertyGrid* propgrid, wxPGProperty* property,
                                                   const wxPoint& pos, const wxSize& sz ) const
#endif
{
    const int margin = 1;
    wxSize butSz(18, sz.y);
    wxSize tcSz(sz.x - butSz.x - margin, sz.y);
    wxPoint butPos(pos.x + tcSz.x + margin, pos.y);

    wxSpinButton* wnd2 = new wxSpinButton();
#ifdef __WXMSW__
    wnd2->Hide();
#endif
    wnd2->Create( propgrid, wxPG_SUBID2, butPos, butSz, wxSP_VERTICAL );
    wnd2->SetRange( INT_MIN, INT_MAX );
    //wnd2->SetRange( 5, 12 );
    wnd2->SetValue( 0 );

    propgrid->Connect( wxPG_SUBID2, wxEVT_SCROLL_LINEUP,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent, NULL, propgrid );
    propgrid->Connect( wxPG_SUBID2, wxEVT_SCROLL_LINEDOWN,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent, NULL, propgrid );

    // Let's add validator to make sure only numbers can be entered
    wxString temps;
    wxTextValidator validator(wxFILTER_NUMERIC, &temps);

#ifndef __WXPYTHON__
    wxTextCtrl* wnd1 = (wxTextCtrl*) wxPGTextCtrlEditor::CreateControls( propgrid, property, pos, tcSz, NULL );
    wnd1->SetValidator(validator);

    *pSecondary = wnd2;
    return wnd1;
#else
    wxTextCtrl* wnd1 = (wxTextCtrl*) wxPGTextCtrlEditor::CreateControls( propgrid, property, pos, tcSz ).m_primary;
    wnd1->SetValidator(validator);

    return wxPGWindowPair(wnd1, wnd2);
#endif
}

// Control's events are redirected here
bool wxPGSpinCtrlEditor::OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
                                  wxWindow* wnd, wxEvent& event ) const
{
    int evtType = event.GetEventType();

    if ( evtType == wxEVT_SCROLL_LINEUP || evtType == wxEVT_SCROLL_LINEDOWN )
    {
        wxString s;
        // Can't use wnd since it might be clipper window
        wxTextCtrl* tc = wxDynamicCast(propgrid->GetEditorControl(), wxTextCtrl);

        if ( tc )
            s = tc->GetValue();
        else
            s = property->GetValueAsString(wxPG_FULL_VALUE);

        wxSpinButton* spinButton = (wxSpinButton*) propgrid->GetEditorControlSecondary();
        int spinMin = spinButton->GetMin();
        int spinMax = spinButton->GetMax();

        if ( property->GetValueType() == wxPG_VALUETYPE(double) )
        {
            double v_d;

            // Try double
            if ( s.ToDouble(&v_d) )
            {
                if ( evtType == wxEVT_SCROLL_LINEUP ) v_d += 1.0;
                else v_d -= 1.0;

                // Min/Max
                double dSpinMin = (double) spinMin;
                double dSpinMax = (double) spinMax;
                if ( v_d > dSpinMax ) v_d = dSpinMax;
                else if ( v_d < dSpinMin ) v_d = dSpinMin;
                
                wxPropertyGrid::DoubleToString(s, v_d, 6, true, NULL);
            }
            else
            {
                return false;
            }
        }
        else
        {
            long v_l;

            // Try long
            if ( s.ToLong(&v_l, 0) )
            {
                if ( evtType == wxEVT_SCROLL_LINEUP ) v_l++;
                else v_l--;

                // Min/Max
                if ( v_l > spinMax ) v_l = spinMax;
                else if ( v_l < spinMin ) v_l = spinMin;

                s = wxString::Format(wxT("%i"),(int)v_l);
            }
            else
            {
                return false;
            }
        }

        if ( tc )
            tc->SetValue(s);

        return true;
    }

    return wxPGTextCtrlEditor::OnEvent(propgrid,property,wnd,event);
}

#endif // wxUSE_SPINBTN


// -----------------------------------------------------------------------
// wxDatePickerCtrl-based property editor
// -----------------------------------------------------------------------

#if wxUSE_DATEPICKCTRL


#include <wx/datectrl.h>
#include <wx/dateevt.h>

class wxPGDatePickerCtrlEditor : public wxPGEditor
{
    WX_PG_DECLARE_EDITOR_CLASS()
public:
    virtual ~wxPGDatePickerCtrlEditor();

    wxPG_DECLARE_CREATECONTROLS

    virtual void UpdateControl( wxPGProperty* property, wxWindow* wnd ) const;
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxPGProperty* property,
        wxWindow* wnd, wxEvent& event ) const;
    virtual bool CopyValueFromControl( wxPGProperty* property, wxWindow* wnd ) const;
    virtual void SetValueToUnspecified( wxWindow* wnd ) const;
};


WX_PG_IMPLEMENT_EDITOR_CLASS(DatePickerCtrl,wxPGDatePickerCtrlEditor,wxPGEditor)


wxPGDatePickerCtrlEditor::~wxPGDatePickerCtrlEditor()
{
}

#ifndef __WXPYTHON__
wxWindow* wxPGDatePickerCtrlEditor::CreateControls( wxPropertyGrid* propgrid,
                                                    wxPGProperty* property,
                                                    const wxPoint& pos,
                                                    const wxSize& sz,
                                                    wxWindow** ) const
#else
wxPGWindowPair wxPGDatePickerCtrlEditor::CreateControls( wxPropertyGrid* propgrid,
                                                         wxPGProperty* property,
                                                         const wxPoint& pos,
                                                         const wxSize& sz ) const
#endif
{
    wxCHECK_MSG( property->IsKindOf(WX_PG_CLASSINFO(wxDateProperty)),
                 NULL,
                 wxT("DatePickerCtrl editor can only be used with wxDateProperty or derivative.") );

    wxDatePropertyClass* prop = (wxDatePropertyClass*) property;

    // Use two stage creation to allow cleaner display on wxMSW
    wxDatePickerCtrl* ctrl = new wxDatePickerCtrl();
#ifdef __WXMSW__
    ctrl->Hide();
    wxSize useSz = wxDefaultSize;
    useSz.x = sz.x;
#else
    wxSize useSz = sz;
#endif
    ctrl->Create(propgrid,
                 wxPG_SUBID1,
                 prop->GetDateValue(),
                 pos,
                 useSz,
                 prop->GetDatePickerStyle() | wxNO_BORDER);

    // Connect all required events to grid's OnCustomEditorEvent
    // (all relevenat wxTextCtrl, wxComboBox and wxButton events are
    // already connected)
    propgrid->Connect( wxPG_SUBID1, wxEVT_DATE_CHANGED,
                       (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction)
                       &wxPropertyGrid::OnCustomEditorEvent );

#ifdef __WXMSW__
    ctrl->Show();
#endif

    return ctrl;
}

// Copies value from property to control
void wxPGDatePickerCtrlEditor::UpdateControl( wxPGProperty* property, wxWindow* wnd ) const
{
    wxDatePickerCtrl* ctrl = (wxDatePickerCtrl*) wnd;
    wxASSERT( ctrl && ctrl->IsKindOf(CLASSINFO(wxDatePickerCtrl)) );

    // We assume that property's data type is 'int' (or something similar),
    // thus allowing us to get raw, unchecked value via DoGetValue.
    ctrl->SetValue( *((const wxDateTime*)property->DoGetValue().GetVoidPtr()) );
}

// Control's events are redirected here
bool wxPGDatePickerCtrlEditor::OnEvent( wxPropertyGrid* WXUNUSED(propgrid),
                                        wxPGProperty* WXUNUSED(property),
                                        wxWindow* WXUNUSED(wnd),
                                        wxEvent& event ) const
{
    if ( event.GetEventType() == wxEVT_DATE_CHANGED )
        return true;

    return false;
}

bool wxPGDatePickerCtrlEditor::CopyValueFromControl( wxPGProperty* property, wxWindow* wnd ) const
{
    wxDatePickerCtrl* ctrl = (wxDatePickerCtrl*) wnd;
    wxASSERT( ctrl && ctrl->IsKindOf(CLASSINFO(wxDatePickerCtrl)) );

    wxDatePropertyClass* prop = (wxDatePropertyClass*) property;
    prop->SetDateValue( ctrl->GetValue() );

    return true;
}

void wxPGDatePickerCtrlEditor::SetValueToUnspecified( wxWindow* WXUNUSED(wnd) ) const
{
    // TODO?
    //wxDateProperty* prop = (wxDateProperty*) property;
    //ctrl->SetValue(?);
}

#endif // wxUSE_DATEPICKCTRL


// -----------------------------------------------------------------------
// wxFontProperty
// -----------------------------------------------------------------------

#include <wx/fontdlg.h>
#include <wx/fontenum.h>

static const wxChar* gs_fp_es_family_labels[] = {
    wxT("Default"), wxT("Decorative"),
    wxT("Roman"), wxT("Script"),
    wxT("Swiss"), wxT("Modern"),
	wxT("Teletype"),
    (const wxChar*) NULL
};

static long gs_fp_es_family_values[] = {
    wxFONTFAMILY_DEFAULT, wxFONTFAMILY_DECORATIVE,
    wxFONTFAMILY_ROMAN, wxFONTFAMILY_SCRIPT,
    wxFONTFAMILY_SWISS, wxFONTFAMILY_MODERN,
	wxFONTFAMILY_TELETYPE
};

static const wxChar* gs_fp_es_style_labels[] = {
    wxT("Normal"),
    wxT("Slant"),
    wxT("Italic"),
    (const wxChar*) NULL
};

static long gs_fp_es_style_values[] = {
    wxNORMAL,
    wxSLANT,
    wxITALIC
};

static const wxChar* gs_fp_es_weight_labels[] = {
    wxT("Normal"),
    wxT("Light"),
    wxT("Bold"),
    (const wxChar*) NULL
};

static long gs_fp_es_weight_values[] = {
    wxNORMAL,
    wxLIGHT,
    wxBOLD
};

// Class body is in advprops.h


WX_PG_IMPLEMENT_PROPERTY_CLASS(wxFontProperty,wxBaseParentProperty,
                               wxFont,const wxFont&,TextCtrlAndButton)


wxFontPropertyClass::wxFontPropertyClass( const wxString& label, const wxString& name,
                                          const wxFont& value )
    : wxPGPropertyWithChildren(label,name)
{
    wxPG_INIT_REQUIRED_TYPE(wxFont)
    DoSetValue( wxPGVariantCreator(value) );

    // Initialize font family choices list
    if ( !wxPGGlobalVars->m_fontFamilyChoices )
    {
        WX_PG_GLOBALS_LOCKER()

        wxFontEnumerator enumerator;
        enumerator.EnumerateFacenames();

#if wxMINOR_VERSION > 6
        wxArrayString faceNames = enumerator.GetFacenames();
#else
        wxArrayString& faceNames = *enumerator.GetFacenames();
#endif

        faceNames.Sort();

        wxPGGlobalVars->m_fontFamilyChoices = new wxPGChoices(faceNames);
    }

    wxString emptyString(wxEmptyString);

    AddChild( wxIntProperty( _("Point Size"),emptyString,m_value_wxFont.GetPointSize() ) );

    AddChild( wxEnumProperty(_("Family"), emptyString,
              gs_fp_es_family_labels,gs_fp_es_family_values,
              m_value_wxFont.GetFamily()) );

    wxString faceName = m_value_wxFont.GetFaceName();
    // If font was not in there, add it now
    if ( faceName.length() &&
         wxPGGlobalVars->m_fontFamilyChoices->Index(faceName) == wxNOT_FOUND )
        wxPGGlobalVars->m_fontFamilyChoices->AddAsSorted(faceName);

    wxPGProperty* p = wxEnumProperty(_("Face Name"),emptyString,
                                     *wxPGGlobalVars->m_fontFamilyChoices);

    p->SetValueFromString(faceName,wxPG_FULL_VALUE);

    AddChild( p );

    AddChild( wxEnumProperty(_("Style"),emptyString,
            gs_fp_es_style_labels,gs_fp_es_style_values,m_value_wxFont.GetStyle()) );

    AddChild( wxEnumProperty(_("Weight"),emptyString,
            gs_fp_es_weight_labels,gs_fp_es_weight_values,m_value_wxFont.GetWeight()) );

    AddChild( wxBoolProperty(_("Underlined"),emptyString,
            m_value_wxFont.GetUnderlined()) );

}

wxFontPropertyClass::~wxFontPropertyClass () { }

void wxFontPropertyClass::DoSetValue( wxPGVariant value )
{
    const wxFont* font = wxPGVariantToWxObjectPtr(value,wxFont);

    if ( font && font->Ok() )
        m_value_wxFont = *font;
    else
        m_value_wxFont = wxFont(10,wxSWISS,wxNORMAL,wxNORMAL);

    RefreshChildren();
}

wxPGVariant wxFontPropertyClass::DoGetValue() const
{
    return wxPGVariantCreator(m_value_wxFont);
}

wxString wxFontPropertyClass::GetValueAsString( int argFlags ) const
{
    return wxPGPropertyWithChildren::GetValueAsString(argFlags);
}

bool wxFontPropertyClass::OnEvent( wxPropertyGrid* propgrid, wxWindow* primary,
                                   wxEvent& event )
{
    if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED )
    {
        // Update value from last minute changes
        PrepareValueForDialogEditing(propgrid);

        wxFontData data;
        data.SetInitialFont(m_value_wxFont);
        data.SetColour(*wxBLACK);

        wxFontDialog dlg(propgrid, data);
        if ( dlg.ShowModal() == wxID_OK )
        {
            propgrid->EditorsValueWasModified();

            wxFontData retData = dlg.GetFontData();
            wxFont font = retData.GetChosenFont();

            DoSetValue(wxPGVariantCreator(font));
            UpdateControl(primary);

            return true;
        }
    }
    return false;
}

void wxFontPropertyClass::RefreshChildren()
{
    if ( !GetCount() ) return;
    Item(0)->DoSetValue( (long)m_value_wxFont.GetPointSize() );
    Item(1)->DoSetValue( (long)m_value_wxFont.GetFamily() );
    Item(2)->SetValueFromString( m_value_wxFont.GetFaceName(), wxPG_FULL_VALUE );
    Item(3)->DoSetValue( (long)m_value_wxFont.GetStyle() );
    Item(4)->DoSetValue( (long)m_value_wxFont.GetWeight() );
    Item(5)->DoSetValue( m_value_wxFont.GetUnderlined() );
}

void wxFontPropertyClass::ChildChanged( wxPGProperty* p )
{
    wxASSERT( this == p->GetParent() );

    int ind = p->GetIndexInParent();

    if ( ind == 0 )
    {
        m_value_wxFont.SetPointSize( wxPGVariantToLong(p->DoGetValue()) );
    }
    else if ( ind == 1 )
    {
        int fam = p->DoGetValue().GetLong();
        if ( fam < wxDEFAULT ||
             fam > wxTELETYPE )
             fam = wxDEFAULT;
        m_value_wxFont.SetFamily( fam );
    }
    else if ( ind == 2 )
    {
        m_value_wxFont.SetFaceName( p->GetValueAsString(wxPG_FULL_VALUE) );
    }
    else if ( ind == 3 )
    {
        int st = wxPGVariantToLong(p->DoGetValue());
        if ( st != wxFONTSTYLE_NORMAL &&
             st != wxFONTSTYLE_SLANT &&
             st != wxFONTSTYLE_ITALIC )
             st = wxFONTWEIGHT_NORMAL;
        m_value_wxFont.SetStyle( st );
    }
    else if ( ind == 4 )
    {
        int wt = wxPGVariantToLong(p->DoGetValue());
        if ( wt != wxFONTWEIGHT_NORMAL &&
             wt != wxFONTWEIGHT_LIGHT &&
             wt != wxFONTWEIGHT_BOLD )
             wt = wxFONTWEIGHT_NORMAL;
        m_value_wxFont.SetWeight( wt );
    }
    else if ( ind == 5 )
    {
        m_value_wxFont.SetUnderlined( wxPGVariantToBool(p->DoGetValue())?true:false );
    }
}

/*
wxSize wxFontPropertyClass::GetImageSize() const
{
    return wxSize(-1,-1);
}

void wxFontPropertyClass::OnCustomPaint(wxDC& dc,
                                        const wxRect& rect,
                                        wxPGPaintData& paintData)
{
    wxString drawFace;
    if ( paintData.m_choiceItem >= 0 )
        drawFace = wxPGGlobalVars->m_fontFamilyChoices->GetLabel(paintData.m_choiceItem);
    else
        drawFace = m_value_wxFont.GetFaceName();

    if ( drawFace.length() )
    {
        // Draw the background
        dc.SetBrush( wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)) );
        //dc.SetBrush( *wxWHITE_BRUSH );
        //dc.SetPen( *wxMEDIUM_GREY_PEN );
        dc.DrawRectangle( rect );

        wxFont oldFont = dc.GetFont();
        wxFont drawFont(oldFont.GetPointSize(),
                        wxDEFAULT,wxNORMAL,wxBOLD,false,drawFace);
        dc.SetFont(drawFont);

        dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT) );
        dc.DrawText( wxT("Aa"), rect.x+2, rect.y+1 );

        dc.SetFont(oldFont);
    }
    else
    {
        // No file - just draw a white box
        dc.SetBrush ( *wxWHITE_BRUSH );
        dc.DrawRectangle ( rect );
    }
}
*/


// -----------------------------------------------------------------------
// wxSystemColourProperty
// -----------------------------------------------------------------------

// wxEnumProperty based classes cannot use wxPG_PROP_CLASS_SPECIFIC_1
#define wxPG_PROP_HIDE_CUSTOM_COLOUR        wxPG_PROP_CLASS_SPECIFIC_2

#include <wx/colordlg.h>

//#define wx_cp_es_syscolours_len 25
static const wxChar* gs_cp_es_syscolour_labels[] = {
    wxT("AppWorkspace"),
    wxT("ActiveBorder"),
    wxT("ActiveCaption"),
    wxT("ButtonFace"),
    wxT("ButtonHighlight"),
    wxT("ButtonShadow"),
    wxT("ButtonText"),
    wxT("CaptionText"),
    wxT("ControlDark"),
    wxT("ControlLight"),
    wxT("Desktop"),
    wxT("GrayText"),
    wxT("Highlight"),
    wxT("HighlightText"),
    wxT("InactiveBorder"),
    wxT("InactiveCaption"),
    wxT("InactiveCaptionText"),
    wxT("Menu"),
    wxT("Scrollbar"),
    wxT("Tooltip"),
    wxT("TooltipText"),
    wxT("Window"),
    wxT("WindowFrame"),
    wxT("WindowText"),
    wxT("Custom"),
    (const wxChar*) NULL
};

static long gs_cp_es_syscolour_values[] = {
    wxSYS_COLOUR_APPWORKSPACE,
    wxSYS_COLOUR_ACTIVEBORDER,
    wxSYS_COLOUR_ACTIVECAPTION,
    wxSYS_COLOUR_BTNFACE,
    wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_BTNSHADOW,
    wxSYS_COLOUR_BTNTEXT ,
    wxSYS_COLOUR_CAPTIONTEXT,
    wxSYS_COLOUR_3DDKSHADOW,
    wxSYS_COLOUR_3DLIGHT,
    wxSYS_COLOUR_BACKGROUND,
    wxSYS_COLOUR_GRAYTEXT,
    wxSYS_COLOUR_HIGHLIGHT,
    wxSYS_COLOUR_HIGHLIGHTTEXT,
    wxSYS_COLOUR_INACTIVEBORDER,
    wxSYS_COLOUR_INACTIVECAPTION,
    wxSYS_COLOUR_INACTIVECAPTIONTEXT,
    wxSYS_COLOUR_MENU,
    wxSYS_COLOUR_SCROLLBAR,
    wxSYS_COLOUR_INFOBK,
    wxSYS_COLOUR_INFOTEXT,
    wxSYS_COLOUR_WINDOW,
    wxSYS_COLOUR_WINDOWFRAME,
    wxSYS_COLOUR_WINDOWTEXT,
    wxPG_COLOUR_CUSTOM
};


// Class body is in advprops.h


WX_PG_IMPLEMENT_PROPERTY_CLASS(wxSystemColourProperty,wxEnumProperty,
                               wxColourPropertyValue,const wxColourPropertyValue&,Choice)


void wxSystemColourPropertyClass::Init( int type, const wxColour& colour )
{

    m_value.m_type = type;
    if ( colour.Ok() )
        m_value.m_colour = colour;
    else
        m_value.m_colour = *wxWHITE;

    m_flags |= wxPG_PROP_STATIC_CHOICES; // Colour selection cannot be changed.
}


static wxPGChoices gs_wxSystemColourProperty_choicesCache;


wxSystemColourPropertyClass::wxSystemColourPropertyClass( const wxString& label, const wxString& name,
    const wxColourPropertyValue& value )
    : wxEnumPropertyClass( label,
                           name,
                           gs_cp_es_syscolour_labels,
                           gs_cp_es_syscolour_values,
                           &gs_wxSystemColourProperty_choicesCache )
{
    wxPG_INIT_REQUIRED_TYPE(wxColourPropertyValue)

    if ( &value )
        Init(value.m_type,value.m_colour);
    else
        Init(0,*wxBLACK);

    DoSetValue( &m_value );
}


wxSystemColourPropertyClass::wxSystemColourPropertyClass( const wxString& label, const wxString& name,
    const wxChar** labels, const long* values, wxPGChoices* choicesCache,
    const wxColourPropertyValue& value )
    : wxEnumPropertyClass( label, name, labels, values, choicesCache )
{
    if ( &value )
        Init(value.m_type,value.m_colour);
    else
        Init(wxPG_COLOUR_CUSTOM,*wxBLACK);
}


wxSystemColourPropertyClass::wxSystemColourPropertyClass( const wxString& label, const wxString& name,
    const wxChar** labels, const long* values, wxPGChoices* choicesCache,
    const wxColour& value )
    : wxEnumPropertyClass( label, name, labels, values, choicesCache )
{
    Init(wxPG_COLOUR_CUSTOM,value);
}


wxSystemColourPropertyClass::~wxSystemColourPropertyClass() { }


int wxSystemColourPropertyClass::ColToInd( const wxColour& colour )
{
    size_t i;
    size_t i_max = m_choices.GetCount() - 1;

    if ( !colour.Ok() )
        return wxNOT_FOUND;

    long pixval = wxPG_COLOUR(colour.Red(),colour.Green(),colour.Blue());
    const wxArrayInt& arrValues = m_choices.GetValues();

    for ( i=0; i<i_max; i++ )
    {
        int ind = i;
        if ( arrValues.GetCount() )
            ind = arrValues[i];

        if ( GetColour(ind) == pixval )
        {
            /*wxLogDebug(wxT("%s(%s): Index %i for ( getcolour(%i,%i,%i), colour(%i,%i,%i))"),
                GetClassName(),GetLabel().c_str(),
                (int)i,(int)GetColour(ind).Red(),(int)GetColour(ind).Green(),(int)GetColour(ind).Blue(),
                (int)colour.Red(),(int)colour.Green(),(int)colour.Blue());*/
            return ind;
        }
    }
    return wxNOT_FOUND;
}


static inline wxColour wxColourFromPGLong( long col )
{
    return wxColour((col&0xFF),((col>>8)&0xFF),((col>>16)&0xFF));
}


void wxSystemColourPropertyClass::DoSetValue( wxPGVariant value )
{
    wxColourPropertyValue* pval = wxPGVariantToWxObjectPtr(value,wxColourPropertyValue);

    m_flags &= ~(wxPG_PROP_UNSPECIFIED);

    if ( pval != (wxColourPropertyValue*) NULL )
    {
        if ( !pval->m_colour.Ok() )
        {
            m_flags |= wxPG_PROP_UNSPECIFIED;
            m_index = wxNOT_FOUND;

            m_value.Init( wxPG_COLOUR_CUSTOM, *wxWHITE );
            return;
        }
        else if ( pval != &m_value )
        {
            m_value = *pval;
        }
    }
    else
    {
        m_value.Init( wxPG_COLOUR_CUSTOM, *wxWHITE );
    }

    if ( m_value.m_type < wxPG_COLOUR_WEB_BASE )
    {
        m_value.m_colour = GetColour( m_value.m_type );
        wxEnumPropertyClass::DoSetValue( (long)m_value.m_type );
    }
    else
    {
        m_index = m_choices.GetCount()-1;
    }
}


long wxSystemColourPropertyClass::GetColour( int index )
{
    wxColour colour = wxSystemSettings::GetColour ( (wxSystemColour)index );
    return wxPG_COLOUR(colour.Red(),colour.Green(),colour.Blue());
}


wxPGVariant wxSystemColourPropertyClass::DoGetValue() const
{
    return wxPGVariantCreator(&m_value);
}


wxString wxSystemColourPropertyClass::GetValueAsString( int argFlags ) const
{
    // Always show custom colour for non-choice editor
    // Assumes changed editor means its textctrl based...
    if ( m_value.m_type == wxPG_COLOUR_CUSTOM ||
         (argFlags & wxPG_PROPERTY_SPECIFIC) )
    {

/*#ifdef __WXDEBUG__
        // Sanity check
        if ( m_value.m_type != wxPG_COLOUR_CUSTOM &&
             (GetEditorClass() == wxPG_EDITOR(Choice) ||
              GetEditorClass() == wxPG_EDITOR(ChoiceAndButton)) )
        {
            wxLogDebug(wxT("wxSystemColourPropertyClass: Assumed wrong editor type!!!"));
        }
#endif*/

        return wxString::Format(wxT("(%i,%i,%i)"),
                                (int)m_value.m_colour.Red(),
                                (int)m_value.m_colour.Green(),
                                (int)m_value.m_colour.Blue());
    }
    return m_choices.GetLabel(m_index);
}


wxSize wxSystemColourPropertyClass::GetImageSize() const
{
    return wxSize(-1,-1);
}


bool wxSystemColourPropertyClass::QueryColourFromUser( wxPropertyGrid* propgrid, wxWindow* primary )
{
    bool res = false;

    m_value.m_type = wxPG_COLOUR_CUSTOM;

    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(m_value.m_colour);
    int i;
    for ( i = 0; i < 16; i++)
    {
        wxColour colour(i*16, i*16, i*16);
        data.SetCustomColour(i, colour);
    }

    wxColourDialog dialog(propgrid, &data);
    if ( dialog.ShowModal() == wxID_OK )
    {
        wxColourData retData = dialog.GetColourData();
        m_value.m_colour = retData.GetColour();
        wxSystemColourPropertyClass::DoSetValue(&m_value);

        res = true;
    }

    // Update text in combo box (so it is "(R,G,B)" not "Custom").
    if ( primary )
        GetEditorClass()->SetControlStringValue(primary,GetValueAsString(0));

    return res;
}


// Need to do some extra event handling.
bool wxSystemColourPropertyClass::OnEvent( wxPropertyGrid* propgrid, wxWindow* primary, wxEvent& event )
{
    if ( event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED )
    {
        int index = m_index; // m_index has already been updated.
        int type = wxEnumPropertyClass::DoGetValue().GetLong();

        const wxArrayInt& arrValues = m_choices.GetValues();

        if ( ( arrValues.GetCount() && type == wxPG_COLOUR_CUSTOM ) ||
             ( !arrValues.GetCount() && (index == (int)(m_choices.GetCount()-1) &&
                                         !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR))
             )
           )
        {
            QueryColourFromUser(propgrid,primary);

            return true;
        }
        else
        {
            m_value.m_type = type;
            m_value.m_colour = GetColour( type );
        }
    }
    else if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED )
    {
        // We need to handle button click in case editor has been
        // switched to one that has wxButton as well.
        return QueryColourFromUser(propgrid,primary);
    }
    return false;
}


void wxSystemColourPropertyClass::OnCustomPaint( wxDC& dc, const wxRect& rect,
                                                 wxPGPaintData& paintdata )
{
    if ( paintdata.m_choiceItem >= 0 &&
         ( paintdata.m_choiceItem < (int)(GetItemCount()-1) || (m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR))
       )
    {
        int colInd;
        const wxArrayInt& values = m_choices.GetValues();
        if ( values.GetCount() )
            colInd = values[paintdata.m_choiceItem];
        else
            colInd = paintdata.m_choiceItem;
        dc.SetBrush ( wxColour ( GetColour ( colInd ) ) );
    }
    else if ( !(m_flags & wxPG_PROP_UNSPECIFIED) )
        dc.SetBrush ( m_value.m_colour );
    else
        dc.SetBrush ( *wxWHITE );

    dc.DrawRectangle ( rect );
}


bool wxSystemColourPropertyClass::SetValueFromString( const wxString& text, int flags )
{
    wxColourPropertyValue val;
    if ( text[0] == wxT('(') )
    {
        // Custom colour.
        val.m_type = wxPG_COLOUR_CUSTOM;

        int r, g, b;
        wxSscanf(text.c_str(),wxT("(%i,%i,%i)"),&r,&g,&b);
        val.m_colour.Set(r,g,b);

        wxSystemColourPropertyClass::DoSetValue( &val );

        return true;
    }
    else
    {
        // Predefined colour.
        bool res = wxEnumPropertyClass::SetValueFromString(text,flags);
        if ( res )
        {
            val.m_type = m_index;
            const wxArrayInt& values = GetValues();
            if ( values.GetCount() )
                val.m_type = values[m_index];

            // Get proper colour for type.
            val.m_colour = wxColourFromPGLong(GetColour(val.m_type));

            wxSystemColourPropertyClass::DoSetValue( &val );
            return true;
        }
    }
    return false;
}


void wxSystemColourPropertyClass::SetAttribute( int id, wxVariant& value )
{
    if ( id == wxPG_COLOUR_ALLOW_CUSTOM )
    {
        int ival = value.GetLong();

        SetChoicesExclusive(); // Make sure we don't corrupt colour lists of other properties

        if ( ival && (m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        {
            // Show custom choice
            m_choices.Add(wxT("Custom"),wxPG_COLOUR_CUSTOM);
            m_flags &= ~(wxPG_PROP_HIDE_CUSTOM_COLOUR);
        }
        else if ( !ival && !(m_flags & wxPG_PROP_HIDE_CUSTOM_COLOUR) )
        {
            // Hide custom choice
            m_choices.RemoveAt(m_choices.GetCount()-1);
            m_flags |= wxPG_PROP_HIDE_CUSTOM_COLOUR;
        }
    }
}


// -----------------------------------------------------------------------
// wxColourProperty
// -----------------------------------------------------------------------

static const wxChar* gs_cp_es_normcolour_labels[] = {
    wxT("Black"),
    wxT("Maroon"),
    wxT("Navy"),
    wxT("Purple"),
    wxT("Teal"),
    wxT("Gray"),
    wxT("Green"),
    wxT("Olive"),
    wxT("Brown"),
    wxT("Blue"),
    wxT("Fuchsia"),
    wxT("Red"),
    wxT("Orange"),
    wxT("Silver"),
    wxT("Lime"),
    wxT("Aqua"),
    wxT("Yellow"),
    wxT("White"),
    wxT("Custom"),
    (const wxChar*) NULL
};

static unsigned long gs_cp_es_normcolour_colours[] = {
    wxPG_COLOUR(0,0,0),
    wxPG_COLOUR(128,0,0),
    wxPG_COLOUR(0,0,128),
    wxPG_COLOUR(128,0,128),
    wxPG_COLOUR(0,128,128),
    wxPG_COLOUR(128,128,128),
    wxPG_COLOUR(0,128,0),
    wxPG_COLOUR(128,128,0),
    wxPG_COLOUR(166,124,81),
    wxPG_COLOUR(0,0,255),
    wxPG_COLOUR(255,0,255),
    wxPG_COLOUR(255,0,0),
    wxPG_COLOUR(247,148,28),
    wxPG_COLOUR(192,192,192),
    wxPG_COLOUR(0,255,0),
    wxPG_COLOUR(0,255,255),
    wxPG_COLOUR(255,255,0),
    wxPG_COLOUR(255,255,255),
    wxPG_COLOUR(0,0,0)
};

WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR2(wxColourProperty,
                                                      wxColourPropertyClass,
                                                      gs_cp_es_normcolour_labels,
                                                      (const long*)NULL,
                                                      gs_cp_es_normcolour_colours,
                                                      TextCtrlAndButton)

// -----------------------------------------------------------------------
// wxCursorProperty
// -----------------------------------------------------------------------

#define wxPG_CURSOR_IMAGE_WIDTH     32

//#define wx_cp_es_syscursors_len 28
static const wxChar* gs_cp_es_syscursors_labels[] = {
    wxT("Default"),
    wxT("Arrow"),
    wxT("Right Arrow"),
    wxT("Blank"),
    wxT("Bullseye"),
    wxT("Character"),
    wxT("Cross"),
    wxT("Hand"),
    wxT("I-Beam"),
    wxT("Left Button"),
    wxT("Magnifier"),
    wxT("Middle Button"),
    wxT("No Entry"),
    wxT("Paint Brush"),
    wxT("Pencil"),
    wxT("Point Left"),
    wxT("Point Right"),
    wxT("Question Arrow"),
    wxT("Right Button"),
    wxT("Sizing NE-SW"),
    wxT("Sizing N-S"),
    wxT("Sizing NW-SE"),
    wxT("Sizing W-E"),
    wxT("Sizing"),
    wxT("Spraycan"),
    wxT("Wait"),
    wxT("Watch"),
    wxT("Wait Arrow"),
    (const wxChar*) NULL
};

static long gs_cp_es_syscursors_values[] = {
    wxCURSOR_NONE,
    wxCURSOR_ARROW,
    wxCURSOR_RIGHT_ARROW,
    wxCURSOR_BLANK,
    wxCURSOR_BULLSEYE,
    wxCURSOR_CHAR,
    wxCURSOR_CROSS,
    wxCURSOR_HAND,
    wxCURSOR_IBEAM,
    wxCURSOR_LEFT_BUTTON,
    wxCURSOR_MAGNIFIER,
    wxCURSOR_MIDDLE_BUTTON,
    wxCURSOR_NO_ENTRY,
    wxCURSOR_PAINT_BRUSH,
    wxCURSOR_PENCIL,
    wxCURSOR_POINT_LEFT,
    wxCURSOR_POINT_RIGHT,
    wxCURSOR_QUESTION_ARROW,
    wxCURSOR_RIGHT_BUTTON,
    wxCURSOR_SIZENESW,
    wxCURSOR_SIZENS,
    wxCURSOR_SIZENWSE,
    wxCURSOR_SIZEWE,
    wxCURSOR_SIZING,
    wxCURSOR_SPRAYCAN,
    wxCURSOR_WAIT,
    wxCURSOR_WATCH,
    wxCURSOR_ARROWWAIT
};

WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(wxCursorProperty,wxEnumProperty,int)

wxCursorPropertyClass::wxCursorPropertyClass( const wxString& label, const wxString& name,
    int value )
    : wxEnumPropertyClass( label,
                           name,
                           gs_cp_es_syscursors_labels,
                           gs_cp_es_syscursors_values,
                           value )
{
    m_flags |= wxPG_PROP_STATIC_CHOICES; // Cursor selection cannot be changed.
    //wxEnumPropertyClass::DoSetValue ( (void*)&value, NULL );
}

wxCursorPropertyClass::~wxCursorPropertyClass()
{
}

wxSize wxCursorPropertyClass::GetImageSize() const
{
#if wxPG_CAN_DRAW_CURSOR
    return wxSize(wxPG_CURSOR_IMAGE_WIDTH,wxPG_CURSOR_IMAGE_WIDTH);
#else
    return wxSize(0,0);
#endif
}

#if wxPG_CAN_DRAW_CURSOR

void wxCursorPropertyClass::OnCustomPaint( wxDC& dc,
                                           const wxRect& rect,
                                           wxPGPaintData& paintdata )
{

    // Background brush
    dc.SetBrush( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE ) );

    if ( paintdata.m_choiceItem >= 0 )
    {
        dc.DrawRectangle( rect );

        int cursorindex = gs_cp_es_syscursors_values[paintdata.m_choiceItem];

        /*
        if ( cursorindex == wxPG_CURSOR_FROM_FILE )
        {
            wxFAIL_MSG(wxT("not implemented"));
        }
        else if ( cursorindex == wxPG_CURSOR_FROM_RESOURCE )
        {
            wxFAIL_MSG(wxT("not implemented"));
        }
        else
        */
        {
            if ( cursorindex == wxCURSOR_NONE )
                cursorindex = wxCURSOR_ARROW;

            wxCursor cursor( cursorindex );

        #ifdef __WXMSW__
            ::DrawIconEx( (HDC)dc.GetHDC(),
                          rect.x,
                          rect.y,
                          (HICON)cursor.GetHandle(),
                          0,
                          0,
                          0,
                          NULL,
                          DI_COMPAT | DI_DEFAULTSIZE | DI_NORMAL
                        );
        #endif
        }
    }
}
#else
void wxCursorPropertyClass::OnCustomPaint( wxDC&, const wxRect&, wxPGPaintData& ) { }
#endif

// -----------------------------------------------------------------------
// wxImageFileProperty
// -----------------------------------------------------------------------

#if wxUSE_IMAGE

const wxString& wxPGGetDefaultImageWildcard()
{
    WX_PG_GLOBALS_LOCKER()

    // Form the wildcard, if not done yet
    if ( !wxPGGlobalVars->m_pDefaultImageWildcard.length() )
    {

        wxString str;

        // TODO: This section may require locking (using global).

        wxList& handlers = wxImage::GetHandlers();

        wxList::iterator node;

        // Let's iterate over the image handler list.
        //for ( wxList::Node *node = handlers.GetFirst(); node; node = node->GetNext() )
        for ( node = handlers.begin(); node != handlers.end(); node++ )
        {
            wxImageHandler *handler = (wxImageHandler*)*node;

            wxString ext_lo = handler->GetExtension();
            wxString ext_up = ext_lo.Upper();

            str.append( ext_up );
            str.append( wxT(" files (*.") );
            str.append( ext_up );
            str.append( wxT(")|*.") );
            str.append( ext_lo );
            str.append( wxT("|") );
        }

        str.append ( wxT("All files (*.*)|*.*") );

        wxPGGlobalVars->m_pDefaultImageWildcard = str;
    }

    return wxPGGlobalVars->m_pDefaultImageWildcard;
}

WX_PG_IMPLEMENT_DERIVED_PROPERTY_CLASS(wxImageFileProperty,
                                       wxFileProperty,
                                       const wxString&)

wxImageFilePropertyClass::wxImageFilePropertyClass( const wxString& label, const wxString& name,
    const wxString& value )
    : wxFilePropertyClass(label,name,value)
{

    m_wildcard = wxPGGetDefaultImageWildcard();

    m_pImage = (wxImage*) NULL;
    m_pBitmap = (wxBitmap*) NULL;
}

wxImageFilePropertyClass::~wxImageFilePropertyClass()
{
    if ( m_pBitmap )
        delete m_pBitmap;
    if ( m_pImage )
        delete m_pImage;
}

void wxImageFilePropertyClass::DoSetValue( wxPGVariant value )
{
    wxFilePropertyClass::DoSetValue(value);

    // Delete old image
    if ( m_pImage )
    {
        delete m_pImage;
        m_pImage = NULL;
    }
    if ( m_pBitmap )
    {
        delete m_pBitmap;
        m_pBitmap = NULL;
    }

    // Create the image thumbnail
    if ( m_filename.FileExists() )
    {
        m_pImage = new wxImage ( m_filename.GetFullPath() );
    }
}

wxSize wxImageFilePropertyClass::GetImageSize() const
{
    return wxSize(-1,-1);
}

void wxImageFilePropertyClass::OnCustomPaint( wxDC& dc,
                                              const wxRect& rect,
                                              wxPGPaintData& )
{
    if ( m_pBitmap || (m_pImage && m_pImage->Ok() ) )
    {
        // Draw the thumbnail

        // Create the bitmap here because required size is not known in DoSetValue().
        if ( !m_pBitmap )
        {
            m_pImage->Rescale( rect.width, rect.height );
            m_pBitmap = new wxBitmap( *m_pImage );
            delete m_pImage;
            m_pImage = NULL;
        }

        dc.DrawBitmap( *m_pBitmap, rect.x, rect.y, false );
    }
    else
    {
        // No file - just draw a white box
        dc.SetBrush( *wxWHITE_BRUSH );
        dc.DrawRectangle ( rect );
    }
}

#endif // wxUSE_IMAGE

// -----------------------------------------------------------------------
// wxMultiChoiceProperty
// -----------------------------------------------------------------------

#if wxUSE_CHOICEDLG

#include <wx/choicdlg.h>

#ifndef __WXPYTHON__
wxPGProperty* wxPG_CONSTFUNC(wxMultiChoiceProperty)(const wxString& label,
                                                    const wxString& name,
                                                    const wxPGChoices& choices,
                                                    const wxArrayInt& value)
{
    return new wxPG_PROPCLASS(wxMultiChoiceProperty)(label,name,choices,value);
}
#endif

wxPGProperty* wxPG_CONSTFUNC(wxMultiChoiceProperty)(const wxString& label,
                                                    const wxString& name,
                                                    const wxArrayString& strings,
                                                    const wxArrayInt& value)
{
    return new wxPG_PROPCLASS(wxMultiChoiceProperty)(label,name,strings,value);
}

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxMultiChoiceProperty,wxBaseProperty,
                               wxArrayInt,const wxArrayInt&,TextCtrlAndButton)

wxMultiChoicePropertyClass::wxMultiChoicePropertyClass(const wxString& label,
                                                       const wxString& name,
                                                       const wxPGChoices& choices,
                                                       const wxArrayInt& value)
                                                         : wxPGProperty(label,name)
{
    wxPG_INIT_REQUIRED_TYPE(wxArrayInt)

    m_choices.Assign(choices);
    SetValueI(value);
}


wxMultiChoicePropertyClass::wxMultiChoicePropertyClass(const wxString& label,
                                                       const wxString& name,
                                                       const wxArrayString& strings,
                                                       const wxArrayInt& value)
                                                         : wxPGProperty(label,name)
{
    wxPG_INIT_REQUIRED_TYPE(wxArrayInt)

    m_choices.Set(strings);
    SetValueI(value);
}

wxMultiChoicePropertyClass::wxMultiChoicePropertyClass(const wxString& label,
                                                       const wxString& name,
                                                       const wxArrayInt& WXUNUSED(value))
                                                         : wxPGProperty(label,name)
{
}

wxMultiChoicePropertyClass::~wxMultiChoicePropertyClass()
{
}

void wxMultiChoicePropertyClass::SetValueI( const wxArrayInt& arr )
{
    if ( &arr )
    {
        m_value_wxArrayInt = arr;
        GenerateValueAsString();
    }
    else
    {
        m_display = wxEmptyString;
    }
}

void wxMultiChoicePropertyClass::DoSetValue( wxPGVariant value )
{
#if !wxPG_PGVARIANT_IS_VARIANT
    wxArrayInt* pObj = (wxArrayInt*)wxPGVariantToVoidPtr(value);
    SetValueI(*pObj);
#else
    wxArrayInt arr = wxPGVariantToArrayInt(value);
    SetValueI(arr);
#endif
}

wxPGVariant wxMultiChoicePropertyClass::DoGetValue() const
{
    return wxPGVariantCreator(m_value_wxArrayInt);
}

wxString wxMultiChoicePropertyClass::GetValueAsString( int ) const
{
    return m_display;
}

void wxMultiChoicePropertyClass::GenerateValueAsString()
{
    // Allow zero-length strings list
    if ( !m_choices.IsOk() || !m_choices.GetCount() )
    {
        m_display = wxEmptyString;
        return;
    }

    wxString& tempStr = m_display;
    wxArrayInt indices = GetValueAsIndices();
    unsigned int i;
    unsigned int itemCount = indices.GetCount();

    tempStr.Empty();

    if ( itemCount )
        tempStr.append( wxT("\"") );

    for ( i = 0; i < itemCount; i++ )
    {
        int ind = indices.Item(i);
        wxCHECK_RET( ind >= 0 && ind < (int)m_choices.GetCount(),
                     wxT("value out of range") );
        tempStr.append( m_choices.GetLabel(ind) );
        tempStr.append( wxT("\"") );
        if ( i < (itemCount-1) )
            tempStr.append ( wxT(" \"") );
    }
}

wxArrayInt wxMultiChoicePropertyClass::GetValueAsIndices() const
{
    const wxArrayInt& choiceValues = m_choices.GetValues();
    if ( choiceValues.GetCount() )
    {
        // Translate values to string indices.
        wxArrayInt selections;

        unsigned int i;
        for ( i=0; i<m_value_wxArrayInt.GetCount(); i++ )
        {
            int sIndex = choiceValues.Index(m_value_wxArrayInt[i]);
            if ( sIndex >= 0 )
                selections.Add(sIndex);
        }

        return selections;
    }

    return m_value_wxArrayInt;
}

bool wxMultiChoicePropertyClass::OnEvent( wxPropertyGrid* propgrid,
                                          wxWindow* primary,
                                          wxEvent& event )
{
    if ( event.GetEventType() == wxEVT_COMMAND_BUTTON_CLICKED )
    {
        // Update the value
        PrepareValueForDialogEditing(propgrid);

        // launch editor dialog
        wxMultiChoiceDialog dlg( propgrid,
                                 _("Make a selection:"),
                                 m_label,
                                 m_choices.GetCount(),
                                 &m_choices.GetLabels()[0],
                                 wxCHOICEDLG_STYLE );

        dlg.Move( propgrid->GetGoodEditorDialogPosition(this,dlg.GetSize()) );

        dlg.SetSelections(GetValueAsIndices());

        if ( dlg.ShowModal() == wxID_OK )
        {
            wxArrayInt arrInt = dlg.GetSelections();

            const wxArrayInt& choiceValues = m_choices.GetValues();
            if ( choiceValues.GetCount() )
            {
                // Translate string indices to values.
                wxArrayInt values;

                unsigned int i;
                for ( i=0; i<arrInt.GetCount(); i++ )
                    values.Add(choiceValues.Item(arrInt.Item(i)));

                SetValueI( values );
            }
            else
            {
                SetValueI( arrInt );
            }
            UpdateControl( primary );

            return true;
        }
    }
    return false;
}

int wxMultiChoicePropertyClass::GetChoiceInfo( wxPGChoiceInfo* choiceinfo )
{
    if ( choiceinfo )
    {
        if ( m_choices.IsOk() )
            choiceinfo->m_itemCount = m_choices.GetCount();

        choiceinfo->m_choices = &m_choices;
    }
    return -1;
}

bool wxMultiChoicePropertyClass::SetValueFromString( const wxString& text, int )
{
    m_value_wxArrayInt.Empty();
    const wxArrayString& strings = m_choices.GetLabels();
    const wxArrayInt& values = m_choices.GetValues();

    WX_PG_TOKENIZER2_BEGIN(text,wxT('"'))
        int ind = strings.Index( token );
        if ( ind != wxNOT_FOUND )
        {
            if ( values.GetCount() )
                ind = values.Item(ind);

            m_value_wxArrayInt.Add(ind);
        }
    WX_PG_TOKENIZER2_END()

    GenerateValueAsString();

    return true;
}

#endif // wxUSE_CHOICEDLG


// -----------------------------------------------------------------------
// wxDateProperty
// -----------------------------------------------------------------------

#if wxUSE_DATETIME


#if wxUSE_DATEPICKCTRL
    #define dtCtrl      DatePickerCtrl
#else
    #define dtCtrl      TextCtrl
#endif

WX_PG_IMPLEMENT_PROPERTY_CLASS(wxDateProperty,
                               wxBaseProperty,
                               wxDateTime,
                               const wxDateTime&,
                               dtCtrl)


wxString wxDatePropertyClass::ms_defaultDateFormat;


wxDatePropertyClass::wxDatePropertyClass( const wxString& label,
                                                  const wxString& name,
                                                  const wxDateTime& value )
    : wxPGProperty(label,name)
{
    wxPGRegisterDefaultValueType(wxDateTime)

#if wxUSE_DATEPICKCTRL
    wxPGRegisterEditorClass(DatePickerCtrl);

    m_dpStyle = wxDP_DEFAULT | wxDP_SHOWCENTURY;
#else
    m_dpStyle = 0;
#endif

    DoSetValue( value );
}

wxDatePropertyClass::~wxDatePropertyClass()
{
}

void wxDatePropertyClass::DoSetValue( wxPGVariant value )
{
    m_valueDateTime = wxPGVariantToDateTime(value);
}

wxPGVariant wxDatePropertyClass::DoGetValue() const
{
    return wxPGVariantCreator(m_valueDateTime);
}

bool wxDatePropertyClass::SetValueFromString( const wxString& text,
                                                  int WXUNUSED(argFlags) )
{
    const wxChar* c = m_valueDateTime.ParseFormat(text.c_str(),wxDefaultDateTimeFormat);

    return c ? true : false;
}

wxString wxDatePropertyClass::GetValueAsString( int argFlags ) const
{
    const wxChar* format = (const wxChar*) NULL;

    if ( !m_valueDateTime.IsValid() )
        return wxT("Invalid");

    if ( !ms_defaultDateFormat.length() )
    {
#if wxUSE_DATEPICKCTRL
        bool showCentury = m_dpStyle & wxDP_SHOWCENTURY ? true : false;
#else
        bool showCentury = true;
#endif
        ms_defaultDateFormat = DetermineDefaultDateFormat( showCentury );
    }

    if ( m_format.length() &&
         !(argFlags & wxPG_FULL_VALUE) )
            format = m_format.c_str();

    // Determine default from locale
    // NB: This is really simple stuff, but can't figure anything
    //     better without proper support in wxLocale
    if ( !format )
        format = ms_defaultDateFormat.c_str();

    return m_valueDateTime.Format(format);
}

wxString wxDatePropertyClass::DetermineDefaultDateFormat( bool showCentury )
{
    // This code is basicly copied from datectlg.cpp's SetFormat
    //
    wxString format;

    wxDateTime dt;
    dt.ParseFormat(wxT("2003-10-13"), wxT("%Y-%m-%d"));
    wxString str(dt.Format(wxT("%x")));

    const wxChar *p = str.c_str();
    while ( *p )
    {
        int n=wxAtoi(p);
        if (n == dt.GetDay())
        {
            format.Append(wxT("%d"));
            p += 2;
        }
        else if (n == (int)dt.GetMonth()+1)
        {
            format.Append(wxT("%m"));
            p += 2;
        }
        else if (n == dt.GetYear())
        {
            format.Append(wxT("%Y"));
            p += 4;
        }
        else if (n == (dt.GetYear() % 100))
        {
            if (showCentury)
                format.Append(wxT("%Y"));
            else
                format.Append(wxT("%y"));
            p += 2;
        }
        else
            format.Append(*p++);
    }

    return format;
}

void wxDatePropertyClass::SetAttribute( int id, wxVariant& value )
{
    if ( id == wxPG_DATE_FORMAT )
    {
        m_format = value.GetString();
    }
    else if ( id == wxPG_DATE_PICKER_STYLE )
    {
        m_dpStyle = value.GetLong();
        ms_defaultDateFormat.clear();  // This may need recalculation
    }
}

#endif  // wxUSE_DATETIME


// -----------------------------------------------------------------------
// wxPropertyContainerMethods
// -----------------------------------------------------------------------

void wxPropertyContainerMethods::InitAllTypeHandlers()
{
    wxPG_INIT_REQUIRED_TYPE(wxColour)
    wxPG_INIT_REQUIRED_TYPE(wxFont)
    wxPG_INIT_REQUIRED_TYPE(wxArrayInt)
    wxPG_INIT_REQUIRED_TYPE(wxColourPropertyValue)
#if wxUSE_DATETIME
    wxPGRegisterDefaultValueType(wxDateTime)
#endif
}

// -----------------------------------------------------------------------

void wxPropertyContainerMethods::RegisterAdditionalEditors()
{
#if wxUSE_SPINBTN
    wxPGRegisterEditorClass(SpinCtrl);
#endif
#if wxUSE_DATEPICKCTRL
    wxPGRegisterEditorClass(DatePickerCtrl);
#endif
}

// -----------------------------------------------------------------------

void wxPropertyContainerMethods::RegisterAdvancedPropertyClasses()
{
    wxPGRegisterPropertyClass(wxMultiChoiceProperty);
    wxPGRegisterPropertyClass(wxImageFileProperty);
    wxPGRegisterPropertyClass(wxColourProperty);
    wxPGRegisterPropertyClass(wxFontProperty);
    wxPGRegisterPropertyClass(wxSystemColourProperty);
    wxPGRegisterPropertyClass(wxCursorProperty);
#if wxUSE_DATETIME
    wxPGRegisterPropertyClass(wxDateProperty);
#endif
}

// -----------------------------------------------------------------------

#endif // wxPG_INCLUDE_ADVPROPS
