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
#include "GGApp.h"
#include "GGDrawUtil.h"

namespace GG {

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, 
               Clr text_color/* = CLR_BLACK*/, Uint32 flags/* = CLICKABLE*/) : 
   StaticText(x, y, w, h, str, font_filename, pts, TF_NONE, text_color, flags), 
   m_state(BN_UNPRESSED)
{
   m_color = color;
}

Button::Button(const XMLElement& elem) : 
   StaticText(elem.Child("GG::StaticText"))
{
   if (elem.Tag() != "GG::Button")
      throw std::invalid_argument("Attempted to construct a GG::Button from an XMLElement that had a tag other than \"GG::Button\"");
   
   const XMLElement* curr_elem = &elem.Child("m_state");
   m_state = ButtonState(lexical_cast<int>(curr_elem->Attribute("value")));
   
   curr_elem = &elem.Child("m_unpressed_graphic");
   m_unpressed_graphic = SubTexture(curr_elem->Child("GG::SubTexture"));
   
   curr_elem = &elem.Child("m_pressed_graphic");
   m_pressed_graphic = SubTexture(curr_elem->Child("GG::SubTexture"));
   
   curr_elem = &elem.Child("m_rollover_graphic");
   m_rollover_graphic = SubTexture(curr_elem->Child("GG::SubTexture"));
}

int Button::Render()
{
   switch (m_state)
   {
   case BN_PRESSED:
      RenderPressed();
      OffsetMove(1,1);
      StaticText::Render();
      OffsetMove(-1,-1);
      break;
   case BN_UNPRESSED:
   case BN_ROLLOVER:
      if (m_state == BN_UNPRESSED)
         RenderUnpressed();
      else
         RenderRollover();
      // draw text shadow
      Clr temp = m_text_color;  // save original color
      m_text_color = CLR_SHADOW; // shadow color
      OffsetMove(2,2);
      StaticText::Render();
      OffsetMove(-2,-2);
      m_text_color = temp;    // restore original color
      // draw text
      StaticText::Render();
      break;
   }
   return 1;
}

XMLElement Button::XMLEncode() const
{
   XMLElement retval("GG::Button");
   retval.AppendChild(StaticText::XMLEncode());
   
   XMLElement temp;
   
   temp = XMLElement("m_state");
   temp.SetAttribute("value", lexical_cast<string>(m_state));
   retval.AppendChild(temp);

   temp = XMLElement("m_unpressed_graphic");
   temp.AppendChild(m_unpressed_graphic.XMLEncode());
   retval.AppendChild(temp);
   
   temp = XMLElement("m_pressed_graphic");
   temp.AppendChild(m_pressed_graphic.XMLEncode());
   retval.AppendChild(temp);

   temp = XMLElement("m_rollover_graphic");
   temp.AppendChild(m_rollover_graphic.XMLEncode());
   retval.AppendChild(temp);

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
}

void Button::RenderUnpressed()
{
   if (!m_unpressed_graphic.Empty()) {
      glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v); 
      m_unpressed_graphic.OrthoBlit(UpperLeft(), LowerRight(), false);
   } else {
      RenderDefault();
   }
}

void Button::RenderRollover()
{
   if (!m_rollover_graphic.Empty()) {
      glColor4ubv(Disabled() ? DisabledColor(m_color).v : m_color.v); 
      m_rollover_graphic.OrthoBlit(UpperLeft(), LowerRight(), false);
   } else {
      RenderDefault();
   }
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
StateButton::StateButton(int x, int y, int w, int h, const string& str, const string& font_filename, 
                         int pts, Uint32 text_fmt, Clr color, Clr text_color/* = CLR_BLACK*/,
                         Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/, 
                         int bn_x/* = -1*/, int bn_y/* = -1*/, int bn_w/* = -1*/, int bn_h/* = -1*/, 
                         Uint32 flags/* = CLICKABLE*/) : 
   StaticText(x, y, w, h, str, font_filename, pts, text_fmt, text_color, flags), 
   m_checked(false), 
   m_int_color(interior),
   m_style(style), 
   m_button_x(0), 
   m_button_y(0), 
   m_text_x(0), 
   m_text_y(0)
{
   m_color = color;

   if (bn_w == -1 || bn_h == -1)       // if one of these is not specified
      bn_w = bn_h = pts;               // set button width and height to text height

   double SPACING = 0.5f; // the space to leave between the button and text, as a factor of the button's size (width or height) 
   if (bn_x == -1 || bn_y == -1) {
      if (m_format & TF_VCENTER)       // center button vertically
         bn_y = int((h - bn_h) / 2.0f + 0.5f);
      if (m_format & TF_TOP) {         // put button at top, text just below
         bn_y = 0;
         m_text_y = bn_h;
      }
      if (m_format & TF_BOTTOM) {      // put button at bottom, text just above
         bn_y = (h - bn_h);
         m_text_y = int(h - (bn_h * (1 + SPACING)) - TextImage::DefaultHeight() + 0.5f);
      }

      if (m_format & TF_CENTER) {      // center button horizontally
         if (m_format & TF_VCENTER) { // if both the button and the text are to be centered, bad things happen
            m_format |= TF_LEFT;      // so go to the default (TF_CENTER|TF_LEFT)
            m_format &= ~TF_CENTER;
         } else {
            bn_x = int((w - bn_x) / 2.0f - bn_w / 2.0f + 0.5f);
         }
      }
      if (m_format & TF_LEFT) {        // put button at left, text just to the right
         bn_x = 0;
         if (m_format & TF_VCENTER)
            m_text_x = int(bn_w * (1 + SPACING) + 0.5f);
      }
      if (m_format & TF_RIGHT) {       // put button at right, text just to the left
         bn_x = (w - bn_w);            
         if (m_format & TF_VCENTER)
            m_text_x = int(-bn_w * (1 + SPACING) + 0.5f);
      }
   }
   m_button_x = bn_x;
   m_button_y = bn_y;
   m_button_wd = bn_w;
   m_button_ht = bn_h;
}

StateButton::StateButton(const XMLElement& elem) : 
   StaticText(elem.Child("GG::StaticText"))
{
   if (elem.Tag() != "GG::StateButton")
      throw std::invalid_argument("Attempted to construct a GG::StateButton from an XMLElement that had a tag other than \"GG::StateButton\"");
   
   const XMLElement* curr_elem = &elem.Child("m_checked");
   m_checked = lexical_cast<bool>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_int_color");
   m_int_color = Clr(curr_elem->Child("GG::Clr"));
   
   curr_elem = &elem.Child("m_style");
   m_style = StateButtonStyle(lexical_cast<int>(curr_elem->Attribute("value")));
   
   curr_elem = &elem.Child("m_button_x");
   m_button_x = lexical_cast<int>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_button_y");
   m_button_y = lexical_cast<int>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_button_wd");
   m_button_wd = lexical_cast<int>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_button_ht");
   m_button_ht = lexical_cast<int>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_text_x");
   m_text_x = lexical_cast<int>(curr_elem->Attribute("value"));
   
   curr_elem = &elem.Child("m_text_y");
   m_text_y = lexical_cast<int>(curr_elem->Attribute("value"));
}


int StateButton::Render()
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
   StaticText::Render();
   OffsetMove(-m_text_x, -m_text_y);

   return 1;
}

int StateButton::LClick(const Pt& pt, Uint32 keys) 
{
   if (!Disabled())
      SetCheck(!m_checked);
   return 1;
}

XMLElement StateButton::XMLEncode() const
{
   XMLElement retval("GG::StateButton");
   retval.AppendChild(StaticText::XMLEncode());
   
   XMLElement temp;
   
   temp = XMLElement("m_checked");
   temp.SetAttribute("value", lexical_cast<string>(m_checked));
   retval.AppendChild(temp);

   temp = XMLElement("m_int_color");
   temp.AppendChild(m_int_color.XMLEncode());
   retval.AppendChild(temp);
   
   temp = XMLElement("m_style");
   temp.SetAttribute("value", lexical_cast<string>(m_style));
   retval.AppendChild(temp);

   temp = XMLElement("m_button_x");
   temp.SetAttribute("value", lexical_cast<string>(m_button_x));
   retval.AppendChild(temp);

   temp = XMLElement("m_button_y");
   temp.SetAttribute("value", lexical_cast<string>(m_button_y));
   retval.AppendChild(temp);

   temp = XMLElement("m_button_wd");
   temp.SetAttribute("value", lexical_cast<string>(m_button_wd));
   retval.AppendChild(temp);

   temp = XMLElement("m_button_ht");
   temp.SetAttribute("value", lexical_cast<string>(m_button_ht));
   retval.AppendChild(temp);

   temp = XMLElement("m_text_x");
   temp.SetAttribute("value", lexical_cast<string>(m_text_x));
   retval.AppendChild(temp);

   temp = XMLElement("m_text_y");
   temp.SetAttribute("value", lexical_cast<string>(m_text_y));
   retval.AppendChild(temp);

   return retval;
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
   
   const XMLElement* curr_elem = &elem.Child("m_checked_button");
   m_checked_button = lexical_cast<int>(curr_elem->Attribute("value"));
   
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
   if (bn->LowerRight() >= WindowDimensions()) // stretch group to encompass all its children
      Resize(bn->LowerRight());
   AttachChild(bn);
}
   
XMLElement RadioButtonGroup::XMLEncode() const
{
   XMLElement retval("GG::RadioButtonGroup");
   retval.AppendChild(Control::XMLEncode());
   
   XMLElement temp("m_checked_button");
   temp.SetAttribute("value", lexical_cast<string>(m_checked_button));
   retval.AppendChild(temp);
   
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

