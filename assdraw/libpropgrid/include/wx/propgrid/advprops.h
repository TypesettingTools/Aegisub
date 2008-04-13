/////////////////////////////////////////////////////////////////////////////
// Name:        advprops.h
// Purpose:     wxPropertyGrid Advanced Properties (font, colour, etc.)
// Author:      Jaakko Salli
// Modified by:
// Created:     Sep-25-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_ADVPROPS_H_
#define _WX_PROPGRID_ADVPROPS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "advprops.cpp"
#endif

// -----------------------------------------------------------------------


//
// Additional Value Type Handlers
//
#ifndef SWIG

WX_PG_DECLARE_VALUE_TYPE_WITH_DECL(wxFont,WXDLLIMPEXP_PG)

WX_PG_DECLARE_VALUE_TYPE_WITH_DECL(wxColour,WXDLLIMPEXP_PG)

WX_PG_DECLARE_VALUE_TYPE_VOIDP_WITH_DECL(wxArrayInt,WXDLLIMPEXP_PG)


#if wxUSE_DATETIME
#include <wx/datetime.h>
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(wxDateTime,WXDLLIMPEXP_PG)
#endif

bool WXDLLIMPEXP_PG operator == (const wxFont&, const wxFont&);

bool WXDLLIMPEXP_PG operator == (const wxArrayInt& array1, const wxArrayInt& array2);

#endif


//
// Additional Property Editors
//
#if wxUSE_SPINBTN
WX_PG_DECLARE_EDITOR_WITH_DECL(SpinCtrl,WXDLLIMPEXP_PG)
#endif

#if wxUSE_DATEPICKCTRL && defined(wxPG_ALLOW_WXADV)
WX_PG_DECLARE_EDITOR_WITH_DECL(DatePickerCtrl,WXDLLIMPEXP_PG)
#endif

// -----------------------------------------------------------------------


// Web colour is currently unsupported
#define wxPG_COLOUR_WEB_BASE        0x10000
//#define wxPG_TO_WEB_COLOUR(A)   ((wxUint32)(A+wxPG_COLOUR_WEB_BASE))


#define wxPG_COLOUR_CUSTOM      0xFFFFFF

/** \class wxColourPropertyValue
    \ingroup classes
    \brief Because text, background and other colours tend to differ between
    platforms, wxSystemColourProperty must be able to select between system
    colour and, when necessary, to pick a custom one. wxSystemColourProperty
    value makes this possible.
*/
class WXDLLIMPEXP_PG wxColourPropertyValue : public wxObject
{
public:
    /** An integer value relating to the colour, and which exact
        meaning depends on the property with which it is used.

        For wxSystemColourProperty:

        Any of wxSYS_COLOUR_XXX, or any web-colour ( use wxPG_TO_WEB_COLOUR
        macro - (currently unsupported) ), or wxPG_COLOUR_CUSTOM.

        For custom colour properties without values array specified:

        index or wxPG_COLOUR_CUSTOM

        For custom colour properties <b>with</b> values array specified:

        m_arrValues[index] or wxPG_COLOUR_CUSTOM
    */
    wxUint32    m_type;

    /** Resulting colour. Should be correct regardless of type. */
    wxColour    m_colour;

    wxColourPropertyValue() { }

    inline void Init( wxUint32 type, const wxColour& colour )
    {
        m_type = type;
        m_colour = colour;
    }

    inline wxColourPropertyValue( const wxColour& colour )
    {
        m_type = wxPG_COLOUR_CUSTOM;
        m_colour = colour;
    }

    inline wxColourPropertyValue( wxUint32 type )
    {
        m_type = type;
    }

    inline wxColourPropertyValue( wxUint32 type, const wxColour& colour )
    {
        Init( type, colour );
    }

#ifndef SWIG
private:
    DECLARE_DYNAMIC_CLASS(wxColourPropertyValue)
#endif
};

#ifndef SWIG
bool WXDLLIMPEXP_PG operator == (const wxColourPropertyValue&, const wxColourPropertyValue&);

WX_PG_DECLARE_VALUE_TYPE_WITH_DECL(wxColourPropertyValue,WXDLLIMPEXP_PG)
#endif

#ifndef SWIG
    #define wxPG_EMPTY_CPV          (*((wxColourPropertyValue*)NULL))
    #define wxPG_NORMAL_FONT        (*wxNORMAL_FONT)
#else
    #define wxPG_EMPTY_CPV          wxCPV_wxPG_EMPTY
    #define wxPG_NORMAL_FONT        wxFONT_wxPG_NORMAL_FONT
#endif


// -----------------------------------------------------------------------
// Declare part of custom colour property macro pairs.

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_WITH_DECL(NAME,DECL) \
extern DECL wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxColourPropertyValue& value = wxPG_EMPTY_CPV ); \
extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY(NAME) \
extern wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxColourPropertyValue& value = wxPG_EMPTY_CPV ); \
extern wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR_WITH_DECL(NAME,DECL) \
extern DECL wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxColour& value = wxPG_COLOUR_BLACK ); \
extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(NAME) \
extern wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxColour& value = wxPG_COLOUR_BLACK ); \
extern wxPGPropertyClassInfo NAME##ClassInfo;

// Declare advanced properties.
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxFontProperty,const wxFont&,wxPG_NORMAL_FONT,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxSystemColourProperty,const wxColourPropertyValue&,wxPG_EMPTY_CPV,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxCursorProperty,int,wxCURSOR_NONE,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxDateProperty,const wxDateTime&,wxDateTime(),WXDLLIMPEXP_PG)

#if wxUSE_IMAGE || defined(SWIG)
#include <wx/image.h>
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxImageFileProperty,const wxString&,wxEmptyString,WXDLLIMPEXP_PG)
#endif

WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR_WITH_DECL(wxColourProperty,WXDLLIMPEXP_PG)

// MultiChoice is trickier.

#ifndef __WXPYTHON__

extern WXDLLIMPEXP_PG wxPGProperty* wxMultiChoiceProperty(const wxString& label,
                                                          const wxString& name,
                                                          const wxArrayString& choices = wxArrayString(),
                                                          const wxArrayInt& value = wxPG_EMPTY_ARRAYINT);

extern WXDLLIMPEXP_PG wxPGProperty* wxMultiChoiceProperty(const wxString& label,
                                                          const wxString& name,
                                                          const wxPGChoices& choices,
                                                          const wxArrayInt& value = wxPG_EMPTY_ARRAYINT);

extern WXDLLIMPEXP_PG wxPGProperty* wxMultiChoiceProperty(const wxString& label,
                                                          const wxString& name,
                                                          const wxArrayInt& value);

#else

extern WXDLLIMPEXP_PG wxPGProperty* wxMultiChoiceProperty(const wxString& label,
                                                          const wxString& name = wxPG_LABEL,
                                                          const wxArrayString& choices = wxArrayString(),
                                                    // This crazyness is needed for Python 2.3 (which uses
                                                    // VC6) compatibility.
                                                    #ifndef SWIG
                                                          const wxArrayInt& value = (*((wxArrayInt*)NULL)));
                                                    #else
                                                          const wxArrayInt& value = wxArrayInt());
                                                    #endif

#endif

// -----------------------------------------------------------------------

//
// Define property classes *only* if propdev.h was included
//
#if defined(_WX_PROPGRID_PROPDEV_H_)

//#ifndef SWIG

// -----------------------------------------------------------------------

class WXDLLIMPEXP_PG wxFontPropertyClass : public wxPGPropertyWithChildren
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxFontPropertyClass( const wxString& label, const wxString& name, const wxFont& value );
    virtual ~wxFontPropertyClass();

    WX_PG_DECLARE_PARENTAL_TYPE_METHODS()
    virtual wxString GetValueAsString( int argFlags = 0 ) const;

    WX_PG_DECLARE_EVENT_METHODS()
    WX_PG_DECLARE_PARENTAL_METHODS()
    //WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

protected:
    wxFont m_value_wxFont;
};

// -----------------------------------------------------------------------


/** If set, then match from list is searched for a custom colour. */
#define wxPG_PROP_TRANSLATE_CUSTOM      wxPG_PROP_CLASS_SPECIFIC_1


class WXDLLIMPEXP_PG wxSystemColourPropertyClass : public wxEnumPropertyClass
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxSystemColourPropertyClass( const wxString& label, const wxString& name,
        const wxColourPropertyValue& value );
    ~wxSystemColourPropertyClass();

    WX_PG_DECLARE_BASIC_TYPE_METHODS()
    WX_PG_DECLARE_EVENT_METHODS()
    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    // Helper function to show the colour dialog
    bool QueryColourFromUser( wxPropertyGrid* propgrid, wxWindow* primary );

    // Default is to use wxSystemSettings::GetColour(index). Override to use
    // custom colour tables etc.
    virtual long GetColour( int index );

protected:

    // Special constructors to be used by derived classes.
    wxSystemColourPropertyClass( const wxString& label, const wxString& name,
        const wxChar** labels, const long* values, wxPGChoices* choicesCache,
        const wxColourPropertyValue& value );
    wxSystemColourPropertyClass( const wxString& label, const wxString& name,
        const wxChar** labels, const long* values, wxPGChoices* choicesCache,
        const wxColour& value );

    void Init ( int type, const wxColour& colour );

    // Translates colour to a int value, return wxNOT_FOUND if no match.
    int ColToInd ( const wxColour& colour );

    wxColourPropertyValue   m_value;
};

// -----------------------------------------------------------------------

#ifndef SWIG

class WXDLLIMPEXP_PG wxCursorPropertyClass : public wxEnumPropertyClass
{
    WX_PG_DECLARE_DERIVED_PROPERTY_CLASS()
public:

    wxCursorPropertyClass( const wxString& label, const wxString& name, int value );
    virtual ~wxCursorPropertyClass();

    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()
};

#endif

// -----------------------------------------------------------------------

#if wxUSE_IMAGE || defined(SWIG)

WXDLLIMPEXP_PG const wxString& wxPGGetDefaultImageWildcard();

class WXDLLIMPEXP_PG wxImageFilePropertyClass : public wxFilePropertyClass
{
    WX_PG_DECLARE_DERIVED_PROPERTY_CLASS()
public:

    wxImageFilePropertyClass( const wxString& label, const wxString& name, const wxString& value );
    virtual ~wxImageFilePropertyClass ();

    virtual void DoSetValue ( wxPGVariant value );
    WX_PG_DECLARE_CUSTOM_PAINT_METHODS()

protected:
    wxBitmap*   m_pBitmap; // final thumbnail area
    wxImage*    m_pImage; // intermediate thumbnail area

};

#endif

#if wxUSE_CHOICEDLG && !defined(SWIG) //|| defined(SWIG)

class WXDLLIMPEXP_PG wxMultiChoicePropertyClass : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxMultiChoicePropertyClass( const wxString& label,
                                const wxString& name,
                                const wxArrayString& strings,
                                const wxArrayInt& value );
    wxMultiChoicePropertyClass( const wxString& label,
                                const wxString& name = wxPG_LABEL,
                                const wxArrayInt& value = wxArrayInt() );
    wxMultiChoicePropertyClass( const wxString& label,
                                const wxString& name,
                                const wxPGChoices& choices,
                                const wxArrayInt& value = wxArrayInt() );
    virtual ~wxMultiChoicePropertyClass();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int flags = 0 ) const;
    virtual bool SetValueFromString( const wxString& text, int flags );
    WX_PG_DECLARE_EVENT_METHODS()

    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

protected:

    void SetValueI( const wxArrayInt& arr );  // I stands for internal
    void GenerateValueAsString();

    // Returns translation of values into string indices.
    wxArrayInt GetValueAsIndices() const;

    wxPGChoices         m_choices; // Holds strings (any values given are ignored).
    wxArrayInt          m_value_wxArrayInt;  // Actual value.

    wxString            m_display; // Cache displayed text since generating it is relatively complicated.
};

#endif // wxUSE_CHOICEDLG

// -----------------------------------------------------------------------

#if wxUSE_DATETIME && !defined(SWIG)

class WXDLLIMPEXP_PG wxDatePropertyClass : public wxPGProperty
{
    WX_PG_DECLARE_PROPERTY_CLASS()
public:

    wxDatePropertyClass( const wxString& label, const wxString& name, const wxDateTime& value );
    virtual ~wxDatePropertyClass();

    virtual void DoSetValue( wxPGVariant value );
    virtual wxPGVariant DoGetValue() const;
    virtual wxString GetValueAsString( int flags = 0 ) const;
    virtual bool SetValueFromString( const wxString& text, int flags );

    //WX_PG_DECLARE_EVENT_METHODS()
    WX_PG_DECLARE_ATTRIBUTE_METHODS()

    inline void SetFormat( const wxString& format )
    {
        m_format = format;
    }

    inline const wxString& GetFormat() const
    {
        return m_format;
    }
    
    inline void SetDateValue( const wxDateTime& dt )
    {
        m_valueDateTime = dt;
    }

    inline const wxDateTime& GetDateValue() const
    {
        return m_valueDateTime;
    }

    inline long GetDatePickerStyle() const
    {
        return m_dpStyle;
    }

protected:
    wxDateTime      m_valueDateTime;
    wxString        m_format;
    long            m_dpStyle;  // DatePicker style

    static wxString ms_defaultDateFormat;
    static wxString DetermineDefaultDateFormat( bool showCentury );
};

#endif

// -----------------------------------------------------------------------

//#endif // #ifndef SWIG

#endif // _WX_PROPGRID_PROPDEV_H_

#endif // _WX_PROPGRID_ADVPROPS_H_
