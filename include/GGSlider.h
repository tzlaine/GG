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

#ifndef _GGSlider_h_
#define _GGSlider_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

namespace GG {

class Button;

/** a slider control.  This control allows the user to drag a tab to a desired setting; it is somewhat like a Scroll.
   Sliders can be either vertical or horizontal, but cannot switch between the two.  Unlike vertical Scrolls, whose 
   values increase downward, vertical Sliders increase upward by default.  Note that it is acceptible to define a range 
   that increases from min to max, or one that decreases from min to max; both are legal. */
class GG_API Slider : public Control
{
public:
    using Wnd::SizeMove;

    enum Orientation     {VERTICAL, HORIZONTAL};    ///< the orientation of the slider must be one of these two values
    enum LineStyleType   {FLAT, RAISED, GROOVED};   ///< the rendering styles of the line the tab slides over

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int, int, int)> SlidSignalType; ///< emitted whenever the slider is moved; the tab position and the upper and lower bounds of the slider's range are indicated, respectively
    //@}

    /** \name Slot Types */ //@{
    typedef SlidSignalType::slot_type SlidSlotType;   ///< type of functor(s) invoked on a SlidSignalType
    //@}

    /** \name Structors */ //@{
    Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, LineStyleType style, Clr color, int tab_width, int line_width = 5, Uint32 flags = CLICKABLE); ///< ctor
    Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, LineStyleType style, Clr color, Button* tab = 0, int line_width = 5, Uint32 flags = CLICKABLE); ///< ctor
    Slider(const XMLElement& elem); ///< ctor that constructs a Slider object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Slider object
    //@}

    /** \name Accessors */ //@{
    int            Posn() const         {return m_posn;}        ///< returns the current tab position
    pair<int,int>  SliderRange() const  {return pair<int,int>(m_range_min, m_range_max);}  ///< returns the defined possible range of control
    Orientation    GetOrientation() const{return m_orientation;} ///< returns the orientation of the slider (VERTICAL or HORIZONTAL)
    int            TabWidth() const     {return m_tab_width;} ///< returns the width of the slider's tab, in pixels
    int            LineWidth() const    {return m_line_width;} ///< returns the width of the line along which the tab slides, in pixels
    LineStyleType  LineStyle() const    {return m_line_style;}  ///< returns the style of line used to render the control

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Slider object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object

    SlidSignalType& SlidSignal() const  {return m_slid_sig;} ///< returns the slid signal object for this DynamicGraphic
    //@}

    /** \name Mutators */ //@{
    virtual int    Render();
    virtual int    LButtonDown(const Pt& pt, Uint32 keys);
    virtual int    LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual int    MouseHere(const Pt& pt, Uint32 keys)               {m_tab_drag_offset = -1; return 1;}
    virtual int    Keypress(Key key, Uint32 key_mods);

    virtual void   SizeMove(int x1, int y1, int x2, int y2); ///< sizes the conrol, then resizes the tab as needed
    virtual void   Disable(bool b = true);

    void           SizeSlider(int min, int max);                      ///< sets the logical range of the control
    void           SetMax(int max)   {SizeSlider(m_range_min, max);}  ///< sets the maximum value of the control
    void           SetMin(int min)   {SizeSlider(min, m_range_max);}  ///< sets the minimum value of the control

    void           SlideTo(int p);  ///< slides the control to a certain spot
   
    void           SetLineStyle(LineStyleType style)   {m_line_style = style;}  ///< returns the style of line used to render the control
    //@}

protected:
    /** \name Accessors */ //@{
    int TabDragOffset() const {return m_tab_drag_offset;} ///< returns the offset from the cursor to the left edge of the tab; -1 when the tab is not being dragged

    const shared_ptr<Button>& Tab() const {return m_tab;} ///< returns a pointer to the Button used as this control's sliding tab
    //@}

private:
    void  MoveTabToPosn();
    void  UpdatePosn();
   
    int                  m_posn;
    int                  m_range_min;
    int                  m_range_max;
   
    Orientation          m_orientation;
   
    int                  m_line_width;
    int                  m_tab_width;
    LineStyleType        m_line_style;
   
    int                  m_tab_drag_offset;
    shared_ptr<Button>   m_tab;

    mutable SlidSignalType m_slid_sig;
};

// define EnumMap and stream operators for Slider::Orientation
ENUM_MAP_BEGIN(Slider::Orientation)
    ENUM_MAP_INSERT(Slider::VERTICAL)
    ENUM_MAP_INSERT(Slider::HORIZONTAL)
ENUM_MAP_END

ENUM_STREAM_IN(Slider::Orientation)
ENUM_STREAM_OUT(Slider::Orientation)

// define EnumMap and stream operators for Slider::LineStyleType
ENUM_MAP_BEGIN(Slider::LineStyleType)
    ENUM_MAP_INSERT(Slider::FLAT)
    ENUM_MAP_INSERT(Slider::RAISED)
    ENUM_MAP_INSERT(Slider::GROOVED)
ENUM_MAP_END

ENUM_STREAM_IN(Slider::LineStyleType)
ENUM_STREAM_OUT(Slider::LineStyleType)

} // namespace GG

#endif // _GGSlider_h_

