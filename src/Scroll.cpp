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

#include <GG/Scroll.h>

#include <GG/Button.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>

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
    m_tab_drag_offset(-1), 
    m_initial_depressed_area(SBR_NONE),
    m_depressed_area(SBR_NONE)
{
}

Scroll::Scroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior, Uint32 flags/* = CLICKABLE*/) :
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
    m_tab_drag_offset(-1), 
    m_initial_depressed_area(SBR_NONE),
    m_depressed_area(SBR_NONE)
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
}

std::pair<int, int> Scroll::PosnRange() const
{
    return std::pair<int,int>(m_posn, m_posn + m_page_sz);
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
    m_decr->OffsetMove(UpperLeft());
    m_incr->OffsetMove(UpperLeft());
    m_decr->Render();
    m_incr->Render();
    m_decr->OffsetMove(-UpperLeft());
    m_incr->OffsetMove(-UpperLeft());
    if (!Disabled()) { // hide tab if disabled
        m_tab->OffsetMove(UpperLeft());
        m_tab->Render();
        m_tab->OffsetMove(-UpperLeft());
    }
}

void Scroll::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        // when a button is pressed, record the region of the control the cursor is over
        switch (RegionUnder(pt))
        {
        case SBR_TAB:
            m_initial_depressed_area = SBR_NONE; // tab can't be "depressed"
            m_depressed_area = SBR_NONE;
            m_tab_drag_offset = m_orientation == VERTICAL ? m_tab->ScreenToWindow(pt - UpperLeft()).y : m_tab->ScreenToWindow(pt - UpperLeft()).x;
            break;
        case SBR_LINE_DN:
            m_initial_depressed_area = SBR_LINE_DN;
            m_depressed_area = SBR_LINE_DN;
            m_decr->SetState(Button::BN_PRESSED);
            ScrollLineDecr();
            break;
        case SBR_LINE_UP:
            m_initial_depressed_area = SBR_LINE_UP;
            m_depressed_area = SBR_LINE_UP;
            m_incr->SetState(Button::BN_PRESSED);
            ScrollLineIncr();
            break;
        case SBR_PAGE_DN:
            m_initial_depressed_area = SBR_PAGE_DN;
            m_depressed_area = SBR_PAGE_DN;
            ScrollPageDecr();
            break;
        case SBR_PAGE_UP:
            m_initial_depressed_area = SBR_PAGE_UP;
            m_depressed_area = SBR_PAGE_UP;
            ScrollPageIncr();
            break;
        default: break;
        }
    }
}

void Scroll::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled()) {
        // when dragging occurs, if we're dragging the tab, move it
        if (m_tab_drag_offset != -1) // if tab is being dragged
        {
            int click_pos;
            int scroll_width;   // "width" in these is not necessarily in the x direction, it means more like "extent" in horz or vert direction
            int tab_width;
            int decr_width;
            int incr_width;
            if (m_orientation == VERTICAL) {
                click_pos = ScreenToWindow(pt).y;
                scroll_width = Size().y;
                tab_width = m_tab->Size().y;
                decr_width = m_decr->Size().y;
                incr_width = m_incr->Size().y;
            } else {
                click_pos = ScreenToWindow(pt).x;
                scroll_width = Size().x;
                tab_width = m_tab->Size().x;
                decr_width = m_decr->Size().x;
                incr_width = m_incr->Size().x;
            }
            if (click_pos - m_tab_drag_offset < decr_width)
                m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, decr_width)) : m_tab->MoveTo(Pt(decr_width, 0));
            else if (click_pos - m_tab_drag_offset + tab_width > scroll_width - incr_width)
                m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, scroll_width - incr_width - tab_width)) : m_tab->MoveTo(Pt(scroll_width - incr_width - tab_width, 0));
            else
                m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, click_pos - m_tab_drag_offset)) : m_tab->MoveTo(Pt(click_pos - m_tab_drag_offset, 0));
            UpdatePosn();
        } else { // otherwise, if there is dragging going on elsewhere, mark that area as "depressed", if it is the area we started in
            switch (RegionUnder(pt)) { // figure out where cursor is now
            case SBR_LINE_DN: if (m_initial_depressed_area == SBR_LINE_DN) ScrollLineDecr(); break;
            case SBR_LINE_UP: if (m_initial_depressed_area == SBR_LINE_UP) ScrollLineIncr(); break;
            case SBR_PAGE_DN: if (m_initial_depressed_area == SBR_PAGE_DN) ScrollPageDecr(); break;
            case SBR_PAGE_UP: if (m_initial_depressed_area == SBR_PAGE_UP) ScrollPageIncr(); break;
            default: break;
            }
        }
    }
}

void Scroll::LButtonUp(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        m_decr->SetState(Button::BN_UNPRESSED);
        m_incr->SetState(Button::BN_UNPRESSED);
        m_initial_depressed_area = SBR_NONE;
        m_depressed_area = SBR_NONE;
        if (m_tab_drag_offset != -1)
            ScrolledAndStoppedSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
        m_tab_drag_offset = -1;
    }
}

void Scroll::LClick(const Pt& pt, Uint32 keys)
{
    LButtonUp(pt, keys);
}

void Scroll::MouseHere(const Pt& pt, Uint32 keys)
{
    LButtonUp(pt, keys);
}

void Scroll::MouseLeave(const Pt& pt, Uint32 keys)
{
    m_depressed_area = SBR_NONE;
}

void Scroll::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    int bn_width = (m_orientation == VERTICAL) ? Size().x : Size().y;
    m_decr->SizeMove(Pt(0, 0), Pt(bn_width, bn_width));
    m_incr->SizeMove(Size() - Pt(bn_width, bn_width), Size());
    m_tab->SizeMove(m_tab->UpperLeft(), (m_orientation == VERTICAL) ? Pt(bn_width, m_tab->LowerRight().y) :
                    Pt(m_tab->LowerRight().x, bn_width));
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
    Pt tab_ul = m_tab->UpperLeft();
    Pt tab_lr = m_orientation == VERTICAL ? Pt(m_tab->LowerRight().x, tab_ul.y + TabWidth()):
    Pt(tab_ul.x + TabWidth(), m_tab->LowerRight().y);
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
    Pt ul = UpperLeft();
    if (m_decr->InWindow(pt - ul))
        retval = SBR_LINE_DN;
    else if (m_incr->InWindow(pt - ul))
        retval = SBR_LINE_UP;
    else if (m_tab->InWindow(pt - ul))
        retval = SBR_TAB;
    else if (!InWindow(pt))
        retval = SBR_NONE;
    else if (pt.x - ul.x < m_tab->UpperLeft().x || pt.y - ul.y <= m_tab->UpperLeft().y)
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

void Scroll::UpdatePosn()
{
    int old_posn = m_posn;
    int before_tab = (m_orientation == VERTICAL ?            // the tabspace before the tab's lower-value side
                      m_tab->UpperLeft().y - m_decr->Size().y :
                      m_tab->UpperLeft().x - m_decr->Size().x );
    int tab_space = TabSpace();
    m_posn = static_cast<int>(m_range_min + static_cast<double>(before_tab) / tab_space * (m_range_max - m_range_min + 1) + 0.5);
    m_posn = std::min(m_range_max - m_page_sz + 1, std::max(m_range_min, m_posn));
    if (old_posn != m_posn) {
        ScrolledSignal(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
    }
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
                  Pt(m_tab->UpperLeft().x,
                     static_cast<int>((static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace)) :
                  Pt(static_cast<int>((static_cast<double>(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace),
                     m_tab->UpperLeft().y));
}
