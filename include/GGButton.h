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

/** \file GGButton.h
    Contains the Button push-button control class; the StateButton control class, which represents check boxes and radio 
    buttons; and the RadioButtonGroup control class, which allows multiple radio buttons to be combined into a single control. */

#ifndef _GGButton_h_
#define _GGButton_h_

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

#include <boost/serialization/access.hpp>

namespace GG {

/** This is a basic button control.  Has three states: BN_UNPRESSED, BN_PRESSED, and BN_ROLLOVER.  BN_ROLLOVER is when
    the cursor "rolls over" the button, without depressing it, allowing rollover effects on the button.  To create a
    bitmap button, simply set the unpressed, pressed, and/or rollover graphics to the desired SubTextures. \see
    GG::SubTexture */
class GG_API Button : public TextControl
{
public:
    /// the states of being for a GG::Button
    enum ButtonState {
        BN_PRESSED,    ///< the button is being pressed by the user, and the cursor is over the button
        BN_UNPRESSED,  ///< the button is unpressed
        BN_ROLLOVER    ///< the button has the cursor over it, but is unpressed
    };

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ClickedSignalType; ///< emitted when the button is clicked by the user
    //@}

    /** \name Slot Types */ //@{
    typedef ClickedSignalType::slot_type ClickedSlotType; ///< type of functor(s) invoked on a ClickedSignalType
    //@}

    /** \name Structors */ //@{
    Button(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<GG::Font>& font, Clr color, Clr text_color = CLR_BLACK, Uint32 flags = CLICKABLE); ///< ctor
    Button(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, Clr color, Clr text_color = CLR_BLACK, Uint32 flags = CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ //@{
    /** returns button state \see ButtonState */
    ButtonState       State() const;

    const SubTexture& UnpressedGraphic() const; ///< returns the SubTexture to be used as the image of the button when unpressed
    const SubTexture& PressedGraphic() const;   ///< returns the SubTexture to be used as the image of the button when pressed
    const SubTexture& RolloverGraphic() const;  ///< returns the SubTexture to be used as the image of the button when it contains the cursor, but is not pressed

    mutable ClickedSignalType ClickedSignal; ///< the clicked signal object for this Button
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);
    virtual void   LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual void   LButtonUp(const Pt& pt, Uint32 keys);
    virtual void   LClick(const Pt& pt, Uint32 keys);
    virtual void   MouseHere(const Pt& pt, Uint32 keys);
    virtual void   MouseLeave(const Pt& pt, Uint32 keys);

    virtual void   SetColor(Clr c); ///< sets the control's color; does not affect the text color

    /** sets button state programmatically \see ButtonState */
    void           SetState(ButtonState state);

    void           SetUnpressedGraphic(const SubTexture& st); ///< sets the SubTexture to be used as the image of the button when unpressed
    void           SetPressedGraphic(const SubTexture& st);   ///< sets the SubTexture to be used as the image of the button when pressed
    void           SetRolloverGraphic(const SubTexture& st);  ///< sets the SubTexture to be used as the image of the button when it contains the cursor, but is not pressed
    //@}

protected:
    /** \name Structors */ //@{
    Button(); ///< default ctor
    //@}

    /** \name Mutators */ //@{
    virtual void   RenderUnpressed();   ///< draws the button unpressed.  If an unpressed graphic has been supplied, it is used.
    virtual void   RenderPressed();     ///< draws the button pressed.  If an pressed graphic has been supplied, it is used.
    virtual void   RenderRollover();    ///< draws the button rolled-over.  If an rollover graphic has been supplied, it is used.
    //@}

private:
    void           RenderDefault();     ///< this just draws the default unadorned square-and-rectangle button

    ButtonState    m_state;               ///< button is always in exactly one of the ButtonState states above

    SubTexture     m_unpressed_graphic; ///< graphic used to display button when it's unpressed
    SubTexture     m_pressed_graphic;   ///< graphic used to display button when it's depressed
    SubTexture     m_rollover_graphic;  ///< graphic used to display button when it's under the mouse and not pressed

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// define EnumMap and stream operators for Button::ButtonState
GG_ENUM_MAP_BEGIN(Button::ButtonState)
    GG_ENUM_MAP_INSERT(Button::BN_PRESSED)
    GG_ENUM_MAP_INSERT(Button::BN_UNPRESSED)
    GG_ENUM_MAP_INSERT(Button::BN_ROLLOVER)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(Button::ButtonState)
GG_ENUM_STREAM_OUT(Button::ButtonState)


/** This is a basic state button control.  This class is for checkboxes and radio buttons, etc.  The button/checkbox
    area can be provided via the bn_* contructor parameters, or it can be determined from the text height and format;
    the button height and width will be the text height, and the the button will be positioned to the left of the text
    and vertically the same as the text, unless the text is centered, in which case the button and text will be
    centered, and the button will appear above or below the text.  Whenever there is not room to place the button and
    the text in the proper orientation because the entire control's size is too small, the button and text are
    positioned in their default spots (button on left, text on right, centered vertically).  If no text format flags are
    provided, the default text orientation is TF_VCENTER | TF_LEFT.  Note that the bn_x and bn_y paramters are taken to
    be relative to the control's x and y position.*/
class GG_API StateButton : public TextControl
{
public:
    /// the built-in visual styles of state buttons
    enum StateButtonStyle {
        SBSTYLE_3D_XBOX,        ///< draws a down-beveled box with a 3D x-mark inside
        SBSTYLE_3D_CHECKBOX,    ///< draws a down-beveled box with a 3D check-mark inside
        SBSTYLE_3D_RADIO,       ///< draws a down-beveled circle with a 3D "dot" or "bubble" inside
        SBSTYLE_3D_BUTTON,      ///< draws a button that toggles bewtween popped up and pushed down
        SBSTYLE_3D_ROUND_BUTTON ///< draws a down-beveled circle with an up-beveled circle inside
    };

    /** \name Signal Types */ //@{
    typedef boost::signal<void (bool)> CheckedSignalType; ///< emitted when the StateButton is checked or unchecked; the checked/unchecked status is indicated by the bool parameter
    //@}

    /** \name Slot Types */ //@{
    typedef CheckedSignalType::slot_type CheckedSlotType; ///< type of functor(s) invoked on a CheckedSignalType
    //@}

    /** \name Structors */ //@{
    StateButton(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<GG::Font>& font, Uint32 text_fmt, 
                Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, StateButtonStyle style = SBSTYLE_3D_XBOX,
                int bn_x = -1, int bn_y = -1, int bn_w = -1, int bn_h = -1, Uint32 flags = CLICKABLE); ///< ctor
    StateButton(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, Uint32 text_fmt, 
                Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, StateButtonStyle style = SBSTYLE_3D_XBOX,
                int bn_x = -1, int bn_y = -1, int bn_w = -1, int bn_h = -1, Uint32 flags = CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ //@{
    bool             Checked() const;       ///< returns true if button is checked
    Clr              InteriorColor() const; ///< returns the interior color of the box, circle, or other enclosing shape

    /** returns the visual style of the button \see StateButtonStyle */
    StateButtonStyle Style() const;

    mutable CheckedSignalType CheckedSignal; ///< the checked signal object for this StaticButton
    //@}

    /** \name Mutators */ //@{
    void             Reset();                 ///< unchecks button
    void             SetCheck(bool b = true); ///< (un)checks button
    virtual void     SetColor(Clr c);         ///< sets the color of the button; does not affect text color
    void             SetInteriorColor(Clr c); ///< sets the interior color of the box, circle, or other enclosing shape

    /** sets the visual style of the button \see StateButtonStyle */
    void             SetStyle(StateButtonStyle bs);
    //@}

protected:
    /** \name Structors */ //@{
    StateButton(); ///< default ctor
    //@}

    /** \name Accessors */ //@{
    int ButtonX() const;  ///< returns the x coordinate of the button part of the control
    int ButtonY() const;  ///< returns the y coordinate of the button part of the control
    int ButtonWd() const; ///< returns the width of the button part of the control
    int ButtonHt() const; ///< returns the height of the button part of the control
    int TextX() const;    ///< returns the x coordinate of the text part of the control
    int TextY() const;    ///< returns the y coordinate of the text part of the control
    //@}

    /** \name Mutators */ //@{
    virtual bool     Render();
    virtual void     LClick(const Pt& pt, Uint32 keys);
    //@}

private:
    void Init(int w, int h, int pts, Clr color, int bn_x, int bn_y, int bn_w, int bn_h);

    bool              m_checked;     ///< true when this button in a checked, active state
    Clr               m_int_color;   ///< color inside border
    StateButtonStyle  m_style;       ///< style of appearance to use when rendering button

    int               m_button_x,  m_button_y;
    int               m_button_wd, m_button_ht;
    int               m_text_x,    m_text_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// define EnumMap and stream operators for Button::ButtonState
GG_ENUM_MAP_BEGIN(StateButton::StateButtonStyle)
    GG_ENUM_MAP_INSERT(StateButton::SBSTYLE_3D_XBOX)
    GG_ENUM_MAP_INSERT(StateButton::SBSTYLE_3D_CHECKBOX)
    GG_ENUM_MAP_INSERT(StateButton::SBSTYLE_3D_RADIO)
    GG_ENUM_MAP_INSERT(StateButton::SBSTYLE_3D_BUTTON)
    GG_ENUM_MAP_INSERT(StateButton::SBSTYLE_3D_ROUND_BUTTON)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(StateButton::StateButtonStyle)
GG_ENUM_STREAM_OUT(StateButton::StateButtonStyle)


/** This is a class that encapsulates multiple GG::StateButtons into a single radio-button control.  The location of the
    radio button group should be set to the desired location of the upper-left corner of the entire group.  The group
    starts out having 1x1 dimensions; as each button is added, the lower-right corner of the group moves to encompass
    it.  The upper-left corner will not move.  RadioButtonGroup emits a signal whenever its currently-checked button
    changes.  The signal indicates which button has been pressed, by passing the index of the button; the
    currently-checked button index is -1 when no button is checked.  Any StateButton-derived controls can be used in a
    RadioButtonGroup.  However, if you want to automatically serialize a RadioButtonGroup that has custom buttons, you
    must register the new types.  See the boost serialization documentation for details.*/
class GG_API RadioButtonGroup : public Control
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> ButtonChangedSignalType; ///< emitted when the currently-selected button has changed; the new selected button's index in the group is provided (this may be -1 if no button is currently selected)
    //@}

    /** \name Slot Types */ //@{
    typedef ButtonChangedSignalType::slot_type ButtonChangedSlotType; ///< type of functor(s) invoked on a ButtonChangedSignalType
    //@}

    /** \name Structors */ //@{
    RadioButtonGroup(int x, int y); ///< ctor
    //@}

    /** \name Accessors */ //@{
    /** returns the number of buttons in this control */
    int              NumButtons() const;

    /** returns the index of the currently checked button, or -1 if none are checked */
    int              CheckedButton() const;

    mutable ButtonChangedSignalType ButtonChangedSignal; ///< the button changed signal object for this RadioButtonGroup
    //@}

    /** \name Mutators */ //@{
    /** checks the idx-th button, and unchecks all others.  If there is no idx-th button, they are all unchecked, and the 
        currently-checked button index is set to -1. */
    void SetCheck(int idx);

    /** disables (with b == true) or enables (with b == false) the idx-th button, if it exists.  If the button exists,
        is being disabled, and is the one currently checked, the currently-checked button index is set to -1. */
    void DisableButton(int idx, bool b = true); 

    /** adds a button to the group. \note There is no way to remove buttons; RadioButtonGroup is meant to be a 
        simple grouping control. */
    void AddButton(StateButton* bn);
    //@}

protected:
    /** \name Structors */ //@{
    RadioButtonGroup(); ///< default ctor
    //@}

    /** \name Accessors */ //@{
    const std::vector<StateButton*>&               Buttons() const;     ///< returns the state buttons in the group
    const std::vector<boost::signals::connection>& Connections() const; ///< returns the connections to the state buttons
    //@}

    /** \name Mutators */ //@{
    virtual bool Render();
    //@}

private:
    class ButtonClickedFunctor // for catching button-click signals from the contained buttons
    {
    public:
        ButtonClickedFunctor(RadioButtonGroup* grp, int idx);
        void operator()(bool checked);
    private:
        RadioButtonGroup* m_grp;
        int m_idx;
    };

    void ConnectSignals();
    void HandleRadioClick(bool checked, int index);   ///< if a state button is clicked, this function ensures it and only it is active

    std::vector<StateButton*>               m_buttons;     ///< the state buttons in the group
    std::vector<boost::signals::connection> m_connections; ///< the connections to the state buttons; these must be disconnected when programmatically unclicking the buttons

    int  m_checked_button; ///< the index of the currently-checked button; -1 if none is clicked

    friend class ButtonClickedFunctor;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::Button::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TextControl)
        & BOOST_SERIALIZATION_NVP(m_state)
        & BOOST_SERIALIZATION_NVP(m_unpressed_graphic)
        & BOOST_SERIALIZATION_NVP(m_pressed_graphic)
        & BOOST_SERIALIZATION_NVP(m_rollover_graphic);
}

template <class Archive>
void GG::StateButton::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TextControl)
        & BOOST_SERIALIZATION_NVP(m_checked)
        & BOOST_SERIALIZATION_NVP(m_int_color)
        & BOOST_SERIALIZATION_NVP(m_style)
        & BOOST_SERIALIZATION_NVP(m_button_x)
        & BOOST_SERIALIZATION_NVP(m_button_y)
        & BOOST_SERIALIZATION_NVP(m_button_wd)
        & BOOST_SERIALIZATION_NVP(m_button_ht)
        & BOOST_SERIALIZATION_NVP(m_text_x)
        & BOOST_SERIALIZATION_NVP(m_text_y);
}

template <class Archive>
void GG::RadioButtonGroup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_checked_button)
        & BOOST_SERIALIZATION_NVP(m_buttons);

    if (Archive::is_loading::value)
        ConnectSignals();
}

#endif // _GGButton_h_

