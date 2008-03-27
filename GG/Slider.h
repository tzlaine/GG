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

/** \file Slider.h
    Contains the Slider class, which provides a slider control that allows the user to select a value from a range 
    if integers. */

#ifndef _GG_Slider_h_
#define _GG_Slider_h_

#include <GG/Control.h>

#include <boost/serialization/version.hpp>


namespace GG {

class Button;
class WndEvent;

/** a slider control.  This control allows the user to drag a tab to a desired setting; it is somewhat like a Scroll.
    Sliders can be either vertical or horizontal, but cannot switch between the two.  Unlike vertical Scrolls, whose
    values increase downward, vertical Sliders increase upward by default.  Note that it is acceptible to define a range
    that increases from min to max, or one that decreases from min to max; both are legal. */
class GG_API Slider : public Control
{
public:
    /** \name Signal Types */ ///@{
    typedef boost::signal<void (int, int, int)> SlidSignalType;           ///< emitted whenever the slider is moved; the tab position and the upper and lower bounds of the slider's range are indicated, respectively
    typedef boost::signal<void (int, int, int)> SlidAndStoppedSignalType; ///< emitted when the slider's tab is stopped after being dragged, the slider is adjusted using the keyboard, or the slider is moved programmatically; the tab position and the upper and lower bounds of the slider's range are indicated, respectively
    //@}

    /** \name Slot Types */ ///@{
    typedef SlidSignalType::slot_type SlidSlotType;           ///< type of functor(s) invoked on a SlidSignalType
    typedef SlidSignalType::slot_type SlidAndStoppedSlotType; ///< type of functor(s) invoked on a SlidAndStoppedSignalType
    //@}

    /** \name Structors */ ///@{
    Slider(int x, int y, int w, int h, int min, int max, Orientation orientation, SliderLineStyle style, Clr color, int tab_width, int line_width = 5, Flags<WndFlag> flags = CLICKABLE); ///< ctor
    //@}

    /** \name Accessors */ ///@{
    virtual Pt           MinUsableSize() const;

    int                  Posn() const;           ///< returns the current tab position
    std::pair<int, int>  SliderRange() const;    ///< returns the defined possible range of control

    /** returns the current page size, or the amount that the slider increments/decrements when a click occurs off of
        the tab.  If not set, this defaults to 10% of the slider's range. */
    int                  PageSize() const;

    Orientation          GetOrientation() const; ///< returns the orientation of the slider (VERTICAL or HORIZONTAL)
    int                  TabWidth() const;       ///< returns the width of the slider's tab, in pixels
    int                  LineWidth() const;      ///< returns the width of the line along which the tab slides, in pixels
    SliderLineStyle      LineStyle() const;      ///< returns the style of line used to render the control

    mutable SlidSignalType           SlidSignal;           ///< returns the slid signal object for this Slider
    mutable SlidAndStoppedSignalType SlidAndStoppedSignal; ///< returns the slid-and-stopped signal object for this Slider
    //@}

    /** \name Mutators */ ///@{
    virtual void   Render();
    virtual void   LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   KeyPress(Key key, Flags<ModKey> mod_keys);
    virtual void   SizeMove(const Pt& ul, const Pt& lr);
    virtual void   Disable(bool b = true);
    virtual void   SetColor(Clr c);

    void           SizeSlider(int min, int max); ///< sets the logical range of the control; \a min must not equal \a max
    void           SetMax(int max);              ///< sets the maximum value of the control
    void           SetMin(int min);              ///< sets the minimum value of the control
    void           SlideTo(int p);               ///< slides the control to a certain spot

    /** sets the size of a "page", or the amount that the slider increments/decrements when a click occurs off of the
        tab.  If not set, this defaults to 10% of the slider's range.  To disable clicks off the tab, set the page size
        to 0. */
    void           SetPageSize(int size);

    void           SetLineStyle(SliderLineStyle style); ///< returns the style of line used to render the control

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ ///@{
    Slider(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    Button* Tab() const;                  ///< returns a pointer to the Button used as this control's sliding tab
    int     PtToPosn(const Pt& pt) const; ///< maps an arbitrary screen point to its nearest logical slider position
    //@}

    /** \name Mutators */ ///@{
    virtual bool EventFilter(Wnd* w, const WndEvent& event);

    void MoveTabToPosn(); ///< moves the tab to the current logical position
    //@}

private:
    void UpdatePosn();

    int                       m_posn;
    int                       m_range_min;
    int                       m_range_max;
    int                       m_page_sz;
    Orientation               m_orientation;
    int                       m_line_width;
    int                       m_tab_width;
    SliderLineStyle           m_line_style;
    int                       m_tab_drag_offset;
    Button*                   m_tab;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

BOOST_CLASS_VERSION(GG::Slider, 1)

// template implementations
template <class Archive>
void GG::Slider::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_posn)
        & BOOST_SERIALIZATION_NVP(m_range_min)
        & BOOST_SERIALIZATION_NVP(m_range_max)
        & BOOST_SERIALIZATION_NVP(m_orientation)
        & BOOST_SERIALIZATION_NVP(m_line_width)
        & BOOST_SERIALIZATION_NVP(m_tab_width)
        & BOOST_SERIALIZATION_NVP(m_line_style)
        & BOOST_SERIALIZATION_NVP(m_tab_drag_offset)
        & BOOST_SERIALIZATION_NVP(m_tab);

    if (1 <= version)
        ar & BOOST_SERIALIZATION_NVP(m_page_sz);
}

#endif // _GG_Slider_h_
