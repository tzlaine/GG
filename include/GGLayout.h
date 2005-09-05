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

/** \file GGLayout.h
    Contains the Layout class, which is used to size and align GG windows. */

#ifndef _GGLayout_h_
#define _GGLayout_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

namespace GG {

/** an invisible Wnd subclass whose only purpose is to arrange its child Wnds.  A Layout consists of a grid of cells.  A
    cell may have at most one Wnd covering it, but need not contain a Wnd at all.  A Wnd may cover any rectangular
    region of cells, though they will commonly only cover one.  The cells are arranged into rows and columns.  Most
    attributes of the layout are set for an entire row or column, but alignment is set for each child in the layout
    separately.  The row/column attributes are stretch and minimum row width/column height (hereafter refered to as
    min).  Stretch indicates a propotional factor by which each row/column is stretched when the layout is resized.  For
    example, if the sum of the row stretch factors is 5, a row with a stretch of 2 will gain 2/5 of the increased space
    if the layout grows vertically, or lose 2/5 of the decreased space if the layout shrinks vertically.  Note that this
    means that rows with a stretch of 0 will not change size at all.  The exception to this is when all rows have a
    stretch of 0, in which case all the rows grow and shrink evenly.  Obviously, this applies to columns as well.  The
    min sets a lower bound on the height of a row or the width of a column.  By default, no alignment value is set for a
    child in the layout.  If one is set, the child is not grown and shrunk when the layout is resized, if possible.
    Aligned children just sit in place in the center or on the side they are aligned to.  If the layout becomes too
    small, aligned windows will be shrunk as necessary and if possible.  Note that the MinSize() and MaxSize() of a
    child will affect how much it can be stretched when the layout is resized.

    <p>Layouts are best used to arrange the children of another window, such as arranging the controls of a dialog box.
    When used this way, the Layout becomes the sole child of its parent, and contains the parent's children as its own.
    This scheme allows Layouts to be easily nested, since all Layouts are Wnd-derived.  Like a Control, a Layout will
    forward all MouseWheel() and Keypress() calls to its parent.  Clicks fall through as well, since Layouts are not
    constructed with the Wnd::CLICKABLE flag.

    <p>There are two attributes that affect the spacing of all the layout's child windows: border margin and cell
    margin.  Border margin is the space left around the entire layout, between the outer edges of the children in the
    layout and the layout's outer edges.  Cell margin is the space left between individual Wnds in the layout, but does
    not add to the margin around the outside of the layout.

    <p>A note about how layout minimum sizes are determined: <br>The border margin adds to the minimum size of the
    layout.  Further, the cell margin will have an effect on the minimum size of a cell, even an empty one, if it is \a
    greater than the row or column minimum for that cell.  So an empty layout with 5 columns, a border margin of 3, and
    a cell margin of 2 will have a minumum width of 14.  This is determined as follows: 5 columns means 4 cell margins
    between the columns, so 4 * 2 = 8.  The border margin is added to both sides, which means the total minimum width is
    8 + 2 * 3 = 14.  Also, the minimum size of each child in the layout will affect the minimum sizes of the rows and
    columns it covers.  If a child covers more than one row/column, its minimum size is distributed over the
    rows/columns it covers, proportional to the stretch factor for each row/column.  Finally, the min values and stretch
    factors must both be satisfied simultaneously.  For example, if the row mins are set to be [1 2 3] and the row
    stretch factors are set to be [1 2 3], the minimum width will be 6 (neglecting the margins).  However, if the mins
    were instead set to be [4 2 3], the stretch factors would lead to effective minimums of [4 8 12] to maintain
    proportionality, making the minimum width 24.

    \see The Wnd documentation has further information about layouts attached to Wnds.
*/
class GG_API Layout : public Wnd
{
public:
    using Wnd::SizeMove;

    /** \name Structors */ //@{
    /** ctor.  \throw std::invalid_argument May throw std::invalid_argument if \a border_margin is < 0. */
    Layout(int rows, int columns, int border_margin = 0, int cell_margin = -1);

    /** ctor.  \throw std::invalid_argument May throw std::invalid_argument if \a m_border_margin is < 0. */
    Layout(int x, int y, int w, int h, int rows, int columns, int border_margin = 0, int cell_margin = -1);
    //@}

    /** \name Accessors */ //@{
    int    Rows() const;                             ///< returns the number of rows in the layout
    int    Columns() const;                          ///< returns the number of columns in the layout
    Uint32 ChildAlignment(Wnd* wnd) const;           ///< returns the aligment of child \a wnd.  \throw std::runtime_error May throw std::runtime_error if no such child exists.
    int    BorderMargin() const;                     ///< returns the number of pixels that the layout will leave between its edges and the windows it contains
    int    CellMargin() const;                       ///< returns the number of pixels the layout leaves between the edges of windows in adjacent cells
    double RowStretch(int row) const;                ///< returns the stretch factor for row \a row.  Note that \a row is not range-checked.
    double ColumnStretch(int column) const;          ///< returns the stretch factor for column \a column.  Note that \a column is not range-checked.
    int    MinimumRowHeight(int row) const;          ///< returns the minimum height allowed for row \a row.  Note that \a row is not range-checked.
    int    MinimumColumnWidth(int column) const;     ///< returns the minimum height allowed for column \a column.  Note that \a column is not range-checked.
    //@}
   
    /** \name Mutators */ //@{
    virtual void SizeMove(int x1, int y1, int x2, int y2);
    virtual void MouseWheel(const Pt& pt, int move, Uint32 keys);
    virtual void Keypress(Key key, Uint32 key_mods);

    /** inserts \a w into the layout in the indicated cell, expanding the layout grid as necessary.  Note that \a row
        and \a column must not be negative, though this is not checked. */
    void Add(Wnd *w, int row, int column, Uint32 alignment = 0);

    /** inserts \a w into the layout, covering the indicated cell(s), expanding the layout grid as necessary.  The
        num_rows and num_columns indicate how many rows and columns \a w covers, respectively.  So Add(foo, 1, 2, 2, 3)
        covers cells (1, 2) through (2, 4), inclusive.  Note that \a row, and \a column must be nonnegative and \a
        num_rows and \a num_columns must be positive, though this is not checked. */
    void Add(Wnd *w, int row, int column, int num_rows, int num_columns, Uint32 alignment = 0);

    /** resizes the layout to be \a rows by \a columns.  If the layout shrinks, any contained windows are deleted.  Each
        of \a rows and \a columns must be greater than 0, though this is not checked. */
    void ResizeLayout(int rows, int columns);

    /** sets the aligment of child \a wnd to \a alignment.  If no such child exists, no action is taken. */
    void SetChildAlignment(Wnd* wnd, Uint32 alignment);

    /** sets the number of pixels that the layout will leave between its edges and the windows it contains */
    void SetBorderMargin(int margin);

    /** sets the number of pixels the layout leaves between the edges of windows in adjacent cells */
    void SetCellMargin(int margin);

    /** sets the amount of stretching, relative to other rows, that \a row will do when the layout is resized.  0.0
        indicates that the row's size will not change unless all rows have 0.0 stretch as well.  Note that \a row is not
        range-checked. */
    void SetRowStretch(int row, double stretch);

    /** sets the amount of stretching, relative to other columns, that \a column will do when the layout is resized.
        0.0 indicates that the column's size will not change unless all columns have 0.0 stretch as well.  Note that \a
        column is not range-checked. */
    void SetColumnStretch(int column, double stretch);

    /** sets the minimum height of row \a row to \a height.  Note that \a row is not range-checked. */
    void SetMinimumRowHeight(int row, int height);

    /** sets the minimum width of column \a column to \a width.  Note that \a column is not range-checked. */
    void SetMinimumColumnWidth(int column, int width);

    virtual void DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ //@{
    Layout(); ///< default ctor
    //@}

private:
    struct RowColParams
    {
        RowColParams();

        double stretch;
        int    min;
        int    effective_min;   ///< current effective minimum width of this row or column, based on min, layout margins, and layout cell contents
        int    current_origin;  ///< current position of top or left side
        int    current_width;   ///< current extent in downward or right direction

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    struct WndPosition
    {
        WndPosition();
        WndPosition(int first_row_, int first_column_, int last_row_, int last_column_, Uint32 alignment_, const Pt& original_size_);

        int    first_row;
        int    first_column;
        int    last_row;
        int    last_column;
        Uint32 alignment;
        Pt     original_size;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    double TotalStretch(const std::vector<RowColParams>& params_vec) const;
    int    TotalMinWidth() const;
    int    TotalMinHeight() const;
    void   ValidateAlignment(Uint32& alignment);
    void   RedoLayout();
    void   ChildSizeOrMinSizeOrMaxSizeChanged();

    std::vector<std::vector<Wnd*> > m_cells;
    int                             m_border_margin;
    int                             m_cell_margin;
    std::vector<RowColParams>       m_row_params;
    std::vector<RowColParams>       m_column_params;
    std::map<Wnd*, WndPosition>     m_wnd_positions;
    bool                            m_ignore_child_resize;
    bool                            m_ignore_parent_resize;

    friend class Wnd;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::Layout::RowColParams::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(stretch)
        & BOOST_SERIALIZATION_NVP(min)
        & BOOST_SERIALIZATION_NVP(effective_min)
        & BOOST_SERIALIZATION_NVP(current_origin)
        & BOOST_SERIALIZATION_NVP(current_width);
}

template <class Archive>
void GG::Layout::WndPosition::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(first_row)
        & BOOST_SERIALIZATION_NVP(first_column)
        & BOOST_SERIALIZATION_NVP(last_row)
        & BOOST_SERIALIZATION_NVP(last_column)
        & BOOST_SERIALIZATION_NVP(alignment)
        & BOOST_SERIALIZATION_NVP(original_size);
}

template <class Archive>
void GG::Layout::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd)
        & BOOST_SERIALIZATION_NVP(m_cells)
        & BOOST_SERIALIZATION_NVP(m_border_margin)
        & BOOST_SERIALIZATION_NVP(m_cell_margin)
        & BOOST_SERIALIZATION_NVP(m_row_params)
        & BOOST_SERIALIZATION_NVP(m_column_params)
        & BOOST_SERIALIZATION_NVP(m_wnd_positions)
        & BOOST_SERIALIZATION_NVP(m_ignore_child_resize);
}

#endif // _GGLayout_h_
