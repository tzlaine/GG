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

#include "GGSlider.h"

#include <GGApp.h>
#include <GGButton.h>
#include <GGDrawUtil.h>
#include <XMLValidators.h>

namespace GG {

Slider::Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, LineStyleType style, Clr color,
               int tab_width, int line_width/* = 5*/, Uint32 flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(tab_width),
    m_line_style(style),
    m_tab_drag_offset(-1),
    m_tab(new Button(0, 0, m_orientation == VERTICAL ? Width() : m_tab_width,
		     m_orientation == VERTICAL ? m_tab_width : Height(), "", "", 0, color))
{
    SetColor(color);
    SizeMove(UpperLeft(), LowerRight());
}

Slider::Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, LineStyleType style, Clr color,
               Button* tab/* = 0*/, int line_width/* = 5*/, Uint32 flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(-1),
    m_line_style(style),
    m_tab_drag_offset(-1),
    m_tab(tab)
{
    SetColor(color);
    if (!m_tab)
        m_tab.reset(new Button(0, 0, m_orientation == VERTICAL ? Width() : m_tab_width,
                               m_orientation == VERTICAL ? m_tab_width : Height(), "", "", 0, color));
    SizeMove(UpperLeft(), LowerRight());
}

Slider::Slider(const XMLElement& elem) :
    Control(elem.Child("GG::Control")),
    m_tab_drag_offset(-1)
{
    if (elem.Tag() != "GG::Slider")
        throw std::invalid_argument("Attempted to construct a GG::Slider from an XMLElement that had a tag other than \"GG::Slider\"");

    m_posn = lexical_cast<int>(elem.Child("m_posn").Text());
    m_range_min = lexical_cast<int>(elem.Child("m_range_min").Text());
    m_range_max = lexical_cast<int>(elem.Child("m_range_max").Text());
    m_orientation = lexical_cast<Orientation>(elem.Child("m_orientation").Text());
    m_line_width = lexical_cast<int>(elem.Child("m_line_width").Text());
    m_tab_width = lexical_cast<int>(elem.Child("m_tab_width").Text());
    m_line_style = lexical_cast<LineStyleType>(elem.Child("m_line_style").Text());

    if (Button* b = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(elem.Child("m_tab").Child(0))))
        m_tab.reset(b);
    else
        throw std::runtime_error("Slider::Slider : Attempted to use a non-Button object as the tab for a GG::Slider.");

    SizeMove(UpperLeft(), LowerRight());
}

XMLElement Slider::XMLEncode() const
{
    XMLElement retval("GG::Slider");
    retval.AppendChild(Control::XMLEncode());
    retval.AppendChild(XMLElement("m_posn", lexical_cast<string>(m_posn)));
    retval.AppendChild(XMLElement("m_range_min", lexical_cast<string>(m_range_min)));
    retval.AppendChild(XMLElement("m_range_max", lexical_cast<string>(m_range_max)));
    retval.AppendChild(XMLElement("m_orientation", lexical_cast<string>(m_orientation)));
    retval.AppendChild(XMLElement("m_line_width", lexical_cast<string>(m_line_width)));
    retval.AppendChild(XMLElement("m_tab_width", lexical_cast<string>(m_tab_width)));
    retval.AppendChild(XMLElement("m_line_style", lexical_cast<string>(m_line_style)));
    retval.AppendChild(XMLElement("m_tab", m_tab->XMLEncode()));
    return retval;
}

XMLElementValidator Slider::XMLValidator() const
{
    XMLElementValidator retval("GG::Slider");
    retval.AppendChild(Control::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_posn", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_range_min", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_range_max", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_orientation", new MappedEnumValidator<Orientation>()));
    retval.AppendChild(XMLElementValidator("m_line_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_tab_width", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_line_style", new MappedEnumValidator<LineStyleType>()));
    retval.AppendChild(XMLElementValidator("m_tab", m_tab->XMLValidator()));
    return retval;
}

bool Slider::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    int tab_width = m_orientation == VERTICAL ? m_tab->Height() : m_tab->Width();
    int x_start, x_end, y_start, y_end;
    if (m_orientation == VERTICAL) {
        x_start = ((lr.x + ul.x) - m_line_width) / 2;
        x_end   = x_start + m_line_width;
        y_start = ul.y + tab_width / 2;
        y_end   = lr.y - tab_width / 2;
    } else {
        x_start = ul.x + tab_width / 2;
        x_end   = lr.x - tab_width / 2;
        y_start = ((lr.y + ul.y) - m_line_width) / 2;
        y_end   = y_start + m_line_width;
    }
    switch (m_line_style) {
    case FLAT:
        FlatRectangle(x_start, y_start, x_end, y_end, color_to_use, CLR_BLACK, 1);
        break;
    case RAISED:
        BeveledRectangle(x_start, y_start, x_end, y_end, color_to_use, color_to_use, true, m_line_width / 2);
        break;
    case GROOVED:
        BeveledRectangle(x_start, y_start, x_end, y_end, color_to_use, color_to_use, false, m_line_width / 2);
        break;
    }
    m_tab->OffsetMove(UpperLeft());
    m_tab->Render();
    m_tab->OffsetMove(-UpperLeft());
    return true;
}

void Slider::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        Pt ul = UpperLeft();
        m_tab_drag_offset = -1;   // until we know the tab is being dragged, clear the offset
        if (m_tab->InWindow(pt - ul))
            m_tab_drag_offset = m_orientation == VERTICAL ? m_tab->ScreenToWindow(pt - ul).y : m_tab->ScreenToWindow(pt - ul).x;
    }
}

void Slider::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled() && m_tab_drag_offset != -1) // if tab is being dragged
    {
        int click_pos;
        int slide_width;   // "width" in these is not necessarily in the x direction, it means more like "extent" in horiz or vert direction
        int tab_width;
        if (m_orientation == VERTICAL) {
            click_pos = ScreenToWindow(pt).y;
            slide_width = Size().y;
            tab_width = m_tab->Size().y;
        } else {
            click_pos = ScreenToWindow(pt).x;
            slide_width = Size().x;
            tab_width = m_tab->Size().x;
        }
        if (click_pos - m_tab_drag_offset < 0)
            m_orientation == VERTICAL ? m_tab->MoveTo(0, 0) : m_tab->MoveTo(0, 0);
        else if (click_pos - m_tab_drag_offset + tab_width > slide_width)
            m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, slide_width - tab_width)) : m_tab->MoveTo(Pt(slide_width - tab_width, 0));
        else
            m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, click_pos - m_tab_drag_offset)) : m_tab->MoveTo(Pt(click_pos - m_tab_drag_offset, 0));
        UpdatePosn();
    }
}

void Slider::Keypress(Key key, Uint32 key_mods)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_HOME:
            SlideTo(m_range_min);
            break;
        case GGK_END:
            SlideTo(m_range_max);
            break;
        case GGK_UP:
            if (m_orientation != HORIZONTAL)
                SlideTo(m_posn + ((m_range_max - m_range_min) > 0 ? 1 : -1));
            break;
        case GGK_RIGHT:
            if (m_orientation != VERTICAL)
                SlideTo(m_posn + ((m_range_max - m_range_min) > 0 ? 1 : -1));
            break;
        case GGK_DOWN:
            if (m_orientation != HORIZONTAL)
                SlideTo(m_posn - ((m_range_max - m_range_min) > 0 ? 1 : -1));
            break;
        case GGK_LEFT:
            if (m_orientation != VERTICAL)
                SlideTo(m_posn - ((m_range_max - m_range_min) > 0 ? 1 : -1));
            break;
        case GGK_PLUS:
        case GGK_KP_PLUS:
            SlideTo(m_posn + 1);
            break;
        case GGK_MINUS:
        case GGK_KP_MINUS:
            SlideTo(m_posn - 1);
            break;
        default:
            if (Parent())
                Parent()->Keypress(key, key_mods);
            break;
        }
    } else {
        if (Parent())
            Parent()->Keypress(key, key_mods);
    }
}

void Slider::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    if (m_orientation == VERTICAL)
        m_tab->SizeMove(0, 0, x2 - x1, m_tab_width);
    else
        m_tab->SizeMove(0, 0, m_tab_width, y2 - y1);
    MoveTabToPosn();
}

void Slider::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_tab->Disable(b);
}

void Slider::SizeSlider(int min, int max)
{
    m_range_min = min;
    m_range_max = max;
    if (m_posn < m_range_min)
        SlideTo(m_range_min);
    else if (m_range_max < m_posn)
        SlideTo(m_range_max);
    else
        MoveTabToPosn();
}

void Slider::SlideTo(int p)
{
    int old_posn = m_posn;
    if ((m_range_max - m_range_min) > 0 ? p < m_range_min : p > m_range_min)
        m_posn = m_range_min;
    else if ((m_range_max - m_range_min) > 0 ? m_range_max < p : m_range_max > p)
        m_posn = m_range_max;
    else
        m_posn = p;
    MoveTabToPosn();
    if (m_posn != old_posn)
        m_slid_sig(m_posn, m_range_min, m_range_max);
}

void Slider::MoveTabToPosn()
{
    double fractional_distance = static_cast<double>(std::abs(m_posn - m_range_min)) / (std::abs(m_range_max - m_range_min));
    int line_length = (m_orientation == VERTICAL ? Height() : Width()) - m_tab_width;
    int pixel_distance = static_cast<int>(line_length * fractional_distance);
    if (m_orientation == VERTICAL)
        m_tab->MoveTo(m_tab->UpperLeft().x, Height() - m_tab_width - pixel_distance);
    else
        m_tab->MoveTo(pixel_distance, m_tab->UpperLeft().y);
}

void Slider::UpdatePosn()
{
    int old_posn = m_posn;
    int line_length = (m_orientation == VERTICAL ? Height() : Width()) - m_tab_width;
    int tab_posn = (m_orientation == VERTICAL ? Height() - m_tab->LowerRight().y : m_tab->UpperLeft().x);
    double fractional_distance = static_cast<double>(tab_posn) / line_length;
    m_posn = static_cast<int>((m_range_max - m_range_min) * fractional_distance) + m_range_min;
    if (m_posn != old_posn)
        m_slid_sig(m_posn, m_range_min, m_range_max);
}

} // namespace GG

