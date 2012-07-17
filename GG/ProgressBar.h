// -*- C++ -*-
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

/** \file ProgressBar.h \brief Contains the ProgressBar class. */

#ifndef _GG_ProgressBar_h_
#define _GG_ProgressBar_h_

#include <GG/Control.h>


namespace GG {

class GG_API ProgressBar : public Control
{
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    ProgressBar(X x, Y y, X w, Y h, Orientation orientation,
                unsigned int bar_width, Clr color,
                Clr bar_color = CLR_BLUE, Clr interior_color = CLR_ZERO);
    //@}

    /** \name Accessors */ ///@{
    virtual Pt MinUsableSize() const;

    /** Returns the color used to render the bar. */
    Clr BarColor() const;

    /** Returns the color used to render the non-bar interior of the control. */
    Clr InteriorColor() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();

    /** Sets the progress shown on the bar, in the range [0.0, 1.0].  Values
        outside this range will be clamped. */
    void SetPosition(double pos);

    /** Sets the color used to render the bar. */
    void SetBarColor(Clr color);

    /** Sets the color used to render the non-bar interior of the control. */
    void SetInteriorColor(Clr color);
    //@}

private:
    double m_pos;
    Orientation m_orientation;
    unsigned int m_bar_width;
    Clr m_bar_color;
    Clr m_interior_color;
};

}

#endif
