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

/** \file GGWnd.h
    Contains the Wnd class, upon which all GG GUI elements are based. */

#ifndef _GGWnd_h_
#define _GGWnd_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

#include <list>
#include <set>

#include <boost/serialization/access.hpp>

namespace GG {

class Layout;

/** This is the basic GG window class.

    <p>Window Geometry
    <br>The coordinates of Wnd boundaries are STL-style, as are most range-values throughout GG, meaning that
    LowerRight() denotes the "last + 1" pixel of a Wnd.  The on-screen representation of a rectangular Wnd covers the
    pixels from UpperLeft() to LowerRight() - Pt(1, 1), \a not UpperLeft() to LowerRight().  Each Wnd has a client area
    from ClientUpperLeft() to ClientLowerRight().  These two methods are virtual, and may return anything the user
    likes; the default implementation is to return UpperLeft() and LowerRight(), respectively, meaning that the client
    area is the entire window.

    <p>Child Windows
    <br>It is assumed that child windows exists within the boundaries of their parents, although this is not required.
    By default, Wnds do not clip their children; child clipping can be turned on or off using EnableChildClipping(),
    which clips all children to the client area of the Wnd.  Subclasses can override BeginClipping() and EndClipping()
    if the clipping desired is something other than the client area of the Wnd, or if the Wnd is not rectangular.
    Regardless of clipping, all clicks that land on a child but outside of the parent will not reach the child, since
    clicks are detected by seaching the top-level Wnds and then searching the children within the ones that are hit.
    Ideally, "sibling" child windows should not overlap (unless they can without interfering).  If this is impossible or
    undesirable and control is needed over the order in which children are rendered, MoveChildUp() and MoveChildDown()
    provide such control.

    <p>Effects of Window-Creation Flags
    <br>DragKeeper() windows are those that maintain cursor drags, even when they leave the bounds of the window, and
    even though such windows are not themselves draggable.  This is useful for such controls as Edit and Scroll.
    <br>Resizable() windows are able to be stretched by the user, by dragging the areas of the window outside the client
    area.  So the RESIZABLE flag will have no effect on a window that does not have non-default ClientUpperLeft() and/or
    ClientLowerRight().  The WindowRegion() method can also be overidden in derived classes, and can return regions that
    are appropriate to nonrectangular windows, or those whose client area must cover the entire window.
    <br>OnTop() windows are drawn after all others (except Modal() ones), to ensure that they remain on top.  This means
    that other non-OnTop() windows that are moved to the top of the z-order stop at some z-value below the lowest
    OnTop() window in the z-order.  On-topness is useful for modeless dialog boxes, among other things.
    <br>Modal() windows are available (by setting the MODAL window creation flag), and are also always-on-top, but are
    handled differently and do not have ONTOP specified in their creation flags.  Modal windows are executed by calling
    Run(), which registers them as modal windows and starts the local execution of the application's event pump.
    Execution of the code that calls Run() is effectively halted until Run() returns.  Derived classes that wish to use
    modal execution should set m_done = true to escape from the modal execution loop.  EventPump has more information
    about processing during modal dialog execution.

    <p>Signal Considerations
    <br>Wnd inherits from boost::signals::trackable.  This means that any slots contained in a Wnd object or Wnd-derived
    object will automatically be disconnected from any connected signals when the Wnd is destroyed.  Every Wnd responds
    to input as driven by the singleton App object.

    <p>Event Filters
    <br>Every Wnd can also have its incoming Events filtered by an arbitrary number of other Wnds.
    Each such Wnds in a Wnd's "filter chain" gets an opportunity, one at a time, to process an incoming Event, or pass
    it on to the next filter in the chain.  If all EventFilter() calls in the chain return false, the filtered Wnd then
    gets the chance to process the Event as normal.  Filter Wnds are traversed in reverse order that they are installed,
    and no filter Wnd can be in a filter chain more than once.  Installing the same filter Wnd more than once removes
    the Wnd from the filter chain and re-adds it to the beginning of the chain.  Note that the default implementation of
    EventFilter() is to return false and do nothing else, so installing a Wnd-derived type with no overridden
    EventFilter() in a filter Wnd will have no effect.  Also note that just as it is legal for keyboard accelerator
    slots to do nontrivial work and still return false (causing a keystroke event to be generated), EventFilter() may
    return false even when it does nontrivial work, and the next filter in the chain will also get a chance to process
    the Event.  It is even possible to have an arbitrary number of filters that all do processing on an Event, and
    finally let the filtered Wnd do its normal Event processing.

    <p>Layouts
    <br>Layouts arrange children in the client area of a window, and can be assigned to a window in 4 ways.
    HorizontalLayout(), VerticalLayout(), and GridLayout() all arrange the window's children automatically, and take
    ownership of them as their own children, becoming the window's only child.  Any existing layout is removed first.
    SetLayout() allows you to attach a pre-configured Layout object directly, without automatically arranging the
    window's children.  Because SetLayout() does no auto-arrangement, it does not know how to place any children the
    window may have at the time it is called; for this reason, it not only removes any previous layout, but deletes all
    current children as well.  Therefore, SetLayout() should usually only be called before any children are attached to
    a window; all children should be attached directly to the layout.
    <br>When a window has an attached layout and is resized, it resizes its layout automatically.  Further, if a window
    is part of a layout, it notifies its containing layout whenever it is moved, resized, or has its MinSize() changed.
    This ensures that layouts are always current.

    <p>Note that while a Wnd can contain arbitrary Wnd-derived children, in order for such children to be automatically
    serialized, any user-defined Wnd subclasses must be registered.  See the boost serialization documentation for
    details. */
class GG_API Wnd : public boost::signals::trackable
{
public:
    /// window creation flags
    enum WndFlag {
        CLICKABLE =    1 << 0,  ///< clicks hit this window, rather than passing through it
        DRAGABLE =     1 << 1,  ///< this window can be dragged around independently
        DRAG_KEEPER =  1 << 2,  ///< this window receives drag messages, even if it is not dragable
        RESIZABLE =    1 << 3,  ///< this window can be resized by the user, with the mouse
        ONTOP =        1 << 4,  ///< this windows is an "on-top" window, and will always appear above all non-on-top and non-modal windows
        MODAL =        1 << 5   ///< this window is modal; while it is active, no other windows are interactive.  Modal windows are considered above "on-top" windows, and should not be flagged as ONTOP.
    };

    /** exception class \see GG::GGEXCEPTION */
    GGEXCEPTION(WndException);

    /** \name Structors */ //@{
    virtual ~Wnd(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    bool           Clickable() const;    ///< does a click over this window pass through?
    bool           Dragable() const;     ///< does a click here become a drag? 
    bool           DragKeeper() const;   ///< when a drag is started on this obj, and it's non-dragable, does it need to receive all drag messages anyway?
    bool           Resizable() const;    ///< can this window be resized using the mouse?
    bool           OnTop() const;        ///< is this an on-top window?
    bool           Modal() const;        ///< is this a modal window?
    bool           ClipChildren() const; ///< is child clipping enabled?
    bool           Visible() const;      ///< is the window visible?
    const std::string&
                   WindowText() const;   ///< returns text associated with this window
    Pt             UpperLeft() const;    ///< returns the upper-left corner of window in \a screen \a coordinates (taking into account parent's screen position, if any)
    Pt             LowerRight() const;   ///< returns (one pixel past) the lower-right corner of window in \a screen \a coordinates (taking into account parent's screen position, if any)
    int            Width() const;        ///< returns width of window in pixels
    int            Height() const;       ///< returns width of window in pixels
    int            ZOrder() const;       ///< returns the position of this window in the z-order (root (non-child) windows only)
    Pt             Size() const;         ///< returns a \a Pt packed with width in \a x and height in \a y
    Pt             MinSize() const;      ///< returns the minimum allowable size of window
    Pt             MaxSize() const;      ///< returns the maximum allowable size of window

    /** returns upper-left corner of window's client area in screen coordinates (or of the entire area, if no client area is specified). 
        virtual b/c different windows have different shapes (and so ways of calculating client area)*/
    virtual Pt     ClientUpperLeft() const;

    /** returns (one pixel past) lower-right corner of window's client area in screen coordinates (or of the entire area, if no client area is specified). 
        virtual b/c different windows have different shapes (and so ways of calculating client area)*/
    virtual Pt     ClientLowerRight() const;

    /** returns the size of the client area \see Size() */
    Pt             ClientSize() const;

    int            ClientWidth() const;                 ///< returns the width of the client area
    int            ClientHeight() const;                ///< returns the height of the client area

    Pt             ScreenToWindow(const Pt& pt) const;  ///< returns \a pt translated from screen- to window-coordinates
    Pt             ScreenToClient(const Pt& pt) const;  ///< returns \a pt translated from screen- to client-coordinates
    virtual bool   InWindow(const Pt& pt) const;        ///< returns true if screen-coordinate point \a pt falls within the window
    virtual bool   InClient(const Pt& pt) const;        ///< returns true if screen-coordinate point \a pt falls within the window's client area

    Wnd*           Parent() const;                      ///< returns the window's parent (may be null)
    Wnd*           RootParent() const;                  ///< returns the earliest ancestor window (may be null)

    const Layout* GetLayout() const;                    ///< returns the layout for the window, if any
    const Layout* ContainingLayout() const;             ///< returns the layout containing the window, if any

    virtual WndRegion WindowRegion(const Pt& pt) const; ///< also virtual b/c of different window shapes
    //@}

    /** \name Mutators */ //@{
    virtual void   SetText(const std::string& str);          ///< set window text
    virtual void   SetText(const char* str);                 ///< set window text
    void           Hide(bool children = true);               ///< suppresses rendering of this window (and possibly its children) during render loop
    void           Show(bool children = true);               ///< enables rendering of this window (and possibly its children) during render loop
    virtual void   ModalInit();                              ///< called during Run(), after a modal window is registered, this is the place that subclasses should put specialized modal window initialization, such as setting focus to child controls
    void           EnableChildClipping(bool enable = true);  ///< enables or disables clipping of child windows to the boundaries of this Wnd
    virtual void   BeginClipping();                          ///< sets up child clipping for this window
    virtual void   EndClipping();                            ///< restores state to what it was before BeginClipping() was called
    void           MoveTo(const Pt& pt);                     ///< moves upper-left corner of window to \a pt
    void           MoveTo(int x, int y);                     ///< moves upper-left corner of window to \a x,\a y
    void           OffsetMove(const Pt& pt);                 ///< moves window by \a pt pixels
    void           OffsetMove(int x, int y);                 ///< moves window by \a x, \a y pixels
    void           SizeMove(const Pt& ul, const Pt& lr);     ///< resizes and/or moves window to new upper-left and lower right boundaries
    virtual void   SizeMove(int x1, int y1, int x2, int y2); ///< resizes and/or moves window to new upper-left and lower right boundaries
    void           Resize(const Pt& sz);                     ///< resizes window without moving upper-left corner
    void           Resize(int x, int y);                     ///< resizes window without moving upper-left corner
    void           SetMinSize(const Pt& sz);                 ///< sets the minimum allowable size of window \a pt
    void           SetMinSize(int x, int y);                 ///< sets the minimum allowable size of window to \a x,\a y
    void           SetMaxSize(const Pt& sz);                 ///< sets the maximum allowable size of window \a pt
    void           SetMaxSize(int x, int y);                 ///< sets the maximum allowable size of window to \a x,\a y
    void           AttachChild(Wnd* wnd);                    ///< places \a wnd in child ptr list, sets's child's \a m_parent member to \a this
    void           MoveChildUp(Wnd* wnd);                    ///< places \a wnd at the end of the child ptr list, so it is rendered last (on top of the other children)
    void           MoveChildDown(Wnd* wnd);                  ///< places \a wnd at the beginning of the child ptr list, so it is rendered first (below the other children)
    void           DetachChild(Wnd* wnd);                    ///< removes \a wnd from child ptr list, sets child's m_parent = 0
    void           DetachChildren();                         ///< removes all Wnds from child ptr list, sets childrens' m_parent = 0
    void           DeleteChild(Wnd* wnd);                    ///< removes, detaches, and deletes \a wnd; does nothing if wnd is not in the child list
    void           DeleteChildren();                         ///< removes, detaches, and deletes all Wnds in the child list
    void           InstallEventFilter(Wnd* wnd);             ///< adds \a wnd to the front of the event filtering chain
    void           RemoveEventFilter(Wnd* wnd);              ///< removes \a wnd from the filter chain

    /** places the window's children in a horizontal layout, handing ownership of the window's children over to the
        layout.  Removes any current layout which may exist. */
    void           HorizontalLayout();

    /** places the window's children in a vertical layout, handing ownership of the window's children over to the
        layout.  Removes any current layout which may exist. */
    void           VerticalLayout();

    /** places the window's children in a grid layout, handing ownership of the window's children over to the layout.
        Removes any current layout which may exist. */
    void           GridLayout();

    /** sets \a layout as the layout for the window.  Removes any current layout which may exist, and deletes all child
        windows. */
    void           SetLayout(Layout* layout);

    /** removes the window's layout, handing ownership of all children back to the window.  If no layout exists for the
        window, no action is taken. */
    void           RemoveLayout();

    /** sets the margin that should exist between the outer edges of the windows in the layout and the edge of the
        client area.  If no layout exists for the window, this has no effect. */
    void           SetLayoutBorderMargin(int margin);

    /** sets the margin that should exist between the windows in the layout.  If no layout exists for the window, this
        has no effect. */
    void           SetLayoutCellMargin(int margin);

    virtual bool   Render();                                 ///< draws this Wnd in scene; a return value of false that children should be skipped in subsequent rendering
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);   ///< respond to left button down msg.  A window receives this whenever any input device button changes from up to down while over the window.
    virtual void   LDrag(const Pt& pt, const Pt& move, Uint32 keys); ///< respond to drag msg (even if this Wnd is not dragable).   Drag messages are only sent to the window over which the button was dressed at teh beginning of the drag. A window receives this whenever any input device button is down and the mouse is moving while over the window.  If a window has the DRAG_KEEPER flag set, the window will also receive drag messages when the mouse is being dragged outside the window's area.
    virtual void   LButtonUp(const Pt& pt, Uint32 keys);             ///< respond to release of left mouse button outside window, if it was originally depressed over window.  A window will receive an LButtonUp() message whenever a drag that started over its area ends, even if the cursor is not currently over the window when this happens.
    virtual void   LClick(const Pt& pt, Uint32 keys);                ///< respond to release of left mouse button over window, if it was also originally depressed over window.  A window will receive an LButtonUp() message whenever a drag that started over its area ends over its area as well.
    virtual void   LDoubleClick(const Pt& pt, Uint32 keys);          ///< respond to second left click in window within the time limit.  A window will receive an LButtonUp() message instead of an LButtonDown() or LClick() message if the left input device button is pressed over a window that was l-clicked within a double-click time interval.  Note that this means a double click is always preceded by a click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in during the interval.

    /** respond to right button down msg. \see LButtonDown() */
    virtual void   RButtonDown(const Pt& pt, Uint32 keys);

    /** respond to release of right mouse button over window, if it was also originally depressed over window. \see LButtonUp() */
    virtual void   RClick(const Pt& pt, Uint32 keys);

    virtual void   RDoubleClick(const Pt& pt, Uint32 keys);          ///< respond to second right click in window within the time limit.  A window will receive an RButtonUp() message instead of an RButtonDown() or RClick() message if the right input device button is pressed over a window that was r-clicked within a double-click time interval  Note that this means a double click is always preceded by a click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in during the interval.
    virtual void   MouseEnter(const Pt& pt, Uint32 keys);            ///< respond to cursor entering window's coords
    virtual void   MouseHere(const Pt& pt, Uint32 keys);             ///< respond to cursor moving about in window's coords.  A MouseHere() message will not be generated the first time the cursor enters the window's area.  In that case, a MouseEnter() message is generated.
    virtual void   MouseLeave(const Pt& pt, Uint32 keys);            ///< respond to cursor leaving window's coords
    virtual void   MouseWheel(const Pt& pt, int move, Uint32 keys);  ///< respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down)
    virtual void   Keypress(Key key, Uint32 key_mods);               ///< respond to keystrokes (focus window only).  A window may receive Keypress() messages passed up to it from its children.  For instance, many Control-derived classes pass Keypress() messages to their Parent() windows by default.
    virtual void   GainingFocus();                                   ///< respond to this window gaining the input focus
    virtual void   LosingFocus();                                    ///< respond to this window losing the input focus

    /** this executes a modal window and gives it its modality.  For non-modal windows, this function is a no-op.
        It returns 0 if the window is non-modal, or non-zero after successful modal execution.*/
    virtual int    Run();
    //@}

protected:
    /** encapsulates a Wnd event that is passed from the singleton App to a Wnd.  The various types of Events correspond to 
        the various message member functions of Wnd, some of which have different parameterizations.  Rather than have a
        less-efficient but more-easily-extensible hierarchy of Event types, a single Event type exists that has all possible
        parameters to a Wnd message function call.  Therefore, not all of Event's accessors will return sensical results, 
        depending on the EventType of the Event.  Note that Wnd events may be filtered before they actually reach the target 
        Wnd \see Wnd */
    class GG_API Event
    {
    public:
        /** the types of Wnd events.  Each of these corresponds to a Wnd member function of the same name. */
        enum EventType {
            LButtonDown,
            LDrag,
            LButtonUp,
            LClick,
            LDoubleClick,
            RButtonDown,
            RClick,
            RDoubleClick,
            MouseEnter,
            MouseHere,
            MouseLeave,
            MouseWheel,
            Keypress,
            GainingFocus,
            LosingFocus
        };

        /** constructs an Event that is used to invoke a function taking parameters (const GG::Pt& pt, Uint32 keys), eg LButtonDown(). */
        Event(EventType type, const GG::Pt& pt, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (const Pt& pt, const Pt& move, Uint32 keys), eg LDrag(). */
        Event(EventType type, const Pt& pt, const Pt& move, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (const Pt& pt, int move, Uint32 keys), eg MouseWheel(). */
        Event(EventType type, const Pt& pt, int move, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (Key key, Uint32 key_mods), eg Keypress(). */
        Event(EventType type, Key key, Uint32 key_mods);

        /** constructs an Event that is used to invoke a function taking no parameters, eg GainingFocus(). */
        Event(EventType type);

        EventType      Type() const;      ///< returns the type of the Event
        const GG::Pt&  Point() const;     ///< returns the point at which the event took place, if any
        Key            KeyPress() const;  ///< returns the keypress represented by the Event, if any
        Uint32         KeyMods() const;   ///< returns the modifiers to the Event's keypress, if any
        const GG::Pt&  DragMove() const;  ///< returns the amount of drag movement represented by the Event, if any
        int            WheelMove() const; ///< returns the ammount of mouse wheel movement represented by the Event, if any

    private:
        EventType  m_type;
        GG::Pt     m_point;
        Key        m_keypress;
        Uint32     m_key_mods;
        GG::Pt     m_drag_move;
        int        m_wheel_move;
    };

    /** \name Structors */ //@{
    Wnd(); ///< default ctor
    Wnd(int x, int y, int w, int h, Uint32 flags = CLICKABLE | DRAGABLE); ///< ctor that allows a size and position to be specified, as well as creation flags
    //@}

    /** \name Accessors */ //@{
    const std::list<Wnd*>& Children() const; ///< returns child list; the list is const, but the children may be manipulated
    //@}

    /** \name Mutators */ //@{
    virtual bool EventFilter(Wnd* w, const Event& event); ///< handles an Event destined for Wnd \a w, but which this Wnd is allowed to handle first.  Returns true if this filter processed the message.

    Layout*      GetLayout();        ///< returns the layout for the window, if any
    Layout*      ContainingLayout(); ///< returns the layout containing the window, if any
    //@}

    std::string  m_text; ///< text associated with the window, such as a window title or button label, etc.
    bool         m_done; ///< derived modal Wnd's set this to true to stop modal loop

private:
    void ValidateFlags();                 ///< sanity-checks the window creation flags
    void HandleEvent(const Event& event); ///< handles all messages, and calls appropriate function (LButtonDown(), LDrag(), etc.)

    Wnd*           m_parent;            ///< ptr to this window's parent; may be 0
    std::list<Wnd*>m_children;          ///< list of ptrs to child windows kept in order of decreasing area
    int            m_zorder;            ///< where this window is in the z-order (root (non-child) windows only)
    bool           m_visible;           ///< is this window drawn?
    bool           m_clip_children;     ///< should the children of this window be clipped?
    Pt             m_upperleft;         ///< upper left point of window
    Pt             m_lowerright;        ///< lower right point of window
    Pt             m_min_size;          ///< minimum window size Pt(0, 0) (= none) by default
    Pt             m_max_size;          ///< maximum window size Pt(1 << 30, 1 << 30) (= none) by default
    std::vector<Wnd*>
                   m_filters;           ///< the Wnds that are filtering this Wnd's events. These are in reverse order: top of the stack is back().
    std::set<Wnd*> m_filtering;         ///< the Wnds in whose filter chains this Wnd lies
    Layout*        m_layout;            ///< the layout for this Wnd, if any
    Layout*        m_containing_layout; ///< the layout that contains this Wnd, if any
    Uint32         m_flags;             ///< flags supplied at window creation for clickability, dragability, drag-keeping, and resizability

    friend class App;   ///< App needs access to \a m_zorder, m_children, etc.
    friend class ZList; ///< ZList needs access to \a m_zorder in order to order windows

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// define EnumMap and stream operators for Wnd::WndFlag
GG_ENUM_MAP_BEGIN(Wnd::WndFlag)
    GG_ENUM_MAP_INSERT(Wnd::CLICKABLE)
    GG_ENUM_MAP_INSERT(Wnd::DRAGABLE)
    GG_ENUM_MAP_INSERT(Wnd::DRAG_KEEPER)
    GG_ENUM_MAP_INSERT(Wnd::RESIZABLE)
    GG_ENUM_MAP_INSERT(Wnd::ONTOP)
    GG_ENUM_MAP_INSERT(Wnd::MODAL)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(Wnd::WndFlag)
GG_ENUM_STREAM_OUT(Wnd::WndFlag)

} // namespace GG

// template implementations
template <class Archive>
void GG::Wnd::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_text)
        & BOOST_SERIALIZATION_NVP(m_done)
        & BOOST_SERIALIZATION_NVP(m_parent)
        & BOOST_SERIALIZATION_NVP(m_children)
        & BOOST_SERIALIZATION_NVP(m_zorder)
        & BOOST_SERIALIZATION_NVP(m_visible)
        & BOOST_SERIALIZATION_NVP(m_clip_children)
        & BOOST_SERIALIZATION_NVP(m_upperleft)
        & BOOST_SERIALIZATION_NVP(m_lowerright)
        & BOOST_SERIALIZATION_NVP(m_min_size)
        & BOOST_SERIALIZATION_NVP(m_max_size)
        & BOOST_SERIALIZATION_NVP(m_filters)
        & BOOST_SERIALIZATION_NVP(m_filtering)
        & BOOST_SERIALIZATION_NVP(m_layout)
        & BOOST_SERIALIZATION_NVP(m_containing_layout)
        & BOOST_SERIALIZATION_NVP(m_flags);
}

#endif // _GGWnd_h_
