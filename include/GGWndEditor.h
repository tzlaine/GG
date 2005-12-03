// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003 T. Zachary Laine

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
   whatwasthataddress@hotmail.com */

/* $Header$ */

/** \file GGWndEditor.h
    Contains the GGWndEditor class, a window that contains controls that can alter the properties of a Wnd
    interactively. */

#ifndef _GGWndEditor_h_
#define _GGWndEditor_h_

#include "GGApp.h"
#include "GGButton.h"
#include "GGDropDownList.h"
#include "GGEdit.h"

#include <boost/type_traits.hpp>

#include <iostream>

namespace GG {

namespace detail {
    GG_API extern const int ATTRIBUTE_ROW_HEIGHT;
    GG_API extern const int ATTRIBUTE_ROW_CONTROL_WIDTH;
}

/** Allows Wnds to be edited texually in a GUI, primarily for use in GG Sketch.  WndEditor takes an assigned Wnd and
    queries it for controls that can be used to edit it.  Each Wnd to be edited calls methods in the WndEditor that
    create the appropriate controls, and provides references to its internal members that may be altered by the controls
    created.*/
class GG_API WndEditor : public Wnd
{
public:
    /** basic ctor.  Note that WndEditor has an integral width. */
    WndEditor(int h, const std::string& font_filename, int pts);

    /** basic ctor.  Note that WndEditor has an integral width. */
    WndEditor(int h, const boost::shared_ptr<Font>& font);

    virtual void Render ();

    /** sets the edited window to \a wnd, and updates the contents of WndEditor's controls to contain the controls that
        edit the new Wnd. */
    void SetWnd(Wnd* wnd);

    /** creates a row containing just the text \a name. */
    void Label(const std::string& name);

    /** creates a row containing an edit box controlling the value of \a value. */
    template <class T>
    void Attribute(const std::string& name, T& value);

    /** creates a row containing the uneditable string representation of \a value. */
    template <class T>
    void ConstAttribute(const std::string& name, T& value);

    /** creates a row that displays the uneditable result of calling \a functor on the edited Wnd. */
    template <class T>
    void CustomDisplay(const std::string& name, const T& functor);

    /** creates a row containing an edit box controlling the value of \a value.  The legal values for \a value are
        restricted to the range [\a min, \a max].*/
    template <class T>
    void Attribute(const std::string& name, T& value, const T& min, const T& max);

    /** marks the beginning of a section of flag and flag-group rows.  Until EndFlags() is called, all Flag() and
        FlagGroup() calls will set values in \a flags. */
    void BeginFlags(Uint32& flags);

    /** creates a row representing a single bit flag in the currently-set flags variable. */
    template <class T>
    void Flag(const std::string& name, T flag);

    /** creates a row representing a group of bit flags in the currently-set flags variable.  Exactly one of the given
        flags will be enabled at one time. */
    template <class T>
    void FlagGroup(const std::string& name, T min_flag, T max_flag);

    /** marks the end of a section of flag and flag-group rows. */
    void EndFlags();

    mutable boost::signal<void (Wnd*)> WndChangedSignal; ///< emitted when the edited window has been changed

private:
    void Init();
    void AttributeChangedSlot();

    Wnd* m_wnd;
    ListBox* m_list_box;
    boost::shared_ptr<Font> m_font;
    boost::shared_ptr<Font> m_label_font;
    Uint32* m_current_flags;
};

/** the base class for the hierarchy of rows of controls used by WndEditor to accept user modifications of its edited
    Wnd. */
struct GG_API AttributeRowBase : ListBox::Row
{
    virtual void Update(); ///< refreshes the contents of the row to match the value associated with the row
    mutable boost::signal<void ()> ChangedSignal; ///< emitted when the row has modified its associated value
};

/** the most general form of attribute row, which displays an editable value in an edit box. */
template <class T>
struct AttributeRow : AttributeRowBase
{
    AttributeRow(const std::string& name, T& value, const boost::shared_ptr<Font>& font);
private:
    void TextChanged(const std::string& value_text);
    T& m_value;
    Edit* m_edit;
};

/** the specialization of AttributeRow<T> for Pt. */
template <>
struct GG_API AttributeRow<Pt> : AttributeRowBase
{
    AttributeRow(const std::string& name, Pt& value, const boost::shared_ptr<Font>& font);
private:
    Pt& m_value;
    Edit* m_x_edit;
    Edit* m_y_edit;
};

/** the specialization of AttributeRow<T> for Clr. */
template <>
struct GG_API AttributeRow<Clr> : AttributeRowBase
{
    AttributeRow(const std::string& name, Clr& value, const boost::shared_ptr<Font>& font);
private:
    Clr& m_value;
    Edit* m_red_edit;
    Edit* m_green_edit;
    Edit* m_blue_edit;
    Edit* m_alpha_edit;
};

/** the specialization of AttributeRow<T> for bool. */
template <>
struct GG_API AttributeRow<bool> : AttributeRowBase
{
    AttributeRow(const std::string& name, bool& value, const boost::shared_ptr<Font>& font);
private:
    void SelectionChanged(int selection);
    bool& m_value;
    DropDownList* m_bool_drop_list;
};

/** the specialization of AttributeRow<T> for Font shared pointers. */
template <>
struct GG_API AttributeRow<boost::shared_ptr<Font> > : AttributeRowBase
{
    AttributeRow(const std::string& name, boost::shared_ptr<Font>& value, const boost::shared_ptr<Font>& font);
private:
    void FilenameChanged(const std::string& filename_text);
    void PointsChanged(const std::string& points_text);
    boost::shared_ptr<Font>& m_value;
    Edit* m_filename_edit;
    Edit* m_points_edit;
};

/** a AttributeRowBase subclass that is restricted to a certain range of values.  Note that all this type of row should
    be used for all enum types, since the range of valid enum values must be known for the control to display them
    all. */
template <class T, bool is_enum = boost::is_enum<T>::value>
struct RangedAttributeRow : AttributeRowBase
{
    RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font);
private:
    void TextChanged(const std::string& value_text);
    T& m_value;
    T m_min;
    T m_max;
    Edit* m_edit;
};

/** the specialization of RangedAttributeRow<T, bool> for enum types. */
template <class T>
struct RangedAttributeRow<T, true> : AttributeRowBase
{
    RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font);
private:
    void SelectionChanged(int selection);
    T& m_value;
    T m_min;
    DropDownList* m_enum_drop_list;
};

/** an uneditable attribute row. */
template <class T>
struct ConstAttributeRow : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const T& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
private:
    const T& m_value;
    TextControl* m_value_text;
};

/** the specialization of ConstAttributeRow<T> for Pt. */
template <>
struct GG_API ConstAttributeRow<Pt> : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const Pt& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
private:
    const Pt& m_value;
    TextControl* m_value_text;
};

/** the specialization of ConstAttributeRow<Clr> for Pt. */
template <>
struct GG_API ConstAttributeRow<Clr> : AttributeRowBase
{
    ConstAttributeRow(const std::string& name, const Clr& value, const boost::shared_ptr<Font>& font);
    virtual void Update();
private:
    const Clr& m_value;
    TextControl* m_value_text;
};

/** the subclass of AttributeRowBase used to represent a single flag attribute. */
template <class T>
struct FlagAttributeRow : AttributeRowBase
{
    /** basic ctor.  \a flags should be the variable that holds all the flag values, and \a value should be the flag
        represented by this row. */
    FlagAttributeRow(const std::string& name, Uint32& flags, const T& value, const boost::shared_ptr<Font>& font);
private:
    void CheckChanged(bool checked);
    Uint32& m_flags;
    T m_value;
    StateButton* m_check_box;
};

/** the AttributeRowBase subclass used to represent a group of mutually-exclusive flag attributes, one of which must be
    set to true at all times. */
template <class T>
struct FlagGroupAttributeRow : AttributeRowBase
{
    /** basic ctor.  \a flags should be the variable that holds all the flag values, and \a min and \a max should define
        the range of flags represented by this row. */
    FlagGroupAttributeRow(const std::string& name, Uint32& flags, const T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font);
private:
    void SelectionChanged(int selection);
    Uint32& m_flags;
    T m_value;
    std::vector<T> m_group_values;
    DropDownList* m_flag_drop_list;
};

/** the AttributeRowBase subclass used to display some custom text about a Wnd, that does not necessarily correspond to
    a single data member in that Wnd.  CustomDisplayRow accepts a functor with the signature std::string (const Wnd*);
    when the row's Update() method is called, the row will set its text to <i>functor</i>(<i>m_wnd</i>).  This allows a
    Wnd subclass to display arbitrary (uneditable) information about itself, without being restricted to displaying just
    data members. */
template <class T>
struct CustomDisplayRow : AttributeRowBase
{
    CustomDisplayRow(const std::string& name, const T& functor, const Wnd*& wnd, const boost::shared_ptr<Font>& font);
    virtual void Update();
private:
    T m_functor;
    const Wnd*& m_wnd;
    TextControl* m_display_text;
};


// template implementations
template <class T>
void WndEditor::Attribute(const std::string& name, T& value)
{
    AttributeRowBase* attribute = new AttributeRow<T>(name, value, m_font);
    m_list_box->Insert(attribute);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::ConstAttribute(const std::string& name, T& value)
{
    AttributeRowBase* attribute = new ConstAttributeRow<T>(name, value, m_font);
    m_list_box->Insert(attribute);
}

template <class T>
void WndEditor::CustomDisplay(const std::string& name, const T& functor)
{
    AttributeRowBase* display_row = new CustomDisplayRow<T>(name, functor, const_cast<const GG::Wnd*&>(m_wnd), m_font);
    m_list_box->Insert(display_row);
}

template <class T>
void WndEditor::Attribute(const std::string& name, T& value, const T& min, const T& max)
{
    AttributeRowBase* attribute = new RangedAttributeRow<T>(name, value, min, max, m_font);
    m_list_box->Insert(attribute);
    Connect(attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::Flag(const std::string& name, T flag)
{
    if (!m_current_flags) {
        throw std::runtime_error("WndEditor::Flag() : Attempted to create a flag outside of a BeginFlags()/EndFlags() "
                                 "block.");
    }
    AttributeRowBase* flag_attribute = new FlagAttributeRow<T>(name, *m_current_flags, flag, m_font);
    m_list_box->Insert(flag_attribute);
    Connect(flag_attribute->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
void WndEditor::FlagGroup(const std::string& name, T min_flag, T max_flag)
{
    if (!m_current_flags) {
        throw std::runtime_error("WndEditor::FlagGroup() : Attempted to create a flag group outside of a BeginFlags()/"
                                 "EndFlags() block.");
    }
    T value = min_flag;
    while (value <= max_flag && !(*m_current_flags & value)) {
        value = T(value * 2);
    }
    if (max_flag < value) {
        throw std::runtime_error("WndEditor::FlagGroup() : Attempted to initialize a flag group from a set of flags "
                                 "that contains no flags in the group.");
    }
    AttributeRowBase* flag_group = new FlagGroupAttributeRow<T>(name, *m_current_flags, value, min_flag, max_flag, m_font);
    m_list_box->Insert(flag_group);
    Connect(flag_group->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

template <class T>
AttributeRow<T>::AttributeRow(const std::string& name, T& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_edit = new Edit(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    push_back(m_edit);
    *m_edit << value;
    Connect(m_edit->FocusUpdateSignal, &AttributeRow::TextChanged, this);
}

template <class T>
void AttributeRow<T>::TextChanged(const std::string& value_text)
{
    try {
        T value = boost::lexical_cast<T>(value_text);
        m_value = value;
        m_edit->SetTextColor(CLR_BLACK);
        ChangedSignal();
    } catch (const boost::bad_lexical_cast& e) {
        m_edit->SetTextColor(CLR_RED);
    }
}

template <class T, bool is_enum>
RangedAttributeRow<T, is_enum>::RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_min(min),
    m_max(max),
    m_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_edit = new Edit(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    push_back(m_edit);
    *m_edit << value;
    Connect(m_edit->FocusUpdateSignal, &RangedAttributeRow::TextChanged, this);
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
        ChangedSignal();
    } catch (const boost::bad_lexical_cast& e) {
        m_edit->SetTextColor(CLR_RED);
    }
}

template <class T>
RangedAttributeRow<T, true>::RangedAttributeRow(const std::string& name, T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_min(min),
    m_enum_drop_list(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_enum_drop_list = new DropDownList(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, detail::ATTRIBUTE_ROW_HEIGHT * (max - min + 1) + 4, CLR_GRAY, CLR_WHITE);
    m_enum_drop_list->SetStyle(LB_NOSORT);
    for (T i = min; i <= max; i = T(i + 1)) {
        Row* row = new ListBox::Row();
        std::string enum_label = boost::lexical_cast<std::string>(i);
        unsigned int pos = enum_label.find_last_of(':');
        if (pos != std::string::npos) {
            ++pos;
            enum_label = enum_label.substr(pos);
        }
        row->push_back(CreateControl(enum_label, font, CLR_BLACK));
        m_enum_drop_list->Insert(row);
    }
    push_back(m_enum_drop_list);
    m_enum_drop_list->Select(m_value - m_min);
    Connect(m_enum_drop_list->SelChangedSignal, &RangedAttributeRow::SelectionChanged, this);
}

template <class T>
void RangedAttributeRow<T, true>::SelectionChanged(int selection)
{
    m_value = T(m_min + selection);
    ChangedSignal();
}

template <class T>
ConstAttributeRow<T>::ConstAttributeRow(const std::string& name, const T& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_value_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_value_text = new TextControl(0, 0, boost::lexical_cast<std::string>(m_value), font, CLR_BLACK);
    push_back(m_value_text);
}

template <class T>
void ConstAttributeRow<T>::Update()
{
    m_value_text->SetText(boost::lexical_cast<std::string>(m_value));
}

template <class T>
FlagAttributeRow<T>::FlagAttributeRow(const std::string& name, Uint32& flags, const T& value, const boost::shared_ptr<Font>& font) :
    m_flags(flags),
    m_value(value),
    m_check_box(0)
{
    boost::shared_ptr<Font> font_to_use = App::GetApp()->GetFont(font->FontName(), font->PointSize() + 2);
    push_back(CreateControl(name, font, CLR_BLACK));
    m_check_box = new StateButton(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, "", font_to_use, GG::TF_LEFT, GG::CLR_GRAY);
    m_check_box->SetCheck(m_flags & m_value);
    push_back(m_check_box);
    Connect(m_check_box->CheckedSignal, &FlagAttributeRow::CheckChanged, this);
}

template <class T>
void FlagAttributeRow<T>::CheckChanged(bool checked)
{
    if (checked)
        m_flags |= m_value;
    else
        m_flags &= ~m_value;
    ChangedSignal();
}

template <class T>
FlagGroupAttributeRow<T>::FlagGroupAttributeRow(const std::string& name, Uint32& flags, const T& value, const T& min, const T& max, const boost::shared_ptr<Font>& font) :
    m_flags(flags),
    m_value(value),
    m_group_values(),
    m_flag_drop_list(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    for (T i = min; i <= max; i = T(i * 2)) {
        m_group_values.push_back(T(i));
    }
    m_flag_drop_list = new DropDownList(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, detail::ATTRIBUTE_ROW_HEIGHT * static_cast<int>(m_group_values.size()) + 4, CLR_GRAY, CLR_WHITE);
    m_flag_drop_list->SetStyle(LB_NOSORT);
    for (unsigned int i = 0; i < m_group_values.size(); ++i) {
        Row* row = new ListBox::Row();
        row->push_back(CreateControl(boost::lexical_cast<std::string>(m_group_values[i]), font, CLR_BLACK));
        m_flag_drop_list->Insert(row);
    }
    push_back(m_flag_drop_list);
    unsigned int initial_index = 0;
    for (; initial_index < m_group_values.size(); ++initial_index) {
        if (m_group_values[initial_index] == value)
            break;
    }
    if (initial_index == m_group_values.size()) {
        throw std::runtime_error("FlagGroupAttributeRow::FlagGroupAttributeRow() : Attempted to initialize a "
                                 "flag group's drop-down list with a value that is not a power-of-two in the "
                                 "range (min, max).");
    }
    m_flag_drop_list->Select(initial_index);
    Connect(m_flag_drop_list->SelChangedSignal, &FlagGroupAttributeRow::SelectionChanged, this);
}

template <class T>
void FlagGroupAttributeRow<T>::SelectionChanged(int selection)
{
    m_flags &= ~m_value;
    m_value = m_group_values[selection];
    m_flags |= m_value;
    ChangedSignal();
}

template <class T>
CustomDisplayRow<T>::CustomDisplayRow(const std::string& name, const T& functor, const Wnd*& wnd, const boost::shared_ptr<Font>& font) :
    m_functor(functor),
    m_wnd(wnd),
    m_display_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_display_text = new TextControl(0, 0, m_functor(m_wnd), font, CLR_BLACK);
    push_back(m_display_text);
}

template <class T>
void CustomDisplayRow<T>::Update()
{
    m_display_text->SetText(m_functor(m_wnd));
}

} // namespace GG

#endif // _GGWndEditor_h_
