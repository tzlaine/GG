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

/** \file GGSpin.h
    Contains the Spin class template, which provides a spin-box control that allows the user to select a value from a 
    range an arbitrary type (int, double, an enum, etc.). */

#ifndef _GGSpin_h_
#define _GGSpin_h_

#ifndef _GGApp_h_
#include <GGApp.h>
#endif

#ifndef _GGButton_h_
#include <GGButton.h>
#endif

#ifndef _GGDrawUtil_h_
#include <GGDrawUtil.h>
#endif

#ifndef _GGEdit_h_
#include <GGEdit.h>
#endif

#ifndef _GGWndEditor_h_
#include <GGWndEditor.h>
#endif

#include <cmath>
#include <limits>


namespace GG {

// forward declaration of spin_details::mod and spin_details::div
namespace spin_details {
    template <class T> T mod(T, T);
    template <class T> T div(T, T);
}


/** the regions of the a SpinBox that a click may fall within; used to catch clicks on the contained Buttons */
enum SpinRegion {
    SR_NONE,
    SR_UP_BN,
    SR_DN_BN
};

/** a spin box control.  This control class is templated so that arbitrary data types can be used with Spin.  All the
    built-in numeric types are supported by the code here.  If you want to use some other type, such as an enum type,
    you need to define operator+(), operator-(), and template specializations of spin_details::mod() and
    spin_details::div().  Spin controls are optionally directly editable by the user.  When the user inputs a value that
    is not valid for the Spin's parameters (not on a step boundary, or outside the allowed range), the input gets locked
    to the nearest valid value.  The user is responsible for selecting a min, max, and step size that make sense.  For
    instance, min = 0, max = 4, step = 3 may produce odd results if the user increments all the way to the top, then
    back down, to produce the sequence 0, 3, 4, 1, 0.  To avoid this, choose the values so that (max - min) mod step ==
    0.  It is possible to provide custom buttons for a Spin to use; if you choose to add custom buttons, make sure they
    look alright at arbitrary sizes, and note that Spin buttons are always H wide by H/2 tall, where H is the height of
    the Spin, less the thickness of the Spin's border.  Note that if you want Spin controls to be automatically
    serialized in your application, you need to export each instantiation of Spin<> yourself (e.g. Spin<int> or
    Spin<double>).  See the boost serialization documentation for details. */
template <class T>
class Spin : public Control
{
public:
    using Wnd::SizeMove;

    /** \name Signal Types */ //@{
    typedef typename boost::signal<void (T)> ValueChangedSignalType;  ///< emitted whenever the value of the Spin has changed
    //@}

    /** \name Slot Types */ //@{
    typedef typename ValueChangedSignalType::slot_type ValueChangedSlotType;   ///< type of functor(s) invoked on a ValueChangedSignalType
    //@}

    /** \name Structors */ //@{
    /** ctor*/
    Spin(int x, int y, int w, int h, T value, T step, T min, T max, bool edits, const boost::shared_ptr<Font>& font, Clr color, 
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Button* up = 0, Button* down = 0, 
         Uint32 flags = CLICKABLE | DRAG_KEEPER);

    /** ctor*/
    Spin(int x, int y, int w, int h, T value, T step, T min, T max, bool edits, const std::string& font_filename, int pts, Clr color, 
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Button* up = 0, Button* down = 0, 
         Uint32 flags = CLICKABLE | DRAG_KEEPER);

    /** ctor that does not required height. Height is determined from the font and point size used.*/
    Spin(int x, int y, int w, T value, T step, T min, T max, bool edits, const boost::shared_ptr<Font>& font, Clr color, 
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Button* up = 0, Button* down = 0, 
         Uint32 flags = CLICKABLE | DRAG_KEEPER);

    /** ctor that does not required height. Height is determined from the font and point size used.*/
    Spin(int x, int y, int w, T value, T step, T min, T max, bool edits, const std::string& font_filename, int pts, Clr color, 
         Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Button* up = 0, Button* down = 0, 
         Uint32 flags = CLICKABLE | DRAG_KEEPER);

    ~Spin(); // dtor
    //@}

    /** \name Accessors */ //@{
    T     Value() const;              ///< returns the current value of the control's text
    T     StepSize() const;           ///< returns the step size of the control
    T     MinValue() const;           ///< returns the minimum value of the control
    T     MaxValue() const;           ///< returns the maximum value of the control
    bool  EditsAllowed() const;       ///< returns true if the spinbox can have its value typed in directly

    Clr   TextColor() const;          ///< returns the text color
    Clr   InteriorColor() const;      ///< returns the the interior color of the control
    Clr   HiliteColor() const;        ///< returns the color used to render hiliting around selected text
    Clr   SelectedTextColor() const;  ///< returns the color used to render selected text

    mutable ValueChangedSignalType ValueChangedSignal; ///< the value changed signal object for this DynamicGraphic
    //@}

    /** \name Mutators */ //@{
    virtual bool Render();
    virtual void LButtonDown(const Pt& pt, Uint32 keys);
    virtual void LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual void LButtonUp(const Pt& pt, Uint32 keys);
    virtual void LClick(const Pt& pt, Uint32 keys);
    virtual void MouseHere(const Pt& pt, Uint32 keys);
    virtual void MouseLeave(const Pt& pt, Uint32 keys);
    virtual void Keypress(Key key, Uint32 key_mods);

    virtual void SizeMove(int x1, int y1, int x2, int y2);

    virtual void Disable(bool b = true);

    void Incr();  ///< increments the value of the control's text by StepSize(), up to at most MaxValue()
    void Decr();  ///< decrements the value of the control's text by StepSize(), down to at least MinValue()

    /** sets the value of the control's text to \a value, locked to the range [MinValue(), MaxValue()]*/
    void SetValue(T value);

    void           SetStepSize(T step);   ///< sets the step size of the control to \a step
    void           SetMinValue(T value);  ///< sets the minimum value of the control to \a value
    void           SetMaxValue(T value);  ///< sets the maximum value of the control to \a value

    /** turns on or off the mode that allows the user to edit the value in the spinbox directly. */
    void AllowEdits(bool b = true);

    virtual void   SetColor(Clr c);
    void           SetTextColor(Clr c);          ///< sets the text color
    void           SetInteriorColor(Clr c);      ///< sets the interior color of the control
    void           SetHiliteColor(Clr c);        ///< sets the color used to render hiliting around selected text
    void           SetSelectedTextColor(Clr c);  ///< sets the color used to render selected text   

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

protected:
    typedef T ValueType;

    enum {BORDER_THICK = 2, PIXEL_MARGIN = 5};

    /** \name Structors */ //@{
    Spin(); ///< default ctor
    //@}

    /** \name Accessors */ //@{
    const boost::shared_ptr<Button>&   UpButton() const; ///< returns a pointer to the Button control used as this control's up button
    const boost::shared_ptr<Button>&   DownButton() const; ///< returns a pointer to the Button control used as this control's down button

    SpinRegion  InitialDepressedRegion() const;  ///< returns the part of the control originally under cursor in LButtonDown msg
    SpinRegion  DepressedRegion() const;         ///< returns the part of the control currently being "depressed" by held-down mouse button

    boost::signals::connection EditConnection() const; ///< returns the connection to the internal edit control's FocusUpdateSignal
    //@}

    /** \name Mutators */ //@{
    Edit* GetEdit();  ///< returns a pointer to the Edit control used to render this control's text and accept keyboard input
    //@}

private:
    void ConnectSignals();
    void Init(const boost::shared_ptr<Font>& font, Clr color, Clr text_color, Clr interior, Uint32 flags);
    void ValueUpdated(const std::string& val_text);

    T        m_value;
    T        m_step_size;
    T        m_min_value;
    T        m_max_value;

    bool     m_editable;

    Edit*                     m_edit;
    boost::shared_ptr<Button> m_up_bn;
    boost::shared_ptr<Button> m_dn_bn;

    SpinRegion   m_initial_depressed_area;  ///< the part of the control originally under cursor in LButtonDown msg
    SpinRegion   m_depressed_area;          ///< the part of the control currently being "depressed" by held-down mouse button

    boost::signals::connection m_edit_connection;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
template<class T>
Spin<T>::Spin() :
    Control()
{
}

template<class T>
Spin<T>::Spin(int x, int y, int w, int h, T value, T step, T min, T max, bool edits, const boost::shared_ptr<Font>& font, Clr color, 
              Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Button* up/* = 0*/, Button* down/* = 0*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    Control(x, y, w, h),
    m_value(value),
    m_step_size(step),
    m_min_value(min),
    m_max_value(max),
    m_editable(edits),
    m_up_bn(up),
    m_dn_bn(down),
    m_initial_depressed_area(SR_NONE),
    m_depressed_area(SR_NONE)
{
    Init(font, color, text_color, interior, flags);
}

template<class T>
Spin<T>::Spin(int x, int y, int w, int h, T value, T step, T min, T max, bool edits, const std::string& font_filename, int pts, Clr color, 
              Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Button* up/* = 0*/, Button* down/* = 0*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    Control(x, y, w, h),
    m_value(value),
    m_step_size(step),
    m_min_value(min),
    m_max_value(max),
    m_editable(edits),
    m_up_bn(up),
    m_dn_bn(down),
    m_initial_depressed_area(SR_NONE),
    m_depressed_area(SR_NONE)
{
    Init(App::GetApp()->GetFont(font_filename, pts), color, text_color, interior, flags);
}

template<class T>
Spin<T>::Spin(int x, int y, int w, T value, T step, T min, T max, bool edits, const boost::shared_ptr<Font>& font, Clr color, 
              Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Button* up/* = 0*/, Button* down/* = 0*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    Control(x, y, w, font->Height() + 2 * PIXEL_MARGIN, flags),
    m_value(value),
    m_step_size(step),
    m_min_value(min),
    m_max_value(max),
    m_editable(edits),
    m_up_bn(up),
    m_dn_bn(down),
    m_initial_depressed_area(SR_NONE),
    m_depressed_area(SR_NONE)
{
    Init(font, color, text_color, interior, flags);
}

template<class T>
Spin<T>::Spin(int x, int y, int w, T value, T step, T min, T max, bool edits, const std::string& font_filename, int pts, Clr color, 
              Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Button* up/* = 0*/, Button* down/* = 0*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
    Control(x, y, w, App::GetApp()->GetFont(font_filename, pts)->Height() + 2 * PIXEL_MARGIN),
    m_value(value),
    m_step_size(step),
    m_min_value(min),
    m_max_value(max),
    m_editable(edits),
    m_up_bn(up),
    m_dn_bn(down),
    m_initial_depressed_area(SR_NONE),
    m_depressed_area(SR_NONE)
{
    Init(App::GetApp()->GetFont(font_filename, pts), color, text_color, interior, flags);
}

template<class T>
Spin<T>::~Spin()
{
    DetachChildren();
    delete m_edit;
}

template<class T>
T Spin<T>::Value() const
{
    return m_value;
}

template<class T>
T Spin<T>::StepSize() const
{
    return m_step_size;
}

template<class T>
T Spin<T>::MinValue() const
{
    return m_min_value;
}

template<class T>
T Spin<T>::MaxValue() const
{
    return m_max_value;
}

template<class T>
bool Spin<T>::EditsAllowed() const 
{
    return m_editable;
}

template<class T>
Clr Spin<T>::TextColor() const
{
    return m_edit->TextColor();
}

template<class T>
Clr Spin<T>::InteriorColor() const
{
    return m_edit->InteriorColor();
}

template<class T>
Clr Spin<T>::HiliteColor() const
{
    return m_edit->HiliteColor();
}

template<class T>
Clr Spin<T>::SelectedTextColor() const
{
    return m_edit->SelectedTextColor();
}

template<class T>
bool Spin<T>::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, BORDER_THICK);
    if (!m_editable) {
        m_edit->OffsetMove(UpperLeft());
        m_edit->Render();
        m_edit->OffsetMove(-UpperLeft());
    }
    m_up_bn->OffsetMove(UpperLeft());
    m_dn_bn->OffsetMove(UpperLeft());
    m_up_bn->Render();
    m_dn_bn->Render();
    m_up_bn->OffsetMove(-UpperLeft());
    m_dn_bn->OffsetMove(-UpperLeft());
    return true;
}

template<class T>
void Spin<T>::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        Pt ul = UpperLeft();
        if (m_up_bn->InWindow(pt - ul)) {
            m_initial_depressed_area = SR_UP_BN;
            m_depressed_area = SR_UP_BN;
            m_up_bn->SetState(Button::BN_PRESSED);
            Incr();
        } else if (m_dn_bn->InWindow(pt - ul)) {
            m_initial_depressed_area = SR_DN_BN;
            m_depressed_area = SR_DN_BN;
            m_dn_bn->SetState(Button::BN_PRESSED);
            Decr();
        } else {
            m_initial_depressed_area = SR_NONE;
            m_depressed_area = SR_NONE;
        }
    }
}

template<class T>
void Spin<T>::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled()) {
        Pt ul = UpperLeft();
        if (m_up_bn->InWindow(pt - ul)) {
            Incr();
        } else if (m_dn_bn->InWindow(pt - ul)) {
            Decr();
        }
    }
}

template<class T>
void Spin<T>::LButtonUp(const Pt& pt, Uint32 keys)
{
    m_up_bn->SetState(Button::BN_UNPRESSED);
    m_dn_bn->SetState(Button::BN_UNPRESSED);
    m_initial_depressed_area = SR_NONE;
    m_depressed_area = SR_NONE;
}

template<class T>
void Spin<T>::LClick(const Pt& pt, Uint32 keys)
{
    LButtonUp(pt, keys);
}

template<class T>
void Spin<T>::MouseHere(const Pt& pt, Uint32 keys)
{
    LButtonUp(pt, keys);
}

template<class T>
void Spin<T>::MouseLeave(const Pt& pt, Uint32 keys)
{
    m_depressed_area = SR_NONE;
}

template<class T>
void Spin<T>::Keypress(Key key, Uint32 key_mods)
{
    switch (key) {
    case GGK_HOME:
        SetValue(m_min_value);
        break;
    case GGK_END:
        SetValue(m_max_value);
        break;
    case GGK_PAGEUP:
    case GGK_UP:
    case GGK_PLUS:
    case GGK_KP_PLUS:
        Incr();
        break;
    case GGK_PAGEDOWN:
    case GGK_DOWN:
    case GGK_MINUS:
    case GGK_KP_MINUS:
        Decr();
        break;
    default:
        break;
    }
}

template<class T>
void Spin<T>::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    const int BN_X_POS = Width() - (Height() - 2 * BORDER_THICK) - BORDER_THICK;
    const int BN_WIDTH = Height() - 2 * BORDER_THICK;
    const int BNS_HEIGHT = BN_WIDTH; // height of BOTH buttons
    m_edit->SizeMove(0, 0, Width() - Height(), Height());
    m_up_bn->SizeMove(BN_X_POS, BORDER_THICK,
                      BN_X_POS + BN_WIDTH, BORDER_THICK + BNS_HEIGHT / 2);
    m_dn_bn->SizeMove(BN_X_POS, BORDER_THICK + BNS_HEIGHT / 2,
                      BN_X_POS + BN_WIDTH, BORDER_THICK + BNS_HEIGHT);
}

template<class T>
void Spin<T>::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_edit->Disable(b);
    m_up_bn->Disable(b);
    m_dn_bn->Disable(b);
}

template<class T>
void Spin<T>::Incr()
{
    SetValue(m_value + m_step_size);
}

template<class T>
void Spin<T>::Decr()
{
    SetValue(m_value - m_step_size);
}

template<class T>
void Spin<T>::SetValue(T value)
{
    T old_value = m_value;
    if (value < m_min_value) {
        m_value = m_min_value;
    } else if (m_max_value < value) {
        m_value = m_max_value;
    } else {
        // if the value supplied does not equal a valid value
        if (std::abs(spin_details::mod((value - m_min_value), m_step_size)) > std::numeric_limits<T>::epsilon()) {
            // find nearest valid value to the one supplied
            T closest_below = spin_details::div((value - m_min_value), m_step_size) * m_step_size + m_min_value;
            T closest_above = closest_below + m_step_size;
            m_value = ((value - closest_below) < (closest_above - value) ? closest_below : closest_above);
        } else {
            m_value = value;
        }
    }
    *m_edit << m_value;
    if (m_value != old_value)
        ValueChangedSignal(m_value);
}

template<class T>
void Spin<T>::SetStepSize(T step)
{
    m_step_size = step;
}

template<class T>
void Spin<T>::SetMinValue(T value)
{
    m_min_value = value;
}

template<class T>
void Spin<T>::SetMaxValue(T value)    
{
    m_max_value = value;
}

template<class T>
void Spin<T>::AllowEdits(bool b/* = true*/)
{
    DetachChildren();
    m_edit_connection.disconnect();
    if (m_editable = b) {
        AttachChild(m_edit);
        m_edit_connection = Connect(m_edit->FocusUpdateSignal, &Spin::ValueUpdated, this);
    }
}

template<class T>
void Spin<T>::SetColor(Clr c)
{
    Control::SetColor(c);
    m_up_bn->SetColor(c);
    m_dn_bn->SetColor(c);
}

template<class T>
void Spin<T>::SetTextColor(Clr c)
{
    m_edit->SetTextColor(c);
}

template<class T>
void Spin<T>::SetInteriorColor(Clr c)
{
    m_edit->SetInteriorColor(c);
}

template<class T>
void Spin<T>::SetHiliteColor(Clr c)
{
    m_edit->SetHiliteColor(c);
}

template<class T>
void Spin<T>::SetSelectedTextColor(Clr c)
{
    m_edit->SetSelectedTextColor(c);
}

template<class T>
void Spin<T>::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    if (boost::is_same<T, int>::value)
        editor->Label("Spin<int>");
    else if (boost::is_same<T, double>::value)
        editor->Label("Spin<double>");
    else
        editor->Label("Spin<T>");
    editor->Attribute("Value", m_value);
    editor->Attribute("Step Size", m_step_size);
    editor->Attribute("Min Value", m_min_value);
    editor->Attribute("Max Value", m_max_value);
    editor->Attribute("Editable", m_editable);
}

template<class T>
const boost::shared_ptr<Button>& Spin<T>::UpButton() const
{
    return m_up_bn;
}

template<class T>
const boost::shared_ptr<Button>& Spin<T>::DownButton() const
{
    return m_dn_bn;
}

template<class T>
SpinRegion Spin<T>::InitialDepressedRegion() const
{
    return m_initial_depressed_area;
}

template<class T>
SpinRegion Spin<T>::DepressedRegion() const
{
    return m_depressed_area;
}

template<class T>
boost::signals::connection Spin<T>::EditConnection() const
{
    return m_edit_connection;
}

template<class T>
Edit* Spin<T>::GetEdit()
{
    return m_edit;
}

template<class T>
void Spin<T>::ConnectSignals()
{
    if (m_editable)
        m_edit_connection = Connect(m_edit->FocusUpdateSignal, &Spin::ValueUpdated, this);
}

template<class T>
void Spin<T>::Init(const boost::shared_ptr<Font>& font, Clr color, Clr text_color, Clr interior, Uint32 flags)
{
    Control::SetColor(color);
    m_edit = new Edit(0, 0, 1, 1, boost::lexical_cast<std::string>(m_value), font, CLR_ZERO, text_color, interior);
    if (!m_up_bn)
        m_up_bn = boost::shared_ptr<Button>(new Button(0, 0, 1, 1, "+", font->FontName(), static_cast<int>(font->PointSize() * 0.75), color));
    if (!m_dn_bn)
        m_dn_bn = boost::shared_ptr<Button>(new Button(0, 0, 1, 1, "-", font->FontName(), static_cast<int>(font->PointSize() * 0.75), color));
    if (m_editable)
        AttachChild(m_edit);
    ConnectSignals();
    SizeMove(UpperLeft(), LowerRight());
}

template<class T>
void Spin<T>::ValueUpdated(const std::string& val_text)
{
    T value;
    try {
        value = boost::lexical_cast<T>(val_text);
    } catch (boost::bad_lexical_cast) {
        SetValue(m_min_value);
        return;
    }
    SetValue(value);
}

template <class T>
template <class Archive>
void Spin<T>::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_value)
        & BOOST_SERIALIZATION_NVP(m_step_size)
        & BOOST_SERIALIZATION_NVP(m_min_value)
        & BOOST_SERIALIZATION_NVP(m_max_value)
        & BOOST_SERIALIZATION_NVP(m_editable)
        & BOOST_SERIALIZATION_NVP(m_edit)
        & BOOST_SERIALIZATION_NVP(m_up_bn)
        & BOOST_SERIALIZATION_NVP(m_dn_bn);

    if (Archive::is_loading::value)
        ConnectSignals();
}

namespace spin_details {
    // provides a typesafe mod function
    template <class T> inline 
    T mod (T dividend, T divisor) {return dividend % divisor;}

    // template specializations
    template <> inline 
    float mod<float> (float dividend, float divisor) {return std::fmod(dividend, divisor);}
    template <> inline 
    double mod<double> (double dividend, double divisor) {return std::fmod(dividend, divisor);}
    template <> inline 
    long double mod<long double> (long double dividend, long double divisor) {return std::fmod(dividend, divisor);}

    // provides a typesafe div function
    template <class T> inline 
    T div (T dividend, T divisor) {return dividend / divisor;}

    // template specializations
    template <> inline 
    float div<float> (float dividend, float divisor) {return std::floor(dividend / divisor);}
    template <> inline 
    double div<double> (double dividend, double divisor) {return std::floor(dividend / divisor);}
    template <> inline 
    long double div<long double> (long double dividend, long double divisor) {return std::floor(dividend / divisor);}
} // namespace spin_details

} // namespace GG

#endif // _GGSpin_h_

