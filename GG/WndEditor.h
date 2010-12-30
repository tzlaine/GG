// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

/** \file WndEditor.h \brief Contains the WndEditor class, a window that
    contains controls that can alter the properties of a Wnd interactively. */

#ifndef _GG_WndEditor_h_
#define _GG_WndEditor_h_

#include <GG/ClrConstants.h>
#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/ListBox.h>
#include <GG/dialogs/ColorDlg.h>

#include <boost/type_traits.hpp>


namespace GG {

namespace detail {
    GG_API extern const X ATTRIBUTE_ROW_CONTROL_WIDTH;
    GG_API extern const Y ATTRIBUTE_ROW_HEIGHT;
}

struct AttributeRowBase;

/** This is the base class for functors that respond to a change in a Wnd
    attribute edited by a WndEditor.  This is needed when a change in a value
    within a Wnd must be accompanied by some action that is taken in response
    to that change. */
template <class T>
struct AttributeChangedAction
{
    virtual ~AttributeChangedAction() {}
    virtual void operator()(const T& value) {}
};

/** Allows Wnds to be edited texually in a GUI, primarily for use in GG
    Sketch.  WndEditor takes an assigned Wnd and queries it for controls that
    can be used to edit it.  Each Wnd to be edited calls methods in the
    WndEditor that create the appropriate controls, and provides references to
    its internal members that may be altered by the controls created.*/
class GG_API WndEditor : public Wnd
{
public:
    /** Contains a Flags object and the AttributeChangedAction associated with
        it. */
    template <class FlagType>
    struct FlagsAndAction
    {
        Flags<FlagType>* m_flags;
        boost::shared_ptr<AttributeChangedAction<Flags<FlagType> > > m_action;
    };

    /** basic ctor.  Note that WndEditor has an integral width. */
    WndEditor(Y h, const boost::shared_ptr<Font>& font);

    /** Returns the font used by this WndEditor to create its attribute rows. */
    const boost::shared_ptr<Font>& GetFont() const;

    /** Returns the Wnd currently being edited. */
    const Wnd* GetWnd() const;

    virtual void Render ();

    /** Sets the edited window to \a wnd, and updates the contents of
        WndEditor's controls to contain the controls that edit the new Wnd. */
    void SetWnd(Wnd* wnd, const std::string& name = "");

    /** Creates a row containing just the text \a name. */
    void Label(const std::string& name);

    /** Adds \a row and attaches to its ChangedSignal */
    void Attribute(AttributeRowBase* row);

    /** Creates a row containing an edit box controlling the value of \a
        value. */
    template <class T>
    void Attribute(const std::string& name, T& value,
                   const boost::shared_ptr<AttributeChangedAction<T> >& attribute_changed_action);

    /** Creates a row containing an edit box controlling the value of \a
        value. */
    template <class T>
    void Attribute(const std::string& name, T& value);

    /** Creates a row containing an edit box controlling the value of \a
        value.  The legal values for \a value are restricted to the range [\a
        min, \a max].*/
    template <class T>
    void Attribute(const std::string& name, T& value, const T& min, const T& max,
                   const boost::shared_ptr<AttributeChangedAction<T> >& attribute_changed_action);

    /** Creates a row containing an edit box controlling the value of \a
        value.  The legal values for \a value are restricted to the range [\a
        min, \a max].*/
    template <class T>
    void Attribute(const std::string& name, T& value, const T& min, const T& max);

    /** Creates a row containing the uneditable string representation of \a
        value. */
    template <class T>
    void ConstAttribute(const std::string& name, const T& value);

    /** Creates a row that displays the uneditable result of calling \a
        functor on the edited Wnd. */
    template <class T>
    void CustomText(const std::string& name, const T& functor);

    /** Marks the beginning of a section of flag and flag-group rows.  Until
        EndFlags() is called, all Flag() and FlagGroup() calls will set values
        in \a flags. */
    template <class FlagType>
    void BeginFlags(Flags<FlagType>& flags,
                    const boost::shared_ptr<AttributeChangedAction<Flags<FlagType> > >& attribute_changed_action);

    /** Marks the beginning of a section of flag and flag-group rows.  Until
        EndFlags() is called, all Flag() and FlagGroup() calls will set values
        in \a flags. */
    template <class FlagType>
    void BeginFlags(Flags<FlagType>& flags);

    /** Creates a row representing a single bit flag in the currently-set
        flags variable. */
    template <class FlagType>
    void Flag(const std::string& name, FlagType flag);

    /** Creates a row representing a group of bit flags in the currently-set
        flags variable.  Exactly one of the given flags will be enabled at one
        time. */
    template <class FlagType>
    void FlagGroup(const std::string& name, const std::vector<FlagType>& group_values);

    /** Marks the end of a section of flag and flag-group rows. */
    void EndFlags();

    mutable boost::signal<void (Wnd*, const std::string&)> WndNameChangedSignal; ///< emitted when the edited window's name has been changed
    mutable boost::signal<void (Wnd*)> WndChangedSignal; ///< emitted when the edited window has been changed

private:
    void Init();
    void AttributeChangedSlot();
    void NameChangedSlot(const std::string& name);

    Wnd* m_wnd;
    ListBox* m_list_box;
    boost::shared_ptr<Font> m_font;
    boost::shared_ptr<Font> m_label_font;
    boost::any m_current_flags_and_action;
};

/** The base class for the hierarchy of rows of controls used by WndEditor to
    accept user modifications of its edited Wnd. */
struct GG_API AttributeRowBase : ListBox::Row
{
    virtual void Refresh(); ///< refreshes the contents of the row to match the value associated with the row every frame
    virtual void Update();  ///< updates the contents of the row to match the value associated with the row on response to a change in the edited Wnd
    mutable boost::signal<void ()> ChangedSignal; ///< emitted when the row has modified its associated value
};

/** The most general form of attribute row, which displays an editable value
    in an edit box. */
template <class T>
struct AttributeRow : AttributeRowBase
{
    AttributeRow(const std::string& name, T& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const T&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void TextChanged(const std::string& value_text);
    T& m_value;
    Edit* m_edit;
    boost::signals::connection m_edit_connection;
};

/** The specialization of AttributeRow<T> for Pt. */
template <>
struct GG_API AttributeRow<Pt> : AttributeRowBase
{
    AttributeRow(const std::string& name, Pt& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const Pt&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    Pt& m_value;
    Edit* m_x_edit;
    Edit* m_y_edit;
    boost::signals::connection m_x_connection;
    boost::signals::connection m_y_connection;
};

/** The specialization of AttributeRow<T> for Clr. */
template <>
struct GG_API AttributeRow<Clr> : AttributeRowBase
{
    AttributeRow(const std::string& name, Clr& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const Clr&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void ColorButtonClicked();
    Clr& m_value;
    ColorDlg::ColorButton* m_color_button;
    boost::shared_ptr<Font> m_font;
};

/** The specialization of AttributeRow<T> for bool. */
template <>
struct GG_API AttributeRow<bool> : AttributeRowBase
{
    AttributeRow(const std::string& name, bool& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const bool&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void SelectionChanged(std::size_t selection);
    bool& m_value;
    RadioButtonGroup* m_radio_button_group;
    boost::signals::connection m_button_group_connection;
};

/** The specialization of AttributeRow<T> for Font shared pointers. */
template <>
struct GG_API AttributeRow<boost::shared_ptr<Font> > : AttributeRowBase
{
    AttributeRow(const std::string& name, boost::shared_ptr<Font>& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const boost::shared_ptr<Font>&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void FilenameChanged(const std::string& filename_text);
    void PointsChanged(const std::string& points_text);
    boost::shared_ptr<Font>& m_value;
    Edit* m_filename_edit;
    Edit* m_points_edit;
    boost::signals::connection m_filename_connection;
    boost::signals::connection m_points_connection;
};

/** A AttributeRowBase subclass that is restricted to a certain range of
    values.  Note that all this type of row should be used for all enum types,
    since the range of valid enum values must be known for the control to
    display them all. */
template <class T, bool is_enum = boost::is_enum<T>::value>
struct RangedAttributeRow : AttributeRowBase
{
    RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const T&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void TextChanged(const std::string& value_text);
    T& m_value;
    T m_min;
    T m_max;
    Edit* m_edit;
    boost::signals::connection m_edit_connection;
};

/** The specialization of RangedAttributeRow<T, bool> for enum types. */
template <class T>
struct RangedAttributeRow<T, true> : AttributeRowBase
{
    RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const T&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void SelectionChanged(DropDownList::iterator selection);
    T& m_value;
    T m_min;
    DropDownList* m_enum_drop_list;
};

/** An uneditable attribute row. */
template <class T>
struct ConstAttributeRow : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const T& value, const boost::shared_ptr<Font>& font);
    virtual void Refresh();
private:
    const T& m_value;
    TextControl* m_value_text;
};

/** The specialization of ConstAttributeRow<T> for Pt. */
template <>
struct GG_API ConstAttributeRow<Pt> : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const Pt& value, const boost::shared_ptr<Font>& font);
    virtual void Refresh();
private:
    const Pt& m_value;
    TextControl* m_value_text;
};

/** The specialization of ConstAttributeRow<Clr> for Pt. */
template <>
struct GG_API ConstAttributeRow<Clr> : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const Clr& value, const boost::shared_ptr<Font>& font);
    virtual void Refresh();
private:
    const Clr& m_value;
    TextControl* m_value_text;
};

/** The subclass of AttributeRowBase used to represent a single flag
    attribute. */
template <class FlagType>
struct FlagAttributeRow : AttributeRowBase
{
    /** Basic ctor.  \a flags should be the variable that holds all the flag
        values, and \a value should be the flag represented by this row. */
    FlagAttributeRow(const std::string& name, Flags<FlagType>& flags, FlagType value, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const Flags<FlagType>&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void CheckChanged(bool checked);
    Flags<FlagType>& m_flags;
    FlagType m_value;
    StateButton* m_check_box;
    boost::signals::connection m_check_box_connection;
};

/** The AttributeRowBase subclass used to represent a group of
    mutually-exclusive flag attributes, one of which must be set to true at
    all times. */
template <class FlagType>
struct FlagGroupAttributeRow : AttributeRowBase
{
    /** Basic ctor.  \a flags should be the variable that holds all the flag
        values, and \a min and \a max should define the range of flags
        represented by this row. */
    FlagGroupAttributeRow(const std::string& name, Flags<FlagType>& flags, FlagType value, const std::vector<FlagType>& group_values, const boost::shared_ptr<Font>& font);
    virtual void Update();
    mutable boost::signal<void (const Flags<FlagType>&)> ValueChangedSignal; ///< when the row has modified its associated value, this emits the new value
private:
    void SelectionChanged(DropDownList::iterator selection);
    Flags<FlagType>& m_flags;
    FlagType m_value;
    std::vector<FlagType> m_group_values;
    DropDownList* m_flag_drop_list;
};

/** The AttributeRowBase subclass used to display some custom text about a
    Wnd, that does not necessarily correspond to a single data member in that
    Wnd.  CustomTextRow accepts a functor with the signature std::string
    (const Wnd*); when the row's Refresh() method is called, the row will set
    its text to <i>functor</i>(<i>m_wnd</i>).  This allows a Wnd subclass to
    display arbitrary (uneditable) information about itself, without being
    restricted to displaying just data members. */
template <class T>
struct CustomTextRow : AttributeRowBase
{
    CustomTextRow(const std::string& name, const T& functor, const Wnd*& wnd, const boost::shared_ptr<Font>& font);
    virtual void Refresh();
private:
    T m_functor;
    const Wnd*& m_wnd;
    TextControl* m_display_text;
};


// template implementations
template <class T>
void WndEditor::Attribute(const std::string& name, T& value,
                          const boost::shared_ptr<AttributeChangedAction<T> >& attribute_changed_action)
{
    AttributeRow<T>* attribute = new AttributeRow<T>(name, value, m_font);
    m_list_box->Insert(attribute);
    if (attribute_changed_action)
        Connect(attribute->ValueChangedSignal, &AttributeChangedAction<T>::operator(), attribute_changed_action);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::Attribute(const std::string& name, T& value)
{
    AttributeRow<T>* attribute = new AttributeRow<T>(name, value, m_font);
    m_list_box->Insert(attribute);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::Attribute(const std::string& name, T& value, const T& min, const T& max,
                          const boost::shared_ptr<AttributeChangedAction<T> >& attribute_changed_action)
{
    RangedAttributeRow<T>* attribute = new RangedAttributeRow<T>(name, value, min, max, m_font);
    m_list_box->Insert(attribute);
    if (attribute_changed_action)
        Connect(attribute->ValueChangedSignal, &AttributeChangedAction<T>::operator(), attribute_changed_action);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::Attribute(const std::string& name, T& value, const T& min, const T& max)
{
    RangedAttributeRow<T>* attribute = new RangedAttributeRow<T>(name, value, min, max, m_font);
    m_list_box->Insert(attribute);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::ConstAttribute(const std::string& name, const T& value)
{
    ConstAttributeRow<T>* attribute = new ConstAttributeRow<T>(name, value, m_font);
    m_list_box->Insert(attribute);
}

template <class T>
void WndEditor::CustomText(const std::string& name, const T& functor)
{
    CustomTextRow<T>* display_row = new CustomTextRow<T>(name, functor, const_cast<const Wnd*&>(m_wnd), m_font);
    m_list_box->Insert(display_row);
}

template <class FlagType>
void WndEditor::BeginFlags(Flags<FlagType>& flags,
                           const boost::shared_ptr<AttributeChangedAction<Flags<FlagType> > >& attribute_changed_action)
{
    FlagsAndAction<FlagType> flags_and_action;
    flags_and_action.m_flags = &flags;
    flags_and_action.m_action = attribute_changed_action;
    m_current_flags_and_action = flags_and_action;
}


template <class FlagType>
void WndEditor::BeginFlags(Flags<FlagType>& flags)
{
    FlagsAndAction<FlagType> flags_and_action;
    flags_and_action.m_flags = &flags;
    m_current_flags_and_action = flags_and_action;
}

template <class FlagType>
void WndEditor::Flag(const std::string& name, FlagType flag)
{
    if (m_current_flags_and_action.empty()) {
        throw std::runtime_error("WndEditor::Flag() : Attempted to create a flag outside of a BeginFlags()/EndFlags() "
                                 "block.");
    }
    FlagsAndAction<FlagType> flags_and_action;
    try {
        flags_and_action = boost::any_cast<FlagsAndAction<FlagType> >(m_current_flags_and_action);
    } catch (const boost::bad_any_cast&) {
        throw std::runtime_error("WndEditor::Flag() : Attempted to initialize a flag group from a set of flags "
                                 "of a type that does not match the most recent call to BeginFlags().");
    }
    FlagAttributeRow<FlagType>* flag_attribute = new FlagAttributeRow<FlagType>(name, *flags_and_action.m_flags, flag, m_font);
    m_list_box->Insert(flag_attribute);
    if (flags_and_action.m_action)
        Connect(flag_attribute->ValueChangedSignal, &AttributeChangedAction<Flags<FlagType> >::operator(), flags_and_action.m_action);
    Connect(flag_attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class FlagType>
void WndEditor::FlagGroup(const std::string& name, const std::vector<FlagType>& group_values)
{
    if (m_current_flags_and_action.empty()) {
        throw std::runtime_error("WndEditor::FlagGroup() : Attempted to create a flag group outside of a BeginFlags()/"
                                 "EndFlags() block.");
    }
    FlagsAndAction<FlagType> flags_and_action;
    try {
        flags_and_action = boost::any_cast<FlagsAndAction<FlagType> >(m_current_flags_and_action);
    } catch (const boost::bad_any_cast&) {
        throw std::runtime_error("WndEditor::FlagGroup() : Attempted to initialize a flag group from a set of flags "
                                 "of a type that does not match the type of the flags given to the most recent call "
                                 "to BeginFlags().");
    }
    if (group_values.empty()) {
        throw std::runtime_error("WndEditor::FlagGroup() : Attempted to initialize a flag group from a n empty set of flags.");
    }
    bool value_found = false;
    FlagType value;
    for (std::size_t i = 0; i < group_values.size(); ++i) {
        if (*flags_and_action.m_flags & group_values[i]) {
            value = group_values[i];
            value_found = true;
            break;
        }
    }
    FlagGroupAttributeRow<FlagType>* flag_group = new FlagGroupAttributeRow<FlagType>(name, *flags_and_action.m_flags, value, group_values, m_font);
    m_list_box->Insert(flag_group);
    if (flags_and_action.m_action)
        Connect(flag_group->ValueChangedSignal, &AttributeChangedAction<Flags<FlagType> >::operator(), flags_and_action.m_action);
    Connect(flag_group->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
AttributeRow<T>::AttributeRow(const std::string& name, T& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_edit->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, m_edit->Height()));
    Resize(m_edit->Size());
    push_back(m_edit);
    *m_edit << value;
    m_edit_connection = Connect(m_edit->FocusUpdateSignal, &AttributeRow::TextChanged, this);
}

template <class T>
void AttributeRow<T>::TextChanged(const std::string& value_text)
{
    try {
        T value = boost::lexical_cast<T>(value_text);
        m_value = value;
        m_edit->SetTextColor(CLR_BLACK);
        ValueChangedSignal(m_value);
        ChangedSignal();
    } catch (const boost::bad_lexical_cast&) {
        m_edit->SetTextColor(CLR_RED);
    }
}

template <class T>
void AttributeRow<T>::Update()
{
    m_edit_connection.block();
    *m_edit << m_value;
    m_edit_connection.unblock();
}

template <class T, bool is_enum>
RangedAttributeRow<T, is_enum>::RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_min(min),
    m_max(max),
    m_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_edit->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, m_edit->Height()));
    Resize(m_edit->Size());
    push_back(m_edit);
    *m_edit << value;
    m_edit_connection = Connect(m_edit->FocusUpdateSignal, &RangedAttributeRow::TextChanged, this);
}

template <class T, bool is_enum>
void RangedAttributeRow<T, is_enum>::TextChanged(const std::string& value_text)
{
    try {
        T value = boost::lexical_cast<T>(value_text);
        if (value < m_min || m_max < value)
            throw boost::bad_lexical_cast();
        m_value = value;
        m_edit->SetTextColor(CLR_BLACK);
        ValueChangedSignal(m_value);
        ChangedSignal();
    } catch (const boost::bad_lexical_cast&) {
        m_edit->SetTextColor(CLR_RED);
    }
}

template <class T, bool is_enum>
void RangedAttributeRow<T, is_enum>::Update()
{
    m_edit_connection.block();
    *m_edit << m_value;
    m_edit_connection.unblock();
}

template <class T>
RangedAttributeRow<T, true>::RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_min(min),
    m_enum_drop_list(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_enum_drop_list = new DropDownList(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, detail::ATTRIBUTE_ROW_HEIGHT * (max - min + 1) + 4, CLR_GRAY);
    m_enum_drop_list->SetInteriorColor(CLR_WHITE);
    m_enum_drop_list->SetStyle(LIST_NOSORT);
    for (T i = min; i <= max; i = T(i + 1)) {
        Row* row = new ListBox::Row();
        std::string enum_label = boost::lexical_cast<std::string>(i);
        std::string::size_type pos = enum_label.find_last_of(':');
        if (pos != std::string::npos) {
            ++pos;
            enum_label = enum_label.substr(pos);
        }
        row->push_back(CreateControl(enum_label, font, CLR_BLACK));
        m_enum_drop_list->Insert(row);
    }
    push_back(m_enum_drop_list);
    m_enum_drop_list->Select(boost::next(m_enum_drop_list->begin(), m_value - m_min));
    Connect(m_enum_drop_list->SelChangedSignal, &RangedAttributeRow::SelectionChanged, this);
}

template <class T>
void RangedAttributeRow<T, true>::SelectionChanged(DropDownList::iterator selection)
{
    m_value = T(m_min + std::distance(m_enum_drop_list->begin(), selection));
    ValueChangedSignal(m_value);
    ChangedSignal();
}

template <class T>
void RangedAttributeRow<T, true>::Update()
{ m_enum_drop_list->Select(boost::next(m_enum_drop_list->begin(), m_value - m_min)); }

template <class T>
ConstAttributeRow<T>::ConstAttributeRow(const std::string& name, const T& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_value_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_value_text = new TextControl(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, boost::lexical_cast<std::string>(m_value), font, CLR_BLACK, FORMAT_LEFT);
    push_back(m_value_text);
}

template <class T>
void ConstAttributeRow<T>::Refresh()
{
    m_value_text->SetText(boost::lexical_cast<std::string>(m_value));
}

template <class FlagType>
FlagAttributeRow<FlagType>::FlagAttributeRow(const std::string& name, Flags<FlagType>& flags, FlagType value, const boost::shared_ptr<Font>& font) :
    m_flags(flags),
    m_value(value),
    m_check_box(0)
{
    boost::shared_ptr<Font> font_to_use = GUI::GetGUI()->GetFont(font->FontName(), font->PointSize() + 2);
    push_back(CreateControl(name, font, CLR_BLACK));
    m_check_box = new StateButton(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, "", font_to_use, FORMAT_LEFT, CLR_GRAY);
    m_check_box->SetCheck(m_flags & m_value);
    push_back(m_check_box);
    m_check_box_connection = Connect(m_check_box->CheckedSignal, &FlagAttributeRow::CheckChanged, this);
}

template <class FlagType>
void FlagAttributeRow<FlagType>::CheckChanged(bool checked)
{
    if (checked)
        m_flags |= m_value;
    else
        m_flags &= ~m_value;
    ValueChangedSignal(m_flags);
    ChangedSignal();
}

template <class FlagType>
void FlagAttributeRow<FlagType>::Update()
{
    m_check_box_connection.block();
    m_check_box->SetCheck(m_flags & m_value);
    m_check_box_connection.unblock();
}

template <class FlagType>
FlagGroupAttributeRow<FlagType>::FlagGroupAttributeRow(const std::string& name, Flags<FlagType>& flags, FlagType value, const std::vector<FlagType>& group_values, const boost::shared_ptr<Font>& font) :
    m_flags(flags),
    m_value(value),
    m_group_values(group_values),
    m_flag_drop_list(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_flag_drop_list = new DropDownList(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, font->Height() + 8, detail::ATTRIBUTE_ROW_HEIGHT * static_cast<int>(m_group_values.size()) + 4, CLR_GRAY);
    Resize(m_flag_drop_list->Size());
    m_flag_drop_list->SetInteriorColor(CLR_WHITE);
    m_flag_drop_list->SetStyle(LIST_NOSORT);
    for (std::size_t i = 0; i < m_group_values.size(); ++i) {
        Row* row = new ListBox::Row();
        row->push_back(CreateControl(boost::lexical_cast<std::string>(m_group_values[i]), font, CLR_BLACK));
        m_flag_drop_list->Insert(row);
    }
    push_back(m_flag_drop_list);
    std::size_t index = 0;
    DropDownList::iterator it = m_flag_drop_list->begin();
    for (; index < m_group_values.size(); ++index, ++it) {
        if (m_group_values[index] == value)
            break;
    }
    if (index == m_group_values.size()) {
        throw std::runtime_error("FlagGroupAttributeRow::FlagGroupAttributeRow() : Attempted to initialize a "
                                 "flag group's drop-down list with a value that is not in the given set of group values.");
    }
    m_flag_drop_list->Select(it);
    Connect(m_flag_drop_list->SelChangedSignal, &FlagGroupAttributeRow::SelectionChanged, this);
}

template <class FlagType>
void FlagGroupAttributeRow<FlagType>::SelectionChanged(DropDownList::iterator selection)
{
    m_flags &= ~m_value;
    m_value = m_group_values[std::distance(m_flag_drop_list->begin(), selection)];
    m_flags |= m_value;
    ValueChangedSignal(m_flags);
    ChangedSignal();
}

template <class FlagType>
void FlagGroupAttributeRow<FlagType>::Update()
{
    std::size_t index = 0;
    DropDownList::iterator it = m_flag_drop_list->begin();
    for (; index < m_group_values.size(); ++index, ++it) {
        if (m_group_values[index] == m_value)
            break;
    }
    m_flag_drop_list->Select(it);
}

template <class T>
CustomTextRow<T>::CustomTextRow(const std::string& name, const T& functor, const Wnd*& wnd, const boost::shared_ptr<Font>& font) :
    m_functor(functor),
    m_wnd(wnd),
    m_display_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_display_text = new TextControl(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, m_functor(m_wnd), font, CLR_BLACK, FORMAT_LEFT);
    Resize(m_display_text->Size());
    push_back(m_display_text);
}

template <class T>
void CustomTextRow<T>::Refresh()
{
    m_display_text->SetText(m_functor(m_wnd));
}

} // namespace GG

#endif // _GG_WndEditor_h_
