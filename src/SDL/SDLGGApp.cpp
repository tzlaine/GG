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

#include "SDL/SDLGGApp.h"

using std::string;

// member functions
SDLGGApp::SDLGGApp(int w/* = 1024*/, int h/* = 768*/, bool calc_FPS/* = false*/, const std::string& app_name/* = "GG"*/) :
    GG::App(app_name),
    m_app_width(w),
    m_app_height(h),
    m_delta_t(1),
    m_FPS(-1.0),
    m_calc_FPS(calc_FPS)
{
}

SDLGGApp::~SDLGGApp()
{
    SDLQuit();
}

SDLGGApp* SDLGGApp::GetApp()
{
    return dynamic_cast<SDLGGApp*>(GG::App::GetApp());
}

void SDLGGApp::Exit(int code)
{
    Logger().fatalStream() << "Initiating Exit (code " << code << " - " << (code ? "error" : "normal") << " termination)";
    SDLQuit();
    exit(code);
}

void SDLGGApp::CalcuateFPS(bool b/* = true*/)
{
    m_calc_FPS = b;
    if (!b) 
        m_FPS = -1.0f;
}

const string& SDLGGApp::FPSString() const
{
    static string retval;
    char buf[64];
    sprintf(buf, "%.2f frames per second", m_FPS);
    retval = buf;
    return retval;
}

GG::Key SDLGGApp::GGKeyFromSDLKey(const SDL_keysym& key)
{
    GG::Key retval = GG::Key(key.sym);
    bool shift = key.mod & KMOD_SHIFT;
    bool caps_lock = key.mod & KMOD_CAPS;

    // this code works because both SDLKey and GG::Key map (at least
    // partially) to the printable ASCII characters
    if (shift || caps_lock) {
        if (shift != caps_lock && (retval >= 'a' && retval <= 'z')) {
            retval = GG::Key(toupper(retval));
        } else if (shift) { // the caps lock key should not affect these
            // this assumes a US keyboard layout
            switch (retval) {
            case '`': retval = GG::Key('~'); break;
            case '1': retval = GG::Key('!'); break;
            case '2': retval = GG::Key('@'); break;
            case '3': retval = GG::Key('#'); break;
            case '4': retval = GG::Key('$'); break;
            case '5': retval = GG::Key('%'); break;
            case '6': retval = GG::Key('^'); break;
            case '7': retval = GG::Key('&'); break;
            case '8': retval = GG::Key('*'); break;
            case '9': retval = GG::Key('('); break;
            case '0': retval = GG::Key(')'); break;
            case '-': retval = GG::Key('_'); break;
            case '=': retval = GG::Key('+'); break;
            case '[': retval = GG::Key('{'); break;
            case ']': retval = GG::Key('}'); break;
            case '\\': retval = GG::Key('|'); break;
            case ';': retval = GG::Key(':'); break;
            case '\'': retval = GG::Key('"'); break;
            case ',': retval = GG::Key('<'); break;
            case '.': retval = GG::Key('>'); break;
            case '/': retval = GG::Key('?'); break;
            default: break;
            }
        }
    }
    return retval;
}

void SDLGGApp::SDLInit()
{
    const SDL_VideoInfo* vid_info = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
        Logger().errorStream() << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

#if defined(GG_USE_NET) && GG_USE_NET
    if (SDLNet_Init() < 0) {
        Logger().errorStream() << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }

    if (FE_Init() < 0) {
        Logger().errorStream() << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }
#endif // GG_USE_NET

    vid_info = SDL_GetVideoInfo();

    if (!vid_info) {
        Logger().errorStream() << "Video info query failed: " << SDL_GetError();
        Exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(m_app_width, m_app_height, 16, SDL_OPENGL) == 0) {
        Logger().errorStream() << "Video mode set failed: " << SDL_GetError();
        Exit(1);
    }

#if defined(GG_USE_NET) && GG_USE_NET
    if (NET2_Init() < 0) {
        Logger().errorStream() << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }
#endif // GG_USE_NET

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    EnableMouseDragRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL / 2);

    Logger().debugStream() << "SDLInit() complete.";
    GLInit();
}

void SDLGGApp::GLInit()
{
    double ratio = m_app_width / (float)(m_app_height);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, m_app_width - 1, m_app_height - 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);

    Logger().debugStream() << "GLInit() complete.";
}

void SDLGGApp::HandleSDLEvent(const SDL_Event& event)
{
    bool send_to_gg = false;
    GG::App::EventType gg_event;
    GG::Key key = GGKeyFromSDLKey(event.key.keysym);
    Uint32 key_mods = SDL_GetModState();
    GG::Pt mouse_pos(event.motion.x, event.motion.y);
    GG::Pt mouse_rel(event.motion.xrel, event.motion.yrel);

    switch(event.type) {
    case SDL_KEYDOWN:
        if (key < GG::GGK_NUMLOCK)
            send_to_gg = true;
        gg_event = GG::App::KEYPRESS;
        break;
    case SDL_MOUSEMOTION:
        send_to_gg = true;
        gg_event = GG::App::MOUSEMOVE;
        break;
    case SDL_MOUSEBUTTONDOWN:
        send_to_gg = true;
        switch (event.button.button) {
        case SDL_BUTTON_LEFT:      gg_event = GG::App::LPRESS; break;
        case SDL_BUTTON_MIDDLE:    gg_event = GG::App::MPRESS; break;
        case SDL_BUTTON_RIGHT:     gg_event = GG::App::RPRESS; break;
        case SDL_BUTTON_WHEELUP:   gg_event = GG::App::MOUSEWHEEL; mouse_rel = GG::Pt(0, 1); break;
        case SDL_BUTTON_WHEELDOWN: gg_event = GG::App::MOUSEWHEEL; mouse_rel = GG::Pt(0, -1); break;
        }
        key_mods = SDL_GetModState();
        break;
    case SDL_MOUSEBUTTONUP:
        send_to_gg = true;
        switch (event.button.button) {
        case SDL_BUTTON_LEFT:   gg_event = GG::App::LRELEASE; break;
        case SDL_BUTTON_MIDDLE: gg_event = GG::App::MRELEASE; break;
        case SDL_BUTTON_RIGHT:  gg_event = GG::App::RRELEASE; break;
        }
        key_mods = SDL_GetModState();
        break;
    case SDL_QUIT:
        Exit(0);
        break;
    }
    if (send_to_gg)
        GG::App::HandleEvent(gg_event, key, key_mods, mouse_pos, mouse_rel);
}

void SDLGGApp::RenderBegin()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SDLGGApp::RenderEnd()
{
    SDL_GL_SwapBuffers();
}

void SDLGGApp::SDLQuit()
{
    FinalCleanup();
#if defined(GG_USE_NET) && GG_USE_NET
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
#endif // GG_USE_NET
    SDL_Quit();
    Logger().debugStream() << "SDLQuit() complete.";
}

void SDLGGApp::PollAndRender()
{
    static int last_FPS_time = 0;
    static int most_recent_time = 0;
    static int time = 0;
    static int frames = 0;

    static int last_mouse_event_time = 0;
    static int mouse_drag_repeat_start_time = 0;
    static int last_mouse_drag_repeat_time = 0;
    static int old_mouse_repeat_delay = MouseRepeatDelay();
    static int old_mouse_repeat_interval = MouseRepeatInterval();

    // handle events
    SDL_Event event;
#if defined(GG_USE_NET) && GG_USE_NET
    while (0 < FE_PollEvent(&event)) {
#else
    while (0 < SDL_PollEvent(&event)) {
#endif // GG_USE_NET
        if (event.type  == SDL_MOUSEBUTTONDOWN || event.type  == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION)
            last_mouse_event_time = time;
        HandleSDLEvent(event);
    }

    // update time and track FPS if needed
    time = Ticks();
    m_delta_t = time - most_recent_time;
    if (m_calc_FPS) {
        ++frames;
        if (time - last_FPS_time > 1000) { // calculate FPS at most once a second
            m_FPS = frames / ((time - last_FPS_time) / 1000.0f);
            last_FPS_time = time;
            frames = 0;
        }
    }
    most_recent_time = time;

    // handle mouse drag repeats
    if (old_mouse_repeat_delay != MouseRepeatDelay() || old_mouse_repeat_interval != MouseRepeatInterval()) { // if there's a change in the values, zero everything out and start the counting over
        old_mouse_repeat_delay = MouseRepeatDelay();
        old_mouse_repeat_interval = MouseRepeatInterval();
        mouse_drag_repeat_start_time = 0;
        last_mouse_drag_repeat_time = 0;
    }
    int x, y;
    // if drag repeat is enabled, the left mouse button is depressed (a drag is ocurring), and the last event processed wasn't too recent
    if (MouseRepeatDelay() && SDL_GetMouseState(&x, &y) & SDL_BUTTON_LEFT && time - last_mouse_event_time > old_mouse_repeat_interval) {
        if (!mouse_drag_repeat_start_time) { // if we're just starting the drag, mark the time we started
            mouse_drag_repeat_start_time = time;
        } else if (mouse_drag_repeat_start_time == MouseRepeatDelay()) { // if we're counting repeat intervals
            if (time - last_mouse_drag_repeat_time > MouseRepeatInterval()) {
                last_mouse_drag_repeat_time = time;
                event.type = SDL_MOUSEMOTION;
                event.motion.x = x;
                event.motion.y = y;
                event.motion.xrel = event.motion.yrel = 0; // this is just an update, so set the motion to 0
                HandleSDLEvent(event);
            }
        } else if (time - mouse_drag_repeat_start_time > MouseRepeatDelay()) { // if we're done waiting for the initial delay period
            mouse_drag_repeat_start_time = MouseRepeatDelay(); // set this as equal so we know later that we've passed the delay interval
            last_mouse_drag_repeat_time = time;
            event.type = SDL_MOUSEMOTION;
            event.motion.x = x;
            event.motion.y = y;
            event.motion.xrel = event.motion.yrel = 0;
            HandleSDLEvent(event);
        }
    } else { // otherwise, reset the mouse drag repeat start time to zero
        mouse_drag_repeat_start_time = 0;
    }

    // do one iteration of the render loop
    Update();
    RenderBegin();
    Render();
    RenderEnd();
}

void SDLGGApp::Run()
{
    try {
        SDLInit();
        Initialize();
        while (1)
            PollAndRender();
    } catch (const std::invalid_argument& exception) {
        Logger().fatal("std::invalid_argument Exception caught in App::Run(): " + string(exception.what()));
        Exit(1);
    } catch (const std::runtime_error& exception) {
        Logger().fatal("std::runtime_error Exception caught in App::Run(): " + string(exception.what()));
        Exit(1);
    } catch (const GG::GGException& exception) {
        Logger().fatal("GG::GGException (subclass " + string(exception.what()) + ") caught in App::Run(): " + exception.Message());
        Exit(1);
    }
}

