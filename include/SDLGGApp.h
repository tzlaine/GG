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

#ifndef _SDLGGApp_h_
#define _SDLGGApp_h_

#ifndef _SDL_thread_h
#include <SDL_thread.h>
#endif 

#ifndef _FASTEVENTS_H_
#include "net/fastevents.h"
#endif

#ifndef _NET2_H_
#include "net/net2.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

/** This is an abstract singleton class that represents the application framework of an SDL OpenGL 
    application. By default, it includes the Bob Pendleton's fastevents/net2 library.

    Usage indications:
    Any application including an object of this class should declare that object as a global variable. The name 
    of this variable will herein be assumed to be g_app. It must be allocated on the stack; if it is created 
    dynamically a memory leak may ocurr, and there is no guarantee that its destructor will be called. 
    SDLGGApp is designed so the main() of the application can consist of just the one line "g_app();".
    
    To do this, the user needs only to override the Initialize() and FinalCleanup() methods, and ensure that 
    the program does not terminate abnormally; this ensures FinalCleanup() is called when g_app's destructor 
    is invoked. Exit() can also perform cleanup and terminate the application cleanly.
    
    Most of the member methods of SDLGGApp have been declared virtual, to give the user great control when
    subclassing. The virtual function calls are not a performance issue, since none of the methods is called
    repeatedly, except HandleEvent(); if this is a problem, just create a new function in your subclass and
    call that from within Run() instead of HandleEvent(). Note that though the bulk of the program execution 
    takes place within Run(), Run() itself is also only called once.*/
class SDLGGApp : public GG::App
{
public:
    /** \name Structors */ //@{
    SDLGGApp(int w = 1024, int h = 768, bool calc_FPS = false, const std::string& app_name = "GG"); ///< ctor
    virtual ~SDLGGApp();
    //@}

    /** \name Accessors */ //@{
    virtual int    AppWidth() const     {return m_app_width;}
    virtual int    AppHeight() const    {return m_app_height;}
    virtual int    DeltaT() const       {return m_delta_t;}
    virtual int    Ticks() const        {return SDL_GetTicks();}
    virtual double FPS() const          {return m_FPS;}
    //@}
   
    /** \name Mutators */ //@{
    void operator()()                   {App::operator()();} ///< external interface to Run()
   
    virtual void   Exit(int code);                  ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()

    virtual void   Enter2DMode() = 0;               ///< saves relevant OpenGL states, sets up ortho projection
    virtual void   Exit2DMode() = 0;                ///< restores states saved in Enter2DMode()

    virtual void   CalcuateFPS(bool b = true);      ///< sets m_calc_FPS. also sets m_FPS to -1 if b == false
    virtual const std::string& FPSString() const;   ///< returns a string of the form "[m_FPS] frames per second"
    //@}

    static SDLGGApp* GetApp();                               ///< allows any code to access the app framework by calling SDLGGApp::GetApp()
    static GG::Key GGKeyFromSDLKey(const SDL_keysym& key);   ///< gives the GGKey equivalent of key

private:
    // these are called at the beginning of the app's execution
    virtual void   SDLInit();              ///< initializes SDL, FE, and SDL OpenGL functionality
    virtual void   GLInit();               ///< allows user to specify OpenGL initialization code; called at the end of SDLInit()
    virtual void   Initialize() = 0;       ///< provides one-time app initialization

    virtual void   HandleSDLEvent(const SDL_Event& event);///< event handler for all events

    // these four are called in this order every time the render loop executes
    virtual void   Update() = 0;           ///< updates world state prior to rendering
    virtual void   RenderBegin();          ///< clears the backbuffer, etc.
    virtual void   RenderEnd();            ///< swaps buffers, etc.

    // these are called at the end of the app's execution
    virtual void   FinalCleanup() {}       ///< provides one-time app cleanup
    virtual void   SDLQuit();              ///< cleans up FE and SDL

    virtual void   PollAndRender();        ///< handles all waiting messages, then renders once
    virtual void   Run();                  ///< calls SDLInit() and Initialize(), then executes main event handler/render loop (PollAndRender())

    int            m_app_width;            ///< application width and height (defaults to 1024 x 768)
    int            m_app_height;

    int            m_delta_t;              ///< time elapsed since the last frame, in ms
    double         m_FPS;                  ///< frames per second that the app is running (= -1.0f when not being calcuated)
    bool           m_calc_FPS;             ///< set to true when the app should calculate FPS
};

#endif // _SDLGGApp_h_


