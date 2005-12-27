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

class BrowseInfoWnd;
class Layout;
class StyleFactory;
class WndEditor;

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

    <p>Browse Info
    <br>Browse info is a non-interactive informational window that pops up after the user keeps the mouse over the Wnd
    for a certain period of time.  This can reproduce "tooltip"-like functionality, but is not limited to displaying
    only text.  An arbitrary BrowseInfoWnd-derived window can be displayed.  There can be multiple browse info modes,
    numbered 0 through N.  Each mode has a time associated with it, and after the associated time has elapsed, that mode
    is entered.  This is intended to allow different levels of detail to be shown for different lengths of mouse
    presence.  For instance, hovering over a Wnd for 1 second might produce a box that says "FooWnd", but leaving it
    there a total of 2 seconds might produce a box that says "FooWnd: currently doing nothing".  When the mouse leaves
    the Wnd, a click occurs, etc., the Wnd reverts to browse mode -1, indicating that no browse info should be
    displayed.  By default, every Wnd has a single browse info mode at time DefaultBrowseTime(), using the
    DefaultBrowseInfoWnd(), with no associated text.  Note that DefaultBrowseInfoWnd() returns a null window unless it
    is set by the user, and as this implies, it is legal to have no BrowseInfoWnd associated with a browse mode, in
    which case nothing is shown.  Also note that it is legal to have no text associated with a browse mode. \see
    BrowseInfoWnd

    <p>Style Factory
    <br>A StyleFactory is responsible for creating controls and dialogs that other Wnds may need (e.g. when Slider needs
    to create a Button for its sliding tab).  There is an app-wide StyleFactory available, but for complete
    customization, each Wnd may have one installed as well.  The GetStyleFactory() method returns the one installed in
    the Wnd, if one exists, or the app-wide one otherwise.

    <p>Note that while a Wnd can contain arbitrary Wnd-derived children, in order for such children to be automatically
    serialized, any user-defined Wnd subclasses must be registered.  See the boost serialization documentation for
    details, and/or the serialization tutorial for examples. */
class GG_API Wnd : public boost::signals::trackable
{
public:
    /** The data necessary to represent a browse info mode.  Though \a browse_text will not apply to all browse info
        schemes, it is nevertheless part of BrowseInfoMode, since it will surely be the most common data displayed in a
        BrowseInfoWnd. */
    struct GG_API BrowseInfoMode
    {
        int                              time; ///< the time the cursor must linger over the Wnd before this mode becomes active, in ms
        boost::shared_ptr<BrowseInfoWnd> wnd;  ///< the BrowseInfoWnd used to display the browse info for this mode
        std::string                      text; ///< the text to display in the BrowseInfoWnd shown for this mode
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** \name Structors */ //@{
    virtual ~Wnd(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    bool           Clickable() const;    ///< does a click over this window pass through?
    bool           Dragable() const;     ///< does a click here become a drag? 

    /** when a drag is started on this obj, and it's non-dragable, does it need to receive all drag messages anyway? */
    bool           DragKeeper() const;

    bool           Resizable() const;    ///< can this window be resized using the mouse?
    bool           OnTop() const;        ///< is this an on-top window?
    bool           Modal() const;        ///< is this a modal window?
    bool           ClipChildren() const; ///< is child clipping enabled?
    bool           Visible() const;      ///< is the window visible?
    const std::string&
                   WindowText() const;   ///< returns text associated with this window

    /** returns the string key that defines the type of data that this Wnd represents when used as a drag-drop Wnd.
        Returns the empty string when this Wnd cannot be used as a drag-drop Wnd. */
    const std::string&
                   DragDropDataType() const;

    /** returns the upper-left corner of window in \a screen \a coordinates (taking into account parent's screen
        position, if any) */
    Pt             UpperLeft() const;

    /** returns (one pixel past) the lower-right corner of window in \a screen \a coordinates (taking into account
        parent's screen position, if any) */
    Pt             LowerRight() const;

    /** returns the upper-left corner of window, relative to its parent's client area, or in screen coordinates if no
        parent exists. */
    Pt             RelativeUpperLeft() const;

    /** returns (one pixel past) the lower-right corner of window, relative to its parent's client area, or in screen
        coordinates if no parent exists. */
    Pt             RelativeLowerRight() const;

    int            Width() const;        ///< returns width of window in pixels
    int            Height() const;       ///< returns width of window in pixels
    int            ZOrder() const;       ///< returns the position of this window in the z-order (root (non-child) windows only)
    Pt             Size() const;         ///< returns a \a Pt packed with width in \a x and height in \a y
    Pt             MinSize() const;      ///< returns the minimum allowable size of window
    Pt             MaxSize() const;      ///< returns the maximum allowable size of window

    /** returns upper-left corner of window's client area in screen coordinates (or of the entire area, if no client
        area is specified).  Virtual because different windows have different shapes (and so ways of calculating client
        area) */
    virtual Pt     ClientUpperLeft() const;

    /** returns (one pixel past) lower-right corner of window's client area in screen coordinates (or of the entire
        area, if no client area is specified).  Virtual because different windows have different shapes (and so ways of
        calculating client area)*/
    virtual Pt     ClientLowerRight() const;

    /** returns the size of the client area \see Size() */
    Pt             ClientSize() const;

    int            ClientWidth() const;                 ///< returns the width of the client area
    int            ClientHeight() const;                ///< returns the height of the client area

    Pt             ScreenToWindow(const Pt& pt) const;  ///< returns \a pt translated from screen- to window-coordinates
    Pt             ScreenToClient(const Pt& pt) const;  ///< returns \a pt translated from screen- to client-coordinates
    virtual bool   InWindow(const Pt& pt) const;        ///< returns true if screen-coordinate point \a pt falls within the window

    /** returns true if screen-coordinate point \a pt falls within the window's client area */
    virtual bool   InClient(const Pt& pt) const;

    Wnd*           Parent() const;                      ///< returns the window's parent (may be null)
    Wnd*           RootParent() const;                  ///< returns the earliest ancestor window (may be null)

    const Layout*  GetLayout() const;                   ///< returns the layout for the window, if any
    const Layout*  ContainingLayout() const;            ///< returns the layout containing the window, if any

    /** returns the browse modes for the Wnd, including time cutoffs (in milliseconds), the BrowseInfoWnds to be
        displayed for each browse info mode, and the text (if any) to be displayed in each mode.  As the time that the
        cursor is over this Wnd exceeds each mode's time, the corresponding Wnd is shown superimposed over this Wnd and
        its children.  Set the first time cutoff to 0 for immediate browse info display. */
    const std::vector<BrowseInfoMode>&  BrowseModes() const;

    /** returns the text to display for browse info mode \a mode.  \throw std::out_of_range May throw
        std::out_of_range if \a mode is not a valid browse mode. */
    const std::string& BrowseInfoText(int mode) const;

    const boost::shared_ptr<StyleFactory>& GetStyleFactory() const; ///< returns the currently-installed style factory if none exists, or the app-wide one otherwise

    virtual WndRegion WindowRegion(const Pt& pt) const; ///< also virtual b/c of different window shapes
    //@}

    /** \name Mutators */ //@{
    /** sets the string key that defines the type of data that this Wnd represents when used as a drag-drop Wnd.
        This should be set to the empty string when this Wnd cannot be used as a drag-drop Wnd. */
    void SetDragDropDataType(const std::string& data_type);

    /** indicates to the Wnd that a child widget \a w is being drag-dropped, which gives it the opportunity to add other
        associated drag-drop Wnds.  \a offset indicates the position of the mouse realtive to \a wnd's UpperLeft(). */
    virtual void StartingChildDragDrop(const Wnd* wnd, const Pt& offset);

    /** handles a drop of a drag-drop wnd into this Wnd; returns true iff the drag was accepted.  This function will
        will return false if any DontAcceptDropException occurs within it or any function called by it, and will stop
        the propogation of the exception. */
    virtual bool AcceptDrop(Wnd* wnd, const Pt& pt);

    /** handles the removal of a child window that has been dropped onto another window which has accepted it as a drop.
        The accepting window retains ownership, so this function must not delete the child. */
    virtual void ChildDraggedAway(Wnd* child, const Wnd* destination);

    virtual void   SetText(const std::string& str);     ///< set window text

    /** suppresses rendering of this window (and possibly its children) during render loop */
    void           Hide(bool children = true);

    /** enables rendering of this window (and possibly its children) during render loop */
    void           Show(bool children = true);

    /** called during Run(), after a modal window is registered, this is the place that subclasses should put
        specialized modal window initialization, such as setting focus to child controls */
    virtual void   ModalInit();

    /** enables or disables clipping of child windows to the boundaries of this Wnd */
    void           EnableChildClipping(bool enable = true);

    virtual void   BeginClipping();                     ///< sets up child clipping for this window
    virtual void   EndClipping();                       ///< restores state to what it was before BeginClipping() was called
    void           MoveTo(const Pt& pt);                ///< moves upper-left corner of window to \a pt
    void           OffsetMove(const Pt& pt);            ///< moves window by \a pt pixels

    /** resizes and/or moves window to new upper-left and lower right boundaries */
    virtual void   SizeMove(const Pt& ul, const Pt& lr);

    void           Resize(const Pt& sz);                ///< resizes window without moving upper-left corner
    void           SetMinSize(const Pt& sz);            ///< sets the minimum allowable size of window \a pt
    void           SetMaxSize(const Pt& sz);            ///< sets the maximum allowable size of window \a pt

    /** places \a wnd in child ptr list, sets's child's \a m_parent member to \a this */
    void           AttachChild(Wnd* wnd);

    /** places \a wnd at the end of the child ptr list, so it is rendered last (on top of the other children) */
    void           MoveChildUp(Wnd* wnd);

    /** places \a wnd at the beginning of the child ptr list, so it is rendered first (below the other children) */
    void           MoveChildDown(Wnd* wnd);

    void           DetachChild(Wnd* wnd);               ///< removes \a wnd from child ptr list, sets child's m_parent = 0
    void           DetachChildren();                    ///< removes all Wnds from child ptr list, sets childrens' m_parent = 0

    /** removes, detaches, and deletes \a wnd; does nothing if wnd is not in the child list */
    void           DeleteChild(Wnd* wnd);

    void           DeleteChildren();                    ///< removes, detaches, and deletes all Wnds in the child list
    void           InstallEventFilter(Wnd* wnd);        ///< adds \a wnd to the front of the event filtering chain
    void           RemoveEventFilter(Wnd* wnd);         ///< removes \a wnd from the filter chain

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

    /** removes the window's layout, handing ownership of all children back to the window, with the sizes and positions
	they had before the layout resized them.  If no layout exists for the window, no action is taken. */
    void           RemoveLayout();

    /** removes the window's layout, including all attached children, and returns it.  If no layout exists for the
	window, no action is taken. */
    Layout*        DetachLayout();

    /** sets the margin that should exist between the outer edges of the windows in the layout and the edge of the
        client area.  If no layout exists for the window, this has no effect. */
    void           SetLayoutBorderMargin(int margin);

    /** sets the margin that should exist between the windows in the layout.  If no layout exists for the window, this
        has no effect. */
    void           SetLayoutCellMargin(int margin);


    virtual void   Render(); ///< draws this Wnd

    /** respond to left button down msg.  A window receives this whenever any input device button changes from up to
        down while over the window. */
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);

    /** respond to drag msg (even if this Wnd is not dragable).  Drag messages are only sent to the window over which
        the button was dressed at the beginning of the drag. A window receives this whenever any input device button is
        down and the mouse is moving while over the window.  If a window has the DRAG_KEEPER flag set, the window will
        also receive drag messages when the mouse is being dragged outside the window's area. */
    virtual void   LDrag(const Pt& pt, const Pt& move, Uint32 keys);

    /** respond to release of left mouse button outside window, if it was originally depressed over window.  A window
        will receive an LButtonUp() message whenever a drag that started over its area ends, even if the cursor is not
        currently over the window when this happens. */
    virtual void   LButtonUp(const Pt& pt, Uint32 keys);

    /** respond to release of left mouse button over window, if it was also originally depressed over window.  A window
        will receive an LButtonUp() message whenever a drag that started over its area ends over its area as well. */
    virtual void   LClick(const Pt& pt, Uint32 keys);

    /** respond to second left click in window within the time limit.  A window will receive an LButtonUp() message
        instead of an LButtonDown() or LClick() message if the left input device button is pressed over a window that
        was l-clicked within a double-click time interval.  Note that this means a double click is always preceded by a
        click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in
        during the interval. */
    virtual void   LDoubleClick(const Pt& pt, Uint32 keys);

    /** respond to right button down msg. \see LButtonDown() */
    virtual void   RButtonDown(const Pt& pt, Uint32 keys);

    /** respond to release of right mouse button over window, if it was also originally depressed over window. \see
        LButtonUp() */
    virtual void   RClick(const Pt& pt, Uint32 keys);

    /** respond to second right click in window within the time limit.  A window will receive an RButtonUp() message
        instead of an RButtonDown() or RClick() message if the right input device button is pressed over a window that
        was r-clicked within a double-click time interval Note that this means a double click is always preceded by a
        click.  For a double click to occur, no other window may have received a *Click() or *ButtonDown() message in
        during the interval. */
    virtual void   RDoubleClick(const Pt& pt, Uint32 keys);

    virtual void   MouseEnter(const Pt& pt, Uint32 keys);  ///< respond to cursor entering window's coords

    /** respond to cursor moving about in window's coords.  A MouseHere() message will not be generated the first time
        the cursor enters the window's area.  In that case, a MouseEnter() message is generated. */
    virtual void   MouseHere(const Pt& pt, Uint32 keys);

    virtual void   MouseLeave(const Pt& pt, Uint32 keys);  ///< respond to cursor leaving window's coords

    /** respond to movement of the mouse wheel (move > 0 indicates the wheel is rolled up, < 0 indicates down) */
    virtual void   MouseWheel(const Pt& pt, int move, Uint32 keys);

    /** respond to keystrokes (focus window only).  A window may receive Keypress() messages passed up to it from its
        children.  For instance, many Control-derived classes pass Keypress() messages to their Parent() windows by
        default. */
    virtual void   Keypress(Key key, Uint32 key_mods);

    virtual void   GainingFocus();                         ///< respond to this window gaining the input focus
    virtual void   LosingFocus();                          ///< respond to this window losing the input focus


    /** this executes a modal window and gives it its modality.  For non-modal windows, this function is a no-op.
        It returns 0 if the window is non-modal, or non-zero after successful modal execution.*/
    virtual int    Run();

    /** sets the time cutoff (in milliseconds) for a browse info mode.  If \a mode is not less than the current number
        of modes, extra modes will be created as needed.  The extra nodes will be set to the value of the last time at
        the time the method is called, or \a time if there were initially no modes. */
    void SetBrowseModeTime(int time, int mode = 0);

    /** sets the Wnd that is used to show browse info about this Wnd in the browse info mode \a mode.  \throw
        std::out_of_range May throw std::out_of_range if \a mode is not a valid browse mode. */
    void SetBrowseInfoWnd(const boost::shared_ptr<BrowseInfoWnd>& wnd, int mode = 0);

    /** sets the browse info window for mode \a mode to a Wnd with the specified color and border color which contains
        the specified text.  \throw std::out_of_range May throw std::out_of_range if \a mode is not a valid browse
        mode. */
    void SetBrowseText(const std::string& text, int mode = 0);

    /** sets the browse modes for the Wnd, including time cutoffs (in milliseconds), the BrowseInfoWnds to be displayed
        for each browse info mode, and the text (if any) to be displayed in each mode.  As the time that the cursor is
        over this Wnd exceeds each mode's time, the corresponding Wnd is shown superimposed over this Wnd and its
        children.  Set the first time cutoff to 0 for immediate browse info display. */
    void SetBrowseModes(const std::vector<BrowseInfoMode>& modes);

    void SetStyleFactory(const boost::shared_ptr<StyleFactory>& factory); ///< sets the currently-installed style factory

    /** provides the attributes of this object that are appropriate for a user to edit in a WndEditor; see WndEditor for
        details. */
    virtual void DefineAttributes(WndEditor* editor);
    //@}


    /** returns the single time to place in the browse modes during Wnd construction. */
    static int DefaultBrowseTime();

    /** sets the single time to place in the browse modes during Wnd construction. */
    static void SetDefaultBrowseTime(int time);

    /** returns the single BrowseInfoWnd to place in the browse modes during Wnd construction.  This returns a
        TextBoxBrowseInfoWnd with a default parameterization. */
    static const boost::shared_ptr<BrowseInfoWnd>& DefaultBrowseInfoWnd();

    /** sets the single BrowseInfoWnd to place in the browse modes during Wnd construction. */
    static void SetDefaultBrowseInfoWnd(const boost::shared_ptr<BrowseInfoWnd>& browse_info_wnd);

    /** \name Exceptions */ //@{
    /** The base class for Wnd exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when a request to perform a layout fails due to child Wnds in illegal starting positions, or when a
        SetLayout() call would result in an illegal state. */
    GG_CONCRETE_EXCEPTION(BadLayout, GG::Wnd, Exception);
    //@}

protected:
    /** encapsulates a Wnd event that is passed from the singleton App to a Wnd.  The various types of Events correspond
        to the various message member functions of Wnd, some of which have different parameterizations.  Rather than
        have a less-efficient but more-easily-extensible hierarchy of Event types, a single Event type exists that has
        all possible parameters to a Wnd message function call.  Therefore, not all of Event's accessors will return
        sensical results, depending on the EventType of the Event.  Note that Wnd events may be filtered before they
        actually reach the target Wnd \see Wnd */
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

        /** constructs an Event that is used to invoke a function taking parameters (const GG::Pt& pt, Uint32 keys), eg
            LButtonDown(). */
        Event(EventType type, const Pt& pt, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (const Pt& pt, const Pt& move,
            Uint32 keys), eg LDrag(). */
        Event(EventType type, const Pt& pt, const Pt& move, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (const Pt& pt, int move, Uint32
            keys), eg MouseWheel(). */
        Event(EventType type, const Pt& pt, int move, Uint32 keys);

        /** constructs an Event that is used to invoke a function taking parameters (Key key, Uint32 key_mods), eg
            Keypress(). */
        Event(EventType type, Key key, Uint32 key_mods);

        /** constructs an Event that is used to invoke a function taking no parameters, eg GainingFocus(). */
        Event(EventType type);

        EventType  Type() const;       ///< returns the type of the Event
        const Pt&  Point() const;      ///< returns the point at which the event took place, if any
        Key        KeyPress() const;   ///< returns the keypress represented by the Event, if any
        Uint32     KeyMods() const;    ///< returns the modifiers to the Event's keypress, if any
        const Pt&  DragMove() const;   ///< returns the amount of drag movement represented by the Event, if any
        int        WheelMove() const;  ///< returns the ammount of mouse wheel movement represented by the Event, if any

    private:
        EventType  m_type;
        Pt         m_point;
        Key        m_keypress;
        Uint32     m_key_mods;
        Pt         m_drag_move;
        int        m_wheel_move;
    };

    /** \name Structors */ //@{
    Wnd(); ///< default ctor

    /** ctor that allows a size and position to be specified, as well as creation flags */
    Wnd(int x, int y, int w, int h, Uint32 flags = CLICKABLE | DRAGABLE);
    //@}

    /** \name Accessors */ //@{
    const std::list<Wnd*>& Children() const; ///< returns child list; the list is const, but the children may be manipulated
    //@}

    /** \name Mutators */ //@{
    /** handles an Event destined for Wnd \a w, but which this Wnd is allowed to handle first.  Returns true if this
        filter processed the message. */
    virtual bool EventFilter(Wnd* w, const Event& event);

    Layout*      GetLayout();        ///< returns the layout for the window, if any
    Layout*      ContainingLayout(); ///< returns the layout containing the window, if any
    //@}

    std::string  m_text;            ///< text associated with the window, such as a window title or button label, etc.
    bool         m_done;            ///< derived modal Wnd's set this to true to stop modal loop

private:
    void ValidateFlags();                 ///< sanity-checks the window creation flags
    void HandleEvent(const Event& event); ///< handles all messages, and calls appropriate function (LButtonDown(), LDrag(), etc.)

    Wnd*              m_parent;        ///< ptr to this window's parent; may be 0
    std::list<Wnd*>   m_children;      ///< list of ptrs to child windows kept in order of decreasing area
    int               m_zorder;        ///< where this window is in the z-order (root (non-child) windows only)
    bool              m_visible;       ///< is this window drawn?
    std::string       m_drag_drop_data_type; ///< the type of drag-drop data this Wnd represents, if any
    bool              m_clip_children; ///< should the children of this window be clipped?
    Pt                m_upperleft;     ///< upper left point of window
    Pt                m_lowerright;    ///< lower right point of window
    Pt                m_min_size;      ///< minimum window size Pt(0, 0) (= none) by default
    Pt                m_max_size;      ///< maximum window size Pt(1 << 30, 1 << 30) (= none) by default

    /** the Wnds that are filtering this Wnd's events. These are in reverse order: top of the stack is back(). */
    std::vector<Wnd*> m_filters;

    std::set<Wnd*>    m_filtering;         ///< the Wnds in whose filter chains this Wnd lies
    Layout*           m_layout;            ///< the layout for this Wnd, if any
    Layout*           m_containing_layout; ///< the layout that contains this Wnd, if any
    std::vector<BrowseInfoMode>
                      m_browse_modes;      ///< the browse info modes for this window

    boost::shared_ptr<StyleFactory>
                      m_style_factory;     ///< the style factory to use when creating dialogs or child controls

    /** flags supplied at window creation for clickability, dragability, drag-keeping, and resizability */
    Uint32            m_flags;

    /** the default time to set for the first (and only) value in m_browse_mode_times during Wnd contruction */
    static int        s_default_browse_time;

    /** the default BrowseInfoWmd to set for the first (and only) value in m_browse_mode_times during Wnd contruction */
    static boost::shared_ptr<BrowseInfoWnd>
                      s_default_browse_info_wnd;

    friend class App;   ///< App needs access to \a m_zorder, m_children, etc.
    friend class ZList; ///< ZList needs access to \a m_zorder in order to order windows

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::Wnd::BrowseInfoMode::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(time)
        & BOOST_SERIALIZATION_NVP(wnd)
        & BOOST_SERIALIZATION_NVP(text);
}

template <class Archive>
void GG::Wnd::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_text)
        & BOOST_SERIALIZATION_NVP(m_done)
        & BOOST_SERIALIZATION_NVP(m_parent)
        & BOOST_SERIALIZATION_NVP(m_children)
        & BOOST_SERIALIZATION_NVP(m_zorder)
        & BOOST_SERIALIZATION_NVP(m_visible)
        & BOOST_SERIALIZATION_NVP(m_drag_drop_data_type)
        & BOOST_SERIALIZATION_NVP(m_clip_children)
        & BOOST_SERIALIZATION_NVP(m_upperleft)
        & BOOST_SERIALIZATION_NVP(m_lowerright)
        & BOOST_SERIALIZATION_NVP(m_min_size)
        & BOOST_SERIALIZATION_NVP(m_max_size)
        & BOOST_SERIALIZATION_NVP(m_filters)
        & BOOST_SERIALIZATION_NVP(m_filtering)
        & BOOST_SERIALIZATION_NVP(m_layout)
        & BOOST_SERIALIZATION_NVP(m_containing_layout)
        & BOOST_SERIALIZATION_NVP(m_browse_modes)
        & BOOST_SERIALIZATION_NVP(m_style_factory)
        & BOOST_SERIALIZATION_NVP(m_flags);

    if (Archive::is_loading::value)
        ValidateFlags();
}

#endif // _GGWnd_h_
