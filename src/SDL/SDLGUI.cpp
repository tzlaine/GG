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

#include <GG/SDL/SDLGUI.h>
#include <GG/EventPump.h>

#include <iostream>

// member functions
SDLGUI::SDLGUI(int w/* = 1024*/, int h/* = 768*/, bool calc_FPS/* = false*/, const std::string& app_name/* = "GG"*/) :
    GUI(app_name),
    m_app_width(w),
    m_app_height(h)
{
}

SDLGUI::~SDLGUI()
{
    SDLQuit();
}

int SDLGUI::AppWidth() const
{
    return m_app_width;
}

int SDLGUI::AppHeight() const
{
    return m_app_height;
}

int SDLGUI::Ticks() const
{
    return SDL_GetTicks();
}

void SDLGUI::operator()()
{
    GUI::operator()();
}

void SDLGUI::Exit(int code)
{
    if (code)
        std::cerr << "Initiating Exit (code " << code << " - error termination)";
    SDLQuit();
    exit(code);
}

void SDLGUI::Wait(int ms)
{
    SDL_Delay(ms);
}

SDLGUI* SDLGUI::GetGUI()
{
    return dynamic_cast<SDLGUI*>(GUI::GetGUI());
}

GG::Key SDLGUI::GGKeyFromSDLKey(const SDL_keysym& key)
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

void SDLGUI::SDLInit()
{
    const SDL_VideoInfo* vid_info = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

#if defined(GG_USE_NET) && GG_USE_NET
    if (SDLNet_Init() < 0) {
        std::cerr << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }

    if (FE_Init() < 0) {
        std::cerr << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }
#endif // GG_USE_NET

    vid_info = SDL_GetVideoInfo();

    if (!vid_info) {
        std::cerr << "Video info query failed: " << SDL_GetError();
        Exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(m_app_width, m_app_height, 16, SDL_OPENGL) == 0) {
        std::cerr << "Video mode set failed: " << SDL_GetError();
        Exit(1);
    }

#if defined(GG_USE_NET) && GG_USE_NET
    if (NET2_Init() < 0) {
        std::cerr << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }
#endif // GG_USE_NET

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    EnableMouseDragRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL / 2);

    GLInit();
}

void SDLGUI::GLInit()
{
    double ratio = m_app_width / (float)(m_app_height);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, m_app_width, m_app_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);
}

void SDLGUI::HandleSystemEvents(int& last_mouse_event_time)
{
    // handle events
    SDL_Event event;
#if defined(GG_USE_NET) && GG_USE_NET
    while (0 < FE_PollEvent(&event)) {
#else
    while (0 < SDL_PollEvent(&event)) {
#endif // GG_USE_NET
        if (event.type  == SDL_MOUSEBUTTONDOWN || event.type  == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEMOTION)
            last_mouse_event_time = Ticks();

        bool send_to_gg = false;
        EventType gg_event = MOUSEMOVE;
        GG::Key key = GG::GGK_UNKNOWN;
        Uint32 key_mods = SDL_GetModState();
#ifdef __APPLE__
        GG::Pt mouse_pos(event.motion.x, m_app_height - event.motion.y);
        GG::Pt mouse_rel(event.motion.xrel, -event.motion.yrel);
#else
        GG::Pt mouse_pos(event.motion.x, event.motion.y);
        GG::Pt mouse_rel(event.motion.xrel, event.motion.yrel);
#endif

        switch (event.type) {
        case SDL_KEYDOWN:
            key = GGKeyFromSDLKey(event.key.keysym);
            if (key < GG::GGK_NUMLOCK)
                send_to_gg = true;
            gg_event = KEYPRESS;
            break;
        case SDL_MOUSEMOTION:
            send_to_gg = true;
            gg_event = MOUSEMOVE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:      gg_event = LPRESS; break;
                case SDL_BUTTON_MIDDLE:    gg_event = MPRESS; break;
                case SDL_BUTTON_RIGHT:     gg_event = RPRESS; break;
                case SDL_BUTTON_WHEELUP:   gg_event = MOUSEWHEEL; mouse_rel = GG::Pt(0, 1); break;
                case SDL_BUTTON_WHEELDOWN: gg_event = MOUSEWHEEL; mouse_rel = GG::Pt(0, -1); break;
            }
            key_mods = SDL_GetModState();
            break;
        case SDL_MOUSEBUTTONUP:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:   gg_event = LRELEASE; break;
                case SDL_BUTTON_MIDDLE: gg_event = MRELEASE; break;
                case SDL_BUTTON_RIGHT:  gg_event = RRELEASE; break;
            }
            key_mods = SDL_GetModState();
            break;
        }

        if (send_to_gg)
            HandleGGEvent(gg_event, key, key_mods, mouse_pos, mouse_rel);
        else
            HandleNonGGEvent(event);
    }
}

void SDLGUI::HandleNonGGEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_QUIT:
        Exit(0);
        break;
    }
}

void SDLGUI::RenderBegin()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SDLGUI::RenderEnd()
{
    SDL_GL_SwapBuffers();
}

void SDLGUI::FinalCleanup()
{
}

void SDLGUI::SDLQuit()
{
    FinalCleanup();
#if defined(GG_USE_NET) && GG_USE_NET
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
#endif // GG_USE_NET
    SDL_Quit();
}

void SDLGUI::Run()
{
    try {
        SDLInit();
        Initialize();
        GG::EventPump pump;
        pump();
    } catch (const std::invalid_argument& e) {
        std::cerr << "std::invalid_argument exception caught in GUI::Run(): " << e.what();
        Exit(1);
    } catch (const std::runtime_error& e) {
        std::cerr << "std::runtime_error exception caught in GUI::Run(): " << e.what();
        Exit(1);
    } catch (const GG::ExceptionBase& e) {
        std::cerr << "GG exception (subclass " << e.type() << ") caught in GUI::Run(): " << e.what();
        Exit(1);
    }
}

