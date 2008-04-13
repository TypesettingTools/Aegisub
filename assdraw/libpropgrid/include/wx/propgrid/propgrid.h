/////////////////////////////////////////////////////////////////////////////
// Name:        propgrid.h
// Purpose:     wxPropertyGrid
// Author:      Jaakko Salli
// Modified by:
// Created:     Sep-25-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PROPGRID_PROPGRID_H__
#define __WX_PROPGRID_PROPGRID_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "propgrid.cpp"
#endif

#include <wx/window.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/scrolwin.h>
#include <wx/dynarray.h>
#include <wx/hashmap.h>
#include <wx/variant.h>
#include <wx/longlong.h>
#include <wx/tooltip.h>
#include <wx/datetime.h>

// NB: Do *NOT * remove this.
#if defined(SWIG) || defined(SWIG_TYPE_TABLE)
    #ifndef __WXPYTHON__
        #define __WXPYTHON__
    #endif
#endif

//
// In case DOXYGEN was not specified...
#if !defined(DOXYGEN) && !defined(_WX_WINDOW_H_BASE_) && !defined(SWIG)
    // I've tried to get this auto-defined in Doxygen config, but have not
    // succeeded thus far... so using a hack here.
    #define DOXYGEN
#endif


// Doxygen special includes
#ifdef DOXYGEN
    #include "pg_dox_mainpage.h"
    #include "propdev.h"
#endif


//
// Need to define some things for DOXYGEN
#ifdef DOXYGEN
    #define wxUSE_VALIDATORS        1
    #define wxUSE_DATETIME          1
    #define wxUSE_TOOLTIPS          1
    #define wxUSE_SPINBTN           1
    #define wxUSE_DATEPICKCTRL      1
#endif


//
// Check some wxUSE_FOOs
#ifndef wxUSE_VALIDATORS
    #error "wxUSE_VALIDATORS not defined"
#endif

#ifndef wxUSE_DATETIME
    #error "wxUSE_DATETIME not defined"
#endif

#ifndef wxUSE_DATEPICKCTRL
    #error "wxUSE_DATEPICKCTRL not defined"
#endif


// Uncomment next line to allow wxAdv linkage (required for DatePickerCtrl editor)
//    #define wxPG_ALLOW_WXADV

#if defined(__WXPYTHON__)
    #include <Python.h>

    #undef wxPG_ALLOW_WXADV
    #define wxPG_ALLOW_WXADV
#endif

// Defines for component version.
// The following symbols should be updated for each new component release
// since some kind of tests, like those of AM_WXCODE_CHECKFOR_COMPONENT_VERSION()
// for "configure" scripts under unix, use them.
#define wxPROPGRID_MAJOR          1
#define wxPROPGRID_MINOR          2
#define wxPROPGRID_RELEASE        11

// For non-Unix systems (i.e. when building without a configure script),
// users of this component can use the following macro to check if the
// current version is at least major.minor.release
#define wxCHECK_PROPGRID_VERSION(major,minor,release) \
    (wxPROPGRID_MAJOR > (major) || \
    (wxPROPGRID_MAJOR == (major) && wxPROPGRID_MINOR > (minor)) || \
    (wxPROPGRID_MAJOR == (major) && wxPROPGRID_MINOR == (minor) && wxPROPGRID_RELEASE >= (release)))


// Legacy version number
#define wxPG_VERSION        ((wxPROPGRID_MAJOR*1000)+(wxPROPGRID_MINOR*100)+(wxPROPGRID_RELEASE*10))


// -----------------------------------------------------------------------


//
// Here are some platform dependent defines
// NOTE: More in propertygrid.cpp
//

#ifndef SWIG

#if defined(__WXMSW__)
    // tested

    #define wxPG_XBEFORETEXT            5 // space between vertical line and value text
    #define wxPG_XBEFOREWIDGET          1 // space between vertical line and value editor control

    #define wxPG_TEXTCTRLXADJUST        3 // x position adjustment for wxTextCtrl (and like)

    #define wxPG_ICON_WIDTH             9 // comment to use bitmap buttons
    #define wxPG_USE_RENDERER_NATIVE    0 // 1 if wxRendererNative should be employed

    #define wxPG_DOUBLE_BUFFER          1 // 1 to use double-buffer that guarantees flicker-free painting

    #define wxPG_HEAVY_GFX              1 // 1 for prettier appearance

    #define wxPG_SUPPORT_TOOLTIPS       1  // Enable tooltips

#elif defined(__WXGTK__)
    // tested

    #define wxPG_XBEFORETEXT            6 // space between vertical line and value text
    #define wxPG_XBEFOREWIDGET          1 // space between vertical line and value editor control

    #define wxPG_TEXTCTRLXADJUST        3 // x position adjustment for wxTextCtrl (and like)

    #define wxPG_ICON_WIDTH             9 // comment to use bitmap buttons
    #define wxPG_USE_RENDERER_NATIVE    0 // 1 if wxRendererNative should be employed

    #define wxPG_DOUBLE_BUFFER          1 // 1 to use double-buffer that guarantees flicker-free painting

    #define wxPG_HEAVY_GFX              1 // 1 for prettier appearance

    #define wxPG_SUPPORT_TOOLTIPS       1  // Enable tooltips

#elif defined(__WXMAC__)
    // *not* tested

    #define wxPG_XBEFORETEXT            5 // space between vertical line and value text
    #define wxPG_XBEFOREWIDGET          1 // space between vertical line and value editor widget

    #define wxPG_TEXTCTRLXADJUST        3 // x position adjustment for wxTextCtrl (and like)

    #define wxPG_ICON_WIDTH             11  // comment to use bitmap buttons
    #define wxPG_USE_RENDERER_NATIVE    1 // 1 if wxRendererNative should be employed

    #define wxPG_DOUBLE_BUFFER          0 // 1 to use double-buffer that guarantees flicker-free painting

    #define wxPG_HEAVY_GFX              1 // 1 for prettier appearance

    #define wxPG_SUPPORT_TOOLTIPS       1  // Enable tooltips

#else
    // defaults
    // tested on: none.

    #define wxPG_XBEFORETEXT            6 // space between vertical line and value text
    #define wxPG_XBEFOREWIDGET          1 // space between vertical line and value editor widget

    #define wxPG_TEXTCTRLXADJUST        3 // x position adjustment for wxTextCtrl (and like)

    #define wxPG_ICON_WIDTH             9 // comment to use bitmap buttons
    #define wxPG_USE_RENDERER_NATIVE    0 // 1 if wxRendererNative should be employed

    #define wxPG_DOUBLE_BUFFER          0 // 1 to use double-buffer that guarantees flicker-free painting

    #define wxPG_HEAVY_GFX              0 // 1 for prettier appearance

    #define wxPG_SUPPORT_TOOLTIPS       0  // Enable tooltips

#endif // #if defined(__WXMSW__)


#if defined(__WXWINCE__)
    #define wxPG_SMALL_SCREEN       1
    #undef wxPG_DOUBLE_BUFFER
    #define wxPG_DOUBLE_BUFFER      0
#else
    #define wxPG_SMALL_SCREEN       0
#endif

#endif // #ifndef SWIG

// Undefine wxPG_ICON_WIDTH to use supplied xpm bitmaps instead
// (for tree buttons)
//#undef wxPG_ICON_WIDTH

// Need to force disable tooltips?
#if !wxUSE_TOOLTIPS
    #undef wxPG_SUPPORT_TOOLTIPS
    #define wxPG_SUPPORT_TOOLTIPS       0
#endif


// Set 1 to include basic properties ( ~48k in 0.9.9.2 )
#define wxPG_INCLUDE_BASICPROPS         1

// Set 1 to include advanced properties (wxFontProperty, wxColourProperty, etc.) ( ~32k in 0.9.9.2 )
#define wxPG_INCLUDE_ADVPROPS           1

// Set 1 include wxPropertyGridManager ( ~36k in 0.9.9.1 )
//#define wxPG_INCLUDE_MANAGER            1

// Set 1 to include checkbox editor class ( ~4k in 0.9.9.1 )
#define wxPG_INCLUDE_CHECKBOX           1

// 1 to allow user data for each property
#define wxPG_USE_CLIENT_DATA            1

// NOTE: This is not supported as 1.
#define wxPG_INCLUDE_WXOBJECT           0

// -----------------------------------------------------------------------

#ifdef wxPG_COMPATIBILITY_1_0_0

    #define wxRECURSE               wxPG_RECURSE
    #define wxKEEP_STRUCTURE        wxPG_KEEP_STRUCTURE
    #define wxPGConstants           wxPGChoices
    #define wxPG_EX_CLASSIC_SPACING 0

    #define wxPGCtrlClass           wxWindow
    #define wxCCustomTextCtrl       wxTextCtrl
    #define wxCCustomComboBox       wxPGOwnerDrawnComboBox
    #define wxCCustomButton         wxButton

#endif // #ifdef wxPG_COMPATIBILITY_1_0_0

#ifdef __WXPYTHON__
    #define wxPG_PGVARIANT_IS_VARIANT   1  // 1
    #define wxPG_VALUETYPE_IS_STRING    0  // 1
#else
    #define wxPG_PGVARIANT_IS_VARIANT   0
    #define wxPG_VALUETYPE_IS_STRING    0
#endif


#ifndef SWIG
    #if !wxCHECK_VERSION(2,9,0)
        #if !defined(wxUniChar)
            #define wxUniChar   wxChar
        #endif
        //#define wxPGGetIterChar(str, i)  str[i]
        #define wxPGGetIterChar(str, i)  *i
    #else
        #define wxPGGetIterChar(str, i)  *i
    #endif
#endif


// -----------------------------------------------------------------------

//
// wxPython special considerations
//
// TODO: Using wxPG_GETVALUE_CONST yields some ugly function
//   names, so might as well make those GetPropertyValueAsXXX non-static
//   for regular C++ build as well.s
//

#if !wxPG_VALUETYPE_IS_STRING
    #define wxPG_VALUETYPE_MSGVAL       const wxPGValueType*
#else
    #define wxPG_VALUETYPE_MSGVAL       wxString
#endif


#ifndef __WXPYTHON__

// Some Strings are returned as const wxChar* in C++, and as wxString in wxPython
// (using just wxString for everything would've been better, but the current scheme
// is necessary for better backwards compatibility).
#define wxPG_CONST_WXCHAR_PTR       const wxChar*
#define wxPG_CONST_WXCHAR_DEFVAL    ((const wxChar*)NULL)
#define wxPG_TO_WXCHAR_PTR(A)       A

#define wxPG_PYTHON_STATIC    static
#define wxPG_GETVALUE_CONST

// In C++ we can stick with overloaded methods
#define SetPropertyValueLong        SetPropertyValue
#define SetPropertyValueDouble      SetPropertyValue
#define SetPropertyValueBool        SetPropertyValue
#define SetPropertyValueString      SetPropertyValue
#define SetPropertyValueWxObjectPtr SetPropertyValue
#define SetPropertyValuePoint       SetPropertyValue
#define SetPropertyValueSize        SetPropertyValue
#define SetPropertyValueArrint2     SetPropertyValue
#define SetPropertyValueArrstr2     SetPropertyValue
#define SetPropertyValueDatetime    SetPropertyValue
#define SetPropertyValueLongLong    SetPropertyValue
#define SetPropertyValueULongLong   SetPropertyValue

#else

// Some Strings are returned as const wxChar* in C++, and as wxString in wxPython
// (using just wxString for everything would've been better, but the current scheme
// is necessary for better backwards compatibility).
#define wxPG_CONST_WXCHAR_PTR       wxString
#define wxPG_CONST_WXCHAR_DEFVAL    wxEmptyString
#define wxPG_TO_WXCHAR_PTR(A)       (A.c_str())

// Declaring GetValues as static will yield problems
#define wxPG_PYTHON_STATIC
#define wxPG_GETVALUE_CONST   const

// Because SWIG has problems combining overloaded functions and
// Python object-to-wxXXX conversion, we need to use Python proxy
// functions for these value setters.
#define SetPropertyValueArrstr2     _SetPropertyValueArrstr
#define SetPropertyValueArrint2     _SetPropertyValueArrint

#endif


// wxPG_CHECK_FOO_DBG - on Release and wxPython builds, show wxLogWarning instead
// (so that the program flow is not interrupted, but the message can still be seen).
#if !defined(__WXDEBUG__) || defined(__WXPYTHON__)
    #define wxPG_CHECK_RET_DBG(A,B) \
        if ( !(A) ) { wxLogWarning(B); return; }
    #define wxPG_CHECK_MSG_DBG(A,B,C) \
        if ( !(A) ) { wxLogWarning(C); return B; }
#else
    #define wxPG_CHECK_RET_DBG(A,B)     wxCHECK_RET(A,B)
    #define wxPG_CHECK_MSG_DBG(A,B,C)   wxCHECK_MSG(A,B,C)
#endif


// -----------------------------------------------------------------------

// Our very custom dynamic object macros. Should only be used
// directly in an abstract (typeless etc.) base property classes.
#if wxPG_INCLUDE_WXOBJECT
    #error "wxPG_INCLUDE_WXOBJECT is not currently supported (as of 1.0.0b)."
    #define WX_PG_DECLARE_GETCLASSNAME
    #define WX_PG_DECLARE_GETCLASSINFO
#else
    #define WX_PG_DECLARE_GETCLASSNAME   virtual wxPG_CONST_WXCHAR_PTR GetClassName() const;
    #define WX_PG_DECLARE_GETCLASSINFO   virtual const wxPGPropertyClassInfo* GetClassInfo() const;
#endif

// -----------------------------------------------------------------------

#ifdef WXMAKINGLIB_PROPGRID
    #define WXDLLIMPEXP_PG
#elif defined(WXMAKINGDLL_PROPGRID)
    #define WXDLLIMPEXP_PG WXEXPORT
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_PG WXIMPORT
#else // not making nor using DLL
    #define WXDLLIMPEXP_PG
#endif

// -----------------------------------------------------------------------

#if wxPG_PGVARIANT_IS_VARIANT
    #define wxPGVariant wxVariant
#else
    class WXDLLIMPEXP_PG wxPGVariant;
#endif

#ifndef SWIG
class WXDLLIMPEXP_PG wxPGValueType;
class WXDLLIMPEXP_PG wxPGEditor;
class WXDLLIMPEXP_PG wxPGProperty;
class WXDLLIMPEXP_PG wxPGPropertyWithChildren;
class WXDLLIMPEXP_PG wxPropertyCategoryClass;
class WXDLLIMPEXP_PG wxPGChoices;
class WXDLLIMPEXP_PG wxPropertyGridState;
class WXDLLIMPEXP_PG wxPropertyContainerMethods;
class WXDLLIMPEXP_PG wxPropertyGrid;
class WXDLLIMPEXP_PG wxPropertyGridEvent;
class WXDLLIMPEXP_PG wxPropertyGridManager;
class WXDLLIMPEXP_PG wxPGOwnerDrawnComboBox;
class WXDLLIMPEXP_PG wxPGCustomComboControl;

struct wxPGPaintData;

extern WXDLLIMPEXP_PG const wxChar *wxPropertyGridNameStr;

#endif // #ifndef SWIG


#ifdef __WXPYTHON__
    class wxPGPyEditor;
#endif // #ifndef __WXPYTHON__


/** @defgroup miscellaneous wxPropertyGrid Miscellanous
    This section describes some miscellanous values, types and macros.
    @{
*/

#if wxPG_PGVARIANT_IS_VARIANT
    #define wxPG_EMPTY_ARRAYINT     wxArrayInt()
    #define wxPG_EMPTY_ARRAYSTRING  wxArrayString()
#elif !defined(SWIG)
    #define wxPG_EMPTY_ARRAYINT     (*((wxArrayInt*)NULL))
    #define wxPG_EMPTY_ARRAYSTRING  (*((wxArrayString*)NULL))
#else
    #define wxPG_EMPTY_ARRAYINT     wxArrayInt_wxPG_EMPTY
    #define wxPG_EMPTY_ARRAYSTRING  wxArrayString_wxPG_EMPTY
#endif

#if !defined(SWIG)
    #define wxPG_LABEL              (*((wxString*)NULL))  // Used to tell wxPGProperty to use label as name as well.
    #define wxPG_NULL_BITMAP        wxNullBitmap
    #define wxPG_COLOUR_BLACK       (*wxBLACK)
#else
    #define wxPG_LABEL              wxString_wxPG_LABEL
    #define wxPG_NULL_BITMAP        wxBitmap_NULL
    #define wxPG_COLOUR_BLACK       wxColour_BLACK
#endif // #ifndef SWIG


// Used to indicate wxPGChoices::Add etc that the value is actually not given
// by the caller.
#define wxPG_INVALID_VALUE      INT_MAX


/** Convert Red, Green and Blue to a single 32-bit value.
*/
#define wxPG_COLOUR(R,G,B) ((wxUint32)(R+(G<<8)+(B<<16)))

/** Return this in GetImageSize() to indicate that the custom painted
    property image is flexible. That is, it will paint (dropdown)
    list item images with PREFWID,PREFHEI size.
*/
#define wxPG_FLEXIBLE_SIZE(PREFWID,PREFHEI) wxSize(-(PREFWID),-(PREFHEI))

#define wxPG_FULL_CUSTOM_PAINT_WIDTH        -99999

/** Return this in GetImageSize() to indicate that the property is
    custom painted completely (ie. the text as well).
*/
#define wxPG_FULL_CUSTOM_PAINT_SIZE(HEI)                \
    wxSize(wxPG_FULL_CUSTOM_PAINT_WIDTH,HEI)

/** Return this in GetImageSize() to indicate that the property is
    custom painted completely (ie. the text as well), and with flexible
    height.
*/
#define wxPG_FULL_CUSTOM_PAINT_FLEXIBLE_SIZE(PREFHEI)    \
    wxSize(wxPG_FULL_CUSTOM_PAINT_WIDTH,-(PREFHEI))


/** This callback function is used by atleast wxCustomProperty
    to facilitiate easy custom action on button press.
    \param propGrid
    related wxPropertyGrid
    \param property
    related wxPGProperty
    \param ctrl
    If not NULL (for example, not selected), a wxWindow* or equivalent
    \param data
    Value depends on the context.
    \retval
    True if changed value of the property.
*/
typedef bool (*wxPropertyGridCallback)(wxPropertyGrid* propGrid,
                                       wxPGProperty* property,
                                       wxWindow* ctrl,
                                       int data);

/** This callback function is used by atleast wxCustomProperty
    to facilitiate drawing items in drop down list.

    Works very much like wxPGProperty::OnCustomPaint.
*/
typedef void (*wxPGPaintCallback)(wxPGProperty* property,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  wxPGPaintData& paintdata);

/** Use this with wxPropertyGrid::IsPropertyKindOf. For example, as in
    \code
        pg->IsPropertyKindOf(WX_PG_CLASSINFO(wxStringProperty))
    \endcode
*/
#define WX_PG_CLASSINFO(NAME) NAME##ClassInfo

/** @}
*/

// -----------------------------------------------------------------------

/** @defgroup wndflags wxPropertyGrid Window Styles
    SetWindowStyleFlag method can be used to modify some of these at run-time.
    @{
*/
/** This will cause Sort() automatically after an item is added.
    When inserting a lot of items in this mode, it may make sense to
    use Freeze() before operations and Thaw() afterwards to increase
    performance.
*/
#define wxPG_AUTO_SORT              0x00000010

/** Categories are not initially shown (even if added).
    IMPORTANT NOTE: If you do not plan to use categories, then this
    style will waste resources.
    This flag can also be changed using wxPropertyGrid::EnableCategories method.
*/
#define wxPG_HIDE_CATEGORIES        0x00000020

/* This style combines non-categoric mode and automatic sorting.
*/
#define wxPG_ALPHABETIC_MODE        (wxPG_HIDE_CATEGORIES|wxPG_AUTO_SORT)

/** Modified values are shown in bold font. Changing this requires Refresh()
    to show changes.
*/
#define wxPG_BOLD_MODIFIED          0x00000040

/** When wxPropertyGrid is resized, splitter moves to the center. This
    behaviour stops once the user manually moves the splitter.
*/
#define wxPG_SPLITTER_AUTO_CENTER   0x00000080

/** Display tooltips for cell text that cannot be shown completely. If
    wxUSE_TOOLTIPS is 0, then this doesn't have any effect.
*/
#define wxPG_TOOLTIPS               0x00000100

/** Disables margin and hides all expand/collapse buttons that would appear
    outside the margin (for sub-properties). Toggling this style automatically
    expands all collapsed items.
*/
#define wxPG_HIDE_MARGIN            0x00000200

/** This style prevents user from moving the splitter.
*/
#define wxPG_STATIC_SPLITTER        0x00000400

/** Combination of other styles that make it impossible for user to modify
    the layout.
*/
#define wxPG_STATIC_LAYOUT          (wxPG_HIDE_MARGIN|wxPG_STATIC_SPLITTER)

/** Disables wxTextCtrl based editors for properties which
    can be edited in another way. Equals calling wxPropertyGrid::LimitPropertyEditing
    for all added properties.
*/
#define wxPG_LIMITED_EDITING        0x00000800

#ifdef DOXYGEN

/** wxTAB_TRAVERSAL allows using Tab/Shift-Tab to travel between properties
    in grid. Travelling forwards from last property will navigate to the
    next control, and backwards from first will navigate to the previous one.
*/
    #define wxTAB_TRAVERSAL         0x00080000

#endif

/** wxPropertyGridManager only: Show toolbar for mode and page selection. */
#define wxPG_TOOLBAR                0x00001000

/** wxPropertyGridManager only: Show adjustable text box showing description
    or help text, if available, for currently selected property.
*/
#define wxPG_DESCRIPTION            0x00002000

/** wxPropertyGridManager only: Show compactor button that toggles hidden
    state of low-priority properties.
*/
#define wxPG_COMPACTOR              0x00004000

/**
    NOTE: wxPG_EX_xxx are extra window styles and must be set using SetExtraStyle()
    member function.

    Speeds up switching to wxPG_HIDE_CATEGORIES mode. Initially, if wxPG_HIDE_CATEGORIES
    is not defined, the non-categorized data storage is not activated, and switching
    the mode first time becomes somewhat slower. wxPG_EX_INIT_NOCAT activates the
    non-categorized data storage right away. IMPORTANT NOTE: If you do plan not
    switching to non-categoric mode, or if you don't plan to use categories at
    all, then using this style will result in waste of resources.

*/
#define wxPG_EX_INIT_NOCAT          0x00001000

/** Extended window style that sets wxPropertyGridManager toolbar to not
    use flat style.
*/
#define wxPG_EX_NO_FLAT_TOOLBAR     0x00002000

/** This extra style allows editing more similar to some Microsoft/Mono
    provided property sheet controls. Currently this includes (but more may be
    added later, incase I missed something):
    * Pressing ENTER in control, in addition to confirming changes, will
      unfocus it.
    * Pressing ESC doesn't cancel edit (but still unfocuses the editor).

    Note that ESC and ENTER events in editor controls are relayed to the
    wxPropertyGrid itself, so that they can be detected by the application.
*/
//#define wxPG_EX_ALTERNATE_KEYS      0x00004000

/** Shows alphabetic/categoric mode buttons from toolbar.
*/
#define wxPG_EX_MODE_BUTTONS        0x00008000

/** Show property help strings as tool tips instead as text on the status bar.
    You can set the help strings using SetPropertyHelpString member function.
*/
#define wxPG_EX_HELP_AS_TOOLTIPS    0x00010000

/** Prevent TAB from focusing to wxButtons. This behaviour was default
    in version 1.2.0 and earlier.
    NOTE! Tabbing to button doesn't work yet. Problem seems to be that on wxMSW
      atleast the button doesn't properly propagate key events (yes, I'm using
      wxWANTS_CHARS).
*/
//#define wxPG_EX_NO_TAB_TO_BUTTON    0x00020000

/** Set this style to have labels of disabled properties become greyed
    along with the values.
*/
#define wxPG_EX_GREY_LABEL_WHEN_DISABLED    0x00040000


/** Allows relying on native double-buffering.
*/
#define wxPG_EX_NATIVE_DOUBLE_BUFFERING     0x00080000


/** Process all events immediately, if possible. That is, ProcessEvent is
    called instead of AddPendingEvent.
*/
#define wxPG_EX_PROCESS_EVENTS_IMMEDIATELY  0x00100000


/** Set this style to let user have ability to set values of properties to
    unspecified state. Currently, this applies to following properties:
    - wxIntProperty, wxUIntProperty, and wxFloatProperty: Clear the
      text field.
*/
#define wxPG_EX_AUTO_UNSPECIFIED_VALUES     0x00200000


/** Combines various styles.
*/
#define wxPG_DEFAULT_STYLE	        (0)

/** Combines various styles.
*/
#define wxPGMAN_DEFAULT_STYLE	    (0)

/** @}
*/

/** Flags for wxPropertyGrid::GetPropertyValues and wxPropertyGridManager::GetPropertyValues. */
#define wxPG_KEEP_STRUCTURE               0x00000010

/** Flags for wxPropertyGrid::SetPropertyAttribute etc */
#define wxPG_RECURSE                      0x00000020
#define wxPG_RECURSE_STARTS               0x00000040
#define wxPG_FORCE                        0x00000080

// -----------------------------------------------------------------------

// Property priorities
#define wxPG_LOW                    1
#define wxPG_HIGH                   2

// -----------------------------------------------------------------------

// Misc argument flags.
#define wxPG_FULL_VALUE             0x00000001 // Get/Store full value instead of displayed value.
#define wxPG_REPORT_ERROR           0x00000002
#define wxPG_PROPERTY_SPECIFIC      0x00000004
#define wxPG_EDITABLE_VALUE         0x00000008 // Get/Store value that must be editable in wxTextCtrl

// -----------------------------------------------------------------------


#if defined(__WXPYTHON__)
    #define wxPG_ID_IS_PTR      1
#else
    #define wxPG_ID_IS_PTR      0
#endif


#if wxPG_ID_IS_PTR

#define wxNullProperty  ((wxPGProperty*)NULL)
#define wxPGId          wxPGProperty*
#define wxPGIdGen(PTR)  PTR
#define wxPGIdToPtr(ID) ((wxPGProperty*)ID)
#define wxPGIdIsOk(ID)  ( ID != ((wxPGProperty*)NULL) )

#else

#define wxNullProperty wxPGId(NULL)


/** \class wxPGId
    \ingroup classes
    \brief
    Simple wrapper for the wxPGProperty pointer.

    NB: This class exists because:
        - Abstract wxPGId would allow both flexibility and speed
          (for possible native'ish implementations, altough this doesn't make
          sense anymore).
        - wxPG methods should be mostly used for property manipulation
          (or such vision I had at first), and since wxPGId id = pg->Append(...)
          is faster tow write, it seemed useful.

    *However* in future I may just start using wxPG_ID_IS_PTR by the default.
    It might even result into slightly smaller code (altough I have checked out
    some MSVC generated assembly, and it seems to optimize out the wrapper in
    usual scenarios).
*/
class WXDLLIMPEXP_PG wxPGId
{
public:
    inline wxPGId() { m_ptr = (wxPGProperty*) NULL; }
    ~wxPGId() {}

    bool IsOk() const { return ( m_ptr != NULL ); }

    bool operator == (const wxPGId& other)
    {
        return m_ptr == other.m_ptr;
    }

    inline const wxString& GetName() const;

#ifndef SWIG
    inline wxPGId( wxPGProperty* ptr ) { m_ptr = ptr; }
    inline wxPGId( wxPGProperty& ref ) { m_ptr = &ref; }

    operator wxPGProperty* ()
    {
        return m_ptr;
    }
    wxPGProperty* GetPropertyPtr() const { return m_ptr; }
#endif // #ifndef SWIG

    wxPGProperty& GetProperty() const { return *m_ptr; }
private:
    wxPGProperty* m_ptr;
};

#define wxPGIdGen(PTR) wxPGId(PTR)
#define wxPGIdToPtr(ID) ID.GetPropertyPtr()
#define wxPGIdIsOk(ID) ID.IsOk()

#endif // wxPG_ID_IS_PTR


// -----------------------------------------------------------------------

WXDLLIMPEXP_PG void wxPGTypeOperationFailed ( const wxPGProperty* p, const wxChar* typestr, const wxChar* op );
WXDLLIMPEXP_PG void wxPGGetFailed ( const wxPGProperty* p, const wxChar* typestr );

// -----------------------------------------------------------------------

/** @defgroup propflags wxPGProperty Flags
    @{
*/

// NOTE: Do not change order of these, and if you add
//   any, remember also to update gs_property_flag_to_string
//   in propgrid.cpp.

/** Indicates bold font.
*/
#define wxPG_PROP_MODIFIED          0x0001

/** Disables ('greyed' text and editor does not activate) property.
*/
#define wxPG_PROP_DISABLED          0x0002

/** Hider button will hide this property.
*/
#define wxPG_PROP_HIDEABLE          0x0004

/** This property has custom paint image just in front of its value.
    If property only draws custom images into a popup list, then this
    flag should not be set.
*/
#define wxPG_PROP_CUSTOMIMAGE       0x0008

/** Do not create text based editor for this property (but button-triggered
    dialog and choice are ok).
*/
#define wxPG_PROP_NOEDITOR          0x0010

/** Value is unspecified.
*/
#define wxPG_PROP_UNSPECIFIED       0x0020

/** Indicates the bit useable by derived properties.
*/
#define wxPG_PROP_CLASS_SPECIFIC_1  0x0040

/** Indicates the bit useable by derived properties.
*/
#define wxPG_PROP_CLASS_SPECIFIC_2  0x0080

/** Property value cannot be modified. However, editor may be created
    so that the value can be easily copied.
*/
#define wxPG_PROP_READONLY          0x0100

/** @}
*/

// Amalgam of flags that should be inherited by sub-properties
#define wxPG_INHERITED_PROPFLAGS        (wxPG_PROP_HIDEABLE|wxPG_PROP_NOEDITOR)


// -----------------------------------------------------------------------

/** @defgroup attrids wxPropertyGrid Property Attribute Identifiers
    wxPropertyGrid::SetPropertyAttribute accepts one of these as
    attrid argument when used with one of the built-in property classes.
    @{
*/

/** wxBoolProperty specific, int, default 0. When 1 sets bool property to
    use checkbox instead of choice.
*/
#define wxPG_BOOL_USE_CHECKBOX              64

/** wxBoolProperty specific, int, default 0. When 1 sets bool property value
    to cycle on double click (instead of showing the popup listbox).
*/
#define wxPG_BOOL_USE_DOUBLE_CLICK_CYCLING  65

/** wxFloatProperty (and similar) specific, int, default -1. Sets the (max) precision
    used when floating point value is rendered as text. The default -1 means infinite
    precision.
*/
#define wxPG_FLOAT_PRECISION                66

/** The text will be echoed as asterisks (wxTE_PASSWORD will be passed to textctrl etc).
*/
#define wxPG_STRING_PASSWORD                67

/** Define base used by a wxUIntProperty. Valid constants are
    wxPG_BASE_OCT, wxPG_BASE_DEC, wxPG_BASE_HEX and wxPG_BASE_HEXL
    (lowercase characters).
*/
#define wxPG_UINT_BASE                      68

/** Define prefix rendered to wxUIntProperty. Accepted constants
    wxPG_PREFIX_NONE, wxPG_PREFIX_0x, and wxPG_PREFIX_DOLLAR_SIGN.
    <b>Note:</b> Only wxPG_PREFIX_NONE works with Decimal and Octal
    numbers.
*/
#define wxPG_UINT_PREFIX                    69

/** wxFileProperty/wxImageFileProperty specific, wxChar*, default is detected/varies.
    Sets the wildcard used in the triggered wxFileDialog. Format is the
    same.
*/
#define wxPG_FILE_WILDCARD                  70

/** wxFileProperty/wxImageFileProperty specific, int, default 1.
    When 0, only the file name is shown (i.e. drive and directory are hidden).
*/
#define wxPG_FILE_SHOW_FULL_PATH            71

/** Specific to wxFileProperty and derived properties, wxString, default empty.
    If set, then the filename is shown relative to the given path string.
*/
#define wxPG_FILE_SHOW_RELATIVE_PATH        72

/** Specific to wxFileProperty and derived properties, wxString, default is empty.
    Sets the initial path of where to look for files.
*/
#define wxPG_FILE_INITIAL_PATH              73

/** Specific to wxFileProperty and derivatives, wxString, default is empty.
    Sets a specific title for the dir dialog.
*/
#define wxPG_FILE_DIALOG_TITLE              74

/** Specific to wxDirProperty, wxString, default is empty.
    Sets a specific message for the dir dialog.
*/
#define wxPG_DIR_DIALOG_MESSAGE             75

/** Sets displayed date format for wxDateProperty.
*/
#define wxPG_DATE_FORMAT                    76

/** Sets wxDatePickerCtrl window style used with wxDateProperty. Default
    is wxDP_DEFAULT | wxDP_SHOWCENTURY.
*/
#define wxPG_DATE_PICKER_STYLE              77


#ifdef wxPG_COMPATIBILITY_1_0_0

/** wxCustomProperty specific, wxPGEditor*. Set editor control. Editor pointer is stored
    in variable named wxPGEditor_EDITORNAME. So the basic built-in editors are at
    wxPGEditor_TextCtrl, wxPGEditor_Choice, wxPGEditor_ComboBox, wxPGEditor_CheckBox,
    wxPGEditor_TextCtrlAndButton, and wxPGEditor_ChoiceAndButton.
*/
#define wxPG_CUSTOM_EDITOR                  128

/** wxCustomProperty specific, wxBitmap*. Sets a small bitmap. Value must be given as
    pointer and it is then copied. If you give it wxNullBitmap, then the current
    image (if any) is deleted.
*/
#define wxPG_CUSTOM_IMAGE                   129

/** wxCustomProperty specific, void*. Sets callback function (of type wxPropertyGridCallback)
    that is called whenever button is pressed.
*/
#define wxPG_CUSTOM_CALLBACK                130

#endif // wxPG_COMPATIBILITY_1_0_0

/** wxCustomProperty specific, void*. Sets callback function (of type wxPGPaintCallback)
    that is called whenever image in front of property needs to be repainted. This attribute
    takes precedence over bitmap set with wxPG_CUSTOM_IMAGE, and it is only proper way
    to draw images to wxCustomProperty's drop down choices list.
    \remarks
    Callback must handle measure calls (i.e. when rect.x < 0, set paintdata.m_drawnHeight to
    height of item in question).
*/
#define wxPG_CUSTOM_PAINT_CALLBACK          131


/** wxCustomProperty specific, int, default 0. Setting to 1 makes children private, similar to other
    properties with children.
    \remarks
    - Children must be added <b>when this attribute has value 0</b>. Otherwise
      there will be an assertion failure.
    - Changed event occurs on the parent only.
*/
#define wxPG_CUSTOM_PRIVATE_CHILDREN        132


/** wxColourProperty and its kind, int, default 1. Setting this attribute to 0 hides custom
    colour from property's list of choices.
*/
#define wxPG_COLOUR_ALLOW_CUSTOM            151


/** First attribute id that is guaranteed not to be used built-in
    properties.
*/
#define wxPG_USER_ATTRIBUTE                 192

/** @}
*/

//
// Valid constants for wxPG_UINT_BASE attribute
// (long because of wxVariant constructor)
#define wxPG_BASE_OCT                       (long)8
#define wxPG_BASE_DEC                       (long)10
#define wxPG_BASE_HEX                       (long)16
#define wxPG_BASE_HEXL                      (long)32

//
// Valid constants for wxPG_UINT_PREFIX attribute
#define wxPG_PREFIX_NONE                    (long)0
#define wxPG_PREFIX_0x                      (long)1
#define wxPG_PREFIX_DOLLAR_SIGN             (long)2


// -----------------------------------------------------------------------
// Value type.

// Value type declarer, with optional declaration part (with creator function).
#define WX_PG_DECLARE_VALUE_TYPE_WITH_DECL(VALUETYPE,DECL) \
    extern DECL const wxPGValueType *wxPGValueType_##VALUETYPE; \
    extern DECL wxPGValueType* wxPGNewVT##VALUETYPE();

// Value type declarer (with creator function).
#define WX_PG_DECLARE_VALUE_TYPE(VALUETYPE) \
    extern const wxPGValueType *wxPGValueType_##VALUETYPE; \
    wxPGValueType* wxPGNewVT##VALUETYPE();

// Value type declarer, with optional declaration part.
#define WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(VALUETYPE,DECL) \
    extern DECL const wxPGValueType *wxPGValueType_##VALUETYPE;

// Value type accessor.
#if !wxPG_VALUETYPE_IS_STRING
    #define wxPG_VALUETYPE(T)       wxPGValueType_##T
    #define wxPG_VALUETYPE_PTR(T)   wxPGValueType_##T
#else
    #define wxPG_VALUETYPE(T)       wxT(#T)
    #define wxPG_VALUETYPE_PTR(T)   wxPropertyContainerMethods::GetValueType(wxT(#T))
#endif

// Like wxPG_VALUETYPE, but casts pointer to exact class.
#define wxPG_VALUETYPE_EXACT(T) ((wxPGValueType##VALUETYPE##Class)wxPGValueType##T)

// Declare builtin value types.
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(none,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(wxString,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(long,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(bool,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(double,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(void,WXDLLIMPEXP_PG)
WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(wxArrayString,WXDLLIMPEXP_PG)
#ifdef __WXPYTHON__
    WX_PG_DECLARE_VALUE_TYPE_BUILTIN_WITH_DECL(PyObject,WXDLLIMPEXP_PG)
#endif

//
// With wxWidgets 2.9 and later we demand native C++ RTTI support
#if wxCHECK_VERSION(2,9,0)
    #ifdef wxNO_RTTI
        #error "You need to enable compiler RTTI support when using wxWidgets 2.9.0 or later"
    #endif
    #define WX_PG_DECLARE_DYNAMIC_CLASS_VARIANTDATA(A)
    #define WX_PG_IMPLEMENT_DYNAMIC_CLASS_VARIANTDATA(A, B)
#else
    #define WX_PG_DECLARE_DYNAMIC_CLASS_VARIANTDATA DECLARE_DYNAMIC_CLASS
    #define WX_PG_IMPLEMENT_DYNAMIC_CLASS_VARIANTDATA IMPLEMENT_DYNAMIC_CLASS
#endif

// VDC = wxVariantData Class
#define WX_PG_DECLARE_VALUE_TYPE_VDC(VALUETYPE) \
wxVariantData_##VALUETYPE : public wxPGVariantDataWxObj \
{ \
    WX_PG_DECLARE_DYNAMIC_CLASS_VARIANTDATA(wxVariantData_##VALUETYPE) \
protected: \
    VALUETYPE   m_value; \
public: \
    wxVariantData_##VALUETYPE(); \
    wxVariantData_##VALUETYPE(const VALUETYPE& value); \
    virtual void Copy(wxVariantData& data); \
    virtual bool Eq(wxVariantData& data) const; \
    virtual wxString GetType() const; \
    virtual void* GetValuePtr(); \
    inline const VALUETYPE& GetValue () const { return m_value; }

// Value type declarer for void* that need auto-generated .
#define WX_PG_DECLARE_VALUE_TYPE_VOIDP_WITH_DECL(VALUETYPE,DECL) \
class DECL WX_PG_DECLARE_VALUE_TYPE_VDC(VALUETYPE) \
}; \
extern DECL const wxPGValueType *wxPGValueType_##VALUETYPE;

#define WX_PG_DECLARE_VALUE_TYPE_VOIDP(VALUETYPE) \
class WX_PG_DECLARE_VALUE_TYPE_VDC(VALUETYPE) \
}; \
WX_PG_DECLARE_VALUE_TYPE(VALUETYPE)

#ifndef SWIG

/** \class wxPGVariantDataWxObj
    \ingroup classes
    \brief Identical to wxVariantDataWxObjectPtr except that it deletes the
       ptr on destruction.
*/
class WXDLLIMPEXP_PG wxPGVariantDataWxObj : public wxVariantData
{
public:
    wxPGVariantDataWxObj();
    virtual ~wxPGVariantDataWxObj();

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& str) const;
#endif
    virtual bool Write(wxString& str) const;
#if wxUSE_STD_IOSTREAM
    virtual bool Read(wxSTD istream& str);
#endif
    virtual bool Read(wxString& str);

    virtual void* GetValuePtr() = 0;
};

#endif // #ifndef SWIG

#if !wxCHECK_VERSION(2,9,0)
typedef wxList wxVariantList;
#endif

// -----------------------------------------------------------------------
// Editor class.

// Editor accessor.
#define wxPG_EDITOR(T)          wxPGEditor_##T

// Declare editor class, with optional part.
#define WX_PG_DECLARE_EDITOR_WITH_DECL(EDITOR,DECL) \
extern DECL wxPGEditor* wxPGEditor_##EDITOR; \
extern DECL wxPGEditor* wxPGConstruct##EDITOR##EditorClass();

// Declare editor class.
#define WX_PG_DECLARE_EDITOR(EDITOR) \
extern wxPGEditor* wxPGEditor_##EDITOR; \
extern wxPGEditor* wxPGConstruct##EDITOR##EditorClass();

// Declare builtin editor classes.
WX_PG_DECLARE_EDITOR_WITH_DECL(TextCtrl,WXDLLIMPEXP_PG)
WX_PG_DECLARE_EDITOR_WITH_DECL(Choice,WXDLLIMPEXP_PG)
WX_PG_DECLARE_EDITOR_WITH_DECL(ComboBox,WXDLLIMPEXP_PG)
WX_PG_DECLARE_EDITOR_WITH_DECL(TextCtrlAndButton,WXDLLIMPEXP_PG)
#if wxPG_INCLUDE_CHECKBOX
WX_PG_DECLARE_EDITOR_WITH_DECL(CheckBox,WXDLLIMPEXP_PG)
#endif
WX_PG_DECLARE_EDITOR_WITH_DECL(ChoiceAndButton,WXDLLIMPEXP_PG)

// -----------------------------------------------------------------------

/** \class wxPGValueType
	\ingroup classes
    \brief wxPGValueType is base class for property value types.
*/

class WXDLLIMPEXP_PG wxPGValueType
{
public:

    virtual ~wxPGValueType() = 0;

    /** Returns type name. If there is wxVariantData for this type, then name should
    be the same that the class uses (otherwise wxT("void*")). */
    virtual wxPG_CONST_WXCHAR_PTR GetTypeName() const = 0;

    /** Returns custom type name. If this is base for a type, should not be overridden,
        as the default implementation already does good thing and calls GetTypeName.
        Otherwise, should be an unique string, such as the class name etc.
    */
    virtual wxPG_CONST_WXCHAR_PTR GetCustomTypeName() const;

    /** Returns default value.
    */
    virtual wxPGVariant GetDefaultValue() const = 0;

    /** Creates wxVariant with supplied value and name.
    */
    virtual wxVariant GenerateVariant( wxPGVariant value, const wxString& name ) const = 0;

    /** Creates new property instance with "proper" class. Initial value is set
        to default.
    */
    virtual wxPGProperty* GenerateProperty( const wxString& label, const wxString& name ) const = 0;

    /** Sets property value from wxVariant.
    */
    virtual void SetValueFromVariant( wxPGProperty* property, wxVariant& value ) const = 0;

    /** Returns type that can be passed to CreatePropertyByType.
    */
    inline wxPG_CONST_WXCHAR_PTR GetType() const
    {
        return GetCustomTypeName();
    }

protected:
};


// -----------------------------------------------------------------------

// wxVariant definition macro (sans functional eq-operator)
#define WX_PG_DECLARE_VARIANT_DATA(CLASSNAME, DATATYPE, DECL) \
class DECL CLASSNAME : public wxVariantData \
{ \
    DECLARE_DYNAMIC_CLASS(CLASSNAME) \
public: \
    CLASSNAME() { } \
    CLASSNAME(const DATATYPE& value) { m_value = value; } \
    inline DATATYPE GetValue() const { return m_value; } \
    inline const DATATYPE& GetValueRef() const { return m_value; } \
    inline void SetValue(const DATATYPE& value) { m_value = value; } \
    virtual bool Eq(wxVariantData&) const { return false; } \
    virtual wxString GetType() const { return wxT(#DATATYPE); } \
    virtual wxVariantData* Clone() { return new CLASSNAME; } \
    virtual void Copy(wxVariantData &data) { ((CLASSNAME&)data).SetValue(m_value); } \
    virtual bool Read(wxString &) { return false; } \
    virtual bool Write(wxString &) const { return true; } \
protected: \
    DATATYPE m_value; \
};


#define WX_PG_DECLARE_VARIANT_DATA_PTR(CLASSNAME, DATATYPE, DECL) \
class DECL CLASSNAME : public wxVariantData \
{ \
    DECLARE_DYNAMIC_CLASS(CLASSNAME) \
public: \
    CLASSNAME() { } \
    CLASSNAME(DATATYPE* value) { m_value = value; } \
    inline void SetValue(DATATYPE* value) { m_value = value; } \
    inline DATATYPE* GetValue() const { return m_value; } \
    virtual bool Eq(wxVariantData&) const { return false; } \
    virtual wxString GetType() const { return wxT(#DATATYPE); } \
    virtual wxVariantData* Clone() { return new CLASSNAME; } \
    virtual void Copy(wxVariantData &data) { ((CLASSNAME&)data).SetValue(m_value); } \
    virtual bool Read(wxString &) { return false; } \
    virtual bool Write(wxString &) const { return true; } \
protected: \
    DATATYPE* m_value; \
};


#if wxPG_PGVARIANT_IS_VARIANT

    #define wxPGVariantToWxObjectPtr(A,B)   wxDynamicCast(A.GetWxObjectPtr(),B);
    #define wxPGVariantGetWxObjectPtr(A)    A.GetWxObjectPtr()
    #define wxPGVariantToWxObject(A)        (*A.GetWxObjectPtr())
    #define wxPGVariantToDateTime(A)        A.GetDateTime()
    #define wxPGVariantToWxPoint(A)         ((wxPGVariantDataPoint*)(A.GetData()))->GetValueRef()
    #define wxPGVariantToWxSize(A)          ((wxPGVariantDataSize*)(A.GetData()))->GetValueRef()
    #define wxPGVariantToWxLongLong(A)      ((wxPGVariantDataLongLong*)(A.GetData()))->GetValueRef()
    #define wxPGVariantToWxULongLong(A)     ((wxPGVariantDataULongLong*)(A.GetData()))->GetValueRef()
    #define wxPGVariantToArrayInt(A)        ((wxPGVariantDataArrayInt*)(A.GetData()))->GetValueRef()
    #define wxPGVariantToPyObject(A)        ((wxPGVariantDataPyObject*)(A.GetData()))->GetValue()
    #define wxPGVariantFromWxObject(A)      wxVariant((wxObject*)A)
    #define wxPGVariantFromLong(A)          wxVariant(((long)A))

  #if !defined(SWIG)

    WX_PG_DECLARE_VARIANT_DATA(wxPGVariantDataPoint, wxPoint, WXDLLIMPEXP_PG)
    WX_PG_DECLARE_VARIANT_DATA(wxPGVariantDataSize, wxSize, WXDLLIMPEXP_PG)
    WX_PG_DECLARE_VARIANT_DATA(wxPGVariantDataArrayInt, wxArrayInt, WXDLLIMPEXP_PG)
    WX_PG_DECLARE_VARIANT_DATA(wxPGVariantDataLongLong, wxLongLong, WXDLLIMPEXP_PG)
    WX_PG_DECLARE_VARIANT_DATA(wxPGVariantDataULongLong, wxULongLong, WXDLLIMPEXP_PG)
   #ifdef __WXPYTHON__
    WX_PG_DECLARE_VARIANT_DATA_PTR(wxPGVariantDataPyObject, PyObject, WXDLLIMPEXP_PG)
   #endif

    // We need this because wxVariant lacks all necessary constructors
    inline wxVariant wxPGVariantCreator(long a) { return wxVariant((long)a); }
    inline wxVariant wxPGVariantCreator(int a) { return wxVariant((long)a); }
    inline wxVariant wxPGVariantCreator(bool a) { return wxVariant(a); }
    inline wxVariant wxPGVariantCreator(const double& a) { return wxVariant(a); }
    inline wxVariant wxPGVariantCreator(const wxString& a) { return wxVariant(a); }

    // NB: This may look dangerous. However, the wxVariant lives only a very short
    //     time, so it is very unlikely they value will be modified by some
    //     "third party".
    inline wxVariant wxPGVariantCreator(const wxObject* a) { return wxVariant((wxObject*)a); }
    inline wxVariant wxPGVariantCreator(const wxObject& a) { return wxVariant((wxObject*)(&a)); }

    inline wxVariant wxPGVariantCreator(wxObject* a) { return wxVariant(a); }
    inline wxVariant wxPGVariantCreator(wxObject& a) { return wxVariant(&a); }
    inline wxVariant wxPGVariantCreator(void* a) { return wxVariant(a); }
    inline wxVariant wxPGVariantCreator(const wxArrayString& a) { return wxVariant((wxArrayString&)a); }
    inline wxVariant wxPGVariantCreator(const wxArrayInt& a) { return wxVariant(new wxPGVariantDataArrayInt(a)); }
    inline wxVariant wxPGVariantCreator(const wxPoint& a) { return wxVariant(new wxPGVariantDataPoint(a)); }
    inline wxVariant wxPGVariantCreator(const wxSize& a) { return wxVariant(new wxPGVariantDataSize(a)); }
    inline wxVariant wxPGVariantCreator(const wxLongLong& a) { return wxVariant(new wxPGVariantDataLongLong(a)); }
    inline wxVariant wxPGVariantCreator(const wxULongLong& a) { return wxVariant(new wxPGVariantDataULongLong(a)); }
   #ifdef __WXPYTHON__
    inline wxVariant wxPGVariantCreator(PyObject* a) { return wxVariant(new wxPGVariantDataPyObject(a)); }
   #endif
   #if wxUSE_DATETIME
    inline wxVariant wxPGVariantCreator(const wxDateTime& a) { return wxVariant(a); }
   #endif
  #endif // !defined(SWIG)

#else // !wxPG_PGVARIANT_IS_VARIANT

union wxPGVariantUnion
{
    long        m_long;
    void*       m_ptr;
    bool        m_bool;
};

//
// Very simple value wrapper class
//
// NB: It only holds the pointers for a short period, so don't
//     worry about it not making copies.
class WXDLLIMPEXP_PG wxPGVariant
{
public:

    /** Constructor for none. */
    wxPGVariant()
    {
        m_v.m_ptr = (void*)NULL;
    }
#ifndef SWIG
    /** Constructor for long integer. */
    wxPGVariant( long v_long )
    {
        m_v.m_long = v_long;
    }
    /** Constructor for integer. */
    wxPGVariant( int v_long )
    {
        m_v.m_long = v_long;
    }
    /** Constructor for bool. */
    wxPGVariant( bool value )
    {
        m_v.m_bool = value;
    }
    /** Constructor for float. */
    wxPGVariant( const double& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxString*. */
    wxPGVariant( const wxString& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxArrayString*. */
    wxPGVariant( const wxArrayString& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxArrayInt. */
    wxPGVariant( const wxArrayInt& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxPoint. */
    wxPGVariant( const wxPoint& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxSize. */
    wxPGVariant( const wxSize& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxLongLong. */
    wxPGVariant( const wxLongLong& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxULongLong. */
    wxPGVariant( const wxULongLong& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxObject&. */
    wxPGVariant( const wxObject& v_ptr )
    {
        m_v.m_ptr = (void*)&v_ptr;
    }
    /** Constructor for wxObject*. */
    wxPGVariant( const wxObject* v_ptr )
    {
        m_v.m_ptr = (void*)v_ptr;
    }
    /** Constructor for void*. */
    wxPGVariant( void* v_ptr )
    {
        m_v.m_ptr = v_ptr;
    }
#if wxUSE_DATETIME
    /** Constructor for wxDateTime ptr. */
    wxPGVariant( const wxDateTime& dt )
    {
        m_v.m_ptr = (void*) &dt;
    }
#endif

    /** Returns value as long integer. */
    inline long GetLong() const
    {
        return m_v.m_long;
    }
    /** Returns value as boolean integer. */
    inline bool GetBool() const
    {
        return m_v.m_bool;
    }
    /** Returns value as floating point number. */
    inline double GetDouble() const
    {
        return *((double*)m_v.m_ptr);
    }
    /** Returns value as floating point number ptr. */
    inline double* GetDoublePtr() const
    {
        return (double*) m_v.m_ptr;
    }
    /** Returns value as a wxString. */
    inline const wxString& GetString() const
    {
        return *((const wxString*)m_v.m_ptr);
    }
    /** Returns value as a reference to a wxArrayString. */
    inline wxArrayString& GetArrayString() const
    {
        wxArrayString* temp = (wxArrayString*)m_v.m_ptr;
        return *temp;
    }

    inline const wxObject& GetWxObject() const
    {
        return *((const wxObject*)m_v.m_ptr);
    }

    inline wxObject* GetWxObjectPtr() const
    {
        return (wxObject*)m_v.m_ptr;
    }

    /** Returns value as void*. */
    inline void* GetVoidPtr() const
    {
        return m_v.m_ptr;
    }
#if wxUSE_DATETIME
    /** Returns value as const wxDateTime&. */
    inline const wxDateTime& GetDateTime() const
    {
        return *((const wxDateTime*)m_v.m_ptr);
    }
#endif

#endif // #ifndef SWIG

    /** Returns value as long integer without type checking. */
    inline long GetRawLong() const { return m_v.m_long; }

    /** Returns value as void* without type checking. */
    inline void* GetRawPtr() const { return m_v.m_ptr; }

#undef wxPG_ASSERT_VARIANT_GET

    /** Value in portable format. */
    wxPGVariantUnion          m_v;
};

    #define wxPGVariantGetWxObjectPtr(A)    ((wxObject*)A.GetRawPtr())
    #define wxPGVariantToWxObjectPtr(A,B)   wxDynamicCast((wxObject*)A.GetRawPtr(),B);
    #define wxPGVariantToWxObject(A)        A.GetWxObject()
    #define wxPGVariantToDateTime(A)        *((wxDateTime*)A.GetVoidPtr())
    #define wxPGVariantToWxPoint(A)         *((wxPoint*)wxPGVariantToVoidPtr(A))
    #define wxPGVariantToWxSize(A)          *((wxSize*)wxPGVariantToVoidPtr(A))
    #define wxPGVariantToWxLongLong(A)      *((wxLongLong*)wxPGVariantToVoidPtr(A))
    #define wxPGVariantToWxULongLong(A)     *((wxULongLong*)wxPGVariantToVoidPtr(A))
    #define wxPGVariantToArrayInt(A)        *((wxArrayInt*)A.GetVoidPtr())
    #define wxPGVariantFromWxObject(A)      *((const wxObject*)A)
    #define wxPGVariantFromLong(A)          wxPGVariant(A)
    #define wxPGVariantCreator              wxPGVariant

#endif // !wxPG_PGVARIANT_IS_VARIANT

// Helper macros
#define wxPGVariantToString(A)      A.GetString()
#define wxPGVariantToLong(A)        A.GetLong()
#define wxPGVariantToBool(A)        A.GetBool()
#define wxPGVariantToDouble(A)      A.GetDouble()
#define wxPGVariantToArrayString(A) A.GetArrayString()
#define wxPGVariantToVoidPtr(A)     A.GetVoidPtr()

#define wxPGVariantFromString(A)        A
#define wxPGVariantFromDouble(A)        A
#define wxPGVariantFromArrayString(A)   A
#define wxPGVariantFromBool(A)          A


// -----------------------------------------------------------------------

//
// Property class declaration helper macros
// (wxPGRootPropertyClass and wxPropertyCategory require this).
//

#define WX_PG_DECLARE_CLASSINFO() \
    WX_PG_DECLARE_GETCLASSNAME \
    WX_PG_DECLARE_GETCLASSINFO

// We don't want to create SWIG interface for DoGetEditorClass (we'll use GetEditor instead)
#ifndef SWIG
    #define WX_PG_DECLARE_DOGETEDITORCLASS  virtual const wxPGEditor* DoGetEditorClass() const;
#else
    #define WX_PG_DECLARE_DOGETEDITORCLASS
#endif

#define WX_PG_DECLARE_PROPERTY_CLASS() \
public: \
    virtual wxPG_VALUETYPE_MSGVAL GetValueType() const; \
    WX_PG_DECLARE_DOGETEDITORCLASS \
    WX_PG_DECLARE_CLASSINFO() \
private:


// Added for SWIG (which apparently doesn't support 0-argument macros)
// NB: Othwise, this is *identical* to WX_PG_DECLARE_PROPERTY_CLASS()
#define WX_PG_DECLARE_PROPERTY_CLASS_NOPARENS \
public: \
    virtual wxPG_VALUETYPE_MSGVAL GetValueType() const; \
    WX_PG_DECLARE_DOGETEDITORCLASS \
    WX_PG_DECLARE_CLASSINFO() \
private:



#ifndef SWIG

// -----------------------------------------------------------------------
// wxPGPropertyClassInfo


typedef wxPGProperty* (*wxPGPropertyConstructor) (const wxString&,const wxString&);

/** \class wxPGPropertyClassInfo
	\ingroup classes
    \brief Class info structure for wxPGProperty derivatives (may be deprecated
    in a future release).
*/
struct wxPGPropertyClassInfo
{
    /** One returned by GetPropertyClassName */
    const wxChar*                   m_name;

    /** Classinfo of the base property class. */
    const wxPGPropertyClassInfo*    m_baseInfo;

    /** Simple property constructor function. */
    wxPGPropertyConstructor         m_constructor;
};


// Use this macro to register your custom property classes.
#define wxPGRegisterPropertyClass(NAME) \
    wxPropertyGrid::RegisterPropertyClass(wxT(#NAME),&NAME##ClassInfo)


// -----------------------------------------------------------------------


// Structure for relaying choice/list info.
struct wxPGChoiceInfo
{
    const wxChar**  m_arrWxChars;
    wxString*       m_arrWxString;
    wxPGChoices*    m_choices;
    int             m_itemCount;
};


// -----------------------------------------------------------------------


/** \class wxPGPropertyDataExt
	\ingroup classes
    \brief wxPGPropertyDataExt is data extension class for wxPGProperty.
*/
class WXDLLIMPEXP_PG wxPGPropertyDataExt
{
public:

    wxPGPropertyDataExt()
    {
#if wxUSE_VALIDATORS
        m_validator = (wxValidator*) NULL;
#endif
        m_customEditor = (wxPGEditor*) NULL;
        m_valueBitmap = (wxBitmap*) NULL;
    }

    ~wxPGPropertyDataExt()
    {
        // Do not delete m_customEditor
#if wxUSE_VALIDATORS
        delete m_validator;
#endif
        delete m_valueBitmap;
    }

    // For conviency, declare all members public.

    wxString                    m_helpString; // Help shown in statusbar or help box.
    const wxPGEditor*           m_customEditor; // Overrides editor returned by property class

#if wxUSE_VALIDATORS
    // NOTE: This is candidate for hash mapping.
    wxValidator*                m_validator; // Editor is going to get this validator
#endif

    wxBitmap*                   m_valueBitmap; // Show this in front of the value
};

#endif

// -----------------------------------------------------------------------

/** \class wxPGProperty
	\ingroup classes
    \brief wxPGProperty, alias wxBasePropertyClass, is base class for properties.
    Information here is provided primarily for anyone who creates new properties,
    since <b>all operations on properties should be done via wxPropertyGrid's or
    wxPropertyGridManager's methods</b>.

    \remarks
    - When changing name of a property, it is essential to use wxPropertyGrid::SetPropertyName
      (that's why there is no SetName method).
*/
class WXDLLIMPEXP_PG wxPGProperty
{
#ifndef SWIG
    friend class wxPGPropertyWithChildren;
    friend class wxPropertyGrid;
    friend class wxPropertyContainerMethods;
    friend class wxPropertyGridState;
#endif
public:

// PYSWIG is a special symbol used by my custom scripts. Code to remove it
// automatically should be added in future.
#ifndef PYSWIG
    /** Basic constructor. Should not be necessary to override.
    */
    wxPGProperty();
#endif

    /** Constructor.
        Real used property classes should have constructor of this style:

        \code

        // If MyValueType is a class, then it should be a constant reference
        // (e.g. const MyValueType& ) instead.
        wxMyProperty( const wxString& label, const wxString& name,
            MyValueType value ) : wxPGProperty
        {
            // Only required if MyValueType is not built-in default
            // (wxString, long, double, bool, and wxArrayString are;
            // wxFont, wxColour, etc. are not).
            wxPG_INIT_REQUIRED_TYPE(MyValueType)
            DoSetValue(value); // Generally recommended way to set the initial value.

            // If has child properties (i.e. wxPGPropertyWithChildren is used
            // as the parent class), then create children here. For example:
            //     AddChild( new wxStringProperty( wxT("Subprop 1"), wxPG_LABEL, value.GetSubProp1() ) );
        }

        \endcode

        Of course, in this example, wxPGProperty could also be wxPGPropertyWithChildren
        (if it has sub-properties) or actually any other property class.
    */
    wxPGProperty( const wxString& label, const wxString& name );

    /** Virtual destructor. It is customary for derived properties to override this. */
    virtual ~wxPGProperty();

    /** Sets property's internal value.
        \param value
        Simple container with GetString(), GetLong() etc. methods. Currently recommended
        means to extract value is to use wxPGVariantToXXX(value) macro.
        \remarks
        Example pseudo-implementation with comments:
        \code

        void wxMyProperty::DoSetValue ( wxPGVariant value )
        {
            // A) Get value. For example
            const wxMyValueType* pvalue = wxPGVariantToWxObjectPtr(value,wxMyValueType);
            //    or:
            const wxString& str = wxPGVariantToString(value);
            //    or:
            long val = wxPGVariantToLong(value);

            // B) If value is wxObject or void based with NULL default, then handle that:
            if ( pvalue )
                m_value = *pvalue;
            else
                pmyvalue->SetToDefault();

            // Otherwise
            // m_value = *pvalue;
            // is sufficient.

            // C) If has children, this should be here (before displaying in control).
            RefreshChildren();

        }

        \endcode
    */
    virtual void DoSetValue( wxPGVariant value );

    /** Returns properly constructed wxPGVariant.
    */
    virtual wxPGVariant DoGetValue() const;

    /** Returns text representation of property's value.
        \param argFlags
        If wxPG_FULL_VALUE is set, returns complete, storable value instead of displayable
        one (they may be different).
    */
    virtual wxString GetValueAsString( int argFlags = 0 ) const;

    /** Converts string to a value, and if succesfull, calls DoSetValue() on it.
        Default behaviour is to do nothing.
        \param text
        String to get the value from.
        \param report_error
        If true, invalid string will be reported (prefer to use wxLogError).
        \retval
        true if value was changed.
    */
    virtual bool SetValueFromString( const wxString& text, int flags = 0 );

    /** Converts integer to a value, and if succesfull, calls DoSetValue() on it.
        Default behaviour is to do nothing.
        \param value
        Int to get the value from.
        \param flags
        If has wxPG_FULL_VALUE, then the value given is a actual value and not an index.
        \retval
        true if value was changed.
    */
    virtual bool SetValueFromInt( long value, int flags = 0 );

    /** Returns size of the custom painted image in front of property. This method
        must be overridden to return non-default value if OnCustomPaint is to be
        called.
        \remarks
        - If flexible image size is desired, return wxPG_FLEXIBLE_SIZE(wid,hei).
          OnCustomPaint is then called to measure items as well (see for
          wxPGProperty::OnCustomPaint for measure call specs).
        - If entire property, including text, is to be custom painted, then
          wxPG_FULL_CUSTOM_PAINT_SIZE(hei) or wxPG_FULL_CUSTOM_PAINT_FLEXIBLE_SIZE(hei)
          is to be returned.
        - Default behaviour is to return wxSize(0,0), which means no image.
        - Default image width or height is indicated with dimension -1.
    */
    virtual wxSize GetImageSize() const;

    /** Events received by editor widgets are processed here. Note that editor class
        usually processes most events. Some, such as button press events of
        TextCtrlAndButton class, should be handled here. Also, if custom handling
        for regular events is desired, then that can also be done (for example,
        wxSystemColourProperty custom handles wxEVT_COMMAND_CHOICE_SELECTED).
        \param event
        Associated wxEvent.
        \retval
        Should return true if any changes in value should be reported. This is case,
        for example, when enter is pressed in wxTextCtrl.
    */
    virtual bool OnEvent( wxPropertyGrid* propgrid, wxWindow* wnd_primary, wxEvent& event );

#if wxPG_INCLUDE_WXOBJECT
    inline wxPG_CONST_WXCHAR_PTR GetClassName() const
    {
        return GetClassInfo()->GetClassName();
    }
#else
    /** Returns classname (for example, "wxStringProperty" for wxStringProperty)
        of a property class.
    */
    virtual wxPG_CONST_WXCHAR_PTR GetClassName() const = 0;
#endif

    /** Returns pointer to the object that has methods related to
        the value type of this property. Keep atleast this method
        abstract so the class is kept abstract.
    */
#ifndef __WXPYTHON__
    virtual const wxPGValueType* GetValueType() const = 0;
#else
  #ifndef SWIG
    virtual const wxPGValueType* GetValueType() const;
  #endif

    // Implement this in Python
    virtual wxString GetType() const;
#endif

#if !wxPG_VALUETYPE_IS_STRING
    const wxPGValueType* GetValueTypePtr() const { return GetValueType(); }
#else
    const wxPGValueType* GetValueTypePtr() const;
#endif

#ifndef SWIG
    /** Returns pointer to an instance of editor class.
    */
    virtual const wxPGEditor* DoGetEditorClass() const;
#endif

#ifdef __WXPYTHON__
    /** Returns name of editor used. Takes precendence in the wxPython bindings.
    */
    virtual wxString GetEditor() const;
#endif

#if wxUSE_VALIDATORS
    /** Returns pointer to the wxValidator that should be used
        with the editor of this property (NULL for no validator).
        Setting validator explicitly via SetPropertyValidator
        will override this.

        In most situations, code like this should work well
        (macros are used to maintain one actual validator instance,
        so on the second call the function exits within the first
        macro):

        \code

        wxValidator* wxMyPropertyClass::DoGetValidator () const
        {
            WX_PG_DOGETVALIDATOR_ENTRY()

            wxMyValidator* validator = new wxMyValidator(...);

            ... prepare validator...

            WX_PG_DOGETVALIDATOR_EXIT(validator)
        }

        \endcode

        \remarks
        You can get common filename validator by returning
        wxFilePropertyClass::GetClassValidator(). wxDirProperty,
        for example, uses it.
    */
    virtual wxValidator* DoGetValidator () const;
#endif // #if wxUSE_VALIDATORS

    /** Returns 0 for normal items. 1 for categories, -1 for other properties with children,
        -2 for wxCustomProperty (mostly like -1 ones but with few expections).
        \remarks
        Should not be overridden by new custom properties. Usually only used internally.
    */
    inline signed char GetParentingType() const { return m_parentingType; }

    /** Returns current value's index to the choice control. May also return,
        through pointer arguments, strings that should be inserted to that control.
        Irrelevant to classes which do not employ wxPG_EDITOR(Choice) or similar.
        \remarks
        - If returns -1 in choiceinfo->m_itemCount, then in that case this
          class be derived from wxBaseEnumPropertyClass (see propdev.h) and
          GetEntry is used to fill an array (slower, but handier if you don't
          store your labels as arrays of strings).
        - Must not crash even if property's set of choices is uninitialized
          (i.e. it points to wxPGGlobalVars->m_emptyConstants).
    */
    virtual int GetChoiceInfo( wxPGChoiceInfo* choiceinfo );

    /** Override to paint an image in front of the property value text or drop-down
        list item (but only if wxPGProperty::GetImageSize is overridden as well).

        If property's GetImageSize() returns size that has height != 0 but less than
        row height ( < 0 has special meanings), wxPropertyGrid calls this method to
        draw a custom image in a limited area in front of the editor control or
        value text/graphics, and if control has drop-down list, then the image is
        drawn there as well (even in the case GetImageSize() returned higher height
        than row height).

        NOTE: Following applies when GetImageSize() returns a "flexible" height (
        using wxPG_FLEXIBLE_SIZE(W,H) macro), which implies variable height items:
        If rect.x is < 0, then this is a measure item call, which means that
        dc is invalid and only thing that should be done is to set paintdata.m_drawnHeight
        to the height of the image of item at index paintdata.m_choiceItem. This call
        may be done even as often as once every drop-down popup show.

        \param dc
        wxDC to paint on.
        \param rect
        Box reserved for custom graphics. Includes surrounding rectangle, if any.
        If x is < 0, then this is a measure item call (see above).
        \param paintdata
        wxPGPaintData structure with much useful data.

        \remarks
            - You can actually exceed rect width, but if you do so then paintdata.m_drawnWidth
              must be set to the full width drawn in pixels.
            - Due to technical reasons, rect's height will be default even if custom height
              was reported during measure call.
            - Changing font and text colour affects the text drawn next to the painted image
              (which is done immediately after the OnCustomPaint call finishes).
            - Brush is guaranteed to be default background colour. It has been already used to
              clear the background of area being painted. It can be modified.
            - Pen is guaranteed to be 1-wide 'black' (or whatever is the proper colour) pen for
              drawing framing rectangle. It can be changed as well.
        \sa @link GetValueAsString @endlink
    */
    virtual void OnCustomPaint( wxDC& dc,
        const wxRect& rect, wxPGPaintData& paintdata );

    /** Sets an attribute of this property. This is quite property class specific,
        and there are no common attributes. Note that attribute may be specific
        to a property instance, or it may affect all properties of its class.
        \param id
        Identifier of attribute
        \param value
        Value for that attribute.
    */
    virtual void SetAttribute( int id, wxVariant& value );


    /** Adds entry to property's wxPGChoices and editor control (if it is active).
        Returns index of item added.
    */
    inline int AppendChoice( const wxString& label, int value = wxPG_INVALID_VALUE )
    {
        return InsertChoice(label,-1,value);
    }

    /** Removes entry from property's wxPGChoices and editor control (if it is active).

        If selected item is deleted, then the value is set to unspecified.
    */
    void DeleteChoice( int index );

    /** Returns comma-delimited string of property attributes.
    */
    wxString GetAttributes( unsigned int flagmask = 0xFFFF );

#if !wxPG_INCLUDE_WXOBJECT
    /** Returns classinfo of the property class.
    */
    virtual const wxPGPropertyClassInfo* GetClassInfo() const = 0;
#endif

    /** Returns property's label. */
    inline const wxString& GetLabel() const { return m_label; }

#ifndef SWIG
    /** Returns wxPropertyGridState to which this property belongs. */
    wxPropertyGridState* GetParentState() const;
#endif

    /** Returns property's name (alternate way to access property). */
    inline const wxString& GetName() const { return m_name; }
    inline void DoSetName(const wxString& str) { m_name = str; }

    /** If property did not have data extension, one is created now
        (returns true in that case).
    */
    bool EnsureDataExt();

    /** Gets pre-calculated top y coordinate of property graphics.
        This cannot be relied on all times (wxPropertyGrid knows when :) ),
        and value is -1 if property is not visible.
    */
    inline int GetY() const { return m_y; }

    void UpdateControl( wxWindow* primary );

    inline wxString GetDisplayedString() const
    {
        return GetValueAsString(0);
    }

    /** Returns property id. */
    inline wxPGId GetId() { return wxPGIdGen(this); }

    /** Returns property grid where property lies. */
    wxPropertyGrid* GetGrid() const;

    /** Returns highest level non-category, non-root parent. Useful when you
        have nested wxCustomProperties/wxParentProperties.
        \remarks
        Thus, if immediate parent is root or category, this will return the
        property itself.
    */
    wxPGProperty* GetMainParent() const;

    /** Return parent of property */
    inline wxPGPropertyWithChildren* GetParent() const { return m_parent; }

    /** Returns true if property is valid and wxPropertyGrid methods
        can operate on it safely.
    */
    inline bool IsOk() const
    {
        return (( m_y >= -1 )?true:false);
    }

	typedef short FlagType;
#ifndef __WXPYTHON__
    typedef void* ClientDataType;
#else
    typedef PyObject* ClientDataType;
#endif

    inline bool IsFlagSet( FlagType flag ) const
    {
        return ( m_flags & flag ) ? true : false;
    }

    inline bool IsValueUnspecified() const
    {
        return ( m_flags & wxPG_PROP_UNSPECIFIED ) ? true : false;
    }

    bool HasFlag( FlagType flag ) const
    {
        return ( m_flags & flag ) ? true : false;
    }

    /** Initializes the property. Usually only called in the constructor.
    */
    void Init( const wxString& label, const wxString& name );

    /** Returns true if extra children can be added for this property
        (i.e. it is wxPropertyCategory or wxCustomProperty)
    */
    inline bool CanHaveExtraChildren() const
    {
        return ( m_parentingType == 1 || m_parentingType == -2 );
    }

    /** Returns property's data extension (NULL if none). */
    inline wxPGPropertyDataExt* GetDataExt() { return m_dataExt; }

    unsigned int GetFlags() const
    {
        return (unsigned int)m_flags;
    }

    const wxPGEditor* GetEditorClass() const;

#ifndef __WXPYTHON__
    /** Returns type name of property that is compatible with CreatePropertyByType.
        and wxVariant::GetType.
    */
    inline const wxChar* GetType() const
    {
        return GetValueTypePtr()->GetType();
    }
#endif

    /** Adds entry to property's wxPGChoices and editor control (if it is active).
        Returns index of item added.
    */
    int InsertChoice( const wxString& label, int index, int value = wxPG_INVALID_VALUE );

    bool IsKindOf( wxPGPropertyClassInfo& info );

    /** Returns true if this is a sub-property. */
    inline bool IsSubProperty() const
    {
        wxPGProperty* parent = (wxPGProperty*)m_parent;
        if ( parent && parent->GetParentingType() < 0 && parent->m_y > -2 )
            return true;
        return false;
    }

    /** Returns last visible sub-property, recursively.
    */
    const wxPGProperty* GetLastVisibleSubItem() const;

    inline int GetMaxLength() const
    {
        return (int) m_maxLen;
    }

#ifdef SWIG
    %pythoncode {
        def GetValue(self):
            return self.GetGrid().GetPropertyValue(self)
    }
#else
    /** Returns value as wxVariant.
    */
    wxVariant GetValueAsVariant() const;
#endif

    /** Returns true if containing grid uses wxPG_EX_AUTO_UNSPECIFIED_VALUES.
    */
    bool UsesAutoUnspecified() const;

    inline wxBitmap* GetValueImage() const
    {
        if ( m_dataExt )
            return m_dataExt->m_valueBitmap;
        return (wxBitmap*) NULL;
    }

    /** Returns number of children (always 0 for normal properties). */
    size_t GetChildCount() const;

    inline unsigned int GetArrIndex() const { return m_arrIndex; }

    inline unsigned int GetDepth() const { return (unsigned int)m_depth; }

    /** Returns position in parent's array. */
    inline unsigned int GetIndexInParent() const
    {
        return (unsigned int)m_arrIndex;
    }

    /** Hides or reveals the property.
        \param hide
        true for hide, false for reveal.
    */
    inline bool Hide( bool hide );

    inline bool IsEnabled() const
    {
        return ( m_flags & wxPG_PROP_DISABLED ) ? false : true;
    }

    /** If property's editor is created this forces its recreation. Useful
        in SetAttribute etc. Returns true if actually did anything.
    */
    bool RecreateEditor();

    inline void SetAttrib( int id, wxVariant value )
    {
        SetAttribute(id,value);
    }

    /** Sets attributes from a comma-delimited string.
    */
    void SetAttributes( const wxString& attributes );

    /** Sets editor for a property. */
#ifndef SWIG
    inline void SetEditor( const wxPGEditor* editor )
    {
        EnsureDataExt();
        m_dataExt->m_customEditor = editor;
    }
#endif

    /** Sets editor for a property. */
    inline void SetEditor( const wxString& editorName );

    /** Changes value of a property with choices, but only
        works if the value type is long or string. */
    void SetChoiceSelection( int newValue, const wxPGChoiceInfo& choiceInfo );

    /** Set wxBitmap in front of the value. This bitmap will be ignored if
        property class has implemented OnCustomPaint.
    */
    void SetValueImage( wxBitmap& bmp );

    /** If property has choices and they are not yet exclusive, new such copy
        of them will be created.
    */
    void SetChoicesExclusive();

    void SetFlag( FlagType flag ) { m_flags |= flag; }

    inline void SetHelpString( const wxString& helpString )
    {
        EnsureDataExt();
        m_dataExt->m_helpString = helpString;
    }

    inline void SetLabel( const wxString& label ) { m_label = label; }

    inline void SetValueToUnspecified()
    {
        m_flags |= wxPG_PROP_UNSPECIFIED;
    }

#if wxUSE_VALIDATORS
    /** Sets wxValidator for a property*/
    inline void SetValidator( const wxValidator& validator )
    {
        EnsureDataExt();
        m_dataExt->m_validator = wxDynamicCast(validator.Clone(),wxValidator);
    }

    /** Gets assignable version of property's validator. */
    inline wxValidator* GetValidator() const
    {
        if ( m_dataExt )
            return m_dataExt->m_validator;
        return DoGetValidator();
    }
#endif // #if wxUSE_VALIDATORS

    inline bool StdValidationProcedure( wxPGVariant value )
    {
        DoSetValue( value );
        return true;
    }

    /** Updates property value in case there were last minute
        changes. If value was unspecified, it will be set to default.
        Use only for properties that have TextCtrl-based editor.
        \remarks
        If you have code similar to
        \code
            // Update the value in case of last minute changes
            if ( primary && propgrid->IsEditorsValueModified() )
                 GetEditorClass()->CopyValueFromControl( this, primary );
        \endcode
        in wxPGProperty::OnEvent wxEVT_COMMAND_BUTTON_CLICKED handler,
        then replace it with call to this method.
        \retval
        True if value changed.
    */
    bool PrepareValueForDialogEditing( wxPropertyGrid* propgrid );

#if wxPG_USE_CLIENT_DATA
    inline ClientDataType GetClientData() const { return m_clientData; }

    /** Sets client data (void*) of a property.
        \remarks
        This untyped client data has to be deleted manually.
    */
    inline void SetClientData( ClientDataType clientData )
    {
#ifdef __WXPYTHON__
        if ( m_clientData )
            Py_DECREF( m_clientData );
        Py_INCREF( clientData );
#endif
        m_clientData = clientData;
    }
#endif

    /** Sets new set of choices for property.
    */
    bool SetChoices( wxPGChoices& choices );

    /** Sets new set of choices for property.
    */
    inline bool SetChoices( const wxArrayString& labels,
                            const wxArrayInt& values = wxPG_EMPTY_ARRAYINT );

    /** Set max length of text editor.
    */
    inline bool SetMaxLength( int maxLen );

    inline wxString GetHelpString() const
    {
        if (m_dataExt)
            return m_dataExt->m_helpString;
        return wxEmptyString;
    }

    void ClearFlag( FlagType flag ) { m_flags &= ~(flag); }

    // Use, for example, to detect if item is inside collapsed section.
    bool IsSomeParent( wxPGProperty* candidate_parent ) const;

    // Shows error as a tooltip or something similar (depends on platform).
    void ShowError( const wxString& msg );

#if defined(__WXPYTHON__) && !defined(SWIG)
    // This is the python object that contains and owns the C++ representation.
    PyObject*                   m_scriptObject;
#endif

#ifndef SWIG
protected:

    // Called in constructors.
    void Init();

    wxString                    m_label;
    wxString                    m_name;
    wxPGPropertyWithChildren*   m_parent;
#if wxPG_USE_CLIENT_DATA
    ClientDataType              m_clientData;
#endif
    wxPGPropertyDataExt*        m_dataExt; // Optional data extension.
    unsigned int                m_arrIndex; // Index in parent's property array.
    int                         m_y; // This could be short int.

    short                       m_maxLen; // Maximum length (mainly for string properties).
                                          // Could be in some sort of wxBaseStringProperty, but currently,
                                          // for maximum flexibility and compatibility, we'll stick it here.
                                          // Anyway, we had 3 excess bytes to use so short int will fit in
                                          // just fine.

    FlagType                    m_flags;

    // 1 = category
    // 0 = no children
    // -1 = has fixed-set of sub-properties
    // -2 = this is wxCustomProperty (sub-properties can be added)
    signed char                 m_parentingType;

    unsigned char               m_depth; // Root has 0, categories etc. at that level 1, etc.

    // m_depthBgCol indicates width of background colour between margin and item
    // (essentially this is category's depth, if none then equals m_depth).
    unsigned char               m_depthBgCol;

    unsigned char               m_bgColIndex; // Background brush index.
    unsigned char               m_fgColIndex; // Foreground colour index.

#endif // #ifndef SWIG
};

extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxBasePropertyClassInfo;


//
// wxPGId comparison operators.
// TODO: Are these really used?
//

#if !defined(__WXPYTHON__)

inline bool operator==(const wxPGId& id, const wxString& b)
{
    wxASSERT(wxPGIdIsOk(id));
    const wxString& a = wxPGIdToPtr(id)->GetName();
    return (a.Len() == b.Len()) && (a.Cmp(b) == 0);
}

inline bool operator==(const wxPGId& id, const wxChar* b)
{
    wxASSERT(wxPGIdIsOk(id));
    return wxPGIdToPtr(id)->GetName().Cmp(b) == 0;
}

#endif // !defined(__WXPYTHON__)


// For dual-pointer-usage reasons, we need to use this trickery
// instead of wxObjArray. wxPGValueType hash map is declared
// in propdev.h.
typedef wxArrayPtrVoid wxPGArrayProperty;


// Always use wxString based hashmap with unicode, stl, swig and GCC 4.0+
#if !defined(SWIG)
WX_DECLARE_STRING_HASH_MAP_WITH_DECL(void*,
                                     wxPGHashMapS2P,
                                     class WXDLLIMPEXP_PG);
#else
class WXDLLIMPEXP_PG wxPGHashMapS2P;
#endif

#define wxPGPropNameStr            const wxString&
#define wxPGNameConv(STR)      STR


// -----------------------------------------------------------------------


#ifndef SWIG

WX_DECLARE_VOIDPTR_HASH_MAP_WITH_DECL(void*,
                                      wxPGHashMapP2P,
                                      class WXDLLIMPEXP_PG);

#else
class WXDLLIMPEXP_PG wxPGHashMapP2P;
#endif // #ifndef SWIG

// -----------------------------------------------------------------------

/** \class wxPGPropertyWithChildren
    \ingroup classes
    \brief wxPGPropertyWithChildren, alias wxParentPropertyClass, is a base
    class for new properties that have sub-properties. For example,
    wxFontProperty and wxFlagsProperty descend from this class.
*/
class WXDLLIMPEXP_PG wxPGPropertyWithChildren : public wxPGProperty
{
#ifndef SWIG
    friend class wxPGProperty;
    friend class wxPropertyGridState;
    friend class wxPropertyGrid;
#endif
    //_WX_PG_DECLARE_PROPERTY_CLASS(wxPGPropertyWithChildren)
public:

    /** Special constructor only used in special cases. */
    wxPGPropertyWithChildren();

    /** When new class is derived, call this constructor.
        \param label
        Label for the property.
    */
    wxPGPropertyWithChildren( const wxString& label, const wxString& name );

    /** Destructor. */
    virtual ~wxPGPropertyWithChildren();

    //virtual int GetParentingType() const;

    /** Advanced variant of GetValueAsString() that forms a string that
        contains sequence of text representations of sub-properties.
    */
    // Advanced version that gives property list and index to this item
    virtual wxString GetValueAsString( int argFlags = 0 ) const;

    /** This overridden version converts comma or semicolon separated
        tokens into child values.
    */
    virtual bool SetValueFromString( const wxString& text, int flags );

    /** Refresh values of child properties.
    */
    virtual void RefreshChildren();

    /** Called after child property p has been altered.
        The value of this parent property should now be updated accordingly.
    */
    virtual void ChildChanged( wxPGProperty* p );

    /** This is used by properties that have fixed sub-properties. */
    void AddChild( wxPGProperty* prop );

    /** This is used by Insert etc. */
    void AddChild2( wxPGProperty* prop, int index = -1, bool correct_mode = true );

    /** Returns number of sub-properties. */
    inline size_t GetCount() const { return m_children.GetCount(); }

    /** Returns sub-property at index i. */
    inline wxPGProperty* Item( size_t i ) const { return (wxPGProperty*)m_children.Item(i); }

    /** Returns last sub-property.
    */
    wxPGProperty* Last() const { return (wxPGProperty*)m_children.Last(); }

    /** Returns index of given sub-property. */
    inline int Index( const wxPGProperty* p ) const { return m_children.Index((void*)p); }

    /** Deletes all sub-properties. */
    void Empty();

    inline bool IsExpanded() const
    {
        return ( m_expanded > 0 ) ? true : false;
    }

    // Puts correct indexes to children
    void FixIndexesOfChildren( size_t starthere = 0 );

#ifndef SWIG
    // Returns wxPropertyGridState in which this property resides.
    wxPropertyGridState* GetParentState() const { return m_parentState; }
#endif

    wxPGProperty* GetItemAtY( unsigned int y, unsigned int lh );

    /** Returns (direct) child property with given name (or NULL if not found).
    */
    wxPGProperty* GetPropertyByName( const wxString& name ) const;

#ifndef SWIG
    // Call for after sub-properties added with AddChild
    void PrepareSubProperties();

    inline void SetParentState( wxPropertyGridState* pstate ) { m_parentState = pstate; }

    // Call after fixed sub-properties added/removed after creation.
    // if oldSelInd >= 0 and < new max items, then selection is
    // moved to it.
    void SubPropsChanged( int oldSelInd = -1 );

protected:

    wxPropertyGridState*    m_parentState;

    wxPGArrayProperty       m_children;
    unsigned char           m_expanded;
#endif // SWIG
};

extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxBaseParentPropertyClassInfo;

// -----------------------------------------------------------------------

/** \class wxPGRootPropertyClass
    \ingroup classes
    \brief Root parent property.
*/
class WXDLLIMPEXP_PG wxPGRootPropertyClass : public wxPGPropertyWithChildren
{
public:
    WX_PG_DECLARE_PROPERTY_CLASS_NOPARENS
public:

    /** Constructor. */
    wxPGRootPropertyClass();
    virtual ~wxPGRootPropertyClass();

protected:
};

// -----------------------------------------------------------------------

/** \class wxPropertyCategoryClass
    \ingroup classes
    \brief Category (caption) property.
*/
class WXDLLIMPEXP_PG wxPropertyCategoryClass : public wxPGPropertyWithChildren
{
    WX_PG_DECLARE_PROPERTY_CLASS_NOPARENS
public:

    /** Special constructor only used in special cases. */
    wxPropertyCategoryClass();

    /** Construct.
        \param label
        Label for the category.
        \remarks
        All non-category properties appended will have most recently
        added category.
    */
    wxPropertyCategoryClass( const wxString& label, const wxString& name = wxPG_LABEL );
    ~wxPropertyCategoryClass();

    /** Must be overridden with function that doesn't do anything. */
    virtual wxString GetValueAsString( int argFlags ) const;

    inline int GetTextExtent() const { return m_textExtent; }

    void CalculateTextExtent( wxWindow* wnd, wxFont& font );

    void SetTextColIndex( unsigned int colInd ) { m_capFgColIndex = (wxByte) colInd; }
    unsigned int GetTextColIndex() const { return (unsigned int) m_capFgColIndex; }

protected:
    int     m_textExtent;  // pre-calculated length of text
    wxByte  m_capFgColIndex;  // caption text colour index
};

// -----------------------------------------------------------------------


#ifndef SWIG

typedef void* wxPGChoicesId;

class WXDLLIMPEXP_PG wxPGChoicesData
{
public:

    // Constructor sets m_refCount to 1.
    wxPGChoicesData();

    ~wxPGChoicesData();

    wxArrayString   m_arrLabels;
    wxArrayInt      m_arrValues;

    // So that multiple properties can use the same set
    int             m_refCount;

};

#define wxPGChoicesEmptyData    ((wxPGChoicesData*)NULL)

#endif // SWIG


/** \class wxPGChoices
    \ingroup classes
    \brief Helper class for managing constant (key=value) sequences.
*/
class WXDLLIMPEXP_PG wxPGChoices
{
public:

    /** Basic constructor. */
    wxPGChoices()
    {
        Init();
    }

    /** Copy constructor. */
    wxPGChoices( wxPGChoices& a )
    {
        wxASSERT(a.m_data);
        m_data = a.m_data;
        a.m_data->m_refCount++;
    }

    /** Constructor. */
    wxPGChoices( const wxChar** labels, const long* values = NULL )
    {
        Init();
        Set(labels,values);
    }

    /** Constructor. */
    wxPGChoices( const wxArrayString& labels, const wxArrayInt& values = wxPG_EMPTY_ARRAYINT )
    {
        Init();
        Set(labels,values);
    }

    /** Simple interface constructor. */
    inline wxPGChoices( wxPGChoicesData* data )
    {
        wxASSERT(data);
        m_data = data;
        data->m_refCount++;
    }

    /** Destructor. */
    ~wxPGChoices()
    {
        Free ();
    }

    void AssignData( wxPGChoicesData* data );

    inline void Assign( const wxPGChoices& a )
    {
        AssignData(a.m_data);
    }

    /** Adds to current. If did not have own copies, creates them now. If was empty,
        identical to set except that creates copies.
    */
    void Add( const wxChar** labels, const long* values = NULL );

    /** Version that works with wxArrayString. */
    void Add( const wxArrayString& arr, const long* values = NULL );

    /** Version that works with wxArrayString and wxArrayInt. */
    void Add( const wxArrayString& arr, const wxArrayInt& arrint );

    /** Adds single item. */
    void Add( const wxChar* label, int value = wxPG_INVALID_VALUE );

    /** Adds single item. */
    void AddAsSorted( const wxString& label, int value = wxPG_INVALID_VALUE );

    inline void EnsureData()
    {
        if ( m_data == wxPGChoicesEmptyData )
            m_data = new wxPGChoicesData();
    }

    /** Returns reference to wxArrayString of labels for you to modify.
    */
    inline wxArrayString& GetLabels()
    {
        wxASSERT( m_data->m_refCount != 0xFFFFFFF );
        return m_data->m_arrLabels;
    }

    /** Returns reference to wxArrayInt of values for you to modify.
    */
    inline wxArrayInt& GetValues()
    {
        wxASSERT( m_data->m_refCount != 0xFFFFFFF );
        return m_data->m_arrValues;
    }

    /** Returns false if this is a constant empty set of choices,
        which should not be modified.
    */
    bool IsOk () const
    {
        return ( m_data != wxPGChoicesEmptyData );
    }

    /** Gets a unsigned number identifying this list. */
    wxPGChoicesId GetId() const { return (wxPGChoicesId) m_data; };

    /** Removes count items starting at position nIndex. */
    inline void RemoveAt(size_t nIndex, size_t count = 1)
    {
        wxASSERT( m_data->m_refCount != 0xFFFFFFF );
        wxPGChoicesData* data = m_data;
        data->m_arrLabels.RemoveAt(nIndex,count);
        if ( data->m_arrValues.GetCount() )
            data->m_arrValues.RemoveAt(nIndex,count);
    }

#ifndef SWIG
    /** Does not create copies for itself. */
    void Set( const wxChar** labels, const long* values = NULL )
    {
        Free();
        Add(labels,values);
    }

    /** Version that works with wxArrayString.
        TODO: Deprecate this.
    */
    void Set( wxArrayString& arr, const long* values = (const long*) NULL )
    {
        Free();
        Add(arr,values);
    }
#endif // SWIG

    /** Version that works with wxArrayString and wxArrayInt. */
    void Set( const wxArrayString& labels, const wxArrayInt& values = wxPG_EMPTY_ARRAYINT )
    {
        Free();
        if ( &values )
            Add(labels,values);
        else
            Add(labels);
    }

    // Creates exclusive copy of current choices
    inline void SetExclusive()
    {
        if ( m_data->m_refCount != 1 )
        {
            wxPGChoicesData* data = new wxPGChoicesData;
            data->m_arrLabels = m_data->m_arrLabels;
            data->m_arrValues = m_data->m_arrValues;
            Free();
            m_data = data;
        }
    }

    inline const wxString& GetLabel( size_t ind ) const
    {
        return m_data->m_arrLabels[ind];
    }

    inline const wxArrayString& GetLabels() const { return m_data->m_arrLabels; }

    inline size_t GetCount () const
    {
        wxASSERT_MSG( m_data,
            wxT("When checking if wxPGChoices is valid, use IsOk() instead of GetCount()") );
        return m_data->m_arrLabels.GetCount();
    }

    inline int GetValue( size_t ind ) const { return m_data->m_arrValues[ind]; }
    inline const wxArrayInt& GetValues() const { return m_data->m_arrValues; }

    inline int Index( const wxString& str ) const { return m_data->m_arrLabels.Index(str); }

    /** Inserts single item. */
#if wxCHECK_VERSION(2,9,0)
    void Insert( const wxString& label, int index, int value = wxPG_INVALID_VALUE );
#else
    void Insert( const wxChar* label, int index, int value = wxPG_INVALID_VALUE );
#endif

    // Returns data, increases refcount.
    inline wxPGChoicesData* GetData()
    {
        wxASSERT( m_data->m_refCount != 0xFFFFFFF );
        m_data->m_refCount++;
        return m_data;
    }

    // Returns plain data ptr - no refcounting stuff is done.
    inline wxPGChoicesData* GetDataPtr() const { return m_data; }

    // Changes ownership of data to you.
    inline wxPGChoicesData* ExtractData()
    {
        wxPGChoicesData* data = m_data;
        m_data = wxPGChoicesEmptyData;
        return data;
    }

    inline void AddString( const wxString& str ) { m_data->m_arrLabels.Add(str); }
    inline void AddInt( int val ) { m_data->m_arrValues.Add(val); }

    inline void SetLabels( wxArrayString& arr ) { m_data->m_arrLabels = arr; }
    inline void SetValues( wxArrayInt& arr ) { m_data->m_arrValues = arr; }
#ifndef SWIG
    inline void SetLabels( const wxArrayString& arr ) { m_data->m_arrLabels = arr; }
    inline void SetValues( const wxArrayInt& arr ) { m_data->m_arrValues = arr; }

protected:

    wxPGChoicesData*    m_data;

    void Init();
    void Free();
#endif
};


// -----------------------------------------------------------------------
// Property declaration.

// Doxygen will only generate warnings here
#ifndef DOXYGEN


#define wxPG_CONSTFUNC(PROP) PROP
#define wxPG_PROPCLASS(PROP) PROP##Class

// Macro based constructor.
#define wxPG_NEWPROPERTY(PROP,LABEL,NAME,VALUE) wx##PROP##Property(LABEL,NAME,VALUE)

#define wxPG_DECLARE_PROPERTY_CLASSINFO(NAME) \
    extern wxPGPropertyClassInfo NAME##ClassInfo;

#define wxPG_DECLARE_PROPERTY_CLASSINFO_WITH_DECL(NAME,DECL) \
    extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_PROPERTY_WITH_DECL(NAME,VALARG,DEFVAL,DECL) \
    extern DECL wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, VALARG value = DEFVAL ); \
    extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_PROPERTY(NAME,VALARG,DEFVAL) \
    extern wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, VALARG value = DEFVAL ); \
    wxPG_DECLARE_PROPERTY_CLASSINFO(NAME)

//
// Specific macro-based declarations.
//

#define WX_PG_DECLARE_STRING_PROPERTY_WITH_DECL(NAME,DECL) \
extern DECL wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name= wxPG_LABEL, const wxString& value = wxEmptyString ); \
extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_STRING_PROPERTY(NAME) \
extern wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name= wxPG_LABEL, const wxString& value = wxEmptyString ); \
wxPG_DECLARE_PROPERTY_CLASSINFO(NAME)

#define WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY_WITH_DECL(NAME,DECL) \
WX_PG_DECLARE_PROPERTY_WITH_DECL(NAME,long,-1,DECL)

#define WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY(NAME) \
WX_PG_DECLARE_PROPERTY(NAME,long,-1)

#define WX_PG_DECLARE_CUSTOM_ENUM_PROPERTY_WITH_DECL(NAME,DECL) \
WX_PG_DECLARE_PROPERTY_WITH_DECL(NAME,int,-1,DECL)

#define WX_PG_DECLARE_CUSTOM_ENUM_PROPERTY(NAME) \
WX_PG_DECLARE_PROPERTY(NAME,int,-1)

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY_WITH_DECL(NAME,DECL) \
extern DECL wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxArrayString& value = wxArrayString() ); \
extern DECL wxPGPropertyClassInfo NAME##ClassInfo;

#define WX_PG_DECLARE_ARRAYSTRING_PROPERTY(NAME) \
extern wxPGProperty* wxPG_CONSTFUNC(NAME)( const wxString& label, const wxString& name = wxPG_LABEL, const wxArrayString& value = wxArrayString() ); \
wxPG_DECLARE_PROPERTY_CLASSINFO(NAME)

// Declare basic property classes.
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxStringProperty,const wxString&,wxEmptyString,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxIntProperty,long,0,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxUIntProperty,unsigned long,0,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxFloatProperty,double,0.0,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxBoolProperty,bool,false,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxLongStringProperty,const wxString&,wxEmptyString,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxFileProperty,const wxString&,wxEmptyString,WXDLLIMPEXP_PG)
WX_PG_DECLARE_PROPERTY_WITH_DECL(wxArrayStringProperty,const wxArrayString&,wxArrayString(),WXDLLIMPEXP_PG)

WX_PG_DECLARE_STRING_PROPERTY_WITH_DECL(wxDirProperty,WXDLLIMPEXP_PG)

// Enum and Flags Properties require special attention.
#ifndef SWIG

extern WXDLLIMPEXP_PG wxPGProperty* wxEnumProperty( const wxString& label, const wxString& name,
    const wxChar** labels = (const wxChar**) NULL,
    const long* values = NULL, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& labels, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxEnumProperty( const wxString& label, const wxString& name,
    wxPGChoices& constants, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& choices, const wxArrayInt& values, int value = 0 );

#else

// Separate for SWIG inorder to have more default arguments
extern WXDLLIMPEXP_PG wxPGProperty* wxEnumProperty( const wxString& label, const wxString& name = wxPG_LABEL,
    const wxArrayString& choices = wxArrayString(), const wxArrayInt& values = wxArrayInt(), int value = 0 );

#endif // SWIG

extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxEnumPropertyClassInfo;


#ifndef SWIG

extern WXDLLIMPEXP_PG wxPGProperty* wxEditEnumProperty( const wxString& label, const wxString& name,
    const wxChar** labels = (const wxChar**) NULL,
    const long* values = NULL, const wxString& value = wxEmptyString );

extern WXDLLIMPEXP_PG wxPGProperty* wxEditEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& labels, const wxString& value = wxEmptyString );

extern WXDLLIMPEXP_PG wxPGProperty* wxEditEnumProperty( const wxString& label, const wxString& name,
    wxPGChoices& constants, const wxString& value = wxEmptyString );

extern WXDLLIMPEXP_PG wxPGProperty* wxEditEnumProperty( const wxString& label, const wxString& name,
    const wxArrayString& choices, const wxArrayInt& values, const wxString& value = wxEmptyString );

#else

// Separate for SWIG inorder to have more default arguments
extern WXDLLIMPEXP_PG wxPGProperty* wxEditEnumProperty( const wxString& label, const wxString& name = wxPG_LABEL,
    const wxArrayString& choices = wxArrayString(), const wxArrayInt& values = wxArrayInt(), const wxString& value = wxEmptyString );

#endif // SWIG

extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxEditEnumPropertyClassInfo;


#ifndef SWIG

extern WXDLLIMPEXP_PG wxPGProperty* wxFlagsProperty( const wxString& label, const wxString& name, const wxChar** labels = (const wxChar**) NULL,
    const long* values = NULL, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxFlagsProperty( const wxString& label, const wxString& name,
    const wxArrayString& labels, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxFlagsProperty( const wxString& label, const wxString& name,
    wxPGChoices& constants, int value = 0 );

extern WXDLLIMPEXP_PG wxPGProperty* wxFlagsProperty( const wxString& label, const wxString& name,
    const wxArrayString& flag_labels, const wxArrayInt& values, int value = 0 );

#else

// Separate for SWIG inorder to have more default arguments
extern WXDLLIMPEXP_PG wxPGProperty* wxFlagsProperty( const wxString& label, const wxString& name = wxPG_LABEL,
    const wxArrayString& flag_labels = wxArrayString(), const wxArrayInt& values = wxArrayInt(), int value = 0 );

#endif // SWIG


extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxFlagsPropertyClassInfo;


// wxCustomProperty doesn't have value argument.
extern WXDLLIMPEXP_PG wxPGProperty* wxCustomProperty( const wxString& label, const wxString& name = wxPG_LABEL );
extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxCustomPropertyClassInfo;

// wxParentProperty doesn't have value argument.
extern WXDLLIMPEXP_PG wxPGProperty* wxParentProperty( const wxString& label, const wxString& name );
extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxParentPropertyClassInfo;

// wxPropertyCategory doesn't have value argument.
extern WXDLLIMPEXP_PG wxPGProperty* wxPropertyCategory ( const wxString& label, const wxString& name = wxPG_LABEL );
extern WXDLLIMPEXP_PG wxPGPropertyClassInfo wxPropertyCategoryClassInfo;

#endif // DOXYGEN


#ifndef wxDynamicCastVariantData
    #define wxDynamicCastVariantData wxDynamicCast
#endif

// FIXME: Should this be out-of-inline?
inline wxObject* wxPG_VariantToWxObject( wxVariant& variant, wxClassInfo* classInfo )
{
    if ( !variant.IsValueKindOf(classInfo) )
        return (wxObject*) NULL;
    wxVariantData* vdata = variant.GetData();

    wxPGVariantDataWxObj* vdataWxObj = wxDynamicCastVariantData(vdata, wxPGVariantDataWxObj);
    if ( vdataWxObj )
        return (wxObject*) vdataWxObj->GetValuePtr();

    return variant.GetWxObjectPtr();
}

//
// Redefine wxGetVariantCast to also take propertygrid variantdata
// classes into account.
// TODO: Remove after persistent wxObject classes added (i.e.
//   GetWxObjectPtr works for all).
//
#undef wxGetVariantCast
#define wxGetVariantCast(var,classname) (classname*)wxPG_VariantToWxObject(var,&classname::ms_classInfo)

// TODO: After a while, remove this.
#define WX_PG_VARIANT_TO_WXOBJECT(VARIANT,CLASSNAME) (CLASSNAME*)wxPG_VariantToWxObject(VARIANT,&CLASSNAME::ms_classInfo)
//#define WX_PG_VARIANT_TO_WXOBJECT(VARIANT,CLASSNAME) wxGetVariantCast(VARIANT,CLASSNAME)

// -----------------------------------------------------------------------

#ifndef SWIG
// We won't need this class from wxPython

/** \class wxPropertyGridState
	\ingroup classes
    \brief
    Contains information of a single wxPropertyGrid page.
*/
// BM_STATE
class WXDLLIMPEXP_PG wxPropertyGridState
{
    friend class wxPGProperty;
    friend class wxPropertyGrid;
    friend class wxPropertyGridManager;
public:

    /** Constructor. */
    wxPropertyGridState();

    /** Destructor. */
    virtual ~wxPropertyGridState();

    /** Base append. */
    wxPGId Append( wxPGProperty* property );

    wxPGId AppendIn( wxPGPropertyWithChildren* pwc, const wxString& label, const wxString& propname, wxVariant& value );

    /** Returns property by its name. */
    wxPGId BaseGetPropertyByName( wxPGPropNameStr name ) const;

    /** Called in, for example, wxPropertyGrid::Clear. */
    void Clear();

    void ClearModifiedStatus( wxPGProperty* p );

    static void ClearPropertyAndChildrenFlags( wxPGProperty* p, long flags );
    static void SetPropertyAndChildrenFlags( wxPGProperty* p, long flags );

    bool ClearPropertyValue( wxPGProperty* p );

    inline bool ClearSelection()
    {
        return DoSelectProperty(wxPGIdGen((wxPGProperty*)NULL));
    }

    bool Collapse( wxPGProperty* p );

    /** Override this member function to add custom behaviour on property deletion.
    */
    virtual void DoDelete( wxPGProperty* item );

    /** Override this member function to add custom behaviour on property insertion.
    */
    virtual wxPGId DoInsert( wxPGPropertyWithChildren* parent, int index, wxPGProperty* property );

    bool EnableCategories( bool enable );

    /** Enables or disables given property and its subproperties. */
    bool EnableProperty( wxPGProperty* p, bool enable );

    bool Expand( wxPGProperty* p );

    bool ExpandAll( unsigned char do_expand );

    /** Returns id of first item, whether it is a category or property. */
    inline wxPGId GetFirst() const
    {
        wxPGProperty* p = (wxPGProperty*) NULL;
        if ( m_properties->GetCount() )
            p = m_properties->Item(0);
        return wxPGIdGen(p);
    }

    wxPGId GetFirstCategory() const;

    wxPGId GetFirstProperty() const;

    wxPropertyGrid* GetGrid() const { return m_pPropGrid; }

    wxPGId GetNextCategory( wxPGId id ) const;

    wxPGId GetNextProperty( wxPGId id ) const;

    static wxPGId GetNextSibling( wxPGId id );

    static wxPGId GetPrevSibling( wxPGId id );

    wxPGId GetPrevProperty( wxPGId id ) const;

    wxPGId GetPropertyByLabel( const wxString& name, wxPGPropertyWithChildren* parent  = (wxPGPropertyWithChildren*) NULL ) const;

    wxVariant GetPropertyValues( const wxString& listname, wxPGId baseparent, long flags ) const;

    inline wxPGProperty* GetSelection() const { return m_selected; }

    /** Used by SetSplitterLeft. */
    int GetLeftSplitterPos( wxClientDC& dc, wxPGPropertyWithChildren* pwc, bool subProps );

    inline bool IsDisplayed() const;

    inline bool IsInNonCatMode() const { return (bool)(m_properties == m_abcArray); }

    /** Only inits arrays, doesn't migrate things or such. */
    void InitNonCatMode ();

    void LimitPropertyEditing ( wxPGProperty* p, bool limit = true );

    bool DoSelectProperty( wxPGProperty* p, unsigned int flags = 0 );

    void SetPropertyLabel( wxPGProperty* p, const wxString& newlabel );

    bool SetPropertyPriority( wxPGProperty* p, int priority );

    void SetPropVal( wxPGProperty* p, const wxPGVariant& value );

    bool SetPropertyValue( wxPGProperty* p, const wxPGValueType* typeclass, const wxPGVariant& value );

    bool SetPropertyValue( wxPGProperty* p, const wxChar* typestring, const wxPGVariant& value );

    bool SetPropertyValueString( wxPGProperty* p, const wxString& value );

    bool SetPropertyValue( wxPGProperty* p, wxVariant& value );

    bool SetPropertyValueWxObjectPtr( wxPGProperty* p, wxObject* value );

    /** Sets value (long integer) of a property. */
    inline void SetPropertyValueLong( wxPGProperty* p, long value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(long), wxPGVariantFromLong(value) );
    }
    /** Sets value (integer) of a property. */
    inline void SetPropertyValue( wxPGProperty* p, int value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(long), wxPGVariantFromLong((long)value) );
    }
    /** Sets value (floating point) of a property. */
    inline void SetPropertyValueDouble( wxPGProperty* p, double value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(double), wxPGVariantFromDouble(value) );
    }
    /** Sets value (bool) of a property. */
    inline void SetPropertyValueBool( wxPGProperty* p, bool value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(bool), wxPGVariantFromLong(value?(long)1:(long)0) );
    }
    /** Sets value (wxArrayString) of a property. */
    inline void SetPropertyValueArrstr2( wxPGProperty* p, const wxArrayString& value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(wxArrayString), wxPGVariantFromArrayString(value) );
    }
    /** Sets value (void*) of a property. */
    inline void SetPropertyValue( wxPGProperty* p, void* value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(void), value );
    }
    /** Sets value (wxPoint&) of a property. */
    inline void SetPropertyValuePoint( wxPGProperty* p, const wxPoint& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("wxPoint"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxSize&) of a property. */
    inline void SetPropertyValueSize( wxPGProperty* p, const wxSize& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("wxSize"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxArrayInt&) of a property. */
    inline void SetPropertyValueArrint2( wxPGProperty* p, const wxArrayInt& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("wxArrayInt"), wxPGVariantCreator(value));
    }
#if wxUSE_DATETIME
    /** Sets value (wxDateTime&) of a property. */
    inline void SetPropertyValueDatetime( wxPGProperty* p, const wxDateTime& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("datetime"), wxPGVariantCreator(value) );
    }
#endif
#ifdef __WXPYTHON__
    inline void SetPropertyValuePyObject( wxPGProperty* p, PyObject* value )
    {
        SetPropertyValue( p, wxPG_VALUETYPE(PyObject), wxPGVariantCreator(value) );
    }
#endif
    /** Sets value (wxLongLong&) of a property. */
    inline void SetPropertyValueLongLong( wxPGProperty* p, const wxLongLong& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("wxLongLong"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxULongLong&) of a property. */
    inline void SetPropertyValueULongLong( wxPGProperty* p, const wxULongLong& value )
    {
        wxASSERT(p);
        SetPropertyValue( p, wxT("wxULongLong"), wxPGVariantCreator(value) );
    }

    void SetPropertyValues( const wxVariantList& list, wxPGId default_category );

    void SetPropertyUnspecified( wxPGProperty* p );

#ifdef wxPG_COMPATIBILITY_1_0_0
    #define SetPropertyValueUnspecified SetPropertyUnspecified
#endif

    void Sort( wxPGProperty* p );
    void Sort();

protected:

#ifndef DOXYGEN
    int PrepareToAddItem ( wxPGProperty* property, wxPGPropertyWithChildren* scheduledParent );

    /** If visible, then this is pointer to wxPropertyGrid.
        This shall *never* be NULL to indicate that this state is not visible.
    */
    wxPropertyGrid*             m_pPropGrid;

    /** Pointer to currently used array. */
    wxPGPropertyWithChildren*   m_properties;

    /** Array for categoric mode. */
    wxPGRootPropertyClass       m_regularArray;

    /** Array for root of non-categoric mode. */
    wxPGRootPropertyClass*      m_abcArray;

    /** Dictionary for name-based access. */
    wxPGHashMapS2P              m_dictName;

    /** Most recently added category. */
    wxPropertyCategoryClass*    m_currentCategory;

    /** Pointer to selected property. */
    wxPGProperty*               m_selected;

    /** 1 if m_lastCaption is also the bottommost caption. */
    unsigned char               m_lastCaptionBottomnest;
    /** 1 items appended/inserted, so stuff needs to be done before drawing;
        If m_bottomy == 0, then calcylatey's must be done.
        Otherwise just sort.
    */
    unsigned char               m_itemsAdded;

    /** 1 if any value is modified. */
    unsigned char               m_anyModified;

#endif // #ifndef DOXYGEN

};

#endif // #ifndef SWIG

inline bool wxPGProperty::SetChoices( const wxArrayString& labels,
                                      const wxArrayInt& values )
{
    wxPGChoices chs(labels,values);
    return SetChoices(chs);
}

// -----------------------------------------------------------------------

/*

    wxASSERT_MSG( wxPGIdIsOk(id), \
                  wxT("invalid property id") ); \

*/


// Helper macro that does necessary preparations when calling
// some wxPGProperty's member function.
#define wxPG_PROP_ID_CALL_PROLOG() \
    wxPGProperty *p = wxPGIdToPtr(id); \
    wxCHECK_RET( p, wxT("invalid property id") );

#define wxPG_PROP_NAME_CALL_PROLOG() \
    wxPGProperty *p = wxPGIdToPtr(GetPropertyByNameI(name)); \
    if ( !p ) return;

#define wxPG_PROP_ID_CALL_PROLOG_RETVAL(RVAL) \
    wxPGProperty *p = wxPGIdToPtr(id); \
    wxCHECK_MSG( p, RVAL, wxT("invalid property id") );

#define wxPG_PROP_NAME_CALL_PROLOG_RETVAL(RVAL) \
    wxPGProperty *p = wxPGIdToPtr(GetPropertyByNameI(name)); \
    if ( !p ) return RVAL;

// GetPropertyName version used internally. Use GetPropertyName for slight speed advantage,
// or GetPropertyNameA for nice assertion (essential for wxPython bindings).
#define GetPropertyByNameI          GetPropertyByNameA

// FOR BACKWARDS COMPATIBILITY
#define GetPropertyByNameWithAssert GetPropertyByNameA


/** \class wxPropertyContainerMethods
    \ingroup classes
    \brief In order to have most same base methods, both wxPropertyGrid and
    wxPropertyGridManager must derive from this.
*/
class WXDLLIMPEXP_PG wxPropertyContainerMethods
// BM_METHODS
{
public:

    /** Destructor */
    virtual ~wxPropertyContainerMethods() { };

    /** Adds choice to a property that can accept one.
        \remarks
        - If you need to make sure that you modify only the set of choices of
          a single property (and not also choices of other properties with initially
          identical set), call wxPropertyGrid::SetPropertyChoicesPrivate.
        - This usually only works for wxEnumProperty and derivatives (wxFlagsProperty
          can get accept new items but its items may not get updated).
    */
    void AddPropertyChoice( wxPGId id, const wxString& label, int value = wxPG_INVALID_VALUE );
    inline void AddPropertyChoice( wxPGPropNameStr name, const wxString& label, int value = wxPG_INVALID_VALUE )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        AddPropertyChoice(wxPGIdGen(p),label,value);
    }

    /** Inorder to add new items into a property with fixed children (for instance, wxFlagsProperty),
        you need to call this method. After populating has been finished, you need to call EndAddChildren.
    */
    void BeginAddChildren( wxPGId id );
    inline void BeginAddChildren( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        BeginAddChildren(wxPGIdGen(p));
    }

    /** Called after population of property with fixed children has finished.
    */
    void EndAddChildren( wxPGId id );
    inline void EndAddChildren( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        EndAddChildren(wxPGIdGen(p));
    }

    /** Inserts choice to a property that can accept one.

        See AddPropertyChoice for more details.
    */
    void InsertPropertyChoice( wxPGId id, const wxString& label, int index, int value = wxPG_INVALID_VALUE );
    inline void InsertPropertyChoice( wxPGPropNameStr name, const wxString& label, int index, int value = wxPG_INVALID_VALUE )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        InsertPropertyChoice(wxPGIdGen(p),label,index,value);
    }

    /** Deletes choice from a property.

        If selected item is deleted, then the value is set to unspecified.

        See AddPropertyChoice for more details.
    */
    void DeletePropertyChoice( wxPGId id, int index );
    inline void DeletePropertyChoice( wxPGPropNameStr name, int index )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        DeletePropertyChoice(wxPGIdGen(p),index);
    }

    /** Constructs a property. Class used is given as the first
        string argument. It may be either normal property class
        name, such as "wxIntProperty" or a short one such as
        "Int".
    */
    static wxPGProperty* CreatePropertyByClass(const wxString &classname,
                                               const wxString &label,
                                               const wxString &name);

    /** Constructs a property. Value type name used is given as the first
        string argument. It may be "string", "long", etc. Any value returned
        by wxVariant::GetType fits there.

        Otherwise, this is similar as CreatePropertyByClass.
        \remarks
        <b>Cannot</b> generate property category.
    */
    static wxPGProperty* CreatePropertyByType(const wxString &valuetype,
                                              const wxString &label,
                                              const wxString &name);

    /** Deletes a property by id. If category is deleted, all children are automatically deleted as well. */
    void Delete( wxPGId id );

    /** Deletes a property by name. */
    inline void Delete( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        Delete( wxPGIdGen(p) );
    }

    /** Returns id of first child of given property.
        \remarks
        Does not return sub-properties!
    */
    inline wxPGId GetFirstChild( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)
        wxPGPropertyWithChildren* pwc = (wxPGPropertyWithChildren*) p;
        if ( pwc->GetParentingType()==0 || pwc->GetParentingType()==-1 || !pwc->GetCount() )
            return wxNullProperty;
        return wxPGIdGen(pwc->Item(0));
    }
    inline wxPGId GetFirstChild( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return GetFirstChild( wxPGIdGen(p) );
    }

    /** Returns next item under the same parent. */
    inline wxPGId GetNextSibling( wxPGId id )
    {
        return wxPropertyGridState::GetNextSibling(id);
    }
    inline wxPGId GetNextSibling( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return wxPropertyGridState::GetNextSibling(wxPGIdGen(p));
    }

    /** Returns comma-delimited string with property's attributes (both
        pseudo-attributes such as "Disabled" and "Modified" and real
        attributes such as "BoolUseCheckbox" - actual names may vary).
        \param flagmask
        Combination of property flags that should be included (in addition
        to any other attributes). For example, to avoid adding Modified
        attribute use ~(wxPG_PROP_MODIFIED).
        \remarks
        Atleast in 1.2.x and earlier this does not return complete list of attributes
        (for example, no floating point precision) and some attributes have
        generic names (such as "Special1" instead of "UseCheckbox" etc)
    */
    inline wxString GetPropertyAttributes( wxPGId id, unsigned int flagmask = 0xFFFF ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetAttributes(flagmask);
    }

    /** Sets attributes from a string generated by GetPropertyAttributes.
        \remarks
        Performance may not be top-notch.
    */
    inline static void SetPropertyAttributes( wxPGId id, const wxString& attributes )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetAttributes(attributes);
    }

    inline void SetPropertyAttributes( wxPGPropNameStr name, const wxString& attributes ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetAttributes(attributes);
    }

    /** Returns id of property with given name (case-sensitive). If there is no
        property with such name, returned property id is invalid ( i.e. it will return
        false with IsOk method).
        \remarks
        - Sub-properties (i.e. properties which have parent that is not category or
          root) can not be accessed globally by their name. Instead, use
          "<property>.<subproperty>" in place of "<subproperty>".
    */
    wxPGId GetPropertyByName( wxPGPropNameStr name ) const;

    /** Returns id of a sub-property 'subname' of property 'name'. Same as
        calling GetPropertyByNameI(wxT("name.subname")), albeit slightly faster.
    */
    wxPGId GetPropertyByName( wxPGPropNameStr name, wxPGPropNameStr subname ) const;

    /** Returns writable reference to property's list of choices (and relevant
        values). If property does not have any choices, will return reference
        to an invalid set of choices that will return false on IsOk call.
    */
    wxPGChoices& GetPropertyChoices( wxPGId id );
    wxPGChoices& GetPropertyChoices( wxPGPropNameStr name );

    /** Gets name of property's constructor function. */
    inline wxPG_CONST_WXCHAR_PTR GetPropertyClassName( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxPG_CONST_WXCHAR_DEFVAL)
        return p->GetClassName();
    }

    /** Gets name of property's constructor function. */
    inline wxPG_CONST_WXCHAR_PTR GetPropertyClassName( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxPG_CONST_WXCHAR_DEFVAL)
        return p->GetClassName();
    }

#if wxPG_USE_CLIENT_DATA
    /** Returns client data (void*) of a property. */
    inline wxPGProperty::ClientDataType GetPropertyClientData( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(NULL)
        return p->GetClientData();
    }
    /** Returns client data (void*) of a property. */
    inline wxPGProperty::ClientDataType GetPropertyClientData( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(NULL)
        return p->GetClientData();
    }
#endif

    /** Returns property's editor. */
    inline const wxPGEditor* GetPropertyEditor( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(NULL)
        return p->GetEditorClass();
    }

    inline const wxPGEditor* GetPropertyEditor( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(NULL)
        return p->GetEditorClass();
    }

    /** Returns property's custom value image (NULL of none). */
    inline wxBitmap* GetPropertyImage( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(NULL)
        if ( p->GetDataExt() )
            return p->GetDataExt()->m_valueBitmap;
        return (wxBitmap*) NULL;
    }

    inline wxBitmap* GetPropertyImage( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(NULL)
        return GetPropertyImage(wxPGIdGen(p));
    }

    /** Returns property's position under its parent. */
    inline unsigned int GetPropertyIndex( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(INT_MAX)
        return p->GetIndexInParent();
    }

    /** Returns property's position under its parent. */
    inline unsigned int GetPropertyIndex( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(INT_MAX)
        return p->GetIndexInParent();
    }

    /** Returns label of a property. */
    inline const wxString& GetPropertyLabel( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetLabel();
    }
    inline const wxString& GetPropertyLabel( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetLabel();
    }

    /** Returns name of a property. Note that obviously there is no name-version
        of this member function. */
    inline const wxString& GetPropertyName( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetName();
    }

    /** Returns parent item of a property. */
    inline wxPGId GetPropertyParent( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)
        return p->GetParent();
    }

    /** Returns parent item of a property. */
    inline wxPGId GetPropertyParent( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return p->GetParent();
    }

    /** Returns priority of a property (wxPG_HIGH or wxPG_LOW). */
    inline int GetPropertyPriority( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxPG_HIGH)
        if ( p && p->IsFlagSet(wxPG_PROP_HIDEABLE) )
            return wxPG_LOW;
        return wxPG_HIGH;
    }

    /** Returns priority of a property (wxPG_HIGH or wxPG_LOW). */
    inline int GetPropertyPriority( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxPG_HIGH)
        return GetPropertyPriority(wxPGIdGen(p));
    }

    /** Returns pointer to a property.
    */
    inline wxPGProperty* GetPropertyPtr( wxPGId id ) const { return wxPGIdToPtr(id); }

    /** Returns pointer to a property.
    */
    inline wxPGProperty* GetPropertyPtr( wxPGPropNameStr name ) const
    {
        return wxPGIdToPtr(GetPropertyByName(name));
    }

    /** Returns help string associated with a property. */
    inline wxString GetPropertyHelpString( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetHelpString();
    }

    /** Returns help string associated with a property. */
    inline wxString GetPropertyHelpString( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(m_emptyString)
        return p->GetHelpString();
    }

    /** Returns short name for property's class. For example,
        "wxPropertyCategory" translates to "Category" and "wxIntProperty"
        to "Int".
    */
    wxPG_PYTHON_STATIC wxString GetPropertyShortClassName( wxPGId id );

#if wxUSE_VALIDATORS
    /** Returns validator of a property as a reference, which you
        can pass to any number of SetPropertyValidator.
    */
    inline wxValidator* GetPropertyValidator( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(NULL)
        return p->GetValidator();
    }
    inline wxValidator* GetPropertyValidator( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(NULL)
        return p->GetValidator();
    }
#endif

#ifndef SWIG
    /** Returns value as wxVariant. To get wxObject pointer from it,
        you will have to use WX_PG_VARIANT_TO_WXOBJECT(VARIANT,CLASSNAME) macro.

        If property value is unspecified, Null variant is returned.
    */
    inline wxVariant GetPropertyValue( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxVariant())
        return p->GetValueAsVariant();
    }

    /** Returns value as wxVariant. To get wxObject pointer from it,
        you will have to use WX_PG_VARIANT_TO_WXOBJECT(VARIANT,CLASSNAME) macro.

        If property value is unspecified, Null variant is returned.
    */
    inline wxVariant GetPropertyValue( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxVariant())
        return p->GetValueAsVariant();
    }
#endif

    wxPG_PYTHON_STATIC wxString GetPropertyValueAsString( wxPGId id ) wxPG_GETVALUE_CONST;
    wxPG_PYTHON_STATIC long GetPropertyValueAsLong( wxPGId id ) wxPG_GETVALUE_CONST;
#ifndef SWIG
    wxPG_PYTHON_STATIC inline int GetPropertyValueAsInt( wxPGId id ) wxPG_GETVALUE_CONST { return (int)GetPropertyValueAsLong(id); }
#endif
    wxPG_PYTHON_STATIC bool GetPropertyValueAsBool( wxPGId id ) wxPG_GETVALUE_CONST;
    wxPG_PYTHON_STATIC double GetPropertyValueAsDouble( wxPGId id ) wxPG_GETVALUE_CONST;
    wxPG_PYTHON_STATIC const wxObject* GetPropertyValueAsWxObjectPtr( wxPGId id ) wxPG_GETVALUE_CONST;
    wxPG_PYTHON_STATIC void* GetPropertyValueAsVoidPtr( wxPGId id ) wxPG_GETVALUE_CONST;

#define wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(TYPENAME, DEFVAL) \
    wxPG_PROP_ID_CALL_PROLOG_RETVAL(DEFVAL) \
    if ( wxStrcmp(p->GetValueTypePtr()->GetCustomTypeName(),TYPENAME) != 0 ) \
    { \
        wxPGGetFailed(p,TYPENAME); \
        return DEFVAL; \
    }

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC const wxArrayString& GetPropertyValueAsArrayString( wxPGId id ) wxPG_GETVALUE_CONST;
#else
    wxPG_PYTHON_STATIC inline wxArrayString GetPropertyValueAsArrayString( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("arrstring"), wxArrayString())
        return wxPGVariantToArrayString(p->DoGetValue());
    }
#endif

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC inline const wxPoint& GetPropertyValueAsPoint( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxPoint"), *((const wxPoint*)NULL))
        return wxPGVariantToWxPoint(p->DoGetValue());
    }
#else
    wxPG_PYTHON_STATIC inline wxPoint GetPropertyValueAsPoint( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxPoint"), wxPoint())
        return wxPGVariantToWxPoint(p->DoGetValue());
    }
#endif

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC inline const wxSize& GetPropertyValueAsSize( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxSize"), *((const wxSize*)NULL))
        return wxPGVariantToWxSize(p->DoGetValue());
    }
#else
    wxPG_PYTHON_STATIC inline wxSize GetPropertyValueAsSize( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxSize"), wxSize())
        return wxPGVariantToWxSize(p->DoGetValue());
    }
#endif

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC inline const wxLongLong& GetPropertyValueAsLongLong( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxLongLong"), *((const wxLongLong*)NULL))
        return wxPGVariantToWxLongLong(p->DoGetValue());
    }
#else
    wxPG_PYTHON_STATIC inline wxLongLong GetPropertyValueAsLongLong( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxLongLong"), wxLongLong())
        return wxPGVariantToWxLongLong(p->DoGetValue());
    }
#endif

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC inline const wxULongLong& GetPropertyValueAsULongLong( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxULongLong"), *((const wxULongLong*)NULL))
        return wxPGVariantToWxULongLong(p->DoGetValue());
    }
#else
    wxPG_PYTHON_STATIC inline wxULongLong GetPropertyValueAsULongLong( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxULongLong"), wxULongLong())
        return wxPGVariantToWxULongLong(p->DoGetValue());
    }
#endif

#if !wxPG_PGVARIANT_IS_VARIANT
    wxPG_PYTHON_STATIC inline const wxArrayInt& GetPropertyValueAsArrayInt( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxArrayInt"), wxPG_EMPTY_ARRAYINT)
        return wxPGVariantToArrayInt(p->DoGetValue());
    }
#else
    wxPG_PYTHON_STATIC inline wxArrayInt GetPropertyValueAsArrayInt( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_GETPROPVAL_CALL_PROLOG_RETVAL(wxT("wxArrayInt"), wxArrayInt())
        wxArrayInt arr = wxPGVariantToArrayInt(p->DoGetValue());
        return arr;
    }
#endif

#if wxUSE_DATETIME
    wxPG_PYTHON_STATIC inline wxDateTime GetPropertyValueAsDateTime( wxPGId id ) wxPG_GETVALUE_CONST
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxDateTime())

        if ( wxStrcmp(p->GetValueTypePtr()->GetCustomTypeName(),wxT("datetime")) != 0 )
        {
            wxPGGetFailed(p,wxT("datetime"));
            return wxDateTime();
        }
        return p->DoGetValue().GetDateTime();
    }
#endif

#ifdef __WXPYTHON__
    wxPG_PYTHON_STATIC PyObject* GetPropertyValueAsPyObject( wxPGId id ) wxPG_GETVALUE_CONST;
#endif

    inline wxString GetPropertyValueAsString( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsString( GetPropertyByNameI(name) );
    }
    inline long GetPropertyValueAsLong( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsLong( GetPropertyByNameI(name) );
    }
#ifndef SWIG
    inline int GetPropertyValueAsInt( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsInt( GetPropertyByNameI(name) );
    }
#endif
    inline bool GetPropertyValueAsBool( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsBool( GetPropertyByNameI(name) );
    }
    inline double GetPropertyValueAsDouble( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsDouble( GetPropertyByNameI(name) );
    }
    inline const wxObject* GetPropertyValueAsWxObjectPtr ( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsWxObjectPtr( GetPropertyByNameI(name) );
    }
#if !wxPG_PGVARIANT_IS_VARIANT
    inline const wxArrayString& GetPropertyValueAsArrayString ( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsArrayString( GetPropertyByNameI(name) );
    }
    inline const wxPoint& GetPropertyValueAsPoint( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsPoint( GetPropertyByNameI(name) );
    }
    inline const wxSize& GetPropertyValueAsSize( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsSize( GetPropertyByNameI(name) );
    }
    inline const wxArrayInt& GetPropertyValueAsArrayInt( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsArrayInt( GetPropertyByNameI(name) );
    }
#else
    inline wxArrayString GetPropertyValueAsArrayString ( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsArrayString( GetPropertyByNameI(name) );
    }
    inline wxPoint GetPropertyValueAsPoint( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsPoint( GetPropertyByNameI(name) );
    }
    inline wxSize GetPropertyValueAsSize( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsSize( GetPropertyByNameI(name) );
    }
    inline wxArrayInt GetPropertyValueAsArrayInt( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsArrayInt( GetPropertyByNameI(name) );
    }
#endif
#if wxUSE_DATETIME
    inline wxDateTime GetPropertyValueAsDateTime( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsDateTime( GetPropertyByNameI(name) );
    }
#endif
#ifdef __WXPYTHON__
    inline PyObject* GetPropertyValueAsPyObject( wxPGPropNameStr name ) const
    {
        return GetPropertyValueAsPyObject( GetPropertyByNameI(name) );
    }
#endif

    /** Returns a wxPGValueType class instance that describes
        the property's data type.
    */
    wxPG_VALUETYPE_MSGVAL GetPropertyValueType( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxPG_VALUETYPE(none))
        return p->GetValueType();
    }
    wxPG_VALUETYPE_MSGVAL GetPropertyValueType( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxPG_VALUETYPE(none))
        return p->GetValueType();
    }

    /** Returns property value type name.
    */
    inline wxString GetPVTN( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(m_emptyString)
        const wxPGValueType* vt = p->GetValueTypePtr();
        return vt->GetCustomTypeName();
    }

    inline wxString GetPVTN( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(m_emptyString)
        const wxPGValueType* vt = p->GetValueTypePtr();
        return vt->GetCustomTypeName();
    }

    /** Returns property value type identifier.
    */
    inline size_t GetPVTI( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(0)
        const wxPGValueType* vt = p->GetValueTypePtr();
        return size_t(vt);
    }

    inline size_t GetPVTI( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(0)
        const wxPGValueType* vt = p->GetValueTypePtr();
        return size_t(vt);
    }

#ifndef SWIG
    inline wxPropertyGridState* GetState() const { return m_pState; }
#endif

    /** Returns value type class instance for given type name.
    */
    static wxPGValueType* GetValueType( const wxString &type );

#if wxPG_VALUETYPE_IS_STRING
    /** Return value type class instance for given value type class name.
    */
    static wxPGValueType* GetValueTypeByName( const wxString &className );
#endif

    /** Hides or reveals a property.
        \param hide
        If true, hides property, otherwise reveals it.
        \remarks
        Hiding properties is not compatible with priority system. Using both
        at the same time will yield unpredictable results.
    */
    bool HideProperty( wxPGId id, bool hide = true );

    inline bool HideProperty( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return HideProperty(wxPGIdGen(p));
    }

#if wxPG_INCLUDE_ADVPROPS
    /** Initializes additional property editors (SpinCtrl etc.). Causes references
        to most object files in the library, so calling this may cause significant increase
        in executable size when linking with static library.
    */
    static void RegisterAdditionalEditors();
#else
    static inline void RegisterAdditionalEditors() { }
#endif

#if wxPG_INCLUDE_ADVPROPS
    /** Initializes *all* property types. Causes references to most object
        files in the library, so calling this may cause significant increase
        in executable size when linking with static library.
    */
    static void InitAllTypeHandlers();
#else
    static inline void InitAllTypeHandlers() { }
#endif

    /** Returns true if property is enabled. */
    inline bool IsPropertyEnabled( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return (!(p->GetFlags() & wxPG_PROP_DISABLED))?true:false;
    }

    /** Returns true if property is enabled. */
    inline bool IsPropertyEnabled( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return (!(p->GetFlags() & wxPG_PROP_DISABLED))?true:false;
    }

    /** Returns true if property is shown (ie. hideproperty with true not called for it). */
    inline bool IsPropertyShown( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return (!(p->GetFlags() & wxPG_PROP_HIDEABLE))?true:false;
    }

    /** Returns true if property is shown (ie. hideproperty with true not called for it). */
    inline bool IsPropertyShown( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return (!(p->GetFlags() & wxPG_PROP_HIDEABLE))?true:false;
    }

    /** Returns true if property's value type has name typestr. */
    inline bool IsPropertyValueType( wxPGId id, const wxChar* typestr )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return (wxStrcmp(p->GetValueTypePtr()->GetTypeName(),typestr) == 0);
    }

#if !wxPG_VALUETYPE_IS_STRING
    /** Returns true if property's value type is valuetype */
    inline bool IsPropertyValueType( wxPGId id, const wxPGValueType* valuetype )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return ( p->GetValueTypePtr() == valuetype );
    }
#endif

    /** Returns true if property's value type has same name as a class. */
    inline bool IsPropertyValueType( wxPGId id, const wxClassInfo* classinfo )
    {
        return IsPropertyValueType(id,classinfo->GetClassName());
    }

    /** Returns true if property's value type has name typestr. */
    inline bool IsPropertyValueType( wxPGPropNameStr name, const wxChar* typestr )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return (wxStrcmp(p->GetValueTypePtr()->GetTypeName(),typestr) == 0);
    }

#if !wxPG_VALUETYPE_IS_STRING
    /** Returns true if property's value type is valuetype */
    inline bool IsPropertyValueType( wxPGPropNameStr name, const wxPGValueType* valuetype )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return ( p->GetValueType() == valuetype );
    }
#endif

    /** Returns true if property's value type has same name as a class. */
    inline bool IsPropertyValueType( wxPGPropNameStr name, const wxClassInfo* classinfo )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return IsPropertyValueType(wxPGIdGen(p),classinfo->GetClassName());
    }

    /** Returns true if given property is expanded. Naturally, always returns false
        for properties that cannot be expanded.
    */
    static bool IsPropertyExpanded( wxPGId id );
    inline bool IsPropertyExpanded( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return IsPropertyExpanded(wxPGIdGen(p));
    }

    /** Returns true if property is of certain type.
        \param info
        Preferably use WX_PG_CLASSINFO(PROPERTYNAME). Alternative is
        PROPERTYNAMEClassInfo.
    */
    static inline bool IsPropertyKindOf( wxPGId id, wxPGPropertyClassInfo& info )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return p->IsKindOf(info);
    }
    inline bool IsPropertyKindOf( wxPGPropNameStr name, wxPGPropertyClassInfo& info )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return p->IsKindOf(info);
    }

    /** Returns true if property has been modified after value set or modify flag
        clear by software.

        NOTE: Try to use IsPropertyModified instead.
    */
    inline bool IsModified( wxPGId id ) const
    {
        return IsPropertyModified(id);
    }

    inline bool IsModified( wxPGPropNameStr name )
    {
        return IsPropertyModified(name);
    }

    /** Returns true if property is a category. */
    inline bool IsPropertyCategory( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return (p->GetParentingType()>0)?true:false;
    }

    inline bool IsPropertyCategory( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return (p->GetParentingType()>0)?true:false;
    }

    /** Returns true if property has been modified after value set or modify flag
        clear by software.
    */
    inline bool IsPropertyModified( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return ( (p->GetFlags() & wxPG_PROP_MODIFIED) ? true : false );
    }
    inline bool IsPropertyModified( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return ( (p->GetFlags() & wxPG_PROP_MODIFIED) ? true : false );
    }

    /** Returns true if property value is set to unspecified.
    */
#ifdef wxPG_COMPATIBILITY_1_0_0
    inline bool IsPropertyValueUnspecified( wxPGId id ) const
#else
    inline bool IsPropertyUnspecified( wxPGId id ) const
#endif
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return ( (p->GetFlags() & wxPG_PROP_UNSPECIFIED) ? true : false );
    }
#ifdef wxPG_COMPATIBILITY_1_0_0
    inline bool IsPropertyValueUnspecified( wxPGPropNameStr name )
    {
        return IsPropertyValueUnspecified(GetPropertyByNameI(name));
    }
#else
    inline bool IsPropertyUnspecified( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return IsPropertyUnspecified(wxPGIdGen(p));
    }
#endif

    /** Basic property classes are registered by the default, but this
        registers advanced ones as well.
    */
    static void RegisterAdvancedPropertyClasses();

    /** Registers property class info with specific name. Preferably use
        wxPGRegisterPropertyClass(PROPERTYNAME) macro.
    */
    static bool RegisterPropertyClass( const wxChar* name, wxPGPropertyClassInfo* classinfo );

    /** Replaces property with id with newly created property. For example,
        this code replaces existing property named "Flags" with one that
        will have different set of items:
        \code
            pg->ReplaceProperty(wxT("Flags"),
                wxFlagsProperty(wxT("Flags"),wxPG_LABEL,newItems))
        \endcode
        For more info, see wxPropertyGrid::Insert.
    */
    wxPGId ReplaceProperty( wxPGId id, wxPGProperty* property );

    inline wxPGId ReplaceProperty( wxPGPropNameStr name, wxPGProperty* property )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return ReplaceProperty(wxPGIdGen(p),property);
    }

    /** Lets user to set the strings listed in the choice dropdown of a wxBoolProperty.
        Defaults are "True" and "False", so changing them to, say, "Yes" and "No" may
        be useful in some less technical applications.
    */
    static void SetBoolChoices( const wxChar* true_choice, const wxChar* false_choice );

    /** Set choices of a property to specified set of labels and values.
    */
    static inline void SetPropertyChoices(wxPGId id, wxPGChoices& choices)
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetChoices(choices);
    }


    /** Set choices of a property to specified set of labels and values.
    */
    inline void SetPropertyChoices(wxPGPropNameStr name, wxPGChoices& choices)
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetChoices(choices);
    }

    /** If property's set of choices is shared, then calling this method converts
        it to private.
    */
    inline void SetPropertyChoicesExclusive( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetChoicesExclusive();
    }
    inline void SetPropertyChoicesExclusive( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetChoicesExclusive();
    }

    /** Sets an attribute of a property. Ids and relevants values are totally
        specific to property classes and may affect either the given instance
        or all instances of that class. See \ref attrids for list of built-in
        attributes.
        \param argFlags
        Optional. Use wxPG_RECURSE to set the attribute to child properties
        as well.
        \remarks
        wxVariant doesn't have int constructor (as of 2.5.4), so <b>you will
        need to cast int values (including most numeral constants) to long</b>.
    */
    inline void SetPropertyAttribute( wxPGId id, int attrid, wxVariant value, long argFlags = 0 )
    {
        DoSetPropertyAttribute(id,attrid,value,argFlags);
    }
    inline void SetPropertyAttribute( wxPGPropNameStr name, int attrid, wxVariant value, long argFlags = 0  )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        DoSetPropertyAttribute(wxPGIdGen(p),attrid,value,argFlags);
    }

#ifndef SWIG
    /** Sets editor control of a property. As editor argument, use
        wxPG_EDITOR(EditorName), where basic built-in editor names are TextCtrl, Choice,
        ComboBox, CheckBox, TextCtrlAndButton, and ChoiceAndButton. Additional editors
        include SpinCtrl and DatePickerCtrl, which also require wxPropertyGrid::RegisterAdditionalEditors()
        call prior using.
    */
    inline void SetPropertyEditor( wxPGId id, const wxPGEditor* editor )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        wxCHECK_RET( editor, wxT("unknown/NULL editor") );
        p->SetEditor(editor);
        RefreshProperty(p);
    }
    inline void SetPropertyEditor( wxPGPropNameStr name, const wxPGEditor* editor )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        wxCHECK_RET( editor, wxT("unknown/NULL editor") );
        p->SetEditor(editor);
        RefreshProperty(p);
    }
#endif // #ifndef SWIG

    /** Sets editor control of a property. As editor argument, use
        editor name string, such as wxT("TextCtrl") or wxT("Choice").
    */
    inline void SetPropertyEditor( wxPGId id, const wxString& editorName )
    {
        SetPropertyEditor(id,GetEditorByName(editorName));
    }
    inline void SetPropertyEditor( wxPGPropNameStr name, const wxString& editorName )
    {
        SetPropertyEditor(name,GetEditorByName(editorName));
    }

#if wxPG_USE_CLIENT_DATA
    /** Sets client data (void*) of a property.
        \remarks
        This untyped client data has to be deleted manually.
    */
    inline void SetPropertyClientData( wxPGId id, wxPGProperty::ClientDataType clientData )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetClientData(clientData);
    }
    /** Sets client data (void*) of a property.
        \remarks
        This untyped client data has to be deleted manually.
    */
    inline void SetPropertyClientData( wxPGPropNameStr name, wxPGProperty::ClientDataType clientData )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetClientData(clientData);
    }
#endif

    /** Associates the help string with property.
        \remarks
        By default, text is shown either in the manager's "description"
        text box or in the status bar. If extra window style wxPG_EX_HELP_AS_TOOLTIPS
        is used, then the text will appear as a tooltip.
    */
    inline void SetPropertyHelpString( wxPGId id, const wxString& helpString )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetHelpString(helpString);
    }

    inline void SetPropertyHelpString( wxPGPropNameStr name, const wxString& helpString )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetHelpString(helpString);
    }

    /** Set wxBitmap in front of the value.
        \remarks
        - Bitmap will be ignored if property class has implemented OnCustomPaint.
        - Bitmap will be scaled to a size returned by wxPropertyGrid::GetImageSize();
    */
    inline void SetPropertyImage( wxPGId id, wxBitmap& bmp )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetValueImage(bmp);
        RefreshProperty(p);
    }

    inline void SetPropertyImage( wxPGPropNameStr name, wxBitmap& bmp )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetValueImage(bmp);
        RefreshProperty(p);
    }

    /** Sets max length of property's text.
    */
    bool SetPropertyMaxLength( wxPGId id, int maxLen );

    /** Sets max length of property's text.
    */
    inline bool SetPropertyMaxLength( wxPGPropNameStr name, int maxLen )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return SetPropertyMaxLength(wxPGIdGen(p),maxLen);
    }

    /** Property is to be hidden/shown when hider button is toggled or
        when wxPropertyGrid::Compact is called.
    */
    bool SetPropertyPriority( wxPGId id, int priority );

    /** Property is to be hidden/shown when hider button is toggled or
        when wxPropertyGrid::Compact is called.
    */
    inline bool SetPropertyPriority( wxPGPropNameStr name, int priority )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return SetPropertyPriority(wxPGIdGen(p),priority);
    }

#if wxUSE_VALIDATORS
    /** Sets validator of a property. For example
        \code
          // Allow property's value range from -100 to 100
          wxIntPropertyValidator validator(-100,100);
          wxPGId id = pg->Append( wxIntProperty(wxT("Value 1",wxPG_LABEL,0)) );
          pg->SetPropertyValidator( id, validator );
        \endcode
    */
    inline void SetPropertyValidator( wxPGId id, const wxValidator& validator )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->SetValidator(validator);
    }
    inline void SetPropertyValidator( wxPGPropNameStr name, const wxValidator& validator )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        p->SetValidator(validator);
    }
#endif

    /** Toggles priority of a property between wxPG_HIGH and wxPG_LOW.
    */
    inline void TogglePropertyPriority( wxPGId id )
    {
        int priority = wxPG_LOW;
        if ( GetPropertyPriority(id) == wxPG_LOW )
            priority = wxPG_HIGH;
        SetPropertyPriority(id,priority);
    }

    /** Toggles priority of a property between wxPG_HIGH and wxPG_LOW.
    */
    inline void TogglePropertyPriority( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        TogglePropertyPriority(wxPGIdGen(p));
    }

#ifdef SWIG
    %pythoncode {
        def MapType(class_,factory):
            "Registers Python type/class to property mapping.\n\nfactory: Property builder function/class."
            global _type2property
            try:
                mappings = _type2property
            except NameError:
                raise AssertionError("call only after a propertygrid or manager instance constructed")

            mappings[class_] = factory


        def DoDefaultTypeMappings(self):
            "Map built-in properties."
            global _type2property
            try:
                mappings = _type2property

                return
            except NameError:
                mappings = {}
                _type2property = mappings

            mappings[str] = StringProperty
            mappings[unicode] = StringProperty
            mappings[int] = IntProperty
            mappings[float] = FloatProperty
            mappings[bool] = BoolProperty
            mappings[list] = ArrayStringProperty
            mappings[tuple] = ArrayStringProperty
            mappings[wx.Font] = FontProperty
            mappings[wx.Colour] = ColourProperty
            mappings[wx.Size] = SizeProperty
            mappings[wx.Point] = PointProperty
            mappings[wx.FontData] = FontDataProperty


        def GetPropertyValue(self,p):
            "Returns Python object value for property.\n\nCaches getters on value type id basis for performance purposes."
            global _vt2getter
            vtid = self.GetPVTI(p)
            if not vtid:
                raise TypeError("Property '%s' doesn't have valid value type"%(p.GetName()))
            try:
                getter = _vt2getter[vtid]
            except KeyError:

                cls = PropertyContainerMethods
                vtn = self.GetPVTN(p)

                if vtn == 'long':
                    getter = cls.GetPropertyValueAsLong
                elif vtn == 'string':
                    getter = cls.GetPropertyValueAsString
                elif vtn == 'double':
                    getter = cls.GetPropertyValueAsDouble
                elif vtn == 'bool':
                    getter = cls.GetPropertyValueAsBool
                elif vtn == 'arrstring':
                    getter = cls.GetPropertyValueAsArrayString
                elif vtn == 'wxArrayInt':
                    getter = cls.GetPropertyValueAsArrayInt
                elif vtn == 'PyObject':
                    getter = cls.GetPropertyValueAsPyObject
                elif vtn == 'datetime':
                    getter = cls.GetPropertyValueAsDateTime
                elif vtn == 'wxPoint':
                    getter = cls.GetPropertyValueAsPoint
                elif vtn == 'wxSize':
                    getter = cls.GetPropertyValueAsSize
                elif vtn.startswith('wx'):
                    getter = cls.GetPropertyValueAsWxObjectPtr
                elif not vtn:
                    if p:
                        raise ValueError("no property with name '%s'"%p)
                    else:
                        raise ValueError("NULL property")
                else:
                    raise AssertionError("Unregistered property grid value type '%s'"%vtn)
                _vt2getter[vtid] = getter
            return getter(self,p)


        def SetPropertyValueArrstr(self,p,v):
            "NB: We must implement this in Python because SWIG has problems combining"
            "    conversion of list to wxArrayXXX and overloaded arguments."
            if not isinstance(p,basestring):
                self._SetPropertyValueArrstr(p,v)
            else:
                self._SetPropertyValueArrstr(self.GetPropertyByNameI(p),v)


        def SetPropertyValueArrint(self,p,v):
            "NB: We must implement this in Python because SWIG has problems combining"
            "    conversion of list to wxArrayXXX and overloaded arguments."
            if not isinstance(p,basestring):
                self._SetPropertyValueArrint(p,v)
            else:
                self._SetPropertyValueArrint(self.GetPropertyByNameI(p),v)


        def SetPropertyValue(self,p,v):
            "Set property value from Python object.\n\nCaches setters on value type id basis for performance purposes."
            cls = self.__class__
            if not isinstance(v,basestring):
                _vt2setter = cls._vt2setter
                vtid = self.GetPVTI(p)
                try:
                    setter = _vt2setter[vtid]
                except KeyError:

                    vtn = self.GetPVTN(p)

                    if vtn == 'long':
                        setter = cls.SetPropertyValueLong
                    elif vtn == 'string':
                        setter = cls.SetPropertyValueString
                    elif vtn == 'double':
                        setter = cls.SetPropertyValueDouble
                    elif vtn == 'bool':
                        setter = cls.SetPropertyValueBool
                    elif vtn == 'arrstring':
                        setter = cls.SetPropertyValueArrstr
                    elif vtn == 'wxArrayInt':
                        setter = cls.SetPropertyValueArrint
                    elif vtn == 'PyObject':
                        setter = cls.SetPropertyValuePyObject
                    elif vtn == 'datetime':
                        setter = cls.SetPropertyValueDatetime
                    elif vtn == 'wxPoint':
                        setter = cls.SetPropertyValuePoint
                    elif vtn == 'wxSize':
                        setter = cls.SetPropertyValueSize
                    elif vtn == 'wxLongLong':
                        setter = cls.SetPropertyValueLongLong
                    elif vtn == 'wxULongLong':
                        setter = cls.SetPropertyValueULongLong
                    elif vtn.startswith('wx'):
                        setter = cls.SetPropertyValueWxObjectPtr
                    elif not vtn:
                        if p:
                            raise ValueError("no property with name '%s'"%p)
                        else:
                            raise ValueError("NULL property")
                    else:
                        raise AssertionError("Unregistered property grid value type '%s'"%vtn)
                    _vt2setter[vtid] = setter
            else:
                setter = cls.SetPropertyValueString

            return setter(self,p,v)


        def DoDefaultValueTypeMappings(self):
            "Map pg value type ids to getter methods."
            global _vt2getter
            try:
                vt2getter = _vt2getter

                return
            except NameError:
                vt2getter = {}
                _vt2getter = vt2getter


        def _GetValues(self,parent,fc,dict_,getter):
            p = fc

            while p:
                pfc = self.GetFirstChild(p)
                if pfc:
                    self._GetValues(p,pfc,dict_,getter)
                else:
                    dict_[p.GetName()] = getter(p)

                p = self.GetNextSibling(p)


        def GetPropertyValues(self,dict_=None,as_strings=False):
            "Returns values in the grid."
            ""
            "dict_: if not given, then a new one is created. dict_ can be"
            "  object as well, in which case it's __dict__ is used."
            "as_strings: if True, then string representations of values"
            "  are fetched instead of native types. Useful for config and such."
            ""
            "Return value: dictionary with values. It is always a dictionary,"
            "so if dict_ was object with __dict__ attribute, then that attribute"
            "is returned."

            if dict_ is None:
                dict_ = {}
            elif hasattr(dict_,'__dict__'):
                dict_ = dict_.__dict__

            if not as_strings:
                getter = self.GetPropertyValue
            else:
                getter = self.GetPropertyValueAsString

            root = self.GetRoot()
            self._GetValues(root,self.GetFirstChild(root),dict_,getter)

            return dict_

        GetValues = GetPropertyValues


        def SetPropertyValues(self,dict_):
            "Sets property values from dict_, which can be either\ndictionary or an object with __dict__ attribute."
            ""
            "autofill: If true, keys with not relevant properties"
            "  are auto-created. For more info, see AutoFill."
            ""
            "Notes:"
            "  * Keys starting with underscore are ignored."

            autofill = False

            if dict_ is None:
                dict_ = {}
            elif hasattr(dict_,'__dict__'):
                dict_ = dict_.__dict__

            def set_sub_obj(k0,dict_):
                for k,v in dict_.iteritems():
                    if k[0] != '_':
                        try:
                            self.SetPropertyValue(k,v)
                        except:
                            try:
                                if autofill:
                                    self._AutoFillOne(k0,k,v)
                                    continue
                            except:
                                if isinstance(v,dict):
                                    set_sub_obj(k,v)
                                elif hasattr(v,'__dict__'):
                                    set_sub_obj(k,v.__dict__)


            cur_page = False
            is_manager = isinstance(self,PropertyGridManager)

            try:
                set_sub_obj(self.GetRoot(),dict_)
            except:
                import traceback
                traceback.print_exc()

            self.Refresh()


        SetValues = SetPropertyValues


        def _AutoFillMany(self,cat,dict_):
            for k,v in dict_.iteritems():
                self._AutoFillOne(cat,k,v)


        def _AutoFillOne(self,cat,k,v):
            global _type2property

            factory = _type2property.get(v.__class__,None)

            if factory:
                self.AppendIn( cat, factory(k,k,v) )
            elif hasattr(v,'__dict__'):
                cat2 = self.AppendIn( cat, PropertyCategory(k) )
                self._AutoFillMany(cat2,v.__dict__)
            elif isinstance(v,dict):
                cat2 = self.AppendIn( cat, PropertyCategory(k) )
                self._AutoFillMany(cat2,v)
            elif not k.startswith('_'):
                raise AssertionError("member '%s' is of unregisted type/class '%s'"%(k,v.__class__))


        def AutoFill(self,obj,parent=None):
            "Clears properties and re-fills to match members and\nvalues of given object or dictionary obj."

            self.edited_objects[parent] = obj

            cur_page = False
            is_manager = isinstance(self,PropertyGridManager)

            if not parent:
                if is_manager:
                    page = self.GetTargetPage()
                    self.ClearPage(page)
                    parent = self.GetPageRoot(page)
                else:
                    self.Clear()
                    parent = self.GetRoot()
            else:
                p = self.GetFirstChild(parent)
                while p:
                    self.Delete(p)
                    p = self.GetNextSibling(p)

            if not is_manager or page == self.GetSelectedPage():
                self.Freeze()
                cur_page = True

            try:
                self._AutoFillMany(parent,obj.__dict__)
            except:
                import traceback
                traceback.print_exc()

            if cur_page:
                self.Thaw()


        def RegisterEditor(self, editor, editorName=None):
            "Transform class into instance, if necessary."
            if not isinstance(editor, PGEditor):
                editor = editor()
            if not editorName:
                editorName = editor.__class__.__name__
            try:
                self._editor_instances.append(editor)
            except:
                self._editor_instances = [editor]
            RegisterEditor(editor, editorName)

    }
#endif

	/** Sets property as read-only. It's value cannot be changed by the user, but the
	    editor may still be created for copying purposes.
	*/
	void SetPropertyReadOnly( wxPGId id, bool readOnly = true )
	{
        wxPG_PROP_ID_CALL_PROLOG()
		if ( readOnly )
			p->SetFlag(wxPG_PROP_READONLY);
		else
			p->ClearFlag(wxPG_PROP_READONLY);
	}

	/** Sets property as read-only. It's value cannot be changed by the user, but the
	    editor may still be created for copying purposes.
	*/
	void SetPropertyReadOnly( wxPGPropNameStr name, bool readOnly = true )
	{
        wxPG_PROP_NAME_CALL_PROLOG()
		if ( readOnly )
			p->SetFlag(wxPG_PROP_READONLY);
		else
			p->ClearFlag(wxPG_PROP_READONLY);
	}

    // GetPropertyByNameI With nice assertion error message.
    wxPGId GetPropertyByNameA( wxPGPropNameStr name ) const;

#ifndef SWIG

    static wxPGEditor* GetEditorByName( const wxString& editorName );
    
protected:

    // Deriving classes must set this (it must be only or current page).
    wxPropertyGridState*         m_pState;

    // Default call's m_pState's BaseGetPropertyByName
    virtual wxPGId DoGetPropertyByName( wxPGPropNameStr name ) const;

    virtual void RefreshProperty( wxPGProperty* p ) = 0;

    // Intermediate version needed due to wxVariant copying inefficiency
    static void DoSetPropertyAttribute( wxPGId id, int attrid, wxVariant& value, long argFlags );

    // Empty string object to return from member functions returning const wxString&.
    wxString                    m_emptyString;

#endif // #ifndef SWIG
};

// -----------------------------------------------------------------------


// wxPropertyGrid::DoSelectProperty flags
#define wxPG_SEL_FOCUS      0x01 // Focuses to created editor
#define wxPG_SEL_FORCE      0x02 // Forces deletion and recreation of editor
#define wxPG_SEL_NONVISIBLE 0x04 // For example, doesn't cause EnsureVisible
#define wxPG_SEL_NOVALIDATE 0x08 // Do not validate editor's value before selecting
#define wxPG_SEL_DELETING   0x10 // Property being deselected is about to be deleted
#define wxPG_SEL_SETUNSPEC  0x20 // Property's values was set to unspecified by the user


// -----------------------------------------------------------------------

#ifndef SWIG

// Internal flags
#define wxPG_FL_INITIALIZED                 0x0001
#define wxPG_FL_ACTIVATION_BY_CLICK         0x0002 // Set when creating editor controls if it was clicked on.
#define wxPG_FL_DONT_CENTER_SPLITTER        0x0004
#define wxPG_FL_FOCUSED                     0x0008
#define wxPG_FL_MOUSE_CAPTURED              0x0010
#define wxPG_FL_MOUSE_INSIDE                0x0020
#define wxPG_FL_VALUE_MODIFIED              0x0040
#define wxPG_FL_PRIMARY_FILLS_ENTIRE        0x0080 // don't clear background of m_wndPrimary
#define wxPG_FL_CUR_USES_CUSTOM_IMAGE       0x0100 // currently active editor uses custom image
#define wxPG_FL_HIDE_STATE                  0x0200 // set when hideable properties should be hidden
#define wxPG_FL_SCROLLED                    0x0400
#define wxPG_FL_ADDING_HIDEABLES            0x0800 // set when all added/inserted properties get hideable flag
#define wxPG_FL_NOSTATUSBARHELP             0x1000 // Disables showing help strings on statusbar.
#define wxPG_FL_CREATEDSTATE                0x2000 // Marks that we created the state, so we have to destroy it too.
#define wxPG_FL_SCROLLBAR_DETECTED          0x4000 // Set if scrollbar's existence was detected in last onresize.
#define wxPG_FL_DESC_REFRESH_REQUIRED       0x8000 // Set if wxPGMan requires redrawing of description text box.
#define wxPG_FL_SELECTED_IS_PAINT_FLEXIBLE  0x00010000 // Set if selected has flexible imagesize
#define wxPG_FL_IN_MANAGER                  0x00020000 // Set if contained in wxPropertyGridManager
#define wxPG_FL_GOOD_SIZE_SET               0x00040000 // Set after wxPropertyGrid is shown in its initial good size
#define wxPG_FL_IGNORE_NEXT_NAVKEY          0x00080000 // Next navigation key event will get ignored
#define wxPG_FL_IN_SELECT_PROPERTY          0x00100000 // Set when in SelectProperty.
#define wxPG_FL_STRING_IN_STATUSBAR         0x00200000 // Set when help string is shown in status bar
#define wxPG_FL_SPLITTER_PRE_SET            0x00400000 // Splitter position has been custom-set by the user
#define wxPG_FL_VALIDATION_FAILED           0x00800000 // Validation failed. Clear on modify event.
#define wxPG_FL_SELECTED_IS_FULL_PAINT      0x01000000 // Set if selected is fully painted (ie. both image and text)
#define wxPG_MAN_FL_PAGE_INSERTED           0x02000000 // Set after page has been inserted to manager
#define wxPG_FL_ABNORMAL_EDITOR             0x04000000 // Active editor control is abnormally large

#endif // #ifndef SWIG


// -----------------------------------------------------------------------

#define wxPG_USE_STATE  m_pState

/** \class wxPropertyGrid
	\ingroup classes
    \brief
    wxPropertyGrid is a specialized two-column grid for editing properties
    such as strings, numbers, flagsets, fonts, and colours. wxPropertySheet
    used to do the very same thing, but it hasn't been updated for a while
    and it is currently deprecated.

    wxPropertyGrid is modeled after .NET propertygrid (hence the name),
    and thus features are similar. However, inorder to keep the widget lightweight,
    it does not (and will not) have toolbar for mode and page selection, nor the help
    text box. wxAdvancedPropertyGrid (or something similarly named) is planned to have
    these features in some distant future.

    <h4>Derived from</h4>

    wxPropertyContainerMethods\n
    wxScrolledWindow\n
    wxPanel\n
    wxWindow\n
    wxEvtHandler\n
    wxObject\n

    <h4>Include files</h4>

    <wx/propertygrid/propertygrid.h>

    <h4>Window styles</h4>

    @link wndflags Additional Window Styles@endlink

    <h4>Event handling</h4>

    To process input from a propertygrid control, use these event handler macros to
    direct input to member functions that take a wxPropertyGridEvent argument.

    <table>
    <tr><td>EVT_PG_SELECTED (id, func)</td><td>Property is selected.</td></tr>
    <tr><td>EVT_PG_CHANGED (id, func)</td><td>Property value is modified.</td></tr>
    <tr><td>EVT_PG_HIGHLIGHTED (id, func)</td><td>Mouse moves over property. Event's property is NULL if hovered on area that is not a property.</td></tr>
    <tr><td>EVT_PG_RIGHT_CLICK (id, func)</td><td>Mouse right-clicked on a property.</td></tr>
    <tr><td>EVT_PG_DOUBLE_CLICK (id, func)</td><td>Mouse double-clicked on a property.</td></tr>
    <tr><td>EVT_PG_ITEM_COLLAPSED (id, func)</td><td>User collapses a property or category.</td></tr>
    <tr><td>EVT_PG_ITEM_EXPANDED (id, func)</td><td>User expands a property or category.</td></tr>
    <tr><td>EVT_BUTTON (id, func)</td><td>Button in a property editor was clicked. Only occurs if the property doesn't handle button clicks itself.</td></tr>
    <tr><td>EVT_TEXT (id, func)</td><td>wxTextCtrl based editor was updated (but property value was not yet modified)</td></tr>
    </table>

    \sa @link wxPropertyGridEvent wxPropertyGridEvent@endlink

    \remarks

    - Following functions do not automatically update the screen: Append. You
      probably need to explicitly call Refresh() <b>if</b> you called one of these
      functions outside parent window constructor.

    - Use Freeze() and Thaw() respectively to disable and enable drawing. This
      will also delay sorting etc. miscellaneous calculations to the last possible
      moment.

    - Most methods have two versions - one which accepts property id (faster) and
      another that accepts property name (which is a bit slower since it does a hashmap
      lookup).

    For code examples, see the main page.

*/
// BM_GRID
class WXDLLIMPEXP_PG wxPropertyGrid : public wxScrolledWindow, public wxPropertyContainerMethods
{
#ifndef SWIG
    friend class wxPropertyGridState;
    friend class wxPropertyContainerMethods;
    friend class wxPropertyGridManager;

    DECLARE_CLASS(wxPropertyGrid)
#endif

public:
	/** Two step constructor. Call Create when this constructor is called to build up the
	    wxPropertyGrid
	*/

#ifdef SWIG
    %pythonAppend wxPropertyGrid {
        self._setOORInfo(self)
        self.DoDefaultTypeMappings()
        self.edited_objects = {}
        self.DoDefaultValueTypeMappings()
        if not hasattr(self.__class__,'_vt2setter'):
            self.__class__._vt2setter = {}
    }
    %pythonAppend wxPropertyGrid() ""

    wxPropertyGrid( wxWindow *parent, wxWindowID id = wxID_ANY,
               	    const wxPoint& pos = wxDefaultPosition,
               	    const wxSize& size = wxDefaultSize,
               	    long style = wxPG_DEFAULT_STYLE,
               	    const wxChar* name = wxPyPropertyGridNameStr );
    %RenameCtor(PrePropertyGrid,  wxPropertyGrid());

#else

    wxPropertyGrid();

    /** The default constructor. The styles to be used are styles valid for
        the wxWindow and wxScrolledWindow.
        \sa @link wndflags Additional Window Styles@endlink
    */
    wxPropertyGrid( wxWindow *parent, wxWindowID id = wxID_ANY,
               	    const wxPoint& pos = wxDefaultPosition,
               	    const wxSize& size = wxDefaultSize,
               	    long style = wxPG_DEFAULT_STYLE,
               	    const wxChar* name = wxPropertyGridNameStr );

    /** Destructor */
    virtual ~wxPropertyGrid();
#endif

    /** Appends property to the list. wxPropertyGrid assumes ownership of the object.
        Becomes child of most recently added category.
        \remarks
        - wxPropertyGrid takes the ownership of the property pointer.
        - If appending a category with name identical to a category already in the
          wxPropertyGrid, then newly created category is deleted, and most recently
          added category (under which properties are appended) is set to the one with
          same name. This allows easier adding of items to same categories in multiple
          passes.
        - Does not automatically redraw the control, so you may need to call Refresh
          when calling this function after control has been shown for the first time.
    */
    wxPGId Append( wxPGProperty* property );

    inline wxPGId AppendCategory( const wxString& label, const wxString& name = wxPG_LABEL )
    {
        return Append( new wxPropertyCategoryClass(label,name) );
    }

#ifndef SWIG
#if wxPG_INCLUDE_BASICPROPS
    inline wxPGId Append( const wxString& label, const wxString& name = wxPG_LABEL, const wxString& value = wxEmptyString )
    {
        return Append( wxStringProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name = wxPG_LABEL, int value = 0 )
    {
        return Append( wxIntProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name = wxPG_LABEL, double value = 0.0 )
    {
        return Append( wxFloatProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name = wxPG_LABEL, bool value = false )
    {
        return Append( wxBoolProperty(label,name,value) );
    }
#endif
#endif

    inline wxPGId AppendIn( wxPGId id, wxPGProperty* property )
    {
        return Insert(id,-1,property);
    }

    inline wxPGId AppendIn( wxPGPropNameStr name, wxPGProperty* property )
    {
        return Insert(GetPropertyByNameI(name),-1,property);
    }

    inline wxPGId AppendIn( wxPGId id, const wxString& label, const wxString& propname, wxVariant& value )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)
        return m_pState->AppendIn( (wxPGPropertyWithChildren*)p, label, propname, value );
    }

    inline wxPGId AppendIn( wxPGPropNameStr name, const wxString& label, const wxString& propname, wxVariant& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return m_pState->AppendIn( (wxPGPropertyWithChildren*)p, label, propname, value );
    }

    /** This static function enables or disables automatic use of wxGetTranslation for
        following strings: wxEnumProperty list labels, wxFlagsProperty sub-property
        labels.
        Default is false.
    */
    static void AutoGetTranslation( bool enable );

    /** Returns true if all property grid data changes have been committed. Usually
        only returns false if value in active editor has been invalidated by a
        wxValidator.
    */
    inline bool CanClose()
    {
        return DoEditorValidate();
    }

    /** Returns true if all property grid data changes have been committed. Usually
        only returns false if value in active editor has been invalidated by a
        wxValidator.
    */
    inline bool EditorValidate()
    {
        return DoEditorValidate();
    }

    /** Centers the splitter. If argument is true, automatic splitter centering is
        enabled (only applicapple if style wxPG_SPLITTER_AUTO_CENTER was defined).
    */
    void CenterSplitter( bool enable_auto_centering = false );

    /** Two step creation. Whenever the control is created without any parameters, use Create to actually
        create it. Don't access the control's public methods before this is called
        \sa @link wndflags Additional Window Styles@endlink
    */
    bool Create( wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxPG_DEFAULT_STYLE,
                 const wxChar* name = wxPropertyGridNameStr );

    /** Deletes all properties. Does not free memory allocated for arrays etc.
        This should *not* be called in wxPropertyGridManager.
    */
    void Clear();

    /** Resets modified status of a property and all sub-properties.
    */
    inline void ClearModifiedStatus( wxPGId id )
    {
        m_pState->ClearModifiedStatus(wxPGIdToPtr(id));
    }

    /** Resets modified status of all properties.
    */
    inline void ClearModifiedStatus()
    {
        m_pState->ClearModifiedStatus(m_pState->m_properties);
        m_pState->m_anyModified = false;
    }

    /** Resets value of a property to its default. */
    bool ClearPropertyValue( wxPGId id );

    /** Resets value of a property to its default. */
    inline bool ClearPropertyValue( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return ClearPropertyValue( wxPGIdGen(p) );
    }

    /** Deselect current selection, if any. Returns true if success
        (ie. validator did not intercept). */
    bool ClearSelection();

    /** Synonymous to Clear.
    */
    inline void ClearTargetPage()
    {
        Clear();
    }

    /** Collapses given category or property with children.
        Returns true if actually collapses.
    */
    inline bool Collapse( wxPGId id )
    {
        return _Collapse(wxPGIdToPtr(id));
    }

    /** Collapses given category or property with children.
        Returns true if actually collapses.
    */
    inline bool Collapse( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return _Collapse(p);
    }

    /** Collapses all items that can be collapsed.
        \retval
        Return false if failed (may fail if editor value cannot be validated).
    */
    inline bool CollapseAll() { return m_pState->ExpandAll(0); }

    /** Shows(arg = false) or hides(arg = true) all hideable properties. */
    bool Compact( bool compact );

    /** Disables property. */
    inline bool Disable( wxPGId id ) { return EnableProperty(id,false); }

    /** Disables property. */
    inline bool Disable( wxPGPropNameStr name ) { return EnableProperty(name,false); }

    /** Disables property. */
    inline bool DisableProperty( wxPGId id ) { return EnableProperty(id,false); }

    /** Disables property. */
    inline bool DisableProperty( wxPGPropNameStr name ) { return EnableProperty(name,false); }

    /** Enables or disables (shows/hides) categories according to parameter enable. */
    bool EnableCategories( bool enable );

    /** Enables or disables property, depending on whether enable is true or false. */
    bool EnableProperty( wxPGId id, bool enable = true );

    /** Enables or disables property, depending on whether enable is true or false. */
    inline bool EnableProperty( wxPGPropNameStr name, bool enable = true )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return EnableProperty( wxPGIdGen(p), enable );
    }

    /** Scrolls and/or expands items to ensure that the given item is visible.
        Returns true if something was actually done.
    */
    bool EnsureVisible( wxPGId id );

    /** Scrolls and/or expands items to ensure that the given item is visible.
        Returns true if something was actually done.
    */
    inline bool EnsureVisible( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return EnsureVisible( wxPGIdGen(p) );
    }

    /** Expands given category or property with children.
        Returns true if actually expands.
    */
    inline bool Expand( wxPGId id )
    {
        return _Expand(wxPGIdToPtr(id));
    }

    /** Expands given category or property with children.
        Returns true if actually expands.
    */
    inline bool Expand( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return _Expand(p);
    }

    /** Expands all items that can be expanded.
    */
    inline void ExpandAll() { m_pState->ExpandAll(1); }

#ifndef SWIG
    /** Returns a wxVariant list containing wxVariant versions of all
        property values. Order is not guaranteed, but generally it should
        match the visible order in the grid.
        \param flags
        Use wxPG_KEEP_STRUCTURE to retain category structure; each sub
        category will be its own wxList of wxVariant.
        \remarks
    */
    wxVariant GetPropertyValues( const wxString& listname = wxEmptyString,
        wxPGId baseparent = wxPGIdGen((wxPGProperty*)NULL), long flags = 0 ) const
    {
        return m_pState->GetPropertyValues(listname,baseparent,flags);
    }
#endif

    inline wxFont& GetCaptionFont() { return m_captionFont; }

    /** Returns current category caption background colour. */
    inline wxColour GetCaptionBackgroundColour() const { return m_colCapBack; }

    /** Returns current category caption text colour. */
    inline wxColour GetCaptionForegroundColour() const { return m_colCapFore; }

    /** Returns current cell background colour. */
    inline wxColour GetCellBackgroundColour() const { return m_colPropBack; }

    /** Returns current cell text colour when disabled. */
    inline wxColour GetCellDisabledTextColour() const { return m_colDisPropFore; }

    /** Returns current cell text colour. */
    inline wxColour GetCellTextColour() const { return m_colPropFore; }

    /** Returns number of children of the root property.
    */
    inline size_t GetChildrenCount()
    {
        return GetChildrenCount( wxPGIdGen(m_pState->m_properties) );
    }

    /** Returns number of children for the property.

        NB: Cannot be in container methods class due to name hiding.
    */
    inline size_t GetChildrenCount( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(0)
        return p->GetChildCount();
    }

    /** Returns number of children for the property. */
    inline size_t GetChildrenCount( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(0)
        return p->GetChildCount();
    }

    /** Returns id of first item, whether it is a category or property. */
    inline wxPGId GetFirst() const
    {
        return m_pState->GetFirst();
    }

    /** Returns id of first visible item, whether it is a category or property.
        Note that visible item means category, property, or sub-property which
        user can see when control is scrolled properly. It does not only mean
        items that are actually painted on the screen.
    */
    inline wxPGId GetFirstVisible() const
    {
        wxPGProperty* p = NULL;
        if ( m_pState->m_properties->GetCount() )
        {
            p = m_pState->m_properties->Item(0);
            if ( (m_iFlags & wxPG_FL_HIDE_STATE) && p->m_flags & wxPG_PROP_HIDEABLE )
                p = GetNeighbourItem ( p, true, 1 );
        }
        return wxPGIdGen(p);
    }

    /** Returns height of highest characters of used font. */
    int GetFontHeight() const { return m_fontHeight; }

    /** Returns pointer to itself. Dummy function that enables same kind
        of code to use wxPropertyGrid and wxPropertyGridManager.
    */
    wxPropertyGrid* GetGrid() { return this; }

    /** Returns id of first category (from target page). */
    inline wxPGId GetFirstCategory() const
    {
        return m_pState->GetFirstCategory();
    }

    /** Returns id of first property that is not a category. */
    inline wxPGId GetFirstProperty()
    {
        return m_pState->GetFirstProperty();
    }

    /** Returns size of the custom paint image in front of property.
        If no argument is given, returns preferred size.
    */
    wxSize GetImageSize( wxPGId id = wxPGIdGen((wxPGProperty*)NULL) ) const;

    /** Returns property (or category) at given y coordinate (relative to control's
        top left).
    */
    wxPGId GetItemAtY( int y ) { return wxPGIdGen(DoGetItemAtY(y)); }

    /** Returns id of last item. Ignores categories and sub-properties.
    */
    inline wxPGId GetLastProperty()
    {
        if ( !m_pState->m_properties->GetCount() ) return wxPGIdGen((wxPGProperty*)NULL);
        wxPGProperty* p = GetLastItem(false, false);
        if ( p->GetParentingType() > 0 )
            return GetPrevProperty ( wxPGIdGen(p) );
        return wxPGIdGen(p);
    }

    /** Returns id of last child of given property.
        \remarks
        Returns even sub-properties.
    */
    inline wxPGId GetLastChild( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)

        wxPGPropertyWithChildren* pwc = (wxPGPropertyWithChildren*) p;
        if ( !pwc->GetParentingType() || !pwc->GetCount() ) return wxNullProperty;
        return wxPGIdGen(pwc->Last());
    }
    inline wxPGId GetLastChild( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return GetLastChild( wxPGIdGen(p) );
    }

    /** Returns id of last visible item. Does <b>not</b> ignore categories sub-properties.
    */
    inline wxPGId GetLastVisible()
    {
        return wxPGIdGen( GetLastItem (true, true) );
    }

    /** Returns colour of lines between cells. */
    inline wxColour GetLineColour() const { return m_colLine; }

    /** Returns background colour of margin. */
    inline wxColour GetMarginColour() const { return m_colMargin; }

    /** Returns id of next property. This does <b>not</b> iterate to sub-properties
        or categories, unlike GetNextVisible.
    */
    inline wxPGId GetNextProperty( wxPGId id )
    {
        return m_pState->GetNextProperty(id);
    }

    /** Returns id of next category after a given property (which does not have to be category). */
    inline wxPGId GetNextCategory( wxPGId id ) const
    {
        return m_pState->GetNextCategory(id);
    }

    /** Returns id of next visible item.
        Note that visible item means category, property, or sub-property which
        user can see when control is scrolled properly. It does not only mean
        items that are actually painted on the screen.
    */
    inline wxPGId GetNextVisible( wxPGId property ) const
    {
        return wxPGIdGen ( GetNeighbourItem( wxPGIdToPtr(property),
            true, 1 ) );
    }

    /** Returns id of previous property. Unlike GetPrevVisible, this skips categories
        and sub-properties.
    */
    inline wxPGId GetPrevProperty( wxPGId id )
    {
        return m_pState->GetPrevProperty(id);
    }

    /** Returns id of previous item under the same parent. */
    inline wxPGId GetPrevSibling( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)
        return wxPropertyGridState::GetPrevSibling(id);
    }
    inline wxPGId GetPrevSibling( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return wxPropertyGridState::GetPrevSibling(wxPGIdGen(p));
    }

    /** Returns id of previous visible property.
    */
    inline wxPGId GetPrevVisible( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(wxNullProperty)
        return wxPGIdGen( GetNeighbourItem( wxPGIdToPtr(id), true, -1 ) );
    }

    /** Returns id of property's nearest parent category. If no category
        found, returns invalid wxPGId.
    */
    inline wxPGId GetPropertyCategory( wxPGId id ) const
    {
        return wxPGIdGen( _GetPropertyCategory ( wxPGIdToPtr(id) ) );
    }
    inline wxPGId GetPropertyCategory( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return _GetPropertyCategory(p);
    }

    /** Returns cell background colour of a property. */
    wxColour GetPropertyBackgroundColour( wxPGId id ) const;
    inline wxColour GetPropertyBackgroundColour( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxColour())
        return GetPropertyBackgroundColour(wxPGIdGen(p));
    }

    /** Returns cell background colour of a property. */
    inline wxColour GetPropertyColour( wxPGId id ) const
    {
        return GetPropertyBackgroundColour( id );
    }
    inline wxColour GetPropertyColour( wxPGPropNameStr name ) const
    {
        return GetPropertyBackgroundColour( name );
    }

    /** Returns cell background colour of a property. */
    wxColour GetPropertyTextColour( wxPGId id ) const;
    inline wxColour GetPropertyTextColour( wxPGPropNameStr name ) const
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxColour())
        return GetPropertyTextColour(wxPGIdGen(p));
    }

    /** Returns id of property with given label (case-sensitive). If there is no
        property with such label, returned property id is invalid ( i.e. it will return
        false with IsOk method). If there are multiple properties with identical name,
        most recent added is returned.
    */
    inline wxPGId GetPropertyByLabel( const wxString& name ) const
    {
        return m_pState->GetPropertyByLabel(name);
    }

    /** Returns "root property". It does not have name, etc. and it is not
        visible. It is only useful for accessing its children.
    */
    wxPGId GetRoot() const { return wxPGIdGen(m_pState->m_properties); }

    /** Returns height of a single grid row (in pixels). */
    int GetRowHeight() const { return m_lineHeight; }

    inline wxPGId GetSelectedProperty () const { return GetSelection(); }

    /** Returns currently selected property. */
    inline wxPGId GetSelection() const
    {
        return wxPGIdGen(m_selected);
    }

    /** Returns current selection background colour. */
    inline wxColour GetSelectionBackgroundColour() const { return m_colSelBack; }

    /** Returns current selection text colour. */
    inline wxColour GetSelectionForegroundColour() const { return m_colSelFore; }

    /** Returns current splitter x position. */
    inline int GetSplitterPosition() const { return m_splitterx; }

    /** Returns a binary copy of the current property state.
        NOTE: Too much work to implement, and uses would be few indeed.
    */
    //wxPropertyGridState* GetCopyOfState() const;

    /** Returns current vertical spacing. */
    inline int GetVerticalSpacing() const { return (int)m_vspacing; }

    /** Returns true if a property is selected. */
    inline bool HasSelection() const { return ((m_selected!=(wxPGProperty*)NULL)?true:false); }

    /** Hides all low priority properties. */
    inline void HideLowPriority() { Compact ( true ); }

    /** Inserts property to the list.

        \param priorthis
        New property is inserted just prior to this. Available only
        in the first variant. There are two versions of this function
        to allow this parameter to be either an id or name to
        a property.

        \param parent
        New property is inserted under this category. Available only
        in the second variant. There are two versions of this function
        to allow this parameter to be either an id or name to
        a property.

        \param index
        Index under category. Available only in the second variant.
        If index is < 0, property is appended in category.

        \param newproperty
        Pointer to the inserted property. wxPropertyGrid will take
        ownership of this object.

        \return
        Returns id for the property,

        \remarks

        - wxPropertyGrid takes the ownership of the property pointer.

        While Append may be faster way to add items, make note that when
        both data storages (categoric and
        non-categoric) are active, Insert becomes even more slow. This is
        especially true if current mode is non-categoric.

        Example of use:

        \code

            // append category
            wxPGId my_cat_id = propertygrid->Append( new wxPropertyCategoryClass (wxT("My Category")) );

            ...

            // insert into category - using second variant
            wxPGId my_item_id_1 = propertygrid->Insert( my_cat_id, 0, new wxStringProperty(wxT("My String 1")) );

            // insert before to first item - using first variant
            wxPGId my_item_id_2 = propertygrid->Insert ( my_item_id, new wxStringProperty(wxT("My String 2")) );

        \endcode

      */
    inline wxPGId Insert( wxPGId priorthis, wxPGProperty* newproperty )
    {
        wxPGId res = _Insert( wxPGIdToPtr( priorthis ), newproperty );
        DrawItems( newproperty, (wxPGProperty*) NULL );
        return res;
    }
    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGPropNameStr name, wxPGProperty* newproperty )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        wxPGId res = _Insert( (wxPGPropertyWithChildren*)p, newproperty );
        DrawItems( newproperty, (wxPGProperty*) NULL );
        return res;
    }

    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGId id, int index, wxPGProperty* newproperty )
    {
        wxPGId res = _Insert( (wxPGPropertyWithChildren*)wxPGIdToPtr (id), index, newproperty );
        DrawItems( newproperty, (wxPGProperty*) NULL );
        return res;
    }

    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGPropNameStr name, int index, wxPGProperty* newproperty )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        wxPGId res = _Insert( (wxPGPropertyWithChildren*)p, index, newproperty );
        DrawItems( newproperty, (wxPGProperty*) NULL );
        return res;
    }

    inline wxPGId InsertCategory( wxPGId id, int index, const wxString& label, const wxString& name = wxPG_LABEL )
    {
        return Insert( id, index, new wxPropertyCategoryClass(label,name) );
    }

#if wxPG_INCLUDE_BASICPROPS
    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGId id, int index, const wxString& label, const wxString& name, const wxString& value = wxEmptyString )
    {
        return Insert( id, index, wxStringProperty(label,name,value) );
    }

    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGId id, int index, const wxString& label, const wxString& name, int value )
    {
        return Insert( id, index, wxIntProperty(label,name,value) );
    }

    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGId id, int index, const wxString& label, const wxString& name, double value )
    {
        return Insert( id, index, wxFloatProperty(label,name,value) );
    }

    /** @link wxPropertyGrid::Insert Insert @endlink */
    inline wxPGId Insert( wxPGId id, int index, const wxString& label, const wxString& name, bool value )
    {
        return Insert( id, index, wxBoolProperty(label,name,value) );
    }
#endif

    /** Returns true if any property has been modified by the user. */
    inline bool IsAnyModified() const { return (m_pState->m_anyModified>0); }

    /** Returns true if updating is frozen (ie. Freeze() called but not yet Thaw() ). */
    inline bool IsFrozen() const { return (m_frozen>0)?true:false; }

    /** Returns true if given property is selected. */
    inline bool IsPropertySelected( wxPGId id ) const
    {
        wxPG_PROP_ID_CALL_PROLOG_RETVAL(false)
        return ( m_selected == p ) ? true : false;
    }

    /** Returns true if given property is selected. */
    inline bool IsPropertySelected( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return ( m_selected == p ) ? true : false;
    }

    /** Disables (limit = true) or enables (limit = false) wxTextCtrl editor of a property,
        if it is not the sole mean to edit the value.
    */
    void LimitPropertyEditing( wxPGId id, bool limit = true );

    /** Disables (limit = true) or enables (limit = false) wxTextCtrl editor of a property,
        if it is not the sole mean to edit the value.
    */
    inline void LimitPropertyEditing( wxPGPropNameStr name, bool limit = true )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        LimitPropertyEditing(wxPGIdGen(p),limit);
    }

    /** Moves splitter as left as possible, while still allowing all
        labels to be shown in full.
        \param subProps
        If false, will still allow sub-properties (ie. properties which
        parent is not root or category) to be cropped.
    */
    void SetSplitterLeft( bool subProps = false );

    /** Registers a new value type. Takes ownership of the object.
        \retval
        Pointer to the value type that should be used. If on with
        the same name already existed, then the first one will be used,
        and its pointer is returned instead.
    */
    static wxPGValueType* RegisterValueType( wxPGValueType* valueclass, bool noDefCheck = false,
                                             const wxString& className = wxEmptyString );

#ifndef SWIG
    /** Registers a new editor class.
        \retval
        Pointer to the editor class instance that should be used.
    */
    static wxPGEditor* RegisterEditorClass( wxPGEditor* editor, const wxString& name,
                                            bool noDefCheck = false );
#endif

    /** Resets all colours to the original system values.
    */
    void ResetColours();

    /** Changes keyboard shortcut to push the editor button.
        \remarks
        You can set default with keycode 0. Good value for the platform is guessed,
        but don't expect it to be very accurate.
    */
    void SetButtonShortcut( int keycode, bool ctrlDown = false, bool altDown = false );

    /** Sets text colour of a category caption (but not it's children).
    */
    void SetCaptionTextColour( wxPGId id, const wxColour& col );
    inline void SetCaptionTextColour( wxPGPropNameStr name, const wxColour& col )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetCaptionTextColour( wxPGIdGen(p), col );
    }

    /** Sets the current category - Append will add non-categories under this one.
    */
    inline void SetCurrentCategory( wxPGId id )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        wxPropertyCategoryClass* pc = (wxPropertyCategoryClass*)p;
#ifdef __WXDEBUG__
        if ( pc ) wxASSERT( pc->GetParentingType() > 0 );
#endif
        m_pState->m_currentCategory = pc;
    }

    /** Sets the current category - Append will add non-categories under this one.
    */
    inline void SetCurrentCategory( wxPGPropNameStr name = wxEmptyString )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetCurrentCategory(wxPGIdGen(p));
    }

    /** Sets property attribute for all applicapple properties.
        Be sure to use this method after all properties have been
        added to the grid.
    */
    void SetPropertyAttributeAll( int attrid, wxVariant value );

    /** Sets background colour of property and all its children, recursively. Colours of
        captions are not affected. Background brush cache is optimized for often
        set colours to be set last.
        \remarks
        * Children which already have custom background colour are not affected.
    */
    void SetPropertyBackgroundColour( wxPGId id, const wxColour& col );
    inline void SetPropertyBackgroundColour( wxPGPropNameStr name, const wxColour& col )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyBackgroundColour( wxPGIdGen(p), col );
    }

    /** Sets background colour of property and all its children. Colours of
        captions are not affected. Background brush cache is optimized for often
        set colours to be set last.

        NOTE: This function is deprecated. Use SetPropertyBackgroundColour.
    */
    inline void SetPropertyColour( wxPGId id, const wxColour& col )
    {
        SetPropertyBackgroundColour( id, col );
    }
    inline void SetPropertyColour( wxPGPropNameStr name, const wxColour& col )
    {
        SetPropertyBackgroundColour( name, col );
    }

    /** Sets text colour of property and all its children.
        \remarks
        * Children which already have custom text colour are not affected.
    */
    void SetPropertyTextColour( wxPGId id, const wxColour& col );
    inline void SetPropertyTextColour( wxPGPropNameStr name, const wxColour& col )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyTextColour( wxPGIdGen(p), col );
    }

    /** Sets background and text colour of property and all its children to the default. */
    void SetPropertyColourToDefault( wxPGId id );
    inline void SetPropertyColourToDefault( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyColourToDefault( wxPGIdGen(p) );
    }

    /** Sets category caption background colour. */
    void SetCaptionBackgroundColour(const wxColour& col);

    /** Sets category caption text colour. */
    void SetCaptionForegroundColour(const wxColour& col);

    /** Sets default cell background colour - applies to property cells.
        Note that appearance of editor widgets may not be affected.
    */
    void SetCellBackgroundColour(const wxColour& col);

    /** Sets cell text colour for disabled properties.
    */
    void SetCellDisabledTextColour(const wxColour& col);

    /** Sets default cell text colour - applies to property name and value text.
        Note that appearance of editor widgets may not be affected.
    */
    void SetCellTextColour(const wxColour& col);

    /** Sets colour of lines between cells. */
    void SetLineColour(const wxColour& col);

    /** Sets background colour of margin. */
    void SetMarginColour(const wxColour& col);

    /** Sets selection background colour - applies to selected property name background. */
    void SetSelectionBackground(const wxColour& col);

    /** Sets selection foreground colour - applies to selected property name text. */
    void SetSelectionForeground(const wxColour& col);

    /** Sets x coordinate of the splitter. */
    inline void SetSplitterPosition( int newxpos, bool refresh = true )
    {
        DoSetSplitterPosition(newxpos,refresh);
        m_iFlags |= wxPG_FL_SPLITTER_PRE_SET;
    }

    /** Selects a property. Editor widget is automatically created, but
        not focused unless focus is true. This will generate wxEVT_PG_SELECT event.
        \param id
        Id to property to select.
        \retval
        True if selection finished succesfully. Usually only fails if current
        value in editor is not valid.
        \sa wxPropertyGrid::Unselect
    */
    inline bool SelectProperty( wxPGId id, bool focus = false )
    {
        return DoSelectProperty(wxPGIdToPtr(id),focus?wxPG_SEL_FOCUS:0);
    }
    inline bool SelectProperty( wxPGPropNameStr name, bool focus = false )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return DoSelectProperty(p,focus?wxPG_SEL_FOCUS:0);
    }

    /** Mostly useful for page switching.
    */
#ifndef SWIG
    void SwitchState( wxPropertyGridState* pNewState );
#endif

    /** Sets label of a property.
        \remarks
        This is the only way to set property's name. There is not
        wxPGProperty::SetLabel() method.
    */
    inline void SetPropertyLabel( wxPGId id, const wxString& newproplabel )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        _SetPropertyLabel( p, newproplabel );
    }
    /** Sets label of a property.
        \remarks
        This is the only way to set property's label. There is no
        wxPGProperty::SetLabel() method.
    */
    inline void SetPropertyLabel( wxPGPropNameStr name, const wxString& newproplabel )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        _SetPropertyLabel( p, newproplabel );
    }

    /** Sets name of a property.
        \param id
        Id of a property.
        \param newname
        New name.
        \remarks
        This is the only way to set property's name. There is not
        wxPGProperty::SetName() method.
    */
    inline void SetPropertyName( wxPGId id, const wxString& newname )
    {
        DoSetPropertyName( wxPGIdToPtr(id), newname );
    }
    /** Sets name of a property.
        \param name
        Label of a property.
        \param newname
        New name.
        \remarks
        This is the only way to set property's name. There is not
        wxPGProperty::SetName() method.
    */
    inline void SetPropertyName( wxPGPropNameStr name, const wxString& newname )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        DoSetPropertyName( p, newname );
    }

    /** Sets value (long integer) of a property.
    */
    inline void SetPropertyValueLong( wxPGId id, long value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(long), wxPGVariantFromLong(value) );
    }

#ifndef __WXPYTHON__
    /** Sets value (integer) of a property.
    */
    inline void SetPropertyValue( wxPGId id, int value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(long), wxPGVariantFromLong((long)value) );
    }
#endif
    /** Sets value (floating point) of a property.
    */
    inline void SetPropertyValueDouble( wxPGId id, double value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(double), wxPGVariantFromDouble(value) );
    }
    /** Sets value (bool) of a property.
    */
    inline void SetPropertyValueBool( wxPGId id, bool value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(bool), wxPGVariantFromLong(value?(long)1:(long)0) );
    }

    /** Sets value (wxString) of a property.
        \remarks
        This method uses wxPGProperty::SetValueFromString, which all properties
        should implement. This means that there should not be a type error,
        and instead the string is converted to property's actual value type.
    */
    void SetPropertyValueString( wxPGId id, const wxString& value );

#ifndef __WXPYTHON__
    inline void SetPropertyValue( wxPGId id, const wxChar* value )
    {
        SetPropertyValue(id,wxString(value));
    }
#endif

    /** Sets value (wxArrayString) of a property.
    */
    inline void SetPropertyValueArrstr2( wxPGId id, const wxArrayString& value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(wxArrayString), wxPGVariantFromArrayString(value) );
    }
    /** Sets value (wxObject*) of a property.
    */
    void SetPropertyValueWxObjectPtr( wxPGId id, wxObject* value );
#ifndef __WXPYTHON__
    /** Sets value (void*) of a property. */
    inline void SetPropertyValue( wxPGId id, void* value )
    {
        SetPropertyValue( id, wxPG_VALUETYPE(void), value );
    }
    inline void SetPropertyValue ( wxPGId id, wxObject& value )
    {
        SetPropertyValue(id,&value);
    }

    /** Sets value (wxVariant&) of a property. */
    void SetPropertyValue( wxPGId id, wxVariant& value );
#endif

    /** Sets value (wxPoint&) of a property.
    */
    inline void SetPropertyValuePoint( wxPGId id, const wxPoint& value )
    {
        SetPropertyValue( id, wxT("wxPoint"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxSize&) of a property.
    */
    inline void SetPropertyValueSize( wxPGId id, const wxSize& value )
    {
        SetPropertyValue( id, wxT("wxSize"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxLongLong&) of a property.
    */
    inline void SetPropertyValueLongLong( wxPGId id, const wxLongLong& value )
    {
        SetPropertyValue( id, wxT("wxLongLong"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxULongLong&) of a property.
    */
    inline void SetPropertyValueULongLong( wxPGId id, const wxULongLong& value )
    {
        SetPropertyValue( id, wxT("wxULongLong"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxArrayInt&) of a property.
    */
    inline void SetPropertyValueArrint2( wxPGId id, const wxArrayInt& value )
    {
        SetPropertyValue( id, wxT("wxArrayInt"), wxPGVariantCreator(value) );
    }
    /** Sets value (wxDateTime&) of a property.
    */
#if wxUSE_DATETIME
    inline void SetPropertyValueDatetime( wxPGId id, const wxDateTime& value )
    {
        SetPropertyValue( id, wxT("datetime"), value );
    }
#endif
#ifdef __WXPYTHON__
    inline void SetPropertyValuePyObject( wxPGId id, PyObject* value )
    {
        SetPropertyValue( id, wxT("PyObject"), wxPGVariantCreator(value) );
    }
#endif

    /** Sets value (long integer) of a property.
    */
    inline void SetPropertyValueLong( wxPGPropNameStr name, long value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(long), wxPGVariantFromLong(value) );
    }
#ifndef __WXPYTHON__
    /** Sets value (integer) of a property. */
    inline void SetPropertyValue( wxPGPropNameStr name, int value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(long), wxPGVariantFromLong(value) );
    }
#endif
    /** Sets value (floating point) of a property.
    */
    inline void SetPropertyValueDouble( wxPGPropNameStr name, double value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(double), wxPGVariantFromDouble(value) );
    }
    /** Sets value (bool) of a property.
    */
    inline void SetPropertyValueBool( wxPGPropNameStr name, bool value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(bool), wxPGVariantFromLong(value?(long)1:(long)0) );
    }
    /** Sets value (wxString) of a property. For properties which value type is
        not string, calls wxPGProperty::SetValueFromString to translate the value.
    */
    inline void SetPropertyValueString( wxPGPropNameStr name, const wxString& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueString( wxPGIdGen(p), value );
    }

#ifndef __WXPYTHON__
    /** Sets value (wxString) of a property. For properties which value type is
        not string, calls wxPGProperty::SetValueFromString to translate the value.
    */
    inline void SetPropertyValue( wxPGPropNameStr name, const wxChar* value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxString(value) );
    }

    /** Sets value (void*) of a property. */
    inline void SetPropertyValue( wxPGPropNameStr name, void* value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(void), value );
    }
#endif

    /** Sets value (wxObject*) of a property.
    */
    inline void SetPropertyValueWxObjectPtr( wxPGPropNameStr name, wxObject* value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueWxObjectPtr( wxPGIdGen(p), value );
    }

#ifndef __WXPYTHON__
    inline void SetPropertyValue( wxPGPropNameStr name, wxObject& value )
    {
        SetPropertyValue(name,&value);
    }

    /** Sets value (wxVariant&) of a property. */
    void SetPropertyValue( wxPGPropNameStr name, wxVariant& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), value );
    }

    /** Sets value (wxArrayString) of a property. */
    inline void SetPropertyValueArrstr2( wxPGPropNameStr name, const wxArrayString& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValue( wxPGIdGen(p), wxPG_VALUETYPE(wxArrayString), wxPGVariantFromArrayString(value) );
    }
    /** Sets value (wxArrayInt&) of a property. */
    inline void SetPropertyValueArrint2( wxPGPropNameStr name, const wxArrayInt& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueArrint2( wxPGIdGen(p), value );
    }
#endif
    /** Sets value (wxDateTime&) of a property.
    */
#if wxUSE_DATETIME
    inline void SetPropertyValueDatetime( wxPGPropNameStr name, const wxDateTime& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueDatetime( wxPGIdGen(p), value );
    }
#endif
    /** Sets value (wxPoint&) of a property.
    */
    inline void SetPropertyValuePoint( wxPGPropNameStr name, const wxPoint& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValuePoint( wxPGIdGen(p), value );
    }
    /** Sets value (wxSize&) of a property.
    */
    inline void SetPropertyValueSize( wxPGPropNameStr name, const wxSize& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueSize( wxPGIdGen(p), value );
    }
    /** Sets value (wxLongLong&) of a property.
    */
    inline void SetPropertyValueLongLong( wxPGPropNameStr name, const wxLongLong& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueLongLong( wxPGIdGen(p), value );
    }
    /** Sets value (wxULongLong&) of a property.
    */
    inline void SetPropertyValueULongLong( wxPGPropNameStr name, const wxULongLong& value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValueULongLong( wxPGIdGen(p), value );
    }
#ifdef __WXPYTHON__
    inline void SetPropertyValuePyObject( wxPGPropNameStr name, PyObject* value )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyValuePyObject( wxPGIdGen(p), value );
    }
#endif

    /** Sets property's value to unspecified. If it has children (it may be category),
        then the same thing is done to them.
    */
    void SetPropertyUnspecified( wxPGId id );
    inline void SetPropertyUnspecified ( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetPropertyUnspecified( wxPGIdGen(p) );
    }

#ifndef SWIG
    /** Sets various property values from a list of wxVariants. If property with
        name is missing from the grid, new property is created under given default
        category (or root if omitted).
    */
    void SetPropertyValues( const wxVariantList& list, wxPGId default_category )
    {
        m_pState->SetPropertyValues(list,default_category);
    }

    inline void SetPropertyValues( const wxVariant& list, wxPGId default_category )
    {
        SetPropertyValues(list.GetList(),default_category);
    }
    inline void SetPropertyValues( const wxVariantList& list, const wxString& default_category = wxEmptyString )
    {
        SetPropertyValues(list,GetPropertyByNameI(default_category));
    }
    inline void SetPropertyValues( const wxVariant& list, const wxString& default_category = wxEmptyString )
    {
        SetPropertyValues(list.GetList(),GetPropertyByNameI(default_category));
    }
#endif

    /** Sets vertical spacing. Can be 1, 2, or 3 - a value relative to font
        height. Value of 2 should be default on most platforms.
        \remarks
        On wxMSW, wxComboBox, when used as property editor widget, will spill
        out with anything less than 3.
    */
    inline void SetVerticalSpacing( int vspacing )
    {
        m_vspacing = (unsigned char)vspacing;
        CalculateFontAndBitmapStuff( vspacing );
        if ( !m_pState->m_itemsAdded ) Refresh();
    }

    /** Shows all low priority properties. */
    inline void ShowLowPriority() { Compact ( false ); }

    /** Shows an brief error message that is related to a property. */
    inline void ShowPropertyError( wxPGId id, const wxString& msg )
    {
        wxPG_PROP_ID_CALL_PROLOG()
        p->ShowError(msg);
    }
    inline void ShowPropertyError( wxPGPropNameStr name, const wxString& msg )
    {
        ShowPropertyError (GetPropertyByNameI(name), msg);
    }

    /** Sorts all items at all levels (except sub-properties). */
    void Sort();

    /** Sorts children of a category.
    */
    void Sort( wxPGId id );

    /** Sorts children of a category.
    */
    inline void Sort( wxPGPropNameStr name )
    {
        Sort( GetPropertyByNameI(name) );
    }

    /** Overridden function.
        \sa @link wndflags Additional Window Styles@endlink
    */
    virtual void SetWindowStyleFlag( long style );

    /** All properties added/inserted will have given priority by default.
        \param
        priority can be wxPG_HIGH (default) or wxPG_LOW.
    */
    inline void SetDefaultPriority( int priority )
    {
        if ( priority == wxPG_LOW )
            m_iFlags |= wxPG_FL_ADDING_HIDEABLES;
        else
            m_iFlags &= ~(wxPG_FL_ADDING_HIDEABLES);
    }

    /** Same as SetDefaultPriority(wxPG_HIGH). */
    inline void ResetDefaultPriority()
    {
        SetDefaultPriority(wxPG_HIGH);
    }

    /** Property editor widget helper methods. */
    //@{
    /** Call when editor widget's contents is modified. For example, this is called
        when changes text in wxTextCtrl (used in wxStringProperty and wxIntProperty).
        \remarks
        This should only be called by properties.
        \sa @link wxPGProperty::OnEvent @endlink
    */
    inline void EditorsValueWasModified() { m_iFlags |= wxPG_FL_VALUE_MODIFIED; }
    /** Reverse of EditorsValueWasModified(). */
    inline void EditorsValueWasNotModified()
    {
        m_iFlags &= ~(wxPG_FL_VALUE_MODIFIED);
    }

    /** Returns true if editor's value was marked modified. */
    inline bool IsEditorsValueModified() const { return  ( m_iFlags & wxPG_FL_VALUE_MODIFIED ) ? true : false; }

    /** Shortcut for creating dialog-caller button. Used, for example, by wxFontProperty.
        \remarks
        This should only be called by properties.
    */
    wxWindow* GenerateEditorButton( const wxPoint& pos, const wxSize& sz );

    /** Fixes position of wxTextCtrl-like control (wxSpinCtrl usually
        fits as one). Call after control has been created (but before
        shown).
    */
    void FixPosForTextCtrl( wxWindow* ctrl );

    /** Shortcut for creating text editor widget.
        \param pos
        Same as pos given for CreateEditor.
        \param sz
        Same as sz given for CreateEditor.
        \param value
        Initial text for wxTextCtrl.
        \param secondary
        If right-side control, such as button, also created, then create it first
        and pass it as this parameter.
        \param extraStyle
        Extra style flags to pass for wxTextCtrl.
        \remarks
        Note that this should generally be called only by new classes derived
        from wxPGProperty.
    */
    wxWindow* GenerateEditorTextCtrl( const wxPoint& pos,
                                      const wxSize& sz,
                                      const wxString& value,
                                      wxWindow* secondary,
                                      int extraStyle = 0,
                                      int maxLen = 0 );

    /* Generates both textctrl and button.
    */
    wxWindow* GenerateEditorTextCtrlAndButton( const wxPoint& pos,
        const wxSize& sz, wxWindow** psecondary, int limited_editing,
        wxPGProperty* property );

    /** Generates position for a widget editor dialog box.
        \param p
        Property for which dialog is positioned.
        \param sz
        Known or over-approximated size of the dialog.
        \retval
        Position for dialog.
    */
    wxPoint GetGoodEditorDialogPosition( wxPGProperty* p,
                                         const wxSize& sz );

    // Converts escape sequences in src_str to newlines,
    // tabs, etc. and copies result to dst_str.
    static wxString& ExpandEscapeSequences( wxString& dst_str, wxString& src_str );

    // Converts newlines, tabs, etc. in src_str to escape
    // sequences, and copies result to dst_str.
    static wxString& CreateEscapeSequences( wxString& dst_str, wxString& src_str );

    /** Returns rectangle that fully contains properties between and including p1 and p2.
    */
    wxRect GetPropertyRect( const wxPGProperty* p1, const wxPGProperty* p2 ) const;

    /** Returns pointer to current active primary editor control (NULL if none).

        If editor uses clipper window, pointer is returned to the actual editor, not the clipper.
    */
    wxWindow* GetEditorControl() const;

    inline wxWindow* GetPrimaryEditor() const
    {
        return m_wndPrimary;
    }

    /** Returns pointer to current active secondary editor control (NULL if none).
    */
    inline wxWindow* GetEditorControlSecondary() const
    {
        return m_wndSecondary;
    }

    inline int IsNextEventIgnored() const
    {
        return m_ignoredEvents;
    }

    inline void IgnoreNextEvent()
    {
        m_ignoredEvents++;
    }

    inline void IgnoredEventPasses()
    {
        wxASSERT( m_ignoredEvents > 0 );
        m_ignoredEvents--;
    }

#ifdef __WXPYTHON__
    // Dummy method to put wxRect type info into the wrapper
    wxRect DummywxRectTypeInit() const { return wxRect(1,2,3,4); }
#endif

#ifndef SWIG

    /** Generates contents for string dst based on the convetents of wxArrayString
        src. Format will be <preDelim>str1<postDelim> <preDelim>str2<postDelim>
        and so on. Set flags to 1 inorder to convert backslashes to double-back-
        slashes and "<preDelims>"'s to "\<preDelims>".
    */
    static void ArrayStringToString( wxString& dst, const wxArrayString& src,
                                     wxChar preDelim, wxChar postDelim,
                                     int flags );

    /** Pass this function to Connect calls in propertyclass::CreateEditor.
    */
    void OnCustomEditorEvent( wxCommandEvent &event );
    /** Puts items into sl. Automatic wxGetTranslation is used if enabled. */
    void SLAlloc ( unsigned int itemcount, const wxChar** items );
    /** Returns sl. */
    inline wxArrayString& SLGet() { return m_sl; }
    //@}

    inline long GetInternalFlags() const { return m_iFlags; }
    inline void ClearInternalFlag( long flag ) { m_iFlags &= ~(flag); }
    inline unsigned int GetBottomY() const { return m_bottomy; }
    inline void SetBottomY( unsigned int y ) { m_bottomy = y; }
    inline void IncFrozen() { m_frozen++; }
    inline void DecFrozen() { m_frozen--; }

    /** Call after a property modified internally.
        selFlags are the same as with DoSelectProperty.
        NB: Avoid using this method, if possible.
    */
    void PropertyWasModified( wxPGProperty* p, int selFlags = 0 );

    void OnComboItemPaint( wxPGCustomComboControl* pCb,int item,wxDC& dc,
                           wxRect& rect,int flags );

    // Used by simple check box for keyboard navigation
    void SendNavigationKeyEvent( int dir );

#if 0
    /* Creates choices for labels and values, or returns existing choices which
       point to the same memory.
    */
    static wxPGChoices* CreateChoicesArray(const wxChar** labels,
                                           const long* values,
                                           int itemcount);

    /* Creates choices for labels and values, or returns existing choices which
       point to the same memory (*only* if acceptLabelsAsId=true).
    */
    static wxPGChoices* CreateChoicesArray(const wxArrayString& labels,
                                           const wxArrayInt& values = wxPG_EMPTY_ARRAYINT,
                                           bool acceptLabelsAsId = false);

    /* Creates choices for labels and values in wxPGChoices, or returns existing
       choices which is identical.
    */
    static wxPGChoices* CreateChoicesArray(wxPGChoices& choices);

#ifdef __WXDEBUG__
    // Displays what dynamic arrays are allocated
    static void DumpAllocatedChoiceSets();
#endif

#endif // #if 0

    /** Standardized double-to-string conversion.
    */
    static void DoubleToString( wxString& target,
                                double value,
                                int precision,
                                bool removeZeroes,
                                wxString* precTemplate );


protected:

    /** wxPropertyGridState used by the grid is created here. If grid is used
        in wxPropertyGridManager, there is no point overriding this - instead,
        set custom wxPropertyGridPage classes.
    */
    virtual wxPropertyGridState* CreateState() const;

#ifndef DOXYGEN
public:

    // Control font changer helper.
    void SetCurControlBoldFont();

    //
    // Public methods for semi-public use
    // (not protected for optimization)
    //
    bool DoSelectProperty( wxPGProperty* p, unsigned int flags = 0 );

    // Usually called internally after items added/deleted.
    void CalculateYs( wxPGPropertyWithChildren* startparent, int startindex );

    // Overridden functions.
    virtual bool Destroy();
    virtual wxSize DoGetBestSize() const;
    virtual void Refresh( bool eraseBackground = true,
                          const wxRect *rect = (const wxRect *) NULL );
    virtual bool SetFont( const wxFont& font );
#if wxPG_SUPPORT_TOOLTIPS
    void SetToolTip( const wxString& tipString );
#endif
    virtual void Freeze();
    virtual void SetExtraStyle( long exStyle );
    virtual void Thaw();


protected:

    /** 1 if calling property event handler. */
    unsigned char       m_processingEvent;

#ifndef wxPG_ICON_WIDTH
	wxBitmap            *m_expandbmp, *m_collbmp;
#endif

    wxCursor            *m_cursorSizeWE;

    /** wxWindow pointers to editor control(s). */
    wxWindow            *m_wndPrimary;
    wxWindow            *m_wndSecondary;

#if wxPG_DOUBLE_BUFFER
    wxBitmap            *m_doubleBuffer;
#endif

    wxArrayPtrVoid      *m_windowsToDelete;

    /** Local time ms when control was created. */
    wxLongLong          m_timeCreated;

    /** Indicates bottom of drawn and clickable area on the control. Updated
        by CalculateYs. */
    unsigned int        m_bottomy;

	/** Extra Y spacing between the items. */
	int                 m_spacingy;

    /** Control client area width; updated on resize. */
    int                 m_width;

    /** Control client area height; updated on resize. */
    int                 m_height;

    /** Non-client width (auto-centering helper). */
    int                 m_fWidth;

    /** List of currently visible properties. */
    wxPGArrayProperty   m_arrVisible;

    /** Previously recorded scroll start position. */
    int                 m_prevVY;

    /** Necessary so we know when to re-calculate visibles on resize. */
    int                 m_calcVisHeight;

	/** The gutter spacing in front and back of the image. This determines the amount of spacing in front
	    of each item */
	int                 m_gutterWidth;

    /** Includes separator line. */
    int                 m_lineHeight;

    /** Gutter*2 + image width. */
    int                 m_marginWidth;

    int                 m_buttonSpacingY; // y spacing for expand/collapse button.

    /** Extra margin for expanded sub-group items. */
    int                 m_subgroup_extramargin;

	/** The image width of the [+] icon. This is also calculated in the gutter */
	int                 m_iconWidth;

#ifndef wxPG_ICON_WIDTH

	/** The image height of the [+] icon. This is calculated as minimal size and to align */
	int                 m_iconHeight;
#endif

    /** Current cursor id. */
    int                 m_curcursor;

	/** This captionFont is made equal to the font of the wxScrolledWindow. As extra the bold face
	    is set on it when this is wanted by the user (see flags) */
	wxFont              m_captionFont;

#if !wxPG_HEAVY_GFX
    int                 m_splitterprevdrawnx;

    /** Pen used to draw splitter column when it is being dragged. */
    wxPen               m_splitterpen;

#endif

	int                 m_fontHeight;  // Height of the font.

    int                 m_pushButKeyCode;  // Base keycode for triggering push button.

    //
    // Temporary values
    //

    /** m_splitterx when drag began. */
    int                 m_startingSplitterX;

    /** Bits are used to indicate which colours are customized. */
    unsigned short      m_coloursCustomized;

    /** 0 = not dragging, 1 = drag just started, 2 = drag in progress */
    unsigned char       m_dragStatus;

    /** x - m_splitterx. */
    signed char         m_dragOffset;

    /** 0 = margin, 1 = label, 2 = value. */
    unsigned char       m_mouseSide;

    /** True when editor control is focused. */
    unsigned char       m_editorFocused;

    /** 1 if m_latsCaption is also the bottommost caption. */
    //unsigned char       m_lastCaptionBottomnest;

    /** Set to 1 when graphics frozen. */
    unsigned char       m_frozen;

    unsigned char       m_vspacing;

    unsigned char       m_pushButKeyCodeNeedsAlt;  // Does triggering push button need Alt down?

    unsigned char       m_pushButKeyCodeNeedsCtrl;  // Does triggering push button need Ctrl down?

    unsigned char       m_keyComboConsumed;  // Used to track when Alt/Ctrl+Key was consumed.

    unsigned char       m_ignoredEvents;  // Number of EVT_TEXT-style events to ignore.

    /** Internal flags - see wxPG_FL_XXX constants. */
    wxUint32            m_iFlags;

    /** When drawing next time, clear this many item slots at the end. */
    int                 m_clearThisMany;

    /** Pointer to selected property. Note that this is duplicated in
        m_state for better transiency between pages so that the selected
        item can be retained.
    */
    wxPGProperty*       m_selected;

    wxPGProperty*       m_propHover;  // pointer to property that has mouse on itself

    wxWindow*           m_eventObject;  // EventObject for wxPropertyGridEvents

    wxWindow*           m_curFocused;  // What (global) window is currently focused
                                       // (needed to resolve event handling mess).

    wxEvtHandler*       m_tlwHandler;  // wxPGTLWHandler

    wxWindow*           m_tlp;  // Top level parent

    int                 m_splitterx; // x position for the vertical line dividing name and value

    float               m_fSplitterX; // accurate splitter position

    int                 m_ctrlXAdjust; // x relative to splitter (needed for resize).

    wxColour            m_colLine;     // lines between cells
    wxColour            m_colPropFore; // property labels and values are written in this colour
    wxColour            m_colDisPropFore;  // or with this colour when disabled
    wxColour            m_colPropBack; // background for m_colPropFore
    wxColour            m_colCapFore;  // text color for captions
    wxColour            m_colCapBack;  // background color for captions
    wxColour            m_colSelFore;  // foreground for selected property
    wxColour            m_colSelBack;  // background for selected property (actually use background color when control out-of-focus)
    wxColour            m_colMargin;   // background colour for margin

    // NB: These *cannot* be moved to globals.
    wxArrayPtrVoid      m_arrBgBrushes; // Array of background colour brushes.
    wxArrayPtrVoid      m_arrFgCols; // Array of foreground colours.

    wxArrayString       m_sl;           // string control helper

protected:

    // Sets some members to defaults (called constructors).
	void Init1();

    // Initializes some members (called by Create and complex constructor).
	void Init2();

	void OnPaint(wxPaintEvent &event );

    // main event receivers
    void OnMouseMove( wxMouseEvent &event );
    void OnMouseClick( wxMouseEvent &event );
    void OnMouseRightClick( wxMouseEvent &event );
    void OnMouseDoubleClick( wxMouseEvent &event );
    void OnMouseUp( wxMouseEvent &event );
    void OnKey( wxKeyEvent &event );
    void OnKeyUp( wxKeyEvent &event );
    void OnNavigationKey( wxNavigationKeyEvent& event );
    void OnResize( wxSizeEvent &event );

    // event handlers
    bool HandleMouseMove( int x, unsigned int y, wxMouseEvent &event );
    bool HandleMouseClick( int x, unsigned int y, wxMouseEvent &event );
    bool HandleMouseRightClick( int x, unsigned int y, wxMouseEvent &event );
    bool HandleMouseDoubleClick( int x, unsigned int y, wxMouseEvent &event );
    bool HandleMouseUp( int x, unsigned int y, wxMouseEvent &event );
    void HandleKeyEvent( wxKeyEvent &event );
    bool HandleChildKey( wxKeyEvent& event, bool canDestroy ); // Handle TAB and ESCAPE in control

    void OnMouseEntry( wxMouseEvent &event );

    void OnIdle( wxIdleEvent &event );
    void OnFocusEvent( wxFocusEvent &event );
    void OnChildFocusEvent( wxChildFocusEvent& event );

    bool OnMouseCommon( wxMouseEvent &event, int* px, int *py );
    bool OnMouseChildCommon( wxMouseEvent &event, int* px, int *py );

    // sub-control event handlers
    void OnMouseClickChild( wxMouseEvent &event );
    void OnMouseRightClickChild( wxMouseEvent &event );
    void OnMouseMoveChild( wxMouseEvent &event );
    void OnMouseUpChild( wxMouseEvent &event );
    void OnChildKeyDown( wxKeyEvent &event );
    void OnChildKeyUp( wxKeyEvent &event );
    //void OnFocusChild( wxFocusEvent &event );

    void OnCaptureChange( wxMouseCaptureChangedEvent &event );

    void OnScrollEvent( wxScrollWinEvent &event );

    void OnSysColourChanged( wxSysColourChangedEvent &event );

protected:

	/** Adjust the centering of the bitmap icons (collapse / expand) when the caption font changes. They need to
	    be centered in the middle of the font, so a bit of deltaY adjustment is needed.
        On entry, m_captionFont must be set to window font. It will be modified properly.
    */
	void CalculateFontAndBitmapStuff( int vspacing );

    inline wxRect GetEditorWidgetRect( wxPGProperty* p );

    void CorrectEditorWidgetSizeX( int newSplitterx, int newWidth );

#ifdef __WXDEBUG__
    void _log_items();
    void OnScreenNote( const wxChar* format, ... );
#endif

    void DoDrawItems( wxDC& dc,
                      const wxPGProperty* first_item,
                      const wxPGProperty* last_item,
                      const wxRect* clip_rect );

    void DoDrawItems2( wxDC& dc,
                       const wxPGProperty* first_item,
                       const wxPGProperty* last_item,
                       const wxRect* clip_rect ) const;

    virtual void RefreshProperty( wxPGProperty* p );

    /** Draws items from topitemy to bottomitemy */
    void DrawItems( wxDC& dc, unsigned int topitemy, unsigned int bottomitemy,
                    const wxRect* clip_rect = (const wxRect*) NULL );

    void DrawItems( const wxPGProperty* p1, const wxPGProperty* p2 );

    // In addition to calling DoDrawItems directly, this is the
    // only alternative for using wxClientDC - others just call
    // RefreshRect.
    void DrawItem( wxDC& dc, wxPGProperty* p );

    inline void DrawItem( wxPGProperty* p )
    {
        DrawItems(p,p);
    }

    virtual void DrawItemAndChildren( wxPGProperty* p );

    /** Draws item, children, and consequtive parents as long as category is not met. */
    void DrawItemAndValueRelated( wxPGProperty* p );

    /** Returns property reference for given property id. */
    inline wxPGProperty& GetPropertyById( wxPGId id )
    {
        return *wxPGIdToPtr(id);
    }

    static wxPropertyCategoryClass* _GetPropertyCategory( wxPGProperty* p );

    void ImprovedClientToScreen( int* px, int* py );

    wxPGId _Insert( wxPGProperty* priorthis, wxPGProperty* newproperty );

    inline wxPGId _Insert( wxPGPropertyWithChildren* parent, int index, wxPGProperty* newproperty )
    {
        return m_pState->DoInsert(parent,index,newproperty);
    }

    // Called by focus event handlers. newFocused is the window that becomes focused.
    void HandleFocusChange( wxWindow* newFocused );

    /** Reloads all non-customized colours from system settings. */
    void RegainColours();

    bool DoEditorValidate();

    wxPGProperty* DoGetItemAtY( int y );

    inline wxPGProperty* DoGetItemAtY_Full( int y )
    {
        wxASSERT( y >= 0 );

        if ( (unsigned int)y >= m_bottomy )
            return NULL;

        return m_pState->m_properties->GetItemAtY ( y, m_lineHeight );
    }

    void DoPropertyChanged( wxPGProperty* p, unsigned int selFlags = 0 );

    void DoSetSplitterPosition( int newxpos, bool refresh = true );

    void FreeEditors();

    wxPGProperty* GetLastItem( bool need_visible, bool allow_subprops = true );

    void CalculateVisibles( int vy, bool full_recalc );

    bool _Expand( wxPGProperty* p, bool sendEvent = false );

    bool _Collapse( wxPGProperty* p, bool sendEvent = false );

    /** Forces updating the value of property from the editor control.
        Returns true if DoPropertyChanged was actually called.
    */
    bool CommitChangesFromEditor( wxUint32 flags = 0 );

    // Returns nearest paint visible property (such that will be painted unless
    // window is scrolled or resized). If given property is paint visible, then
    // it itself will be returned.
    wxPGProperty* GetNearestPaintVisible( wxPGProperty* p );

/*#ifdef __WXMSW__
    virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle) const;
#endif*/

    static void RegisterDefaultEditors();

    static void RegisterDefaultValues();

    // Sets m_bgColIndex to this property and all its children.
    void SetBackgroundColourIndex( wxPGProperty* p, int index, int flags );

    // Sets m_fgColIndex to this property and all its children.
    void SetTextColourIndex( wxPGProperty* p, int index, int flags );

    int CacheColour( const wxColour& colour );

    void _SetPropertyLabel( wxPGProperty* p, const wxString& newproplabel );

    void DoSetPropertyName( wxPGProperty* p, const wxString& newname );

    void SetPropertyValue( wxPGId id, const wxPGValueType* typeclass, const wxPGVariant& value );

    void SetPropertyValue( wxPGId id, const wxChar* typestring, const wxPGVariant& value );

    // Setups event handling for child control
    void SetupEventHandling( wxWindow* wnd, int id );

    void CustomSetCursor( int type, bool override = false );

    void RecalculateVirtualSize();

    void PGAdjustScrollbars( int y );

    inline bool UsesAutoUnspecified() const
    {
        return ( GetExtraStyle() & wxPG_EX_AUTO_UNSPECIFIED_VALUES ) ? true : false;
    }

    /** When splitter is dragged to a new position, this is drawn. */
    void DrawSplitterDragColumn( wxDC& dc, int x );

    /** If given index is -1, scans for item to either up (dir=0) or down (dir!=0) */
    //int GetNearestValidItem ( int index, int dir );
    wxPGProperty* GetNeighbourItem( wxPGProperty* item, bool need_visible,
                                    int dir ) const;

    void PrepareAfterItemsAdded();

    void SendEvent( int eventType, wxPGProperty* p, unsigned int selFlags = 0 );

    bool SetPropertyPriority( wxPGProperty* p, int priority );

private:

    bool ButtonTriggerKeyTest( wxKeyEvent &event );

#endif // DOXYGEN_SHOULD_SKIP_THIS

private:
    DECLARE_EVENT_TABLE()
#endif // #ifndef SWIG
};

#undef wxPG_USE_STATE

// -----------------------------------------------------------------------
//
// Bunch of inlines that need to resolved after all classes have been defined.
//

#ifndef SWIG
inline bool wxPropertyGridState::IsDisplayed() const
{
    return ( this == m_pPropGrid->GetState() );
}
#endif

inline void wxPGProperty::SetEditor( const wxString& editorName )
{
    EnsureDataExt();
    m_dataExt->m_customEditor = wxPropertyContainerMethods::GetEditorByName(editorName);
}

inline bool wxPGProperty::Hide( bool hide )
{
    return GetGrid()->HideProperty(wxPGIdGen(this),hide);
}

inline bool wxPGProperty::SetMaxLength( int maxLen )
{
    return GetGrid()->SetPropertyMaxLength(wxPGIdGen(this),maxLen);
}

#if !wxPG_ID_IS_PTR
inline const wxString& wxPGId::GetName() const
{
    return m_ptr->GetName();
}
#endif

// -----------------------------------------------------------------------

/** \class wxPropertyGridEvent
	\ingroup classes
    \brief A propertygrid event holds information about events associated with
    wxPropertyGrid objects.

    <h4>Derived from</h4>

    wxNotifyEvent\n
    wxCommandEvent\n
    wxEvent\n
    wxObject\n

    <h4>Include files</h4>

    <wx/propertygrid/propertygrid.h>
*/
class WXDLLIMPEXP_PG wxPropertyGridEvent : public wxCommandEvent
{
public:

    /** Constructor. */
    wxPropertyGridEvent(wxEventType commandType=0, int id=0);
#ifndef SWIG
    /** Copy constructor. */
    wxPropertyGridEvent(const wxPropertyGridEvent& event);
#endif
    /** Destructor. */
    ~wxPropertyGridEvent();

    /** Copyer. */
    virtual wxEvent* Clone() const;

    /** Enables property. */
    inline void EnableProperty( bool enable = true )
    {
        m_pg->EnableProperty(wxPGIdGen(m_property),enable);
    }

    /** Disables property. */
    inline void DisableProperty()
    {
        m_pg->EnableProperty(wxPGIdGen(m_property),false);
    }

    inline wxPGId GetMainParent() const
    {
        wxASSERT(m_property);
        return wxPGIdGen(m_property->GetMainParent());
    }

    /** Returns id of associated property. */
    wxPGId GetProperty() const
    {
        return wxPGIdGen(m_property);
    }

#ifndef SWIG
    /** Returns pointer to associated property. */
    wxPGProperty* GetPropertyPtr() const
    {
        return m_property;
    }
#endif

    /** Returns label of associated property. */
    const wxString& GetPropertyLabel() const
    {
        wxASSERT( m_property );
        return m_property->GetLabel();
    }

    /** Returns name of associated property. */
    const wxString& GetPropertyName() const
    {
        wxASSERT( m_property );
        return m_property->GetName();
    }

#if wxPG_USE_CLIENT_DATA
    /** Returns client data of relevant property. */
    wxPGProperty::ClientDataType GetPropertyClientData() const
    {
        wxASSERT( m_property );
        return m_property->GetClientData();
    }
#endif

#ifndef SWIG
    /** Returns value of relevant property. */
    wxVariant GetPropertyValue() const
    {
        wxASSERT( m_property );
        return m_property->GetValueAsVariant();
    }

    inline wxString GetPropertyValueAsString() const
    {
        return m_pg->GetPropertyValueAsString( wxPGIdGen(m_property) );
    }
    inline long GetPropertyValueAsLong() const
    {
        return m_pg->GetPropertyValueAsLong( wxPGIdGen(m_property) );
    }
    inline int GetPropertyValueAsInt() const
    {
        return (int)GetPropertyValueAsLong();
    }
    inline long GetPropertyValueAsBool() const
    {
        return m_pg->GetPropertyValueAsBool( wxPGIdGen(m_property) );
    }
    inline double GetPropertyValueAsDouble() const
    {
        return m_pg->GetPropertyValueAsDouble( wxPGIdGen(m_property) );
    }
    inline const wxObject* GetPropertyValueAsWxObjectPtr() const
    {
        return m_pg->GetPropertyValueAsWxObjectPtr( wxPGIdGen(m_property) );
    }
    inline void* GetPropertyValueAsVoidPtr() const
    {
        return m_pg->GetPropertyValueAsVoidPtr( wxPGIdGen(m_property) );
    }
#if !wxPG_PGVARIANT_IS_VARIANT
    inline const wxArrayString& GetPropertyValueAsArrayString() const
    {
        return m_pg->GetPropertyValueAsArrayString( wxPGIdGen(m_property) );
    }
    inline const wxPoint& GetPropertyValueAsPoint() const
    {
        return m_pg->GetPropertyValueAsPoint( wxPGIdGen(m_property) );
    }
    inline const wxSize& GetPropertyValueAsSize() const
    {
        return m_pg->GetPropertyValueAsSize( wxPGIdGen(m_property) );
    }
    inline const wxArrayInt& GetPropertyValueAsArrayInt() const
    {
        return m_pg->GetPropertyValueAsArrayInt( wxPGIdGen(m_property) );
    }
#else
    inline wxArrayString GetPropertyValueAsArrayString() const
    {
        return m_pg->GetPropertyValueAsArrayString( wxPGIdGen(m_property) );
    }
    inline wxPoint GetPropertyValueAsPoint() const
    {
        return m_pg->GetPropertyValueAsPoint( wxPGIdGen(m_property) );
    }
    inline wxSize GetPropertyValueAsSize() const
    {
        return m_pg->GetPropertyValueAsSize( wxPGIdGen(m_property) );
    }
    inline wxArrayInt GetPropertyValueAsArrayInt() const
    {
        return m_pg->GetPropertyValueAsArrayInt( wxPGIdGen(m_property) );
    }
#endif

    /** Returns value type of relevant property. */
    const wxPGValueType* GetPropertyValueType() const
    {
        return m_property->GetValueTypePtr();
    }

#else
    %pythoncode {
        def GetPropertyValue(self):
            return self.GetProperty().GetValue()
    }
#endif

    /** Returns true if event has associated property. */
    inline bool HasProperty() const { return ( m_property != (wxPGProperty*) NULL ); }

    inline bool IsPropertyEnabled() const
    {
        return m_pg->IsPropertyEnabled(wxPGIdGen(m_property));
    }

#ifndef SWIG

    // IsPending() == true, if sent using AddPendingEvent
    inline void SetPending( bool pending ) { m_pending = pending; }
    inline bool IsPending() const { return m_pending; }

#if !wxPG_ID_IS_PTR
    /** Changes the associated property. */
    void SetProperty( wxPGId id ) { m_property = wxPGIdToPtr(id); }
#endif

    /** Changes the associated property. */
    void SetProperty( wxPGProperty* p ) { m_property = p; }

    void SetPropertyGrid( wxPropertyGrid* pg ) { m_pg = pg; }

private:
    DECLARE_DYNAMIC_CLASS(wxPropertyGridEvent)

    wxPGProperty*       m_property;
    wxPropertyGrid*     m_pg;
    bool                m_pending;
#endif
};


#define wxPG_BASE_EVT_PRE_ID     1775

#ifndef SWIG
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_SELECTED,           wxPG_BASE_EVT_PRE_ID)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_CHANGED,            wxPG_BASE_EVT_PRE_ID+1)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_HIGHLIGHTED,        wxPG_BASE_EVT_PRE_ID+2)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_RIGHT_CLICK,        wxPG_BASE_EVT_PRE_ID+3)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_PAGE_CHANGED,       wxPG_BASE_EVT_PRE_ID+4)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_ITEM_COLLAPSED,     wxPG_BASE_EVT_PRE_ID+5)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_ITEM_EXPANDED,      wxPG_BASE_EVT_PRE_ID+6)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_DOUBLE_CLICK,       wxPG_BASE_EVT_PRE_ID+7)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_COMPACT_MODE_ENTERED,  wxPG_BASE_EVT_PRE_ID+8)
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PG, wxEVT_PG_EXPANDED_MODE_ENTERED, wxPG_BASE_EVT_PRE_ID+9)
END_DECLARE_EVENT_TYPES()
#else
    enum {
        wxEVT_PG_SELECTED = wxPG_BASE_EVT_PRE_ID,
        wxEVT_PG_CHANGED,
        wxEVT_PG_HIGHLIGHTED,
        wxEVT_PG_RIGHT_CLICK,
        wxEVT_PG_PAGE_CHANGED,
        wxEVT_PG_ITEM_COLLAPSED,
        wxEVT_PG_ITEM_EXPANDED,
        wxEVT_PG_DOUBLE_CLICK,
        wxEVT_PG_COMPACT_MODE_ENTERED,
        wxEVT_PG_EXPANDED_MODE_ENTERED
    };
#endif


#define wxPG_BASE_EVT_TYPE       wxEVT_PG_SELECTED
#define wxPG_MAX_EVT_TYPE        (wxPG_BASE_EVT_TYPE+30)


#ifndef SWIG
typedef void (wxEvtHandler::*wxPropertyGridEventFunction)(wxPropertyGridEvent&);

#define EVT_PG_SELECTED(id, fn)              DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_SELECTED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_CHANGED(id, fn)               DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_CHANGED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_HIGHLIGHTED(id, fn)           DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_HIGHLIGHTED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_RIGHT_CLICK(id, fn)           DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_RIGHT_CLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_DOUBLE_CLICK(id, fn)          DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_DOUBLE_CLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_PAGE_CHANGED(id, fn)          DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_PAGE_CHANGED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_ITEM_COLLAPSED(id, fn)        DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_ITEM_COLLAPSED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_ITEM_EXPANDED(id, fn)         DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_ITEM_EXPANDED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_COMPACT_MODE_ENTERED(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_COMPACT_MODE_ENTERED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PG_EXPANDED_MODE_ENTERED(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_PG_EXPANDED_MODE_ENTERED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxPropertyGridEventFunction, & fn ), (wxObject *) NULL ),

#define wxPropertyGridEventHandler(A) ((wxObjectEventFunction)(wxEventFunction)(wxPropertyGridEventFunction)&A)

#endif


// -----------------------------------------------------------------------


/** \class wxPropertyGridPopulator
    \ingroup classes
    \brief Allows populating wxPropertyGrid from arbitrary text source.
*/
class WXDLLIMPEXP_PG wxPropertyGridPopulator
{
public:
    /** Constructor.
        \param pg
        Property grid to populate.
        \param popRoot
        Base parent property. Default is root.
    */
    inline wxPropertyGridPopulator(wxPropertyGrid* pg = (wxPropertyGrid*) NULL,
                                   wxPGId popRoot = wxNullProperty)
    {
        Init(pg, popRoot);
    }

    /** Destructor. */
    ~wxPropertyGridPopulator();

    /** Adds a new set of choices with given id, labels and optional values.
        \remarks
        choicesId can be any id unique in source (so it does not conflict
        with sets of choices created before population process).
    */
    void AddChoices(wxPGChoicesId choicesId,
                    const wxArrayString& choiceLabels,
                    const wxArrayInt& choiceValues = wxPG_EMPTY_ARRAYINT);

    /** Appends a property under current parent.
        \param classname
        Class name of a property. Understands both wxXXXProperty
        and XXX style names. Thus, for example, wxStringProperty
        could be created with class names "wxStringProperty", and
        "String". Short class name of wxPropertyCategory is
        "Category".
        \param label
        Label for property. Use as in constructor functions.
        \param name
        Name for property. Use as in constructor functions.
        \param value
        Value for property is interpreted from this string.
        \param attributes
        Attributes of a property (both pseudo-attributes like
        "Disabled" and "Modified" in addition to real ones
        like "Precision") are read from this string. Is intended
        for string like one generated by GetPropertyAttributes.
        \param choicesId
        If non-zero: Id for set of choices unique in source. Pass
        either id previously given to AddChoices or a new one.
        If new is given, then choiceLabels and choiceValues are
        loaded as the contents for the newly created set of choices.
        \param choiceLabels
        List of choice labels.
        \param choiceValues
        List of choice values.
    */
    wxPGId AppendByClass(const wxString& classname,
                         const wxString& label,
                         const wxString& name = wxPG_LABEL,
                         const wxString& value = wxEmptyString,
                         const wxString& attributes = wxEmptyString,
                         wxPGChoicesId choicesId = (wxPGChoicesId)0,
                         const wxArrayString& choiceLabels = wxPG_EMPTY_ARRAYSTRING,
                         const wxArrayInt& choiceValues = wxPG_EMPTY_ARRAYINT);

    /** Appends a property under current parent. Works just as
        AppendByClass, except accepts value type name instead of
        class name (value type name of a property can be queried using
        wxPropertyGrid::GetPropertyValueType(property)->GetType()).

        \remarks
        <b>Cannot</b> generate property category.
    */
    wxPGId AppendByType(const wxString& valuetype,
                        const wxString& label,
                        const wxString& name = wxPG_LABEL,
                        const wxString& value = wxEmptyString,
                        const wxString& attributes = wxEmptyString,
                        wxPGChoicesId choicesId = (wxPGChoicesId)0,
                        const wxArrayString& choiceLabels = wxPG_EMPTY_ARRAYSTRING,
                        const wxArrayInt& choiceValues = wxPG_EMPTY_ARRAYINT);

    /** Returns id of parent property for which children can currently be added. */
    inline wxPGId GetCurrentParent() const
    {
        return m_curParent;
    }

    /** Returns true if set of choices with given id has already been added. */
    bool HasChoices( wxPGChoicesId id ) const;

    /** Sets the property grid to be populated. */
    inline void SetGrid( wxPropertyGrid* pg )
    {
        m_propGrid = pg;
    }

    /** If possible, sets the property last added as current parent. */
    bool BeginChildren();

    /** Terminates current parent - sets its parent as the new current parent. */
    inline void EndChildren()
    {
        wxASSERT( wxPGIdIsOk(m_curParent) );
        m_curParent = wxPGIdGen(wxPGIdToPtr(m_curParent)->GetParent());
        m_lastProperty = wxPGIdGen((wxPGProperty*)NULL);
    }

protected:

    wxPGId DoAppend(wxPGProperty* p,
                    const wxString& value,
                    const wxString& attributes,
                    wxPGChoicesId choicesId,
                    const wxArrayString& choiceLabels,
                    const wxArrayInt& choiceValues);

    void Init( wxPropertyGrid* pg, wxPGId popRoot );

    /** Used property grid. */
    wxPropertyGrid* m_propGrid;

    /** Population root. */
    wxPGId          m_popRoot;

    /** Parent of currently added properties. */
    wxPGId          m_curParent;

    /** Id of property last added. */
    wxPGId          m_lastProperty;

    /** Hashmap for source-choices-id to wxPGChoicesData mapping. */
    wxPGHashMapP2P  m_dictIdChoices;
};

// -----------------------------------------------------------------------

//
// Undefine macros that are not needed outside propertygrid sources
//
#ifndef __wxPG_SOURCE_FILE__
    #undef wxPG_FL_DESC_REFRESH_REQUIRED
    #undef wxPG_FL_SCROLLBAR_DETECTED
    #undef wxPG_FL_CREATEDSTATE
    #undef wxPG_FL_NOSTATUSBARHELP
    #undef wxPG_FL_SCROLLED
    #undef wxPG_FL_HIDE_STATE
    #undef wxPG_FL_FOCUS_INSIDE_CHILD
    #undef wxPG_FL_FOCUS_INSIDE
    #undef wxPG_FL_MOUSE_INSIDE_CHILD
    #undef wxPG_FL_CUR_USES_CUSTOM_IMAGE
    #undef wxPG_FL_PRIMARY_FILLS_ENTIRE
    #undef wxPG_FL_VALUE_MODIFIED
    #undef wxPG_FL_MOUSE_INSIDE
    #undef wxPG_FL_FOCUSED
    #undef wxPG_FL_MOUSE_CAPTURED
    #undef wxPG_FL_INITIALIZED
    #undef wxPG_FL_ACTIVATION_BY_CLICK
    #undef wxPG_FL_DONT_CENTER_SPLITTER
    #undef wxPG_SUPPORT_TOOLTIPS
    #undef wxPG_DOUBLE_BUFFER
    #undef wxPG_HEAVY_GFX
    #undef wxPG_ICON_WIDTH
    #undef wxPG_USE_RENDERER_NATIVE
// Following are needed by the manager headers
//    #undef wxPGIdGen
//    #undef wxPGPropNameStr
//    #undef wxPGIdToPtr
#endif

// Doxygen special
#ifdef DOXYGEN
    #include "manager.h"
#endif

// -----------------------------------------------------------------------

#endif // __WX_PROPGRID_PROPGRID_H__

