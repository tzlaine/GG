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

#include "GGButton.h"
#include "GGDropDownList.h"
#include "GGDynamicGraphic.h"
#include "GGEdit.h"
#include "GGListBox.h"
#include "GGMenu.h"
#include "GGMultiEdit.h"
#include "GGScroll.h"
#include "GGSlider.h"
#include "GGSpin.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "GGWnd.h"
#include "GGZList.h"
#include "XMLObjectFactory.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>

#include <cassert>

using namespace GG;

namespace {
    // these generate Wnd-subclass objects for the Wnd factory
    Wnd* NewTextControl(const XMLElement& elem)       {return new TextControl(elem);}
    Wnd* NewStaticGraphic(const XMLElement& elem)     {return new StaticGraphic(elem);}
    Wnd* NewDynamicGraphic(const XMLElement& elem)    {return new DynamicGraphic(elem);}
    Wnd* NewButton(const XMLElement& elem)            {return new Button(elem);}
    Wnd* NewStateButton(const XMLElement& elem)       {return new StateButton(elem);}
    Wnd* NewRadioButtonGroup(const XMLElement& elem)  {return new RadioButtonGroup(elem);}
    Wnd* NewEdit(const XMLElement& elem)              {return new Edit(elem);}
    Wnd* NewScroll(const XMLElement& elem)            {return new Scroll(elem);}
    Wnd* NewListBox(const XMLElement& elem)           {return new ListBox(elem);}
    Wnd* NewListBoxRow(const XMLElement& elem)        {return new ListBox::Row(elem);}
    Wnd* NewMenuBar(const XMLElement& elem)           {return new MenuBar(elem);}
    Wnd* NewMultiEdit(const XMLElement& elem)         {return new MultiEdit(elem);}
    Wnd* NewDropDownList(const XMLElement& elem)      {return new DropDownList(elem);}
    Wnd* NewSpinInt(const XMLElement& elem)           {return new Spin<int>(elem);}
    Wnd* NewSpinDouble(const XMLElement& elem)        {return new Spin<double>(elem);}
    Wnd* NewSlider(const XMLElement& elem)            {return new Slider(elem);}

    // returns true if lwnd == rwnd or if lwnd contains rwnd
    inline bool MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd)
    {
        return (rwnd == lwnd || (rwnd && rwnd->RootParent() == lwnd));
    }

    /* returns the storage value of key_mods that should be used with keyboard accelerators
        the accelerators don't care which side of the keyboard you use for CTRL, SHIFT, etc.,
        and whether or not the numlock or capslock are engaged.*/
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
    log_category(log4cpp::Category::getRoot())
    {
        button_state[0] = button_state[1] = button_state[2] = false;
        drag_wnds[0] = drag_wnds[1] = drag_wnds[2] = 0;

        wnd_factory.AddGenerator("GG::TextControl", &NewTextControl);
        wnd_factory.AddGenerator("GG::StaticGraphic", &NewStaticGraphic);
        wnd_factory.AddGenerator("GG::DynamicGraphic", &NewDynamicGraphic);
        wnd_factory.AddGenerator("GG::Button", &NewButton);
        wnd_factory.AddGenerator("GG::StateButton", &NewStateButton);
        wnd_factory.AddGenerator("GG::RadioButtonGroup", &NewRadioButtonGroup);
        wnd_factory.AddGenerator("GG::Edit", &NewEdit);
        wnd_factory.AddGenerator("GG::Scroll", &NewScroll);
        wnd_factory.AddGenerator("GG::ListBox", &NewListBox);
        wnd_factory.AddGenerator("GG::ListBox::Row", &NewListBoxRow);
        wnd_factory.AddGenerator("GG::MenuBar", &NewMenuBar);
        wnd_factory.AddGenerator("GG::MultiEdit", &NewMultiEdit);
        wnd_factory.AddGenerator("GG::DropDownList", &NewDropDownList);
        wnd_factory.AddGenerator(Spin<int>::XMLTypeName(), &NewSpinInt);
        wnd_factory.AddGenerator(Spin<double>::XMLTypeName(), &NewSpinDouble);
        wnd_factory.AddGenerator("GG::Slider", &NewSlider);
    }

    string       app_name;              // the user-defined name of the apllication

    ZList        zlist;                 // object that keeps the GUI windows in the correct depth ordering
    Wnd*         focus_wnd;             // GUI window that currently has the input focus
    list<Wnd*>   modal_wnds;            // modal GUI windows (only the one in back is active, simulating a stack but allowing traversal of the list)

    bool   button_state[3];             // the up/down states of the three buttons on the mouse are kept here
    Pt     mouse_pos;                   // absolute position of mouse based on last MOUSEMOVE event
    Pt     mouse_rel;                   // relative position of mouse based on last MOUSEMOVE event

    int mouse_repeat_delay;             // see note above class definition
    int mouse_repeat_interval;
    int double_click_interval;          // the maximum interval allowed between clicks that is still considered a double-click, in ms

    Wnd*       prev_wnd_under_cursor;   // GUI window most recently under the input cursor; may be 0
    Wnd*       curr_wnd_under_cursor;   // GUI window currently under the input cursor; may be 0
    Wnd*       drag_wnds[3];            // GUI window currently being clicked or dragged by each button (on a mouse)
    WndRegion  wnd_region;              // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    map<Wnd*, Pt> drag_drop_wnds;       // the Wnds (and their offsets) that are being dragged and dropped between Wnds

    std::set<std::pair<Key, Uint32> >
               accelerators;            // the keyboard accelerators

    std::map<std::pair<Key, Uint32>, shared_ptr<App::AcceleratorSignalType> >
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

    XMLObjectFactory<Wnd> wnd_factory;  // object that creates Wnd-subclass objects from XML-formatted text

    log4cpp::Category&    log_category; // log4cpp object used to log events to file
};

// static member(s)
App*                     App::s_app = 0;
shared_ptr<AppImplData>  App::s_impl;

// member functions
App::App(const string& app_name)
{
    assert(!s_app);
    s_app = this;
    assert(!s_impl);
    s_impl.reset(new AppImplData());
    s_impl->app_name = app_name;

    const string GG_LOG_FILENAME(s_impl->app_name + ".log");

    // a platform-independent way to erase the old log
    ofstream temp(GG_LOG_FILENAME.c_str());
    temp.close();

    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", GG_LOG_FILENAME);
    log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
    layout->setConversionPattern("%d %p : %m%n");
    appender->setLayout(layout);
    s_impl->log_category.setAdditivity(false);  // make appender the only appender used...
    s_impl->log_category.setAppender(appender);
    s_impl->log_category.setAdditivity(true);   // ...but allow the addition of others later
    s_impl->log_category.setPriority(log4cpp::Priority::DEBUG);
}

App::~App()
{
    log4cpp::Category::shutdown();
}

Wnd* App::FocusWnd() const
{
    return s_impl->focus_wnd;
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

string App::FPSString() const
{
    char buf[128];
    sprintf(buf, "%.2f frames per second", s_impl->FPS);
    return string(buf);
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

Wnd* App::GenerateWnd(const XMLElement& elem) const
{
    return s_impl->wnd_factory.GenerateObject(elem);
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
    shared_ptr<AcceleratorSignalType>& sig_ptr = s_impl->accelerator_sigs[std::make_pair(key, key_mods)];
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
    if (s_impl->focus_wnd)
        s_impl->focus_wnd->HandleEvent(Wnd::Event(Wnd::Event::LosingFocus));

    s_impl->focus_wnd = wnd;

    // inform new focus wnd that it is gaining focus
    if (s_impl->focus_wnd)
        s_impl->focus_wnd->HandleEvent(Wnd::Event(Wnd::Event::GainingFocus));
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
        s_impl->modal_wnds.push_back(wnd);
    }
}

void App::Remove(Wnd* wnd)
{
    if (wnd) {
        if (!s_impl->modal_wnds.empty() && s_impl->modal_wnds.back() == wnd) { // if it's the current modal window, remove it from the modal list
            s_impl->modal_wnds.pop_back();
        } else { // if it's not a modal window, remove it from the z-order
            s_impl->zlist.Remove(wnd);
        }

        // ensure that GUI state variables don't become dangling pointers when a Wnd is removed
        if (MatchesOrContains(wnd, s_impl->focus_wnd))
            s_impl->focus_wnd = 0;
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

shared_ptr<Font> App::GetFont(const string& font_filename, int pts, Uint32 range/* = Font::ALL_CHARS*/)
{
    return s_impl->font_manager.GetFont(font_filename, pts, range);
}

void App::FreeFont(const string& font_filename, int pts)
{
    s_impl->font_manager.FreeFont(font_filename, pts);
}

shared_ptr<Texture> App::StoreTexture(Texture* texture, const string& texture_name)
{
    return s_impl->texture_manager.StoreTexture(texture, texture_name);
}

shared_ptr<Texture> App::StoreTexture(shared_ptr<Texture> texture, const string& texture_name)
{
    return s_impl->texture_manager.StoreTexture(texture, texture_name);
}

shared_ptr<Texture> App::GetTexture(const string& name, bool mipmap/* = false*/)
{
    return s_impl->texture_manager.GetTexture(name, mipmap);
}

void App::FreeTexture(const string& name)
{
    s_impl->texture_manager.FreeTexture(name);
}

void App::AddWndGenerator(const string& name, Wnd* (*fn)(const XMLElement&))
{
    s_impl->wnd_factory.AddGenerator(name, fn);
}

void App::RemoveWndGenerator(const string& name)
{
    s_impl->wnd_factory.RemoveGenerator(name);
}

log4cpp::Category& App::Logger()
{
    return s_impl->log_category;
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
            if ((*it)->Visible()) {
                RenderWindow(*it);
            }
        }
        if (clip)
            wnd->EndClipping();
    }
}

void App::HandleGGEvent(EventType event, Key key, Uint32 key_mods, const Pt& pos, const Pt& rel)
{
    Wnd*&       prev_wnd_under_cursor = s_impl->prev_wnd_under_cursor;
    Wnd*&       curr_wnd_under_cursor = s_impl->curr_wnd_under_cursor;
    Wnd**       drag_wnds = s_impl->drag_wnds;
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
    case KEYPRESS:{
        bool processed = false;
        // only process accelerators when there are no modal windows active; otherwise, accelerators would be an end-run around modality
        if (s_impl->modal_wnds.empty()) {
            // the focus_wnd may care about the state of the numlock and capslock, or which side of the keyboard's 
            // CTRL, SHIFT, etc. was pressed, but the accelerators don't
            Uint32 massaged_mods = MassagedAccelKeyMods(key_mods);
            if (s_impl->accelerators.find(std::make_pair(key, massaged_mods)) != s_impl->accelerators.end())
                processed = AcceleratorSignal(key, massaged_mods)();
        }
        if (!processed && s_impl->focus_wnd)
            s_impl->focus_wnd->HandleEvent(Wnd::Event(Wnd::Event::Keypress, key, key_mods));
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
                switch (wnd_region)
                {
                case WR_TOPLEFT:     drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft() + rel,
                                                            drag_wnds[0]->LowerRight()); break;
                case WR_TOP:         drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y + rel.y,
                                                            drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y); break;
                case WR_TOPRIGHT:    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y + rel.y,
                                                            drag_wnds[0]->LowerRight().x + rel.x, drag_wnds[0]->LowerRight().y); break;
                case WR_MIDLEFT:     drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x + rel.x, drag_wnds[0]->UpperLeft().y,
                                                            drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y); break;
                case WR_MIDRIGHT:    drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y,
                                                            drag_wnds[0]->LowerRight().x + rel.x, drag_wnds[0]->LowerRight().y); break;
                case WR_BOTTOMLEFT:  drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x + rel.x, drag_wnds[0]->UpperLeft().y,
                                                            drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y + rel.y); break;
                case WR_BOTTOM:      drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft().x, drag_wnds[0]->UpperLeft().y,
                                                            drag_wnds[0]->LowerRight().x, drag_wnds[0]->LowerRight().y + rel.y); break;
                case WR_BOTTOMRIGHT: drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft(), drag_wnds[0]->LowerRight() + Pt(rel.x, rel.y)); break;
                default: break;
                }
            } else if (drag_wnds[0]->DragKeeper()) {
                drag_wnds[0]->HandleEvent(Wnd::Event(Wnd::Event::LDrag, pos, rel, key_mods));
            }
        } else if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) { // if !drag_wnds[0] and we're moving over the same (valid) object we were during the last iteration
            if (curr_wnd_under_cursor) curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseHere, pos, 0));
        } else { // if !drag_wnds[0] and prev_wnd_under_cursor != curr_wnd_under_cursor, we're just moving around
            if (prev_wnd_under_cursor) prev_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseLeave, pos, 0));
            if (curr_wnd_under_cursor) curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseEnter, pos, 0));
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    case LPRESS:
    case MPRESS:
    case RPRESS:{
        curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
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
            if (drag_wnds[2]) {
                drag_wnds[2]->HandleEvent(Wnd::Event(Wnd::Event::RButtonDown, pos, key_mods));
            }
            break;}
        default: break;
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    case LRELEASE:
    case MRELEASE:
    case RRELEASE:{
        curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
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
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LDoubleClick, pos, key_mods));
                    s_impl->double_click_wnd = 0;
                    s_impl->double_click_start_time = -1;
                    s_impl->double_click_time = -1;
                } else {
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LClick, pos, key_mods));
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
                }
            } else {
                if (click_wnd)
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::LButtonUp, pos, key_mods));
                s_impl->double_click_wnd = 0;
                s_impl->double_click_time = -1;
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
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::RDoubleClick, pos, key_mods));
                    s_impl->double_click_wnd = 0;
                    s_impl->double_click_time = -1;
                } else {
                    click_wnd->HandleEvent(Wnd::Event(Wnd::Event::RClick, pos, key_mods));
                    if (s_impl->double_click_time > 0) {
                        s_impl->double_click_wnd = 0;
                        s_impl->double_click_time = -1;
                    } else {
                        s_impl->double_click_time = 0;
                        s_impl->double_click_wnd = click_wnd;
                        s_impl->double_click_button = 2;
                    }
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
        // don't send out 0-movement wheel messages, or send wheel messages when a button is depressed
        if (curr_wnd_under_cursor && rel.y && !(s_impl->button_state[0] || s_impl->button_state[1] || s_impl->button_state[2])) {
            curr_wnd_under_cursor->HandleEvent(Wnd::Event(Wnd::Event::MouseWheel, pos, rel.y, key_mods));
        }
        prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
        break;}
    default:
        break;
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
    for (std::list<Wnd*>::iterator it = s_impl->modal_wnds.begin(); it != s_impl->modal_wnds.end(); ++it) {
        if ((*it)->Visible())
            RenderWindow(*it);
    }
    // render drag-drop windows in arbitrary order (sorted by pointer value)
    for (map<Wnd*, Pt>::const_iterator it = s_impl->drag_drop_wnds.begin(); it != s_impl->drag_drop_wnds.end(); ++it) {
        if (it->first->Visible()) {
            Pt parent_offset = (it->first->Parent() ? it->first->Parent()->ClientUpperLeft() : Pt(0, 0));
            Pt old_pos = it->first->UpperLeft() - parent_offset;
//std::cout << "it->first->UpperLeft()=" << it->first->UpperLeft().x << "," << it->first->UpperLeft().y << "\n";
            it->first->MoveTo(s_impl->mouse_pos - parent_offset - it->second);
/*std::cout << "    s_impl->mouse_pos=" << s_impl->mouse_pos.x << "," << s_impl->mouse_pos.y << " it->second=" 
          << it->second.x << "," << it->second.y << " it->first->UpperLeft()=" 
          << it->first->UpperLeft().x << "," << it->first->UpperLeft().y << "\n";*/
            RenderWindow(it->first);
            it->first->MoveTo(old_pos);
//std::cout << "it->first->UpperLeft()=" << it->first->UpperLeft().x << "," << it->first->UpperLeft().y << "\n\n";
        }
    }
    Exit2DMode();
}

Wnd* App::ModalWindow() const
{
    Wnd* retval = 0;
    if (!s_impl->modal_wnds.empty())
        retval = s_impl->modal_wnds.back();
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
