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
#include <GGWndEditor.h>

using namespace GG;

Slider::Slider() :
    Control(),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_orientation(VERTICAL),
    m_line_width(5),
    m_tab_width(5),
    m_line_style(RAISED),
    m_tab_drag_offset(-1)
{
}

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
                     m_orientation == VERTICAL ? m_tab_width : Height(), "", boost::shared_ptr<Font>(), color))
{
    SetColor(color);
    SizeMove(UpperLeft(), LowerRight());
}

Slider::Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, LineStyleType style, Clr color,
               Button* tab, int line_width/* = 5*/, Uint32 flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(m_orientation == VERTICAL ? tab->Width() : tab->Height()),
    m_line_style(style),
    m_tab_drag_offset(-1),
    m_tab(tab)
{
    SetColor(color);
    SizeMove(UpperLeft(), LowerRight());
}

int Slider::Posn() const
{
    return m_posn;
}

std::pair<int, int> Slider::SliderRange() const
{
    return std::pair<int,int>(m_range_min, m_range_max);
}

Slider::Orientation Slider::GetOrientation() const
{
    return m_orientation;
}

int Slider::TabWidth() const
{
    return m_tab_width;
}

int Slider::LineWidth() const
{
    return m_line_width;
}

Slider::LineStyleType Slider::LineStyle() const
{
    return m_line_style;
}

void Slider::Render()
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
            m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, 0)) : m_tab->MoveTo(Pt(0, 0));
        else if (click_pos - m_tab_drag_offset + tab_width > slide_width)
            m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, slide_width - tab_width)) : m_tab->MoveTo(Pt(slide_width - tab_width, 0));
        else
            m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, click_pos - m_tab_drag_offset)) : m_tab->MoveTo(Pt(click_pos - m_tab_drag_offset, 0));
        UpdatePosn();
    }
}

void Slider::LButtonUp(const Pt& pt, Uint32 keys)
{
    if (!Disabled() && m_tab_drag_offset != -1)
        SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
    m_tab_drag_offset = -1;
}

void Slider::LClick(const Pt& pt, Uint32 keys)
{
    LButtonUp(pt, keys);
}

void Slider::MouseHere(const Pt& pt, Uint32 keys)
{
    m_tab_drag_offset = -1;
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

void Slider::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (m_orientation == VERTICAL)
        m_tab->SizeMove(Pt(0, 0), Pt(lr.x - ul.x, m_tab_width));
    else
        m_tab->SizeMove(Pt(0, 0), Pt(m_tab_width, lr.y - ul.y));
    MoveTabToPosn();
}

void Slider::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_tab->Disable(b);
}

void Slider::SizeSlider(int min, int max)
{
    assert(m_range_min != m_range_max);
    m_range_min = min;
    m_range_max = max;
    if (m_posn < m_range_min)
        SlideTo(m_range_min);
    else if (m_range_max < m_posn)
        SlideTo(m_range_max);
    else
        MoveTabToPosn();
}

void Slider::SetMax(int max)
{
    SizeSlider(m_range_min, max);
}

void Slider::SetMin(int min)
{
    SizeSlider(min, m_range_max);
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
    if (m_posn != old_posn) {
        SlidSignal(m_posn, m_range_min, m_range_max);
        SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
    }
}

void Slider::SetLineStyle(LineStyleType style)
{
    m_line_style = style;
}

void Slider::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("Slider");
    editor->Attribute("Position", m_posn);
    editor->Attribute("Range Min", m_range_min);
    editor->Attribute("Range Max", m_range_max);
    editor->Attribute("Line Width", m_line_width);
    editor->Attribute("Tab Width", m_tab_width);
    editor->Attribute("Line Style", m_line_style,
                      FLAT, GROOVED);
}

int Slider::TabDragOffset() const
{
    return m_tab_drag_offset;
}

const boost::shared_ptr<Button>& Slider::Tab() const
{
    return m_tab;
}

void Slider::MoveTabToPosn()
{
    double fractional_distance = static_cast<double>(std::abs(m_posn - m_range_min)) / (std::abs(m_range_max - m_range_min));
    int line_length = (m_orientation == VERTICAL ? Height() : Width()) - m_tab_width;
    int pixel_distance = static_cast<int>(line_length * fractional_distance);
    if (m_orientation == VERTICAL)
        m_tab->MoveTo(Pt(m_tab->UpperLeft().x, Height() - m_tab_width - pixel_distance));
    else
        m_tab->MoveTo(Pt(pixel_distance, m_tab->UpperLeft().y));
}

void Slider::UpdatePosn()
{
    int old_posn = m_posn;
    int line_length = (m_orientation == VERTICAL ? Height() : Width()) - m_tab_width;
    int tab_posn = (m_orientation == VERTICAL ? Height() - m_tab->LowerRight().y : m_tab->UpperLeft().x);
    double fractional_distance = static_cast<double>(tab_posn) / line_length;
    m_posn = static_cast<int>((m_range_max - m_range_min) * fractional_distance) + m_range_min;
    if (m_posn != old_posn)
        SlidSignal(m_posn, m_range_min, m_range_max);
}
