/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */

#include <GG/Slider.h>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    struct SlidEcho
    {
        SlidEcho(const std::string& name) : m_name(name) {}
        void operator()(int pos, int min, int max)
            {
                std::cerr << "GG SIGNAL : " << m_name
                          << "(pos=" << pos << " min=" << min << " max=" << max << ")\n";
            }
        std::string m_name;
    };
}

const std::size_t Slider::INVALID_PAGE_SIZE = std::numeric_limits<std::size_t>::max();

Slider::Slider() :
    Control(),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_page_sz(INVALID_PAGE_SIZE),
    m_orientation(VERTICAL),
    m_line_width(5),
    m_tab_width(5),
    m_line_style(RAISED),
    m_tab_drag_offset(-1),
    m_tab(0),
    m_dragging_tab(false)
{}

Slider::Slider(X x, Y y, X w, Y h, int min, int max, Orientation orientation, SliderLineStyle style,
               Clr color, int unsigned tab_width, int unsigned line_width/* = 5*/,
               Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(x, y, w, h, flags),
    m_posn(min),
    m_range_min(min),
    m_range_max(max),
    m_page_sz(INVALID_PAGE_SIZE),
    m_orientation(orientation),
    m_line_width(line_width),
    m_tab_width(tab_width),
    m_line_style(style),
    m_tab_drag_offset(-1),
    m_tab(m_orientation == VERTICAL ?
          GetStyleFactory()->NewVSliderTabButton(X0, Y0, Width(), Y(m_tab_width), "", boost::shared_ptr<Font>(), color) :
          GetStyleFactory()->NewHSliderTabButton(X0, Y0, X(m_tab_width), Height(), "", boost::shared_ptr<Font>(), color)),
    m_dragging_tab(false)
{
    Control::SetColor(color);
    AttachChild(m_tab);
    m_tab->InstallEventFilter(this);
    SizeMove(UpperLeft(), LowerRight());

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(SlidSignal, SlidEcho("Slider::SlidSignal"));
        Connect(SlidAndStoppedSignal, SlidEcho("Slider::SlidAndStoppedSignal"));
    }
}

Pt Slider::MinUsableSize() const
{
    Pt tab_min = m_tab->MinUsableSize();
    return Pt(m_orientation == VERTICAL ? tab_min.x : 3 * tab_min.x,
              m_orientation == VERTICAL ? 3 * tab_min.y : tab_min.y);
}

int Slider::Posn() const
{ return m_posn; }

std::pair<int, int> Slider::SliderRange() const
{ return std::pair<int, int>(m_range_min, m_range_max); }

unsigned int Slider::PageSize() const
{ return m_page_sz != INVALID_PAGE_SIZE ? m_page_sz : (m_range_max - m_range_min) / 10; }

Orientation Slider::GetOrientation() const
{ return m_orientation; }

unsigned int Slider::TabWidth() const
{ return m_tab_width; }

unsigned int Slider::LineWidth() const
{ return m_line_width; }

SliderLineStyle Slider::LineStyle() const
{ return m_line_style; }

void Slider::Render()
{
    const Pt UL = UpperLeft();
    const Pt LR = LowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    int tab_width = m_orientation == VERTICAL ? Value(m_tab->Height()) : Value(m_tab->Width());
    Pt ul, lr;
    if (m_orientation == VERTICAL) {
        ul.x = ((LR.x + UL.x) - static_cast<int>(m_line_width)) / 2;
        lr.x = ul.x + static_cast<int>(m_line_width);
        ul.y = UL.y + tab_width / 2;
        lr.y = LR.y - tab_width / 2;
    } else {
        ul.x = UL.x + tab_width / 2;
        lr.x = LR.x - tab_width / 2;
        ul.y = ((LR.y + UL.y) - static_cast<int>(m_line_width)) / 2;
        lr.y = ul.y + static_cast<int>(m_line_width);
    }
    switch (m_line_style) {
    case FLAT:
        FlatRectangle(ul, lr, color_to_use, CLR_BLACK, 1);
        break;
    case RAISED:
        BeveledRectangle(ul, lr, color_to_use, color_to_use, true, m_line_width / 2);
        break;
    case GROOVED:
        BeveledRectangle(ul, lr, color_to_use, color_to_use, false, m_line_width / 2);
        break;
    }
}

void Slider::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (m_orientation == VERTICAL)
        m_tab->SizeMove(Pt(), Pt(lr.x - ul.x, Y(m_tab_width)));
    else
        m_tab->SizeMove(Pt(), Pt(X(m_tab_width), lr.y - ul.y));
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
        SlideToImpl(m_range_min, false);
    else if (m_range_max < m_posn)
        SlideToImpl(m_range_max, false);
    else
        MoveTabToPosn();
}

void Slider::SetMax(int max)
{ SizeSlider(m_range_min, max); }

void Slider::SetMin(int min)
{ SizeSlider(min, m_range_max); }

void Slider::SlideTo(int p)
{ SlideToImpl(p, false); }

void Slider::SetPageSize(unsigned int size)
{ m_page_sz = size; }

void Slider::SetLineStyle(SliderLineStyle style)
{ m_line_style = style; }

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
{ return m_tab; }

int Slider::PtToPosn(const Pt& pt) const
{
    Pt ul = UpperLeft(), lr = LowerRight();
    int line_min = 0;
    int line_max = 0;
    int pixel_nearest_to_pt_on_line = 0;
    if (m_orientation == VERTICAL) {
        line_min = Value(m_tab->Height() / 2);
        line_max = Value(Height() - (m_tab->Height() - m_tab->Height() / 2));
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(Value(pt.y - lr.y), line_max));
    } else {
        line_min = Value(m_tab->Width() / 2);
        line_max = Value(Width() - (m_tab->Width() - m_tab->Width() / 2));
        pixel_nearest_to_pt_on_line = std::max(line_min, std::min(Value(pt.x - ul.x), line_max));
    }
    double fractional_distance = static_cast<double>(pixel_nearest_to_pt_on_line) / (line_max - line_min);
    return m_range_min + static_cast<int>((m_range_max - m_range_min) * fractional_distance);
}

void Slider::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ SlideToImpl(m_posn < PtToPosn(pt) ? m_posn + PageSize() : m_posn - PageSize(), true); }

void Slider::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_HOME:
            SlideToImpl(m_range_min, true);
            break;
        case GGK_END:
            SlideToImpl(m_range_max, true);
            break;
        case GGK_UP:
            if (m_orientation != HORIZONTAL)
                SlideToImpl(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_RIGHT:
            if (m_orientation != VERTICAL)
                SlideToImpl(m_posn + (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_DOWN:
            if (m_orientation != HORIZONTAL)
                SlideToImpl(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_LEFT:
            if (m_orientation != VERTICAL)
                SlideToImpl(m_posn - (0 < (m_range_max - m_range_min) ? 1 : -1), true);
            break;
        case GGK_PLUS:
        case GGK_KP_PLUS:
            SlideToImpl(m_posn + 1, true);
            break;
        case GGK_MINUS:
        case GGK_KP_MINUS:
            SlideToImpl(m_posn - 1, true);
            break;
        default:
            Control::KeyPress(key, key_code_point, mod_keys);
            break;
        }
    } else {
        Control::KeyPress(key, key_code_point, mod_keys);
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
                    new_ul.y = std::max(Y0, std::min(new_ul.y, ClientHeight() - m_tab->Height()));
                } else {
                    new_ul.x = std::max(X0, std::min(new_ul.x, ClientWidth() - m_tab->Width()));
                    new_ul.y = m_tab->RelativeUpperLeft().y;
                }
                m_tab->MoveTo(new_ul);
                UpdatePosn();
            }
            return true;
        }
        case WndEvent::LButtonDown:
            m_dragging_tab = true;
            break;
        case WndEvent::LButtonUp:
        case WndEvent::LClick: {
            if (!Disabled())
                SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
            m_dragging_tab = false;
            break;
        }
        case WndEvent::MouseLeave:
            return m_dragging_tab;
        default:
            break;
        }
    }
    return false;
}

void Slider::MoveTabToPosn()
{
    assert(m_range_min <= m_posn && m_posn <= m_range_max ||
           m_range_max <= m_posn && m_posn <= m_range_min);
    double fractional_distance = static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min);
    int tab_width = m_orientation == VERTICAL ? Value(m_tab->Height()) : Value(m_tab->Width());
    int line_length = (m_orientation == VERTICAL ? Value(Height()) : Value(Width())) - tab_width;
    int pixel_distance = static_cast<int>(line_length * fractional_distance);
    if (m_orientation == VERTICAL)
        m_tab->MoveTo(Pt(m_tab->RelativeUpperLeft().x, Height() - tab_width - pixel_distance));
    else
        m_tab->MoveTo(Pt(X(pixel_distance), m_tab->RelativeUpperLeft().y));
}

void Slider::UpdatePosn()
{
    int old_posn = m_posn;
    int line_length = m_orientation == VERTICAL ? Value(Height() - m_tab->Height()) : Value(Width() - m_tab->Width());
    int tab_posn = (m_orientation == VERTICAL ? Value(Height() - m_tab->RelativeLowerRight().y) : Value(m_tab->RelativeUpperLeft().x));
    double fractional_distance = static_cast<double>(tab_posn) / line_length;
    m_posn = m_range_min + static_cast<int>((m_range_max - m_range_min) * fractional_distance);
    if (m_posn != old_posn)
        SlidSignal(m_posn, m_range_min, m_range_max);
}

void Slider::SlideToImpl(int p, bool signal)
{
    int old_posn = m_posn;
    if (0 < (m_range_max - m_range_min) ? p < m_range_min : p > m_range_min)
        m_posn = m_range_min;
    else if (0 < (m_range_max - m_range_min) ? m_range_max < p : m_range_max > p)
        m_posn = m_range_max;
    else
        m_posn = p;
    MoveTabToPosn();
    if (signal && m_posn != old_posn) {
        SlidSignal(m_posn, m_range_min, m_range_max);
        SlidAndStoppedSignal(m_posn, m_range_min, m_range_max);
    }
}
