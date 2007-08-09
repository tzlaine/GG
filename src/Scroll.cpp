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

#include <GG/Scroll.h>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    const int MIN_TAB_SIZE = 5;
}

////////////////////////////////////////////////
// GG::Scroll
////////////////////////////////////////////////
Scroll::Scroll() :
    Control(),
    m_orientation(VERTICAL),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_line_sz(5),
    m_page_sz(25),
    m_tab(0),
    m_incr(0),
    m_decr(0),
    m_initial_depressed_region(SBR_NONE),
    m_depressed_region(SBR_NONE)
{}

Scroll::Scroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior, Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) :
    Control(x, y, w, h, flags),
    m_int_color(interior),
    m_orientation(orientation),
    m_posn(0),
    m_range_min(0),
    m_range_max(99),
    m_line_sz(5),
    m_page_sz(25),
    m_tab(0),
    m_incr(0),
    m_decr(0),
    m_initial_depressed_region(SBR_NONE),
    m_depressed_region(SBR_NONE)
{
    Control::SetColor(color);
    boost::shared_ptr<Font> null_font;
    boost::shared_ptr<StyleFactory> style = GetStyleFactory();
    if (m_orientation == VERTICAL) {
        m_decr = style->NewScrollUpButton(   0,     0, w, w,          "", null_font, color);
        m_incr = style->NewScrollDownButton( 0, h - w, w, w,          "", null_font, color);
        m_tab  = style->NewVScrollTabButton( 0,     w, w, TabWidth(), "", null_font, color);
    } else {
        m_decr = style->NewScrollLeftButton( 0,     0, h,          h, "", null_font, color);
        m_incr = style->NewScrollRightButton(w - h, 0, h,          h, "", null_font, color);
        m_tab  = style->NewHScrollTabButton( h,     0, TabWidth(), h, "", null_font, color);
    }
    AttachChild(m_decr);
    AttachChild(m_incr);
    AttachChild(m_tab);
    Connect(m_decr->ClickedSignal, &Scroll::ScrollLineDecr, this);
    Connect(m_incr->ClickedSignal, &Scroll::ScrollLineIncr, this);
    m_tab->InstallEventFilter(this);
}

Pt Scroll::MinUsableSize() const
{
    Pt retval;
    const int MIN_DRAGABLE_SIZE = 2;
    if (m_orientation == VERTICAL) {
        retval.x = MIN_DRAGABLE_SIZE;
        int decr_y = m_decr->MinUsableSize().y;
        int incr_y = m_incr->MinUsableSize().y;
        retval.y = decr_y + incr_y + 3 * std::min(decr_y, incr_y);
    } else {
        int decr_x = m_decr->MinUsableSize().x;
        int incr_x = m_incr->MinUsableSize().x;
        retval.x = decr_x + incr_x + 3 * std::min(decr_x, incr_x);
        retval.y = MIN_DRAGABLE_SIZE;
    }
    return retval;
}

std::pair<int, int> Scroll::PosnRange() const
{
    return std::pair<int, int>(m_posn, m_posn + m_page_sz);
}

std::pair<int, int> Scroll::ScrollRange() const
{
    return std::pair<int, int>(m_range_min, m_range_max);
}

int Scroll::LineSize() const
{
    return m_line_sz;
}

int Scroll::PageSize() const
{
    return m_page_sz;
}
    
Clr Scroll::InteriorColor() const
{
    return m_int_color;
}

Orientation Scroll::ScrollOrientation() const
{
    return m_orientation;
}

void Scroll::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, CLR_ZERO, 0);
}

void Scroll::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        // when a button is pressed, record the region of the control the cursor is over
        ScrollRegion region = RegionUnder(pt);
        if (m_initial_depressed_region == SBR_NONE)
            m_initial_depressed_region = region;
        m_depressed_region = region;
        if (m_depressed_region == m_initial_depressed_region) {
            switch (m_depressed_region)
            {
            case SBR_PAGE_DN:
                ScrollPageDecr();
                break;
            case SBR_PAGE_UP:
                ScrollPageIncr();
                break;
            default: break;
            }
        }
    }
}

void Scroll::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_decr->SetState(Button::BN_UNPRESSED);
        m_incr->SetState(Button::BN_UNPRESSED);
        m_initial_depressed_region = SBR_NONE;
        m_depressed_region = SBR_NONE;
    }
}

void Scroll::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    LButtonUp(pt, mod_keys);
}

void Scroll::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    LButtonUp(pt, mod_keys);
}

void Scroll::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    int bn_width = (m_orientation == VERTICAL) ? Size().x : Size().y;
    m_decr->SizeMove(Pt(0, 0), Pt(bn_width, bn_width));
    m_incr->SizeMove(Size() - Pt(bn_width, bn_width), Size());
    m_tab->SizeMove(m_tab->RelativeUpperLeft(), (m_orientation == VERTICAL) ? Pt(bn_width, m_tab->RelativeLowerRight().y) :
                    Pt(m_tab->RelativeLowerRight().x, bn_width));
    SizeScroll(m_range_min, m_range_max, m_line_sz, m_page_sz); // update tab size and position
}

void Scroll::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    m_tab->Disable(b);
    m_incr->Disable(b);
    m_decr->Disable(b);
}

void Scroll::SetColor(Clr c)
{
    Control::SetColor(c);
    m_tab->SetColor(c);
    m_incr->SetColor(c);
    m_decr->SetColor(c);
}

void Scroll::SetInteriorColor(Clr c)
{
    m_int_color = c;
}

void Scroll::SizeScroll(int min, int max, int line, int page)
{
    m_line_sz = line;
    m_range_min = std::min(min, max);
    m_range_max = std::max(min, max);
    int old_posn = m_posn;
    m_page_sz = page;
    if (m_page_sz > (m_range_max - m_range_min + 1)) m_page_sz = (m_range_max - m_range_min + 1);
    if (m_posn > m_range_max - (m_page_sz - 1)) m_posn = m_range_max - (m_page_sz - 1);
    if (m_posn < m_range_min) m_posn = m_range_min;
    Pt tab_ul = m_tab->RelativeUpperLeft();
    Pt tab_lr = m_orientation == VERTICAL ? Pt(m_tab->RelativeLowerRight().x, tab_ul.y + TabWidth()):
    Pt(tab_ul.x + TabWidth(), m_tab->RelativeLowerRight().y);
    m_tab->SizeMove(tab_ul, tab_lr);
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::SetMax(int max)        
{
    SizeScroll(m_range_min, max, m_line_sz, m_page_sz);
}

void Scroll::SetMin(int min)        
{
    SizeScroll(min, m_range_max, m_line_sz, m_page_sz);
}

void Scroll::SetLineSize(int line)
{
    SizeScroll(m_range_min, m_range_max, line, m_page_sz);
}

void Scroll::SetPageSize(int page)  
{
    SizeScroll(m_range_min, m_range_max, m_line_sz, page);
}

void Scroll::ScrollTo(int p)
{
    int old_posn = m_posn;
    if (p < m_range_min)
        m_posn = m_range_min;
    else if (p > m_range_max - m_page_sz)
        m_posn = m_range_max - m_page_sz;
    else
        m_posn = p;
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::ScrollLineIncr()
{
    int old_posn = m_posn;
    if (m_posn + m_line_sz <= m_range_max - m_page_sz)
        m_posn += m_line_sz;
    else
        m_posn = m_range_max - (m_page_sz - 1);
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::ScrollLineDecr()
{
    int old_posn = m_posn;
    if (m_posn - m_line_sz >= m_range_min)
        m_posn -= m_line_sz;
    else
        m_posn = m_range_min;
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::ScrollPageIncr()
{
    int old_posn = m_posn;
    if (m_posn + m_page_sz <= m_range_max - m_page_sz)
        m_posn += m_page_sz;
    else
        m_posn = m_range_max - (m_page_sz - 1);
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::ScrollPageDecr()
{
    int old_posn = m_posn;
    if (m_posn - m_page_sz >= m_range_min)
        m_posn -= m_page_sz;
    else
        m_posn = m_range_min;
    MoveTabToPosn();
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
}

void Scroll::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("Scroll");
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Range Min", m_range_min);
    editor->Attribute("Range Max", m_range_max);
    editor->Attribute("Line Size", m_line_sz);
    editor->Attribute("Page Size", m_page_sz);
}

int Scroll::TabSpace() const
{
    // tab_space is the space the tab has to move about in (the control's width less the width of the incr & decr buttons)
    return (m_orientation == VERTICAL ?
            Size().y - m_incr->Size().y - m_decr->Size().y :
            Size().x - m_incr->Size().x - m_decr->Size().x);
}

int Scroll::TabWidth() const
{
    return std::max(TabSpace() * m_page_sz / (m_range_max - m_range_min + 1), MIN_TAB_SIZE);
}

Scroll::ScrollRegion Scroll::RegionUnder(const Pt& pt)
{
    ScrollRegion retval;
    Pt ul = ClientUpperLeft();
    if (pt.x - ul.x < m_tab->RelativeUpperLeft().x || pt.y - ul.y <= m_tab->RelativeUpperLeft().y)
        retval = SBR_PAGE_DN;
    else
        retval = SBR_PAGE_UP;
    return retval;
}

Button* Scroll::TabButton() const
{
    return m_tab;
}

Button* Scroll::IncrButton() const
{
    return m_incr;
}

Button* Scroll::DecrButton() const
{
    return m_decr;
}

bool Scroll::EventFilter(Wnd* w, const WndEvent& event)
{
    if (w == m_tab) {
        switch (event.Type()) {
        case WndEvent::LDrag: {
            if (!Disabled()) {
                Pt new_ul = m_tab->RelativeUpperLeft() + event.DragMove();
                if (m_orientation == VERTICAL) {
                    new_ul.x = m_tab->RelativeUpperLeft().x;
                    new_ul.y = std::max(0 + m_decr->Height(),
                                        std::min(new_ul.y, ClientHeight() - m_incr->Height() - m_tab->Height()));
                } else {
                    new_ul.x = std::max(0 + m_decr->Width(),
                                        std::min(new_ul.x, ClientWidth() - m_incr->Width() - m_tab->Width()));
                    new_ul.y = m_tab->RelativeUpperLeft().y;
                }
                m_tab->MoveTo(new_ul);
                UpdatePosn();
            }
            return true;
        }
        default:
            break;
        }
    }
    return false;
}

void Scroll::UpdatePosn()
{
    int old_posn = m_posn;
    int before_tab = (m_orientation == VERTICAL ?   // the tabspace before the tab's lower-value side
                      m_tab->RelativeUpperLeft().y - m_decr->Size().y :
                      m_tab->RelativeUpperLeft().x - m_decr->Size().x );
    int tab_space = TabSpace();
    m_posn = static_cast<int>(m_range_min + static_cast<double>(before_tab) / tab_space * (m_range_max - m_range_min + 1) + 0.5);
    m_posn = std::min(m_range_max - m_page_sz + 1, std::max(m_range_min, m_posn));
    if (old_posn != m_posn)
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::MoveTabToPosn()
{
    int start_tabspace = (m_orientation==VERTICAL ?      // what is the tab's lowest posible extent?
                          m_decr->Size().y :
                          m_decr->Size().x);
    int end_tabspace = (m_orientation==VERTICAL ?      // what is its highest?
                        Size().y - m_incr->Size().y :
                        Size().x - m_incr->Size().x);

    m_tab->MoveTo(m_orientation==VERTICAL ?
                  Pt(m_tab->RelativeUpperLeft().x,
                     static_cast<int>((static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace)) :
                  Pt(static_cast<int>((static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace),
                     m_tab->RelativeUpperLeft().y));
}
