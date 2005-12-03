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

#include "GGButton.h"

#include <GGApp.h>
#include <GGDrawUtil.h>
#include <GGWndEditor.h>

#include <boost/lexical_cast.hpp>

using namespace GG;

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button() :
    TextControl()
{
}

Button::Button(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color, 
               Clr text_color/* = CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, text_color, TF_NONE, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;
}

Button::Button(int x, int y, int w, int h, const std::string& str, const std::string& font_filename, int pts, Clr color,
               Clr text_color/* = CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font_filename, pts, text_color, TF_NONE, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;
}

Button::ButtonState Button::State() const
{
    return m_state;
}

const SubTexture& Button::UnpressedGraphic() const
{
    return m_unpressed_graphic;
}

const SubTexture& Button::PressedGraphic() const
{
    return m_pressed_graphic;
}

const SubTexture& Button::RolloverGraphic() const
{
    return m_rollover_graphic;
}

void Button::Render()
{
    switch (m_state)
    {
    case BN_PRESSED:
        RenderPressed();
        break;
    case BN_UNPRESSED:
    case BN_ROLLOVER:
        if (m_state == BN_UNPRESSED)
            RenderUnpressed();
        else
            RenderRollover();
        break;
    }
}

void Button::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_state = BN_PRESSED;
}

void Button::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled())
        m_state = BN_PRESSED;
    if (Dragable())
        OffsetMove(move);
}

void Button::LButtonUp(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::LClick(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        m_state = BN_UNPRESSED;
        ClickedSignal();
    }
}

void Button::MouseHere(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_state = BN_ROLLOVER;
}

void Button::MouseLeave(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::SetColor(Clr c)
{
    Control::SetColor(c);
}

void Button::SetState(ButtonState state)
{
    m_state = state;
}

void Button::SetUnpressedGraphic(const SubTexture& st)
{
    m_unpressed_graphic = st;
}

void Button::SetPressedGraphic(const SubTexture& st)
{
    m_pressed_graphic = st;
}

void Button::SetRolloverGraphic(const SubTexture& st)
{
    m_rollover_graphic = st;
}

void Button::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    // TODO: handle setting graphics
}

void Button::RenderPressed()
{
    if (!m_pressed_graphic.Empty()) {
        glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v);
        m_pressed_graphic.OrthoBlit(UpperLeft(), LowerRight(), false);
    } else {
        RenderDefault();
    }
    OffsetMove(1,1);
    TextControl::Render();
    OffsetMove(-1,-1);
}

void Button::RenderUnpressed()
{
    if (!m_unpressed_graphic.Empty()) {
        glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v);
        m_unpressed_graphic.OrthoBlit(UpperLeft(), LowerRight(), false);
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(2,2);
    TextControl::Render();
    OffsetMove(-2,-2);
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderRollover()
{
    if (!m_rollover_graphic.Empty()) {
        glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v);
        m_rollover_graphic.OrthoBlit(UpperLeft(), LowerRight(), false);
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(2,2);
    TextControl::Render();
    OffsetMove(-2,-2);
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderDefault()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul.x, ul.y, lr.x, lr.y,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     (m_state != BN_PRESSED), 1);
}


////////////////////////////////////////////////
// GG::StateButton
////////////////////////////////////////////////
StateButton::StateButton() :
    TextControl()
{
}

StateButton::StateButton(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Uint32 text_fmt, 
                         Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
                         int bn_x/* = -1*/, int bn_y/* = -1*/, int bn_w/* = -1*/, int bn_h/* = -1*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, text_color, text_fmt, flags),
    m_checked(false),
    m_int_color(interior),
    m_style(style),
    m_button_x(0),
    m_button_y(0),
    m_text_x(0),
    m_text_y(0)
{
    Init(w, h, font->PointSize(), color, bn_w, bn_h, bn_x, bn_y);
}

StateButton::StateButton(int x, int y, int w, int h, const std::string& str, const std::string& font_filename,
                         int pts, Uint32 text_fmt, Clr color, Clr text_color/* = CLR_BLACK*/,
                         Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
                         int bn_x/* = -1*/, int bn_y/* = -1*/, int bn_w/* = -1*/, int bn_h/* = -1*/,
                         Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font_filename, pts, text_color, text_fmt, flags),
    m_checked(false),
    m_int_color(interior),
    m_style(style),
    m_button_x(0),
    m_button_y(0),
    m_text_x(0),
    m_text_y(0)
{
    Init(w, h, pts, color, bn_w, bn_h, bn_x, bn_y);
}

bool StateButton::Checked() const
{
    return m_checked;
}

Clr StateButton::InteriorColor() const
{
    return m_int_color;
}

StateButton::StateButtonStyle StateButton::Style() const
{
    return m_style;
}

void StateButton::Render()
{
    const Uint8 bevel = 2;

    // draw button
    Pt bn_ul = UpperLeft() + Pt(m_button_x, m_button_y);
    Pt bn_lr = bn_ul + Pt(m_button_wd, m_button_ht);

    switch (m_style) {
    case SBSTYLE_3D_XBOX:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, bevel);
        if (m_checked)
            BeveledX(bn_ul.x + 2 * bevel, bn_ul.y + 2 * bevel, bn_lr.x - 2 * bevel, bn_lr.y - 2 * bevel,
                     m_disabled ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_CHECKBOX:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, bevel);
        if (m_checked)
            BeveledCheck(bn_ul.x + 2 * bevel, bn_ul.y + 2 * bevel, bn_lr.x - 2 * bevel, bn_lr.y - 2 * bevel,
                         Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_RADIO:
        BeveledCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                      Disabled() ? DisabledColor(m_int_color) : m_int_color,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      false, bevel);
        if (m_checked)
            Bubble(bn_ul.x + 2 * bevel, bn_ul.y + 2 * bevel, bn_lr.x - 2 * bevel, bn_lr.y - 2 * bevel,
                   Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_BUTTON:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         !m_checked, bevel);
        break;
    case SBSTYLE_3D_ROUND_BUTTON:
        BeveledCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      Disabled() ? DisabledColor(m_color) : Color(),
                      !m_checked, bevel);
        break;
    }

    OffsetMove(m_text_x, m_text_y);
    TextControl::Render();
    OffsetMove(-m_text_x, -m_text_y);
}

void StateButton::LClick(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        SetCheck(!m_checked);
}

void StateButton::Reset()
{
    SetCheck(false);
}

void StateButton::SetCheck(bool b/* = true*/)
{
    CheckedSignal(m_checked = b);
}

void StateButton::SetColor(Clr c)
{
    Control::SetColor(c);
}

void StateButton::SetInteriorColor(Clr c)
{
    m_int_color = c;
}

void StateButton::SetStyle(StateButtonStyle bs)
{
    m_style = bs;
}

void StateButton::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    editor->Label("StateButton");
    editor->Attribute("Checked", m_checked);
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Button Style", m_style,
                      SBSTYLE_3D_XBOX, SBSTYLE_3D_ROUND_BUTTON);
    editor->Attribute("Button X", m_button_x);
    editor->Attribute("Button Y", m_button_x);
    editor->Attribute("Button Width", m_button_wd);
    editor->Attribute("Button Height", m_button_ht);
    editor->Attribute("Text X", m_text_x);
    editor->Attribute("Text Y", m_text_y);
}

int StateButton::ButtonX() const
{
    return m_button_x;
}

int StateButton::ButtonY() const
{
    return m_button_y;
}

int StateButton::ButtonWd() const
{
    return m_button_wd;
}

int StateButton::ButtonHt() const
{
    return m_button_ht;
}

int StateButton::TextX() const
{
    return m_text_x;
}

int StateButton::TextY() const
{
    return m_text_y;
}

void StateButton::Init(int w, int h, int pts, Clr color, int bn_x, int bn_y, int bn_w, int bn_h)
{
    m_color = color;

    if (bn_w == -1 || bn_h == -1)       // if one of these is not specified
        bn_w = bn_h = pts;              // set button width and height to text height

    double SPACING = 0.5; // the space to leave between the button and text, as a factor of the button's size (width or height)
    if (bn_x == -1 || bn_y == -1) {
        Uint32 format = TextFormat();
        if (format & TF_VCENTER)       // center button vertically
            bn_y = int((h - bn_h) / 2.0 + 0.5);
        if (format & TF_TOP) {         // put button at top, text just below
            bn_y = 0;
            m_text_y = bn_h;
        }
        if (format & TF_BOTTOM) {      // put button at bottom, text just above
            bn_y = (h - bn_h);
            m_text_y = int(h - (bn_h * (1 + SPACING)) - ((GetLineData().size() - 1) * GetFont()->Lineskip() + GetFont()->Height()) + 0.5);
        }

        if (format & TF_CENTER) {      // center button horizontally
            if (format & TF_VCENTER) { // if both the button and the text are to be centered, bad things happen
                format |= TF_LEFT;      // so go to the default (TF_CENTER|TF_LEFT)
                format &= ~TF_CENTER;
            } else {
                bn_x = int((w - bn_x) / 2.0 - bn_w / 2.0 + 0.5);
            }
        }
        if (format & TF_LEFT) {        // put button at left, text just to the right
            bn_x = 0;
            if (format & TF_VCENTER)
                m_text_x = int(bn_w * (1 + SPACING) + 0.5);
        }
        if (format & TF_RIGHT) {       // put button at right, text just to the left
            bn_x = (w - bn_w);
            if (format & TF_VCENTER)
                m_text_x = int(-bn_w * (1 + SPACING) + 0.5);
        }
        SetTextFormat(format);
    }
    m_button_x = bn_x;
    m_button_y = bn_y;
    m_button_wd = bn_w;
    m_button_ht = bn_h;
}


////////////////////////////////////////////////
// GG::RadioButtonGroup
////////////////////////////////////////////////
// ButtonClickedFunctor
RadioButtonGroup::ButtonClickedFunctor::ButtonClickedFunctor(RadioButtonGroup* grp, int idx) :
    m_grp(grp),
    m_idx(idx)
{
}

void RadioButtonGroup::ButtonClickedFunctor::operator()(bool checked)
{
    m_grp->HandleRadioClick(checked, m_idx);
}

// RadioButtonGroup
RadioButtonGroup::RadioButtonGroup() :
    Control()
{
}

RadioButtonGroup::RadioButtonGroup(int x, int y) :
    Control(x, y, 10, 10),
    m_checked_button(-1)
{
}

int RadioButtonGroup::NumButtons() const
{
    return m_buttons.size();
}

int RadioButtonGroup::CheckedButton() const
{
    return m_checked_button;
}

void RadioButtonGroup::Render()
{}

void RadioButtonGroup::SetCheck(int idx)
{
    if (idx == m_checked_button)
        return;

    if (idx < 0 || idx >= static_cast<int>(m_buttons.size()))
        idx = -1;
    // for buttons that are already checked, pass false to simulate the unchecking of a button for HandleRadioClick;
    // this prevents sending duplicate signals when a button is already checked
    HandleRadioClick(((idx != -1 && m_buttons[idx]->Checked()) ? false : true), idx);
}

void RadioButtonGroup::DisableButton(int idx, bool b/* = true*/)
{
    if (0 <= idx && idx < static_cast<int>(m_buttons.size())) {
        bool was_disabled = m_buttons[idx]->Disabled();
        m_buttons[idx]->Disable(b);
        if (b && !was_disabled && idx == m_checked_button)
            SetCheck(-1);
    }
}

void RadioButtonGroup::AddButton(StateButton* bn)
{
    m_buttons.push_back(bn);
    m_connections.push_back(Connect(m_buttons.back()->CheckedSignal, ButtonClickedFunctor(this, m_buttons.size() - 1)));
    if (bn->LowerRight() >= Size()) // stretch group to encompass all its children
        Resize(bn->LowerRight());
    AttachChild(bn);
}

const std::vector<StateButton*>& RadioButtonGroup::Buttons() const
{
    return m_buttons;
}

const std::vector<boost::signals::connection>& RadioButtonGroup::Connections() const
{
    return m_connections;
}

void RadioButtonGroup::ConnectSignals()
{
    for (unsigned int i = 0; i < m_buttons.size(); ++i)
        m_connections.push_back(Connect(m_buttons[i]->CheckedSignal, ButtonClickedFunctor(this, m_buttons.size() - 1)));
    SetCheck(m_checked_button);
}

void RadioButtonGroup::HandleRadioClick(bool checked, int index)
{
    m_checked_button = index;
    if (checked) {
        for (unsigned int i = 0; i < m_connections.size(); ++i) {
            if (m_buttons[i]->Checked() != (static_cast<int>(i) == index)) {
                m_connections[i].disconnect();
                m_buttons[i]->SetCheck(static_cast<int>(i) == index);
                m_connections[i] = m_buttons[i]->CheckedSignal.connect(ButtonClickedFunctor(this, i), boost::signals::at_front);
            }
        }
        ButtonChangedSignal(m_checked_button);
    } else {
        m_connections[index].disconnect();
        m_buttons[index]->SetCheck(true);
        m_connections[index] = m_buttons[index]->CheckedSignal.connect(ButtonClickedFunctor(this, index), boost::signals::at_front);
    }
}
