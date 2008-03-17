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

#include <GG/GUI.h>

#include <GG/BrowseInfoWnd.h>
#include <GG/Control.h>
#include <GG/Cursor.h>
#include <GG/EventPump.h>
#include <GG/Layout.h>
#include <GG/PluginInterface.h>
#include <GG/StyleFactory.h>
#include <GG/Timer.h>
#include <GG/ZList.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <cassert>
#include <fstream>
#include <list>


#define INSTRUMENT_GET_WINDOW_UNDER 0
#if INSTRUMENT_GET_WINDOW_UNDER
#include <iostream>
#endif

using namespace GG;

namespace {
    const boost::xpressive::sregex WORD_REGEX =
        +boost::xpressive::set[boost::xpressive::_w | '-'];

    /* returns the storage value of mod_keys that should be used with keyboard accelerators the accelerators don't care
       which side of the keyboard you use for CTRL, SHIFT, etc., and whether or not the numlock or capslock are
       engaged.*/
    Flags<ModKey> MassagedAccelModKeys(Flags<ModKey> mod_keys)
    {
        mod_keys &= ~(MOD_KEY_NUM | MOD_KEY_CAPS);
        if (mod_keys & MOD_KEY_CTRL)
            mod_keys |= MOD_KEY_CTRL;
        if (mod_keys & MOD_KEY_SHIFT)
            mod_keys |= MOD_KEY_SHIFT;
        if (mod_keys & MOD_KEY_ALT)
            mod_keys |= MOD_KEY_ALT;
        if (mod_keys & MOD_KEY_META)
            mod_keys |= MOD_KEY_META;
        return mod_keys;
    }

    WndEvent::EventType ButtonEvent(WndEvent::EventType left_type, int mouse_button)
    { return WndEvent::EventType(left_type + (WndEvent::MButtonDown - WndEvent::LButtonDown) * mouse_button); }
}

// implementation data types
struct GG::GUIImpl
{
    GUIImpl() :
        focus_wnd(0),
        mouse_pos(-1000, -1000),
        mouse_rel(0, 0),
        mod_keys(),
        button_down_repeat_delay(250),
        button_down_repeat_interval(66),
        last_button_down_repeat_time(0),
        double_click_interval(500),
        min_drag_time(250),
        min_drag_distance(5),
        prev_button_press_time(-1),
        prev_wnd_under_cursor(0),
        prev_wnd_under_cursor_time(-1),
        curr_wnd_under_cursor(0),
        drag_wnds(),
        curr_drag_wnd_dragged(false),
        curr_drag_drop_here_wnd(0),
        wnd_region(WR_NONE),
        browse_target(0),
        drag_drop_originating_wnd(0),
        delta_t(0),
        rendering_drag_drop_wnds(false),
        FPS(-1.0),
        calc_FPS(false),
        max_FPS(0.0),
        double_click_wnd(0),
        double_click_start_time(-1),
        double_click_time(-1),
        style_factory(new StyleFactory()),
        render_cursor(false),
        cursor(),
        save_wnd_fn(0),
        load_wnd_fn(0)
    {
        button_state[0] = button_state[1] = button_state[2] = false;
        drag_wnds[0] = drag_wnds[1] = drag_wnds[2] = 0;
    }

    void HandlePress(int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleDrag(int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleRelease(int mouse_button, const GG::Pt& pos, int curr_ticks);

    std::string  app_name;              // the user-defined name of the apllication

    ZList        zlist;                 // object that keeps the GUI windows in the correct depth ordering
    Wnd*         focus_wnd;             // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)
    std::list<std::pair<Wnd*, Wnd*> >
                 modal_wnds;            // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)

    bool         button_state[3];       // the up/down states of the three buttons on the mouse are kept here
    Pt           mouse_pos;             // absolute position of mouse, based on last MOUSEMOVE event
    Pt           mouse_rel;             // relative position of mouse, based on last MOUSEMOVE event
    Flags<ModKey>mod_keys;              // currently-depressed modifier keys, based on last KEYPRESS event

    int          button_down_repeat_delay;     // see note above GUI class definition
    int          button_down_repeat_interval;
    int          last_button_down_repeat_time; // last time of a simulated button-down message

    int          double_click_interval; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          min_drag_time;         // the minimum amount of time that a drag must be in progress before it is considered a drag, in ms
    int          min_drag_distance;     // the minimum distance that a drag must cover before it is considered a drag

    int          prev_button_press_time;// the time of the most recent mouse button press
    Pt           prev_button_press_pos; // the location of the most recent mouse button press
    Wnd*         prev_wnd_under_cursor; // GUI window most recently under the input cursor; may be 0
    int          prev_wnd_under_cursor_time; // the time at which prev_wnd_under_cursor was initially set to its current value
    Wnd*         curr_wnd_under_cursor; // GUI window currently under the input cursor; may be 0
    Wnd*         drag_wnds[3];          // GUI window currently being clicked or dragged by each mouse button
    Pt           prev_wnd_drag_position;// the upper-left corner of the dragged window when the last *Drag message was generated
    Pt           wnd_drag_offset;       // the offset from the upper left corner of the dragged window to the cursor for the current drag
    bool         curr_drag_wnd_dragged; // true iff the currently-pressed window (drag_wnd[N]) has actually been dragged some distance (in which case releasing the mouse button is not a click)
    Wnd*         curr_drag_drop_here_wnd;// the Wnd that most recently received a DragDropEnter or DragDropHere message (0 if DragDropLeave was sent as well, or if none)
    Pt           wnd_resize_offset;     // offset from the cursor of either the upper-left or lower-right corner of the GUI window currently being resized
    WndRegion    wnd_region;            // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    boost::shared_ptr<BrowseInfoWnd>
                 browse_info_wnd;       // the current browse info window, if any
    int          browse_info_mode;      // the current browse info mode (only valid if browse_info_wnd is non-null)
    Wnd*         browse_target;         // the current browse info target

    Wnd*         drag_drop_originating_wnd; // the window that originally owned the Wnds in drag_drop_wnds
    std::map<Wnd*, Pt>
                 drag_drop_wnds;        // the Wnds (and their offsets) that are being dragged and dropped between Wnds

    std::set<std::pair<Key, Flags<ModKey> > >
                 accelerators;          // the keyboard accelerators

    std::map<std::pair<Key, Flags<ModKey> >, boost::shared_ptr<GUI::AcceleratorSignalType> >
                 accelerator_sigs;      // the signals emitted by the keyboard accelerators

    int          delta_t;               // the number of ms since the last frame
    bool         rendering_drag_drop_wnds;
    double       FPS;                   // the most recent calculation of the frames per second rendering speed (-1.0 if calcs are disabled)
    bool         calc_FPS;              // true iff FPS calcs are to be done
    double       max_FPS;               // the maximum allowed frames per second rendering speed

    Wnd*         double_click_wnd;      // GUI window most recently clicked
    int          double_click_button;   // the index of the mouse button used in the last click
    int          double_click_start_time;// the time from which we started measuring double_click_time, in ms
    int          double_click_time;     // time elapsed since last click, in ms

    boost::shared_ptr<StyleFactory> style_factory;
    bool                            render_cursor;
    boost::shared_ptr<Cursor>       cursor;

    std::set<Timer*>  timers;

    GUI::SaveWndFn    save_wnd_fn;
    GUI::LoadWndFn    load_wnd_fn;
};

void GUIImpl::HandlePress(int mouse_button, const Pt& pos, int curr_ticks)
{
    curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys);
    last_button_down_repeat_time = 0;
    prev_wnd_drag_position = Pt();
    wnd_drag_offset = Pt();
    prev_button_press_time = 0;
    browse_info_wnd.reset();
    browse_target = 0;
    prev_wnd_under_cursor_time = curr_ticks;
    prev_button_press_time = curr_ticks;
    prev_button_press_pos = pos;

    button_state[mouse_button] = true;
    drag_wnds[mouse_button] = curr_wnd_under_cursor; // track this window as the one being dragged by this mouse button
    if (curr_wnd_under_cursor) {
        prev_wnd_drag_position = drag_wnds[mouse_button]->UpperLeft();
        wnd_drag_offset = pos - prev_wnd_drag_position;
    }

    // if this window is not a disabled Control window, it becomes the focus window
    Control* control = 0;
    if (drag_wnds[mouse_button] && (!(control = dynamic_cast<Control*>(drag_wnds[mouse_button])) || !control->Disabled()))
        GUI::s_gui->SetFocusWnd(drag_wnds[mouse_button]);

    if (drag_wnds[mouse_button]) {
        wnd_region = drag_wnds[mouse_button]->WindowRegion(pos); // and determine whether a resize-region of it is being dragged
        if (wnd_region % 3 == 0) // left regions
            wnd_resize_offset.x = drag_wnds[mouse_button]->UpperLeft().x - pos.x;
        else
            wnd_resize_offset.x = drag_wnds[mouse_button]->LowerRight().x - pos.x;
        if (wnd_region < 3) // top regions
            wnd_resize_offset.y = drag_wnds[mouse_button]->UpperLeft().y - pos.y;
        else
            wnd_resize_offset.y = drag_wnds[mouse_button]->LowerRight().y - pos.y;
        Wnd* drag_wnds_root_parent = drag_wnds[mouse_button]->RootParent();
        GUI::s_gui->MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : drag_wnds[mouse_button]);
        drag_wnds[mouse_button]->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonDown, mouse_button), pos, mod_keys));
    }

    prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleDrag(int mouse_button, const Pt& pos, int curr_ticks)
{
    if (wnd_region == WR_MIDDLE || wnd_region == WR_NONE) { // send drag message to window or initiate drag-and-drop
        Pt diff = prev_button_press_pos - pos;
        int drag_distance = diff.x * diff.x + diff.y * diff.y;
        // ensure that the minimum drag requirements are met
        if (min_drag_time < (curr_ticks - prev_button_press_time) &&
            (min_drag_distance * min_drag_distance < drag_distance) &&
            drag_drop_wnds.find(drag_wnds[mouse_button]) == drag_drop_wnds.end()) {
            if (!drag_wnds[mouse_button]->Dragable() &&
                drag_wnds[mouse_button]->DragDropDataType() != "") {
                Wnd* parent = drag_wnds[mouse_button]->Parent();
                Pt offset = prev_button_press_pos - drag_wnds[mouse_button]->UpperLeft();
                GUI::s_gui->RegisterDragDropWnd(drag_wnds[mouse_button], offset, parent);
                if (parent)
                    parent->StartingChildDragDrop(drag_wnds[mouse_button], offset);
            } else {
                Pt start_pos = drag_wnds[mouse_button]->UpperLeft();
                Pt move = (pos - wnd_drag_offset) - prev_wnd_drag_position;
                drag_wnds[mouse_button]->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDrag, mouse_button), pos, move, mod_keys));
                prev_wnd_drag_position = drag_wnds[mouse_button]->UpperLeft();
                if (start_pos != drag_wnds[mouse_button]->UpperLeft())
                    curr_drag_wnd_dragged = true;
            }
        }
        // notify wnd under cursor of presence of drag-and-drop wnd(s)
        if (curr_drag_wnd_dragged && !drag_wnds[mouse_button]->DragDropDataType().empty() ||
            !drag_drop_wnds.empty()) {
            bool unregistered_drag = curr_drag_wnd_dragged;
            curr_wnd_under_cursor = zlist.Pick(pos, GUI::s_gui->ModalWindow(), unregistered_drag ? drag_wnds[mouse_button] : 0);
            std::map<Wnd*, Pt> drag_drop_wnds;
            drag_drop_wnds[drag_wnds[mouse_button]] = wnd_drag_offset;
            std::map<Wnd*, Pt>& drag_drop_wnds_to_use = unregistered_drag ? drag_drop_wnds : drag_drop_wnds;
            if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) {
                if (curr_drag_drop_here_wnd) {
                    assert(curr_wnd_under_cursor == curr_drag_drop_here_wnd);
                    curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropHere, pos, drag_drop_wnds_to_use, mod_keys));
                } else {
                    curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropEnter, pos, drag_drop_wnds_to_use, mod_keys));
                    curr_drag_drop_here_wnd = curr_wnd_under_cursor;
                }
            }
        }
    } else if (drag_wnds[mouse_button]->Resizable()) { // send appropriate resize message to window
        Pt offset_pos = pos + wnd_resize_offset;
        if (Wnd* parent = drag_wnds[mouse_button]->Parent())
            offset_pos -= parent->ClientUpperLeft();
        switch (wnd_region)
        {
        case WR_TOPLEFT:
            drag_wnds[mouse_button]->SizeMove(
                offset_pos,
                drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_TOP:
            drag_wnds[mouse_button]->SizeMove(
                Pt(drag_wnds[mouse_button]->RelativeUpperLeft().x,
                   offset_pos.y),
                drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_TOPRIGHT:
            drag_wnds[mouse_button]->SizeMove(
                Pt(drag_wnds[mouse_button]->RelativeUpperLeft().x,
                   offset_pos.y),
                Pt(offset_pos.x,
                   drag_wnds[mouse_button]->RelativeLowerRight().y));
            break;
        case WR_MIDLEFT:
            drag_wnds[mouse_button]->SizeMove(
                Pt(offset_pos.x,
                   drag_wnds[mouse_button]->RelativeUpperLeft().y),
                drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_MIDRIGHT:
            drag_wnds[mouse_button]->SizeMove(
                drag_wnds[mouse_button]->RelativeUpperLeft(),
                Pt(offset_pos.x,
                   drag_wnds[mouse_button]->RelativeLowerRight().y));
            break;
        case WR_BOTTOMLEFT:
            drag_wnds[mouse_button]->SizeMove(
                Pt(offset_pos.x,
                   drag_wnds[mouse_button]->RelativeUpperLeft().y),
                Pt(drag_wnds[mouse_button]->RelativeLowerRight().x,
                   offset_pos.y));
            break;
        case WR_BOTTOM:
            drag_wnds[mouse_button]->SizeMove(
                drag_wnds[mouse_button]->RelativeUpperLeft(),
                Pt(drag_wnds[mouse_button]->RelativeLowerRight().x,
                   offset_pos.y));
            break;
        case WR_BOTTOMRIGHT:
            drag_wnds[mouse_button]->SizeMove(
                drag_wnds[mouse_button]->RelativeUpperLeft(),
                offset_pos);
            break;
        default:
            break;
        }
    }
}

void GUIImpl::HandleRelease(int mouse_button, const GG::Pt& pos, int curr_ticks)
{
    curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys);
    last_button_down_repeat_time = 0;
    prev_wnd_drag_position = Pt();
    browse_info_wnd.reset();
    browse_target = 0;
    prev_wnd_under_cursor_time = curr_ticks;

    Wnd* click_wnd = drag_wnds[mouse_button];
    curr_wnd_under_cursor = zlist.Pick(pos, GUI::s_gui->ModalWindow(), curr_drag_wnd_dragged ? click_wnd : 0);
    button_state[mouse_button] = false;
    drag_wnds[mouse_button] = 0; // if the mouse button is released, stop the tracking the drag window
    wnd_region = WR_NONE;        // and clear this, just in case
    // if the release is over the Wnd where the button-down event occurred, and that Wnd has not been dragged
    if (click_wnd && curr_wnd_under_cursor == click_wnd) {
        // if this is second click over a window that just received an click within
        // the time limit -- it's a double-click, not a click
        if (double_click_time > 0 && double_click_wnd == click_wnd &&
            double_click_button == 0) {
            double_click_wnd = 0;
            double_click_start_time = -1;
            double_click_time = -1;
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDoubleClick, mouse_button), pos, mod_keys));
        } else {
            if (double_click_time > 0) {
                double_click_wnd = 0;
                double_click_start_time = -1;
                double_click_time = -1;
            } else {
                double_click_start_time = curr_ticks;
                double_click_time = 0;
                double_click_wnd = click_wnd;
                double_click_button = 0;
            }
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LClick, mouse_button), pos, mod_keys));
        }
    } else {
        double_click_wnd = 0;
        double_click_time = -1;
        if (click_wnd)
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonUp, mouse_button), pos, mod_keys));
        if (curr_wnd_under_cursor) {
            std::list<Wnd*> drag_wnds;
            if (drag_drop_wnds.empty()) {
                if (click_wnd && click_wnd->DragDropDataType() != "") {
                    drag_wnds.push_back(click_wnd);
                    drag_drop_originating_wnd = click_wnd->Parent();
                    curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                    curr_drag_drop_here_wnd = 0;
                    curr_wnd_under_cursor->AcceptDrops(drag_wnds, pos);
                    if (drag_drop_originating_wnd) {
                        std::list<Wnd*> unaccepted_wnds;
                        if (drag_wnds.empty())
                            unaccepted_wnds.push_back(click_wnd);
                        drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                        drag_drop_originating_wnd->ChildrenDraggedAway(drag_wnds, curr_wnd_under_cursor);
                    }
                }
            } else {
                for (std::map<Wnd*, Pt>::iterator it = drag_drop_wnds.begin();
                     it != drag_drop_wnds.end();
                     ++it) {
                    drag_wnds.push_back(it->first);
                }
                std::list<Wnd*> all_wnds = drag_wnds;
                curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                curr_drag_drop_here_wnd = 0;
                curr_wnd_under_cursor->AcceptDrops(drag_wnds, pos);
                if (drag_drop_originating_wnd) {
                    std::list<Wnd*> unaccepted_wnds;
                    std::set_difference(all_wnds.begin(), all_wnds.end(),
                                        drag_wnds.begin(), drag_wnds.end(),
                                        std::back_inserter(unaccepted_wnds));
                    drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                    drag_drop_originating_wnd->ChildrenDraggedAway(drag_wnds, curr_wnd_under_cursor);
                }
            }
        }
    }
    drag_drop_originating_wnd = 0;
    drag_drop_wnds.clear();
    prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
    curr_drag_wnd_dragged = false;
}

// static member(s)
GUI*                       GUI::s_gui = 0;
boost::shared_ptr<GUIImpl> GUI::s_impl;

// member functions
GUI::GUI(const std::string& app_name)
{
    assert(!s_gui);
    s_gui = this;
    assert(!s_impl);
    s_impl.reset(new GUIImpl());
    s_impl->app_name = app_name;
}

GUI::~GUI()
{ Wnd::s_default_browse_info_wnd.reset(); }

Wnd* GUI::FocusWnd() const
{
    return s_impl->modal_wnds.empty() ? s_impl->focus_wnd : s_impl->modal_wnds.back().second;
}

Wnd* GUI::GetWindowUnder(const Pt& pt) const
{
#if INSTRUMENT_GET_WINDOW_UNDER
    if (Wnd* w = s_impl->zlist.Pick(pt, ModalWindow()))
        std::cerr << "GUI::GetWindowUnder() : " << w->WindowText() << " @ " << w << std::endl;
#endif
    return s_impl->zlist.Pick(pt, ModalWindow());
}

int GUI::DeltaT() const
{
    return s_impl->delta_t;
}

bool GUI::RenderingDragDropWnds() const
{
    return s_impl->rendering_drag_drop_wnds;
}

bool GUI::FPSEnabled() const
{
    return s_impl->calc_FPS;
}

double GUI::FPS() const
{
    return s_impl->FPS;
}

std::string GUI::FPSString() const
{
    return boost::io::str(boost::format("%.2f frames per second") % s_impl->FPS);
}

double GUI::MaxFPS() const
{
    return s_impl->max_FPS;
}

int GUI::ButtonDownRepeatDelay() const
{
    return s_impl->button_down_repeat_delay;
}

int GUI::ButtonDownRepeatInterval() const
{
    return s_impl->button_down_repeat_interval;
}

int GUI::DoubleClickInterval() const
{
    return s_impl->double_click_interval;
}

int GUI::MinDragTime() const
{
    return s_impl->min_drag_time;
}

int GUI::MinDragDistance() const
{
    return s_impl->min_drag_distance;
}

bool GUI::DragDropWnd(const Wnd* wnd) const
{
    return s_impl->drag_drop_wnds.find(const_cast<Wnd*>(wnd)) != s_impl->drag_drop_wnds.end();
}

bool GUI::MouseButtonDown(int bn) const
{
    return (bn >= 0 && bn <= 2) ? s_impl->button_state[bn] : false;
}

Pt GUI::MousePosition() const
{
    return s_impl->mouse_pos;
}

Pt GUI::MouseMovement() const
{
    return s_impl->mouse_rel;
}

Flags<ModKey> GUI::ModKeys() const
{
    return s_impl->mod_keys;
}

std::set<std::pair<int, int> > GUI::FindWords(const std::string& str) const
{
    std::set<std::pair<int, int> > retval;
    using namespace boost::xpressive;
    sregex_iterator it(str.begin(), str.end(), WORD_REGEX);
    sregex_iterator end_it;
    for ( ; it != end_it; ++it) {
        std::pair<int, int> indices;
        indices.first = it->position();
        indices.second = indices.first + it->length();
        retval.insert(indices);
    }
    return retval;
}

const boost::shared_ptr<StyleFactory>& GUI::GetStyleFactory() const
{
    return s_impl->style_factory;
}

bool GUI::RenderCursor() const
{
    return s_impl->render_cursor;
}

const boost::shared_ptr<Cursor>& GUI::GetCursor() const
{
    return s_impl->cursor;
}

GUI::const_accel_iterator GUI::accel_begin() const
{
    const GUIImpl* impl = s_impl.get();
    return impl->accelerators.begin();
}

GUI::const_accel_iterator GUI::accel_end() const
{
    const GUIImpl* impl = s_impl.get();
    return impl->accelerators.end();
}

GUI::AcceleratorSignalType& GUI::AcceleratorSignal(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/) const
{
    boost::shared_ptr<AcceleratorSignalType>& sig_ptr = s_impl->accelerator_sigs[std::make_pair(key, mod_keys)];
    if (!sig_ptr)
        sig_ptr.reset(new AcceleratorSignalType());
    return *sig_ptr;
}

void GUI::operator()()
{
    Run();
}

void GUI::HandleGGEvent(EventType event, Key key, Flags<ModKey> mod_keys, const Pt& pos, const Pt& rel)
{
    s_impl->mod_keys = mod_keys;

    int curr_ticks = Ticks();

    // track double-click time and time-out any pending double-click that has outlived its interval
    if (s_impl->double_click_time >= 0) {
        s_impl->double_click_time = curr_ticks - s_impl->double_click_start_time;
        if (s_impl->double_click_time >= s_impl->double_click_interval) {
            s_impl->double_click_start_time = -1;
            s_impl->double_click_time = -1;
            s_impl->double_click_wnd = 0;
        }
    }

    switch (event) {
    case IDLE: {
        if ((s_impl->curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys))) {
            if (s_impl->button_down_repeat_delay && s_impl->curr_wnd_under_cursor->RepeatButtonDown() &&
                s_impl->drag_wnds[0] == s_impl->curr_wnd_under_cursor) { // convert to a button-down message
                // ensure that the timing requirements are met
                if (curr_ticks - s_impl->prev_button_press_time > s_impl->button_down_repeat_delay) {
                    if (!s_impl->last_button_down_repeat_time ||
                        curr_ticks - s_impl->last_button_down_repeat_time > s_impl->button_down_repeat_interval) {
                        s_impl->last_button_down_repeat_time = curr_ticks;
                        s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::LButtonDown, pos, mod_keys));
                    }
                }
            } else {
                ProcessBrowseInfo();
            }
        }
        break; }
    case KEYPRESS: {
        s_impl->browse_info_wnd.reset();
        s_impl->browse_info_mode = -1;
        s_impl->browse_target = 0;
        bool processed = false;
        // only process accelerators when there are no modal windows active; otherwise, accelerators would be an end-run
        // around modality
        if (s_impl->modal_wnds.empty()) {
            // the focus_wnd may care about the state of the numlock and capslock, or which side of the keyboard's CTRL,
            // SHIFT, etc. was pressed, but the accelerators don't
            Flags<ModKey> massaged_mods = MassagedAccelModKeys(mod_keys);
            if (s_impl->accelerators.find(std::make_pair(key, massaged_mods)) != s_impl->accelerators.end())
                processed = AcceleratorSignal(key, massaged_mods)();
        }
        if (!processed && FocusWnd())
            FocusWnd()->HandleEvent(WndEvent(WndEvent::KeyPress, key, mod_keys));
        break; }
    case MOUSEMOVE: {
        s_impl->curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys);

        s_impl->mouse_pos = pos; // record mouse position
        s_impl->mouse_rel = rel; // record mouse movement

        if (s_impl->drag_wnds[0] || s_impl->drag_wnds[1] || s_impl->drag_wnds[2]) {
            if (s_impl->drag_wnds[0])
                s_impl->HandleDrag(0, pos, curr_ticks);
            if (s_impl->drag_wnds[1])
                s_impl->HandleDrag(1, pos, curr_ticks);
            if (s_impl->drag_wnds[2])
                s_impl->HandleDrag(2, pos, curr_ticks);
        } else if (s_impl->curr_wnd_under_cursor && s_impl->prev_wnd_under_cursor == s_impl->curr_wnd_under_cursor) { // if !s_impl->drag_wnds[0] and we're moving over the same (valid) object we were during the last iteration
            s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseHere, pos, mod_keys));
            ProcessBrowseInfo();
        }
        if (s_impl->prev_wnd_under_cursor != s_impl->curr_wnd_under_cursor) {
            s_impl->browse_info_wnd.reset();
            s_impl->browse_target = 0;
            s_impl->prev_wnd_under_cursor_time = curr_ticks;
        }
        s_impl->prev_wnd_under_cursor = s_impl->curr_wnd_under_cursor; // update this for the next time around
        break; }
    case LPRESS:
    case MPRESS:
    case RPRESS:
        s_impl->HandlePress(event - LPRESS, pos, curr_ticks);
        break;
    case LRELEASE:
    case MRELEASE:
    case RRELEASE:
        s_impl->HandleRelease(event - LRELEASE, pos, curr_ticks);
        break;
    case MOUSEWHEEL: {
        s_impl->curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys);
        s_impl->browse_info_wnd.reset();
        s_impl->browse_target = 0;
        s_impl->prev_wnd_under_cursor_time = curr_ticks;
        // don't send out 0-movement wheel messages, or send wheel messages when a button is depressed
        if (s_impl->curr_wnd_under_cursor && rel.y && !(s_impl->button_state[0] || s_impl->button_state[1] || s_impl->button_state[2]))
            s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseWheel, pos, rel.y, mod_keys));
        s_impl->prev_wnd_under_cursor = s_impl->curr_wnd_under_cursor; // update this for the next time around
        break; }
    default:
        break;
    }
}

void GUI::SetFocusWnd(Wnd* wnd)
{
    // inform old focus wnd that it is losing focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::LosingFocus));

    (s_impl->modal_wnds.empty() ? s_impl->focus_wnd : s_impl->modal_wnds.back().second) = wnd;

    // inform new focus wnd that it is gaining focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::GainingFocus));
}

void GUI::Wait(int ms)
{
    boost::xtime t;
    boost::xtime_get(&t, boost::TIME_UTC);
    int ns_sum = t.nsec + ms * 1000000;
    const int NANOSECONDS_PER_SECOND = 1000000000;
    int delta_secs = ns_sum / NANOSECONDS_PER_SECOND;
    int nanosecs = ns_sum % NANOSECONDS_PER_SECOND;
    t.sec += delta_secs;
    t.nsec = nanosecs;
    boost::thread::sleep(t);
}

void GUI::Register(Wnd* wnd)
{
    if (wnd) s_impl->zlist.Add(wnd);
}

void GUI::RegisterModal(Wnd* wnd)
{
    if (wnd && wnd->Modal()) {
        s_impl->modal_wnds.push_back(std::make_pair(wnd, wnd));
        wnd->HandleEvent(WndEvent(WndEvent::GainingFocus));
    }
}

void GUI::Remove(Wnd* wnd)
{
    if (wnd) {
        if (!s_impl->modal_wnds.empty() && s_impl->modal_wnds.back().first == wnd) // if it's the current modal window, remove it from the modal list
            s_impl->modal_wnds.pop_back();
        else // if it's not a modal window, remove it from the z-order
            s_impl->zlist.Remove(wnd);
    }
}

void GUI::WndDying(Wnd* wnd)
{
    if (wnd) {
        Remove(wnd);
        if (MatchesOrContains(wnd, s_impl->focus_wnd))
            s_impl->focus_wnd = 0;
        for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->modal_wnds.begin(); it != s_impl->modal_wnds.end(); ++it) {
            if (MatchesOrContains(wnd, it->second)) {
                if (MatchesOrContains(wnd, it->first)) {
                    it->second = 0;
                } else { // if the modal window for the removed window's focus level is available, revert focus to the modal window
                    if ((it->second = it->first))
                        it->first->HandleEvent(WndEvent(WndEvent::GainingFocus));
                }
            }
        }
        if (MatchesOrContains(wnd, s_impl->prev_wnd_under_cursor))
            s_impl->prev_wnd_under_cursor = 0;
        if (MatchesOrContains(wnd, s_impl->curr_wnd_under_cursor))
            s_impl->curr_wnd_under_cursor = 0;
        if (MatchesOrContains(wnd, s_impl->drag_wnds[0])) {
            s_impl->drag_wnds[0] = 0;
            s_impl->wnd_region = WR_NONE;
        }
        if (MatchesOrContains(wnd, s_impl->drag_wnds[1])) {
            s_impl->drag_wnds[1] = 0;
            s_impl->wnd_region = WR_NONE;
        }
        if (MatchesOrContains(wnd, s_impl->drag_wnds[2])) {
            s_impl->drag_wnds[2] = 0;
            s_impl->wnd_region = WR_NONE;
        }
        if (MatchesOrContains(wnd, s_impl->curr_drag_drop_here_wnd))
            s_impl->curr_drag_drop_here_wnd = 0;
        if (MatchesOrContains(wnd, s_impl->drag_drop_originating_wnd))
            s_impl->drag_drop_originating_wnd = 0;
        s_impl->drag_drop_wnds.erase(wnd);
        if (MatchesOrContains(wnd, s_impl->double_click_wnd)) {
            s_impl->double_click_wnd = 0;
            s_impl->double_click_start_time = -1;
            s_impl->double_click_time = -1;
        }
        for (std::set<Timer*>::iterator it = s_impl->timers.begin(); it != s_impl->timers.end(); ++it) {
            (*it)->Disconnect(wnd);
        }
    }
}

void GUI::EnableFPS(bool b/* = true*/)
{
    s_impl->calc_FPS = b;
    if (!b) 
        s_impl->FPS = -1.0f;
}

void GUI::SetMaxFPS(double max)
{
    if (max && max < 0.1)
        max = 0.1;
    s_impl->max_FPS = max;
}

void GUI::MoveUp(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveUp(wnd);
}

void GUI::MoveDown(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveDown(wnd);
}

boost::shared_ptr<ModalEventPump> GUI::CreateModalEventPump(bool& done)
{
    return boost::shared_ptr<ModalEventPump>(new ModalEventPump(done));
}

void GUI::RegisterDragDropWnd(Wnd* wnd, const Pt& offset, Wnd* originating_wnd)
{
    assert(wnd);
    if (!s_impl->drag_drop_wnds.empty() && originating_wnd != s_impl->drag_drop_originating_wnd) {
        throw std::runtime_error("GUI::RegisterDragDropWnd() : Attempted to register a drag drop item dragged from "
                                 "one window, when another window already has items being dragged from it.");
    }
    s_impl->drag_drop_wnds[wnd] = offset;
    s_impl->drag_drop_originating_wnd = originating_wnd;
}

void GUI::CancelDragDrop()
{
    s_impl->drag_drop_wnds.clear();
}

void GUI::RegisterTimer(Timer& timer)
{
    s_impl->timers.insert(&timer);
}

void GUI::RemoveTimer(Timer& timer)
{
    s_impl->timers.erase(&timer);
}

void GUI::EnableMouseButtonDownRepeat(int delay, int interval)
{
    if (!delay) { // setting delay = 0 completely disables mouse drag repeat
        s_impl->button_down_repeat_delay = 0;
        s_impl->button_down_repeat_interval = 0;
    } else {
        s_impl->button_down_repeat_delay = delay;
        s_impl->button_down_repeat_interval = interval;
    }
}

void GUI::SetDoubleClickInterval(int interval)
{
    s_impl->double_click_interval = interval;
}

void GUI::SetMinDragTime(int time)
{
    s_impl->min_drag_time = time;
}

void GUI::SetMinDragDistance(int distance)
{
    s_impl->min_drag_distance = distance;
}

void GUI::SetAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    s_impl->accelerators.insert(std::make_pair(key, mod_keys));
}

void GUI::RemoveAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    s_impl->accelerators.erase(std::make_pair(key, mod_keys));
}

boost::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, int pts, unsigned int range/* = Font::ALL_CHARS*/)
{
    return GetFontManager().GetFont(font_filename, pts, range);
}

void GUI::FreeFont(const std::string& font_filename, int pts)
{
    GetFontManager().FreeFont(font_filename, pts);
}

boost::shared_ptr<Texture> GUI::StoreTexture(Texture* texture, const std::string& texture_name)
{
    return GetTextureManager().StoreTexture(texture, texture_name);
}

boost::shared_ptr<Texture> GUI::StoreTexture(const boost::shared_ptr<Texture>& texture, const std::string& texture_name)
{
    return GetTextureManager().StoreTexture(texture, texture_name);
}

boost::shared_ptr<Texture> GUI::GetTexture(const std::string& name, bool mipmap/* = false*/)
{
    return GetTextureManager().GetTexture(name, mipmap);
}

void GUI::FreeTexture(const std::string& name)
{
    GetTextureManager().FreeTexture(name);
}

void GUI::SetStyleFactory(const boost::shared_ptr<StyleFactory>& factory)
{
    s_impl->style_factory = factory;
    if (!s_impl->style_factory)
        s_impl->style_factory.reset(new StyleFactory());
}

void GUI::RenderCursor(bool render)
{
    s_impl->render_cursor = render;
}

void GUI::SetCursor(const boost::shared_ptr<Cursor>& cursor)
{
    s_impl->cursor = cursor;
}

void GUI::SaveWnd(const Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
{
    if (!s_impl->save_wnd_fn)
        throw BadFunctionPointer("GUI::SaveWnd() : Attempted call on null function pointer.");
    s_impl->save_wnd_fn(wnd, name, ar);
}

void GUI::LoadWnd(Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    if (!s_impl->load_wnd_fn)
        throw BadFunctionPointer("GUI::LoadWnd() : Attempted call on null function pointer.");
    s_impl->load_wnd_fn(wnd, name, ar);
}

void GUI::SetSaveWndFunction(SaveWndFn fn)
{
    s_impl->save_wnd_fn = fn;
}

void GUI::SetLoadWndFunction(LoadWndFn fn)
{
    s_impl->load_wnd_fn = fn;
}

void GUI::SetSaveLoadFunctions(const PluginInterface& interface)
{
    s_impl->save_wnd_fn = interface.SaveWnd;
    s_impl->load_wnd_fn = interface.LoadWnd;
}

GUI* GUI::GetGUI()
{
    return s_gui;
}

void GUI::RenderWindow(Wnd* wnd)
{
    if (wnd && wnd->Visible()) {
        wnd->Render();
        bool clip = wnd->ClipChildren();
        if (clip)
            wnd->BeginClipping();
        for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != wnd->m_children.end(); ++it) {
            if ((*it)->Visible())
                RenderWindow(*it);
        }
        if (clip)
            wnd->EndClipping();
    }
}

void GUI::ProcessBrowseInfo()
{
    assert(s_impl->curr_wnd_under_cursor);
    if (!s_impl->button_state[0] && !s_impl->button_state[1] && !s_impl->button_state[2] &&
        (s_impl->modal_wnds.empty() || s_impl->curr_wnd_under_cursor->RootParent() == s_impl->modal_wnds.back().first)) {
        Wnd* wnd = s_impl->curr_wnd_under_cursor;
        while (!ProcessBrowseInfoImpl(wnd) && wnd->Parent() && (dynamic_cast<Control*>(wnd) || dynamic_cast<Layout*>(wnd))) {
            wnd = wnd->Parent();
        }
    }
}

void GUI::Render()
{
    // handle timers
    int ticks = Ticks();
    for (std::set<Timer*>::iterator it = s_impl->timers.begin(); it != s_impl->timers.end(); ++it) {
        Timer* timer = *it;
        if (timer->ShouldFire(ticks)) {
            const std::set<Wnd*>& wnds = timer->Wnds();
            for (std::set<Wnd*>::const_iterator wnd_it = wnds.begin(); wnd_it != wnds.end(); ++wnd_it) {
                (*wnd_it)->HandleEvent(WndEvent(WndEvent::TimerFiring, ticks, timer));
            }
        }
    }

    Enter2DMode();
    // render normal windows back-to-front
    for (ZList::reverse_iterator it = s_impl->zlist.rbegin(); it != s_impl->zlist.rend(); ++it) {
        RenderWindow(*it);
    }
    // render modal windows back-to-front
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->modal_wnds.begin(); it != s_impl->modal_wnds.end(); ++it) {
        RenderWindow(it->first);
    }
    // render the active browse info window, if any
    if (s_impl->browse_info_wnd) {
        if (!s_impl->curr_wnd_under_cursor) {
            s_impl->browse_info_wnd.reset();
            s_impl->browse_info_mode = -1;
            s_impl->browse_target = 0;
            s_impl->prev_wnd_under_cursor_time = Ticks();
        } else {
            assert(s_impl->browse_target);
            s_impl->browse_info_wnd->Update(s_impl->browse_info_mode, s_impl->browse_target);
            RenderWindow(s_impl->browse_info_wnd.get());
        }
    }
    // render drag-and-drop windows in arbitrary order (sorted by pointer value)
    s_impl->rendering_drag_drop_wnds = true;
    for (std::map<Wnd*, Pt>::const_iterator it = s_impl->drag_drop_wnds.begin(); it != s_impl->drag_drop_wnds.end(); ++it) {
        bool old_visible = it->first->Visible();
        if (!old_visible)
            it->first->Show();
        Pt parent_offset = it->first->Parent() ? it->first->Parent()->ClientUpperLeft() : Pt();
        Pt old_pos = it->first->UpperLeft() - parent_offset;
        it->first->MoveTo(s_impl->mouse_pos - parent_offset - it->second);
        RenderWindow(it->first);
        it->first->MoveTo(old_pos);
        if (!old_visible)
            it->first->Hide();
    }
    s_impl->rendering_drag_drop_wnds = false;
    if (s_impl->render_cursor && s_impl->cursor)
        s_impl->cursor->Render(s_impl->mouse_pos);
    Exit2DMode();
}

bool GUI::ProcessBrowseInfoImpl(Wnd* wnd)
{
    bool retval = true;
    const std::vector<Wnd::BrowseInfoMode>& browse_modes = wnd->BrowseModes();
    int delta_t = Ticks() - s_impl->prev_wnd_under_cursor_time;
    for (int i = static_cast<int>(browse_modes.size()) - 1; 0 <= i; --i) {
        if (browse_modes[i].time < delta_t) {
            if (browse_modes[i].wnd && browse_modes[i].wnd->WndHasBrowseInfo(wnd, i)) {
                if (s_impl->browse_target != wnd || s_impl->browse_info_wnd != browse_modes[i].wnd || s_impl->browse_info_mode != i) {
                    s_impl->browse_target = wnd;
                    s_impl->browse_info_wnd = browse_modes[i].wnd;
                    s_impl->browse_info_mode = i;
                    s_impl->browse_info_wnd->SetCursorPosition(s_impl->mouse_pos);
                }
            } else {
                retval = false;
            }
            break;
        }
    }
    return retval;
}

Wnd* GUI::ModalWindow() const
{
    Wnd* retval = 0;
    if (!s_impl->modal_wnds.empty())
        retval = s_impl->modal_wnds.back().first;
    return retval;
}

Wnd* GUI::CheckedGetWindowUnder(const Pt& pt, Flags<ModKey> mod_keys)
{
    Wnd* w = GetWindowUnder(pt);
    bool unregistered_drag_drop =
        s_impl->curr_drag_wnd_dragged && !s_impl->drag_wnds[0]->DragDropDataType().empty();
    bool registered_drag_drop = !s_impl->drag_drop_wnds.empty();
    std::map<Wnd*, Pt> drag_drop_wnds;
    drag_drop_wnds[s_impl->drag_wnds[0]] = s_impl->wnd_drag_offset;
    if (s_impl->curr_drag_drop_here_wnd && !unregistered_drag_drop && !registered_drag_drop) {
        s_impl->curr_drag_drop_here_wnd->HandleEvent(WndEvent(WndEvent::DragDropLeave));
        s_impl->curr_drag_drop_here_wnd = 0;
    }
    if (w != s_impl->curr_wnd_under_cursor) {
        if (s_impl->curr_wnd_under_cursor) {
            if (unregistered_drag_drop) {
                s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                s_impl->curr_drag_drop_here_wnd = 0;
            } else if (registered_drag_drop) {
                s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                s_impl->curr_drag_drop_here_wnd = 0;
            } else {
                s_impl->curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseLeave));
            }
        }
        if (w) {
            if (unregistered_drag_drop) {
                w->HandleEvent(WndEvent(WndEvent::DragDropEnter, pt, drag_drop_wnds, mod_keys));
                s_impl->curr_drag_drop_here_wnd = w;
            } else if (registered_drag_drop) {
                w->HandleEvent(WndEvent(WndEvent::DragDropEnter, pt, s_impl->drag_drop_wnds, mod_keys));
                s_impl->curr_drag_drop_here_wnd = w;
            } else {
                w->HandleEvent(WndEvent(WndEvent::MouseEnter, pt, mod_keys));
            }
        }
    }
    return w;
}

void GUI::SetFPS(double FPS)
{
    s_impl->FPS = FPS;
}

void GUI::SetDeltaT(int delta_t)
{
    s_impl->delta_t = delta_t;
}


bool GG::MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd)
{
    if (rwnd) {
        for (const Wnd* w = rwnd; w; w = w->Parent()) {
            if (w == lwnd)
                return true;
        }
    } else if (rwnd == lwnd) {
        return true;
    }
    return false;
}
