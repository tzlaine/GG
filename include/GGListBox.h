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

/** \file GGListBox.h
    Contains the ListBox class, a control that contains rows of other controls, commonly TextControls. */

#ifndef _GGListBox_h_
#define _GGListBox_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

namespace GG {

class Font;
class Scroll;
class SubTexture;


/** ListBox provides a flexible control that can contain rows and columns of other controls, even other ListBoxes.
    A ListBox consists of rows of controls, usually text or graphics.  Each row represents one item; rows can be 
    added or deleted, but not columns or individual controls (though the individual controls can be removed from
    a row by accessing it directly).  Each Row in a ListBox must have the same number of cells and the same cell 
    widths as all the others.  If you add a row that has fewer cells than the ListBox you are adding it to, it will 
    be padded with 
    empty cells; likewise, if it has too many cells to fit into the Listbox, it will have cells removed.  ListBoxes 
    are designed to be easy to use in common cases, and useful in uncommon cases with only a little work.  Adding 
    a row to an empty ListBox will cause the ListBox to take on the number of columns that the row has cells, 
    and each column will have an equal portion of the ListBox's width.  This allows you to just add rows to a 
    ListBox without worrying about setting up the ListBox in any way ahead of time.  Use LockColWidths() to prevent 
    empty ListBoxes from taking on a new row's number of columns.  To create a ListBox with user-defined widths, 
    use the ctor designed for that, or call SetNumCols(), set individual widths with SetColWidth(), and lock the 
    column widths with LockColWidths().  If non-standard Scrolls are desired, subclasses can create their own 
    Scroll-derived scrolls by overriding NewVScroll() and NewHScroll().    When overriding NewVScroll() and 
    NewHScroll() in subclasses, call RecreateScrolls() at the end of the subclass's constructor, to ensure that 
    the correct type of Scrolls are created; if scrolls are created in the ListBox base constructor, the virtual 
    function table may not be complete yet, so GG::Scrolls may be created instead of the types used in the 
    overloaded New*Scroll() functions. Note that Rows are stored by boost::shared_ptr, so there are three 
    implications: 1) any Row inserted into a ListBox becomes the property of that list box, and should not be 
    deleted; 2) Row pointers received from ListBox signals should never be deleted; and 3) Row pointers 
    received from ListBox signals should never be inserted into another ListBox, nor otherwise used to create a 
    shared_ptr.  If you want to move a Row from one ListBox to another, use GetRowPtr(int) and 
    Insert(const shared_ptr<Row>&, int).  Also note that while a ListBox can contain arbitrary Control-derived 
    controls, in order for such controls to be automatically saved and loaded using XML encoding, any user-defined 
    Control subclasses must be added to the App's XMLObjectFactory.  See GG::App::AddWndGenerator() and 
    GG::XMLObjectFactory for details. */
class GG_API ListBox : public Control
{
public:
    using Wnd::SizeMove;

    /** This is a single item in a listbox.
        A Row is primarily a container for Controls.  Each Cell in a Row is a Control* which points to an 
        object of a subclass of Control.  Each such Control can be connected to arbitrary functionality using signals and 
        slots.  Each Row contains a data type string, which indicates the user's definition
        of what type of data the Row represents (e.g. "Annual Rainfall", "ship_def", etc.).  During dragging and dropping, the 
        data type associated with a Row indicates to potential drop targets what type of data the Row represents; the target 
        may accept or decline the drop based on the data type.  Indentation is also definable for each Row; such indentation
        only affects the first cell in the Row (and not its sub rows).  This is intended to facilitate indented lists
        like tree-views, etc.  A Row may include several subrows; each subrow is considered to be part of its Row.  Note 
        that Rows are stored in ListBoxes by reference, not copy; this means that you can subclass from Row to create your 
        own custom Row types.  This is the recommended method for associating a row with the non-GUI object that it 
        represents.  \note The \a margin member and the \a col_alignment and \a width Cell members are included so that each
        Row has all the necessary information with which to render itself (this is primarily needed to facilitate drag-and-drop);
        these data are duplicates of the margin, alignment, and column widths data found in the owning ListBox, and may be
        overwritten by the ListBox at any time. */
    struct GG_API Row : public Wnd
    {
        /** A cell in a GG::ListBox::Row, consisting of a single Control, a column alignment and a width. */
        struct GG_API Cell
        {
            Cell(); ///< default ctor
            Cell(const XMLElement& elem); ///< XML ctor
            virtual ~Cell();

            XMLElement          XMLEncode() const;    ///< constructs an XMLElement from an Cell object
            XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Cell

            shared_ptr<Control> control;
            ListBoxStyle        col_alignment;
            int                 width;
        };

        /** \name Structors */ //@{
        Row(); ///< default ctor
        Row(const string& data, int ht, ListBoxStyle align = LB_VCENTER, int indent = 0, int margin_ = 2); ///< ctor
        Row(const XMLElement& elem); ///< XML ctor
        virtual ~Row();
        //@}

        /** \name Accessors */ //@{
        int                        Height() const;             ///< returns the height of this row, considering height and/or contents of sub_rows
        virtual const string&      SortKey(int column) const;  ///< returns the string by which this row may be sorted
        size_t                     size() const;               ///< returns the number of Controls in this Row
        const shared_ptr<Control>& operator[](size_t n) const; ///< returns the Control in the \a nth cell of this Row; not range checked
        const shared_ptr<Control>& at(size_t n) const;         ///< returns the Control in the \a nth cell of this Row \throw std::range_error throws when size() <= \a n
        ListBoxStyle               ColAlignment(int n) const;  ///< returns the horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        int                        ColWidth(int n) const;      ///< returns the width of the \a nth cell of this Row; not range checked

        virtual XMLElement          XMLEncode() const;    ///< constructs an XMLElement from a Row object
        virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Row
        //@}

        /** \name Mutators */ //@{
        virtual bool   Render();

        void     push_back(Control* c); ///< adds a given Control to the end of the Row; this Control becomes property of the Row and should not be deleted or added to another Row
        void     push_back(const shared_ptr<Control>&); ///< adds a given Control to the end of the Row; this Control becomes property of the Row and should not be deleted or added to another Row
        void     push_back(const string& str, const shared_ptr<Font>& font, Clr color = CLR_BLACK); ///< overload of push_back that creates a TextControl and adds it to the Row
        void     push_back(const string& str, const string& font_filename, int pts, Clr color = CLR_BLACK); ///< overload of push_back that creates a TextControl and adds it to the Row
        void     push_back(const SubTexture& st); ///< overload of push_back that creates a StaticGraphic Control and adds it to the Row
        void     clear(); ///< removes all cells in this Row
        void     resize(size_t n); ///< resizes the Row to have \a n cells
        void     SetColAlignment(int n, ListBoxStyle align); ///< sets the horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        void     SetColWidth(int n, int width); ///< sets the width of the \a nth cell of this Row; not range checked
        void     SetColAlignments(const vector<ListBoxStyle>& aligns); ///< sets the horizontal alignment of all the Controls in this Row; not range checked
        void     SetColWidths(const vector<int>& widths); ///< sets all the widths of the cells of this Row; not range checked
        //@}

        // these two generate graphics and text controls from basic text or subtextures
        static Control* CreateControl(const string& str, const shared_ptr<Font>& font, Clr color); ///< creates a "shrink-fit" TextControl from text, font, and color parameters
        static Control* CreateControl(const SubTexture& st); ///< creates a "shrink-fit" StaticGraphic Control from a SubTexture parameter

        string       data_type;      ///< for labeling non-text rows, and dragging and dropping; a string value representing the type of data this row is
        int          height;         ///< height of this row, == 0 if undefined; rows already in a ListBox will never have a 0 height
        ListBoxStyle row_alignment;  ///< vertical row alignment, one of LB_TOP, LB_VCENTER, or LB_BOTTOM
        int          indentation;    ///< number of pixels that the \a first cell of the row is shifted to the right (subrows not affected)
        vector<Cell> cells;          ///< the Controls on the top level of this Row; .second is horizontal alignment, one of LB_LEFT, LB_CENTER, or LB_RIGHT
        int          margin;         ///< the amount of space left between each edge of the cell and its contents, in pixels
        vector<shared_ptr<Row> >
                     sub_rows;       ///< for making multi-line rows
    };

    /** thrown by a ListBox that does not wish to accept a received drop, for whatever reason. This may be throw at any 
        time during the receipt of a drop -- even in a function called by a DroppedSignal. */
    class GG_API DontAcceptDropException : public GGException {};

    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>                ClearedSignalType;        ///< emitted when the list box is cleared
    typedef boost::signal<void (const set<int>&)> SelChangedSignalType;     ///< emitted when one or more rows are selected or deselected
    typedef boost::signal<void (int, const shared_ptr<ListBox::Row>&)>
                                                  RowSignalType;            ///< the signature of row-change-notification signals
    typedef boost::signal<void (int, const shared_ptr<ListBox::Row>&, const Pt&)>
                                                  RowClickSignalType;       ///< the signature of row-click-notification signals
    typedef RowSignalType                         InsertedSignalType;       ///< emitted when a row is inserted into the list box; provides the index of the insertion point and the Row inserted
    typedef RowSignalType                         DroppedSignalType;        ///< emitted when a row is inserted into the list box via drag-and-drop; provides the index of the drop point and the Row dropped
    typedef RowClickSignalType                    LeftClickedSignalType;    ///< emitted when a row in the listbox is left-clicked; provides the index of the row left-clicked and the Row contents left-clicked
    typedef RowClickSignalType                    RightClickedSignalType;   ///< emitted when a row in the listbox is right-clicked; provides the index of the row right-clicked and the Row contents right-clicked
    typedef RowSignalType                         DoubleClickedSignalType;  ///< emitted when a row in the listbox is left-double-clicked; provides the index of the row double-clicked and the Row contents double-clicked
    typedef RowSignalType                         DeletedSignalType;        ///< emitted when a row in the listbox is deleted; provides the index of the deletion point
    typedef boost::signal<void (int)>             BrowsedSignalType;        ///< emitted when a row in the listbox is "browsed" (rolled over) by the cursor; provides the index of the browsed row
    //@}

    /** \name Slot Types */ //@{
    typedef ClearedSignalType::slot_type       ClearedSlotType;      ///< type of functor(s) invoked on a ClearedSignalType
    typedef SelChangedSignalType::slot_type    SelChangedSlotType;   ///< type of functor(s) invoked on a SelChangedSignalType
    typedef InsertedSignalType::slot_type      InsertedSlotType;     ///< type of functor(s) invoked on a InsertedSignalType
    typedef LeftClickedSignalType::slot_type   LeftClickedSlotType;  ///< type of functor(s) invoked on a LeftClickedSignalType
    typedef RightClickedSignalType::slot_type  RightClickedSlotType; ///< type of functor(s) invoked on a RightClickedSignalType
    typedef DoubleClickedSignalType::slot_type DoubleClickedSlotType;///< type of functor(s) invoked on a DoubleClickedSignalType
    typedef DeletedSignalType::slot_type       DeletedSlotType;      ///< type of functor(s) invoked on a DeletedSignalType
    typedef DroppedSignalType::slot_type       DroppedSlotType;      ///< type of functor(s) invoked on a DroppedSignalType
    typedef BrowsedSignalType::slot_type       BrowsedSlotType;      ///< type of functor(s) invoked on a BrowsedSignalType
    //@}

    /** \name Structors */ //@{
    /** basic ctor */
    ListBox(int x, int y, int w, int h, Clr color, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER);

    /** ctor that allows the specification of column widths */
    ListBox(int x, int y, int w, int h, Clr color, const vector<int>& col_widths, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER);

    ListBox(const XMLElement& elem); ///< ctor that constructs an ListBox object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ListBox object

    virtual ~ListBox(); ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    virtual Pt      ClientUpperLeft() const;
    virtual Pt      ClientLowerRight() const;

    bool            Empty() const              {return m_rows.empty();}      ///< returns true when the ListBox is empty
    const Row&      GetRow(int n) const        {return *m_rows.at(n);}       ///< returns a const reference to the row at index \a n; not range-checked
    int             Caret() const              {return m_caret;}             ///< returns the index of the row that has the caret
    const set<int>& Selections() const         {return m_selections;}        ///< returns a const reference to the set row indexes that is currently selected
    bool            Selected(int n) const      {return m_selections.find(n) != m_selections.end();} ///< returns true if row \a n is selected
    Clr             InteriorColor() const      {return m_int_color;}         ///< returns the color painted into the client area of the control
    Clr             HiliteColor() const        {return m_hilite_color;}      ///< returns the color behind selected line items

    /** returns the style flags of the listbox \see GG::ListBoxStyle */
    Uint32          Style() const              {return m_style;}

    const Row&      ColHeaders() const         {return *m_header_row;}       ///< returns the row containing the headings for the columns, if any.  If undefined, the returned heading Row will have size() 0.
    int             FirstRowShown() const      {return m_first_row_shown;}   ///< returns the index of the first row visible in the listbox
    int             FirstColShown() const      {return m_first_col_shown;}   ///< returns the index of the first column visible in the listbox

    /** returns the default row height. \note Each row may have its own height, diferent from the one returned by this function. */
    int             RowHeight() const          {return m_row_height;}

    int             NumRows() const            {return int(m_rows.size());}  ///< returns the total number of rows in the ListBox
    int             NumCols() const            {return m_col_widths.size();} ///< returns the total number of columns in the ListBox

    /** returns true iff column widths are fixed \see LockColWidths() */
    bool            KeepColWidths() const      {return m_keep_col_widths;}

    /** returns the index of the column used to sort rows, when sorting is enabled.  \note The sort column is not range checked 
        when it is set by the user; it may be < 0 or >= NumCols(). */
    int             SortCol() const;

    int             ColWidth(int n) const      {return m_col_widths[n];}     ///< returns the width of column \a n in pixels; not range-checked
    ListBoxStyle    ColAlignment(int n) const  {return m_col_alignments[n];} ///< returns the alignment of column \a n; must be LB_LEFT, LB_CENTER, or LB_RIGHT; not range-checked
    ListBoxStyle    RowAlignment(int n) const  {return m_rows[n]->row_alignment;} ///< returns the alignment of row \a n; must be LB_TOP, LB_VCENTER, or LB_BOTTOM; not range-checked

    /** returns the set of data types allowed to be dropped over this ListBox when drag-and-drop is enabled. \note If this set contains 
        "", all drop types are allowed. */
    const set<string>& AllowedDropTypes() const   {return m_allowed_types;}

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an ListBox object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object

    ClearedSignalType&       ClearedSignal() const       {return m_cleared_sig;}        ///< returns the cleared signal object for this ListBox
    SelChangedSignalType&    SelChangedSignal() const    {return m_sel_changed_sig;}    ///< returns the selection change signal object for this ListBox
    InsertedSignalType&      InsertedSignal() const      {return m_inserted_sig;}       ///< returns the inserted signal object for this ListBox
    DroppedSignalType&       DroppedSignal() const       {return m_dropped_sig;}        ///< returns the dropped signal object for this ListBox
    LeftClickedSignalType&   LeftClickedSignal() const   {return m_lclicked_sig;}       ///< returns the left click signal object for this ListBox
    RightClickedSignalType&  RightClickedSignal() const  {return m_rclicked_sig;}       ///< returns the right click signal object for this ListBox
    DoubleClickedSignalType& DoubleClickedSignal() const {return m_double_clicked_sig;} ///< returns the double click signal object for this ListBox
    DeletedSignalType&       DeletedSignal() const       {return m_deleted_sig;}        ///< returns the deleted signal object for this ListBox
    BrowsedSignalType&       BrowsedSignal() const       {return m_browsed_sig;}        ///< returns the browsed signal object for this ListBox
    //@}

    /** \name Mutators */ //@{
    virtual bool   Render();
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);
    virtual void   LButtonUp(const Pt& pt, Uint32 keys);
    virtual void   LClick(const Pt& pt, Uint32 keys);
    virtual void   LDoubleClick(const Pt& pt, Uint32 keys);
    virtual void   RButtonDown(const Pt& pt, Uint32 keys);
    virtual void   RClick(const Pt& pt, Uint32 keys);
    virtual void   Keypress(Key key, Uint32 key_mods);
    virtual void   MouseHere(const Pt& pt, Uint32 keys);
    virtual void   MouseLeave(const Pt& pt, Uint32 keys);
    virtual void   MouseWheel(const Pt& pt, int move, Uint32 keys);

    virtual void   SizeMove(int x1, int y1, int x2, int y2); ///< resizes the control, then resizes the scrollbars as needed

    int            Insert(Row* row, int at = -1);         ///< insertion sorts \a row into the ListBox, or inserts into an unsorted ListBox before index \a at; returns index of insertion point.  This Row becomes the property of the ListBox and should not be deleted or inserted into any other ListBoxes.
    int            Insert(const shared_ptr<Row>& row,
                          int at = -1);                   ///< insertion sorts \a row into the ListBox, or inserts into an unsorted ListBox before index \a at; returns index of insertion point
    void           Delete(int idx);                       ///< removes the row at index \a idx from the ListBox
    void           Clear();                               ///< empties the ListBox
    void           ClearSelection();                      ///< deselects all currently-selected rows
    void           ClearRow(int n);                       ///< deselects row \a n
    void           SelectRow(int n);                      ///< selects row \a n
    void           IndentRow(int n, int i);               ///< sets the indentation of the row at index \a n to \a i; not range-checked
    Row&           GetRow(int n) {return *m_rows.at(n);}  ///< returns a reference to the Row at row index \a n; not range-checked
    const boost::shared_ptr<Row>&
                   GetRowPtr(int n) {return m_rows.at(n);} ///< returns the actual boost::shared_ptr that holds the Row, so that Rows can be moved between ListBoxes by the user

    void           SetSelections(const set<int>& s) {m_selections = s;}     ///< sets the set of selected rows to \a s
    void           SetCaret(int idx)   {m_rows.at(idx); m_caret = idx;}     ///< sets the position of the caret to \a idx

    void           SetInteriorColor(Clr c)      {m_int_color = c;}          ///< sets the color painted into the client area of the control
    void           SetHiliteColor(Clr c)        {m_hilite_color = c;}       ///< sets the color behind selected line items

    /** sets the style flags for the ListBox to \a s. \see GG::ListBoxStyle */
    void           SetStyle(Uint32 s);

    void           SetColHeaders(Row* r);                                   ///< sets the row used as headings for the columns; this Row becomes property of the ListBox and should not be deleted or inserted into any other ListBoxes
    void           SetColHeaders(const shared_ptr<Row>& r);                 ///< sets the row used as headings for the columns
    void           RemoveColHeaders()         {m_header_row->clear();}      ///< removes any columns headings set

    /** sets the default row height \note Each row may have its own height, diferent from the one set by this function. */
    void           SetRowHeight(int h)        {m_row_height = h;}

    void           SetNumCols(int n);                                       ///< sets the number of columns in the ListBox to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void           SetSortCol(int n);                                       ///< sets the index of the column used to sort rows when sorting is enabled; not range-checked
    void           SetColWidth(int n, int w)  {m_col_widths[n] = w;}        ///< sets the width of column \n to \a w; not range-checked

    /** fixes the column widths; by default, an empty ListBox will take on the number of columns of its first added row. \note The 
        number of columns and their widths may still be set via SetNumCols() and SetColWidth() after this function has been called. */
    void           LockColWidths()            {m_keep_col_widths = true;}

    void           UnLockColWidths()          {m_keep_col_widths = false;}  ///< allows the number of columns to be determined by the first row added to an empty ListBox
    void           SetColAlignment(int n, ListBoxStyle align);              ///< sets the alignment of column \a n to \a align; not range-checked
    void           SetRowAlignment(int n, ListBoxStyle align);              ///< sets the alignment of the Row at row index \a n to \a align; not range-checked

    /** allows Rows with data type \a str to be dropped over this ListBox when drag-and-drop is enabled. \note Passing "" enables all 
        drop types. */
    void           AllowDropType(const string& str)     {m_allowed_types.insert(str);}

    /** disallows Rows with data type \a str to be dropped over this ListBox when drag-and-drop is enabled. \note If "" is still an 
        allowed drop type, drops of type \a str will still be allowed, even after disallowed with a call to this function. */
    void           DisallowDropType(const string& str)  {m_allowed_types.erase(str);}
    //@}

    static const int BORDER_THICK; ///< the thickness with which to render the border of the control

protected:
    /** \name Accessors */ //@{
    int            RightMargin() const;     ///< space skipped at right of client area for vertical scroll bar
    int            BottomMargin() const;    ///< space skipped at bottom of client area for horizontal scroll bar
    int            CellMargin() const {return m_cell_margin;} ///< the number of pixels left between the contents of each cell and the cell boundary
    bool           AcceptsDropType(const string& str) const;  ///< called by another listbox when one of its line items is dropped over this listbox

    int            LastVisibleRow() const;  ///< last row that could be drawn, taking into account the contents and the size of client area
    int            LastVisibleCol() const;  ///< last column that could be drawn, taking into account the contents and the size of client area
    int            RowUnderPt(const Pt& pt) const; ///< returns row under pt, if any; value must be checked (it may be < 0 or >= NumRows())
    Pt             DragOffset(const Pt& pt) const; ///< returns offset of \a pt into the row \a pt falls in, or (-1,-1) if pt falls under no row

    int            OldSelRow() const        {return m_old_sel_row;}   ///< returns the last row that was selected with a left-button mouse-down
    int            OldRDownRow() const      {return m_old_rdown_row;} ///< returns the last row that was selected with a right-button mouse-down
    int            LClickRow() const        {return m_lclick_row;}    ///< returns the last row that was left-clicked
    int            RClickRow() const        {return m_rclick_row;}    ///< returns the last row that was right-clicked
    //@}

    /** \name Mutators */ //@{
    int             Insert(const shared_ptr<Row>& row, int at, bool dropped); ///< insertion sorts into list, or inserts into an unsorted list before index "at"; returns index of insertion point
    void            BringCaretIntoView();           ///< makes sure caret is visible when scrolling occurs due to keystrokes etc.
    virtual Scroll* NewVScroll(bool horz_scroll);   ///< creates and returns a new vertical scroll, allowing subclasses to use Scroll-derived scrolls
    virtual Scroll* NewHScroll(bool vert_scroll);   ///< creates and returns a new horizontal scroll, allowing subclasses to use Scroll-derived scrolls
    void            RecreateScrolls();              ///< recreates the vertical and horizontal scrolls as needed.  Subclasses that override NewVScroll or NewHScroll should call this at the end of their ctor.
    //@}

    static void    RenderRow(const Row* row, int left, int top, int first_col, int last_col);       ///< renders a single row of data at (left, top)

private:
    void           ValidateStyle(); ///< reconciles inconsistencies in the style flags
    void           AdjustScrolls(); ///< creates, destroys, or resizes scrolls to reflect size of data in listbox
    void           VScrolled(int tab_low, int tab_high, int low, int high); ///< signals from the vertical scroll bar are caught here
    void           HScrolled(int tab_low, int tab_high, int low, int high); ///< signals from the horizontal scroll bar are caught here
    int            ClickAtRow(int row, Uint32 keys); ///< handles to a mouse-click or spacebar-click on \a row, modified by \a keys
    void           NormalizeRow(const shared_ptr<Row>& row); ///< adjusts a Row so that it has the same number of cells as other rows, and each subrow's height is defined
    void           AttachRowChildren(const shared_ptr<Row>& row); ///< adds all the Controls in \a row (and its subrows) to the child list of the ListBox
    void           DetachRowChildren(const shared_ptr<Row>& row); ///< removes all the Controls in \a row (and its subrows) from the child list of the ListBox

    static void    RenderSubRow(const Row* subrow, int left, int top, int first_col, int last_col); ///< renders a subrow of data

    Scroll*        m_vscroll;          ///< vertical scroll bar on right
    Scroll*        m_hscroll;          ///< horizontal scroll bar at bottom
    int            m_caret;            ///< the item currently selected, or the last item selected by the user 
    set<int>       m_selections;       ///< vector of indexes of selected items
    int            m_old_sel_row;      ///< used to make sure clicks end on the same row where they started
    bool           m_old_sel_row_selected; ///< set to true if m_old_sel_row was selected at the point at which it was designated
    int            m_old_rdown_row;    ///< the row that most recently recieved a right button down message
    int            m_lclick_row;       ///< the row most recently left-clicked
    int            m_rclick_row;       ///< the row most recently right-clicked
    Pt             m_row_drag_offset;  ///< offset used to draw above the drag rect in correct position relative to cursor
    int            m_last_row_browsed; ///< the last row sent out as having been browsed (used to prevent duplicate browse signals)
    bool           m_suppress_delete_signal; ///< needed to use delete internally-only when a drop is refused

    vector<shared_ptr<Row> >
                   m_rows;             ///< line item data

    int            m_first_row_shown;  ///< index of row at top of visible area (always 0 for LB_NOSCROLL)
    int            m_first_col_shown;  ///< like above, but index of column at left
    vector<int>    m_col_widths;       ///< the width of each of the columns goes here
    vector<ListBoxStyle> 
                   m_col_alignments;   ///< the horizontal alignment of each of the columns goes here
    int            m_cell_margin;      ///< the amount of space left between each edge of the cell and its contents, in pixels

    Clr            m_int_color;        ///< color painted into the client area of the control
    Clr            m_hilite_color;     ///< color behind selected line items
    Uint32         m_style;            ///< composed of ListBoxStyles enums (see GUIBase.h)

    shared_ptr<Row>m_header_row;       ///< row of header text/graphics
    int            m_row_height;       ///< default row height
    bool           m_keep_col_widths;  ///< should we keep the column widths, once set?
    bool           m_clip_cells;       ///< if true, the contents of each cell will be clipped to the visible area of that cell (TODO: currently unused)
    int            m_sort_col;         ///< the index of the column data used to sort the list
    set<string>    m_allowed_types;    ///< the line item types allowed for use in this listbox
   
    mutable ClearedSignalType       m_cleared_sig;
    mutable SelChangedSignalType    m_sel_changed_sig;
    mutable InsertedSignalType      m_inserted_sig;
    mutable DroppedSignalType       m_dropped_sig;
    mutable LeftClickedSignalType   m_lclicked_sig;
    mutable RightClickedSignalType  m_rclicked_sig;
    mutable DoubleClickedSignalType m_double_clicked_sig;
    mutable DeletedSignalType       m_deleted_sig;
    mutable BrowsedSignalType       m_browsed_sig;
   
    friend class DropDownList; ///< allow complete access to DropDownList, which relies on ListBox to do its rendering
};

} // namespace GG

#endif // _GGListBox_h_

