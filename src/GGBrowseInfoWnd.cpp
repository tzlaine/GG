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

#include "GGBrowseInfoWnd.h"

#include <GGApp.h>
#include <GGDrawUtil.h>
#include <GGFont.h>
#include <GGLayout.h>
#include <GGTextControl.h>

using namespace GG;

////////////////////////////////////////////////
// GG::BrowseInfoWnd
////////////////////////////////////////////////
BrowseInfoWnd::BrowseInfoWnd() :
    Wnd()
{}

BrowseInfoWnd::BrowseInfoWnd(int x, int y, int w, int h) :
    Wnd(x, y, w, h)
{}

void BrowseInfoWnd::Update(int mode, const Wnd* target)
{}


////////////////////////////////////////////////
// GG::TextBoxBrowseInfoWnd
////////////////////////////////////////////////
TextBoxBrowseInfoWnd::TextBoxBrowseInfoWnd() :
    BrowseInfoWnd()
{}

TextBoxBrowseInfoWnd::TextBoxBrowseInfoWnd(int w, const boost::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color,
                                           Uint32 text_fmt/* = TF_LEFT | TF_WORDBREAK*/, int border_width/* = 2*/, int text_margin/* = 4*/) :
    BrowseInfoWnd(0, 0, w, 1),
    m_text_from_target(true),
    m_font(font),
    m_color(color),
    m_border_color(border_color),
    m_border_width(border_width),
    m_text_control(new TextControl(0, 0, w, 1, "", m_font, text_color, text_fmt))
{
    m_text_control->SetMinSize(true);
    AttachChild(m_text_control);
    GridLayout();
    SetLayoutBorderMargin(text_margin);
}

TextBoxBrowseInfoWnd::TextBoxBrowseInfoWnd(int w, const std::string& font_filename, int pts, Clr color, Clr border_color, Clr text_color,
                                           Uint32 text_fmt/* = TF_LEFT | TF_WORDBREAK*/, int border_width/* = 2*/, int text_margin/* = 4*/) :
    BrowseInfoWnd(0, 0, w, 1),
    m_text_from_target(true),
    m_font(App::GetApp()->GetFont(font_filename, pts)),
    m_color(color),
    m_border_color(border_color),
    m_border_width(border_width),
    m_text_control(new TextControl(0, 0, w, 1, "", m_font, text_color, text_fmt))
{
    m_text_control->SetMinSize(true);
    AttachChild(m_text_control);
    GridLayout();
    SetLayoutBorderMargin(text_margin);
}

bool TextBoxBrowseInfoWnd::TextFromTarget() const
{
    return m_text_from_target;
}

const std::string& TextBoxBrowseInfoWnd::Text() const
{
    return m_text_control->WindowText();
}

const boost::shared_ptr<Font>& TextBoxBrowseInfoWnd::GetFont() const
{
    return m_font;
}

Clr TextBoxBrowseInfoWnd::Color() const
{
    return m_color;
}

Clr TextBoxBrowseInfoWnd::TextColor() const
{
    return m_text_control->TextColor();
}

Uint32 TextBoxBrowseInfoWnd::TextFormat() const
{
    return m_text_control->TextFormat();
}

Clr TextBoxBrowseInfoWnd::BorderColor() const
{
    return m_border_color;
}

int TextBoxBrowseInfoWnd::BorderWidth() const
{
    return m_border_width;
}

int TextBoxBrowseInfoWnd::TextMargin() const
{
    return GetLayout()->BorderMargin();
}

void TextBoxBrowseInfoWnd::SetText(const std::string& str)
{
    m_text = str;
    m_text_control->SetText(str);
    if (str.empty())
        Hide();
    else
        Show();
    Resize(1, 1);
}

bool TextBoxBrowseInfoWnd::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, m_color, m_border_color, m_border_width);
    return true;
}

void TextBoxBrowseInfoWnd::Update(int mode, const Wnd* target)
{
    if (m_text_from_target) {
        std::string text = target->BrowseInfoText(mode);
        SetText(text);
    }
}

void TextBoxBrowseInfoWnd::SetTextFromTarget(bool b)
{
    m_text_from_target = b;
}

void TextBoxBrowseInfoWnd::SetFont(const boost::shared_ptr<Font>& font)
{
    m_font = font;
}

void TextBoxBrowseInfoWnd::SetColor(Clr color)
{
    m_color = color;
}

void TextBoxBrowseInfoWnd::SetBorderColor(Clr border_color)
{
    m_border_color = border_color;
}

void TextBoxBrowseInfoWnd::SetTextColor(Clr text_color)
{
    m_text_control->SetTextColor(text_color);
}

void TextBoxBrowseInfoWnd::SetTextFormat(Uint32 format)
{
    m_text_control->SetTextFormat(format);
}

void TextBoxBrowseInfoWnd::SetBorderWidth(int border_width)
{
    m_border_width = border_width;
}

void TextBoxBrowseInfoWnd::SetTextMargin(int text_margin)
{
    SetLayoutBorderMargin(text_margin);
}
