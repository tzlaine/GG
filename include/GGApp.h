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

#ifndef _GGApp_h_ 
#define _GGApp_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

#ifndef _GGText_h_
#include "GGText.h"
#endif

#ifndef _LOG4CPP_CATEGORY_HH
#include <log4cpp/Category.hh>
#endif

namespace GG {

class Wnd;
class Texture;
class XMLElement;
struct AppImplData;

/** An abstract base for an application framework class to drive the GG GUI.  
    This class has all the essential services that GG requires: 
    - application initialization and emergency exit
    - GG event handling
    - rendering of GUI windows
    - entry into and cleanup after a "2D" mode, in case the app also uses non-orthographic 3D projections
    - registration into, removal from, and movement within a z-ordering of windows, including handling of "always-on-top" windows
    - handling of modal windows
    - inter-frame time updates and FPS calculations
    - access to the dimensions of the application window or screen
    - mouse state information
    - application-wide management of fonts and textures
    - creation of polymorphic Wnd-derived objects from XML-formatted text
    - logging of debug info to log files
    
    The user is required to provide several functions.  The most vital functions the user is required to provide are: Enter2DMode(),
    Exit2DMode(), DeltaT(), PollAndRender() [virtual private], and Run() [virtual private].  Without these App is pretty useless.  
    In addition, HandleEvent() must be driven from PollAndRender().  The code driving HandleEvent() must interact with the hardware 
    and operating system, and supply the appropriate EventType's, key presses, and mouse position info to HandleEvent().  It is 
    the author's recommendation that the user use SDL to do this.  See http://www.libsdl.org for more info.
   
    A note about "mouse drag repeat".  When you click on the down-button on a scroll-bar, you probably expect 
    the the button's action (scrolling down one increment) to repeat when you hold down the button, much like
    the way kestrokes are repeated when you hold down a keyboard key.  But if you just press the button and 
    keep the mouse perfectly still, there will probably be no events sent to the scrollbar control after the first 
    button-down event.  When enabled, mouse drag repeat sends messages to the scrollbar when there otherwise would be none.
*/
class App
{
public: 
    /// these are the only events absolutely necessary for GG to function properly
    enum EventType {KEYPRESS,    ///< a down key press or key repeat, with or without modifiers like Alt, Ctrl, Meta, etc.
		    LPRESS,      ///< a left mouse button press
		    MPRESS,      ///< a middle mouse button press
		    RPRESS,      ///< a right mouse button press
		    LRELEASE,    ///< a left mouse button release
		    MRELEASE,    ///< a middle mouse button release
		    RRELEASE,    ///< a right mouse button release
		    MOUSEMOVE,   ///< movement of the mouse
		    MOUSEWHEEL   ///< rolling of the mouse wheel; this event is accompanied by the amount of roll in the y-component of the mouse's relative position (+ is up, - is down)
                   };

    /** \name Structors */ //@{
    virtual ~App(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    const string&  AppName() const;              ///< returns the user-defined name of the application
    Wnd*           FocusWnd() const;             ///< returns the GG::Wnd that currently has the input focus
    Wnd*           GetWindowUnder(const Pt& pt) const; ///< returns the GG::Wnd under the point pt
    virtual int    DeltaT() const = 0;           ///< returns ms since last frame was rendered
    virtual int    Ticks() const = 0;            ///< returns ms since the app started running
    virtual double FPS() const = 0;              ///< returns the approximate number of frames per second at which the application is rendering
    virtual const string& FPSString() const = 0; ///< returns a string of the form "[m_FPS] frames per second"
    virtual int    AppWidth() const = 0;         ///< returns the width of the application window/screen
    virtual int    AppHeight() const = 0;        ///< returns the height of the application window/screen
    int            MouseRepeatDelay() const;     ///< returns the \a delay value set by EnableMouseDragRepeat()
    int            MouseRepeatInterval() const;  ///< returns the \a interval value set by EnableMouseDragRepeat()
    int            DoubleClickInterval() const;  ///< returns the maximum interval allowed between clicks that is still considered a double-click, in ms
    bool           MouseButtonDown(int bn) const;///< returns the up/down states of the mouse buttons
    Pt             MousePosition() const;        ///< returns the absolute position of mouse based on last mouse motion event
    Pt             MouseMovement() const;        ///< returns the relative position of mouse based on last mouse motion event
    Wnd*           GenerateWnd(const XMLElement& elem) const; ///< returns a heap-allocated Wnd-subclass object of the Wnd subclass most appropriate to \a elem
    //@}

    /** \name Mutators */ //@{
    void           operator()();                 ///< external interface to Run()
    virtual void   Exit(int code) = 0;           ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()

    void           Register(Wnd* wnd);           ///< adds a GG::Wnd into the z-list
    void           Remove(Wnd* wnd);             ///< removes a GG::Wnd from the z-list
    void           MoveUp(Wnd* wnd);             ///< moves a GG::Wnd to the top of the z-list
    void           MoveDown(Wnd* wnd);           ///< moves a GG::Wnd to the bottom of the z-list
    virtual void   Enter2DMode() = 0;            ///< saves any current graphics subsystem states, sets up GG-friendly 2D drawing mode
    virtual void   Exit2DMode() = 0;             ///< restores graphics states to their condition prior to Enter2DMode() call
    virtual void   CalcuateFPS(bool b = true) = 0;///< turns FPS calulations on or off
    void           EnableMouseDragRepeat(int delay, int interval); ///< delay and interval are in ms; Setting delay to 0 disables mouse repeating completely.
    void           SetDoubleClickInterval(int interval); ///< sets the maximum interval allowed between clicks that is still considered a double-click, in ms
   
    shared_ptr<Font>    GetFont(const string& font_filename, int pts, Uint32 range = Font::ALL_DEFINED_RANGES); ///< returns a shared_ptr to the desired font
    void                FreeFont(const string& font_filename, int pts); ///< removes the desired font from the managed pool; since shared_ptr's are used, the font may be deleted much later
    shared_ptr<Texture> StoreTexture(Texture* texture, const string& texture_name); ///< adds an already-constructed texture to the managed pool \warning calling code <b>must not</b> delete \a texture; the texture pool will do that
    shared_ptr<Texture> StoreTexture(shared_ptr<Texture> texture, const string& texture_name); ///< adds an already-constructed texture to the managed pool
    shared_ptr<Texture> GetTexture(const string& name, bool mipmap = false); ///< loads the requested texture from file \a name; mipmap textures are generated if \a mipmap is true
    void                FreeTexture(const string& name); ///< removes the desired texture from the managed pool; since shared_ptr's are used, the texture may be deleted much later
    void                AddWndGenerator(const string& name, Wnd* (*fn)(const XMLElement&)); ///< adds or overrides a Wnd-subclass generator associated with \a name
    log4cpp::Category&  Logger();
    //@}

    static App*  GetApp();  ///< allows any GG code access to app framework by calling App::GetApp()
   
protected:
    /** \name Structors */ //@{
    App(App* app, const string& app_name); ///< protected ctor, called by derived classes in order to provide App with a pointer \a app to the singleton App object.
    //@}

    /** \name Mutators */ //@{
    void           HandleEvent(EventType event, Key key, Uint32 key_mods, const Pt& pos, const Pt& rel); ///< event handler for GG events
    virtual void   Render();   ///< renders the windows in the z-list
    //@}

private:
    virtual void   PollAndRender() = 0;          // handles all waiting messages, then renders once
    virtual void   Run() = 0;                    // initializes app state, then executes main event handler/render loop (PollAndRender())
    void           RenderWindow(Wnd* wnd);       // renders wnd and then recursivley renders all its children
    Wnd*           ModalWindow() const;          // returns the currently modal window, if any
    void           RegisterModal(Wnd* wnd);      // adds a GUI window onto the modal windows "stack"
    void           SetFocusWnd(Wnd* wnd);        // sets focus window to \a wnd; used by modal Wnds

    static App*                    s_app;
    static shared_ptr<AppImplData> s_impl;

    friend class Wnd; ///< allows modal Wnds to call PollAndRender() and RegisterModal()
};

} // namespace GG

#endif // _GGApp_h_

