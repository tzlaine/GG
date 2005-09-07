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

#include "GGApp.h"

#include "GGBrowseInfoWnd.h"
#include "GGPluginInterface.h"
#include "GGZList.h"

#include <cassert>
#include <fstream>
#include <list>


using namespace GG;

namespace {
    // returns true if lwnd == rwnd or if lwnd contains rwnd
    inline bool MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd)
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

    /* returns the storage value of key_mods that should be used with keyboard accelerators the accelerators don't care
       which side of the keyboard you use for CTRL, SHIFT, etc., and whether or not the numlock or capslock are
       engaged.*/
    Uint32 MassagedAccelKeyMods(Uint32 key_mods)
    {
        key_mods &= ~(GGKMOD_NUM | GGKMOD_CAPS);
        if (key_mods & GGKMOD_CTRL)
            key_mods |= GGKMOD_CTRL;
        if (key_mods & GGKMOD_SHIFT)
            key_mods |= GGKMOD_SHIFT;
        if (key_mods & GGKMOD_ALT)
            key_mods |= GGKMOD_ALT;
        if (key_mods & GGKMOD_META)
            key_mods |= GGKMOD_META;
        return key_mods;
    }
}

// implementation data types
struct GG::AppImplData
{
    AppImplData() :
        focus_wnd(0),
        mouse_pos(0,0),
        mouse_rel(0,0),
        mouse_repeat_delay(0),
        mouse_repeat_interval(0),
        double_click_interval(500),
        min_drag_drop_time(500),
        min_drag_drop_distance(5),
        prev_wnd_under_cursor(0),
        curr_wnd_under_cursor(0),
        wnd_region(WR_NONE),
        delta_t(0),
        FPS(-1.0),
        calc_FPS(false),
        max_FPS(0.0),
        double_click_wnd(0),
        double_click_start_time(-1),
        double_click_time(-1),
        save_wnd_fn(0),
        load_wnd_fn(0)
    {
        button_state[0] = button_state[1] = button_state[2] = false;
        drag_wnds[0] = drag_wnds[1] = drag_wnds[2] = 0;
    }

    std::string  app_name;              // the user-defined name of the apllication

    ZList        zlist;                 // object that keeps the GUI windows in the correct depth ordering
    Wnd*         focus_wnd;             // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)
    std::list<std::pair<Wnd*, Wnd*> >
                 modal_wnds;            // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)

    bool         button_state[3];       // the up/down states of the three buttons on the mouse are kept here
    Pt           mouse_pos;             // absolute position of mouse based on last MOUSEMOVE event
    Pt           mouse_rel;             // relative position of mouse based on last MOUSEMOVE event

    int          mouse_repeat_delay;    // see note above App class definition
    int          mouse_repeat_interval;
    int          double_click_interval; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          min_drag_drop_time;
    int          min_drag_drop_distance;

    Wnd*         prev_wnd_under_cursor; // GUI window most recently under the input cursor; may be 0
    int          prev_wnd_under_cursor_time; // the time at which prev_wnd_under_cursor was initially set to its current value
    Wnd*         curr_wnd_under_cursor; // GUI window currently under the input cursor; may be 0
    Wnd*         drag_wnds[3];          // GUI window currently being clicked or dragged by each mouse button
    Pt           wnd_resize_offset;     // offset from the cursor of either the upper-left or lowe-right corner of the GUI window currently being resized
    WndRegion    wnd_region;            // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    boost::shared_ptr<BrowseInfoWnd>
                 browse_info_wnd;       // the current browse info window, if any
    int          browse_info_mode;      // the current browse info mode (only valid if browse_info_wnd is non-null)

    std::map<Wnd*, Pt>
               drag_drop_wnds;          // the Wnds (and their offsets) that are being dragged and dropped between Wnds

    std::set<std::pair<Key, Uint32> >
               accelerators;            // the keyboard accelerators

    std::map<std::pair<Key, Uint32>, boost::shared_ptr<App::AcceleratorSignalType> >
               accelerator_sigs;        // the signals emitted by the keyboard accelerators

    int        delta_t;                 // the number of ms since the last frame
    double     FPS;                     // the most recent calculation of the frames per second rendering speed (-1.0 if calcs are disabled)
    bool       calc_FPS;                // true iff FPS calcs are to be done
    double     max_FPS;                 // the maximum allowed frames per second rendering speed

    Wnd*       double_click_wnd;        // GUI window most recently clicked
    int        double_click_button;     // the index of the mouse button used in the last click
    int        double_click_start_time; // the time from which we started measuring double_click_time, in ms
    int        double_click_time;       // time elapsed since last click, in ms

    FontManager       font_manager;
    TextureManager    texture_manager;

    App::SaveWndFn    save_wnd_fn;
    App::LoadWndFn    load_wnd_fn;
};

// static member(s)
App*                           App::s_app = 0;
boost::shared_ptr<AppImplData> App::s_impl;

// member functions
App::App(const std::string& app_name)
{
    assert(!s_app);
    s_app = this;
    assert(!s_impl);
    s_impl.reset(new AppImplData());
    s_impl->app_name = app_name;
}

App::~App()
{
}

Wnd* App::FocusWnd() const
{
    return s_impl->modal_wnds.empty() ? s_impl->focus_wnd : s_impl->modal_wnds.back().second;
}

Wnd* App::GetWindowUnder(const Pt& pt) const
{
    return s_impl->zlist.Pick(pt, ModalWindow());
}

int App::DeltaT() const
{
    return s_impl->delta_t;
}

bool App::FPSEnabled() const
{
    return s_impl->calc_FPS;
}

double App::FPS() const
{
    return s_impl->FPS;
}

std::string App::FPSString() const
{
    char buf[128];
    sprintf(buf, "%.2f frames per second", s_impl->FPS);
    return std::string(buf);
}

double App::MaxFPS() const
{
    return s_impl->max_FPS;
}

int App::MouseRepeatDelay() const
{
    return s_impl->mouse_repeat_delay;
}

int App::MouseRepeatInterval() const
{
    return s_impl->mouse_repeat_interval;
}

int App::DoubleClickInterval() const
{
    return s_impl->double_click_interval;
}

int App::MinDragDropTime() const
{
    return s_impl->min_drag_drop_time;
}

int App::MinDragDropDistance() const
{
    return s_impl->min_drag_drop_distance;
}

bool App::MouseButtonDown(int bn) const
{
    return (bn >= 0 && bn <= 2) ? s_impl->button_state[bn] : false;
}

Pt App::MousePosition() const
{
    return s_impl->mouse_pos;
}

Pt App::MouseMovement() const
{
    return s_impl->mouse_rel;
}

App::const_accel_iterator App::accel_begin() const
{
    const AppImplData* impl = s_impl.get();
    return impl->accelerators.begin();
}

App::const_accel_iterator App::accel_end() const
{
    const AppImplData* impl = s_impl.get();
    return impl->accelerators.end();
}

App::AcceleratorSignalType& App::AcceleratorSignal(Key key, Uint32 key_mods) const
{
    boost::shared_ptr<AcceleratorSignalType>& sig_ptr = s_impl->accelerator_sigs[std::make_pair(key, key_mods)];
    if (!sig_ptr)
        sig_ptr.reset(new AcceleratorSignalType());
    return *sig_ptr;
}

void App::operator()()
{
    Run();
}

void App::SetFocusWnd(Wnd* wnd)
{
    // inform old focus wnd that it is losing focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(Wnd::Event(Wnd::Event::LosingFocus));

    (s_impl->modal_wnds.empty() ? s_impl->focus_wnd : s_impl->modal_wnds.back().second) = wnd;

    // inform new focus wnd that it is gaining focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(Wnd::Event(Wnd::Event::GainingFocus));
}

void App::Wait(int ms)
{
}

void App::Register(Wnd* wnd)
{
    if (wnd) s_impl->zlist.Add(wnd);
}

void App::RegisterModal(Wnd* wnd)
{
    if (wnd && wnd->Modal()) {
        s_impl->modal_wnds.push_back(std::make_pair(wnd, wnd));
        wnd->HandleEvent(Wnd::Event(Wnd::Event::GainingFocus));
    }
}

void App::Remove(Wnd* wnd)
{
    if (wnd) {
        if (!s_impl->modal_wnds.empty() && s_impl->modal_wnds.back().first == wnd) // if it's the current modal window, remove it from the modal list
            s_impl->modal_wnds.pop_back();
        else // if it's not a modal window, remove it from the z-order
            s_impl->zlist.Remove(wnd);

        // ensure that GUI state variables don't become dangling pointers when a Wnd is removed
        if (MatchesOrContains(wnd, s_impl->focus_wnd))
            s_impl->focus_wnd = 0;
        for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->modal_wnds.begin(); it != s_impl->modal_wnds.end(); ++it) {
            if (MatchesOrContains(wnd, it->second)) {
                if (MatchesOrContains(wnd, it->first)) {
                    it->second = 0;
                } else { // if the modal window for the removed window's focus level is available, revert focus to the modal window
                    if ((it->second = it->first))
                        it->first->HandleEvent(Wnd::Event(Wnd::Event::GainingFocus));
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
        if (MatchesOrContains(wnd, s_impl->double_click_wnd)) {
            s_impl->double_click_wnd = 0;
            s_impl->double_click_start_time = -1;
            s_impl->double_click_time = -1;
        }
        s_impl->drag_drop_wnds.erase(wnd);
    }
}

void App::EnableFPS(bool b/* = true*/)
{
    s_impl->calc_FPS = b;
    if (!b) 
        s_impl->FPS = -1.0f;
}

void App::SetMaxFPS(double max)
{
    if (max && max < 0.1)
        max = 0.1;
    s_impl->max_FPS = max;
}

void App::MoveUp(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveUp(wnd);
}

void App::MoveDown(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveDown(wnd);
}

void App::RegisterDragDropWnd(Wnd* wnd, const Pt& offset)
{
    s_impl->drag_drop_wnds[wnd] = offset;
}

void App::RemoveDragDropWnd(Wnd* wnd)
{
    s_impl->drag_drop_wnds.erase(wnd);
}

void App::ClearDragDropWnds()
{
    s_impl->drag_drop_wnds.clear();
}

void App::EnableMouseDragRepeat(int delay, int interval)
{
    if (!delay) { // setting delay = 0 completely disables mouse drag repeat
        s_impl->mouse_repeat_delay = 0;
        s_impl->mouse_repeat_interval = 0;
    } else {
        s_impl->mouse_repeat_delay = delay;
        s_impl->mouse_repeat_interval = interval;
    }
}

void App::SetDoubleClickInterval(int interval)
{
    s_impl->double_click_interval = interval;
}

void App::SetMinDragDropTime(int time)
{
    s_impl->min_drag_drop_time = time;
}

void App::SetMinDragDropDistance(int distance)
{
    s_impl->min_drag_drop_distance = distance;
}

void App::SetAccelerator(Key key, Uint32 key_mods)
{
    key_mods = MassagedAccelKeyMods(key_mods);
    s_impl->accelerators.insert(std::make_pair(key, key_mods));
}

void App::RemoveAccelerator(Key key, Uint32 key_mods)
{
    key_mods = MassagedAccelKeyMods(key_mods);
    s_impl->accelerators.erase(std::make_pair(key, key_mods));
}

boost::shared_ptr<Font> App::GetFont(const std::string& font_filename, int pts, Uint32 range/* = Font::ALL_CHARS*/)
{
    return s_impl->font_manager.GetFont(font_filename, pts, range);
}

void App::FreeFont(const std::string& font_filename, int pts)
{
    s_impl->font_manager.FreeFont(font_filename, pts);
}

boost::shared_ptr<Texture> App::StoreTexture(Texture* texture, const std::string& texture_name)
{
    return s_impl->texture_manager.StoreTexture(texture, texture_name);
}

boost::shared_ptr<Texture> App::StoreTexture(boost::shared_ptr<Texture> texture, const std::string& texture_name)
{
    return s_impl->texture_manager.StoreTexture(texture, texture_name);
}

boost::shared_ptr<Texture> App::GetTexture(const std::string& name, bool mipmap/* = false*/)
{
    return s_impl->texture_manager.GetTexture(name, mipmap);
}

void App::FreeTexture(const std::string& name)
{
    s_impl->texture_manager.FreeTexture(name);
}

void App::SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
{
    if (!s_impl->save_wnd_fn)
        throw std::runtime_error("App::SaveWnd() : Attempted call on null function pointer.");
    s_impl->save_wnd_fn(wnd, name, ar);
}

void App::LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    if (!s_impl->load_wnd_fn)
        throw std::runtime_error("App::LoadWnd() : Attempted call on null function pointer.");
    s_impl->load_wnd_fn(wnd, name, ar);
}

void App::SetSaveWndFunction(SaveWndFn fn)
{
    s_impl->save_wnd_fn = fn;
}

void App::SetLoadWndFunction(LoadWndFn fn)
{
    s_impl->load_wnd_fn = fn;
}

void App::SetSaveLoadFunctions(const PluginInterface& interface)
{
    s_impl->save_wnd_fn = interface.SaveWnd;
    s_impl->load_wnd_fn = interface.LoadWnd;
}

App* App::GetApp()
{
    return s_app;
}

void App::RenderWindow(Wnd* wnd)
{
    if (wnd->Render() == true) {
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

void App::HandleGGEvent(EventType event, Key key, Uint32 key_mods, const Pt& pos, const Pt& rel)
{
    Wnd*&       prev_wnd_under_cursor = s_impl->prev_wnd_under_cursor;
    int&        prev_wnd_under_cursor_time = s_impl->prev_wnd_under_cursor_time;
    Wnd*&       curr_wnd_under_cursor = s_impl->curr_wnd_under_cursor;
    Wnd**       drag_wnds = s_impl->drag_wnds;
    Pt&         wnd_resize_offset = s_impl->wnd_resize_offset;
    WndRegion&  wnd_region = s_impl->wnd_region;

    // track double-click time and time-out any pending double-click that has outlived its interval
    if (s_impl->double_click_time >= 0) {
        s_impl->double_click_time = Ticks() - s_impl->double_click_start_time;
        if (s_impl->double_click_time >= s_impl->double_click_interval) {
            s_impl->double_click_start_time = -1;
            s_impl->double_click_time = -1;
            s_impl->double_click_wnd = 0;
        }
    }

    switch (event) {
    case IDLE:{
        if (curr_wnd_under_cursor)
            ProcessBrowseInfo();
        break;}
    case KEYPRESS:{
        s_impl->browse_info_wnd.reset();
        s_impl->browse_info_mode = -1;
        bool processed = false;
        // only process accelerators when there are no modal windows active; otherwise, accelerators would be an end-run around modality
        if (s_impl->modal_wnds.empty()) {
            // the focus_wnd may care about the state of the numlock and capslock, or which side of the keyboard's 
            // CTRL, SHIFT, etc. was pressed, but the accelerators don't
            Uint32 massaged_mods = MassagedAccelKeyMods(key_mods);
            if (s_impl->accelerators.find(std::make_pair(key, massaged_mods)) != s_impl->accelerators.end())
                processed = AcceleratorSignal(key, massaged_mods)();
        }
        if (!processed && FocusWnd())
            FocusWnd()->HandleEvent(Wnd::Event(Wnd::Event::Keypress, key, key_mods));
        break;}
    case MOUSEMOVE:{
        curr_wnd_under_cursor = GetWindowUnder(pos); // get window under mouse position

        // record these
        s_impl->mouse_pos = pos; // mouse position
        s_impl->mouse_rel = rel; // mouse movement

        // then act on mouse motion
        if (drag_wnds[0]) { // only respond to left mouse button drags
            if (wnd_region == WR_MIDDLE || wnd_region == WR_NONE) { // send move message to window
                drag_wnds[0]->HandleEvent(Wnd::Event(Wnd::Event::LDrag, pos, rel, key_mods));
            } else if (drag_wnds[0]->Resizable()) { // send appropriate resize message to window
                Pt offset_pos = pos + wnd_resize_offset;
                switch (wnd_region)
                {
                case WR_TOPLEFT:
                    drag_wnds[0]->SizeMove(offset_pos, drag_wnds[0]->LowerRight());
                    break;
                case WR_TOP:
                    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, offset_pos.y,
                                           drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y);
                    break;
                case WR_TOPRIGHT:
                    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, offset_pos.y,
                                           offset_pos.x, drag_wnds[0]->LowerRight().y);
                    break;
                case WR_MIDLEFT:
                    drag_wnds[0]->SizeMove(offset_pos.x, drag_wnds[0]->UpperLeft().y,
                                           drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y);
                    break;
                case WR_MIDRIGHT:
                    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y,
                                           offset_pos.x, drag_wnds[0]->LowerRight().y);
                    break;
                case WR_BOTTOMLEFT:
                    drag_wnds[0]->SizeMove(offset_pos.x, drag_wnds[0]->UpperLeft().y,
                                           drag_wnds[0]->LowerRight().x, offset_pos.y);
                    break;
                case WR_BOTTOM:
                    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y,
                                           drag_wnds[0]->LowerRight().x, offset_pos.y);
                    break;
                case WR_BOTTOMRIGHT:
                    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft(), offset_pos);
                    break;
                default:
                    break;
                }
            } else if (drag_wnds[0]->DragKeeper()) {
                drag_wnds[0]->HandleEvent(Wnd::Event(Wnd::Event::LDrag, pos, rel, key_mods));
            }
        } else if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) { // if !drag_wnds[0] and we're moving over the same (valid) object we were during the last iteration
            curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseHere, pos, 0));
            ProcessBrowseInfo();
        } else { // if !drag_wnds[0] and prev_wnd_under_cursor != curr_wnd_under_cursor, we're just moving around
            if (prev_wnd_under_cursor) prev_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseLeave, pos, 0));
            if (curr_wnd_under_cursor) curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseEnter, pos, 0));
        }
        if (prev_wnd_under_cursor != curr_wnd_under_cursor) {
            s_impl->browse_info_wnd.reset();
            prev_wnd_under_cursor_time = Ticks();
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    case LPRESS:
    case MPRESS:
    case RPRESS:{
        curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
        s_impl->browse_info_wnd.reset();
        prev_wnd_under_cursor_time = Ticks();
        switch (event) {
        case LPRESS:{
            s_impl->button_state[0] = true;
            drag_wnds[0] = curr_wnd_under_cursor; // track this window as the one being dragged by the left mouse button
            // if this window is not a disabled Control window, it becomes the focus window
            Control* control = 0;
            if (drag_wnds[0] && (!(control = dynamic_cast<Control*>(drag_wnds[0])) || !control->Disabled()))
                SetFocusWnd(drag_wnds[0]);
            if (drag_wnds[0]) {
                wnd_region = drag_wnds[0]->WindowRegion(pos); // and determine whether a resize-region of it is being dragged
                if (wnd_region % 3 == 0) // left regions
                    wnd_resize_offset.x = drag_wnds[0]->UpperLeft().x - pos.x;
                else
                    wnd_resize_offset.x = drag_wnds[0]->LowerRight().x - pos.x;
                if (wnd_region < 3) // top regions
                    wnd_resize_offset.y = drag_wnds[0]->UpperLeft().y - pos.y;
                else
                    wnd_resize_offset.y = drag_wnds[0]->LowerRight().y - pos.y;
                Wnd* drag_wnds_root_parent = drag_wnds[0]->RootParent();
                MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : drag_wnds[0]); // move root window up to top of z-order
                drag_wnds[0]->HandleEvent(Wnd::Event(Wnd::Event::LButtonDown, pos, key_mods));
            }
            break;}
        case MPRESS:{
            s_impl->button_state[1] = true;
            break;}
        case RPRESS:{
            s_impl->button_state[2] = true;
            drag_wnds[2] = curr_wnd_under_cursor;  // track this window as the one being dragged by the right mouse button
            if (drag_wnds[2])
                drag_wnds[2]->HandleEvent(Wnd::Event(Wnd::Event::RButtonDown, pos, key_mods));
            break;}
        default: break;
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    case LRELEASE:
    case MRELEASE:
    case RRELEASE:{
        curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
        s_impl->browse_info_wnd.reset();
        prev_wnd_under_cursor_time = Ticks();
        switch (event) {
        case LRELEASE:{
            Wnd* click_wnd = drag_wnds[0];
            s_impl->drag_drop_wnds.clear();
            s_impl->button_state[0] = false;
            drag_wnds[0] = 0;       // if the mouse button is released, stop the tracking the drag window
            wnd_region = WR_NONE;   // and clear this, just in case
            if (click_wnd && curr_wnd_under_cursor == click_wnd) { // if the release is over the place where the button-down event occurred
                // if this is second l-click over a window that just received an l-click within
                // the time limit -- it's a double-click, not a click
                if (s_impl->double_click_time > 0 && s_impl->double_click_wnd == click_wnd &&
                    s_impl->double_click_button == 0) {
                    s_impl->double_click_wnd = 0;
                    s_impl->double_click_start_time = -1;
                    s_impl->double_click_time = -1;
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LDoubleClick, pos, key_mods));
                } else {
                    if (s_impl->double_click_time > 0) {
                        s_impl->double_click_wnd = 0;
                        s_impl->double_click_start_time = -1;
                        s_impl->double_click_time = -1;
                    } else {
                        s_impl->double_click_start_time = Ticks();
                        s_impl->double_click_time = 0;
                        s_impl->double_click_wnd = click_wnd;
                        s_impl->double_click_button = 0;
                    }
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LClick, pos, key_mods));
                }
            } else {
                s_impl->double_click_wnd = 0;
                s_impl->double_click_time = -1;
                if (click_wnd)
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LButtonUp, pos, key_mods));
            }
            break;}
        case MRELEASE:{
            s_impl->button_state[1] = false;
            s_impl->double_click_wnd = 0;
            s_impl->double_click_time = -1;
            break;}
        case RRELEASE:{
            Wnd* click_wnd = drag_wnds[2];
            s_impl->button_state[2] = false;
            drag_wnds[2] = 0;
            if (click_wnd && curr_wnd_under_cursor == click_wnd) { // if the release is over the place where the button-down event occurred
                // if this is second r-click over a window that just received an r-click within
                // the time limit -- it's a double-click, not a click
                if (s_impl->double_click_time > 0 && s_impl->double_click_wnd == click_wnd &&
                    s_impl->double_click_button == 2) {
                    s_impl->double_click_wnd = 0;
                    s_impl->double_click_time = -1;
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::RDoubleClick, pos, key_mods));
                } else {
                    if (s_impl->double_click_time > 0) {
                        s_impl->double_click_wnd = 0;
                        s_impl->double_click_time = -1;
                    } else {
                        s_impl->double_click_time = 0;
                        s_impl->double_click_wnd = click_wnd;
                        s_impl->double_click_button = 2;
                    }
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::RClick, pos, key_mods));
                }
            } else {
                s_impl->double_click_wnd = 0;
                s_impl->double_click_time = -1;
            }
            break;}
        default:
            break;
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    case MOUSEWHEEL:{
        curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
        s_impl->browse_info_wnd.reset();
        prev_wnd_under_cursor_time = Ticks();
        // don't send out 0-movement wheel messages, or send wheel messages when a button is depressed
        if (curr_wnd_under_cursor && rel.y && !(s_impl->button_state[0] || s_impl->button_state[1] || s_impl->button_state[2]))
            curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseWheel, pos, rel.y, key_mods));
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    default:
        break;
    }
}

void App::ProcessBrowseInfo()
{
    if (s_impl->modal_wnds.empty() || s_impl->curr_wnd_under_cursor->RootParent() == s_impl->modal_wnds.back().first) {
        const std::vector<Wnd::BrowseInfoMode>& browse_modes = s_impl->curr_wnd_under_cursor->BrowseModes();
        int delta_t = Ticks() - s_impl->prev_wnd_under_cursor_time;
        for (unsigned int i = 0; i < browse_modes.size(); ++i) {
            if (browse_modes[i].time < delta_t && s_impl->browse_info_wnd != browse_modes[i].wnd) {
                s_impl->browse_info_wnd = browse_modes[i].wnd;
                s_impl->browse_info_mode = i;
                s_impl->browse_info_wnd->MoveTo(s_impl->mouse_pos);
                break;
            }
        }
    }
}

void App::Render()
{
    Enter2DMode();
    // render normal windows back-to-front
    for (ZList::reverse_iterator it = s_impl->zlist.rbegin(); it != s_impl->zlist.rend(); ++it) {
        if ((*it)->Visible())
            RenderWindow(*it);
    }
    // render modal windows back-to-front
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->modal_wnds.begin(); it != s_impl->modal_wnds.end(); ++it) {
        if (it->first->Visible())
            RenderWindow(it->first);
    }
    // render the active browse info window, if any
    if (s_impl->browse_info_wnd) {
        s_impl->browse_info_wnd->Update(s_impl->browse_info_mode, s_impl->curr_wnd_under_cursor);
        RenderWindow(s_impl->browse_info_wnd.get());
    }
    // render drag-drop windows in arbitrary order (sorted by pointer value)
    for (std::map<Wnd*, Pt>::const_iterator it = s_impl->drag_drop_wnds.begin(); it != s_impl->drag_drop_wnds.end(); ++it) {
        if (it->first->Visible()) {
            Pt parent_offset = (it->first->Parent() ? it->first->Parent()->ClientUpperLeft() : Pt(0, 0));
            Pt old_pos = it->first->UpperLeft() - parent_offset;
            it->first->MoveTo(s_impl->mouse_pos - parent_offset - it->second);
            RenderWindow(it->first);
            it->first->MoveTo(old_pos);
        }
    }
    Exit2DMode();
}

Wnd* App::ModalWindow() const
{
    Wnd* retval = 0;
    if (!s_impl->modal_wnds.empty())
        retval = s_impl->modal_wnds.back().first;
    return retval;
}

void App::SetFPS(double FPS)
{
    s_impl->FPS = FPS;
}

void App::SetDeltaT(int delta_t)
{
    s_impl->delta_t = delta_t;
}
