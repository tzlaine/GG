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

/** \file EventFilter.h \brief Contains the EventFilter class, which is used
    filter events passed to Wnds. */

#ifndef _GG_EventFilter_h_
#define _GG_EventFilter_h_


namespace GG {

class Wnd;
class WndEvent;

/** Filters the events passed to the Wnds on which it is installed. */
class GG_API EventFilter
{
public:
    /** Handles a WndEvent destined for Wnd \a w, but which this EventFilter
        is allowed to handle first.  Returns true if this filter processed the
        message. */
    bool Filter(Wnd* w, const WndEvent& event)
        { return FilterImpl(w, event); }

private:
    virtual bool FilterImpl(Wnd* w, const WndEvent& event)
        { return false; }
};

} // namespace GG

#endif
