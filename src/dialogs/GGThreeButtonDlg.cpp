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

#include "GGThreeButtonDlg.h"

#include "../GGApp.h"
#include "../GGTextControl.h"
#include "../GGButton.h"
#include "../GGDrawUtil.h"

namespace GG {

ThreeButtonDlg::ThreeButtonDlg(int x, int y, int w, int h, const string& msg, const string& font_filename,
                               int pts, Clr color, Clr border_color, Clr button_color, Clr text_color/* = CLR_BLACK*/,
                               int buttons/* = 3*/, Button* zero/* = 0*/, Button* one/* = 0*/, Button* two/* = 0*/) :
    Wnd(x, y, w, h, CLICKABLE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(zero),
    m_button_1(one),
    m_button_2(two)
{
    Init(msg, font_filename, pts, buttons);
}

ThreeButtonDlg::ThreeButtonDlg(int w, int h, const string& msg, const string& font_filename, int pts,
                               Clr color, Clr border_color, Clr button_color, Clr text_color/* = CLR_BLACK*/, int buttons/* = 3*/,
                               Button* zero/* = 0*/, Button* one/* = 0*/, Button* two/* = 0*/) :
    Wnd((App::GetApp()->AppWidth() - w) / 2, (App::GetApp()->AppHeight() - h) / 2, w, h, CLICKABLE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(zero),
    m_button_1(one),
    m_button_2(two)
{
    Init(msg, font_filename, pts, buttons);
}

ThreeButtonDlg::ThreeButtonDlg(int x, int y, int w, int h, const string& msg, const string& font_filename,
                               int pts, Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                               const string& zero, const string& one/* = ""*/,
                               const string& two/* = ""*/) :
    Wnd(x, y, w, h, CLICKABLE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(0),
    m_button_1(0),
    m_button_2(0)
{
    Init(msg, font_filename, pts, buttons, zero, one, two);
}

ThreeButtonDlg::ThreeButtonDlg(int w, int h, const string& msg, const string& font_filename, int pts,
                               Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                               const string& zero, const string& one/* = ""*/, const string& two/* = ""*/) :
    Wnd((App::GetApp()->AppWidth() - w) / 2, (App::GetApp()->AppHeight() - h) / 2, w, h, CLICKABLE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_button_color(button_color),
    m_default(0),
    m_escape(buttons - 1),
    m_result(0),
    m_button_0(0),
    m_button_1(0),
    m_button_2(0)
{
    Init(msg, font_filename, pts, buttons, zero, one, two);
}

ThreeButtonDlg::ThreeButtonDlg(const XMLElement& elem) :
    Wnd(elem.Child("GG::Wnd")),
    m_result(0),
    m_button_1(0),
    m_button_2(0)
{
    if (elem.Tag() != "GG::ThreeButtonDlg")
        throw std::invalid_argument("Attempted to construct a GG::ThreeButtonDlg from an XMLElement that had a tag other than \"GG::ThreeButtonDlg\"");

    m_color = Clr(elem.Child("m_color").Child("GG::Clr"));
    m_border_color = Clr(elem.Child("m_border_color").Child("GG::Clr"));
    m_text_color = Clr(elem.Child("m_text_color").Child("GG::Clr"));
    m_button_color = Clr(elem.Child("m_button_color").Child("GG::Clr"));
    m_default = lexical_cast<int>(elem.Child("m_default").Text());
    m_escape = lexical_cast<int>(elem.Child("m_escape").Text());

    const XMLElement* curr_elem = &elem.Child("m_button_0");
    if (!(m_button_0 = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("ThreeButtonDlg::ThreeButtonDlg : Attempted to use a non-Button object as the ok button.");

    curr_elem = &elem.Child("m_button_1");
    if (curr_elem->NumChildren() && !(m_button_1 = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("ThreeButtonDlg::ThreeButtonDlg : Attempted to use a non-Button object as the ok button.");

    curr_elem = &elem.Child("m_button_2");
    if (curr_elem->NumChildren() && !(m_button_2 = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("ThreeButtonDlg::ThreeButtonDlg : Attempted to use a non-Button object as the ok button.");

    Connect(m_button_0->ClickedSignal(), &ThreeButtonDlg::Button0Clicked, this);
    if (m_button_1)
        Connect(m_button_1->ClickedSignal(), &ThreeButtonDlg::Button1Clicked, this);
    if (m_button_2)
        Connect(m_button_2->ClickedSignal(), &ThreeButtonDlg::Button2Clicked, this);
    AttachSignalChildren();
}

XMLElement ThreeButtonDlg::XMLEncode() const
{
    XMLElement retval("GG::ThreeButtonDlg");
    const_cast<ThreeButtonDlg*>(this)->DetachSignalChildren();
    retval.AppendChild(Wnd::XMLEncode());

    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_border_color", m_border_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_text_color", m_text_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_button_color", m_button_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_default", lexical_cast<string>(m_default)));
    retval.AppendChild(XMLElement("m_escape", lexical_cast<string>(m_escape)));
    retval.AppendChild(XMLElement("m_button_0", m_button_0->XMLEncode()));

    XMLElement temp("m_button_1");
    if (m_button_1)
        temp.AppendChild(m_button_1->XMLEncode());
    retval.AppendChild(XMLElement());

    temp = XMLElement("m_button_2");
    if (m_button_2)
        temp.AppendChild(m_button_2->XMLEncode());
    retval.AppendChild(XMLElement());

    const_cast<ThreeButtonDlg*>(this)->AttachSignalChildren();

    return retval;
}

int ThreeButtonDlg::Render()
{
    FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_color, m_border_color, 1);
    return 1;
}

int ThreeButtonDlg::Keypress(Key key, Uint32 key_mods)
{
    if (key == GGK_RETURN || key == GGK_KP_ENTER && m_default != -1) {
        if (m_default == 0)
            Button0Clicked();
        else if (m_default == 1)
            Button1Clicked();
        else if (m_default == 1)
            Button2Clicked();
    } else if (key == GGK_ESCAPE && m_escape != -1) {
        if (m_escape == 0)
            Button0Clicked();
        else if (m_escape == 1)
            Button1Clicked();
        else if (m_escape == 2)
            Button2Clicked();
    }
    return 1;
}

void ThreeButtonDlg::SetButtonColor(Clr color)
{
    m_button_color = color;
    if (m_button_0)
        m_button_0->SetColor(color);
    if (m_button_1)
        m_button_1->SetColor(color);
    if (m_button_2)
        m_button_2->SetColor(color);
}

void ThreeButtonDlg::SetDefaultButton(int i)
{
    if (i < 0 || NumButtons() <= i)
        m_default = -1;
    else
        m_default = i;
}

void ThreeButtonDlg::SetEscapeButton(int i)
{
    if (i < 0 || NumButtons() <= i)
        m_escape = -1;
    else
        m_escape = i;
}

int ThreeButtonDlg::NumButtons() const
{
    int retval = 1;
    if (m_button_2)
        retval = 3;
    else if (m_button_1)
        retval = 2;
    return retval;
}

void ThreeButtonDlg::AttachSignalChildren()
{
    AttachChild(m_button_0);
    AttachChild(m_button_1);
    AttachChild(m_button_2);
}

void ThreeButtonDlg::DetachSignalChildren()
{
    DetachChild(m_button_0);
    DetachChild(m_button_1);
    DetachChild(m_button_2);
}

void ThreeButtonDlg::Init(const string& msg, const string& font_filename, int pts, int buttons,
                          const string& zero/* = ""*/, const string& one/* = ""*/, const string& two/* = ""*/)
{
    if (buttons < 1)
        buttons = 1;
    else if (3 < buttons)
        buttons = 3;

    const int SPACING = 10;
    const int BUTTON_WIDTH = (Width() - (buttons + 1) * SPACING) / buttons;
    const int BUTTON_HEIGHT = pts + 8;

    AttachChild(new TextControl(0, 0, Width(), Height() - BUTTON_HEIGHT - 2 * SPACING, msg, font_filename, pts,
                                TF_CENTER | TF_VCENTER | TF_WORDBREAK, m_text_color));

    if (!m_button_0) {
        m_button_0 = new Button(SPACING + (BUTTON_WIDTH + SPACING) * 0,
                                Height() - BUTTON_HEIGHT - SPACING,
                                BUTTON_WIDTH, BUTTON_HEIGHT,
                                (zero == "" ? (buttons < 3 ? "Ok" : "Yes") : zero),
                                        font_filename, pts, m_button_color, m_text_color);
    }
    if (!m_button_1 && 2 <= buttons) {
        m_button_1 = new Button(SPACING + (BUTTON_WIDTH + SPACING) * 1,
                                Height() - BUTTON_HEIGHT - SPACING,
                                BUTTON_WIDTH, BUTTON_HEIGHT,
                                (one == "" ? (buttons < 3 ? "Cancel" : "No") : one),
                                        font_filename, pts, m_button_color, m_text_color);
    }
    if (!m_button_2 && 3 <= buttons) {
        m_button_2 = new Button(SPACING + (BUTTON_WIDTH + SPACING) * 2,
                                Height() - BUTTON_HEIGHT - SPACING,
                                BUTTON_WIDTH, BUTTON_HEIGHT,
                                (two == "" ? "Cancel" : two),
                                font_filename, pts, m_button_color, m_text_color);
    }

    Connect(m_button_0->ClickedSignal(), &ThreeButtonDlg::Button0Clicked, this);
    if (m_button_1)
        Connect(m_button_1->ClickedSignal(), &ThreeButtonDlg::Button1Clicked, this);
    if (m_button_2)
        Connect(m_button_2->ClickedSignal(), &ThreeButtonDlg::Button2Clicked, this);
    AttachSignalChildren();
}

} // namespace GG

