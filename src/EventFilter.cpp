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

#include <GG/EventFilter.h>

#include <GG/Wnd.h>


using namespace GG;

EventFilter::~EventFilter()
{
    for (std::set<Wnd*>::iterator it = m_targets.begin(); it != m_targets.end(); ++it) {
        std::vector<EventFilter*>::iterator filter_it =
            std::find((*it)->m_filters.begin(), (*it)->m_filters.end(), this);
        if (filter_it != (*it)->m_filters.end())
            (*it)->m_filters.erase(filter_it);
    }
}

void EventFilter::AddTarget(Wnd* w)
{ m_targets.insert(w); }

void EventFilter::RemoveTarget(Wnd* w)
{ m_targets.erase(w); }

bool EventFilter::Filter(Wnd* w, const WndEvent& event)
{ return FilterImpl(w, event); }

bool EventFilter::FilterImpl(Wnd* w, const WndEvent& event)
{ return false; }
