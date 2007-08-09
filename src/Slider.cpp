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

#include <GG/Slider.h>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>


using namespace GG;

Slider::Slider() :
    Control(),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_page_sz(-1),
    m_orientation(VERTICAL),
    m_line_width(5),
    m_tab_width(5),
    m_line_style(RAISED),
    m_tab_drag_offset(-1),
    m_tab(0)
{}

Slider::Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, SliderLineStyle style, Clr color,
               int tab_width, int line_width/* = 5*/, Flags<WndFlag> flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_page_sz(-1),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(tab_width),
    m_line_style(style),
    m_tab_drag_offset(-1),
    m_tab(m_orientation == VERTICAL ?
          GetStyleFactory()->NewVSliderTabButton(0, 0, Width(), m_tab_width, "", boost::shared_ptr<Font>(), color) :
          GetStyleFactory()->NewHSliderTabButton(0, 0, m_tab_width, Height(), "", boost::shared_ptr<Font>(), color))
{
    Control::SetColor(color);
    AttachChild(m_tab);
    m_tab->InstallEventFilter(this);
    SizeMove(UpperLeft(), LowerRight());
}

Pt Slider::MinUsableSize() const
{
    Pt tab_min = m_tab->MinUsableSize();
    return Pt(m_orientation == VERTICAL ? tab_min.x : 3 * tab_min.x,
              m_orientation == VERTICAL ? 3 * tab_min.y : tab_min.y);
}

int Slider::Posn() const
{
    return m_posn;
}

std::pair<int, int> Slider::SliderRange() const
{
    return std::pair<int, int>(m_range_min, m_range_max);
}

int Slider::PageSize() const
{
    return 0 <= m_page_sz ? m_page_sz : (m_range_max - m_range_min) / 10;
}

Orientation Slider::GetOrientation() const
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

SliderLineStyle Slider::LineStyle() const
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
}

bool Slider::EventFilter(Wnd* w, const WndEvent& event)
{
    if (w == m_tab) {
        switch (event.Type()) {
        case WndEvent::LDrag: {
            if (!Disabled()) {
                Pt new_ul = m_tab->RelativeUpperLeft() + event.DragMove();
                if (m_orientation == VERTICAL) {
                    new_ul.x = m_tab->RelativeUpperLeft().x;
                    new_ul.y = std::max(0, std::min(new_ul.y, ClientHeight() - m_tab->Height()));
                } else {
                    new_ul.x = std::max(0, std::min(new_ul.x, ClientWidth() - m_tab->Width()));
                    new_ul.y = m_tab->RelativeUpperLeft().y;
                }
                m_tab->MoveTo(new_ul);
                UpdatePosn();
            }
            return true;
        }
        case WndEvent::LButtonUp:
        case WndEvent::LClick: {
            if (!Disabled())
                SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
            break;
        }
        default:
            break;
        }
    }
    return false;
}

void Slider::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    SlideTo(m_posn + (m_posn < PtToPosn(pt) ? PageSize() : -PageSize()));
}

void Slider::KeyPress(Key key, Flags<ModKey> mod_keys)
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
                SlideTo(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1));
            break;
        case GGK_RIGHT:
            if (m_orientation != VERTICAL)
                SlideTo(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1));
            break;
        case GGK_DOWN:
            if (m_orientation != HORIZONTAL)
                SlideTo(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1));
            break;
        case GGK_LEFT:
            if (m_orientation != VERTICAL)
                SlideTo(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1));
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
            Control::KeyPress(key, mod_keys);
            break;
        }
    } else {
        Control::KeyPress(key, mod_keys);
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

void Slider::SetColor(Clr c)
{
    Control::SetColor(c);
    m_tab->SetColor(c);
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
    if (0 < (m_range_max - m_range_min) ? p < m_range_min : p > m_range_min)
        m_posn = m_range_min;
    else if (0 < (m_range_max - m_range_min) ? m_range_max < p : m_range_max > p)
        m_posn = m_range_max;
    else
        m_posn = p;
    MoveTabToPosn();
    if (m_posn != old_posn) {
        SlidSignal(m_posn, m_range_min, m_range_max);
        SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
    }
}

void Slider::SetPageSize(int size)
{
    m_page_sz = size;
}

void Slider::SetLineStyle(SliderLineStyle style)
{
    m_line_style = style;
}

void Slider::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("Slider");
    editor->Attribute("Range Min", m_range_min);
    editor->Attribute("Range Max", m_range_max);
    editor->Attribute("Line Width", m_line_width);
    editor->Attribute("Tab Width", m_tab_width);
    editor->Attribute("Line Style", m_line_style,
                      FLAT, GROOVED);
}

Button* Slider::Tab() const
{
    return m_tab;
}

int Slider::PtToPosn(const Pt& pt) const
{
    Pt ul = UpperLeft(), lr = LowerRight();
    int line_min = 0;
    int line_max = 0;
    int pixel_nearest_to_pt_on_line = 0;
    if (m_orientation == VERTICAL) {
        line_min = m_tab->Height() / 2;
        line_max = Height() - (m_tab->Height() - m_tab->Height() / 2);
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(pt.y - lr.y, line_max));
    } else {
        line_min = m_tab->Width() / 2;
        line_max = Width() - (m_tab->Width() - m_tab->Width() / 2);
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(pt.x - ul.x, line_max));
    }
    double fractional_distance = static_cast<double>(pixel_nearest_to_pt_on_line) / (line_max - line_min);
    return m_range_min + static_cast<int>((m_range_max - m_range_min) * fractional_distance);
}

void Slider::MoveTabToPosn()
{
    assert(m_range_min <= m_posn && m_posn <= m_range_max ||
           m_range_max <= m_posn && m_posn <= m_range_min);
    double fractional_distance = static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min);
    int tab_width = m_orientation == VERTICAL ? m_tab->Height() : m_tab->Width();
    int line_length = (m_orientation == VERTICAL ? Height() : Width()) - tab_width;
    int pixel_distance = static_cast<int>(line_length * fractional_distance);
    if (m_orientation == VERTICAL)
        m_tab->MoveTo(Pt(m_tab->RelativeUpperLeft().x, Height() - tab_width - pixel_distance));
    else
        m_tab->MoveTo(Pt(pixel_distance, m_tab->RelativeUpperLeft().y));
}

void Slider::UpdatePosn()
{
    int old_posn = m_posn;
    int line_length = m_orientation == VERTICAL ? Height() - m_tab->Height() : Width() - m_tab->Width();
    int tab_posn = (m_orientation == VERTICAL ? Height() - m_tab->RelativeLowerRight().y : m_tab->RelativeUpperLeft().x);
    double fractional_distance = static_cast<double>(tab_posn) / line_length;
    m_posn = m_range_min + static_cast<int>((m_range_max - m_range_min) * fractional_distance);
    if (m_posn != old_posn)
        SlidSignal(m_posn, m_range_min, m_range_max);
}
