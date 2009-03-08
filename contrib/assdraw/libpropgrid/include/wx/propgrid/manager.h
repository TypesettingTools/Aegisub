/////////////////////////////////////////////////////////////////////////////
// Name:        manager.h
// Purpose:     wxPropertyGridManager
// Author:      Jaakko Salli
// Modified by:
// Created:     Jan-14-2005
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPGRID_MANAGER_H_
#define _WX_PROPGRID_MANAGER_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "manager.cpp"
#endif

#include <wx/propgrid/propgrid.h>

// -----------------------------------------------------------------------

#ifndef SWIG
extern WXDLLIMPEXP_PG const wxChar *wxPropertyGridManagerNameStr;
#endif

/** \class wxPropertyGridPage
    \ingroup classes
    \brief
    Holder of property grid page information. You can subclass this and
    give instance in wxPropertyGridManager::AddPage. It inherits from
    wxEvtHandler and can be used to process events specific to this
    page (id of events will still be same as manager's). If you don't
    want to use it to process all events of the page, you need to
    return false in the derived wxPropertyGridPage::IsHandlingAllEvents.

    Please note that wxPropertyGridPage lacks many non-const property
    manipulation functions found in wxPropertyGridManager. Please use
    parent manager (m_manager member variable) when needed.

    <h4>Derived from</h4>

    wxPropertyGridState\n
    wxPropertyContainerMethods\n
    wxEvtHandler\n
    wxObject\n

    <h4>Include files</h4>

    <wx/propgrid/manager.h>

    <h4>Event handling</h4>

    wxPropertyGridPage receives events emitted by its wxPropertyGridManager, but
    only those events that are specific to that page. If wxPropertyGridPage::IsHandlingAllEvents
    returns false, then unhandled events are sent to the manager's parent, as usual.
*/
class WXDLLIMPEXP_PG wxPropertyGridPage : public wxEvtHandler,
                                          public wxPropertyContainerMethods,
                                          public wxPropertyGridState
{
    friend class wxPropertyGridManager;
#ifndef SWIG
	DECLARE_CLASS(wxPropertyGridPage)
#endif
public:

    wxPropertyGridPage();
    virtual ~wxPropertyGridPage();

    /** See wxPropertyGrid::AppendIn. */
    inline wxPGId AppendIn( wxPGId id, wxPGProperty* property )
    {
        return DoInsert((wxPGPropertyWithChildren*)(wxPGProperty*)id,-1,property);
    }

    /** See wxPropertyGrid::AppendIn. */
    inline wxPGId AppendIn( wxPGPropNameStr name, wxPGProperty* property )
    {
        return DoInsert((wxPGPropertyWithChildren*)(wxPGProperty*)BaseGetPropertyByName(name),-1,property);
    }

    /** Return pointer to contained property grid state.
    */
    inline wxPropertyGridState* GetStatePtr()
    {
        return this;
    }

    /** See wxPropertyGrid::Insert. */
    wxPGId Insert( wxPGId id, int index, wxPGProperty* property );

    /** See wxPropertyGrid::Insert. */
    wxPGId Insert( wxPGPropNameStr name, int index, wxPGProperty* property );

    /** Do any member initialization in this method.
        \remarks
        - Called every time the page is added into a manager.
        - You can add properties to the page here.
    */
    virtual void Init() {};

    /** Return false here to indicate unhandled events should be
        propagated to manager's parent, as normal.
    */
    virtual bool IsHandlingAllEvents() const { return true; }

#ifndef SWIG
protected:

    virtual void RefreshProperty( wxPGProperty* p );

    //virtual bool ProcessEvent( wxEvent& event );

    wxPropertyGridManager*  m_manager;
    wxString                m_label;
    int                     m_id;  // toolbar index

private:
    bool                    m_isDefault; // is this base page object?

private:
    DECLARE_EVENT_TABLE()
#endif
};

// -----------------------------------------------------------------------


#define wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_BODY(NAME,AT1) \
{ \
    wxPGProperty* p = wxPGIdToPtr(id); \
    wxASSERT_MSG(p,wxT("invalid property id")); \
    if ( p ) \
    { \
        wxPropertyGridState* pState = p->GetParentState(); \
        wxASSERT( pState != (wxPropertyGridState*) NULL ); \
        if ( pState == m_pPropGrid->m_pState ) m_pPropGrid->NAME(id,_av1_); \
        else pState->NAME(p,_av1_); \
    } \
}

// _P1 version doesn't have overloaded version that accepts name instead of id.
#define wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1(NAME,AT1) \
wxPG_IPAM_DECL void wxPropertyGridManager::NAME( wxPGId id, AT1 _av1_ ) \
wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_BODY(NAME,AT1)

#define wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_INBODY(NAME,AT1) \
wxPG_IPAM_DECL void NAME( wxPGId id, AT1 _av1_ ) \
wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_BODY(NAME,AT1)


// This is for mirroring wxPropertyGrid methods with ease.
// Needs to be in here because of inlines.
#define wxPG_IMPLEMENT_PGMAN_METHOD_NORET1(NAME,AT1) \
wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1(NAME,AT1) \
wxPG_IPAM_DECL void wxPropertyGridManager::NAME( wxPGPropNameStr name, AT1 _av1_ ) \
{ \
    wxPGId id = GetPropertyByNameI(name); \
    if ( !wxPGIdIsOk(id) ) return; \
    NAME(id,_av1_); \
}


#define wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(NAME,AT1) \
wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_INBODY(NAME,AT1) \
wxPG_IPAM_DECL void NAME( wxPGPropNameStr name, AT1 _av1_ ) \
{ \
    NAME(GetPropertyByNameI(name),_av1_); \
}


//class wxPropertyGridPageData;

#define wxPG_IPAM_DECL inline

/** \class wxPropertyGridManager
    \ingroup classes
    \brief
    wxPropertyGridManager is an efficient multi-page version of wxPropertyGrid,
    which can optionally have toolbar for mode and page selection, help text box,
    and a compactor button.
    Use window flags to select which ones to include.

    <h4>Derived from</h4>

    wxPropertyContainerMethods\n
    wxWindow\n
    wxEvtHandler\n
    wxObject\n

    <h4>Include files</h4>

    <wx/propgrid/manager.h>

    <h4>Window styles</h4>

    @link wndflags Additional Window Styles@endlink

    <h4>Event handling</h4>

    To process input from a propertygrid control, use these event handler macros to
    direct input to member functions that take a wxPropertyGridEvent argument.

    <table>
    <tr><td>EVT_PG_SELECTED (id, func)</td><td>Property is selected.</td></tr>
    <tr><td>EVT_PG_CHANGED (id, func)</td><td>Property value is modified.</td></tr>
    <tr><td>EVT_PG_HIGHLIGHTED (id, func)</td><td>Mouse moves over property. Event's property is NULL if hovered on area that is not a property.</td></tr>
    <tr><td>EVT_PG_PAGE_CHANGED (id, func)</td><td>User changed page in manager.</td></tr>
    <tr><td>EVT_PG_ITEM_COLLAPSED (id, func)</td><td>User collapses a property or category.</td></tr>
    <tr><td>EVT_PG_ITEM_EXPANDED (id, func)</td><td>User expands a property or category.</td></tr>
    <tr><td>EVT_PG_COMPACT_MODE_ENTERED (id, func)</td><td>User presses the compactor button, compact mode is entered.</td></tr>
    <tr><td>EVT_PG_EXPANDED_MODE_ENTERED (id, func)</td><td>User presses the compactor button, expanded mode is entered.</td></tr>

    <tr><td>EVT_BUTTON (id, func)</td><td>Button in a property editor was clicked. Only occurs if the property doesn't handle button clicks itself.</td></tr>
    <tr><td>EVT_TEXT (id, func)</td><td>wxTextCtrl based editor was updated (but property value was not yet modified)</td></tr>
    </table>

    \sa @link wxPropertyGridEvent wxPropertyGridEvent@endlink

*/
// BM_MANAGER
class WXDLLIMPEXP_PG wxPropertyGridManager : public wxPanel, public wxPropertyContainerMethods
{
#ifndef SWIG
	DECLARE_CLASS(wxPropertyGridManager)
#endif
    friend class wxPropertyGridPage;
public:

#ifdef SWIG
    %pythonAppend wxPropertyGridManager {
        self._setOORInfo(self)
        self.DoDefaultTypeMappings()
        self.edited_objects = {}
        self.DoDefaultValueTypeMappings()
        if not hasattr(self.__class__,'_vt2setter'):
            self.__class__._vt2setter = {}
    }
    %pythonAppend wxPropertyGridManager() ""

    wxPropertyGridManager( wxWindow *parent, wxWindowID id = wxID_ANY,
               	           const wxPoint& pos = wxDefaultPosition,
               	           const wxSize& size = wxDefaultSize,
               	           long style = wxPGMAN_DEFAULT_STYLE,
               	           const wxChar* name = wxPyPropertyGridManagerNameStr );
    %RenameCtor(PrePropertyGridManager,  wxPropertyGridManager());

#else

	/** Two step constructor. Call Create when this constructor is called to build up the
	    wxPropertyGridManager.
	*/
    wxPropertyGridManager();

    /** The default constructor. The styles to be used are styles valid for
        the wxWindow.
        \sa @link wndflags Additional Window Styles@endlink
    */
    wxPropertyGridManager( wxWindow *parent, wxWindowID id = wxID_ANY,
               	           const wxPoint& pos = wxDefaultPosition,
               	           const wxSize& size = wxDefaultSize,
               	           long style = wxPGMAN_DEFAULT_STYLE,
               	           const wxChar* name = wxPropertyGridManagerNameStr );

    /** Destructor */
    virtual ~wxPropertyGridManager();

#endif

    /** Creates new property page. Note that the first page is not created
        automatically.
        \param label
        A label for the page. This may be shown as a toolbar tooltip etc.
        \param bmp
        Bitmap image for toolbar. If wxNullBitmap is used, then a built-in
        default image is used.
        \param pageObj
        wxPropertyGridPage instance. Manager will take ownership of this object.
        NULL indicates that a default page instance should be created.
        \retval
        Returns index to the page created.
        \remarks
        If toolbar is used, it is highly recommended that the pages are
        added when the toolbar is not turned off using window style flag
        switching.
    */
    inline int AddPage( const wxString& label = wxEmptyString,
                        const wxBitmap& bmp = wxPG_NULL_BITMAP,
                        wxPropertyGridPage* pageObj = (wxPropertyGridPage*) NULL )
    {
        return InsertPage(-1,label,bmp,pageObj);
    }

    /** See wxPropertyGrid::Append. */
    inline wxPGId AppendCategory( const wxString& label, const wxString& name = wxPG_LABEL )
    {
        wxCHECK( m_targetState, wxNullProperty );
        return m_targetState->Append( new wxPropertyCategoryClass(label,name) );
    }

    /** See wxPropertyGrid::Append. */
    inline wxPGId Append( wxPGProperty* property )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->Append(property);
    }

    inline wxPGId Append( const wxString& label, const wxString& name, const wxString& value = wxEmptyString )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->Append( wxStringProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name, int value )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->Append( wxIntProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name, double value )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->Append( wxFloatProperty(label,name,value) );
    }

    inline wxPGId Append( const wxString& label, const wxString& name, bool value )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->Append( wxBoolProperty(label,name,value) );
    }

    /** See wxPropertyGrid::AppendIn. */
    inline wxPGId AppendIn( wxPGId id, wxPGProperty* property )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->DoInsert((wxPGPropertyWithChildren*)(wxPGProperty*)id,-1,property);
    }

    /** See wxPropertyGrid::AppendIn. */
    inline wxPGId AppendIn( wxPGPropNameStr name, wxPGProperty* property )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxCHECK_MSG( m_selPage >= 0, wxNullProperty, wxT("You need to add a page before adding properties") );
        return m_targetState->DoInsert((wxPGPropertyWithChildren*)(wxPGProperty*)m_targetState->BaseGetPropertyByName(name),-1,property);
    }

    /** Returns true if all property grid data changes have been committed. Usually
        only returns false if value in active editor has been invalidated by a
        wxValidator.
    */
    inline bool CanClose()
    {
        return m_pPropGrid->CanClose();
    }

    void ClearModifiedStatus ( wxPGId id );

    inline void ClearModifiedStatus ()
    {
        m_pPropGrid->ClearModifiedStatus();
    }

    /** Clears the target page.
    */
    /*inline void ClearTargetPage()
    {
        ClearPage(m_targetPage);
    }*/

    /** Deletes all properties on given page.
    */
    void ClearPage( int page );

    bool ClearPropertyValue( wxPGId id );
    bool ClearPropertyValue( wxPGPropNameStr name );

    /** Collapses given item. Returns true if it was collapsable and previously expanded. */
    bool Collapse( wxPGId id );
    bool Collapse( wxPGPropNameStr name );

    /** Collapses all items that can be collapsed.
        \retval
        Return false if failed (may fail if editor value cannot be validated).
    */
    inline bool CollapseAll()
    {
        return m_targetState->ExpandAll(0);
    }

    /** Compacts (arg is true) or expands the propertygrid (i.e. low priority
        items are already hidden or shown).
    */
    bool Compact( bool compact );

    /** Two step creation. Whenever the control is created without any parameters,
        use Create to actually create it. Don't access the control's public methods
        before this is called.
        \sa @link wndflags Additional Window Styles@endlink
    */
    bool Create( wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxPGMAN_DEFAULT_STYLE,
                 const wxChar* name = wxPropertyGridManagerNameStr );

    /** Disables a property. */
    inline bool DisableProperty( wxPGId id ) { return EnableProperty (id,false); }

    /** Disables a property. */
    inline bool DisableProperty( wxPGPropNameStr name ) { return EnableProperty (name,false); }

    /** Enables or disables (shows/hides) categories according to parameter enable.
        WARNING: Not tested properly, use at your own risk.
    */
    inline bool EnableCategories( bool enable )
    {
        long fl = m_windowStyle | wxPG_HIDE_CATEGORIES;
        if ( enable ) fl = m_windowStyle & ~(wxPG_HIDE_CATEGORIES);
        SetWindowStyleFlag(m_windowStyle);
        return true;
    }

    /** Enables or disables a property on target page. */
    bool EnableProperty( wxPGId id, bool enable = true );
    bool EnableProperty( wxPGPropNameStr name, bool enable = true );

    /** Selects page, scrolls and/or expands items to ensure that the
        given item is visible. Returns true if something was actually done.
    */
    bool EnsureVisible( wxPGId id );

    /** Selects page, scrolls and/or expands items to ensure that the
        given item is visible. Returns true if something was actually done.
    */
    inline bool EnsureVisible( wxPGPropNameStr name )
    {
        return EnsureVisible(GetPropertyByNameI(name));
    }

    /** Expands given item. Returns true if it was expandable and previously collapsed. */
    bool Expand( wxPGId id );
    bool Expand( wxPGPropNameStr name );

    /** Expands all parents on target page.
        \retval
        Return false if failed (may fail if editor value cannot be validated).
    */
    bool ExpandAll()
    {
        return m_targetState->ExpandAll(1);
    }

    /** Returns number of children of the root property of the selected page. */
    inline size_t GetChildrenCount()
    {
        return GetChildrenCount( wxPGIdGen(m_pPropGrid->m_pState->m_properties) );
    }

    /** Returns number of children of the root property of given page. */
    size_t GetChildrenCount( int pageIndex );

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

    /** Returns height of the description text box. */
    int GetDescBoxHeight() const;

    /** Returns id of first item (in target page), whether it is a category or a property. */
    inline wxPGId GetFirst() const
    {
        return m_targetState->GetFirst();
    }

    /** Returns id of first category (in target page). */
    inline wxPGId GetFirstCategory() const
    {
        return m_targetState->GetFirstCategory ();
    }

    /** Returns id of first property that is not a category (from target page). */
    inline wxPGId GetFirstProperty() const
    {
        return m_targetState->GetFirstProperty();
    }

    /** Returns pointer to the contained wxPropertyGrid. This does not change
        after wxPropertyGridManager has been created, so you can safely obtain
        pointer once and use it for the entire lifetime of the instance.
    */
    inline wxPropertyGrid* GetGrid()
    {
        wxASSERT(m_pPropGrid);
        return m_pPropGrid;
    };

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

    /** Returns id of next category after a given property (which does not have to be category). */
    inline wxPGId GetNextCategory( wxPGId id ) const
    {
        wxCHECK( m_targetState, wxNullProperty );
        return m_targetState->GetNextCategory(id);
    }

    /** Returns id of next property (from target page). This does <b>not</b> iterate
        to sub-properties or categories.
    */
    inline wxPGId GetNextProperty( wxPGId id ) const
    {
        wxCHECK( m_targetState, wxNullProperty );
        return m_targetState->GetNextProperty(id);
    }

    /** Returns page object for given page index.
    */
    inline wxPropertyGridPage* GetPage( unsigned int ind ) const
    {
        return (wxPropertyGridPage*)m_arrPages.Item(ind);
    }

    /** Returns index for a page name. If no match is found, wxNOT_FOUND is returned. */
    int GetPageByName( const wxChar* name ) const;

    /** Returns index for a relevant propertygrid state. If no match is found,
        wxNOT_FOUND is returned.
    */
    int GetPageByState( wxPropertyGridState* pstate ) const;

    /** Returns number of managed pages. */
    size_t GetPageCount() const;

    /** Returns name of given page. */
    const wxString& GetPageName( int index ) const;

    /** Returns "root property" of the given page. It does not have name, etc.
        and it is not visible. It is only useful for accessing its children.
    */
    wxPGId GetPageRoot( int index ) const;

    /** Returns id of previous property (in target page). */
    inline wxPGId GetPrevProperty( wxPGId id ) const
    {
        return m_targetState->GetPrevProperty(id);
    }

    /** Returns id of previous item under the same parent. */
    inline wxPGId GetPrevSibling( wxPGId id )
    {
        return wxPropertyGridState::GetPrevSibling(id);
    }
    inline wxPGId GetPrevSibling( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        return wxPropertyGridState::GetPrevSibling(wxPGIdGen(p));
    }

    /** Returns id of property with given label (case-sensitive). If there is no
        property with such label, returned property id is invalid ( i.e. it will return
        false with IsOk method). If there are multiple properties with identical name,
        most recent added is returned.
    */
    wxPGId GetPropertyByLabel( const wxString& name,
        wxPropertyGridState** ppState = (wxPropertyGridState**)NULL ) const;

    /** Returns id of property's nearest parent category. If no category
        found, returns invalid wxPGId.
    */
    inline wxPGId GetPropertyCategory( wxPGId id ) const
    {
        return m_pPropGrid->GetPropertyCategory(id);
    }
    wxPGId GetPropertyCategory( wxPGPropNameStr name ) const
    {
        return m_pPropGrid->GetPropertyCategory(name);
    }

    /** Returns cell background colour of a property. */
    inline wxColour GetPropertyColour( wxPGId id ) const
    {
        return m_pPropGrid->GetPropertyColour(id);
    }
    inline wxColour GetPropertyColour( wxPGPropNameStr name ) const
    {
        return m_pPropGrid->GetPropertyColour(name);
    }

    /** Returns cell text colour of a property. */
    inline wxColour GetPropertyTextColour( wxPGId id ) const
    {
        return m_pPropGrid->GetPropertyTextColour(id);
    }
    inline wxColour GetPropertyTextColour( wxPGPropNameStr name ) const
    {
        return m_pPropGrid->GetPropertyTextColour(name);
    }

    /** Returns a wxVariant list containing wxVariant versions of all
        property values. Order is not guaranteed, but generally it should
        match the visible order in the grid.
        \param flags
        Use wxKEEP_STRUCTURE to retain category structure; each sub
        category will be its own wxVariantList of wxVariant.
        \remarks
        This works on the target page (*not* the selected page).
    */
#ifndef SWIG
    wxVariant GetPropertyValues( const wxString& listname = wxEmptyString,
        wxPGId baseparent = wxNullProperty, long flags = 0 ) const
    {
        return m_targetState->GetPropertyValues(listname,baseparent,flags);
    }
#endif

    /** Returns "root property" of the target page. It does not have name, etc.
        and it is not visible. It is only useful for accessing its children.
    */
    wxPGId GetRoot() const { return wxPGIdGen(m_targetState->m_properties); }

    /** Returns index to currently selected page. */
    inline int GetSelectedPage() const { return m_selPage; }

    /** Shortcut for GetGrid()->GetSelection(). */
    inline wxPGId GetSelectedProperty() const
    {
        return m_pPropGrid->GetSelection();
    }

    /** Synonyme for GetSelectedPage. */
    inline int GetSelection() const { return m_selPage; }

    /** Returns index of page that is target for insert operations etc. */
    inline int GetTargetPage() const { return m_targetPage; }

    /** Returns a pointer to the toolbar currently associated with the
        wxPropertyGridManager (if any). */
    inline wxToolBar* GetToolBar() const { return m_pToolbar; }

    inline void InitAllTypeHandlers()
    {
        wxPropertyGrid::InitAllTypeHandlers();
    }

    /** See wxPropertyGrid::Insert. */
    inline wxPGId Insert( wxPGId id, int index, wxPGProperty* property )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxPGId res_id = m_targetState->DoInsert((wxPGPropertyWithChildren*)wxPGIdToPtr(id),index,property);
        if ( m_targetState == m_pPropGrid->m_pState )
            m_pPropGrid->DrawItems ( property, (wxPGProperty*) NULL );
        return res_id;
    }

    /** See wxPropertyGrid::Insert. */
    inline wxPGId Insert( wxPGPropNameStr name, int index, wxPGProperty* property )
    {
        wxCHECK( m_targetState, wxNullProperty );
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(wxNullProperty)
        wxPGId res_id = m_targetState->DoInsert((wxPGPropertyWithChildren*)p,index,property);
        if ( m_targetState == m_pPropGrid->m_pState )
            m_pPropGrid->DrawItems( property, (wxPGProperty*) NULL );
        return res_id;
    }

    /** Creates new property page. Note that the first page is not created
        automatically.
        \param index
        Add to this position. -1 will add as the last item.
        \param label
        A label for the page. This may be shown as a toolbar tooltip etc.
        \param bmp
        Bitmap image for toolbar. If wxNullBitmap is used, then a built-in
        default image is used.
        \param pageObj
        wxPropertyGridPage instance. Manager will take ownership of this object.
        If NULL, default page object is constructed.
        \retval
        Returns index to the page created.
    */
    virtual int InsertPage( int index, const wxString& label, const wxBitmap& bmp = wxNullBitmap,
                            wxPropertyGridPage* pageObj = (wxPropertyGridPage*) NULL );

    /** Returns true if any property on any page has been modified by the user. */
    bool IsAnyModified() const;

    /** Returns true if updating is frozen (ie. Freeze() called but not yet Thaw() ). */
    inline bool IsFrozen() const { return (m_pPropGrid->m_frozen>0)?true:false; }

    /** Returns true if any property on given page has been modified by the user. */
    bool IsPageModified( size_t index ) const;

    /** Returns true if property is a category.
        NB: SHOULD BE IN METHODS CLASS.
    */
/*
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
*/

    /** Disables item's textctrl if other way of editing is available. */
    void LimitPropertyEditing( wxPGId id, bool limit = true );
    void LimitPropertyEditing( wxPGPropNameStr name, bool limit = true );

    virtual void Refresh( bool eraseBackground = true,
                          const wxRect* rect = (const wxRect*) NULL );

    /** Removes a page.
        \retval
        Returns false if it was not possible to remove page in question.
    */
    virtual bool RemovePage( int page );

    /** Select and displays a given page. Also makes it target page for
        insert operations etc.
        \param index
        Index of page being seleced. Can be -1 to select nothing.
    */
    void SelectPage( int index );

    /** Select and displays a given page. */
    inline void SelectPage( const wxChar* name )
    {
        SelectPage( GetPageByName(name) );
    }

    /** Select a property. */
    inline bool SelectProperty( wxPGId id, bool focus = false )
    {
        return wxPGIdToPtr(id)->GetParentState()->DoSelectProperty(id,focus);
    }
    inline bool SelectProperty( wxPGPropNameStr name, bool focus = false )
    {
        wxPG_PROP_NAME_CALL_PROLOG_RETVAL(false)
        return p->GetParentState()->DoSelectProperty(p,focus);
    }

    /** Sets label and text in description box.
    */
    void SetDescription( const wxString& label, const wxString& content );

    /** Sets text colour of a category caption (but not it's children).
    */
    inline void SetCaptionTextColour( wxPGId id, const wxColour& col )
    {
        m_pPropGrid->SetCaptionTextColour( id, col );
    }
    inline void SetCaptionTextColour( wxPGPropNameStr name, const wxColour& col )
    {
        m_pPropGrid->SetCaptionTextColour( name, col );
    }

    /** Sets the current category - Append will add non-categories under this one.
    */
    inline void SetCurrentCategory( wxPGId id )
    {
        wxPropertyCategoryClass* pc = (wxPropertyCategoryClass*)wxPGIdToPtr(id);
#ifdef __WXDEBUG__
        if ( pc ) wxASSERT( pc->GetParentingType() > 0 );
#endif
        m_targetState->m_currentCategory = pc;
    }

    /** Sets the current category - Append will add non-categories under this one.
    */
    inline void SetCurrentCategory( wxPGPropNameStr name )
    {
        wxPG_PROP_NAME_CALL_PROLOG()
        SetCurrentCategory(wxPGIdGen(p));
    }

    /** Sets y coordinate of the description box splitter. */
    void SetDescBoxHeight( int ht, bool refresh = true );

    /** All properties added/inserted will have given priority by default.
        \param
        priority can be wxPG_HIGH (default) or wxPG_LOW.
    */
    inline void SetDefaultPriority( int priority )
    {
        m_pPropGrid->SetDefaultPriority(priority);
    }

    /** Same as SetDefaultPriority(wxPG_HIGH). */
    inline void ResetDefaultPriority()
    {
        m_pPropGrid->ResetDefaultPriority();
    }

    /** Sets property attribute for all applicapple properties.
        Be sure to use this method after all properties have been
        added to the grid.
    */
    void SetPropertyAttributeAll( int attrid, wxVariant value );

    /** Sets label of a property.
    */
    void SetPropertyLabel( wxPGId id, const wxString& newlabel );
    void SetPropertyLabel( wxPGPropNameStr name, const wxString& newlabel );

    /** Sets background colour of property and all its children. Colours of
        captions are not affected. Background brush cache is optimized for often
        set colours to be set last.

        NOTE: This function is deprecated. Use SetPropertyBackgroundColour.
    */
    inline void SetPropertyColour( wxPGId id, const wxColour& col )
    {
        m_pPropGrid->SetPropertyBackgroundColour( id, col );
    }
    inline void SetPropertyColour( wxPGPropNameStr name, const wxColour& col )
    {
        m_pPropGrid->SetPropertyBackgroundColour( name, col );
    }

    /** Sets background colour of property and all its children. Colours of
        captions are not affected. Background brush cache is optimized for often
        set colours to be set last.
    */
    inline void SetPropertyBackgroundColour( wxPGId id, const wxColour& col )
    {
        m_pPropGrid->SetPropertyBackgroundColour( id, col );
    }
    inline void SetPropertyBackgroundColour( wxPGPropNameStr name, const wxColour& col )
    {
        m_pPropGrid->SetPropertyBackgroundColour( name, col );
    }

    /** Sets text colour of property and all its children.
    */
    inline void SetPropertyTextColour( wxPGId id, const wxColour& col )
    {
        m_pPropGrid->SetPropertyTextColour( id, col );
    }
    inline void SetPropertyTextColour( wxPGPropNameStr name, const wxColour& col )
    {
        m_pPropGrid->SetPropertyTextColour( name, col );
    }

    /** Sets background and text colour of property and all its children to the default. */
    inline void SetPropertyColourToDefault( wxPGId id )
    {
        m_pPropGrid->SetPropertyColourToDefault(id);
    }
    inline void SetPropertyColourToDefault ( wxPGPropNameStr name )
    {
        m_pPropGrid->SetPropertyColourToDefault(name);
    }

    /** Sets property value.
        \remarks
        Actual name of this method is <b>SetPropertyValue</b>. It may be shown incorrectly
        due to limitations in Doxygen C preprocessor. */
    void SetPropertyValueLong( wxPGId id, long value );
#ifndef __WXPYTHON__
    void SetPropertyValue( wxPGId id, int value );
#endif
    void SetPropertyValueDouble( wxPGId id, double value );
    void SetPropertyValueBool( wxPGId id, bool value );
    void SetPropertyValueString( wxPGId id, const wxString& value );
    void SetPropertyValueArrstr2( wxPGId id, const wxArrayString& value );
    void SetPropertyValueWxObjectPtr( wxPGId id, wxObject* value );
#ifndef __WXPYTHON__
    void SetPropertyValue( wxPGId id, void* value );
    void SetPropertyValue( wxPGId id, wxVariant& value );
#endif

    void SetPropertyValueLong( wxPGPropNameStr name, long value );
#ifndef __WXPYTHON__
    void SetPropertyValue( wxPGPropNameStr name, int value );
#endif
    void SetPropertyValueDouble( wxPGPropNameStr name, double value );
    void SetPropertyValueBool( wxPGPropNameStr name, bool value );
    void SetPropertyValueString( wxPGPropNameStr name, const wxString& value );
    void SetPropertyValueWxObjectPtr( wxPGPropNameStr name, wxObject* value );
#ifndef __WXPYTHON__
    void SetPropertyValue( wxPGPropNameStr name, void* value );
    void SetPropertyValue( wxPGPropNameStr name, wxVariant& value );
    void SetPropertyValueArrstr2( wxPGPropNameStr name, const wxArrayString& value );
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValueArrint2,const wxArrayInt&)
#else
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_P1_INBODY(SetPropertyValueArrint2,const wxArrayInt&)
#endif
#if wxUSE_DATETIME
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValueDatetime,wxDateTime)
#endif
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValuePoint,const wxPoint&)
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValueSize,const wxSize&)
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValueLongLong,const wxLongLong&)
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValueULongLong,const wxULongLong&)
#ifdef __WXPYTHON__
    wxPG_IMPLEMENT_PGMAN_METHOD_NORET1_INBODY(SetPropertyValuePyObject,PyObject*)
#endif

#ifndef __WXPYTHON__
    inline void SetPropertyValue( wxPGId id, wxObject& value )
    {
        SetPropertyValue(id,&value);
    }

    inline void SetPropertyValue( wxPGPropNameStr name, wxObject& value )
    {
        SetPropertyValue(name,&value);
    }

    inline void SetPropertyValue( wxPGId id, const wxChar* value )
    {
        SetPropertyValue(id,wxString(value));
    }
    inline void SetPropertyValue( wxPGPropNameStr name, const wxChar* value )
    {
        SetPropertyValue(name,wxString(value));
    }

    /** Sets various property values from a list of wxVariants. If property with
        name is missing from the grid, new property is created under given default
        category (or root if omitted). Works on target page.
    */
    void SetPropertyValues( const wxVariantList& list, wxPGId defaultCategory )
    {
        m_targetState->SetPropertyValues(list,defaultCategory);
    }

    inline void SetPropertyValues( const wxVariant& list, wxPGId defaultCategory )
    {
        SetPropertyValues(list.GetList(),defaultCategory);
    }
    inline void SetPropertyValues( const wxVariantList& list, const wxString& defaultCategory = wxEmptyString )
    {
        SetPropertyValues(list,GetPropertyByName(defaultCategory));
    }
    inline void SetPropertyValues( const wxVariant& list, const wxString& defaultCategory = wxEmptyString )
    {
        SetPropertyValues(list.GetList(),GetPropertyByName(defaultCategory));
    }
#endif

    /** Sets property's value to unspecified. If it has children (it may be category),
        then the same thing is done to them.
    */
#ifdef wxPG_COMPATIBILITY_1_0_0
    void SetPropertyValueUnspecified( wxPGId id );
    void SetPropertyValueUnspecified( wxPGPropNameStr name );
#else
    void SetPropertyUnspecified( wxPGId id );
    void SetPropertyUnspecified( wxPGPropNameStr name );
#endif

    /** Moves splitter as left as possible, while still allowing all
        labels to be shown in full.
        \param subProps
        If false, will still allow sub-properties (ie. properties which
        parent is not root or category) to be cropped.
        \param allPages
        If true, takes labels on all pages into account.
    */
    void SetSplitterLeft( bool subProps = false, bool allPages = true );

    inline void SetSplitterPosition( int newx, bool refresh = true )
    {
        m_pPropGrid->SetSplitterPosition(newx,refresh);
    }

    /** Synonyme for SelectPage(name). */
    inline void SetStringSelection( const wxChar* name )
    {
        SelectPage( GetPageByName(name) );
    }

    /** Sets page to which append, insert, etc. will add items.
        Every time a page is changed, target page is automatically
        switched to that.
    */
    void SetTargetPage( int index );

    /** Sets page to which append, insert, etc. will add items.
        Every time a page is changed, target page is automatically
        switched to that.
    */
    inline void SetTargetPage( const wxChar* name )
    {
        SetTargetPage( GetPageByName(name) );
    }

    /** Sorts all items at all levels of the target page (except sub-properties). */
    inline void Sort()
    {
        m_pPropGrid->Sort(wxPGIdGen(m_targetState->m_properties));
    }

    /** Sorts children of a category.
    */
    inline void Sort( wxPGId id )
    {
        m_pPropGrid->Sort(id);
    }

    /** Sorts children of a category.
    */
    inline void Sort( wxPGPropNameStr name )
    {
        m_pPropGrid->Sort( GetPropertyByNameI(name) );
    }

    /** Deselect current selection, if any (from current page).
        \retval
        false if editor could not be closed.
    */
    inline bool ClearSelection()
    {
        return m_pPropGrid->ClearSelection();
    }

#ifdef SWIG
    %pythoncode {
        def GetValuesFromPage(self,page,dict_=None,as_strings=False):
            "Same as GetValues, but returns values from specific page only."
            ""
            "For argument descriptions, see GetValues."

            if dict_ is None:
                dict_ = {}
            elif hasattr(dict_,'__dict__'):
                dict_ = dict_.__dict__

            if not as_strings:
                getter = self.GetPropertyValue
            else:
                getter = self.GetPropertyValueAsString

            root = self.GetPageRoot(page)
            self._GetValues(root,self.GetFirstChild(root),dict_,getter)

            return dict_


        def GetValues(self,dict_=None,as_strings=False):
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

            for page in range(0,self.GetPageCount()):
                root = self.GetPageRoot(page)
                self._GetValues(root,self.GetFirstChild(root),dict_,getter)

            return dict_

        GetPropertyValues = GetValues

    }
#endif

protected:

    //
    // Subclassing helpers
    //

    /** Creates property grid for the manager. Override to use subclassed
        wxPropertyGrid.
    */
    virtual wxPropertyGrid* CreatePropertyGrid() const;

public:

#ifndef DOXYGEN

    //
    // Overridden functions - no documentation required.
    //

    virtual wxSize DoGetBestSize() const;
    void SetId( wxWindowID winid );

    virtual void Freeze();
    virtual void Thaw();
    virtual void SetExtraStyle ( long exStyle );
    virtual bool SetFont ( const wxFont& font );
    virtual void SetWindowStyleFlag ( long style );

#ifndef SWIG

    //
    // Event handlers
    //
    void OnMouseMove( wxMouseEvent &event );
    void OnMouseClick( wxMouseEvent &event );
    void OnMouseUp( wxMouseEvent &event );
    void OnMouseEntry( wxMouseEvent &event );

    void OnPaint( wxPaintEvent &event );

    void OnToolbarClick( wxCommandEvent &event );
    void OnResize( wxSizeEvent& event );
    void OnCompactorClick( wxCommandEvent& event );
    void OnPropertyGridSelect( wxPropertyGridEvent& event );

protected:

    wxPropertyGrid* m_pPropGrid;

    wxArrayPtrVoid  m_arrPages;

#if wxUSE_TOOLBAR
    wxToolBar*      m_pToolbar;
#endif
    wxStaticText*   m_pTxtHelpCaption;
    wxStaticText*   m_pTxtHelpContent;
    wxButton*       m_pButCompactor;

    wxPropertyGridState*    m_targetState;

    wxPropertyGridPage*     m_emptyPage;

    long            m_iFlags;

    // Selected page index.
    int             m_selPage;

    int             m_width;

    int             m_height;

    int             m_extraHeight;

    int             m_splitterY;

    int             m_splitterHeight;

    int             m_nextTbInd;

    int             m_dragOffset;

    wxCursor        m_cursorSizeNS;

    int             m_nextDescBoxSize;

    int             m_targetPage;

    unsigned char   m_dragStatus;

    unsigned char   m_onSplitter;



    virtual wxPGId DoGetPropertyByName( wxPGPropNameStr name ) const;

    /** Select and displays a given page. */
    virtual bool DoSelectPage( int index );

    virtual void RefreshProperty( wxPGProperty* p );

    // Sets some members to defaults.
	void Init1();

    // Initializes some members.
	void Init2( int style );

/*#ifdef __WXMSW__
    virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle) const;
#endif*/

    /** Recalculates new positions for components, according to the
        given size.
    */
    void RecalculatePositions( int width, int height );

    /** (Re)creates/destroys controls, according to the window style bits. */
    void RecreateControls();

    void RefreshHelpBox( int new_splittery, int new_width, int new_height );

    void RepaintSplitter( wxDC& dc, int new_splittery, int new_width, int new_height, bool desc_too );

    void SetDescribedProperty( wxPGProperty* p );

    virtual bool ProcessEvent( wxEvent& event );

private:
    DECLARE_EVENT_TABLE()
#endif // #ifndef SWIG
#endif // #ifndef DOXYGEN
};

// -----------------------------------------------------------------------

#ifndef __wxPG_SOURCE_FILE__
    #undef wxPG_IPAM_DECL
    #undef wxPG_IMPLEMENT_PGMAN_METHOD_NORET1
#endif

// -----------------------------------------------------------------------

#endif // _WX_PROPGRID_MANAGER_H_
