/////////////////////////////////////////////////////////////////////////////
// Name:        pg_dox_mainpage.h
// Purpose:     wxPropertyGrid Doxygen Documentation
// Author:      Jaakko Salli
// Modified by:
// Created:     Oct-08-2004
// RCS-ID:      $Id:
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PG_DOX_MAINPAGE_H__
#define __WX_PG_DOX_MAINPAGE_H__

/**
    \mainpage wxPropertyGrid 1.2.11 Overview

      wxPropertyGrid is a specialized two-column grid for editing properties
    such as strings, numbers, flagsets, fonts, and colours. It allows hierarchial,
    collapsible properties ( via so-called categories that can hold child
    properties), sub-properties, and has strong wxVariant support (for example,
    allows populating from near-arbitrary list of wxVariants).

    <b>Documentation for wxPython bindings:</b> For a tutorial see the accompanied
    wxPython readme file and the test_propgrid.py sample. Otherwise, individual
    member functions should work very much the same as with the C++ wxWidgets,
    so you'll probably find wxPropertyGrid and wxPropertyGridManager class references handy.

    Classes:\n
      wxPropertyGrid\n
      wxPropertyGridManager\n
      wxPropertyGridEvent\n

    Header files:\n
      <b>wx/propgrid/propgrid.h:</b> Mandatory when using wxPropertyGrid.\n
      <b>wx/propgrid/advprops.h:</b> For less often used property classes.\n
      <b>wx/propgrid/manager.h:</b> Mandatory when using wxPropertyGridManager.\n
      <b>wx/propgrid/propdev.h:</b> Mandatory when implementing custom property classes.\n

    \ref featurelist\n
    \ref basics\n
    \ref categories\n
    \ref parentprops
    \ref enumandflags\n
    \ref advprops\n
    \ref operations\n
    \ref events\n
    \ref populating\n
    \ref customizing\n
    \ref custprop\n
    \ref usage2\n
    \ref subclassing\n
    \ref misc\n
    \ref proplist\n
    \ref userhelp\n
    \ref bugs\n
    \ref issues\n
    \ref todo\n
    \ref notes\n
    \ref newprops\n
    \ref neweditors\n

    \section featurelist wxPropertyGrid Features

    Following is a non-exhaustive list of various wxPropertyGrid features and classes or methods necessary
    to use them.

    - Hiding property editor: You can either use limited editing mode (wxPG_LIMITED_EDITING
      window style, wxPropertyGrid::LimitPropertyEditing), which hides wxTextCtrl based
      editor when other methods (dialog or sub-properties) are available, <b>or</b> you
      can disable the property, which effectively hides the editor and makes the property
      label to be drawn in grey colour.
    - Unspecified, empty values (wxPropertyGrid::SetPropertyUnspecified,
      wxPropertyGrid::IsPropertyUnspecified).
    - Client data (void*) (wxPropertyGrid::SetPropertyClientData,
      wxPropertyGrid::GetPropertyClientData).
    - Multi-page management (\ref usage2).
    - Setting wxValidator for editor (wxPropertyGrid::SetPropertyValidator).
    - Changing editor of property (wxPropertyGrid::SetPropertyEditor).
    - Property's value bitmap - small image shown in front of the value text
      (wxPropertyGrid::SetPropertyImage).
    - Help text shown in parent frame's status bar or in manager's description
      text box (wxPropertyGrid::SetPropertyHelpString, wxPropertyGrid::GetPropertyHelpString).
    - Changing set of choices used by wxEnumProperty, wxEditEnumProperty or wxMultiChoiceProperty
      (wxPropertyGrid::SetPropertyChoices).
    - Changing background colour of property's cell (wxPropertyGrid::SetPropertyColour).
    - Setting property text max length (wxPropertyGrid::SetPropertyMaxLength)
    - Hiding a property (wxPropertyGrid::HideProperty)
    - Setting property attributes (\ref attributes)
    - Populating wxPropertyGrid (\ref populating).

    \section basics Creating and Populating wxPropertyGrid (updated!)

    As seen here, wxPropertyGrid is constructed in the same way as
    other wxWidgets controls:

    \code

    // Necessary header file
    #include <wx/propgrid/propgrid.h>

    ...

        // Assumes code is in frame/dialog constructor

        // Construct wxPropertyGrid control
        wxPropertyGrid* pg = new wxPropertyGrid(
            this, // parent
            PGID, // id
            wxDefaultPosition, // position
            wxDefaultSize, // size
            // Some specific window styles - for all additional styles,
            // see Modules->PropertyGrid Window Styles
            wxPG_AUTO_SORT | // Automatic sorting after items added
            wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
            // Default style
            wxPG_DEFAULT_STYLE );

        // Window style flags are at premium, so some less often needed ones are
        // available as extra window styles (wxPG_EX_xxx) which must be set using
        // SetExtraStyle member function. wxPG_EX_HELP_AS_TOOLTIPS, for instance,
        // allows displaying help strings as tooltips.
        pg->SetExtraStyle( wxPG_EX_HELP_AS_TOOLTIPS );

    \endcode

      (for complete list of new window styles: @link wndflags Additional Window Styles@endlink)

      wxPropertyGrid is usually populated with lines like this:

    \code
        pg->Append( wxStringProperty(wxT("Label"),wxT("Name"),wxT("Initial Value")) );
    \endcode

    wxStringProperty is a factory function that creates a property instance of
    a property class "wxStringProperty". Only the first function argument (label)
    is mandatory. When necessary, name defaults to label and initial value to
    default value. If wxPG_LABEL is used as the name argument, then the label is
    automatically used as a name as well (this is more efficient than manually
    defining both as the same). Empty name is also allowed, but in this case the
    property cannot be accessed by its name.

    To demonstrate other common property classes, here's another code snippet:

    \code

        // Add int property
        pg->Append( wxIntProperty(wxT("IntProperty"), wxPG_LABEL, 12345678) );

        // Add float property (value type is actually double)
        pg->Append( wxFloatProperty(wxT("FloatProperty"), wxPG_LABEL, 12345.678) );

        // Add a bool property
        pg->Append( wxBoolProperty(wxT("BoolProperty"), wxPG_LABEL, false) );

        // A string property that can be edited in a separate editor dialog.
        pg->Append( wxLongStringProperty(wxT("LongStringProperty"),
                                         wxPG_LABEL,
                                         wxT("This is much longer string than the ")
                                         wxT("first one. Edit it by clicking the button.")));

        // String editor with dir selector button.
        pg->Append( wxDirProperty(wxT("DirProperty"), wxPG_LABEL, ::wxGetUserHome()) );

        // A file selector property.
        pg->Append( wxFileProperty(wxT("FileProperty"), wxPG_LABEL, wxEmptyString) );

        // Extra: set wildcard for file property (format same as in wxFileDialog).
        pg->SetPropertyAttribute(wxT("FileProperty"),
                                 wxPG_FILE_WILDCARD,
                                 wxT("All files (*.*)|*.*"));

    \endcode

      All operations on properties should be done via wxPropertyGrid's
    (or wxPropertyGridManager's) methods. Class reference of the base property
    class should only be interesting for those creating custom property classes.

      Property operations, such as SetPropertyValue or DisableProperty,
    all have two versions: one which accepts property id (of type wxPGId) and
    another that accepts property name. Id is faster since it doesn't require
    hash map lookup, but name is often much more convenient.

      You can get property id as Append/Insert return value, or by calling
    GetPropertyByName.

      Below are samples for using some of the more commong operations. See 
    wxPropertyContainerMethods and wxPropertyGrid class references for complete list.

    \code

        wxPGId MyPropertyId = pg->GetPropertyByName( wxT("MyProperty") );

        // There are many overloaded versions of this method, of which each accept
        // different type of value.
        // NOTE: If type of MyProperty is not "long", then this will yield a
        //       run-time error message.
        pg->SetPropertyValue( wxT("MyProperty"), 200 );

        // Setting a string works for all properties - conversion is done
        // automatically.
        pg->SetPropertyValue( MyPropertyId, wxT("400") );

        // Getting property value as wxVariant.
        wxVariant value = pg->GetPropertyValue( wxT("MyProperty") );

        // Getting property value as String (again, works for all typs).
        wxString value = pg->GetPropertyValueAsString( MyPropertyId );

        // Getting property value as int. Provokes a run-time error
        // if used with property which value type is not "long".
        long value = pg->GetPropertyValueAsLong( wxT("MyProperty") );

        // Set new name.
        pg->SetPropertyName( wxT("MyProperty"), wxT("X") );

        // Set new label - we need to use the new name.
        pg->SetPropertyLabel( wxT("X"), wxT("New Label") );

        // Disable the property.
        pg->DisableProperty( MyPropertyId );

        // Set property as read-only.
        pg->SetPropertyReadOnly( MyPropertyId );

    \endcode


    \section categories Categories

      wxPropertyGrid has a hierarchial property storage and display model, which
    allows property categories to hold child properties and even other
    categories. Other than that, from the programmer's point of view, categories
    can be treated exactly the same as "other" properties. For example, despite
    its name, GetPropertyByName also returns a category by name, and SetPropertyLabel
    also sets label of a category. Note however that sometimes the label of a
    property category may be referred as caption (for example, there is
    SetCaptionForegroundColour method that sets text colour of a property category's label).

      When category is added at the top (i.e. root) level of the hierarchy,
    it becomes a *current category*. This means that all other (non-category)
    properties after it are automatically added to it. You may add
    properties to specific categories by using wxPropertyGrid::Insert or wxPropertyGrid::AppendIn.

      Category code sample:

    \code

        // One way to add category (similar to how other properties are added)
        pg->Append( wxPropertyCategory(wxT("Main")) );

        // All these are added to "Main" category
        pg->Append( wxStringProperty(wxT("Name")) );
        pg->Append( wxIntProperty(wxT("Age"),wxPG_LABEL,25) );
        pg->Append( wxIntProperty(wxT("Height"),wxPG_LABEL,180) );
        pg->Append( wxIntProperty(wxT("Weight")) );

        // Another way
        pg->AppendCategory( wxT("Attributes") );

        // All these are added to "Attributes" category
        pg->Append( wxIntProperty(wxT("Intelligence")) );
        pg->Append( wxIntProperty(wxT("Agility")) );
        pg->Append( wxIntProperty(wxT("Strength")) );

    \endcode


    \section parentprops Parent Properties

      If you want to combine number of properties under single parent (just
    like wxFontProperty combines font attributes), then the easiest way to
    proceed is to use wxParentProperty.

    \remarks
    - wxParentProperty's value type is string, in which
      a child property that has children of its own will be embedded in
      braces ([]).
    - Children of wxParentProperty cannot be accessed globally by their name.
      Instead, use "Parent.Child" format.
    - However, events occur for the children, not the parent. In addition
      to GetPropertyParent, You can use wxPropertyGridEvent::GetMainParent()
      to find out property's highest wxParentProperty or wxCustomProperty.

    Sample:

    \code
        wxPGId pid = pg->Append( wxParentProperty(wxT("Car"),wxPG_LABEL) );

        pg->AppendIn( pid, wxStringProperty(wxT("Model")),
                                            wxPG_LABEL,
                                            wxT("Lamborghini Diablo SV")) );

        pg->AppendIn( pid, wxIntProperty(wxT("Engine Size (cc)"),
                                         wxPG_LABEL,
                                         5707) );

        wxPGId speedId = pg->AppendIn( pid, wxParentProperty(wxT("Speeds"),wxPG_LABEL) );
        pg->AppendIn( speedId, wxIntProperty(wxT("Max. Speed (mph)"),wxPG_LABEL,300) );
        pg->AppendIn( speedId, wxFloatProperty(wxT("0-100 mph (sec)"),wxPG_LABEL,3.9) );
        pg->AppendIn( speedId, wxFloatProperty(wxT("1/4 mile (sec)"),wxPG_LABEL,8.6) );

        pg->AppendIn( pid, wxIntProperty(wxT("Price ($)"),
                                         wxPG_LABEL,
                                         300000) );

        // Displayed value of "Car" property is now:
        // "Lamborghini Diablo SV; [300; 3.9; 8.6]; 300000"

    \endcode

    \section enumandflags wxEnumProperty and wxFlagsProperty

      wxEnumProperty is used when you want property's (integer) value
    to be selected from a popup list of choices.

      Creating wxEnumProperty is more complex than those described earlier.
    You have to provide list of constant labels, and optionally relevant values
    (if label indexes are not sufficient).

    \remarks

    - Value wxPG_INVALID_VALUE (equals 2147483647 which usually equals INT_MAX) is not
      allowed as value.

    A very simple example:

    \code

        //
        // Using wxArrayString
        //
        wxArrayString arrDiet;
        arr.Add(wxT("Herbivore"));
        arr.Add(wxT("Carnivore"));
        arr.Add(wxT("Omnivore"));

        pg->Append( wxEnumProperty(wxT("Diet"),
                                   wxPG_LABEL,
                                   arrDiet) );



        //
        // Using wxChar* array
        //
        const wxChar* arrayDiet[] =
        { wxT("Herbivore"), wxT("Carnivore"), wxT("Omnivore"), NULL };

        pg->Append( wxEnumProperty(wxT("Diet"),
                                   wxPG_LABEL,
                                   arrayDiet) );


    \endcode

    Here's extended example using values as well:

    \code

        //
        // Using wxArrayString and wxArrayInt
        //
        wxArrayString arrDiet;
        arr.Add(wxT("Herbivore"));
        arr.Add(wxT("Carnivore"));
        arr.Add(wxT("Omnivore"));

        wxArrayInt arrIds;
        arrIds.Add(40);
        arrIds.Add(45);
        arrIds.Add(50);

        // Note that the initial value (the last argument) is the actual value,
        // not index or anything like that. Thus, our value selects "Omnivore".
        pg->Append( wxEnumProperty(wxT("Diet"),
                                   wxPG_LABEL,
                                   arrDiet,
                                   arrIds,
                                   50) );


        //
        // Using wxChar* and long arrays
        //
        const wxChar* array_diet[] =
        { wxT("Herbivore"), wxT("Carnivore"), wxT("Omnivore"), NULL };

        long array_diet_ids[] =
        { 40, 45, 50 };

        pg->Append( wxEnumProperty(wxT("Diet"),
                                   wxPG_LABEL,
                                   array_diet,
                                   array_diet_ids) );

    \endcode

      wxPGChoices is a class where wxEnumProperty, and other properties which
      require label storage, actually stores strings and values. It is used
      to facilitiate reference counting, and therefore recommended way of
      adding items when multiple properties share the same set.

      You can use it directly as well, filling it and then passing it to the
      factory function.

    \code

        wxPGChoices chs;
        chs.Add(wxT("Herbivore"),40);
        chs.Add(wxT("Carnivore"),45);
        chs.Add(wxT("Omnivore"),50);

        // Note: you can add even whole arrays to wxPGChoices

        pg->Append( wxEnumProperty(wxT("Diet"),
                                   wxPG_LABEL,
                                   chs) );

        // Add same choices to another property as well - this is efficient due
        // to reference counting
        pg->Append( wxEnumProperty(wxT("Diet 2"),
                                   wxPG_LABEL,
                                   chs) );

     \endcode

    If you later need to change choices used by a property, there is function
    for that as well.

    \code

        //
        // Example 1: Add one extra item
        wxPGChoices& choices = pg->GetPropertyChoices(wxT("Diet"));
        choices.Add(wxT("Custom"),55);

        //
        // Example 2: Replace all the choices
        wxPGChoices chs;
        chs.Add(wxT("<No valid items yet>"),0);
        pg->SetPropertyChoices(wxT("Diet"),chs);

    \endcode

    If you want to create your enum properties with simple (label,name,value)
    constructor, then you need to create a new property class using one of the
    supplied macro pairs. See \ref newprops for details.

    <b>wxEditEnumProperty</b> is works exactly like wxEnumProperty, except
    is uses non-readonly combobox as default editor, and has string values.

    wxFlagsProperty is similar:

    \code

        const wxChar* flags_prop_labels[] = { wxT("wxICONIZE"),
            wxT("wxCAPTION"), wxT("wxMINIMIZE_BOX"), wxT("wxMAXIMIZE_BOX"), NULL };

        // this value array would be optional if values matched string indexes
        long flags_prop_values[] = { wxICONIZE, wxCAPTION, wxMINIMIZE_BOX,
            wxMAXIMIZE_BOX };

        pg->Append( wxFlagsProperty(wxT("Window Style"),
                                    wxPG_LABEL,
                                    flags_prop_labels,
                                    flags_prop_values,
                                    wxDEFAULT_FRAME_STYLE) );

    \endcode

    wxFlagsProperty can use wxPGChoices just the same way as wxEnumProperty
    (and also custom property classes can be created with similar macro pairs).
    <b>Note: </b> When changing "choices" (ie. flag labels) of wxFlagsProperty,
    you will need to use SetPropertyChoices - otherwise they will not get updated
    properly.

    \section advprops Advanced Properties

      This section describes the use of less often needed property classes.
    To use them, you have to include <wx/propgrid/advprops.h>.

    \code

    // Necessary extra header file
    #include <wx/propgrid/advprops.h>

    ...

        // wxArrayStringProperty embeds a wxArrayString.
        pg->Append( wxArrayStringProperty(wxT("Label of ArrayStringProperty"),
                                          wxT("NameOfArrayStringProp")));

        // Date property.
        // NB: This will use wxDatePickerCtrl only if wxPG_ALLOW_WXADV is defined
        //     in propgrid.h or in the library project settings.
        pg->Append( wxDateProperty(wxT("MyDateProperty"),
                                   wxPG_LABEL,
                                   wxDateTime::Now()) );

        // Image file property. Wildcard is auto-generated from available
        // image handlers, so it is not set this time.
        pg->Append( wxImageFileProperty(wxT("Label of ImageFileProperty"),
                                        wxT("NameOfImageFileProp")));

        // Font property has sub-properties. Note that we give window's font as
        // initial value.
        pg->Append( wxFontProperty(wxT("Font"),
                    wxPG_LABEL,
                    GetFont()) );

        // Colour property with arbitrary colour.
        pg->Append( wxColourProperty(wxT("My Colour 1"),
                                     wxPG_LABEL,
                                     wxColour(242,109,0) ) );

        // System colour property.
        pg->Append( wxSystemColourProperty (wxT("My SysColour 1"),
                                            wxPG_LABEL,
                                            wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) );

        // System colour property with custom colour.
        pg->Append( wxSystemColourProperty (wxT("My SysColour 2"),
                                            wxPG_LABEL,
                                            wxColour(0,200,160) ) );

        // Cursor property
        pg->Append( wxCursorProperty (wxT("My Cursor"),
                                      wxPG_LABEL,
                                      wxCURSOR_ARROW));

    \endcode


    \section operations More About Operating with Properties

    Example of iterating through all properties (that are not category captions or
    sub-property items):

    \code

        wxPGId id = pg->GetFirstProperty();

        while ( id.IsOk() )
        {
            // Do something with property id

            ...

            // Get next
            pg->GetNextProperty( id );
        }

	\endcode

    Getting value of selected wxSystemColourProperty (which value type is derived
    from wxObject):

    \code

        wxPGId id = pg->GetSelection();

        if ( id.IsOk() )
        {

            // Get name of property
            const wxString& name = pg->GetPropertyName( id );

            // If type is not correct, GetColour() method will produce run-time error
            if ( pg->IsPropertyValueType( id, CLASSINFO(wxColourPropertyValue) ) )
            {
                wxColourPropertyValue* pcolval =
                    wxDynamicCast(pg->GetPropertyValueAsWxObjectPtr(id),
                                  wxColourPropertyValue);

                // Report value
                wxString text;
                if ( pcolval->m_type == wxPG_CUSTOM_COLOUR )
                    text.Printf( wxT("It is custom colour: (%i,%i,%i)"),
                        (int)pcolval->m_colour.Red(),
                        (int)pcolval->m_colour.Green(),
                        (int)pcolval->m_colour.Blue());
                else
                    text.Printf( wxT("It is wx system colour (number=%i): (%i,%i,%i)"),
                        (int)pcolval->m_type,
                        (int)pcolval->m_colour.Red(),
                        (int)pcolval->m_colour.Green(),
                        (int)pcolval->m_colour.Blue());

                wxMessageBox( text );
            }

        }


    \endcode

    \section populating Populating wxPropertyGrid Automatically

    \subsection fromvariants Populating from List of wxVariants

    Example of populating an empty wxPropertyGrid from a values stored
    in an arbitrary list of wxVariants.

    \code

        // This is a static method that initializes *all* builtin type handlers
        // available, including those for wxColour and wxFont. Refers to *all*
        // included properties, so when compiling with static library, this
        // method may increase the executable size significantly.
        pg->InitAllTypeHandlers ();

        // Get contents of the grid as a wxVariant list
        wxVariant all_values = pg->GetPropertyValues();

        // Populate the list with values. If a property with appropriate
        // name is not found, it is created according to the type of variant.
        pg->SetPropertyValues ( my_list_variant );

        // In order to get wxObject ptr from a variant value,
        // wxGetVariantCast(VARIANT,CLASSNAME) macro has to be called.
        // Like this:
        wxVariant v_txcol = pg->GetPropertyValue(wxT("Text Colour"));
        const wxColour& txcol = wxGetVariantCast(v_txcol,wxColour);

	\endcode


    \section events Event Handling

    Probably the most important event is the Changed event which occurs when
    value of any property is changed by the user. Use EVT_PG_CHANGED(id,func)
    in your event table to use it.
    For complete list of event types, see wxPropertyGrid class reference.

    The custom event class, wxPropertyGridEvent, has methods to directly
    access the property that triggered the event.

    Here's a small sample:

    \code

    // Portion of an imaginary event table
    BEGIN_EVENT_TABLE(MyForm, wxFrame)

        ...

        // This occurs when a property value changes
        EVT_PG_CHANGED( PGID, MyForm::OnPropertyGridChange )

        ...

    END_EVENT_TABLE()

    void MyForm::OnPropertyGridChange( wxPropertyGridEvent& event )
    {

        // Get name of changed property
        const wxString& name = event.GetPropertyName();

        // Get resulting value - wxVariant is convenient here.
        wxVariant value = event.GetPropertyValue();

    }

    \endcode

    \remarks On Sub-property Event Handling
    - For wxParentProperty and wxCustomProperty, events will occur for
      sub-property. For those properties that inherit directly from
      wxPGPropertyWithChildren/wxBaseParentPropertyClass (wxFontProperty,
      wxFlagsProperty, etc), events occur for the main parent property
      only (actually, this has to do whether the children are "private" or not
      - see the attributes).
    - When wxParentProperty or wxCustomProperty's child gets changed, you can
      use wxPropertyGridEvent::GetMainParent to obtain its top non-category
      parent (useful, if you have wxParentProperty as child of another
      wxParentProperty, for example).


    \subsection tofile Saving Population to a Text-based Storage

    \code

    static void WritePropertiesToMyStorage( wxPropertyGrid* pg, wxPGId id, wxMyStorage& f, int depth )
    {
        wxString s;
        wxString s2;

        while ( id.IsOk() )
        {

            // TODO: Save property into text storage using:
            //   wxPropertyGrid::GetPropertyClassName
            //   wxPropertyGrid::GetPropertyName
            //   wxPropertyGrid::GetPropertyLabel
            //   wxPropertyGrid::GetPropertyValueAsString
            //   wxPropertyGrid::GetPropertyChoices
            //   wxPropertyGrid::GetPropertyAttributes

            // Example for adding choices:
            wxPGChoices& choices = pg->GetPropertyChoices(id);
            if ( choices.IsOk() )
            {
                // First add id of the choices list inorder to optimize
                s2.Printf(wxT("\"%X\""),(unsigned int)choices.GetId());
                s.Append(s2);
                f.AddToken(s2);

                size_t i;
                wxArrayString& labels = choices.GetLabels();
                wxArrayInt& values = choices.GetValues();
                if ( values.GetCount() )
                    for ( i=0; i<labels.GetCount(); i++ )
                    {
                        s2.Printf(wxT("\"%s||%i\""),labels[i].c_str(),values[i]);
                        f.AddToken(s2);
                    }
                else
                    for ( i=0; i<labels.GetCount(); i++ )
                    {
                        s2.Printf(wxT("\"%s\""),labels[i].c_str());
                        f.AddToken(s2);
                    }
            }

            // Write children, if any
            wxPGId firstChild = pg->GetFirstChild(id);
            if ( firstChild.IsOk() )
            {
                WritePropertiesToMyStorage( pg, firstChild, f, depth+1 );

                // TODO: Add parent's terminator
            }

            id = pg->GetNextSibling(id);
        }
    }

    ...

        // Then you can use this to store the entire hierarchy
        wxPGId firstChild = pg->GetFirstChild(pg->GetRoot());
        if ( firstChild.IsOk() )
            WritePropertiesToFile(pg,first_child,InstanceOfMyStorage,0);

    \endcode

    For more practical'ish example, see FormMain::OnSaveToFileClick in
    propgridsample.cpp.

    \subsection fromfile Loading Population from a Text-based Storage

    \code

        // Recommended when modifying the grid a lot at once
        pg->Freeze();

        // Necessary if you want a full-page loading
        pg->Clear();

        wxPropertyGridPopulator populator(pg);

        // Store strings from the source here
        wxString s_class;
        wxString s_name;
        wxString s_value;
        wxString s_attr;

        // Each set of choices loaded must have id
        wxPGChoicesId choicesId;

        wxArrayString choiceLabels;
        wxArrayInt choiceValues;

        // Pseudo-code loop to parse the source one "line" at a time
        while ( !source.IsAtEnd() )
        {

            // Clear stuff that doesn't exist at every "line"
            choicesId = (wxPGChoicesId) 0;
            choiceLabels.Empty();
            choiceValues.Empty();

            // TODO: Load "line" to variables

            // TODO: When a sequence of sibling properties is terminated, call this:
            //   populator.EndChildren();

            // TODO: If had choices, use following code:
            //    if ( choicesId && !populator.HasChoices(choicesId) )
            //    {
            //        populator.AddChoices(choicesId,choiceLabels,choiceValues);
            //    }

            // TODO: Add the property.
            //   (for sake of simplicity we use here default name for properties)
            //    populator.AppendByClass(s_class,
            //                            s_name,
            //                            wxPG_LABEL,
            //                            s_value,
            //                            s_attr,
            //                            choicesId);

            // TODO: When a sequence of sibling properties begins, call this:
            //   populator.BeginChildren();

        }

        pg->Thaw();

    \endcode

    For more practical'ish example, see FormMain::OnLoadFromFileClick in
    propgridsample.cpp.


    \section customizing Customizing Properties (without sub-classing)

    In this section are presented various ways to have custom appearance
    and behaviour for your properties without all the necessary hassle
    of sub-classing a property class etc.

    \subsection customimage Setting Value Image

    Every property can have a small value image placed in front of the
    actual value text. Built-in example of this can be seen with
    wxColourProperty and wxImageFileProperty, but for others it can
    be set using wxPropertyGrid::SetPropertyImage method.

    \subsection customvalidator Setting Validator

    You can set wxValidator for a property using wxPropertyGrid::SetPropertyValidator.

    \subsection customeditor Setting Property's Editor Control(s)

    You can set editor control (or controls, in case of a control and button),
    of any property using wxPropertyGrid::SetPropertyEditor. Editors are passed
    using wxPG_EDITOR(EditorName) macro, and valid built-in EditorNames are
    TextCtrl, Choice, ComboBox, CheckBox, TextCtrlAndButton, ChoiceAndButton,
    SpinCtrl, and DatePickerCtrl. Two last mentioned ones require call to
    static member function wxPropertyGrid::RegisterAdditionalEditors().

    Following example changes wxColourProperty's editor from default Choice
    to TextCtrlAndButton. wxColourProperty has its internal event handling set
    up so that button click events of the button will be used to trigger
    colour selection dialog.

    \code

        wxPGId colProp = pg->Append(wxColourProperty(wxT("Text Colour")));

        pg->SetPropertyEditor(colProp,wxPG_EDITOR(TextCtrlAndButton));

    \endcode

    Naturally, creating and setting custom editor classes is a possibility as
    well. For more information, see wxPGEditor class reference.

    \subsection customeventhandling Handling Events Passed from Properties

    <b>wxEVT_COMMAND_BUTTON_CLICKED </b>(corresponds to event table macro EVT_BUTTON):
    Occurs when editor button click is not handled by the property itself
    (as is the case, for example, if you set property's editor to TextCtrlAndButton
    from the original TextCtrl).

    \subsection attributes Property Attributes

    Miscellaneous values, often specific to a property type, can be set
    using wxPropertyGrid::SetPropertyAttribute and wxPropertyGrid::SetPropertyAttributeAll
    methods.

    For complete list of attributes, see @link attrids Property Attributes@endlink.

    \subsection boolcheckbox Setting wxBoolProperties to Use Check Box

    To have all wxBoolProperties to use CheckBox editor instead of Choice, use
    following (call after bool properties have been added):

    \code
        pg->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX,(long)1);
    \endcode



    \section custprop wxCustomProperty

    wxCustomProperty allows extra customizing.

    - May have children.

    For more info on attributes, see \ref attrids. In sample application,
    there is a CustomProperty property that has children that can be
    used to modify the property itself.

    \remarks
    - Children of wxParentProperty cannot be accessed globally by their name.
      Instead, use "Parent.Child" format.
    - However, events occur for the children, not the parent. In addition
      to GetPropertyParent, You can use wxPropertyGridEvent::GetMainParent()
      to find out property's highest wxParentProperty or wxCustomProperty.

    <b>Limitations:</b>
    - Currently wxCustomProperty is limited to wxString value type.
    - As in wxParentProperty: names of child properties are not visible
      globally. You need to use "Parent.SubProperty" name format to access
      them.


    \section usage2 Using wxPropertyGridManager (Updated!)

    wxPropertyGridManager is an efficient multi-page version of wxPropertyGrid,
    which can optionally have toolbar for mode and page selection, help text box,
    and a compactor button.

    wxPropertyGridManager mirrors most non-visual methods of wxPropertyGrid, some
    identically, some so that they can affect a property on any page, and some
    so that they can only affect selected target page.

    Generally, methods that operate on a property ( such as
    GetPropertyValue, SetPropertyValue, EnableProperty, LimitPropertyEditing, Delete, etc. ),
    work globally (so the given property can exist on any managed page).

    Methods that add properties ( Append, Insert, etc.) or operate on multiple properties
    (such as GetPropertyValues or SetPropertyValues), will work in target page only.
    Use SetTargetPage(index) method to set current target page. Changing a displayed page
    (using SelectPage(index), for example) will automatically set the target page
    to the one displayed.

    Global methods such as ExpandAll generally work on the target page only.

    Visual methods, such as SetCellBackgroundColour and GetNextVisible are only
    available in wxPropertyGrid. Use wxPropertyGridManager::GetGrid() to obtain
    pointer to it.

    wxPropertyGridManager constructor has exact same format as wxPropertyGrid
    constructor, and basicly accepts same extra window style flags (albeit also
    has some extra ones).

    Here's some example code for creating and populating a wxPropertyGridManager:

    \code

        wxPropertyGridManager* pgMan = new wxPropertyGridManager(this, PGID,
            wxDefaultPosition, wxDefaultSize,
            // These and other similar styles are automatically
            // passed to the embedded wxPropertyGrid.
            wxPG_BOLD_MODIFIED|wxPG_SPLITTER_AUTO_CENTER|
            // Include toolbar.
            wxPG_TOOLBAR |
            // Include description box.
            wxPG_DESCRIPTION |
            // Include compactor.
            wxPG_COMPACTOR |
            // Plus defaults.
            wxPGMAN_DEFAULT_STYLE
           );

        // Adding a page sets target page to the one added, so
        // we don't have to call SetTargetPage if we are filling
        // it right after adding.
        pgMan->AddPage(wxT("First Page"));

        pgMan->AppendCategory(wxT("Category A1"));

        // Remember, the next line equals pgman->Append( wxIntProperty(wxT("Number"),wxPG_LABEL,1) );
        pgMan->Append( wxT("Number"),wxPG_LABEL,1 );

        pgMan->Append( wxColourProperty(wxT("Colour"),wxPG_LABEL,*wxWHITE) );

        pgMan->AddPage(wxT("Second Page"));

        pgMan->Append( wxT("Text"),wxPG_LABEL,wxT("(no text)") );

        pgMan->Append( wxFontProperty(wxT("Font"),wxPG_LABEL) );

        // For total safety, finally reset the target page.
        pgMan->SetTargetPage(0);

    \endcode

    \subsection propgridpage wxPropertyGridPage (New!)

    wxPropertyGridPage is holder of properties for one page in manager. It is derived from
    wxEvtHandler, so you can subclass it to process page-specific property grid events. Hand
    over your page instance in wxPropertyGridManager::AddPage.

    Please note that the wxPropertyGridPage itself only sports subset of wxPropertyGrid API.
    Naturally it inherits from wxPropertyGridMethods and wxPropertyGridState, but, for instance, setting property values is
    not yet supported. Use parent manager (m_manager member) instead when needed. Basic property
    appending and insertion is supported, however.


    \section subclassing Subclassing wxPropertyGrid and wxPropertyGridManager (New!)

    Few things to note:

    - Only a small percentage of member functions are virtual. If you need more,
      just let me know.

    - Data manipulation is done in wxPropertyGridState class. So, instead of
      overriding wxPropertyGrid::Insert, you'll probably want to override wxPropertyGridState::DoInsert.

    - Override wxPropertyGrid::CreateState to instantiate your derivative wxPropertyGridState.
      For wxPropertyGridManager, you'll need to subclass wxPropertyGridPage instead (since it
      is derived from wxPropertyGridState), and hand over instances in wxPropertyGridManager::AddPage
      calls.

    - You can use a derivate wxPropertyGrid with manager by overriding wxPropertyGridManager::CreatePropertyGrid
      member function.


    \section misc Miscellaneous Topics (Updated!)

    \subsection namescope Property Name Scope

    - All properties which parent is category or root have their names
      globally accessible.

    - Sub-properties (i.e. properties which have parent that is not category or
      root) can not be accessed globally by their name. Instead, use
      "<property>.<subproperty>" in place of "<subproperty>".

    \subsection boolproperty wxBoolProperty

      There are few points about wxBoolProperty that require futher discussion:
      - wxBoolProperty can be shown as either normal combobox or as a checkbox.
        Property attribute wxPG_BOOL_USE_CHECKBOX is used to change this.
        For example, if you have a wxFlagsProperty, you can
        set its all items to use check box using the following:
        \code
            pg->SetPropertyAttribute(wxT("MyFlagsProperty"),wxPG_BOOL_USE_CHECKBOX,(long)1,wxPG_RECURSE);
        \endcode

      - Default item names for wxBoolProperty are [wxT("False"),wxT("True")]. This can be
        changed using wxPropertyGrid::SetBoolChoices(trueChoice,falseChoice).

    \subsection textctrlupdates Updates from wxTextCtrl Based Editor

      Changes from wxTextCtrl based property editors are committed (ie.
    wxEVT_PG_CHANGED is sent etc.) *only* when (1) user presser enter, (2)
    user moves to edit another property, or (3) when focus or mouse leaves
    the grid.

      Because of this, you may find it useful, in some apps, to monitor
    wxEVT_COMMAND_TEXT_UPDATED (EVT_TEXT macro) for non-committed changes
    in editor. However, the current problem is that pressing Esc in editor
    cancels any changes made, thus possibly rendering your knowledge of
    changed state incomplete. There is no current, perfect solution for
    this problem.

    \subsection splittercentering Centering the Splitter (New!)

      If you need to center the splitter, but only once when the program starts,
    then do <b>not</b> use the wxPG_SPLITTER_AUTO_CENTER window style, but the
    wxPropertyGrid::CenterSplitter() method. <b>However, be sure to call it after
    the sizer setup and SetSize calls!</b> (ie. usually at the end of the
    frame/dialog constructor)

    \subsection compilerdefines Supported Preprocessor Defines

    Here is list of supported preprocessor defines (other than those that relate with
    wxWidgets core library):

    <b>wxPG_USE_WXMODULE:</b> Define as 0 to not use wxModule to manage global variables.
    This may be needed in cases where wxPropertyGrid is linked as a plugin DLL, or when
    wxPropertyGrid is linked statically in a DLL.

    <b>WXMAKINGLIB_PROPGRID:</b> Define if you are linking wxPropertyGrid statically
    but wxWidgets itself is DLL.

    <b>WXMAKINGDLL_PROPGRID:</b> Define when building wxPropertyGrid as a DLL. This
    should be automatically defined correctly by the Bakefile-generated makefiles.

    <b>wxPG_COMPATIBILITY_1_0_0:</b> Define to make wxPropertyGrid more compatible with the
    old 1.0.x releases.


    \section proplist Property Type Descriptions (Updated!)

    Here are descriptions of built-in properties, with attributes
    (see wxPropertyGrid::SetPropertyAttribute) that apply to them.
    Note that not all attributes are necessarily here. For complete
    list, see @link attrids Property Attributes@endlink.

    \subsection wxPropertyCategory

    <b>Inheritable Class:</b> wxPropertyCategoryClass.

    Not an actual property per se, but a header for a group of properties.

    \subsection wxParentProperty

    Pseudo-property that can have sub-properties inserted under itself.
    Has textctrl editor that allows editing values of all sub-properties
    in a one string. In essence, it is a category that has look and feel
    of a property, and which children can be edited via the textctrl.

    \subsection wxStringProperty

    <b>Inheritable Class:</b> wxStringProperty

    Simple string property. wxPG_STRING_PASSWORD attribute may be used
    to echo value as asterisks and use wxTE_PASSWORD for wxTextCtrl.

    \subsection wxIntProperty

    Like wxStringProperty, but converts text to a signed long integer.

    \subsection wxUIntProperty

    Like wxIntProperty, but displays value as unsigned int. To set
    the prefix used globally, manipulate wxPG_UINT_PREFIX string attribute.
    To set the globally used base, manipulate wxPG_UINT_BASE int
    attribute. Regardless of current prefix, understands (hex) values starting
    with both "0x" and "$".

    \subsection wxFloatProperty

    Like wxStringProperty, but converts text to a double-precision floating point.
    Default float-to-text precision is 6 decimals, but this can be changed
    by modifying wxPG_FLOAT_PRECISION attribute.

    \subsection wxBoolProperty

    Represents a boolean value. wxChoice is used as editor control, by the
    default. wxPG_BOOL_USE_CHECKBOX attribute can be set to 1 inorder to use
    check box instead.

    \subsection wxLongStringProperty

    <b>Inheritable Class:</b> wxLongStringPropertyClass

    Like wxStringProperty, but has a button that triggers a small text editor
    dialog.

    \subsection wxDirProperty

    Like wxLongStringProperty, but the button triggers dir selector instead.

    \subsection wxFileProperty

    <b>Inheritable Class:</b> wxFilePropertyClass

    Like wxLongStringProperty, but the button triggers file selector instead.
    Default wildcard is "All files..." but this can be changed by setting
    wxPG_FILE_WILDCARD attribute (see wxFileDialog for format details).
    Attribute wxPG_FILE_SHOW_FULL_PATH can be set to 0 inorder to show
    only the filename, not the entire path.

    \subsection wxEnumProperty

    <b>Inheritable Class:</b> wxEnumPropertyClass

    Represents a single selection from a list of choices -
    custom combobox control is used to edit the value.

    \subsection wxFlagsProperty

    <b>Inheritable Class:</b> wxFlagsPropertyClass

    Represents a bit set that fits in a long integer. wxBoolProperty sub-properties
    are created for editing individual bits. Textctrl is created to manually edit
    the flags as a text; a continous sequence of spaces, commas and semicolons
    is considered as a flag id separator.
    <b>Note: </b> When changing "choices" (ie. flag labels) of wxFlagsProperty, you
    will need to use SetPropertyChoices - otherwise they will not get updated properly.

    \subsection wxArrayStringProperty

    <b>Inheritable Class:</b> wxArrayStringPropertyClass

    Allows editing of a list of strings in wxTextCtrl and in a separate dialog.

    \subsection wxDateProperty

    <b>Inheritable Class:</b> wxDatePropertyClass

    wxDateTime property. Default editor is DatePickerCtrl, altough TextCtrl
    should work as well. wxPG_DATE_FORMAT attribute can be used to change
    string wxDateTime::Format uses (altough default is recommended as it is
    locale-dependant), and wxPG_DATE_PICKER_STYLE allows changing window
    style given to DatePickerCtrl (default is wxDP_DEFAULT|wxDP_SHOWCENTURY).

    <b>
    Note that DatePickerCtrl editor depends on wxAdv library, and will only
    be used if wxPG_ALLOW_WXADV is defined in propgrid.h or in the library
    project settings.
    </b>

    \subsection wxEditEnumProperty

    Represents a string that can be freely edited or selected from list of choices -
    custom combobox control is used to edit the value.

    \subsection wxMultiChoiceProperty

    <b>Inheritable Class:</b> wxMultiChoicePropertyClass

    Allows editing a multiple selection from a list of strings. This is
    property is pretty much built around concept of wxMultiChoiceDialog.

    \subsection wxImageFileProperty

    <b>Inheritable Class:</b> wxImageFilePropertyClass

    Like wxFileProperty, but has thumbnail of the image in front of
    the filename and autogenerates wildcard from available image handlers.

    \subsection wxColourProperty

    <b>Inheritable Class:</b> None - instead, see \ref custcolprop.

    <b>Useful alternate editor:</b> Choice.

    Represents wxColour. wxButton is used to trigger a colour picker dialog.

    \subsection wxFontProperty

    <b>Inheritable Class:</b> wxFontPropertyClass

    Represents wxFont. Various sub-properties are used to edit individual
    subvalues.

    \subsection wxSystemColourProperty

    <b>Inheritable Class:</b> wxSystemColourPropertyClass

    Represents wxColour and a system colour index. wxChoice is used to edit
    the value. Drop-down list has color images.

    \subsection wxCursorProperty

    <b>Inheritable Class:</b> wxCursorPropertyPropertyClass

    Represents a wxCursor. wxChoice is used to edit the value.
    Drop-down list has cursor images under some (wxMSW) platforms.

    \subsection wxCustomProperty

    <b>Inheritable Class:</b> wxCustomPropertyClass

    A customizable property class with string data type. Value image, Editor class,
    and children can be modified.

    \subsection Additional Sample Properties

    Sample application has following additional examples of custom properties:
    - wxFontDataProperty ( edits wxFontData )
    - wxPointProperty ( edits wxPoint )
    - wxSizeProperty ( edits wxSize )
    - wxAdvImageFileProperty ( like wxImageFileProperty, but also has a drop-down for recent image selection)
    - wxDirsProperty ( edits a wxArrayString consisting of directory strings)
    - wxArrayDoubleProperty ( edits wxArrayDouble )

    \section userhelp Using wxPropertyGrid (Updated!)

    This is a short summary of how a wxPropertyGrid is used (not how it is programmed),
    or, rather, how it <b>should</b> behave in practice.

    - Basic mouse usage is as follows:\n
      - Clicking property label selects it.
      - Clicking property value selects it and focuses to editor control.
      - Clicking category label selects the category.
      - Double-clicking category label selects the category and expands/collapses it.
      - Double-clicking labels of a property with children expands/collapses it.

    - Keyboard usage is as follows:\n
      - alt + down (or right) - displays editor dialog (if any) for a property. Note
        that this shortcut can be changed using wxPropertyGrid::SetButtonShortcut.\n
      Only when editor control is not focused:\n
      - cursor up - moves to previous visible property\n
      - cursor down - moves to next visible property\n
      - cursor left - if collapsible, collapses, otherwise moves to previous property\n
      - cursor right - if expandable, expands, otherwise moves to next property\n
      - tab (if enabled) - focuses keyboard to the editor control of selected property\n
      Only when editor control is focused:\n
      - return/enter - confirms changes made to a wxTextCtrl based editor\n
      - tab - moves to next visible property (or, if in last one, moves out of grid)\n
      - shift-tab - moves to previous visible property (or, if in first one, moves out of grid)\n
      - escape - unfocuses from editor control and cancels any changes made
        (latter for wxTextCtrl based editor only)\n

    - In long strings tabs are represented by "\t" and line break by "\n".

    \section bugs Known Bugs
    NOTE! This section is severely out of date. TODO section in propgrid.cpp has a lot
          more of these.
    Any insight on these is more than welcome.
    - wxGTK: Pressing ESC to unfocus an editor control will screw the focusing
      (either focuses back to the editor or moves focus to limbo; works perfectly
      on wxMSW though).
    - I have experienced a complete freeze when toying with the popup of
      wxAdvImageProperty. Visiting another window will end it.
    - wxGTK: in sample application, property text colour is not set correct from
      wxSystemColourProperty. Value is correct, but the resulting text colour
      is sometimes a bit skewed if it is not a shade of gray. This may be GTK's
      attempt to auto-adjust it.
    - wxGTK: Sometimes '...' text on editor button may disappear (maybe
      "invisible font" related thingie). wxAdvImageProperty never seem to get
      the text, while other controls do.
    Following are wxWidgets or platform bugs:
    - wxMSW: After (real) popup window is shown in wxPropertyGrid, grid's scrollbar
      won't get hilight until another window is visited. This is probably a Windows
      bug.
    - wxGTK+ w/ unicode: image handlers return only the first character of an extension
    - wxGTK+ 1.2: Controls sometimes disappear. They reappear when cursor is moved
      over them.
    - wxGTK+ 1.2: Cannot set (multiple) items for wxListBox (affects
      wxMultiChoiceProperty).

    \section issues Issues
    These are not bugs per se, but miscellaneous cross-platform issues that have been
    resolved in a less-than-satisfactory manner.
    - wxGTK: When selecting wxCursorProperty in sample, there may be
      warning: Invalid UTF8 string passed to pango_layout_set_text().
      This is probably specific to older versions of GTK.
    - Win2K: Pressing Alt+non-registered key combo resulted in app hanging when
      wxTAB_TRAVERSAL was used directly. Current solution is not to use it, but
      to use wxWANTS_CHARS alone. Strangely enough, this works on wxMSW (but
      not wxGTK - precompiler conditional are used to sort things out).
    - wxMSW: Toolbar's ToggleTool doesn't disable last item in the same radiogroup. AFAIK,
      there is no way to do that (though I didn't do extensive research).
    - Atleast with wxGTK2+Unicode+Debug Mode File Selector dialog may trigger an assertion
      failure (line 1060 in string.cpp with 2.5.3) that can be cancelled
      probably without any ill-effect.
    - Under GTK, EVT_MOTION does not trigger for child control. Causes cursor change
      inconsistencies. Permanent mouse capture is not practical since it causes wxWindow::
      SetCursor to fail (and events cannot be relayed to native controls anyway).
      Easy solution used: Splitter drag detect margin and control do not overlap.
    - When splitter is being dragged, active editor control (button as well under wxGTK)
      is hidden to prevent flickering. This may go unnoticed with some
      controls (wxTextCtrl) but is very noticeable with others (wxChoice).
    - Under MSW, when resizing, editor controls flicker. No easy fix here
      (wxEVT_ONIDLE might be employed).
    - Under GTK 1.2, font may show invisible if it is not bold (so it is forced).
    - Under wxGTK, controls may flicker a bit (actually, a lot) when being shown.

    \section todo Todo
    For a detailed TODO, see propertygrid.cpp (just search for "todo" and you'll find it).

    \section notes Design Notes
    - Currently wxPropertyGridManager uses "easy" way to relay events from embedded
      wxPropertyGrid. That is, the exact same id is used for both.

    - wxHashMap used to access properties by name uses 'const wxChar*' instead of 'wxString'.
      Altough this has somewhat lower performance if used mostly with wxStrings, it is much
      faster if a lot of non-wxString strings are used, since they don't have to be
      recreated as wxString before using with the hashmap.
      If you want to change this behaviour, see propertygrid.h. Comment current version
      (including wxPGNameStr), and uncomment version that uses wxString.
      Note that with unicode, wxString is always used (due to some weird issues).

    - If wxPG_DOUBLE_BUFFER is 1 (default for MSW, GTK and MAC), wxPropertyGrid::OnDrawItems
      composes the image on a wxMemoryDC. This effectively eliminates flicker caused by drawing
      itself (property editor controls are another matter).

    - Under wxMSW, flicker freedom when creating native editor controls is achieved by using
      following undocumented scheme:
      \code
        wxControl* ctrl = new wxControl();
      #ifdef __WXMSW__
        ctrl->Hide();
      #endif
        ctrl->Create(parent,id,...);

        ...further initialize, move, resize, etc...

      #ifdef __WXMSW__
        ctrl->Show();
      #endif
      \endcode

    \section crossplatform Crossplatform Notes (not necessarily wxPropertyGrid specific)

    - GTK1: When showing a dialog you may encounter invisible font!
      Solution: Set parent's font using SetOwnFont instead of SetFont.

    - GTK: Your own control can overdraw wxGTK wxWindow border!

    - wxWindow::SetSizeHints may be necessary to shrink controls below certain treshold,
      but only on some platforms. For example wxMSW might allow any shrinking without
      SetSizeHints call where wxGTK might not.

    - GTK Choice (atleast, maybe other controls as well) likes its items set
      in constructor. Appending them seems to be slower (Freeze+Thaw won't help).
      Even using Append that gets wxArrayString argument may not be good, since it
      may just append every string one at a time.


    \section newprops Creating New Properties (Updated!)

    Easiest solution for creating an arbitrary property is to subclass an existing,
    inheritable property that has the desired value type and editor. Property class
    to derive from is always property name + Class, for instance wxStringPropertyClass in
    case of wxStringProperty. You need to include header file wx/propgrid/propdev.h,
    specify a mandatory constructor, and override some virtual member functions (see
    wxPGProperty and wxPGPropertyWithChildren).

    For instance:

    \code

    #include <wx/propgrid/propdev.h>

    // wxLongStringProperty has wxString as value type and TextCtrlAndButton as editor.
    class MyStringPropertyClass : public wxLongStringPropertyClass
    {
    public:

        // Normal property constructor.
        MyStringPropertyClass(const wxString& name,
                              const wxString& label = wxPG_LABEL,
                              const wxString& value = wxEmptyString)
            : wxLongStringPropertyClass(name,label,value)
        {
        }

        // Do something special when button is clicked.
        virtual bool OnButtonClick(wxPropertyGrid* propGrid,
                                   wxWindow* primaryCtrl)
        {
            // Update value in case last minute changes were made.
            PrepareValueForDialogEditing(propGrid);

            // TODO: Create dialog (m_value has current string, if needed)

            int res = dlg.ShowModal();
            if ( res == wxID_OK && dlg.IsModified() )
            {
                DoSetValue(dlg.GetString());
                UpdateControl(primaryCtrl);
                return true;
            }

            return false;
        }

    protected:
    };

    \endcode

    You can then create a property instance with new keyword (as factory function
    is absent since macros are not used), for instance:

    \code

        pg->Append( new MyStringPropertyClass(name,label,value) );

    \endcode

    If you want to change editor used, use code like below (continues our sample above).

    Note that built-in editors include: TextCtrl, Choice, ComboBox, TextCtrlAndButton,
    ChoiceAndButton, CheckBox, SpinCtrl, and DatePickerCtrl.

    \code

        // In class body:
        virtual const wxPGEditor* DoGetEditorClass() const
        {
            return wxPG_EDITOR(TextCtrl);
        }

    \endcode

    If you want to change value type used, use code like below.

    However, first a word on value types: They are essentially wxPGValueType instances holding
    reimplemented member functions for handling specific type of data. Use wxPG_VALUETYPE(ValueType)
    to get pointer, altought this should usually not be necessary outside GetValueType. Lightweight
    wxPGVariant is used to convey value to (DoSetValue) and from (DoGetValue) property. For common
    and small types such as long and bool, the entire value is stored in wxPGVariant. For large
    types (even double!), only a pointer is stored. This is sufficient since <b>property instance
    is responsible for storing its value</b>.

    Built-in value types include: wxString, long, bool, double, void,
    wxArrayString. advprops.h also has: wxFont, wxColour, wxArrayInt. See below
    for more information about implementing your own value types.

    \code

        // In class body:
    public:

        // Minimal constructor must set the new value.
        MyPropertyClass(const wxString& name,
                        const wxString& label = wxPG_LABEL,
                        UsedDataType value = DefaultValue)
            : wxInheritedPropertyClass(name,label,OtherDefaultValue)
        {
            m_value2 = value;
        }

        virtual const wxPGValueType* GetValueType() const
        {
            return wxPG_VALUETYPE(UsedDataType);
        }

        virtual void DoSetValue(wxPGVariant value)
        {
            // TODO: Retrieve value from wxPGVariant. For simple types,
            //       you can use:
            //
            //       UsedDataType val = value.GetFoo(); // Like GetString, or GetLong, similar to wxVariant
            //
            //       For complex types, use:
            //
            //       UsedDataType* pVal = (UsedDataType*) wxPGVariantToVoidPtr(value);
            //
            //         - OR -
            //
            //       UsedDataType* pVal = wxPGVariantToWxObjectPtr(value,UsedDataType);
            //       wxASSERT(ptr);  // Since its NULL if type-checking failed
            //

            // TODO: Store value to m_value2;
        }

        virtual wxPGVariant DoGetValue() const
        {
            // TODO: Return value as wxPGVariant.
            //
            // For simple types, return the entire value. For example:
            //
            //      return wxPGVariant(m_value2);
            //
            // For complex types, return pointer. For example:
            //
            //      return wxPGVariant((void*)&m_value2);
        }

        virtual wxString GetValueAsString(int argFlags) const
        {
            // TODO: If (argFlags & wxPG_FULL_VALUE), then return storable
            //       (to config, database, etc) string. Otherwise return
            //       shown string.
        }

        virtual bool SetValueFromString(const wxString& text,
                                                       int WXUNUSED(argFlags))
        {
            // TODO: Set value from given string (which is same as previously
            //       returned from GetValueAsString(wxPG_FULL_VALUE)). Return
            //       true if value was actually changed.
        }

    protected:
        UsedDataType     m_value2;

    \endcode

    If you want to add support for the internal RTTI scheme, use code like this:

    \code

        // In private portion of class body:
        WX_PG_DECLARE_CLASSINFO()

        // In source file:
        WX_PG_IMPLEMENT_CLASSINFO(MyStringProperty,wxLongStringProperty)
        wxPG_GETCLASSNAME_IMPLEMENTATION(MyStringProperty)

    \endcode


    \remarks
    - For practical examples of arbitrary properties, please take a look
      at the sample properties in contrib/samples/propgrid/sampleprops.cpp.
    - Read wxPGProperty and wxPGPropertyWithChildren class documentation to
      find out what each virtual member function should do.
    - Value for property is usually stored in a member named m_value.
    - Documentation below may be helpful (altough you'd probably do better
      by looking at the sample properties first).

    \subsection methoda Macro Pairs

    These are quick methods for creating customized properties.

    \subsubsection custstringprop String Property with Button

    This custom property will be exactly the same as wxLongStringProperty,
    except that you can specify a custom code to handle what happens
    when the button is pressed.

      In header:
        \code
        WX_PG_DECLARE_STRING_PROPERTY(PROPNAME)
        \endcode

        In source:
        \code

        #include <wx/propgrid/propdev.h>

        // FLAGS can be wxPG_NO_ESCAPE if escape sequences shall not be expanded.
        WX_PG_IMPLEMENT_STRING_PROPERTY(PROPNAME, FLAGS)

        bool PROPNAMEClass::OnButtonClick( wxPropertyGrid* propgrid, wxString& value )
        {
            //
            // TODO: Show dialog, read initial string from value. If changed,
            //   store new string to value and return TRUE.
            //
        }
        \endcode

    FLAGS is either wxPG_NO_ESCAPE (newlines and tabs are not translated to and
    from escape sequences) or wxPG_ESCAPE (newlines and tabs are transformed
    into C-string escapes).

    There is also WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR variant which
    also allows setting up a validator for the property. Like this:

        \code

        #include <wx/propgrid/propdev.h>

        WX_PG_IMPLEMENT_STRING_PROPERTY_WITH_VALIDATOR(PROPNAME, FLAGS)

        bool PROPNAMEClass::OnButtonClick( wxPropertyGrid* propgrid, wxString& value )
        {
            //
            // TODO: Show dialog, read initial string from value. If changed,
            //   store new string to value and return TRUE.
            //
        }

        wxValidator* PROPNAMEClass::DoGetValidator() const
        {
            //
            // TODO: Return pointer to a new wxValidator instance. In most situations,
            //   code like this should work well:
            //
            //    WX_PG_DOGETVALIDATOR_ENTRY()
            //
            //    wxMyValidator* validator = new wxMyValidator(...);
            //
            //    ... prepare validator...
            //
            //    WX_PG_DOGETVALIDATOR_EXIT(validator)
            //
            //  Macros are used to maintain only one actual validator instance
            //  (ie. on a second call, function exits within the first macro).
            //
            //  For real examples, search props.cpp for ::DoGetValidator, it should
            //  have several.
            //
        }
        \endcode

    \subsubsection custflagsprop Custom Flags Property
    Flags property with custom default value and built-in labels/values.

    In header:
        \code
        WX_PG_DECLARE_CUSTOM_FLAGS_PROPERTY(PROPNAME)
        \endcode

    In source:
        \code

        #include <wx/propgrid/propdev.h>

        // LABELS are VALUES are as in the arguments to wxFlagsProperty
        // constructor. DEFVAL is the new default value (normally it is 0).
        WX_PG_IMPLEMENT_CUSTOM_FLAGS_PROPERTY(PROPNAME,LABELS,VALUES,DEFAULT_FLAGS)
        \endcode

    The new property class will have simple (label,name,value) constructor.

    \subsubsection custenumprop Custom EnumProperty

    Exactly the same as custom FlagsProperty. Simply replace FLAGS with ENUM in
    macro names to create wxEnumProperty based class instead.

    \subsubsection custarraystringprop Custom ArrayString property

    This type of custom property allows selecting different string delimiter
    (default is '"' on both sides of the string - as in C code), and allows
    adding custom button into the editor dialog.

    In header:
        \code
        WX_PG_DECLARE_ARRAYSTRING_PROPERTY(wxMyArrayStringProperty)
        \endcode

    In source:

        \code

        #include <wx/propgrid/propdev.h>

        // second argument = string delimiter. '"' for C string style (default),
        //    and anything else for str1<delimiter> str2<delimiter> str3 style
        //    (so for example, using ';' would result to str1; str2; str3).
        // third argument = const wxChar* text for the custom button. If NULL
        //    then no button is added.
        WX_PG_IMPLEMENT_ARRAYSTRING_PROPERTY(wxMyArrayStringProperty,',',wxT("Browse"))

        bool wxMyArrayStringPropertyClass::OnCustomStringEdit (wxWindow* parent,
                                                               wxString& value)
        {
            //
            // TODO: Show custom editor dialog, read initial string from value.
            //   If changed, store new string to value and return TRUE.
            //
        }

        \endcode

    \subsubsection custcolprop Custom ColourProperty

    wxColourProperty/wxSystemColourProperty that can have custom list of colours
    in dropdown.

    Use version that doesn't have _USES_WXCOLOUR in macro names to have
    wxColourPropertyValue as value type instead of plain wxColour (in this case
    values array might also make sense).

    In header:
        \code
        #include <wx/propgrid/advprops.h>
        WX_PG_DECLARE_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(wxMyColourProperty)
        \endcode

    In source:

        \code

        #include <wx/propgrid/propdev.h>

        // Colour labels. Last (before NULL, if any) must be Custom.
        static const wxChar* mycolprop_labels[] = {
            wxT("Black"),
            wxT("Blue"),
            wxT("Brown"),
            wxT("Custom"),
            (const wxChar*) NULL
        };

        // Relevant colour values as unsigned longs.
        static unsigned long mycolprop_colours[] = {
            wxPG_COLOUR(0,0,0),
            wxPG_COLOUR(0,0,255),
            wxPG_COLOUR(166,124,81),
            wxPG_COLOUR(0,0,0)
        };

        // Implement property class. Third argument is optional values array,
        // but in this example we are only interested in creating a shortcut
        // for user to access the colour values.
        WX_PG_IMPLEMENT_CUSTOM_COLOUR_PROPERTY_USES_WXCOLOUR(wxMyColourProperty,
                                                             mycolprop_labels,
                                                             (long*)NULL,
                                                             mycolprop_colours)
        \endcode

    \subsection declaring Declaring an Arbitrary Property

    To make your property available globally, you need to declare it in a
    header file. Usually you would want to use WX_PG_DECLARE_PROPERTY
    macro to do that (it is defined in propgrid.h). It has three arguments: PROPNAME,
    T_AS_ARG and DEFVAL. PROPNAME is property NAME (eg. wxStringProperty),
    T_AS_ARG is type input in function argument list (eg. "int" for int value type,
    "const wxString&" for wxString value type, etc.), and DEFVAL is default value for that.

    For example:

    \code

        // Declare wxRealPoint Property in the header
        WX_PG_DECLARE_PROPERTY(wxRealPointProperty,const wxRealPoint&,wxRealPoint(0.0,0.0))

    \endcode

    There is also WX_PG_DECLARE_PROPERTY_WITH_DECL which takes an additional declaration
    argument (export, for example, when exporting from a dll).

    If you want that your property could be inherited from, then you would also
    have to define the class body in the header file. In most cases this is probably not
    necessary and the class can be defined and implemented completely in the source.
    In the case of wxPropertyGrid library, most property classes are defined in
    propdev.h to allow them to be inherited from, but .

    \subsection implementing Implementing a Property

    First there is class body with WX_PG_DECLARE_PROPERTY_CLASS macro,
    constructor, virtual destructor, and declarations for other overridden
    methods. Then comes WX_PG_IMPLEMENT_PROPERTY_CLASS macro, and
    after that class method implementations.

    \subsection Tips

    - To get property's parent grid, call GetGrid().

    \subsection valuetypes Creating New Value Types

    If you want to a property to use a value type that is not among the
    builtin types, then you need to create a new property value type. It is
    quite straightforward, using two macros.

    In header, use WX_PG_DECLARE_VALUE_TYPE(DATATYPE), like this:

    \code
        // Example: Declare value type for wxRealPoint.
        WX_PG_DECLARE_VALUE_TYPE(wxRealPoint)
    \endcode

    If, however, class of your value type does not inherit from
    wxObject, and you need to use it in wxVariant list used as a
    persistent storage (for example, see wxPropertyGrid::GetPropertyValues),
    then use this instead, as it also declares a necessary wxVariantData_DATATYPE
    class.

    \code
        // Example: Declare value type and wxVariantData class for wxRealPoint.
        WX_PG_DECLARE_VALUE_TYPE_VOIDP(wxRealPoint)
    \endcode

    There are also _WITH_DECL versions of both.

    However, there are a few different implement macros to place in
    a source file. Pick one according to the type of type.

    \code

        // For implementing value type for a wxObject based class.
        WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ(TYPE,DEFPROPERTY,DEFVAL)

        // Same as above, except that an instance of TYPE is
        // stored in class. Thus, DEFVAL can be any expression
        // that can be assigned to the type.
        WX_PG_IMPLEMENT_VALUE_TYPE_WXOBJ_OWNDEFAULT(TYPE,DEFPROPERTY,DEFVAL)

        // For implementing value type for a non-wxObject based class.
        // Like with ...WXOBJ_OWNDEFAULT macro above, instance of TYPE
        // is stored and DEFVAL can be any expression.
        WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_SIMPLE(TYPE,DEFPROPERTY,DEFVAL)

        // Like above, but also implement the wxVariantData class
        // declared with the second kind of value type declare macro.
        WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP(TYPE,DEFPROPERTY,DEFVAL)

        // Like above, but accepts a custom wxVariantData class.
        // You need to use WX_PG_DECLARE_VALUE_TYPE with this instead
        // of _VOIDP version.
        WX_PG_IMPLEMENT_VALUE_TYPE_VOIDP_CVD(TYPE,DEFPROPERTY,DEFVAL,VARIANTDATACLASS)

        // For implementing value type with different default value.
        // NOTE: With this type you need to use wxPG_INIT_REQUIRED_TYPE2
        //   instead of wxPG_INIT_REQUIRED_TYPE.
        WX_PG_IMPLEMENT_DERIVED_TYPE(TYPENAME,PARENTVT,DEFVAL)

        // For implementing value type for a POD (plain 'ol data) value.
        // Generally should not be used since it is meant for
        // wxString, int, double etc. which are already implemented.
        WX_PG_IMPLEMENT_VALUE_TYPE(TYPE,DEFPROPERTY,TYPESTRING,GETTER,DEFVAL)

    \endcode

    Argument descriptions:

    TYPE - Actual data type represented by the value type, or if
      derived type, any custom name.

    DEFPROPERY - Name of the property that will edit this
      value type by default.

    DEFVAL - Default value for the property.

    TYPENAME - An arbitraty typename for this value type. Applies
      only to the derived type.

    PARENTVT - Name of parent value type, from which this derived
      type inherits from.

    \remarks
    - Your class, which you create value type for, must have a
      copy constructor.

    \section neweditors Creating Custom Property Editor

    - See the sources of built-in editors in contrib/src/propgrid/propgrid.cpp
      (search for wxPGTextCtrlEditor).

    - For additional information, see wxPGEditor class reference

    \subsection wxpythoneditors In wxPython

    - See README-propgrid-wxPython.txt

*/

#endif // __WX_PG_DOX_MAINPAGE_H__

