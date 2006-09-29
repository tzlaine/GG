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
   
/** \file EventPump.h
    Contains the EventPump class and its helper classes.  An EventPump encapsulates the behavior of a GG processing loop, 
    such as the one associated with the singleton GUI object, or one associated with a modal Wnd. */

#ifndef _GG_EventPump_h_
#define _GG_EventPump_h_

#ifndef _GG_GUI_h_
#include <GG/GUI.h>
#endif

namespace GG {

/** encapsulates the state of GG event pumping.  A single state object is shared by all EventPumps, to ensure state
    consistency. */
struct GG_API EventPumpState
{
    EventPumpState(); ///< default ctor.

    int last_FPS_time;
    int last_frame_time;
    int most_recent_time;
    int time;
    int frames;
};

/** the base type for all EventPump types.  The action taken by EventPumpBase is a part of the basic GG::GUI
    functionality; users who wish to define a new type of event pump should do so by inheriting from EventPump instead
    of this class. */
class GG_API EventPumpBase
{
protected:
    /** executes everything but the system event handling portion of the event handling and rendering cycle. If \a
        do_non_rendering is true, all the non-rendering code in LoopBody() will be executed; if \a do_rendering is true,
        all rendering code in LoopBody() will be executed as well.  Set one or both of these to false to remove them
        from the execution of LoopBody().  This is provided mainly so users don't have to manually turn off and then
        turn back on FPS limits, etc., if they don't want them to take place in their custom event pump.  Also, this
        provides a convenient way to turn rendering off completely in a custom event pump, if that is desired. */
    void LoopBody(GUI* gui, EventPumpState& state, bool do_non_rendering, bool do_rendering);

    /** returns the EventPumpState object shared by all event pump types. */
    static EventPumpState& State();
};

/** encapsulates the GG event-pumping mechanism.  Events from the application framework (i.e. SDL, DirectInput, etc.) 
    are received by an EventPump, and appropriate action is taken.  The default action is to call
    GUI::HandleSystemEvents(), but any action can be taken in a EventPump-derived type that overrides operator()().  For
    example, it might be useful to override operator()() with a function that gives all GG-relevant events to a GG
    event-hendler, and gives all other events to a system-specific handler, if your GUI-derived class does not already
    do so. */
class GG_API EventPump : public EventPumpBase
{
public:
    virtual ~EventPump() {} ///< virtual dtor

    /** cycles through event-handling and rendering, calling GUI::HandleSystemEvents() and then EventPumpBase::LoopBody(). */
    virtual void operator()();
};

/** an EventPump that terminates when its m_done reference member is true.  \note Modal Wnds use EventPumps to implement
    their modality. */
class GG_API ModalEventPump : public EventPump
{
public:
    ModalEventPump(const bool& done);
    virtual void operator()();
protected:
    const bool& m_done;
};

} // namespace GG

#endif // _GG_EventPump_h_
