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

#ifndef _GGDropDownList_h_
#define _GGDropDownList_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

#ifndef _GGListBox_h_
#include "GGListBox.h"
#endif

namespace GG {

/** displays a single choice, and allows the user to select items from a pop-up list.  DropDownList is based upon 
    GG::ListBox, but has significant restrictions over the functionality of GG::ListBox.  Specifically, all list items
    must have the same height, list items may not have subrows, and there is no dragging or dropping.  Though any 
    Control-derived object may be placed in 
    an item cell, the items are only interactive in the drop-down list; the currently selected item is displayed only.  
    In the DropDownList constructor, there is no "h" height parameter as there is in the other Wnd-derived class 
    contructors. The "row_ht" parameter takes the place of "h", but it has a slightly different meaning.  A cell-margin 
    and the thickness of the border around the DropDownList are added to "row_ht" to determine the actual height of the 
    control.  All subequent resizing calls will lock the height of the control to this calculated height.  The "drop_ht" 
    parameter determines the vertical size of the drop-down list.  Most of the ListBox interface is duplicated in 
    DropDownList.  Though you can still set the alignment, indentation, etc. of individual rows, as in ListBox, the 
    currently-selected row will have the same alignment, indentation, etc. when displayed in the control in its 
    unopened state.  This may look quite ugly.*/
class GG_API GG::DropDownList : public GG::Control
{
public:
    using Wnd::SizeMove;

    /** This is a single item in a dropdown list. \see See GG::ListBox for details.*/
    typedef ListBox::Row Row;
   
    /** \name Signal Types */ //@{
    typedef boost::signal<void (int)>   SelChangedSignalType;   ///< emitted when a new item is selected; the int parameter will be -1 when no item is selected
    //@}

    /** \name Slot Types */ //@{
    typedef SelChangedSignalType::slot_type   SelChangedSlotType;  ///< type of functor(s) invoked on a SelChangedSignalType
    //@}

    /** \name Structors */ //@{
    /** basic ctor.  DropDownList retains ownership of \a lb, if it is non-null.  Client code should \a not attempt to 
        delete \a lb once it has been passed to the DropDownList ctor. */
    DropDownList(int x, int y, int w, int row_ht, int drop_ht, Clr color, ListBox* lb = 0, Uint32 flags = CLICKABLE);

    /** basic ctor.  DropDownList retains ownership of \a lb, if it is non-null.  Client code should \a not attempt to 
        delete \a lb once it has been passed to the DropDownList ctor. */
    DropDownList(int x, int y, int w, int row_ht, int drop_ht, Clr color, Clr interior, ListBox* lb = 0, Uint32 flags = CLICKABLE);

    DropDownList(const XMLElement& elem); ///< ctor that constructs an DropDownList object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a DropDownList object
    ~DropDownList(); ///< dtor
    //@}
   
    /** \name Accessors */ //@{
    const Row*     CurrentItem() const;                                     ///< returns a pointer to the currently selected list item (returns 0 if none is selected)
    int            CurrentItemIndex() const   {return m_current_item_idx;}  ///< returns the list index of the currently selected list item (returns -1 if none is selected)
   
    bool           Empty() const              {return m_LB->Empty();}       ///< returns true when the list is empty
    const Row&     GetItem(int n) const       {return m_LB->GetRow(n);}     ///< returns a const reference to the row at index \a n; not range-checked
    bool           Selected(int n) const      {return m_LB->Selected(n);}   ///< returns true if row \a n is selected
    Uint32         Style() const              {return m_LB->Style();}       ///< returns the style flags of the list \see GG::ListBoxStyle
    int            RowHeight() const          {return m_LB->RowHeight();}   ///< returns the default row height. \note Unlike a ListBox, every row has the same height.
    int            NumRows() const            {return m_LB->NumRows();}     //< returns the total number of items in the list
    int            NumCols() const            {return m_LB->NumCols();}     ///< returns the total number of columns in each list item
    int            SortCol() const            {return m_LB->SortCol();}     ///< returns the index of the column used to sort items, when sorting is enabled.  \note The sort column is not range checked when it is set by the user; it may be < 0 or >= NumCols().
    int            ColWidth(int n) const      {return m_LB->ColWidth(n);}   ///< returns the width of column \a n in pixels; not range-checked
    ListBoxStyle   ColAlignment(int n) const  {return m_LB->ColAlignment(n);}///< returns the alignment of column \a n; must be LB_LEFT, LB_CENTER, or LB_RIGHT; not range-checked
    ListBoxStyle   RowAlignment(int n) const  {return m_LB->RowAlignment(n);}///< returns the alignment of row \a n; must be LB_TOP, LB_VCENTER, or LB_BOTTOM; not range-checked

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an DropDownList object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object

    SelChangedSignalType& SelChangedSignal() const {return m_sel_changed_sig;}///< returns the selection change signal object for this DropDownList
    //@}
   
    /** \name Mutators */ //@{
    virtual bool   Render();
    virtual void   LClick(const Pt& pt, Uint32 keys);
    virtual void   Keypress(Key key, Uint32 key_mods);

    virtual void   SizeMove(int x1, int y1, int x2, int y2); ///< resizes the control, ensuring the proper height is maintained based on the list's row height

    int            Insert(Row* row, int at = -1);               ///< insertion sorts \a row into the list, or inserts into an unsorted list before index \a at; returns index of insertion point
    void           Delete(int idx);                             ///< removes the row at index \a idx from the list
    void           Clear();                                     ///< empties the list
    void           IndentRow(int n, int i) {m_LB->IndentRow(n, i);}///< sets the indentation of the row at index \a n to \a i; not range-checked
    Row&           GetRow(int n) {return m_LB->GetRow(n);}      ///< returns a reference to the Row at row index \a n; not range-checked
   
    void           Select(int row);                                      ///< selects row-item \a row in the list
    void           SetStyle(Uint32 s);                                   ///< sets the style flags for the list to \a s (invalidates currently selected item). \see GG::ListBoxStyle
    void           SetRowHeight(int h)        {m_LB->SetRowHeight(h);};  ///< sets the row height
    void           SetNumCols(int n)          {m_LB->SetNumCols(n);}     ///< sets the number of columns in each list item to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void           SetSortCol(int n);                                    ///< sets the index of the column used to sort rows when sorting is enabled (invalidates currently selected item); not range-checked
    void           SetColWidth(int n, int w)  {m_LB->SetColWidth(n, w);} ///< sets the width of column \n to \a w; not range-checked
    void           LockColWidths()            {m_LB->LockColWidths();}   ///< fixes the column widths; by default, an empty list will take on the number of columns of its first added row. \note The number of columns and their widths may still be set via SetNumCols() and SetColWidth() after this function has been called.
    void           UnLockColWidths()          {m_LB->UnLockColWidths();} ///< allows the number of columns to be determined by the first row added to an empty ListBox
    void           SetColAlignment(int n, ListBoxStyle align) 
                                              {m_LB->SetColAlignment(n, align);} ///< sets the alignment of column \a n to \a align; not range-checked
    void           SetRowAlignment(int n, ListBoxStyle align) 
                                              {m_LB->SetRowAlignment(n, align);} ///< sets the alignment of the Row at row index \a n to \a align; not range-checked
    //@}

protected:
    /** \name Mutators */ //@{
    ListBox*       LB() {return m_LB;}  ///< returns the ListBox used to render the selected row and the popup list
    //@}

private:
    int            m_current_item_idx;  ///< the index of the currently-selected list item (-1 if none is selected)
    ListBox*       m_LB;                ///< the ListBox used to render the selected row and the popup list
   
    mutable SelChangedSignalType m_sel_changed_sig;
};

} // namespace GG

#endif // _GGDropDownList_h_


