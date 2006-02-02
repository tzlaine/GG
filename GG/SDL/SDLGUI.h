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

/* $Id$ */

/** \file SDLGUI.h
    Contains SDLGUI, the input driver for using SDL with GG. */

#ifndef _GG_SDLGUI_h_
#define _GG_SDLGUI_h_

#ifndef _GG_Config_h_
# include <GG/Config.h>
#endif

#if defined(GG_USE_NET) && GG_USE_NET
# ifndef _FASTEVENTS_H_
#  include <GG/net/fastevents.h>
# endif
# ifndef _NET2_H_
#  include <GG/net/net2.h>
# endif
#endif

#ifndef _GG_GUI_h_
#include <GG/GUI.h>
#endif

#ifndef _SDL_H
#include <SDL.h>
#endif

/** This is an abstract singleton class that represents the GUI framework of an SDL OpenGL 
    application. By default, it includes Bob Pendleton's fastevents/net2 library.
    <p>
    Usage:<br>
    Any application including an object of this class should declare that object as a local variable in main(). 
    The name of this variable will herein be assumed to be "gui". It should be allocated on the stack; if it 
    is created dynamically a leak may occur. 
    SDLGUI is designed so the main() of the application can consist of just the one line "gui();".
    <p>
    To do this, the user needs only to override the Initialize() and FinalCleanup() methods, and ensure that 
    the program does not terminate abnormally; this ensures FinalCleanup() is called when gui's destructor 
    is invoked. Exit() can also perform cleanup and terminate the application cleanly.
    <p>
    Most of the member methods of SDLGUI have been declared virtual, to give the user great control when
    subclassing. The virtual function calls are usually not a performance issue, since none of the methods 
    is called repeatedly, except HandleEvent(); if this is a problem, just create a new function in your 
    subclass and call that from within Run() instead of HandleEvent(). Note that though the bulk of the 
    program execution takes place within Run(), Run() itself is also only called once.
    <p>
    SDLGUI takes a two-tiered approach to event handling.  The event pump calls HandlSystemEvents(), which
    polls for SDL events and handles them by first determining whether the event is GG-related, or some other
    non-GG event, such as SDL_QUIT, a net2 network event, etc.  GG events and non-GG events are passed to 
    HandleGGEvent() and HandleNonGGEvent(), respectively.  For most uses, there should be no need to override 
    the behavior of HandleSDLEvents().  However, the HandleNonGGEvent() default implementation only responds to 
    SDL_QUIT events, and so should be overridden in most cases. */
class GG_API SDLGUI : public GG::GUI
{
public:
    /** \name Structors */ //@{
    SDLGUI(int w = 1024, int h = 768, bool calc_FPS = false, const std::string& app_name = "GG"); ///< ctor
    virtual ~SDLGUI();
    //@}

    /** \name Accessors */ //@{
    virtual int    AppWidth() const;
    virtual int    AppHeight() const;
    virtual int    Ticks() const;
    //@}

    /** \name Mutators */ //@{
    void           operator()();      ///< external interface to Run()
    virtual void   Exit(int code);
    virtual void   Wait(int ms);      ///< suspends the gui thread for \a ms milliseconds

    virtual void   Enter2DMode() = 0;
    virtual void   Exit2DMode() = 0;
    //@}

    static SDLGUI* GetGUI();                             ///< allows any code to access the gui framework by calling SDLGUI::GetGUI()
    static GG::Key GGKeyFromSDLKey(const SDL_keysym& key); ///< gives the GGKey equivalent of key

private:
    // these are called at the beginning of the gui's execution
    virtual void   SDLInit();        ///< initializes SDL, FE, and SDL OpenGL functionality
    virtual void   GLInit();         ///< allows user to specify OpenGL initialization code; called at the end of SDLInit()
    virtual void   Initialize() = 0; ///< provides one-time gui initialization

    virtual void   HandleSystemEvents();
    virtual void   HandleNonGGEvent(const SDL_Event& event); ///< event handler for all SDL events that are not GG-related

    virtual void   RenderBegin();
    virtual void   RenderEnd();

    // these are called at the end of the gui's execution
    virtual void   FinalCleanup();   ///< provides one-time gui cleanup
    virtual void   SDLQuit();        ///< cleans up SDL and (if used) FE

    virtual void   Run();

    int            m_app_width;      ///< application width and height (defaults to 1024 x 768)
    int            m_app_height;
};

#endif // _GG_SDLGUI_h_

