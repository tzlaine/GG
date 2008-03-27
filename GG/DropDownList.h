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

/** \file DropDownList.h
    Contains the DropDownList class, a control that displays a current selection, and allows the user to select one 
    of several options from a list that drops down when the control is clicked. */

#ifndef _GG_DropDownList_h_
#define _GG_DropDownList_h_

#include <GG/ListBox.h>


namespace GG {

/** displays a single choice, and allows the user to select items from a pop-up list.  DropDownList is based upon
    GG::ListBox, but has significant restrictions over the functionality of GG::ListBox.  Specifically, all list items
    must have the same height, list items may not have subrows, and there is no dragging or dropping.  Though any
    Control-derived object may be placed in an item cell, the items are only interactive in the drop-down list; the
    currently selected item is displayed only.  In the DropDownList constructor, there is no "h" height parameter as
    there is in the other Wnd-derived class contructors. The "row_ht" parameter takes the place of "h", but it has a
    slightly different meaning.  A cell-margin and the thickness of the border around the DropDownList are added to
    "row_ht" to determine the actual height of the control.  All subequent resizing calls will lock the height of the
    control to this calculated height.  The "drop_ht" parameter determines the vertical size of the drop-down list.
    Most of the ListBox interface is duplicated in DropDownList.  Though you can still set the alignment, etc. of
    individual rows, as in ListBox, the currently-selected row will have the same alignment, etc. when displayed in the
    control in its unopened state.  This may look quite ugly. */
class GG_API DropDownList : public Control
{
public:
    /** This is a single item in a dropdown list. \see See GG::ListBox for details.*/
    typedef ListBox::Row Row;

    /** \name Signal Types */ ///@{
    typedef boost::signal<void (int)>   SelChangedSignalType;   ///< emitted when a new item is selected; the int parameter will be -1 when no item is selected
    //@}

    /** \name Slot Types */ ///@{
    typedef SelChangedSignalType::slot_type   SelChangedSlotType;  ///< type of functor(s) invoked on a SelChangedSignalType
    //@}

    /** \name Structors */ ///@{
    /** basic ctor.  DropDownList retains ownership of \a lb, if it is non-null. */
    DropDownList(int x, int y, int w, int h, int drop_ht, Clr color, Flags<WndFlag> flags = CLICKABLE);

    ~DropDownList(); ///< dtor
    //@}

    /** \name Accessors */ ///@{
    const Row*     CurrentItem() const;      ///< returns a pointer to the currently selected list item (returns 0 if none is selected)
    int            CurrentItemIndex() const; ///< returns the list index of the currently selected list item (returns -1 if none is selected)

    bool           Empty() const;            ///< returns true when the list is empty
    const Row&     GetItem(int n) const;     ///< returns a const reference to the row at index \a n; not range-checked
    bool           Selected(int n) const;    ///< returns true if row \a n is selected
    Clr            InteriorColor() const;    ///< returns the color painted into the client area of the control

    int            DropHeight() const; ///< returns the height of the drop-down list

    /** returns the style flags of the list \see GG::ListBoxStyle */
    Flags<ListBoxStyle> Style() const;

    int            NumRows() const;          ///< returns the total number of items in the list
    int            NumCols() const;          ///< returns the total number of columns in each list item

    /** returns the index of the column used to sort items, when sorting is enabled.  \note The sort column is not range checked when 
        it is set by the user; it may be < 0 or >= NumCols(). */
    int            SortCol() const;

    int            ColWidth(int n) const;     ///< returns the width of column \a n in pixels; not range-checked
    Alignment      ColAlignment(int n) const; ///< returns the alignment of column \a n; must be LIST_LEFT, LIST_CENTER, or LIST_RIGHT; not range-checked
    Alignment      RowAlignment(int n) const; ///< returns the alignment of row \a n; must be LIST_TOP, LIST_VCENTER, or LIST_BOTTOM; not range-checked

    virtual Pt     ClientUpperLeft() const;
    virtual Pt     ClientLowerRight() const;

    mutable SelChangedSignalType SelChangedSignal; ///< the selection change signal object for this DropDownList
    //@}

    /** \name Mutators */ ///@{
    virtual void   Render();
    virtual void   LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   KeyPress(Key key, Flags<ModKey> mod_keys);

    virtual void   SizeMove(const Pt& ul, const Pt& lr); ///< resizes the control, ensuring the proper height is maintained based on the list's row height

    virtual void   SetColor(Clr c);

    int            Insert(Row* row, int at = -1); ///< insertion sorts \a row into the list, or inserts into an unsorted list before index \a at; returns index of insertion point.  This Row becomes the property of the DropDownList and should not be deleted or inserted into any other DropDownLists
    Row*           Erase(int idx);                ///< removes and returns the row at index \a idx from the list, or 0 if no such row exists
    void           Clear();                       ///< empties the list
    Row&           GetRow(int n);                 ///< returns a reference to the Row at row index \a n; not range-checked

    void           Select(int row);               ///< selects row-item \a row in the list

    void           SetInteriorColor(Clr c);       ///< sets the color painted into the client area of the control
    void           SetDropHeight(int h);          ///< sets the height of the drop-down list

    /** sets the style flags for the list to \a s (invalidates currently selected item). \see GG::ListBoxStyle */
    void           SetStyle(Flags<ListBoxStyle> s);

    void           SetNumCols(int n);         ///< sets the number of columns in each list item to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void           SetSortCol(int n);         ///< sets the index of the column used to sort rows when sorting is enabled (invalidates currently selected item); not range-checked
    void           SetColWidth(int n, int w); ///< sets the width of column \n to \a w; not range-checked

    /** fixes the column widths; by default, an empty list will take on the number of columns of its first added row. \note The number 
        of columns and their widths may still be set via SetNumCols() and SetColWidth() after this function has been called. */
    void           LockColWidths();

    /** allows the number of columns to be determined by the first row added to an empty ListBox */
    void           UnLockColWidths();

    void           SetColAlignment(int n, Alignment align); ///< sets the alignment of column \a n to \a align; not range-checked
    void           SetRowAlignment(int n, Alignment align); ///< sets the alignment of the Row at row index \a n to \a align; not range-checked

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ ///@{
    DropDownList(); ///< default ctor
    //@}

    /** \name Mutators */ ///@{
    ListBox*       LB();                ///< returns the ListBox used to render the selected row and the popup list
    //@}

private:
    int            m_current_item_idx;  ///< the index of the currently-selected list item (-1 if none is selected)
    ListBox*       m_LB;                ///< the ListBox used to render the selected row and the popup list

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::DropDownList::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_current_item_idx)
        & BOOST_SERIALIZATION_NVP(m_LB);
}

#endif // _GG_DropDownList_h_
