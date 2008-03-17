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

#include <GG/Layout.h>

#include <GG/DrawUtil.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>

#include <cassert>
#include <cmath>


using namespace GG;

namespace {
    int MinDueToMargin(int cell_margin, int num_rows_or_columns, int row_or_column)
    {
        return (row_or_column == 0 || row_or_column == num_rows_or_columns - 1) ?
            static_cast<int>(std::ceil(cell_margin / 2.0)) :
            cell_margin;
    }
}

struct GG::SetMarginAction : AttributeChangedAction<int>
{
    SetMarginAction(Layout* layout) : m_layout(layout) {}
    void operator()(const int& sort_col) {m_layout->RedoLayout();}
private:
    Layout* m_layout;
};

// RowColParams
Layout::RowColParams::RowColParams() :
    stretch(0),
    min(0),
    effective_min(0),
    current_origin(0),
    current_width(0)
{}

// WndPosition
Layout::WndPosition::WndPosition() :
    first_row(0),
    first_column(0),
    last_row(0),
    last_column(0),
    alignment(ALIGN_NONE)
{}

Layout::WndPosition::WndPosition(int first_row_, int first_column_, int last_row_, int last_column_, Flags<Alignment> alignment_, const Pt& original_ul_, const Pt& original_size_) :
    first_row(first_row_),
    first_column(first_column_),
    last_row(last_row_),
    last_column(last_column_),
    alignment(alignment_),
    original_ul(original_ul_),
    original_size(original_size_)
{}

// Layout
Layout::Layout() :
    Wnd(),
    m_border_margin(0),
    m_cell_margin(0),
    m_ignore_child_resize(false),
    m_ignore_parent_resize(false),
    m_render_outline(false),
    m_outline_color(CLR_MAGENTA)
{}

Layout::Layout(int x, int y, int w, int h, int rows, int columns, int border_margin/* = 0*/, int cell_margin/* = -1*/) :
    Wnd(x, y, w, h, Flags<WndFlag>()),
    m_cells(rows, std::vector<Wnd*>(columns)),
    m_border_margin(border_margin),
    m_cell_margin(cell_margin < 0 ? border_margin : cell_margin),
    m_row_params(rows),
    m_column_params(columns),
    m_ignore_child_resize(false),
    m_ignore_parent_resize(false),
    m_render_outline(false),
    m_outline_color(CLR_MAGENTA)
{
    if (m_border_margin < 0)
        throw InvalidMargin("Layout::Layout() : m_border_margin may not be less than 0");
}

Pt Layout::MinUsableSize() const
{ return m_min_usable_size; }

int Layout::Rows() const
{ return m_cells.size(); }

int Layout::Columns() const
{ return m_cells.empty() ? 0 : m_cells[0].size(); }

Flags<Alignment> Layout::ChildAlignment(Wnd* wnd) const
{
    std::map<Wnd*, WndPosition>::const_iterator it = m_wnd_positions.find(wnd);
    if (it == m_wnd_positions.end())
        throw NoSuchChild("Layout::ChildAlignment() : Alignment of a nonexistent child was requested");
    return it->second.alignment;
}

int Layout::BorderMargin() const
{ return m_border_margin; }

int Layout::CellMargin() const
{ return m_cell_margin; }

double Layout::RowStretch(int row) const
{ return m_row_params[row].stretch; }

double Layout::ColumnStretch(int column) const
{ return m_column_params[column].stretch; }

int Layout::MinimumRowHeight(int row) const
{ return m_row_params[row].min; }

int Layout::MinimumColumnWidth(int column) const
{ return m_column_params[column].min; }

std::vector<std::vector<const Wnd*> > Layout::Cells() const
{
    std::vector<std::vector<const Wnd*> > retval(m_cells.size());
    for (unsigned int i = 0; i < m_cells.size(); ++i) {
        retval[i].resize(m_cells[i].size());
        for (unsigned int j = 0; j < m_cells[i].size(); ++j) {
            retval[i][j] = m_cells[i][j];
        }
    }    
    return retval;
}

std::vector<std::vector<Rect> > Layout::CellRects() const
{
    std::vector<std::vector<Rect> > retval = RelativeCellRects();
    for (unsigned int i = 0; i < retval.size(); ++i) {
        for (unsigned int j = 0; j < retval[i].size(); ++j) {
            retval[i][j] += ClientUpperLeft();
        }
    }
    return retval;
}

std::vector<std::vector<Rect> > Layout::RelativeCellRects() const
{
    std::vector<std::vector<Rect> > retval(m_cells.size());
    for (unsigned int i = 0; i < m_cells.size(); ++i) {
        retval[i].resize(m_cells[i].size());
        for (unsigned int j = 0; j < m_cells[i].size(); ++j) {
            Pt ul(m_column_params[j].current_origin,
                  m_row_params[i].current_origin);
            Pt lr = ul + Pt(m_column_params[j].current_width,
                            m_row_params[i].current_width);
            Rect rect(ul, lr);
            if (!j)
                rect.ul.x += m_border_margin;
            else
                rect.ul.x += m_cell_margin / 2;
            if (j == m_cells[i].size() - 1)
                rect.lr.x -= m_border_margin;
            else
                rect.lr.x -= m_cell_margin - m_cell_margin / 2;
            if (!i)
                rect.ul.y += m_border_margin;
            else
                rect.ul.y += m_cell_margin / 2;
            if (i == m_cells.size() - 1)
                rect.lr.y -= m_border_margin;
            else
                rect.lr.y -= m_cell_margin - m_cell_margin / 2;
            retval[i][j] = rect;
        }
    }
    return retval;
}

bool Layout::RenderOutline() const
{ return m_render_outline; }

Clr Layout::OutlineColor() const
{ return m_outline_color; }

void Layout::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{
    if (Parent())
        Parent()->StartingChildDragDrop(wnd, offset);
}

void Layout::CancellingChildDragDrop(const std::list<Wnd*>& wnds)
{
    if (Parent())
        Parent()->CancellingChildDragDrop(wnds);
}

void Layout::ChildrenDraggedAway(const std::list<Wnd*>& wnds, const Wnd* destination)
{
    if (Parent())
        Parent()->ChildrenDraggedAway(wnds, destination);
}

void Layout::SizeMove(const Pt& ul, const Pt& lr)
{
    if (m_ignore_parent_resize)
        return;

    // these hold values used to calculate m_min_usable_size
    std::vector<int> row_effective_min_usable_sizes(m_row_params.size());
    std::vector<int> column_effective_min_usable_sizes(m_column_params.size());

    // reset effective_min values
    for (unsigned int i = 0; i < m_row_params.size(); ++i) {
        int min_due_to_margin = MinDueToMargin(m_cell_margin, m_row_params.size(), i);
        m_row_params[i].effective_min = std::max(m_row_params[i].min, min_due_to_margin);
        row_effective_min_usable_sizes[i] = min_due_to_margin;
    }

    for (unsigned int i = 0; i < m_column_params.size(); ++i) {
        int min_due_to_margin = MinDueToMargin(m_cell_margin, m_column_params.size(), i);
        m_column_params[i].effective_min = std::max(m_column_params[i].min, min_due_to_margin);
        column_effective_min_usable_sizes[i] = min_due_to_margin;
    }

    // adjust effective minimums based on cell contents
    for (std::map<Wnd*, WndPosition>::iterator it = m_wnd_positions.begin(); it != m_wnd_positions.end(); ++it) {
        Pt margin;
        if (0 < it->second.first_row && it->second.last_row < static_cast<int>(m_row_params.size()))
            margin.y = m_cell_margin;
        else if (0 < it->second.first_row || it->second.last_row < static_cast<int>(m_row_params.size()))
            margin.y = static_cast<int>(std::ceil(m_cell_margin / 2.0));
        if (0 < it->second.first_column && it->second.last_column < static_cast<int>(m_column_params.size()))
            margin.x = m_cell_margin;
        else if (0 < it->second.first_column || it->second.last_column < static_cast<int>(m_column_params.size()))
            margin.x = static_cast<int>(std::ceil(m_cell_margin / 2.0));

        Pt min_space_needed = it->first->MinSize() + margin;
        Pt min_usable_size = it->first->MinUsableSize() + margin;

        // adjust row minimums
        double total_stretch = 0.0;
        for (int i = it->second.first_row; i < it->second.last_row; ++i) {
            total_stretch += m_row_params[i].stretch;
        }
        if (total_stretch) {
            for (int i = it->second.first_row; i < it->second.last_row; ++i) {
                m_row_params[i].effective_min = std::max(m_row_params[i].effective_min, static_cast<int>(min_space_needed.y / total_stretch * m_row_params[i].stretch));
                row_effective_min_usable_sizes[i] = std::max(row_effective_min_usable_sizes[i], static_cast<int>(min_usable_size.y / total_stretch * m_row_params[i].stretch));
            }
        } else { // if all rows have 0.0 stretch, distribute height evenly
            double per_row_min = min_space_needed.y / static_cast<double>(it->second.last_row - it->second.first_row);
            double per_row_usable_min = min_usable_size.y / static_cast<double>(it->second.last_row - it->second.first_row);
            for (int i = it->second.first_row; i < it->second.last_row; ++i) {
                m_row_params[i].effective_min = std::max(m_row_params[i].effective_min, static_cast<int>(per_row_min + 0.5));
                row_effective_min_usable_sizes[i] = std::max(row_effective_min_usable_sizes[i], static_cast<int>(per_row_usable_min + 0.5));
            }
        }

        // adjust column minimums
        total_stretch = 0.0;
        for (int i = it->second.first_column; i < it->second.last_column; ++i) {
            total_stretch += m_column_params[i].stretch;
        }
        if (total_stretch) {
            for (int i = it->second.first_column; i < it->second.last_column; ++i) {
                m_column_params[i].effective_min = std::max(m_column_params[i].effective_min, static_cast<int>(min_space_needed.x / total_stretch * m_column_params[i].stretch));
                column_effective_min_usable_sizes[i] = std::max(column_effective_min_usable_sizes[i], static_cast<int>(min_usable_size.x / total_stretch * m_column_params[i].stretch));
            }
        } else { // if all columns have 0.0 stretch, distribute width evenly
            double per_column_min = min_space_needed.x / static_cast<double>(it->second.last_column - it->second.first_column);
            double per_column_usable_min = min_usable_size.x / static_cast<double>(it->second.last_column - it->second.first_column);
            for (int i = it->second.first_column; i < it->second.last_column; ++i) {
                m_column_params[i].effective_min = std::max(m_column_params[i].effective_min, static_cast<int>(per_column_min + 0.5));
                column_effective_min_usable_sizes[i] = std::max(column_effective_min_usable_sizes[i], static_cast<int>(per_column_usable_min + 0.5));
            }
        }
    }

    // determine final effective minimums, preserving stretch ratios
    double greatest_min_over_stretch_ratio = 0.0;
    double greatest_usable_min_over_stretch_ratio = 0.0;
    for (unsigned int i = 0; i < m_row_params.size(); ++i) {
        if (m_row_params[i].stretch) {
            greatest_min_over_stretch_ratio = std::max(greatest_min_over_stretch_ratio, m_row_params[i].effective_min / m_row_params[i].stretch);
            greatest_usable_min_over_stretch_ratio = std::max(greatest_usable_min_over_stretch_ratio, row_effective_min_usable_sizes[i] / m_row_params[i].stretch);
        }
    }
    for (unsigned int i = 0; i < m_row_params.size(); ++i) {
        m_row_params[i].effective_min = std::max(m_row_params[i].effective_min, static_cast<int>(m_row_params[i].stretch * greatest_min_over_stretch_ratio));
        row_effective_min_usable_sizes[i] = std::max(row_effective_min_usable_sizes[i], static_cast<int>(m_row_params[i].stretch * greatest_usable_min_over_stretch_ratio));
    }
    greatest_min_over_stretch_ratio = 0.0;
    greatest_usable_min_over_stretch_ratio = 0.0;
    for (unsigned int i = 0; i < m_column_params.size(); ++i) {
        if (m_column_params[i].stretch) {
            greatest_min_over_stretch_ratio = std::max(greatest_min_over_stretch_ratio, m_column_params[i].effective_min / m_column_params[i].stretch);
            greatest_usable_min_over_stretch_ratio = std::max(greatest_usable_min_over_stretch_ratio, column_effective_min_usable_sizes[i] / m_column_params[i].stretch);
        }
    }
    for (unsigned int i = 0; i < m_column_params.size(); ++i) {
        m_column_params[i].effective_min = std::max(m_column_params[i].effective_min, static_cast<int>(m_column_params[i].stretch * greatest_min_over_stretch_ratio));
        column_effective_min_usable_sizes[i] = std::max(column_effective_min_usable_sizes[i], static_cast<int>(m_column_params[i].stretch * greatest_usable_min_over_stretch_ratio));
    }

    m_min_usable_size.x = 2 * m_border_margin;
    for (unsigned int i = 0; i < column_effective_min_usable_sizes.size(); ++i) {
        m_min_usable_size.x += column_effective_min_usable_sizes[i];
    }
    m_min_usable_size.y = 2 * m_border_margin;
    for (unsigned int i = 0; i < row_effective_min_usable_sizes.size(); ++i) {
        m_min_usable_size.y += row_effective_min_usable_sizes[i];
    }

    bool size_or_min_size_changed = false;
    Pt new_min_size(TotalMinWidth(), TotalMinHeight());
    if (new_min_size != MinSize()) {
        SetMinSize(new_min_size);
        size_or_min_size_changed = true;
    }
    Pt original_size = Size();
    Wnd::SizeMove(ul, lr);
    if (Size() != original_size)
        size_or_min_size_changed = true;

    // if this is the layout object for some Wnd, propogate the minimum size up to the owning Wnd
    if (Wnd* parent = Parent()) {
        if (const_cast<const Wnd*>(parent)->GetLayout() == this) {
            Pt new_parent_min_size = MinSize() + parent->Size() - parent->ClientSize();
            Pt parent_min_size = parent->MinSize();
            new_parent_min_size.x = std::max(parent_min_size.x, new_parent_min_size.x);
            new_parent_min_size.y = std::max(parent_min_size.y, new_parent_min_size.y);
            m_ignore_parent_resize = true;
            parent->SetMinSize(Pt(new_parent_min_size.x, new_parent_min_size.y));
            m_ignore_parent_resize = false;
        }
    }

    // determine row and column positions
    double total_stretch = TotalStretch(m_row_params);
    int total_stretch_space = Size().y - MinSize().y;
    double space_per_unit_stretch = total_stretch ? total_stretch_space / total_stretch : 0.0;
    bool larger_than_min = 0 < total_stretch_space;
    double remainder = 0.0;
    int current_origin = m_border_margin;
    for (unsigned int i = 0; i < m_row_params.size(); ++i) {
        if (larger_than_min) {
            if (i < m_row_params.size() - 1) {
                double raw_width =
                    m_row_params[i].effective_min +
                    (total_stretch ?
                     space_per_unit_stretch * m_row_params[i].stretch :
                     total_stretch_space / static_cast<double>(m_row_params.size()));
                int int_raw_width = static_cast<int>(raw_width);
                m_row_params[i].current_width = int_raw_width;
                remainder += raw_width - int_raw_width;
                if (1.0 < remainder) {
                    --remainder;
                    ++m_row_params[i].current_width;
                }
            } else {
                m_row_params[i].current_width = (Height() - m_border_margin) - current_origin;
            }
        } else {
            m_row_params[i].current_width = m_row_params[i].effective_min;
        }
        m_row_params[i].current_origin = current_origin;
        current_origin += m_row_params[i].current_width;
    }

    total_stretch = TotalStretch(m_column_params);
    total_stretch_space = Size().x - MinSize().x;
    space_per_unit_stretch = total_stretch ? total_stretch_space / total_stretch : 0.0;
    larger_than_min = 0 < total_stretch_space;
    remainder = 0.0;
    current_origin = m_border_margin;
    for (unsigned int i = 0; i < m_column_params.size(); ++i) {
        if (larger_than_min) {
            if (i < m_column_params.size() - 1) {
                double raw_width =
                    m_column_params[i].effective_min +
                    (total_stretch ?
                     space_per_unit_stretch * m_column_params[i].stretch :
                     total_stretch_space / static_cast<double>(m_column_params.size()));
                int int_raw_width = static_cast<int>(raw_width);
                m_column_params[i].current_width = int_raw_width;
                remainder += raw_width - int_raw_width;
                if (1.0 < remainder) {
                    --remainder;
                    ++m_column_params[i].current_width;
                }
            } else {
                m_column_params[i].current_width = (Width() - m_border_margin) - current_origin;
            }
        } else {
            m_column_params[i].current_width = m_column_params[i].effective_min;
        }
        m_column_params[i].current_origin = current_origin;
        current_origin += m_column_params[i].current_width;
    }

    if (m_row_params.back().current_origin + m_row_params.back().current_width != Height() - m_border_margin)
        throw FailedCalculationCheck("Layout::SizeMove() : calculated row positions do not sum to the height of the layout");

    if (m_column_params.back().current_origin + m_column_params.back().current_width != Width() - m_border_margin)
        throw FailedCalculationCheck("Layout::SizeMove() : calculated column positions do not sum to the width of the layout");

    // resize cells and their contents
    m_ignore_child_resize = true;
    for (std::map<Wnd*, WndPosition>::iterator it = m_wnd_positions.begin(); it != m_wnd_positions.end(); ++it) {
        Pt ul(m_column_params[it->second.first_column].current_origin,
              m_row_params[it->second.first_row].current_origin);
        Pt lr(m_column_params[it->second.last_column - 1].current_origin + m_column_params[it->second.last_column - 1].current_width,
              m_row_params[it->second.last_row - 1].current_origin + m_row_params[it->second.last_row - 1].current_width);
        if (0 < it->second.first_row)
            ul.y += m_cell_margin / 2;
        if (0 < it->second.first_column)
            ul.x += m_cell_margin / 2;
        if (it->second.last_row < static_cast<int>(m_row_params.size()))
            lr.y -= static_cast<int>(m_cell_margin / 2.0 + 0.5);
        if (it->second.last_column < static_cast<int>(m_column_params.size()))
            lr.x -= static_cast<int>(m_cell_margin / 2.0 + 0.5);

        if (it->second.alignment == ALIGN_NONE) { // expand to fill available space
            it->first->SizeMove(ul, lr);
        } else { // align as appropriate
            Pt available_space = lr - ul;
            Pt min_usable_size = it->first->MinUsableSize();
            Pt window_size(std::min(available_space.x, std::max(it->second.original_size.x, min_usable_size.x)),
                           std::min(available_space.y, std::max(it->second.original_size.y, min_usable_size.y)));
            Pt resize_ul, resize_lr;
            if (it->second.alignment & ALIGN_LEFT) {
                resize_ul.x = ul.x;
                resize_lr.x = resize_ul.x + window_size.x;
            } else if (it->second.alignment & ALIGN_CENTER) {
                resize_ul.x = ul.x + (available_space.x - window_size.x) / 2;
                resize_lr.x = resize_ul.x + window_size.x;
            } else if (it->second.alignment & ALIGN_RIGHT) {
                resize_lr.x = lr.x;
                resize_ul.x = resize_lr.x - window_size.x;
            } else {
                resize_ul.x = ul.x;
                resize_lr.x = lr.x;
            }
            if (it->second.alignment & ALIGN_TOP) {
                resize_ul.y = ul.y;
                resize_lr.y = resize_ul.y + window_size.y;
            } else if (it->second.alignment & ALIGN_VCENTER) {
                resize_ul.y = ul.y + (available_space.y - window_size.y) / 2;
                resize_lr.y = resize_ul.y + window_size.y;
            } else if (it->second.alignment & ALIGN_BOTTOM) {
                resize_lr.y = lr.y;
                resize_ul.y = resize_lr.y - window_size.y;
            } else {
                resize_ul.y = ul.y;
                resize_lr.y = lr.y;
            }
            it->first->SizeMove(resize_ul, resize_lr);
        }
    }
    m_ignore_child_resize = false;

    if (ContainingLayout() && size_or_min_size_changed)
        ContainingLayout()->ChildSizeOrMinSizeOrMaxSizeChanged();
}

void Layout::Render()
{
    if (m_render_outline) {
        Pt ul = UpperLeft(), lr = LowerRight();
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, CLR_ZERO, m_outline_color, 1);
        std::vector<std::vector<Rect> > rects = CellRects();
        for (unsigned int i = 0; i < rects.size(); ++i) {
            for (unsigned int j = 0; j < rects[i].size(); ++j) {
                FlatRectangle(rects[i][j].ul.x, rects[i][j].ul.y, rects[i][j].lr.x, rects[i][j].lr.y,
                              CLR_ZERO, m_outline_color, 1);
            }
        }
    }
}

void Layout::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (Parent())
        Parent()->MouseWheel(pt, move, mod_keys);
}

void Layout::KeyPress(Key key, Flags<ModKey> mod_keys)
{
    if (Parent())
        Parent()->KeyPress(key, mod_keys);
}

void Layout::KeyRelease(Key key, Flags<ModKey> mod_keys)
{
    if (Parent())
        Parent()->KeyRelease(key, mod_keys);
}

void Layout::Add(Wnd* wnd, int row, int column, Flags<Alignment> alignment/* = ALIGN_NONE*/)
{ Add(wnd, row, column, 1, 1, alignment); }

void Layout::Add(Wnd* wnd, int row, int column, int num_rows, int num_columns, Flags<Alignment> alignment/* = ALIGN_NONE*/)
{
    int last_row = row + num_rows;
    int last_column = column + num_columns;
    assert(0 <= row);
    assert(0 <= column);
    assert(row < last_row);
    assert(column < last_column);
    ValidateAlignment(alignment);
    if (m_cells.size() < static_cast<unsigned int>(last_row) || m_cells[0].size() < static_cast<unsigned int>(last_column)) {
        ResizeLayout(std::max(last_row, Rows()), std::max(last_column, Columns()));
    }
    for (int i = row; i < last_row; ++i) {
        for (int j = column; j < last_column; ++j) {
            if (m_cells[i][j])
                throw AttemptedOverwrite("Layout::Add() : Attempted to add a Wnd to a layout cell that is already occupied");
            m_cells[i][j] = wnd;
        }
    }
    if (wnd) {
        m_wnd_positions[wnd] = WndPosition(row, column, last_row, last_column, alignment, wnd->RelativeUpperLeft(), wnd->Size());
        AttachChild(wnd);
    }
    RedoLayout();
}

void Layout::Remove(Wnd* wnd)
{
    std::map<Wnd*, WndPosition>::const_iterator it = m_wnd_positions.find(wnd);
    if (it != m_wnd_positions.end()) {
        const WndPosition& wnd_position = it->second;
        for (int i = wnd_position.first_row; i < wnd_position.last_row; ++i) {
            for (int j = wnd_position.first_column; j < wnd_position.last_column; ++j) {
                m_cells[i][j] = 0;
            }
        }
        Pt original_ul = it->second.original_ul;
        Pt original_size = it->second.original_size;
        m_wnd_positions.erase(wnd);
        RedoLayout();
        DetachChild(wnd);
        wnd->SizeMove(original_ul, original_ul + original_size);
    }
}

void Layout::DetachAndResetChildren()
{
    std::map<Wnd*, WndPosition> wnd_positions = m_wnd_positions;
    DetachChildren();
    for (std::map<Wnd*, WndPosition>::iterator it = wnd_positions.begin(); it != wnd_positions.end(); ++it) {
        it->first->SizeMove(it->second.original_ul, it->second.original_ul + it->second.original_size);
    }
    m_wnd_positions.clear();
}

void Layout::ResizeLayout(int rows, int columns)
{
    assert(0 < rows);
    assert(0 < columns);
    if (static_cast<unsigned int>(rows) < m_cells.size()) {
        for (unsigned int i = static_cast<unsigned int>(rows); i < m_cells.size(); ++i) {
            for (unsigned int j = 0; j < m_cells[i].size(); ++j) {
                DeleteChild(m_cells[i][j]);
                m_wnd_positions.erase(m_cells[i][j]);
            }
        }
    }
    m_cells.resize(rows);
    for (unsigned int i = 0; i < m_cells.size(); ++i) {
        if (static_cast<unsigned int>(columns) < m_cells[i].size()) {
            for (unsigned int j = static_cast<unsigned int>(columns); j < m_cells[i].size(); ++j) {
                DeleteChild(m_cells[i][j]);
                m_wnd_positions.erase(m_cells[i][j]);
            }
        }
        m_cells[i].resize(columns);
    }
    m_row_params.resize(rows);
    m_column_params.resize(columns);
    RedoLayout();
}

void Layout::SetChildAlignment(Wnd* wnd, Flags<Alignment> alignment)
{
    std::map<Wnd*, WndPosition>::iterator it = m_wnd_positions.find(wnd);
    if (it != m_wnd_positions.end()) {
        ValidateAlignment(alignment);
        it->second.alignment = alignment;
        RedoLayout();
    }
}

void Layout::SetBorderMargin(int margin)
{
    m_border_margin = margin;
    RedoLayout();
}

void Layout::SetCellMargin(int margin)
{
    m_cell_margin = margin;
    RedoLayout();
}

void Layout::SetRowStretch(int row, double stretch)
{
    assert(static_cast<unsigned int>(row) < m_row_params.size());
    m_row_params[row].stretch = stretch;
    RedoLayout();
}

void Layout::SetColumnStretch(int column, double stretch)
{
    assert(static_cast<unsigned int>(column) < m_column_params.size());
    m_column_params[column].stretch = stretch;
    RedoLayout();
}

void Layout::SetMinimumRowHeight(int row, int height)
{
    assert(static_cast<unsigned int>(row) < m_row_params.size());
    m_row_params[row].min = height;
    RedoLayout();
}

void Layout::SetMinimumColumnWidth(int column, int width)
{
    assert(static_cast<unsigned int>(column) < m_column_params.size());
    m_column_params[column].min = width;
    RedoLayout();
}

void Layout::RenderOutline(bool render_outline)
{ m_render_outline = render_outline; }

void Layout::SetOutlineColor(Clr color)
{ m_outline_color = color; }

void Layout::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Wnd::DefineAttributes(editor);
    editor->Label("Layout");
    boost::shared_ptr<SetMarginAction> set_margin_action(new SetMarginAction(this));
    editor->Attribute<int>("Border Margin", m_border_margin, set_margin_action);
    editor->Attribute<int>("Cell Margin", m_cell_margin, set_margin_action);
    // TODO: handle setting the number of rows and columns
}

double Layout::TotalStretch(const std::vector<RowColParams>& params_vec) const
{
    double retval = 0.0;
    for (unsigned int i = 0; i < params_vec.size(); ++i) {
        retval += params_vec[i].stretch;
    }
    return retval;
}

int Layout::TotalMinWidth() const
{
    int retval = 2 * m_border_margin;
    for (unsigned int i = 0; i < m_column_params.size(); ++i) {
        retval += m_column_params[i].effective_min;
    }
    return retval;
}

int Layout::TotalMinHeight() const
{
    int retval = 2 * m_border_margin;
    for (unsigned int i = 0; i < m_row_params.size(); ++i) {
        retval += m_row_params[i].effective_min;
    }
    return retval;
}

void Layout::ValidateAlignment(Flags<Alignment>& alignment)
{
    int dup_ct = 0;   // duplication count
    if (alignment & ALIGN_LEFT) ++dup_ct;
    if (alignment & ALIGN_RIGHT) ++dup_ct;
    if (alignment & ALIGN_CENTER) ++dup_ct;
    if (1 < dup_ct) {   // when multiples are picked, use ALIGN_CENTER by default
        alignment &= ~(ALIGN_RIGHT | ALIGN_LEFT);
        alignment |= ALIGN_CENTER;
    }
    dup_ct = 0;
    if (alignment & ALIGN_TOP) ++dup_ct;
    if (alignment & ALIGN_BOTTOM) ++dup_ct;
    if (alignment & ALIGN_VCENTER) ++dup_ct;
    if (1 < dup_ct) {   // when multiples are picked, use ALIGN_VCENTER by default
        alignment &= ~(ALIGN_TOP | ALIGN_BOTTOM);
        alignment |= ALIGN_VCENTER;
    }

    // get rid of any irrelevant bits
    if (!(alignment & (ALIGN_LEFT | ALIGN_RIGHT | ALIGN_CENTER | ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER)))
        alignment = ALIGN_NONE;
}

void Layout::RedoLayout()
{ Resize(Size()); }

void Layout::ChildSizeOrMinSizeOrMaxSizeChanged()
{
    if (!m_ignore_child_resize)
        RedoLayout();
}
