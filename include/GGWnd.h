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

#ifndef _GGWnd_h_
#define _GGWnd_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

/** This is a GUI window class.
    Window boundaries are from m_upperleft to m_lowerright - Pt(1,1).
    It is assumed that a child window exists within the boundaries of its parent, although this is not required.
    However, all clicks that land on a child that are also outside of the parent will not reach the child.
    Ideally, "sibling" child windows should not overlap (unless they can without interfering, as in the case of a 
    bounding box, etc.).  If this is impossible or undesirable and control is needed over the order in which 
    children are rendered, MoveChildUp() and MoveChildDown() provide such control.
    Always-on-top windows are drawn after all others, to ensure that they remain on top.  This means that other 
    non-on-top windows that are moved to the top of the z-order stop at some z-value below the lowest z-valued on-top 
    window.  On-topness is useful for 
    modeless dialog boxes, among other things. Modal windows are available (by setting the MODAL window creation 
    flag), and are also always-on-top, but are 
    handled differently and do not have ONTOP specified in their creation flags.  
    Modal windows are executed by calling Run(), which registers them as modal windows and starts the local 
    execution of the application's event pump.  Execution of the code that calls Run() is effectively halted 
    until Run() returns.  Derived classes that wish to use modal execution should set m_done = true to escape from 
    the modal execution loop.
    Wnd inherits from boost::signals::trackable.  This means that any slots contained in a Wnd object or 
    Wnd-derived object will automatically
    be disconnected from any connected signals when the Wnd is destroyed.  Note that while a Wnd can contain arbitrary 
    Wnd-derived children, in order for such children to be automatically saved and loaded using XML encoding, any 
    user-defined Wnd subclasses must be added to the App's XMLObjectFactory.  See GG::App::AddWndGenerator() and 
    GG::XMLObjectFactory for details.*/
class GG_API Wnd : public boost::signals::trackable
{
public:
    /// window creation flags
    enum WndFlag {CLICKABLE =    1 << 0,  ///< clicks hit this window, rather than passing through it
		  DRAGABLE =     1 << 1,  ///< this window can be dragged around independently
		  DRAG_KEEPER =  1 << 2,  ///< this window receives drag messages, even if it is not dragable
		  RESIZABLE =    1 << 3,  ///< this window can be resized by the user, with the mouse
		  ONTOP =        1 << 4,  ///< this windows is an "on-top" window, and will always appear above all non-on-top and non-modal windows
		  MODAL =        1 << 5   ///< this window is modal; while it is active, no other windows are interactive.  Modal windows are considered above "on-top" windows, and should not be flagged as ONTOP.
                 };
         
    GGEXCEPTION(WndException);   ///< exception class \see GG::GGEXCEPTION
   
    /** \name Structors */ //@{
    virtual ~Wnd(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    bool           Clickable() const   {return m_flags & CLICKABLE;}   ///< does a click over this window pass through?
    bool           Dragable() const    {return m_flags & DRAGABLE;}    ///< does a click here become a drag? 
    bool           DragKeeper() const  {return m_flags & DRAG_KEEPER;} ///< when a drag is started on this obj, and it's non-dragable, does it need to receive all drag messages anyway?
    bool           Visible() const     {return m_visible;}             ///< is the window visible?
    bool           Resizable() const   {return m_flags & RESIZABLE;}   ///< can this window be resized using the mouse?
    bool           OnTop() const       {return m_flags & ONTOP;}       ///< is this an on-top window?
    bool           Modal() const       {return m_flags & MODAL;}       ///< is this a modal window?
    const string&  WindowText() const  {return m_text;}                ///< returns text associated with this window
    Pt             UpperLeft() const;                                  ///< returns the upper-left corner of window in \a screen \a coordinates (taking into account parent's screen position, if any)
    Pt             LowerRight() const;                                 ///< returns (one pixel past) the lower-right corner of window in \a screen \a coordinates (taking into account parent's screen position, if any)
    int            Width() const              {return m_lowerright.x - m_upperleft.x;} ///< returns width of window in pixels
    int            Height() const             {return m_lowerright.y - m_upperleft.y;} ///< returns width of window in pixels
    int            ZOrder() const             {return m_zorder;}       ///< returns the position of this window in the z-order (root (non-child) windows only)
    Pt             WindowDimensions() const   {return Pt(m_lowerright.x - m_upperleft.x, m_lowerright.y - m_upperleft.y);} ///< returns a \a Pt packed with width in \a x and height in \a y
    Pt             MinDimensions() const      {return m_min_size;} ///< returns the minimum allowable dimensions of window
    Pt             MaxDimensions() const      {return m_max_size;} ///< returns the maximum allowable dimensions of window
   
    /** returns upper-left corner of window's client area in screen coordinates (or of the entire area, if no client area is specified). 
        virtual b/c different windows have different shapes (and so ways of calculating client area)*/
    virtual Pt     ClientUpperLeft() const       {return UpperLeft();}
   
    /** returns (one pixel past) lower-right corner of window's client area in screen coordinates (or of the entire area, if no client area is specified). 
        virtual b/c different windows have different shapes (and so ways of calculating client area)*/
    virtual Pt     ClientLowerRight() const      {return LowerRight();}
   
    Pt             ClientDimensions() const      {return ClientLowerRight() - ClientUpperLeft();} ///< \see WindowDimensions()
   
    Pt             ScreenToWindow(const Pt& pt) const   {return pt - UpperLeft();}       ///< returns \a pt translated from screen- to window-coordinates
    Pt             ScreenToClient(const Pt& pt) const   {return pt - ClientUpperLeft();} ///< returns \a pt translated from screen- to client-coordinates
    virtual bool   InWindow(const Pt& pt) const         {return pt >= UpperLeft() && pt < LowerRight();} ///< returns true if screen-coordinate point \a pt falls within the window
    virtual bool   InClient(const Pt& pt) const         {return pt >= ClientUpperLeft() && pt < ClientLowerRight();} ///< returns true if screen-coordinate point \a pt falls within the window's client area
   
    Wnd*           Parent() const {return m_parent;}  ///< returns the window's parent (may be null)
    Wnd*           RootParent() const;                ///< returns the earliest ancestor window (may be null)
    virtual WndRegion WindowRegion(const Pt& pt) const;      ///< also virtual b/c of different window shapes
   
    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Wnd object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object.  This method is intended to be used in GG-Sketch.
    //@}
   
    /** \name Mutators */ //@{
    virtual void   SetText(const string& str)    {m_text = str;}   ///< set window text
    virtual void   SetText(const char* str)      {m_text = str;}   ///< set window text
    void           Hide(bool children = true);                     ///< suppresses rendering of this window (and possibly its children) during render loop
    void           Show(bool children = true);                     ///< enables rendering of this window (and possibly its children) during render loop
    void           MoveTo(int x, int y);                           ///< moves upper-left corner of window to \a x,\a y
    void           MoveTo(const Pt& pt);                           ///< moves upper-left corner of window to \a pt
    void           OffsetMove(int x, int y);                       ///< moves window by \a x, \a y pixels
    void           OffsetMove(const Pt& pt);                       ///< moves window by \a pt pixels
    void           SizeMove(const Pt& ul, const Pt& lr);           ///< resizes and/or moves window to new upper-left and lower right boundaries
    virtual void   SizeMove(int x1, int y1, int x2, int y2);       ///< resizes and/or moves window to new upper-left and lower right boundaries
    void           Resize(const Pt& sz);                           ///< resizes window without moving upper-left corner
    void           Resize(int x, int y);                           ///< resizes window without moving upper-left corner
    void           SetMinDimensions(const Pt& sz){m_min_size = sz;}///< sets the minimum allowable dimensions of window
    void           SetMaxDimensions(const Pt& sz){m_max_size = sz;}///< sets the maximum allowable dimensions of window
    void           AttachChild(Wnd* wnd);  ///< places \a wnd in child ptr list, sets's child's \a m_parent member to \a this
    void           MoveChildUp(Wnd* wnd);  ///< places \a wnd at the end of the child ptr list, so it is rendered last (on top of the other children)
    void           MoveChildDown(Wnd* wnd);///< places \a wnd at the beginning of the child ptr list, so it is rendered first (below the other children)
    void           DetachChild(Wnd* wnd);  ///< removes \a wnd from child ptr list, sets child's m_parent = 0
    void           DetachChildren();       ///< removes all Wnds from child ptr list, sets childrens' m_parent = 0
    void           DeleteChild(Wnd* wnd);  ///< removes, detaches, and deletes \a wnd; does nothing if wnd is not in the child list
    void           DeleteChildren();       ///< removes, detaches, and deletes all Wnds in the child list
    // message functions
    virtual int    Render();                                         ///< draw self on screen; a return value of 0 that children should be skipped in subsequent rendering
    virtual int    LButtonDown(const Pt& pt, Uint32 keys);           ///< respond to left button down msg.  A window receives this whenever any input device button changes from up to down while over the window.
    virtual int    LDrag(const Pt& pt, const Pt& move, Uint32 keys); ///< respond to drag msg (even if this wnd is not dragable).   Drag messages are only sent to the window over which the button was dressed at teh beginning of the drag. A window receives this whenever any input device button is down and the mouse is moving while over the window.  If a window has the DRAG_KEEPER flag set, the window will also receive drag messages when the mouse is being dragged outside the window's area.
    virtual int    LButtonUp(const Pt& pt, Uint32 keys);             ///< respond to release of left mouse button outside window, if it was originally depressed over window.  A window will receive an LButtonUp() message whenever a drag that started over its area ends, even if the cursor is not currently over the window when this happens.
    virtual int    LClick(const Pt& pt, Uint32 keys);                ///< respond to release of left mouse button over window, if it was also originally depressed over window.  A window will receive an LButtonUp() message whenever a drag that started over its area ends over its area as well.
    virtual int    LDoubleClick(const Pt& pt, Uint32 keys);          ///< respond to second left click in window within the time limit.  A window will receive an LButtonUp() message instead of an LButtonDown() or LClick() message if the left input device button is pressed over a window that was l-clicked within a double-click time interval.  Note that this means a double click is always preceded by a click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in during the interval.
    virtual int    RButtonDown(const Pt& pt, Uint32 keys);           ///< respond to right button down msg. \see LButtonDown()
    virtual int    RClick(const Pt& pt, Uint32 keys);                ///< respond to release of right mouse button over window, if it was also originally depressed over window. \see LButtonUp()
    virtual int    RDoubleClick(const Pt& pt, Uint32 keys);          ///< respond to second right click in window within the time limit.  A window will receive an RButtonUp() message instead of an RButtonDown() or RClick() message if the right input device button is pressed over a window that was r-clicked within a double-click time interval  Note that this means a double click is always preceded by a click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in during the interval.
    virtual int    MouseEnter(const Pt& pt, Uint32 keys);            ///< respond to cursor entering window's coords
    virtual int    MouseHere(const Pt& pt, Uint32 keys);             ///< respond to cursor moving about in window's coords.  A MouseHere() message will not be generated the first time the cursor enters the window's area.  In that case, a MouseEnter() message is generated.
    virtual int    MouseLeave(const Pt& pt, Uint32 keys);            ///< respond to cursor leaving window's coords
    virtual int    MouseWheel(const Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    virtual int    Keypress(Key key, Uint32 key_mods);               ///< respond to keystrokes (focus window only).  A window may receive Keypress() messages passed up to it from its children.  For instance, many Control-derived classes pass Keypress() messages to their Parent() windows by default.
    virtual int    GainingFocus();                                   ///< respond to this window gaining the input focus
    virtual int    LosingFocus();                                    ///< respond to this window losing the input focus

    /** this executes a modal window and gives it its modality.  For non-modal windows, this function is a no-op.
        It returns 0 if the window is non-modal, or non-zero after successful modal execution.*/
    virtual int    Run();
    //@}

protected:
    /** \name Structors */ //@{
    Wnd(); ///< default ctor
    Wnd(int x, int y, int w, int h, Uint32 flags = CLICKABLE | DRAGABLE); ///< ctor that allows a size and position to be specified, as well as creation flags
    Wnd(const XMLElement& elem); ///< ctor that constructs a Wnd object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Wnd object
    //@}
   
    /** \name Accessors */ //@{
    const list<Wnd*>& Children() {return m_children;} ///< returns child list; the list is const, but the children may be manipulated
    //@}

    string         m_text;           ///< text associated with the window, such as a window title or button label, etc.
    bool           m_done;           ///< derived modal Wnd's set this to true to stop modal loop

private:
    void ValidateFlags();            ///< sanity-checks the window creation flags

    Wnd*           m_parent;         ///< ptr to this window's parent; =0 if this is the root
    list<Wnd*>     m_children;       ///< list of ptrs to child windows kept in order of decreasing area
    int            m_zorder;         ///< where this window is in the z-order (root (non-child) windows only)
    bool           m_visible;        ///< is this window drawn?
    Pt             m_upperleft;      ///< upper left point of window
    Pt             m_lowerright;     ///< lower right point of window
    Pt             m_min_size;       ///< minimum window size Pt(0, 0) (= none) by default
    Pt             m_max_size;       ///< maximum window size Pt(1 << 30, 1 << 30) (= none) by default
   
    Uint32         m_flags;          ///< flags supplied at window creation for clickability, dragability, drag-keeperness, and resizability

    friend class App;                ///< App needs access to \a m_zorder, m_children, etc.
    friend class ZList;              ///< ZList needs access to \a m_zorder in order to order windows
};

// define EnumMap and stream operators for Wnd::WndFlag
ENUM_MAP_BEGIN(Wnd::WndFlag)
    ENUM_MAP_INSERT(Wnd::CLICKABLE)
    ENUM_MAP_INSERT(Wnd::DRAGABLE)
    ENUM_MAP_INSERT(Wnd::DRAG_KEEPER)
    ENUM_MAP_INSERT(Wnd::RESIZABLE)
    ENUM_MAP_INSERT(Wnd::ONTOP)
    ENUM_MAP_INSERT(Wnd::MODAL)
ENUM_MAP_END

ENUM_STREAM_IN(Wnd::WndFlag)
ENUM_STREAM_OUT(Wnd::WndFlag)

} // namespace GG

#endif // _GGWnd_h_

