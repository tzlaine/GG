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

#include "GGScroll.h"
#include "GGDrawUtil.h"
#include "GGButton.h"
#include "GGApp.h"

namespace GG {

namespace {
const int MIN_TAB_SIZE = 5;
}

////////////////////////////////////////////////
// GG::Scroll
////////////////////////////////////////////////
Scroll::Scroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior, 
               Button* decr/* = 0*/, Button* incr/* = 0*/, Button* tab/* = 0*/, Uint32 flags/* = CLICKABLE*/) : 
    Control(x, y, w, h, flags), 
    m_int_color(interior), 
    m_orientation(orientation), 
    m_posn(0), 
    m_range_min(0), 
    m_range_max(99), 
    m_line_sz(5), 
    m_page_sz(25), 
    m_tab(shared_ptr<Button>(tab)), 
    m_incr(shared_ptr<Button>(incr)), 
    m_decr(shared_ptr<Button>(decr)), 
    m_tab_drag_offset(-1)
{
    SetColor(color);
    if (m_orientation == VERTICAL) {
	if (!m_decr) m_decr = shared_ptr<Button>(new Button(0,     0, w, w,          "", "", 0, color));
	if (!m_incr) m_incr = shared_ptr<Button>(new Button(0, h - w, w, w,          "", "", 0, color));
	if (!m_tab)  m_tab  = shared_ptr<Button>(new Button(0,     w, w, TabWidth(), "", "", 0, color));
    } else {
	if (!m_decr) m_decr = shared_ptr<Button>(new Button(0,     0, h,          h, "", "", 0, color));
	if (!m_incr) m_incr = shared_ptr<Button>(new Button(w - h, 0, h,          h, "", "", 0, color));
	if (!m_tab)  m_tab  = shared_ptr<Button>(new Button(h,     0, TabWidth(), h, "", "", 0, color));
    }
}

Scroll::Scroll(const XMLElement& elem) : 
    Control(elem.Child("GG::Control")), 
    m_orientation(Orientation(lexical_cast<int>(elem.Child("m_orientation").Attribute("value"))))
{
    if (elem.Tag() != "GG::Scroll")
	throw std::invalid_argument("Attempted to construct a GG::Scroll from an XMLElement that had a tag other than \"GG::Scroll\"");
   
    const XMLElement* curr_elem = &elem.Child("m_int_color");
    m_int_color = Clr(curr_elem->Child("GG::Clr"));
   
    curr_elem = &elem.Child("m_posn");
    m_posn = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("m_range_min");
    m_range_min = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("m_range_max");
    m_range_max = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("m_line_sz");
    m_line_sz = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("m_page_sz");
    m_page_sz = lexical_cast<int>(curr_elem->Attribute("value"));
   
    // these three may be any Button-derived class
    curr_elem = &elem.Child("m_tab");
    if (Button* b = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))) {
	m_tab.reset(b);
    } else {
	throw std::runtime_error("Scroll::Scroll : Attempted to use a non-Button object as the tab.");
    }
   
    curr_elem = &elem.Child("m_incr");
    if (Button* b = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))) {
	m_incr.reset(b);
    } else {
	throw std::runtime_error("Scroll::Scroll : Attempted to use a non-Button object as the increment button.");
    }
   
    curr_elem = &elem.Child("m_decr");
    if (Button* b = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))) {
	m_decr.reset(b);
    } else {
	throw std::runtime_error("Scroll::Scroll : Attempted to use a non-Button object as the decrement button.");
    }
   
    curr_elem = &elem.Child("m_tab_drag_offset");
    m_tab_drag_offset = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("m_initial_depressed_area");
    m_initial_depressed_area = ScrollRegion(lexical_cast<int>(curr_elem->Attribute("value")));
   
    curr_elem = &elem.Child("m_depressed_area");
    m_depressed_area = ScrollRegion(lexical_cast<int>(curr_elem->Attribute("value")));
   
    MoveTabToPosn(); // correct initial placement of tab, if necessary
}
   
int Scroll::Render()
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
    return 1;
}

int Scroll::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
	// when a button is pressed, record the region of the control the cursor is over
	m_tab_drag_offset = -1;   // until we know the tab is being dragged, clear the offset
	switch(RegionUnder(pt))
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
    return 1;
}

int Scroll::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled()) {
	// when dragging occurs, if we're dragging the tab, move it
	if (m_tab_drag_offset != -1) // if tab is being dragged
	{
	    int click_pos;
	    int scroll_width;   // "width" in these is not necessarily in the x direction, it means more like "extent" in horiz or vert direction
	    int tab_width;
	    int decr_width;
	    int incr_width;
	    if (m_orientation == VERTICAL) {
		click_pos = ScreenToWindow(pt).y;
		scroll_width = WindowDimensions().y;
		tab_width = m_tab->WindowDimensions().y;
		decr_width = m_decr->WindowDimensions().y;
		incr_width = m_incr->WindowDimensions().y;
	    } else {
		click_pos = ScreenToWindow(pt).x;
		scroll_width = WindowDimensions().x;
		tab_width = m_tab->WindowDimensions().x;
		decr_width = m_decr->WindowDimensions().x;
		incr_width = m_incr->WindowDimensions().x;
	    }
	    if (click_pos - m_tab_drag_offset < decr_width)
		m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, decr_width)) : m_tab->MoveTo(Pt(decr_width, 0));
	    else if (click_pos - m_tab_drag_offset + tab_width > scroll_width - incr_width)
		m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, scroll_width - incr_width - tab_width)) : m_tab->MoveTo(Pt(scroll_width - incr_width - tab_width, 0));
	    else
		m_orientation == VERTICAL ? m_tab->MoveTo(Pt(0, click_pos - m_tab_drag_offset)) : m_tab->MoveTo(Pt(click_pos - m_tab_drag_offset, 0));
	    UpdatePosn();
	} else { // otherwise, if there is dragging going on elsewhere, mark that area as "depressed", if it is the area we started in 
	    switch(RegionUnder(pt)) { // figure out where cursor is now
		case SBR_LINE_DN: if (m_initial_depressed_area == SBR_LINE_DN) ScrollLineDecr(); break;
		case SBR_LINE_UP: if (m_initial_depressed_area == SBR_LINE_UP) ScrollLineIncr(); break;
		case SBR_PAGE_DN: if (m_initial_depressed_area == SBR_PAGE_DN) ScrollPageDecr(); break;
		case SBR_PAGE_UP: if (m_initial_depressed_area == SBR_PAGE_UP) ScrollPageIncr(); break;
		default: break;
	    }
	}
    }
    return 1;
}

int Scroll::LButtonUp(const Pt& pt, Uint32 keys)
{
    m_decr->SetState(Button::BN_UNPRESSED);
    m_incr->SetState(Button::BN_UNPRESSED);
    m_initial_depressed_area = SBR_NONE; 
    m_depressed_area = SBR_NONE;
    return 1;
}

void Scroll::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    int bn_width = (m_orientation == VERTICAL) ? WindowDimensions().x : WindowDimensions().y;
    m_decr->SizeMove(0, 0, bn_width, bn_width);
    m_incr->SizeMove(WindowDimensions() - Pt(bn_width, bn_width), WindowDimensions());
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

void Scroll::SizeScroll(int min, int max, int line, int page) 
{
    m_line_sz = line; 
    m_range_min = min;
    m_range_max = max;
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
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
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
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::ScrollLineIncr() 
{
    int old_posn = m_posn;
    if (m_posn + m_line_sz <= m_range_max - m_page_sz) 
	m_posn += m_line_sz; 
    else 
	m_posn = m_range_max - (m_page_sz - 1); 
    MoveTabToPosn();
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::ScrollLineDecr() 
{
    int old_posn = m_posn;
    if (m_posn - m_line_sz >= m_range_min) 
	m_posn -= m_line_sz; 
    else 
	m_posn = m_range_min; 
    MoveTabToPosn();
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::ScrollPageIncr() 
{
    int old_posn = m_posn;
    if (m_posn + m_page_sz <= m_range_max - m_page_sz) 
	m_posn += m_page_sz; 
    else 
	m_posn = m_range_max - (m_page_sz - 1); 
    MoveTabToPosn();
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

void Scroll::ScrollPageDecr() 
{
    int old_posn = m_posn;
    if (m_posn - m_page_sz >= m_range_min) 
	m_posn -= m_page_sz; 
    else 
	m_posn = m_range_min; 
    MoveTabToPosn();
    if (old_posn != m_posn) m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);
}

XMLElement Scroll::XMLEncode() const
{
    XMLElement retval("GG::Scroll");
    retval.AppendChild(Control::XMLEncode());
   
    XMLElement temp;
   
    temp = XMLElement("m_int_color");
    temp.AppendChild(m_int_color.XMLEncode());
    retval.AppendChild(temp);
   
    temp = XMLElement("m_orientation");
    temp.SetAttribute("value", lexical_cast<string>(int(m_orientation)));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_posn");
    temp.SetAttribute("value", lexical_cast<string>(m_posn));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_range_min");
    temp.SetAttribute("value", lexical_cast<string>(m_range_min));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_range_max");
    temp.SetAttribute("value", lexical_cast<string>(m_range_max));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_line_sz");
    temp.SetAttribute("value", lexical_cast<string>(m_line_sz));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_page_sz");
    temp.SetAttribute("value", lexical_cast<string>(m_page_sz));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_tab");
    temp.AppendChild(m_tab->XMLEncode());
    retval.AppendChild(temp);
   
    temp = XMLElement("m_incr");
    temp.AppendChild(m_incr->XMLEncode());
    retval.AppendChild(temp);
   
    temp = XMLElement("m_decr");
    temp.AppendChild(m_decr->XMLEncode());
    retval.AppendChild(temp);
   
    temp = XMLElement("m_tab_drag_offset");
    temp.SetAttribute("value", lexical_cast<string>(m_tab_drag_offset));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_initial_depressed_area");
    temp.SetAttribute("value", lexical_cast<string>(int(m_initial_depressed_area)));
    retval.AppendChild(temp);
   
    temp = XMLElement("m_depressed_area");
    temp.SetAttribute("value", lexical_cast<string>(int(m_depressed_area)));
    retval.AppendChild(temp);
   
    return retval;
}

int Scroll::TabSpace() const
{
    // tab_space is the space the tab has to move about in (the control's width less the width of the incr & decr buttons)
    return (m_orientation == VERTICAL ? 
	    WindowDimensions().y - m_incr->WindowDimensions().y - m_decr->WindowDimensions().y :
	    WindowDimensions().x - m_incr->WindowDimensions().x - m_decr->WindowDimensions().x);
}

int Scroll::TabWidth() const
{
    int tab_space = TabSpace();
    return std::max(int(m_page_sz / (m_range_max - m_range_min + 1.0f) * tab_space), MIN_TAB_SIZE);
}

void Scroll::UpdatePosn()
{
    int before_tab = (m_orientation == VERTICAL ?            // the tabspace before the tab's lower-value side
		      m_tab->UpperLeft().y - m_decr->WindowDimensions().y :
		      m_tab->UpperLeft().x - m_decr->WindowDimensions().x );
    int tab_space = TabSpace(); // all the tabspace
    m_posn = int(m_range_min + double(before_tab) / tab_space * (m_range_max - m_range_min + 1));
    m_posn = std::min(m_range_max - m_page_sz + 1, std::max(m_range_min, m_posn));
    m_scrolled_sig(m_posn, m_posn + m_page_sz, m_range_min, m_range_max);   // notify interested parties whenever m_posn changes
}

void Scroll::MoveTabToPosn()
{
    int start_tabspace = (m_orientation==VERTICAL ?      // what is the tab's lowest posible extent?
			  m_decr->WindowDimensions().y:
			  m_decr->WindowDimensions().x);
    int end_tabspace = (m_orientation==VERTICAL ?      // what is its highest?
			WindowDimensions().y - m_incr->WindowDimensions().y:
			WindowDimensions().x - m_incr->WindowDimensions().x);
   
    m_tab->MoveTo(m_orientation==VERTICAL ? 
		  Pt(m_tab->UpperLeft().x, 
		     int((double(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace)) :
		  Pt(int((double(m_posn - m_range_min) / (m_range_max - m_range_min + 1)) * (end_tabspace - start_tabspace + 1) + start_tabspace), 
		     m_tab->UpperLeft().y));
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

} // namespace GG

