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
   
/* $Header$ */

/** \file GGApp.h
    Contains App class, which encapsulates the state and behavior of the entire GG GUI. */

#ifndef _GGApp_h_ 
#define _GGApp_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

#ifndef _GGFont_h_
#include "GGFont.h"
#endif

#ifndef _LOG4CPP_CATEGORY_HH
#include <log4cpp/Category.hh>
#endif

namespace GG {

class Wnd;
class EventPumpBase;
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
    - limits on FPS speed
    - access to the dimensions of the application window or screen
    - mouse state information
    - keyboard accelerators
    - application-wide management of fonts and textures
    - creation of polymorphic Wnd-derived objects from XML-formatted text
    - logging of debug info to log files
    <p>
    The user is required to provide several functions.  The most vital functions the user is required to provide are: Enter2DMode(),
    Exit2DMode(), DeltaT(), PollAndRender() [virtual private], and Run() [virtual private].  Without these App is pretty useless.  
    In addition, HandleEvent() must be driven from PollAndRender().  The code driving HandleEvent() must interact with the hardware 
    and operating system, and supply the appropriate EventType's, key presses, and mouse position info to HandleEvent().  It is 
    the author's recommendation that the user use SDL to do this.  See http://www.libsdl.org for more info.
    <p>
    Keyboard accelerators may be defined, as mentioned above.  Each defined accelerator has its own signal which is emitted each 
    time the accelerator is detected.  Client code should listen to the appropriate signal to act on an accelerator invocation.
    Each slot that is signalled with a keyboard accelerator should return true if it processed the accelerator, or false otherwise.
    This lets App know whether or not it should create a keystroke event and process it normally, sending it to the Wnd that currently
    has focus.  Note that since signals can be connected to multiple slots, if even one slot return true, no kestroke event is 
    created.  It is perfectly legal to return false even if an accelerator is processed, as long as you also then want the focus 
    Wnd to receive a keystroke event.  Also, note that all accelerators are processed before, and possbily instead of, any key events.  
    So setting a plain "h" as a keyboard accelerator can (if it is processed normally by a slot) prevent any Wnd anywhere in your 
    application from receiving "h" keystrokes.  To avoid this:
    - Do not define accelerators without modifier keys like CTRL and ALT, or
    - Have slots that process these accelerators return false, or
    - Do not connect anything to such an accelerator, in which case it will return false.
    <p>
    A note about "mouse drag repeat".  When you click on the down-button on a scroll-bar, you probably expect 
    the the button's action (scrolling down one increment) to repeat when you hold down the button, much like
    the way kestrokes are repeated when you hold down a keyboard key.  But if you just press the button and 
    keep the mouse perfectly still, there will probably be no events sent to the scrollbar control after the first 
    button-down event.  When enabled, mouse drag repeat sends messages to the scrollbar when there otherwise would be none.
*/
class GG_API App
{
private:
    struct OrCombiner 
    {
        typedef bool result_type; 
        template<class InIt> bool operator()(InIt first, InIt last) const;
    };

public:
    /** \name Signal Types */ //@{
    /** emitted when a keyboard accelerator is invoked. A return value of true indicates that the accelerator was processed 
        by some slot; otherwise, a keystroke event is processed instead. */
    typedef boost::signal<bool (), OrCombiner> AcceleratorSignalType;
    //@}

    /** \name Slot Types */ //@{
    typedef AcceleratorSignalType::slot_type   AcceleratorSlotType; ///< type of functor(s) invoked on a AcceleratorSignalType
    //@}

    /// these are the only events absolutely necessary for GG to function properly
    enum EventType {KEYPRESS,    ///< a down key press or key repeat, with or without modifiers like Alt, Ctrl, Meta, etc.
                    LPRESS,      ///< a left mouse button press
                    MPRESS,      ///< a middle mouse button press
                    RPRESS,      ///< a right mouse button press
                    LRELEASE,    ///< a left mouse button release
                    MRELEASE,    ///< a middle mouse button release
                    RRELEASE,    ///< a right mouse button release
                    MOUSEMOVE,   ///< movement of the mouse; may include relative motion in addition to absolute position
                    MOUSEWHEEL   ///< rolling of the mouse wheel; this event is accompanied by the amount of roll in the y-component of the mouse's relative position (+ is up, - is down)
                   };

    typedef std::set<std::pair<Key, Uint32> >::const_iterator const_accel_iterator; ///< the type of iterator returned by accel_begin() and accel_end()

    /** \name Structors */ //@{
    virtual ~App(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    const string&  AppName() const;              ///< returns the user-defined name of the application
    Wnd*           FocusWnd() const;             ///< returns the GG::Wnd that currently has the input focus
    Wnd*           GetWindowUnder(const Pt& pt) const; ///< returns the GG::Wnd under the point pt
    int            DeltaT() const;               ///< returns ms since last frame was rendered
    virtual int    Ticks() const = 0;            ///< returns ms since the app started running
    bool           FPSEnabled() const;           ///< returns true iff FPS calulations are turned on
    double         FPS() const;                  ///< returns the frames per second at which the application is rendering
    string         FPSString() const;            ///< returns a string of the form "[m_FPS] frames per second"
    double         MaxFPS() const;               ///< returns the maximum allowed frames per second of rendering speed.  0 indicates no limit.
    virtual int    AppWidth() const = 0;         ///< returns the width of the application window/screen
    virtual int    AppHeight() const = 0;        ///< returns the height of the application window/screen
    int            MouseRepeatDelay() const;     ///< returns the \a delay value set by EnableMouseDragRepeat()
    int            MouseRepeatInterval() const;  ///< returns the \a interval value set by EnableMouseDragRepeat()
    int            DoubleClickInterval() const;  ///< returns the maximum interval allowed between clicks that is still considered a double-click, in ms
    bool           MouseButtonDown(int bn) const;///< returns the up/down states of the mouse buttons
    Pt             MousePosition() const;        ///< returns the absolute position of mouse based on last mouse motion event
    Pt             MouseMovement() const;        ///< returns the relative position of mouse based on last mouse motion event
    Wnd*           GenerateWnd(const XMLElement& elem) const; ///< returns a heap-allocated Wnd-subclass object of the Wnd subclass most appropriate to \a elem

    const_accel_iterator accel_begin() const;    ///< returns an iterator to the first defined keyboard accelerator
    const_accel_iterator accel_end() const;      ///< returns an iterator to the last + 1 defined keyboard accelerator

    /** returns the signal that is emitted when the requested keyboard accelerator is invoked. */
    AcceleratorSignalType& AcceleratorSignal(Key key, Uint32 key_mods) const;
    //@}

    /** \name Mutators */ //@{
    void           operator()();                 ///< external interface to Run()
    virtual void   Exit(int code) = 0;           ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
    
    /** handles all waiting system events (from SDL, DirectInput, etc.).  This function should set last_mouse_event_time to the current 
        time (using Ticks()) if mouse events are processed.  This function should only be called from custom EventPump event handlers. */
    virtual void   HandleSystemEvents(int& last_mouse_event_time) = 0;

    void           SetFocusWnd(Wnd* wnd);        ///< sets the input focus window to \a wnd
    virtual void   Wait(int ms);                 ///< suspends the app thread for \a ms milliseconds.  Singlethreaded App subclasses may do nothing here, or may pause for \a ms milliseconds; the default implementation does nothing.
    void           Register(Wnd* wnd);           ///< adds \a wnd into the z-list.  Registering a null pointer or registering the same window multiple times is a no-op.
    void           RegisterModal(Wnd* wnd);      ///< adds \a wnd onto the modal windows "stack"
    void           Remove(Wnd* wnd);             ///< removes \a wnd from the z-list.  Removing a null pointer or removing the same window multiple times is a no-op.
    void           MoveUp(Wnd* wnd);             ///< moves \a wnd to the top of the z-list
    void           MoveDown(Wnd* wnd);           ///< moves \a wnd to the bottom of the z-list
    virtual void   Enter2DMode() = 0;            ///< saves any current GL state, sets up GG-friendly 2D drawing mode
    virtual void   Exit2DMode() = 0;             ///< restores GL to its condition prior to Enter2DMode() call
    void           EnableFPS(bool b = true);     ///< turns FPS calulations on or off
    void           SetMaxFPS(double max);        ///< sets the maximum allowed FPS, so the render loop does not act as a spinlock when it runs very quickly.  0 indicates no limit.
    void           EnableMouseDragRepeat(int delay, int interval); ///< delay and interval are in ms; Setting delay to 0 disables mouse repeating completely.
    void           SetDoubleClickInterval(int interval); ///< sets the maximum interval allowed between clicks that is still considered a double-click, in ms

    /** establishes a keyboard accelerator.  Any key modifiers may be specified, or none at all. */
    void           SetAccelerator(Key key, Uint32 key_mods);

    /** removes a keyboard accelerator.  Any key modifiers may be specified, or none at all. */
    void           RemoveAccelerator(Key key, Uint32 key_mods);

    shared_ptr<Font>    GetFont(const string& font_filename, int pts, Uint32 range = Font::ALL_CHARS); ///< returns a shared_ptr to the desired font
    void                FreeFont(const string& font_filename, int pts); ///< removes the desired font from the managed pool; since shared_ptr's are used, the font may be deleted much later

    /** adds an already-constructed texture to the managed pool \warning calling code <b>must not</b> delete \a texture; the texture pool will do that. */
    shared_ptr<Texture> StoreTexture(Texture* texture, const string& texture_name);

    shared_ptr<Texture> StoreTexture(shared_ptr<Texture> texture, const string& texture_name); ///< adds an already-constructed texture to the managed pool
    shared_ptr<Texture> GetTexture(const string& name, bool mipmap = false); ///< loads the requested texture from file \a name; mipmap textures are generated if \a mipmap is true
    void                FreeTexture(const string& name); ///< removes the desired texture from the managed pool; since shared_ptr's are used, the texture may be deleted much later
    void                AddWndGenerator(const string& name, Wnd* (*fn)(const XMLElement&)); ///< adds or overrides a Wnd-subclass generator associated with \a name
    void                RemoveWndGenerator(const string& name); ///< removes the Wnd-subclass generator associated with \a name
    log4cpp::Category&  Logger();
    //@}

    static App*  GetApp();                ///< allows any GG code access to app framework by calling App::GetApp()
    static void  RenderWindow(Wnd* wnd);  ///< renders a window and (conditionally) all its descendents
    
protected:
    /** \name Structors */ //@{
    App(const string& app_name); ///< protected ctor, called by derived classes
    //@}

    /** \name Mutators */ //@{
    void           HandleGGEvent(EventType event, Key key, Uint32 key_mods, const Pt& pos, const Pt& rel); ///< event handler for GG events
    virtual void   RenderBegin() = 0;      ///< clears the backbuffer, etc.
    virtual void   Render();               ///< renders the windows in the z-list
    virtual void   RenderEnd() = 0;        ///< swaps buffers, etc.

    // EventPumpBase interface
    void SetFPS(double FPS);               ///< sets the FPS value based on the most recent calculation
    void SetDeltaT(int delta_t);           ///< sets the time between the most recent frame and the one before it, in ms
    //@}

private:
    virtual void   Run() = 0;                    // initializes app state, then executes main event handler/render loop (PollAndRender())
    Wnd*           ModalWindow() const;          // returns the currently modal window, if any

    static App*                    s_app;
    static shared_ptr<AppImplData> s_impl;

    friend class EventPumpBase; ///< allows EventPumpBase types to drive App
};

template<class InIt> 
bool App::OrCombiner::operator()(InIt first, InIt last) const
{
    bool retval = false;
    while (first != last)
        retval |= static_cast<bool>(*first++);
    return retval;
}

} // namespace GG

#endif // _GGApp_h_
