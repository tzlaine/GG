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

#ifndef _GGMenu_h_
#define _GGMenu_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

namespace GG {

class Font;
class TextControl;

/** serves as a single menu entry in a GG::MenuBar or GG::PopupMenu; may include a submenu.  All legal item_IDs are positive
    (and so non-zero); any item_ID <= 0 is considered invlaid.  Each MenuItem has a signal that is emmitted with its 
    menu_ID member whenever it is selected. Such signals may be emitted even when the menu_ID is 0.  These signals allow
    each MenuItem to be attached directly to code that should be executed when that item is selected. */
struct MenuItem
{
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> SelectedSignalType; ///< invokes the appropriate functor to handle the menu selection, and passes the ID assigned to the item
    //@}
   
    /** \name Slot Types */ //@{
    typedef SelectedSignalType::slot_type  SelectedSlotType; ///< type of functor(s) invoked on a SelectedSignalType
    //@}
      
    /** \name Structors */ //@{
    MenuItem() : item_ID(0), disabled(false), checked(false), selected_signal(new SelectedSignalType()) {} ///< ctor
    MenuItem(const string& str, int id, bool disable, bool check) : label(str), item_ID(id), disabled(disable), checked(check), selected_signal(new SelectedSignalType()) {} ///< ctor
    MenuItem(const XMLElement& elem); ///< ctor that constructs an MenuItem object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a MenuItem object
    //@}
   
    /** \name Accessors */ //@{
    XMLElement XMLEncode() const; ///< constructs an XMLElement from an MenuItem object
    //@}

    /** \name Mutators */ //@{
    SelectedSignalType& SelectedSignal() {return *selected_signal;} ///< returns the selected signal object for this MenuItem
    //@}

    string           label;      ///< text shown for this menu item
    int              item_ID;    ///< ID number associated with this menu item
    bool             disabled;   ///< set to true when this menu item is disabled
    bool             checked;    ///< set to true when this menu item can be toggled, and is currently on
    vector<MenuItem> next_level; ///< submenu off of this menu item; may be emtpy
   
    shared_ptr<SelectedSignalType>  selected_signal; ///< this signal is emitted when this item is selected
};



/** a menu bar control providing "browse" updates to user navigation of the menu.  Whenever a menu item is selected, a signal
    is emitted which includes the ID of the selected item.  It is recommended that the user attach each menu item to an 
    appropriate function the will execute the actions associated with the menu item, rather than attaching all the items to
    a single slot which uses the int ID parameter to deduce the appropriate action.  The int ID parameter is best used when
    there are several menu items that should execute the same code with different parameters.  For instance, if a submenu
    contains a list of recently used files, each item that contains a filename might be attached to a Reopen(int) function, 
    and the int can be used to determine which file from the list should be opened. If some action is to be taken as the user 
    browses the menu items, such as displaying some visual cue to indicate the result of chosing a particular menu entry, 
    you can attach a slot function to the BrowsedSignalType object returned by BrowsedSignal().  Whenever the mouse moves 
    to a new menu item, this signal is emitted with the ID number of the item under the cursor.  */
class MenuBar : public Control
{
public:
    using Wnd::SizeMove;

    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> BrowsedSignalType; ///< emits the ID of an item in the menu when the cursor moves over it
    //@}
   
    /** \name Slot Types */ //@{
    typedef BrowsedSignalType::slot_type  BrowsedSlotType;   ///< type of functor(s) invoked on a BrowsedSignalType
    //@}
   
    /** \name Structors */ //@{
    /** ctor.  Parameter \a m should contain the desired menu in its next_level member. */
    MenuBar(int x, int y, int w, const shared_ptr<Font>& font, Clr text_color = GG::CLR_WHITE, Clr color = GG::CLR_BLACK, Clr interior = GG::CLR_SHADOW); ///< ctor
    MenuBar(int x, int y, int w, const string& font_filename, int pts, Clr text_color = GG::CLR_WHITE, Clr color = GG::CLR_BLACK, Clr interior = GG::CLR_SHADOW); ///< ctor
    MenuBar(int x, int y, int w, const shared_ptr<Font>& font, const MenuItem& m, Clr text_color = GG::CLR_WHITE, Clr color = GG::CLR_BLACK, Clr interior = GG::CLR_SHADOW); ///< ctor that takes a MenuItem containing the contents of the menus in the MenuBar
    MenuBar(int x, int y, int w, const string& font_filename, int pts, const MenuItem& m, Clr text_color = GG::CLR_WHITE, Clr color = GG::CLR_BLACK, Clr interior = GG::CLR_SHADOW); ///< ctor that takes a MenuItem containing the contents of the menus in the MenuBar
    MenuBar(const XMLElement& elem); ///< ctor that constructs an MenuBar object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a MenuBar object
    //@}

    /** \name Accessors */ //@{
    const MenuItem&   AllMenus() const {return m_menu_data;}                   ///< returns a const reference to the MenuItem that contains all the menus and their contents
    bool              ContainsMenu(const string& str) const;                   ///< returns true if there is a top-level menu in the MenuBar whose label is \a str
    int               NumMenus() const {return m_menu_data.next_level.size();} ///< returns the number of top-level menus in the MenuBar
    const MenuItem&   GetMenu(const string& str) const;                        ///< returns a const reference to the top-level menu in the MenuBar whose label is \a str.  \note No check is made to ensure such a menu exists.
    const MenuItem&   GetMenu(int n) const;                                    ///< returns a const reference to the \a nth menu in the MenuBar; not range-checked
   
    Clr               BorderColor() const        {return m_border_color;}   ///< returns the color used to render the border of the control
    Clr               InteriorColor() const      {return m_int_color;}      ///< returns the color used to render the interior of the control
    Clr               TextColor() const          {return m_text_color;}     ///< returns the color used to render menu item text
    Clr               HiliteColor() const        {return m_hilite_color;}   ///< returns the color used to indicate a hilited menu item
    Clr               SelectedTextColor() const  {return m_sel_text_color;} ///< returns the color used to render a hilited menu item's text
   
    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an MenuBar object
    //@}
   
    /** \name Mutators */ //@{
    virtual int    Render();
   
    virtual int    LButtonDown(const Pt& pt, Uint32 keys);
    virtual int    MouseHere(const Pt& pt, Uint32 keys);
    virtual int    MouseLeave(const Pt& pt, Uint32 keys) {m_caret = -1; return 1;}

    virtual void   SizeMove(int x1, int y1, int x2, int y2);

    MenuItem&      AllMenus() {return m_menu_data;}                   ///< returns a reference to the MenuItem that contains all the menus and their contents
    MenuItem&      GetMenu(const string& str);                        ///< returns a reference to the top-level menu in the MenuBar whose label is \a str.  \note No check is made to ensure such a menu exists.
    MenuItem&      GetMenu(int n) {return m_menu_data.next_level[n];} ///< returns a reference to the \a nth menu in the MenuBar; not range-checked
    void           AddMenu(const MenuItem& menu);                     ///< adds \a menu to the end of the top level of menus
   
    void           SetBorderColor(Clr clr)       {m_border_color = clr;}    ///< sets the color used to render the border of the control
    void           SetInteriorColor(Clr clr)     {m_int_color = clr;}       ///< sets the color used to render the interior of the control
    void           SetTextColor(Clr clr)         {m_text_color = clr;}      ///< sets the color used to render menu item text
    void           SetHiliteColor(Clr clr)       {m_hilite_color = clr;}    ///< sets the color used to indicate a hilited menu item
    void           SetSelectedTextColor(Clr clr) {m_sel_text_color = clr;}  ///< sets the color used to render a hilited menu item's text
   
    BrowsedSignalType& BrowsedSignal()           {return m_browsed_signal;} ///< returns the browsed signal object for this PopupMenu
    //@}

protected:
    /** \name Accessors */ //@{
    const shared_ptr<Font>&     GetFont() const     {return m_font;}        ///< returns the font used to render text in the control
    const vector<TextControl*>& MenuLabels() const  {return m_menu_labels;} ///< returns the text for each top-level menu item
    int                         Caret() const       {return m_caret;}       ///< returns the current position of the caret
    //@}

private:
    void AdjustLayout(); ///< determines the rects in m_menu_lables, and puts the menus in multiple rows if they will not fit in one
    void BrowsedSlot(int n) {m_browsed_signal(n);} ///< responds to a browse in a PopupMenu submenu, and passes it along

    shared_ptr<Font>  m_font;           ///< the font used to render the text in the control
    Clr               m_border_color;   ///< the color of the menu's border
    Clr               m_int_color;      ///< color painted into the client area of the control
    Clr               m_text_color;     ///< color used to paint text in control
    Clr               m_hilite_color;   ///< color behind selected items
    Clr               m_sel_text_color; ///< color of selected text
   
    MenuItem             m_menu_data;   ///< this is not just a single menu item; the next_level element represents the entire menu
    vector<TextControl*> m_menu_labels; ///< the text for each top-level menu item
    int                  m_caret;       ///< the currently indicated top-level menu (open or under the cursor)
   
    BrowsedSignalType m_browsed_signal;
};



/** this is a modal pop-up menu.  PopupMenu gives calling code the abiltiy to create a pop-up menu (usually in response to a 
    right mouse click), allow the pop-up to execute, and then obtain an integer ID representing the selected menu item, 
    by calling MenuID().  If no menu item has been selected,  MenuID() returns 0.  Though every MenuItem in a PopupMenu may
    be attached to a slot directly, it is not recommended.  The intent of this class is to act as a tool to get immediate 
    input from the user, inline.  However, attaching MenuItem signals directly to slots will work, and it will certainly
    be useful in some cases to do this.  Note also that there is no way to serialize a PopupMenu as an XML object.  This is
    also because of the intent to use PopupMenus in an immediate, short-lived manner.  If you wish to save an often-used 
    popup menu, simply create the MenuItem that the popup is based on, and save and load that. If some action is to be taken 
    as the user browses the menu items, such as displaying some visual cue to indicate the result of chosing a particular 
    menu entry, you can attach a slot function to the BrowsedSignalType object returned by BrowsedSignal().  Whenever the 
    mouse moves to a new menu item, this signal is emitted with the ID number of the item under the cursor. */
class PopupMenu : public Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)> BrowsedSignalType; ///< emits the ID of an item in the menu when the cursor moves over it
    //@}
   
    /** \name Slot Types */ //@{
    typedef BrowsedSignalType::slot_type  BrowsedSlotType;   ///< type of functor(s) invoked on a BrowsedSignalType
    //@}
   
    /** \name Structors */ //@{
    /** ctor.  Parameter \a m should contain the desired menu in its next_level member. */
    PopupMenu(int x, int y, const shared_ptr<Font>& font, const MenuItem& m, Clr text_color = GG::CLR_WHITE, Clr color = GG::CLR_BLACK, Clr interior = GG::CLR_SHADOW);
    //@}

    /** \name Accessors */ //@{
    virtual Pt  ClientUpperLeft() const    {return m_origin;}
   
    int         MenuID() const             {return (m_item_selected ? m_item_selected->item_ID : 0);} ///< returns the integer ID of the menu item selected by the user, or 0 if none was selected
    Clr         BorderColor() const        {return m_border_color;}   ///< returns the color used to render the border of the control
    Clr         InteriorColor() const      {return m_int_color;}      ///< returns the color used to render the interior of the control
    Clr         TextColor() const          {return m_text_color;}     ///< returns the color used to render menu item text
    Clr         HiliteColor() const        {return m_hilite_color;}   ///< returns the color used to indicate a hilited menu item
    Clr         SelectedTextColor() const  {return m_sel_text_color;} ///< returns the color used to render a hilited menu item's text
    //@}
   
    /** \name Mutators */ //@{
    virtual int    Render();
   
    virtual int    LButtonUp(const Pt& pt, Uint32 keys);
    virtual int    LClick(const Pt& pt, Uint32 keys)          {return LButtonUp(pt, keys);}
    virtual int    LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual int    RButtonUp(const Pt& pt, Uint32 keys)       {return LButtonUp(pt, keys);}
    virtual int    RClick(const Pt& pt, Uint32 keys)          {return LButtonUp(pt, keys);}
    virtual int    MouseHere(const Pt& pt, Uint32 keys)       {return LDrag(pt, Pt(), keys);}
   
    virtual int    Run();

    void           SetBorderColor(Clr clr)       {m_border_color = clr;}    ///< sets the color used to render the border of the control
    void           SetInteriorColor(Clr clr)     {m_int_color = clr;}       ///< sets the color used to render the interior of the control
    void           SetTextColor(Clr clr)         {m_text_color = clr;}      ///< sets the color used to render menu item text
    void           SetHiliteColor(Clr clr)       {m_hilite_color = clr;}    ///< sets the color used to indicate a hilited menu item
    void           SetSelectedTextColor(Clr clr) {m_sel_text_color = clr;}  ///< sets the color used to render a hilited menu item's text
   
    BrowsedSignalType& BrowsedSignal()           {return m_browsed_signal;} ///< returns the browsed signal object for this PopupMenu
    //@}

protected:
    /** \name Accessors */ //@{
    const shared_ptr<Font>& GetFont() const     {return m_font;}        ///< returns the font used to render text in the control
    const MenuItem&         MenuData() const    {return m_menu_data;}   ///< returns a const reference to the MenuItem that contains all the menu contents
    const vector<Rect>&     OpenLevels() const  {return m_open_levels;} ///< returns the bounding rectangles for each open submenu, used to detect clicks in them
    const vector<int>&      Caret() const       {return m_caret;}       ///< returns the stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push
    const MenuItem*         ItemSelected() const {return m_item_selected;} ///< returns the menu item selected (0 if none)
    //@}

private:
    shared_ptr<Font>  m_font;           ///< the font used to render the text in the control
    Clr               m_border_color;   ///< the color of the menu's border
    Clr               m_int_color;      ///< color painted into the client area of the control
    Clr               m_text_color;     ///< color used to paint text in control
    Clr               m_hilite_color;   ///< color behind selected items
    Clr               m_sel_text_color; ///< color of selected text
   
    MenuItem          m_menu_data;   ///< this is not just a single menu item; the next_level element represents the entire menu
   
    vector<Rect>      m_open_levels; ///< bounding rectangles for each open submenu, used to detect clicks in them
    vector<int>       m_caret;       ///< stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push
   
    const Pt          m_origin;         ///< the upper left hand corner of the control's visible area
    MenuItem*         m_item_selected;  ///< the menu item selected (0 if none)
   
    BrowsedSignalType m_browsed_signal;
};

} // namespace GG

#endif // _GGMenu_h_

