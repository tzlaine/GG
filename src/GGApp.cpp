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

namespace GG {

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
}

// implementation data types
struct AppImplData
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
	double_click_wnd(0),
	double_click_start_time(-1) ,
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
    Wnd*         focus_wnd;             // GUI window that currently has the input focus and its backup
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

    Wnd*       double_click_wnd;        // GUI window most recently clicked
    int        double_click_button;     // the index of the mouse button used in the last click
    int        double_click_start_time; // the time from which we started measuring double_click_time, in ms
    int        double_click_time;       // time elapsed since last click, in ms

    FontManager       font_manager;
    TextureManager    texture_manager;

    XMLObjectFactory<Wnd> wnd_factory; // object that creates Wnd-subclass objects from XML-formatted text

    log4cpp::Category&   log_category; // log4cpp object used to log events to file
};

// static member(s)
App*                     App::s_app = 0;
shared_ptr<AppImplData>  App::s_impl(new AppImplData());

// member functions
App::App(App* app, const string& app_name)
{
    s_app = app;
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

void App::operator()()
{
    Run();
}

void App::Register(Wnd* wnd)
{
    if (wnd) s_impl->zlist.Add(wnd);
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
    }
}

void App::MoveUp(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveUp(wnd);
}

void App::MoveDown(Wnd* wnd)
{
    if (wnd) s_impl->zlist.MoveDown(wnd);
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

shared_ptr<Font> App::GetFont(const string& font_filename, int pts, Uint32 range/* = Font::ALL_DEFINED_RANGES*/)
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

log4cpp::Category& App::Logger()
{
    return s_impl->log_category;
}

App* App::GetApp()
{
    return s_app;
}

void App::HandleEvent(EventType event, Key key, Uint32 key_mods, const Pt& pos, const Pt& rel)
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
            if (s_impl->focus_wnd)
                s_impl->focus_wnd->Keypress(key, key_mods);
            break;}
    case MOUSEMOVE:{
            curr_wnd_under_cursor = GetWindowUnder(pos); // update window under mouse position

            // record these
            s_impl->mouse_pos = pos; // mouse position
            s_impl->mouse_rel = rel; // mouse movement

            // then act on mouse motion
            if (drag_wnds[0]) { // only respond to left mouse button drags
                if (wnd_region == WR_MIDDLE || wnd_region == WR_NONE) { // send move message to window
                    drag_wnds[0]->LDrag(pos, rel, key_mods);
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
                    case WR_BOTTOMRIGHT: drag_wnds[0]->SizeMove(drag_wnds[0]->UpperLeft(), drag_wnds[0]->LowerRight() + Pt(rel.x,
                                rel.y)); break;
                    default: break;
                    }
                } else if (drag_wnds[0]->DragKeeper()) {
                    drag_wnds[0]->LDrag(pos, rel, key_mods);
                }
            } else if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) { // if !drag_wnds[0] and we're moving over the same (valid) object we were during the last iteration
                if (curr_wnd_under_cursor) curr_wnd_under_cursor->MouseHere(pos, 0);
            } else { // if !drag_wnds[0] and prev_wnd_under_cursor != curr_wnd_under_cursor, we're just moving around
                if (prev_wnd_under_cursor) prev_wnd_under_cursor->MouseLeave(pos, 0);
                if (curr_wnd_under_cursor) curr_wnd_under_cursor->MouseEnter(pos, 0);
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
                        drag_wnds[0]->LButtonDown(pos, key_mods);
                    }
                    break;}
            case MPRESS:{
                    s_impl->button_state[1] = true;
                    break;}
            case RPRESS:{
                    s_impl->button_state[2] = true;
                    drag_wnds[2] = curr_wnd_under_cursor;  // track this window as the one being dragged by the right mouse button
                    if (drag_wnds[2]) {
                        drag_wnds[2]->RButtonDown(pos, key_mods);
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
                    if (drag_wnds[0] && curr_wnd_under_cursor == drag_wnds[0]) { // if the release is over the place where the button-down event occurred
                        // if this is second l-click over a window that just received an l-click within
                        // the time limit -- it's a double-click, not a click
                        if (s_impl->double_click_time > 0 && s_impl->double_click_wnd == drag_wnds[0] &&
                                s_impl->double_click_button == 0) {
                            drag_wnds[0]->LDoubleClick(pos, key_mods);
                            s_impl->double_click_wnd = 0;
                            s_impl->double_click_start_time = -1;
                            s_impl->double_click_time = -1;
                        } else {
                            drag_wnds[0]->LClick(pos, key_mods);
                            if (s_impl->double_click_time > 0) {
                                s_impl->double_click_wnd = 0;
                                s_impl->double_click_start_time = -1;
                                s_impl->double_click_time = -1;
                            } else {
                                s_impl->double_click_start_time = Ticks();
                                s_impl->double_click_time = 0;
                                s_impl->double_click_wnd = drag_wnds[0];
                                s_impl->double_click_button = 0;
                            }
                        }
                    } else {
                        if (drag_wnds[0])
                            drag_wnds[0]->LButtonUp(pos, key_mods);
                        s_impl->double_click_wnd = 0;
                        s_impl->double_click_time = -1;
                    }
                    s_impl->button_state[0] = false;
                    drag_wnds[0] = 0;       // if the mouse button is released, stop the tracking the drag window
                    wnd_region = WR_NONE;   // and clear this, just in case
                    break;}
            case MRELEASE:{
                    s_impl->button_state[1] = false;
                    s_impl->double_click_wnd = 0;
                    s_impl->double_click_time = -1;
                    break;}
            case RRELEASE:{
                    if (drag_wnds[2] && curr_wnd_under_cursor == drag_wnds[2]) { // if the release is over the place where the button-down event occurred
                        // if this is second r-click over a window that just received an r-click within
                        // the time limit -- it's a double-click, not a click
                        if (s_impl->double_click_time > 0 && s_impl->double_click_wnd == drag_wnds[2] &&
                                s_impl->double_click_button == 2) {
                            drag_wnds[2]->RDoubleClick(pos, key_mods);
                            s_impl->double_click_wnd = 0;
                            s_impl->double_click_time = -1;
                        } else {
                            drag_wnds[2]->RClick(pos, key_mods);
                            if (s_impl->double_click_time > 0) {
                                s_impl->double_click_wnd = 0;
                                s_impl->double_click_time = -1;
                            } else {
                                s_impl->double_click_time = 0;
                                s_impl->double_click_wnd = drag_wnds[2];
                                s_impl->double_click_button = 2;
                            }
                        }
                    } else {
                        s_impl->double_click_wnd = 0;
                        s_impl->double_click_time = -1;
                    }
                    s_impl->button_state[2] = false;
                    drag_wnds[2] = 0;
                    break;}
            default:
                break;
            }
            prev_wnd_under_cursor = curr_wnd_under_cursor; // update this for the next time around
            break;}
    case MOUSEWHEEL:{
            curr_wnd_under_cursor = GetWindowUnder(pos);  // update window under mouse position
            if (curr_wnd_under_cursor)
                curr_wnd_under_cursor->MouseWheel(pos, rel.y, key_mods);
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
    Exit2DMode();
}

void App::RenderWindow(Wnd* wnd)
{
    if (wnd->Render() == 1) {
	for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != wnd->m_children.end(); ++it) {
	    if ((*it)->Visible())
		RenderWindow(*it);
	}
    }
}

Wnd* App::ModalWindow() const
{
    Wnd* retval = 0;
    if (!s_impl->modal_wnds.empty())
        retval = s_impl->modal_wnds.back();
    return retval;
}

void App::RegisterModal(Wnd* wnd)
{
    if (wnd) {
        SetFocusWnd(wnd);
        s_impl->modal_wnds.push_back(wnd);
    }
}

void App::SetFocusWnd(Wnd* wnd)
{
    // inform old focus wnd that it is losing focus
    if (s_impl->focus_wnd)
        s_impl->focus_wnd->LosingFocus();

    s_impl->focus_wnd = wnd;

    // inform new focus wnd that it is gaining focus
    if (s_impl->focus_wnd)
        s_impl->focus_wnd->GainingFocus();
}

} // namespace GG

