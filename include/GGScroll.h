// -*- C++ -*-
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

#ifndef _GGScroll_h_
#define _GGScroll_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

namespace GG {

class Button;

/** This is a basic scrollbar control.
    The range of the values the scrollbar represents is [m_range_min, m_range_max].  However, m_posn can only range over 
    [m_range_min, m_range_max - m_page_sz], because the tab has a logical width of m_page_sz.  So the region of the 
    scrollbar's range being viewed at any one time is [m_posn, m_posn + m_page_sz].  (m_posn + m_page_sz is actually the 
    last + 1 element of the range.)  The parent of the control is notified of a scroll via ScrolledSignalType signals; 
    these are emitted from the Scroll*() functions and the UpdatePosn() function.  This should cover every instance 
    in which m_posn is altered.  The parent can poll the control to get its current view area with a call to GetPosnRange().
    An increase in a vertical scroll is down, and a decrease is up; since GG assumes the y-coordinates are downwardly 
    increasing.  The rather plain default buttons and tab can be replaced by any Button-derived controls desired.
    However, if you want to save and load a Scroll (using an XML encoding) that has custom buttons and/or tab, you 
    must add the new derived types to the App's XMLObjectFactory.  Otherwise, Scroll's XMLElement ctor will not know 
    how to create the custom controls at load-time.  See GG::App::AddWndGenerator() and GG::XMLObjectFactory for details.*/
class Scroll : public Control
{
public:
    using Wnd::SizeMove;

    enum Orientation {VERTICAL, HORIZONTAL}; ///< the orientation of the scrollbar must be one of these two values
   
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int, int, int, int)> ScrolledSignalType; ///< emitted whenever the scrollbar is moved; the upper and lower extents of the tab and the upper and lower bounds of the scroll's range are indicated, respectively
    //@}
   
    /** \name Slot Types */ //@{
    typedef ScrolledSignalType::slot_type ScrolledSlotType; ///< type of functor(s) invoked on a ScrolledSignalType
    //@}
   
    /** \name Structors */ //@{
    /** ctor. \warning Calling code <b>must not</b> delete the buttons passed to this ctro, if any.  They become the 
        property of shared_ptrs inside the Scroll.*/
    Scroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior, Button* decr = 0, Button* incr = 0, Button* tab = 0, Uint32 flags = CLICKABLE);
    Scroll(const XMLElement& elem); ///< ctor that constructs a Scroll object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Scroll object
    //@}
   
    /** \name Accessors */ //@{
    pair<int,int>   PosnRange() const    {return pair<int,int>(m_posn, m_posn + m_page_sz);}   ///< range currently being viewed
    pair<int,int>   ScrollRange() const  {return pair<int,int>(m_range_min, m_range_max);}     ///< defined possible range of control
    int             LineSize() const     {return m_line_sz;}    ///< returns the current line size
    int             PageSize() const     {return m_page_sz;}    ///< returns the current page size

    Clr             InteriorColor() const       {return m_int_color;}   ///< returns the color used to render the interior of the Scroll
    Orientation     ScrollOrientation() const   {return m_orientation;} ///< returns the orientation of the Scroll

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Scroll object
    //@}
   
    /** \name Mutators */ //@{
    virtual int    Render();
    virtual int    LButtonDown(const Pt& pt, Uint32 keys);
    virtual int    LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual int    LButtonUp(const Pt& pt, Uint32 keys);
    virtual int    LClick(const Pt& pt, Uint32 keys)         {return LButtonUp(pt, keys);}
    virtual int    MouseHere(const Pt& pt, Uint32 keys)      {return LButtonUp(pt, keys);}
    virtual int    MouseLeave(const Pt& pt, Uint32 keys)     {m_depressed_area = SBR_NONE; return 1;}
   
    virtual void   SizeMove(int x1, int y1, int x2, int y2); ///< sizes the conrol, then resizes the buttons and tab as needed
    virtual void   Disable(bool b = true);
   
    void           SizeScroll(int min, int max, int line, int page); ///< sets the logical ranges of the control, and the logical increment values
    void           SetMax(int max)        {SizeScroll(m_range_min, max, m_line_sz, m_page_sz);}    ///< sets the maximum value of the scroll
    void           SetMin(int min)        {SizeScroll(min, m_range_max, m_line_sz, m_page_sz);}    ///< sets the minimum value of the scroll
    void           SetLineSize(int line)  {SizeScroll(m_range_min, m_range_max, line, m_page_sz);} ///< sets the size of a line in the scroll. This is the number of logical units the tab moves when either of the up or down buttons is pressed.
    void           SetPageSize(int page)  {SizeScroll(m_range_min, m_range_max, m_line_sz, page);} ///< sets the maximum value of the scroll. This is the number of logical units the tab moves when either of the page-up or page-down areas is clicked.
   
    void           ScrollTo(int p);  ///< scrolls the control to a certain spot
    void           ScrollLineIncr(); ///< scrolls the control down (or right) by a line
    void           ScrollLineDecr(); ///< scrolls the control up (or left) by a line
    void           ScrollPageIncr(); ///< scrolls the control down (or right) by a page
    void           ScrollPageDecr(); ///< scrolls the control up (or left) by a page
   
    ScrolledSignalType& ScrolledSignal() {return m_scrolled_sig;}    ///< returns the scrolled signal object for this Scroll
    //@}

protected:
    enum ScrollRegion {SBR_NONE, SBR_TAB, SBR_LINE_DN, SBR_LINE_UP, SBR_PAGE_DN, SBR_PAGE_UP};
   
    /** \name Accessors */ //@{
    int            TabSpace() const;          ///< returns the space the tab has to move about in (the control's width less the width of the incr & decr buttons)
    int            TabWidth() const;          ///< returns the calculated width of the tab, based on PageSize() and the logical size of the control, in pixels
    ScrollRegion   RegionUnder(const Pt& pt); ///< determines whether a pt is in the incr or decr or tab buttons, or in PgUp/PgDn regions in between
   
    const shared_ptr<Button>    TabButton() const   {return m_tab;}     ///< returns the button representing the tab
    const shared_ptr<Button>    IncrButton() const  {return m_incr;}    ///< returns the increase button (line down/line right)
    const shared_ptr<Button>    DecrButton() const  {return m_decr;}    ///< returns the decrease button (line up/line left)
    //@}

private:
    void           UpdatePosn();              ///< adjusts m_posn due to a tab-drag
    void           MoveTabToPosn();           ///< adjusts tab due to a button click, PgUp, etc.

    Clr                  m_int_color;   ///< color inside border of slide area
    const Orientation    m_orientation; ///< vertical or horizontal scroll? (use enum for these declared above)
    int                  m_posn;        ///< current position of tab in logical coords (will be in [m_range_min, m_range_max - m_page_sz])
    int                  m_range_min;   ///< lowest value in range of scrollbar
    int                  m_range_max;   ///< highest value "
    int                  m_line_sz;     ///< logical units traversed in a line movement (such as a click on either end button)
    int                  m_page_sz;     ///< logical units traversed for a page movement (such as a click in non-tab middle area, or PgUp/PgDn)
    shared_ptr<Button>   m_tab;         ///< the button representing the tab
    shared_ptr<Button>   m_incr;        ///< the increase button (line down/line right)
    shared_ptr<Button>   m_decr;        ///< the decrease button (line up/line left)
    int                  m_tab_drag_offset;         ///< the offset on the tab as it is dragged (like drag_offset in ProcessInput function)
    ScrollRegion         m_initial_depressed_area;  ///< the part of the scrollbar originally under cursor in LButtonDown msg
    ScrollRegion         m_depressed_area;          ///< the part of the scrollbar currently being "depressed" by held-down mouse button

    ScrolledSignalType   m_scrolled_sig;
};

} // namespace GG

#endif // _GGScroll_h_

