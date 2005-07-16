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

using namespace GG;

ThreeButtonDlg::ThreeButtonDlg() :
    Wnd()
{
}

ThreeButtonDlg::ThreeButtonDlg(int x, int y, int w, int h, const std::string& msg, const std::string& font_filename,
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

ThreeButtonDlg::ThreeButtonDlg(int w, int h, const std::string& msg, const std::string& font_filename, int pts,
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

ThreeButtonDlg::ThreeButtonDlg(int x, int y, int w, int h, const std::string& msg, const std::string& font_filename,
                               int pts, Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                               const std::string& zero, const std::string& one/* = ""*/,
                               const std::string& two/* = ""*/) :
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

ThreeButtonDlg::ThreeButtonDlg(int w, int h, const std::string& msg, const std::string& font_filename, int pts,
                               Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                               const std::string& zero, const std::string& one/* = ""*/, const std::string& two/* = ""*/) :
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

Clr ThreeButtonDlg::ButtonColor() const
{
    return m_button_color;
}

int ThreeButtonDlg::Result() const
{
    return m_result;
}

int ThreeButtonDlg::DefaultButton() const
{
    return m_default;
}

int ThreeButtonDlg::EscapeButton() const
{
    return m_escape;
}

bool ThreeButtonDlg::Render()
{
    FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_color, m_border_color, 1);
    return true;
}

void ThreeButtonDlg::Keypress(Key key, Uint32 key_mods)
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

void ThreeButtonDlg::Init(const std::string& msg, const std::string& font_filename, int pts, int buttons,
                          const std::string& zero/* = ""*/, const std::string& one/* = ""*/, const std::string& two/* = ""*/)
{
    if (buttons < 1)
        buttons = 1;
    else if (3 < buttons)
        buttons = 3;

    const int SPACING = 10;
    const int BUTTON_WIDTH = (Width() - (buttons + 1) * SPACING) / buttons;
    const int BUTTON_HEIGHT = pts + 8;

    AttachChild(new TextControl(0, 0, Width(), Height() - BUTTON_HEIGHT - 2 * SPACING, msg, font_filename, pts, m_text_color,
                                TF_CENTER | TF_VCENTER | TF_WORDBREAK));

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

    AttachChild(m_button_0);
    AttachChild(m_button_1);
    AttachChild(m_button_2);

    ConnectSignals();
}

void ThreeButtonDlg::ConnectSignals()
{
    Connect(m_button_0->ClickedSignal, &ThreeButtonDlg::Button0Clicked, this);
    if (m_button_1)
        Connect(m_button_1->ClickedSignal, &ThreeButtonDlg::Button1Clicked, this);
    if (m_button_2)
        Connect(m_button_2->ClickedSignal, &ThreeButtonDlg::Button2Clicked, this);
}

void ThreeButtonDlg::Button0Clicked()
{
    m_done = true;
    m_result = 0;
}

void ThreeButtonDlg::Button1Clicked()
{
    m_done = true;
    m_result = 1;
}

void ThreeButtonDlg::Button2Clicked()
{
    m_done = true;
    m_result = 2;
}
