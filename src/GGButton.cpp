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
#include <XMLValidators.h>

namespace GG {

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button(int x, int y, int w, int h, const string& str, const shared_ptr<GG::Font>& font, Clr color, 
	       Clr text_color/* = CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, TF_NONE, text_color, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;
}

Button::Button(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color,
               Clr text_color/* = CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font_filename, pts, TF_NONE, text_color, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;
}

Button::Button(const XMLElement& elem) :
    TextControl(elem.Child("GG::TextControl"))
{
    if (elem.Tag() != "GG::Button")
        throw std::invalid_argument("Attempted to construct a GG::Button from an XMLElement that had a tag other than \"GG::Button\"");

    m_state = lexical_cast<ButtonState>(elem.Child("m_state").Text());
    m_unpressed_graphic = SubTexture(elem.Child("m_unpressed_graphic").Child("GG::SubTexture"));
    m_pressed_graphic = SubTexture(elem.Child("m_pressed_graphic").Child("GG::SubTexture"));
    m_rollover_graphic = SubTexture(elem.Child("m_rollover_graphic").Child("GG::SubTexture"));
}

bool Button::Render()
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
    return true;
}

XMLElement Button::XMLEncode() const
{
    XMLElement retval("GG::Button");
    retval.AppendChild(TextControl::XMLEncode());
    retval.AppendChild(XMLElement("m_state", lexical_cast<string>(m_state)));
    retval.AppendChild(XMLElement("m_unpressed_graphic", m_unpressed_graphic.XMLEncode()));
    retval.AppendChild(XMLElement("m_pressed_graphic", m_pressed_graphic.XMLEncode()));
    retval.AppendChild(XMLElement("m_rollover_graphic", m_rollover_graphic.XMLEncode()));
    return retval;
}

XMLElementValidator Button::XMLValidator() const
{
    XMLElementValidator retval("GG::Button");
    retval.AppendChild(TextControl::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_state", new MappedEnumValidator<ButtonState>()));
    retval.AppendChild(XMLElementValidator("m_unpressed_graphic", m_unpressed_graphic.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_pressed_graphic", m_pressed_graphic.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_rollover_graphic", m_rollover_graphic.XMLValidator()));
    return retval;
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
StateButton::StateButton(int x, int y, int w, int h, const string& str, const shared_ptr<GG::Font>& font, Uint32 text_fmt, 
			 Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
			 int bn_x/* = -1*/, int bn_y/* = -1*/, int bn_w/* = -1*/, int bn_h/* = -1*/, Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, text_fmt, text_color, flags),
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

StateButton::StateButton(int x, int y, int w, int h, const string& str, const string& font_filename,
                         int pts, Uint32 text_fmt, Clr color, Clr text_color/* = CLR_BLACK*/,
                         Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
                         int bn_x/* = -1*/, int bn_y/* = -1*/, int bn_w/* = -1*/, int bn_h/* = -1*/,
                         Uint32 flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font_filename, pts, text_fmt, text_color, flags),
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

StateButton::StateButton(const XMLElement& elem) :
        TextControl(elem.Child("GG::TextControl"))
{
    if (elem.Tag() != "GG::StateButton")
        throw std::invalid_argument("Attempted to construct a GG::StateButton from an XMLElement that had a tag other than \"GG::StateButton\"");

    m_checked = lexical_cast<bool>(elem.Child("m_checked").Text());
    m_int_color = Clr(elem.Child("m_int_color").Child("GG::Clr"));
    m_style = lexical_cast<StateButtonStyle>(elem.Child("m_style").Text());
    m_button_x = lexical_cast<int>(elem.Child("m_button_x").Text());
    m_button_y = lexical_cast<int>(elem.Child("m_button_y").Text());
    m_button_wd = lexical_cast<int>(elem.Child("m_button_wd").Text());
    m_button_ht = lexical_cast<int>(elem.Child("m_button_ht").Text());
    m_text_x = lexical_cast<int>(elem.Child("m_text_x").Text());
    m_text_y = lexical_cast<int>(elem.Child("m_text_y").Text());
}


bool StateButton::Render()
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

    return true;
}

void StateButton::LClick(const Pt& pt, Uint32 keys)
{
    if (!Disabled())
        SetCheck(!m_checked);
}

XMLElement StateButton::XMLEncode() const
{
    XMLElement retval("GG::StateButton");
    retval.AppendChild(TextControl::XMLEncode());
    retval.AppendChild(XMLElement("m_checked", lexical_cast<string>(m_checked)));
    retval.AppendChild(XMLElement("m_int_color", m_int_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_style", lexical_cast<string>(m_style)));
    retval.AppendChild(XMLElement("m_button_x", lexical_cast<string>(m_button_x)));
    retval.AppendChild(XMLElement("m_button_y", lexical_cast<string>(m_button_y)));
    retval.AppendChild(XMLElement("m_button_wd", lexical_cast<string>(m_button_wd)));
    retval.AppendChild(XMLElement("m_button_ht", lexical_cast<string>(m_button_ht)));
    retval.AppendChild(XMLElement("m_text_x", lexical_cast<string>(m_text_x)));
    retval.AppendChild(XMLElement("m_text_y", lexical_cast<string>(m_text_y)));
    return retval;
}

XMLElementValidator StateButton::XMLValidator() const
{
    XMLElementValidator retval("GG::StateButton");
    retval.AppendChild(TextControl::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_checked", new Validator<bool>()));
    retval.AppendChild(XMLElementValidator("m_int_color", m_int_color.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_style", new MappedEnumValidator<StateButtonStyle>()));
    retval.AppendChild(XMLElementValidator("m_button_x", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_button_y", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_button_wd", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_button_ht", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_text_x", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_text_y", new Validator<int>()));
    return retval;
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
RadioButtonGroup::RadioButtonGroup(const XMLElement& elem) :
    Control(elem.Child("GG::Control"))
{
    if (elem.Tag() != "GG::RadioButtonGroup")
        throw std::invalid_argument("Attempted to construct a GG::RadioButtonGroup from an XMLElement that had a tag other than \"GG::RadioButtonGroup\"");

    for (list<Wnd*>::const_iterator it = Wnd::Children().begin(); it != Wnd::Children().end(); ++it) {
        if (StateButton* sb = dynamic_cast<StateButton*>(*it)) {
            m_buttons.push_back(sb);
            m_connections.push_back(Connect(sb->CheckedSignal(), ButtonClickedFunctor(this, m_connections.size())));
        } else {
            throw std::runtime_error("Attempted to use a non-StateButton object as a member of a GG::RadioButtonGroup");
        }
    }

    m_checked_button = lexical_cast<int>(elem.Child("m_checked_button").Text());

    SetCheck(m_checked_button);
}

void RadioButtonGroup::SetCheck(int idx)
{
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
    m_connections.push_back(Connect(m_buttons.back()->CheckedSignal(), ButtonClickedFunctor(this, m_connections.size())));
    if (bn->LowerRight() >= Size()) // stretch group to encompass all its children
        Resize(bn->LowerRight());
    AttachChild(bn);
}

XMLElement RadioButtonGroup::XMLEncode() const
{
    XMLElement retval("GG::RadioButtonGroup");
    retval.AppendChild(Control::XMLEncode());
    retval.AppendChild(XMLElement("m_checked_button", lexical_cast<string>(m_checked_button)));
    return retval;
}

XMLElementValidator RadioButtonGroup::XMLValidator() const
{
    XMLElementValidator retval("GG::RadioButtonGroup");
    retval.AppendChild(Control::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_checked_button", new Validator<int>()));
    return retval;
}

void RadioButtonGroup::HandleRadioClick(bool checked, int index)
{
    m_checked_button = index;
    if (checked) {
        for (unsigned int i = 0; i < m_connections.size(); ++i) {
            if (static_cast<int>(i) == index) {
                m_connections[i].disconnect();
                m_buttons[i]->SetCheck(true);
                m_connections[i] = GG::Connect(m_buttons[i]->CheckedSignal(), ButtonClickedFunctor(this, i));
            } else {
                m_connections[i].disconnect();
                m_buttons[i]->SetCheck(false);
                m_connections[i] = GG::Connect(m_buttons[i]->CheckedSignal(), ButtonClickedFunctor(this, i));
            }
        }
        m_button_changed_sig(m_checked_button);
    } else {
        m_connections[index].disconnect();
        m_buttons[index]->SetCheck(true);
        m_connections[index] = GG::Connect(m_buttons[index]->CheckedSignal(), ButtonClickedFunctor(this, index));
    }
}

} // namespace GG

