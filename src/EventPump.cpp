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

#include <GG/EventPump.h>

using namespace GG;

EventPumpState::EventPumpState() :
    last_FPS_time(0),
    last_frame_time(0),
    most_recent_time(0),
    time(0),
    frames(0),
    last_mouse_event_time(0),
    mouse_drag_repeat_start_time(0),
    last_mouse_drag_repeat_time(0),
    mouse_repeat_delay(0),
    mouse_repeat_interval(0),
    old_mouse_repeat_delay(0),
    old_mouse_repeat_interval(0)
{}


void EventPumpBase::LoopBody(GUI* gui, EventPumpState& state, bool do_non_rendering, bool do_rendering)
{
    if (do_non_rendering) {
        // handle mouse drag repeats; if there's a change in the values, zero everything out and start the counting over
        if (state.old_mouse_repeat_delay != gui->MouseRepeatDelay() || state.old_mouse_repeat_interval != gui->MouseRepeatInterval()) {
            state.old_mouse_repeat_delay = gui->MouseRepeatDelay();
            state.old_mouse_repeat_interval = gui->MouseRepeatInterval();
            state.mouse_drag_repeat_start_time = 0;
            state.last_mouse_drag_repeat_time = 0;
        }

        state.time = gui->Ticks();

        // if drag repeat is enabled, the left mouse button is depressed (a drag is occurring), and the last event
        // processed wasn't too recent
        if (gui->MouseRepeatDelay() && gui->MouseButtonDown(0) && state.time - state.last_mouse_event_time > state.old_mouse_repeat_interval) {
            if (!state.mouse_drag_repeat_start_time) { // if we're just starting the drag, mark the time we started
                state.mouse_drag_repeat_start_time = state.time;
            } else if (state.mouse_drag_repeat_start_time == gui->MouseRepeatDelay()) { // if we're counting repeat intervals
                if (state.time - state.last_mouse_drag_repeat_time > gui->MouseRepeatInterval()) {
                    state.last_mouse_drag_repeat_time = state.time;
                    gui->HandleGGEvent(GUI::MOUSEMOVE, GGK_UNKNOWN, gui->KeyMods(), gui->MousePosition(), Pt());
                }
            } else if (state.time - state.mouse_drag_repeat_start_time > gui->MouseRepeatDelay()) { // if we're done waiting for the initial delay period
                state.mouse_drag_repeat_start_time = gui->MouseRepeatDelay(); // set this as equal so we know later that we've passed the delay interval
                state.last_mouse_drag_repeat_time = state.time;
                gui->HandleGGEvent(GUI::MOUSEMOVE, GGK_UNKNOWN, gui->KeyMods(), gui->MousePosition(), Pt());
            }
        } else {
            // otherwise, send an idle message immediately, so that the gui has timely updates for triggering browse
            // info windows, etc., and reset the mouse drag repeat start time to zero
            gui->HandleGGEvent(GUI::IDLE, GGK_UNKNOWN, gui->KeyMods(), gui->MousePosition(), Pt());
            state.mouse_drag_repeat_start_time = 0;
        }

        // govern FPS speed if needed
        if (double max_FPS = gui->MaxFPS()) {
            double min_ms_per_frame = 1000.0 * 1.0 / max_FPS;
            double ms_to_wait = min_ms_per_frame - (state.time - state.last_frame_time);
            if (0.0 < ms_to_wait)
                gui->Wait(static_cast<int>(ms_to_wait));
        }
        state.last_frame_time = state.time;

        // track FPS if needed
        gui->SetDeltaT(state.time - state.most_recent_time);
        if (gui->FPSEnabled()) {
            ++state.frames;
            if (1000 < state.time - state.last_FPS_time) { // calculate FPS at most once a second
                gui->SetFPS(state.frames / ((state.time - state.last_FPS_time) / 1000.0));
                state.last_FPS_time = state.time;
                state.frames = 0;
            }
        }
        state.most_recent_time = state.time;
    }

    if (do_rendering) {
        // do one iteration of the render loop
        gui->RenderBegin();
        gui->Render();
        gui->RenderEnd();
    }
}

EventPumpState& EventPumpBase::State()
{
    static EventPumpState state;
    return state;
}


void EventPump::operator()()
{
    GUI* gui = GUI::GetGUI();
    EventPumpState& state = State();
    while (1) {
        gui->HandleSystemEvents(state.last_mouse_event_time);
        LoopBody(gui, state, true, true);
    }
}
