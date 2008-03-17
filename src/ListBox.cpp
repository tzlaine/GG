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

#include <GG/ListBox.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/Scroll.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>
#include <GG/WndEditor.h>

#include <boost/assign/list_of.hpp>

#include <cmath>
#include <numeric>

using namespace GG;

namespace {
    const int SCROLL_WIDTH = 14;
    const int DEFAULT_ROW_WIDTH = 50;
    const int DEFAULT_ROW_HEIGHT = 22;

    class RowSorter // used to sort rows by a certain column (which may contain some empty cells)
    {
    public:
        RowSorter(const boost::function<bool (const ListBox::Row&, const ListBox::Row&, int)>& cmp, int col, bool invert) : m_cmp(cmp), m_sort_col(col), m_invert(invert) {}

        bool operator()(const ListBox::Row* l, const ListBox::Row* r)
            {
                bool retval = m_cmp(*l, *r, m_sort_col);
                return m_invert ? !retval : retval;
            }

    private:
        boost::function<bool (const ListBox::Row&, const ListBox::Row&, int)> m_cmp;
        int m_sort_col;
        bool m_invert;
    };

    struct SetStyleAction : AttributeChangedAction<Flags<ListBoxStyle> >
    {
        SetStyleAction(ListBox* list_box) : m_list_box(list_box) {}
        void operator()(const Flags<ListBoxStyle>& style) {m_list_box->SetStyle(style);}
    private:
        ListBox* m_list_box;
    };

    struct SetSortColAction : AttributeChangedAction<int>
    {
        SetSortColAction(ListBox* list_box) : m_list_box(list_box) {}
        void operator()(const int& sort_col) {m_list_box->SetSortCol(sort_col);}
    private:
        ListBox* m_list_box;
    };

    ListBox::Row* GetPtrAtIndex(int i, const std::vector<ListBox::Row*>& rows)
    { return i == -1 ? 0 : rows[i]; }

    void IncrementIfGE(int& lhs, int rhs)
    {
        if (rhs <= lhs)
            ++lhs;
    }

    void DecrementIfGE(int& lhs, int rhs)
    {
        if (rhs <= lhs)
            --lhs;
    }
}

///////////////////////////////////////
// ListBoxStyle
///////////////////////////////////////
const ListBoxStyle GG::LIST_NONE            (0);
const ListBoxStyle GG::LIST_VCENTER         (1 << 0);
const ListBoxStyle GG::LIST_TOP             (1 << 1);
const ListBoxStyle GG::LIST_BOTTOM          (1 << 2);
const ListBoxStyle GG::LIST_CENTER          (1 << 3);
const ListBoxStyle GG::LIST_LEFT            (1 << 4);
const ListBoxStyle GG::LIST_RIGHT           (1 << 5);
const ListBoxStyle GG::LIST_NOSORT          (1 << 6);
const ListBoxStyle GG::LIST_SORTDESCENDING  (1 << 7);
const ListBoxStyle GG::LIST_NOSEL           (1 << 8);
const ListBoxStyle GG::LIST_SINGLESEL       (1 << 9);
const ListBoxStyle GG::LIST_QUICKSEL        (1 << 10);
const ListBoxStyle GG::LIST_USERDELETE      (1 << 11);
const ListBoxStyle GG::LIST_BROWSEUPDATES   (1 << 12);

GG_FLAGSPEC_IMPL(ListBoxStyle);

namespace {
    bool RegisterListBoxStyles()
    {
        FlagSpec<ListBoxStyle>& spec = FlagSpec<ListBoxStyle>::instance();
        spec.insert(LIST_NONE, "LIST_NONE", true);
        spec.insert(LIST_VCENTER, "LIST_VCENTER", true);
        spec.insert(LIST_TOP, "LIST_TOP", true);
        spec.insert(LIST_BOTTOM, "LIST_BOTTOM", true);
        spec.insert(LIST_CENTER, "LIST_CENTER", true);
        spec.insert(LIST_LEFT, "LIST_LEFT", true);
        spec.insert(LIST_RIGHT, "LIST_RIGHT", true);
        spec.insert(LIST_NOSORT, "LIST_NOSORT", true);
        spec.insert(LIST_SORTDESCENDING, "LIST_SORTDESCENDING", true);
        spec.insert(LIST_NOSEL, "LIST_NOSEL", true);
        spec.insert(LIST_SINGLESEL, "LIST_SINGLESEL", true);
        spec.insert(LIST_QUICKSEL, "LIST_QUICKSEL", true);
        spec.insert(LIST_USERDELETE, "LIST_USERDELETE", true);
        spec.insert(LIST_BROWSEUPDATES, "LIST_BROWSEUPDATES", true);
        return true;
    }
    bool dummy = RegisterListBoxStyles();
}


////////////////////////////////////////////////
// GG::ListBox::Row
////////////////////////////////////////////////
ListBox::Row::Row() :
    Control(0, 0, DEFAULT_ROW_WIDTH, DEFAULT_ROW_HEIGHT),
    m_row_alignment(ALIGN_VCENTER),
    m_margin(2)
{}

ListBox::Row::Row(int w, int h, const std::string& drag_drop_data_type, Alignment align/* = ALIGN_VCENTER*/, int margin/* = 2*/) : 
    Control(0, 0, w, h),
    m_row_alignment(align),
    m_margin(margin)
{
    SetDragDropDataType(drag_drop_data_type);
}

ListBox::Row::~Row()
{}

std::string ListBox::Row::SortKey(int column) const
{
    return at(column)->WindowText();
}

size_t ListBox::Row::size() const
{
    return m_cells.size();
}

bool ListBox::Row::empty() const
{
    return m_cells.empty();
}

Control* ListBox::Row::operator[](size_t n) const
{
    return m_cells[n];
}

Control* ListBox::Row::at(size_t n) const
{
    return m_cells.at(n);
}

Alignment ListBox::Row::RowAlignment() const
{
    return m_row_alignment;
}

Alignment ListBox::Row::ColAlignment(int n) const
{
    return m_col_alignments[n];
}

int ListBox::Row::ColWidth(int n) const
{
    return m_col_widths[n];
}

int ListBox::Row::Margin() const
{
    return m_margin;
}

Control* ListBox::Row::CreateControl(const std::string& str, const boost::shared_ptr<Font>& font, Clr color) const
{
    return GetStyleFactory()->NewTextControl(0, 0, str, font, color);
}

Control* ListBox::Row::CreateControl(const SubTexture& st) const
{
    return new StaticGraphic(0, 0, st.Width(), st.Height(), st, GRAPHIC_SHRINKFIT);
}

void ListBox::Row::Render()
{}

void ListBox::Row::push_back(Control* c)
{
    m_cells.push_back(c);
    m_col_widths.push_back(5);
    m_col_alignments.push_back(ALIGN_CENTER);
    if (1 < m_cells.size()) {
        m_col_widths.back() = m_col_widths[m_cells.size() - 1];
    }
    AdjustLayout();
}

void ListBox::Row::push_back(const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/)
{
    push_back(CreateControl(str, font, color));
}

void ListBox::Row::push_back(const std::string& str, const std::string& font_filename, int pts, Clr color/* = CLR_BLACK*/)
{
    push_back(CreateControl(str, GUI::GetGUI()->GetFont(font_filename, pts), color));
}

void ListBox::Row::push_back(const SubTexture& st)
{
    push_back(CreateControl(st));
}

void ListBox::Row::clear()
{
    m_cells.clear();
    RemoveLayout();
    DeleteChildren();
}

void ListBox::Row::resize(size_t n)
{
    size_t old_size = m_cells.size();
    for (size_t i = n; i < old_size; ++i) {
        delete m_cells[i];
    }
    m_cells.resize(n);
    m_col_widths.resize(n);
    m_col_alignments.resize(n);
    for (unsigned int i = old_size; i < n; ++i) {
        m_col_widths[i] = old_size ? m_col_widths[old_size - 1] : 5; // assign new cells reasonable default widths
        m_col_alignments[i] = ALIGN_CENTER;
    }
    AdjustLayout();
}

void ListBox::Row::SetCell(int n, Control* c)
{
    delete m_cells[n];
    m_cells[n] = c;
    AdjustLayout();
}

Control* ListBox::Row::RemoveCell(int n)
{
    Control* retval = m_cells[n];
    m_cells[n] = 0;
    AdjustLayout();
    return retval;
}

void ListBox::Row::SetRowAlignment(Alignment align)
{
    m_row_alignment = align;
    AdjustLayout();
}

void ListBox::Row::SetColAlignment(int n, Alignment align)
{
    m_col_alignments[n] = align;
    AdjustLayout();
}

void ListBox::Row::SetColWidth(int n, int width)
{
    m_col_widths[n] = width;
    AdjustLayout();
}

void ListBox::Row::SetColAlignments(const std::vector<Alignment>& aligns)
{
    m_col_alignments = aligns;
    AdjustLayout();
}

void ListBox::Row::SetColWidths(const std::vector<int>& widths)
{
    m_col_widths = widths;
    AdjustLayout();
}

void ListBox::Row::SetMargin(int margin)
{
    m_margin = margin;
    AdjustLayout();
}

void ListBox::Row::AdjustLayout(bool adjust_for_push_back/* = false*/)
{
    RemoveLayout();
    DetachChildren();

    bool nonempty_cell_found = false;
    for (unsigned int i = 0; i < m_cells.size(); ++i) {
        if (m_cells[i]) {
            nonempty_cell_found = true;
            break;
        }
    }

    if (!nonempty_cell_found)
        return;

    SetLayout(new Layout(0, 0, Width(), Height(), 1, m_cells.size(), m_margin, m_margin));
    Layout* layout = GetLayout();
    for (unsigned int i = 0; i < m_cells.size(); ++i) {
        layout->SetMinimumColumnWidth(i, m_col_widths[i]);
        if (m_cells[i]) {
            layout->Add(m_cells[i], 0, i, m_row_alignment | m_col_alignments[i]);
        }
    }
}

////////////////////////////////////////////////
// GG::ListBox
////////////////////////////////////////////////
// static(s)
const int ListBox::BORDER_THICK = 2;

ListBox::ListBox() :
    Control(),
    m_vscroll(0),
    m_hscroll(0),
    m_caret(-1),
    m_old_sel_row(-1),
    m_old_sel_row_selected(false),
    m_old_rdown_row(-1),
    m_lclick_row(-1),
    m_rclick_row(-1),
    m_last_row_browsed(-1),
    m_suppress_erase_signal(false),
    m_first_row_shown(0),
    m_first_col_shown(0),
    m_cell_margin(2),
    m_style(LIST_NONE),
    m_header_row(0),
    m_keep_col_widths(false),
    m_clip_cells(false),
    m_sort_col(0),
    m_sort_cmp(DefaultRowCmp<Row>()),
    m_auto_scroll_during_drag_drops(true),
    m_auto_scroll_margin(8),
    m_auto_scrolling_up(false),
    m_auto_scrolling_down(false),
    m_auto_scrolling_left(false),
    m_auto_scrolling_right(false),
    m_auto_scroll_timer(250)
{
    m_auto_scroll_timer.Stop();
    m_auto_scroll_timer.Connect(this);
}

ListBox::ListBox(int x, int y, int w, int h, Clr color, Clr interior/* = CLR_ZERO*/,
                 Flags<WndFlag> flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_vscroll(0),
    m_hscroll(0),
    m_caret(-1),
    m_old_sel_row(-1),
    m_old_sel_row_selected(false),
    m_old_rdown_row(-1),
    m_lclick_row(-1),
    m_rclick_row(-1),
    m_last_row_browsed(-1),
    m_suppress_erase_signal(false),
    m_first_row_shown(0),
    m_first_col_shown(0),
    m_cell_margin(2),
    m_int_color(interior),
    m_hilite_color(CLR_SHADOW),
    m_style(LIST_NONE),
    m_header_row(new Row()),
    m_keep_col_widths(false),
    m_clip_cells(false),
    m_sort_col(0),
    m_sort_cmp(DefaultRowCmp<Row>()),
    m_auto_scroll_during_drag_drops(true),
    m_auto_scroll_margin(8),
    m_auto_scrolling_up(false),
    m_auto_scrolling_down(false),
    m_auto_scrolling_left(false),
    m_auto_scrolling_right(false),
    m_auto_scroll_timer(250)
{
    Control::SetColor(color);
    ValidateStyle();
    SetText("ListBox");
    EnableChildClipping();
    m_auto_scroll_timer.Stop();
    m_auto_scroll_timer.Connect(this);
}

ListBox::~ListBox()
{
    delete m_header_row;
}

Pt ListBox::MinUsableSize() const
{
    return Pt(5 * SCROLL_WIDTH + 2 * BORDER_THICK,
              5 * SCROLL_WIDTH + 2 * BORDER_THICK);
}

Pt ListBox::ClientUpperLeft() const
{
    return UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->empty() ? 0 : m_header_row->Height()));
}

Pt ListBox::ClientLowerRight() const
{
    return LowerRight() - Pt(BORDER_THICK + RightMargin(), BORDER_THICK + BottomMargin());
}

bool ListBox::Empty() const
{
    return m_rows.empty();
}

const ListBox::Row& ListBox::GetRow(int n) const
{
    return *m_rows.at(n);
}

int ListBox::Caret() const
{
    return m_caret;
}

const std::set<int>& ListBox::Selections() const
{
    return m_selections;
}

bool ListBox::Selected(int n) const
{
    return m_selections.find(n) != m_selections.end();
}

Clr ListBox::InteriorColor() const
{
    return m_int_color;
}

Clr ListBox::HiliteColor() const
{
    return m_hilite_color;
}

Flags<ListBoxStyle> ListBox::Style() const
{
    return m_style;
}

const ListBox::Row& ListBox::ColHeaders() const
{
    return *m_header_row;
}

int ListBox::FirstRowShown() const
{
    return m_first_row_shown;
}

int ListBox::FirstColShown() const
{
    return m_first_col_shown;
}

int ListBox::LastVisibleRow() const
{
    int visible_pixels = ClientSize().y;
    int acc = 0;
    int i = m_first_row_shown;
    for (; i < static_cast<int>(m_rows.size()); ++i) {
        acc += m_rows[i]->Height();
        if (visible_pixels <= acc)
            break;
    }
    if (static_cast<int>(m_rows.size()) <= i)
        i = static_cast<int>(m_rows.size()) - 1;
    return i;
}

int ListBox::LastVisibleCol() const
{
    int visible_pixels = ClientSize().x;
    int acc = 0;
    int i = m_first_col_shown;
    for (; i < static_cast<int>(m_col_widths.size()); ++i) {
        acc += m_col_widths[i];
        if (visible_pixels <= acc)
            break;
    }
    if (static_cast<int>(m_col_widths.size()) <= i)
        i = static_cast<int>(m_col_widths.size()) - 1;
    return i;
}

int ListBox::NumRows() const
{
    return int(m_rows.size());
}

int ListBox::NumCols() const
{
    return m_col_widths.size();
}

bool ListBox::KeepColWidths() const
{
    return m_keep_col_widths;
}

int ListBox::SortCol() const
{
    return (m_sort_col >= 0 && m_sort_col < static_cast<int>(m_col_widths.size()) ? m_sort_col : 0);
}

int ListBox::ColWidth(int n) const
{
    return m_col_widths[n];
}

Alignment ListBox::ColAlignment(int n) const
{
    return m_col_alignments[n];
}

Alignment ListBox::RowAlignment(int n) const
{
    return m_rows[n]->RowAlignment();
}

const std::set<std::string>& ListBox::AllowedDropTypes() const
{
    return m_allowed_drop_types;
}

bool ListBox::AutoScrollDuringDragDrops() const
{
    return m_auto_scroll_during_drag_drops;
}

int ListBox::AutoScrollMargin() const
{
    return m_auto_scroll_margin;
}

int ListBox::AutoScrollInterval() const
{
    return m_auto_scroll_timer.Interval();
}

void ListBox::StartingChildDragDrop(const Wnd* wnd, const Pt& offset)
{
    if (m_selections.empty())
        return;

    int vertical_offset = offset.y;
    int wnd_idx = -1;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        if (m_rows[i] == wnd) {
            wnd_idx = i;
            break;
        }
    }
    assert(wnd_idx != -1);
    std::set<int>::iterator wnd_it = m_selections.find(wnd_idx);
    assert(wnd_it != m_selections.end());
    for (std::set<int>::iterator it = m_selections.begin(); it != wnd_it; ++it) {
        vertical_offset += m_rows[*it]->Height();
    }    
    for (std::set<int>::iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
        Wnd* row_wnd = static_cast<Wnd*>(m_rows[*it]);
        if (row_wnd != wnd) {
            GUI::GetGUI()->RegisterDragDropWnd(row_wnd, Pt(offset.x, vertical_offset), this);
            vertical_offset -= row_wnd->Height();
        } else {
            vertical_offset -= wnd->Height();
        }
    }
}

void ListBox::AcceptDrops(std::list<Wnd*>& wnds, const Pt& pt)
{
    for (std::list<Wnd*>::iterator it = wnds.begin(); it != wnds.end(); ) {
        std::list<Wnd*>::iterator tmp_it = it++;
        Wnd* wnd = *tmp_it;
        if (m_allowed_drop_types.find("") != m_allowed_drop_types.end() ||
            m_allowed_drop_types.find(wnd->DragDropDataType()) != m_allowed_drop_types.end()) {
            if (Row* row = dynamic_cast<Row*>(wnd)) {
                int insertion_row = RowUnderPt(pt);
                try {
                    Insert(row, insertion_row, true);
                } catch (const DontAcceptDrop&) {
                    wnds.erase(tmp_it);
                }
            }
        }
    }
}

void ListBox::ChildrenDraggedAway(const std::list<Wnd*>& wnds, const Wnd* destination)
{
    for (std::list<Wnd*>::const_iterator it = wnds.begin(); it != wnds.end(); ++it) {
        Row* row = dynamic_cast<Row*>(*it);
        assert(row);
        int idx = -1;
        for (unsigned int i = 0; i < m_rows.size(); ++i) {
            if (m_rows[i] == row) {
                idx = i;
                break;
            }
        }
        assert(0 <= idx && idx < static_cast<int>(m_rows.size()));
        if (!MatchesOrContains(this, destination))
            Erase(idx);
    }
}

void ListBox::Render()
{
    // draw beveled rectangle around client area
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt cl_ul = ClientUpperLeft(), cl_lr = ClientLowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, BORDER_THICK);

    int last_visible_row = LastVisibleRow();

    BeginClipping();

    // draw selection hiliting
    int prev_sel = -1;
    int top = cl_ul.y, bottom = 0;
    for (std::set<int>::iterator  it = m_selections.begin(); it != m_selections.end(); ++it) {
        int curr_sel = *it;
        if (curr_sel >= m_first_row_shown && curr_sel <= last_visible_row) {
            // no need to look for the current selection's height, if we just found it on the last iteration
            top = (prev_sel == curr_sel - 1) ? bottom : 0;
            if (!top) {
                for (int i = m_first_row_shown; i < curr_sel; ++i)
                    top += m_rows[i]->Height();
            }
            bottom = top + m_rows[curr_sel]->Height();
            if (bottom > cl_lr.y)
                bottom = cl_lr.y;
            FlatRectangle(cl_ul.x, cl_ul.y + top, cl_lr.x, cl_ul.y + bottom, hilite_color_to_use, CLR_ZERO, 0);
            prev_sel = curr_sel;
        }
    }

    // draw caret
    if (m_caret >= m_first_row_shown && m_caret <= last_visible_row &&
        MatchesOrContains(this, GUI::GetGUI()->FocusWnd())) {
        Pt row_ul = m_rows[m_caret]->UpperLeft();
        Pt row_lr = m_rows[m_caret]->LowerRight();
        row_lr.x = ClientLowerRight().x;
        FlatRectangle(row_ul.x, row_ul.y, row_lr.x, row_lr.y, CLR_ZERO, CLR_SHADOW, 2);
    }

    EndClipping();

    // HACK! This gets around the issue of how to render headers and scrolls, which do not fall within the client area.
    if (!m_header_row->empty()) {
        Rect header_area(Pt(ul.x + BORDER_THICK, m_header_row->UpperLeft().y), Pt(lr.x - BORDER_THICK, m_header_row->LowerRight().y));
        BeginScissorClipping(header_area.ul, header_area.lr);
        GUI::GetGUI()->RenderWindow(m_header_row);
        EndScissorClipping();
    }
    if (m_vscroll)
        GUI::GetGUI()->RenderWindow(m_vscroll);
    if (m_hscroll)
        GUI::GetGUI()->RenderWindow(m_hscroll);

    // ensure that data in occluded cells is not rendered
    for (int i = 0; i < NumRows(); ++i) {
        if (i < m_first_row_shown || last_visible_row < i)
            m_rows[i]->Hide();
        else
            m_rows[i]->Show();
    }
}

void ListBox::KeyPress(Key key, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_SPACE: // space bar (selects item under caret like a mouse click)
            if (m_caret != -1) {
                m_old_sel_row_selected = m_selections.find(m_caret) != m_selections.end();
                ClickAtRow(m_caret, mod_keys);
            }
            break;
        case GGK_DELETE: // delete key
            if (m_style & LIST_USERDELETE) {
                std::set<int>::reverse_iterator r_it;
                if (m_style & LIST_NOSEL) {
                    if (m_caret != -1) {
                        Row* row = Erase(m_caret);
                        delete row;
                    }
                } else {
                    while ((r_it = m_selections.rbegin()) != m_selections.rend()) {
                        Row* row = Erase(*r_it);
                        delete row;
                    }
                    m_selections.clear();
                }
            }
            break;

        // vertical scrolling keys
        case GGK_UP: // arrow-up (not numpad arrow)
            if (m_caret > 0)
                --m_caret;
            break;
        case GGK_DOWN: // arrow-down (not numpad arrow)
            if (m_caret != -1 && m_caret < static_cast<int>(m_rows.size()) - 1)
                ++m_caret;
            break;
        case GGK_PAGEUP: // page up key (not numpad key)
            if (m_caret != -1) {
                int space = ClientSize().y;
                while (m_caret >= 1 && (space -= m_rows[m_caret - 1]->Height()) > 0)
                    --m_caret;
            }
            break;
        case GGK_PAGEDOWN: // page down key (not numpad key)
            if (m_caret != -1) {
                int space = ClientSize().y;
                while (m_caret < static_cast<int>(m_rows.size()) - 1 && (space -= m_rows[m_caret]->Height()) > 0)
                    ++m_caret;
            }
            break;
        case GGK_HOME: // home key (not numpad)
            if (m_caret != -1 && !m_rows.empty())
                m_caret = 0;
            break;
        case GGK_END: // end key (not numpad)
            if (m_caret != -1)
                m_caret = static_cast<int>(m_rows.size()) - 1;
            break;

        // horizontal scrolling keys
        case GGK_LEFT: // left key (not numpad key)
            if (m_first_col_shown) {
                --m_first_col_shown;
                m_hscroll->ScrollTo(std::accumulate(m_col_widths.begin(), m_col_widths.begin() + m_first_col_shown, 0));
            }
            break;
        case GGK_RIGHT:{ // right key (not numpad)
            int last_fully_visible_col = LastVisibleCol();
            if (std::accumulate(m_col_widths.begin(), m_col_widths.begin() + last_fully_visible_col, 0) > ClientSize().x)
                --last_fully_visible_col;
            if (last_fully_visible_col < static_cast<int>(m_col_widths.size()) - 1) {
                ++m_first_col_shown;
                m_hscroll->ScrollTo(std::accumulate(m_col_widths.begin(), m_col_widths.begin() + m_first_col_shown, 0));
            }
            break;}

        // any other key gets passed along to the parent
        default:
            Control::KeyPress(key, mod_keys);
        }

        if (key != GGK_SPACE && key != GGK_DELETE && key != GGK_LEFT && key != GGK_RIGHT)
            BringCaretIntoView();
    } else {
        Control::KeyPress(key, mod_keys);
   }
}

void ListBox::MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys)
{
    if (m_vscroll) {
        for (int i = 0; i < move; ++i) {
            if (0 < m_first_row_shown)
                m_vscroll->ScrollTo(m_vscroll->PosnRange().first - m_rows[m_first_row_shown - 1]->Height());
        }
        for (int i = 0; i < -move; ++i) {
            if (m_first_row_shown < static_cast<int>(m_rows.size() - 1))
                m_vscroll->ScrollTo(m_vscroll->PosnRange().first + m_rows[m_first_row_shown]->Height());
        }
    }
}

void ListBox::DragDropEnter(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys)
{
    ResetAutoScrollVars();
    DragDropHere(pt, drag_drop_wnds, mod_keys);
}

void ListBox::DragDropHere(const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys)
{
    if (!m_rows.empty() && m_auto_scroll_during_drag_drops && InClient(pt)) {
        const Pt MARGIN_OFFSET(m_auto_scroll_margin, m_auto_scroll_margin);
        Rect client_no_scroll_hole(ClientUpperLeft() + MARGIN_OFFSET, ClientLowerRight() - MARGIN_OFFSET);
        m_auto_scrolling_up = pt.y < client_no_scroll_hole.ul.y;
        m_auto_scrolling_down = client_no_scroll_hole.lr.y < pt.y;
        m_auto_scrolling_left = pt.x < client_no_scroll_hole.ul.x;
        m_auto_scrolling_right = client_no_scroll_hole.lr.x < pt.x;
        if (m_auto_scrolling_up || m_auto_scrolling_down || m_auto_scrolling_left || m_auto_scrolling_right) {
            bool acceptible_drop = false;
            for (std::map<Wnd*, GG::Pt>::const_iterator it = drag_drop_wnds.begin(); it != drag_drop_wnds.end(); ++it) {
                if (m_allowed_drop_types.find("") != m_allowed_drop_types.end() ||
                    m_allowed_drop_types.find(it->first->DragDropDataType()) != m_allowed_drop_types.end()) {
                    acceptible_drop = true;
                    break;
                }
            }
            if (acceptible_drop) {
                if (!m_auto_scroll_timer.Running()) {
                    m_auto_scroll_timer.Reset(GUI::GetGUI()->Ticks());
                    m_auto_scroll_timer.Start();
                }
            } else {
                DragDropLeave();
            }
        }
    }
}

void ListBox::DragDropLeave()
{
    ResetAutoScrollVars();
}

void ListBox::TimerFiring(int ticks, Timer* timer)
{
    if (timer == &m_auto_scroll_timer && !m_rows.empty()) {
        if (m_vscroll) {
            if (m_auto_scrolling_up && 0 < m_first_row_shown)
                m_vscroll->ScrollTo(m_vscroll->PosnRange().first - m_rows[m_first_row_shown - 1]->Height());
            if (m_auto_scrolling_down) {
                int last_visible_row = LastVisibleRow();
                if (last_visible_row < static_cast<int>(m_rows.size() - 1) ||
                    ClientLowerRight().y < m_rows[last_visible_row]->LowerRight().y)
                    m_vscroll->ScrollTo(m_vscroll->PosnRange().first + m_rows[m_first_row_shown]->Height());
            }
        }
        if (m_hscroll) {
            if (m_auto_scrolling_left && 0 < m_first_col_shown)
                m_hscroll->ScrollTo(m_hscroll->PosnRange().first - m_col_widths[m_first_col_shown - 1]);
            if (m_auto_scrolling_right) {
                int last_visible_col = LastVisibleCol();
                if (last_visible_col < static_cast<int>(m_col_widths.size() - 1) ||
                    ClientLowerRight().x < m_rows.front()->LowerRight().x)
                    m_hscroll->ScrollTo(m_hscroll->PosnRange().first + m_col_widths[m_first_col_shown]);
            }
        }
    }
}

void ListBox::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    AdjustScrolls(true);
}

void ListBox::Disable(bool b/* = true*/)
{
    Control::Disable(b);
    if (m_vscroll)
        m_vscroll->Disable(b);
    if (m_hscroll)
        m_hscroll->Disable(b);
}

void ListBox::SetColor(Clr c)
{
    Control::SetColor(c);
    if (m_vscroll)
        m_vscroll->SetColor(c);
    if (m_hscroll)
        m_hscroll->SetColor(c);
}

int ListBox::Insert(Row* row, int at/*= -1*/)
{
    return Insert(row, at, false);
}

ListBox::Row* ListBox::Erase(int idx)
{
    return Erase(idx, false);
}

void ListBox::Clear()
{
    bool signal = !m_rows.empty(); // inhibit signal if the list box was already empty
    m_rows.clear();
    m_caret = -1;
    DetachChild(m_header_row);
    DeleteChildren();
    AttachChild(m_header_row);
    m_vscroll = 0;
    m_hscroll = 0;
    m_first_row_shown = m_first_col_shown = 0;
    m_selections.clear();
    m_old_sel_row = -1;
    m_lclick_row = -1;

    if (!m_keep_col_widths) { // remove column widths and alignments, if needed
        m_col_widths.clear();
        m_col_alignments.clear();
    }

    AdjustScrolls(false);

    if (signal)
        ClearedSignal();
}

void ListBox::SelectRow(int n)
{
    if (!(m_style & LIST_NOSEL) && n >= 0 && n < static_cast<int>(m_rows.size()) && m_selections.find(n) == m_selections.end()) {
        bool emit_signal = m_selections.find(n) == m_selections.end();
        if (m_style & LIST_SINGLESEL)
            m_selections.clear();
        m_selections.insert(n);
        if (emit_signal)
            SelChangedSignal(m_selections);
    }
}

void ListBox::DeselectRow(int n)
{
    if (n >= 0 && n < static_cast<int>(m_rows.size()) && m_selections.find(n) != m_selections.end()) {
        m_selections.erase(n);
        SelChangedSignal(m_selections);
    }
}

void ListBox::SelectAll()
{
    bool emit_signal = false;
    if (m_selections.size() < m_rows.size())
        emit_signal = true;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        m_selections.insert(i);
    }
    if (emit_signal)
        SelChangedSignal(m_selections);
}

void ListBox::DeselectAll()
{
    bool emit_signal = false;
    if (!m_selections.empty())
        emit_signal = true;
    m_selections.clear();
    m_caret = -1;
    if (emit_signal)
        SelChangedSignal(m_selections);
}

ListBox::Row& ListBox::GetRow(int n)
{
    return *m_rows.at(n);
}

void ListBox::SetSelections(const std::set<int>& s)
{
    m_selections = s;
}

void ListBox::SetCaret(int idx)
{
    m_rows.at(idx);
    m_caret = idx;
}

void ListBox::BringRowIntoView(int n)
{
    if (0 <= n && n < static_cast<int>(m_rows.size())) {
        if (0 <= n && n < m_first_row_shown) {
            m_first_row_shown = n;
        } else if (LastVisibleRow() <= n) { // find the row that preceeds the target row by about ClientSize().y pixels, and make it the first row shown
            m_first_row_shown = FirstRowShownWhenBottomIs(n, ClientHeight());
        }
        if (m_vscroll) {
            int acc = 0;
            for (int i = 0; i < m_first_row_shown; ++i)
                acc += m_rows[i]->Height();
            m_vscroll->ScrollTo(acc);
        }
    }
}

void ListBox::SetInteriorColor(Clr c)
{
    m_int_color = c;
}

void ListBox::SetHiliteColor(Clr c)
{
    m_hilite_color = c;
}

void ListBox::SetStyle(Flags<ListBoxStyle> s)
{
    Flags<ListBoxStyle> old_style = m_style;
    m_style = s;
    ValidateStyle();

    // if we're going from an unsorted style to a sorted one, do the sorting now
    if (old_style & LIST_NOSORT) {
        if (!(s & LIST_NOSORT))
            Resort();
    // if we're changing the sorting order of a sorted list, reverse the contents
    } else if (static_cast<bool>(old_style & LIST_SORTDESCENDING) != static_cast<bool>(s & LIST_SORTDESCENDING)) {
        Resort();
    }
}

void ListBox::SetColHeaders(Row* r)
{
    int client_height = ClientHeight();
    delete m_header_row;
    if (r) {
        m_header_row = r;
        // if this column header is being added to an empty listbox, the listbox takes on some of the
        // attributes of the header, similar to the insertion of a row into an empty listbox; see Insert()
        if (m_rows.empty() && m_col_widths.empty()) {
            m_col_widths.resize(m_header_row->size(), (ClientSize().x - SCROLL_WIDTH) / m_header_row->size());
            // put the remainder in the last column, so the total width == ClientSize().x - SCROLL_WIDTH
            m_col_widths.back() += (ClientSize().x - SCROLL_WIDTH) % m_header_row->size();
            Alignment alignment = ALIGN_NONE;
            if (m_style & LIST_LEFT)
                alignment = ALIGN_LEFT;
            if (m_style & LIST_CENTER)
                alignment = ALIGN_CENTER;
            if (m_style & LIST_RIGHT)
                alignment = ALIGN_RIGHT;
            m_col_alignments.resize(m_header_row->size(), alignment);
        }
        NormalizeRow(m_header_row);
        m_header_row->MoveTo(Pt(0, -m_header_row->Height()));
        AttachChild(m_header_row);
    } else {
        m_header_row = new Row();
    }
    if (client_height != ClientHeight())
        AdjustScrolls(true);
}

void ListBox::RemoveColHeaders()
{
    SetColHeaders(0);
}

void ListBox::SetNumCols(int n)
{
    if (n > 0) {
        if (m_col_widths.size()) {
            m_col_widths.resize(n);
            m_col_alignments.resize(n);
        } else {
            m_col_widths.resize(n, ClientSize().x / n);
            m_col_widths.back() += ClientSize().x % n;
            Alignment alignment = ALIGN_NONE;
            if (m_style & LIST_LEFT)
                alignment = ALIGN_LEFT;
            if (m_style & LIST_CENTER)
                alignment = ALIGN_CENTER;
            if (m_style & LIST_RIGHT)
                alignment = ALIGN_RIGHT;
            m_col_alignments.resize(n, alignment);
        }
        if (m_sort_col <= n)
            m_sort_col = 0;
        for (unsigned int i = 0; i < m_rows.size(); ++i) {
            NormalizeRow(m_rows[i]);
        }
    }
    AdjustScrolls(false);
}

void ListBox::SetColWidth(int n, int w)
{
    m_col_widths[n] = w;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        m_rows[i]->SetColWidth(n, w);
    }
    AdjustScrolls(false);
}

void ListBox::SetSortCol(int n)
{
    bool needs_resort = m_sort_col != n && !(m_style & LIST_NOSORT);
    m_sort_col = n;
    if (needs_resort)
        Resort();
}

void ListBox::SetSortCmp(const boost::function<bool (const Row&, const Row&, int)>& sort_cmp)
{
    m_sort_cmp = sort_cmp;
    Resort();
}

void ListBox::LockColWidths()
{
    m_keep_col_widths = true;
}

void ListBox::UnLockColWidths()
{
    m_keep_col_widths = false;
}

void ListBox::SetColAlignment(int n, Alignment align)
{
    m_col_alignments[n] = align;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        m_rows[i]->SetColAlignment(n, align);
    }
}

void ListBox::SetRowAlignment(int n, Alignment align)
{
    m_rows[n]->SetRowAlignment(align);
}

void ListBox::AllowDropType(const std::string& str)
{
    m_allowed_drop_types.insert(str);
}

void ListBox::DisallowDropType(const std::string& str)
{
    m_allowed_drop_types.erase(str);
}

void ListBox::AutoScrollDuringDragDrops(bool auto_scroll)
{
    m_auto_scroll_during_drag_drops = auto_scroll;
}

void ListBox::SetAutoScrollMargin(int margin)
{
    m_auto_scroll_margin = margin;
}

void ListBox::SetAutoScrollInterval(int interval)
{
    m_auto_scroll_timer.SetInterval(interval);
}

void ListBox::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("ListBox");
    // TODO: handle specifying column widths and alignments
    editor->Attribute("Cell Margin", m_cell_margin);
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Highlighting Color", m_hilite_color);
    boost::shared_ptr<SetStyleAction> set_style_action(new SetStyleAction(this));
    editor->BeginFlags<ListBoxStyle>(m_style, set_style_action);
    typedef std::vector<ListBoxStyle> FlagVec;
    using boost::assign::list_of;
    editor->FlagGroup("V. Alignment", FlagVec() = list_of(LIST_TOP)(LIST_VCENTER)(LIST_BOTTOM));
    editor->FlagGroup("H. Alignment", FlagVec() = list_of(LIST_LEFT)(LIST_CENTER)(LIST_RIGHT));
    editor->Flag("Do not Sort", LIST_NOSORT);
    editor->Flag("Sort Descending", LIST_SORTDESCENDING);
    editor->Flag("No Selections", LIST_NOSEL);
    editor->Flag("Single-Selection", LIST_SINGLESEL);
    editor->Flag("Quick-Selection", LIST_QUICKSEL);
    editor->Flag("User Deletions", LIST_USERDELETE);
    editor->Flag("Browse Updates", LIST_BROWSEUPDATES);
    editor->EndFlags();
    // TODO: handle setting header row
    editor->Attribute("Keep Column Widths", m_keep_col_widths);
    editor->Attribute("Clip Cells", m_clip_cells);
    boost::shared_ptr<SetSortColAction> set_sort_col_action(new SetSortColAction(this));
    editor->Attribute<int>("Sort Column", m_sort_col,
                           0, static_cast<int>(m_col_widths.size()),
                           set_sort_col_action);
    // TODO: handle specifying allowed drop types
}

int ListBox::RightMargin() const
{
    return (m_vscroll ? SCROLL_WIDTH : 0);
}

int ListBox::BottomMargin() const
{
    return (m_hscroll ? SCROLL_WIDTH : 0);
}

int ListBox::RowUnderPt(const Pt& pt) const
{
    int retval;
    int acc = ClientUpperLeft().y;
    for (retval = m_first_row_shown; retval < static_cast<int>(m_rows.size()); ++retval) {
        acc += m_rows[retval]->Height();
        if (pt.y <= acc) break;
    }
    return retval;
}

int ListBox::OldSelRow() const
{
    return m_old_sel_row;
}

int ListBox::OldRDownRow() const
{
    return m_old_rdown_row;
}

int ListBox::LClickRow() const
{
    return m_lclick_row;
}

int ListBox::RClickRow() const
{
    return m_rclick_row;
}

bool ListBox::AutoScrollingUp() const
{
    return m_auto_scrolling_up;
}

bool ListBox::AutoScrollingDown() const
{
    return m_auto_scrolling_down;
}

bool ListBox::AutoScrollingLeft() const
{
    return m_auto_scrolling_left;
}

bool ListBox::AutoScrollingRight() const
{
    return m_auto_scrolling_right;
}

int ListBox::VerticalScrollPadding(int client_height_without_horizontal_scroll)
{
    int first_row_shown_when_caret_at_bottom =
        FirstRowShownWhenBottomIs(m_rows.size() - 1, client_height_without_horizontal_scroll);
    int height_of_rows_visible_at_bottom = 0;
    for (unsigned int i = first_row_shown_when_caret_at_bottom; i < m_rows.size(); ++i) {
        height_of_rows_visible_at_bottom += m_rows[i]->Height();
    }
    return client_height_without_horizontal_scroll - height_of_rows_visible_at_bottom;
}

int ListBox::HorizontalScrollPadding(int client_width_without_vertical_scroll)
{
    int first_col_shown_when_caret_at_right =
        FirstColShownWhenRightIs(m_col_widths.size() - 1, client_width_without_vertical_scroll);
    int width_of_cols_visible_at_right = 0;
    for (unsigned int i = first_col_shown_when_caret_at_right; i < m_col_widths.size(); ++i) {
        width_of_cols_visible_at_right += m_col_widths[i];
    }
    return client_width_without_vertical_scroll - width_of_cols_visible_at_right;
}

bool ListBox::EventFilter(Wnd* w, const WndEvent& event)
{
    assert(dynamic_cast<Row*>(w));

    if (!Disabled()) {
        Pt pt = event.Point();
        Flags<ModKey> mod_keys = event.ModKeys();

        switch (event.Type()) {
        case WndEvent::LButtonDown: {
            m_old_sel_row = RowUnderPt(pt);
            if (m_old_sel_row >= static_cast<int>(m_rows.size()) || !InClient(pt)) {
                m_old_sel_row = -1;
            } else {
                m_old_sel_row_selected = m_selections.find(m_old_sel_row) != m_selections.end();
                if (!(m_style & LIST_NOSEL) && !m_old_sel_row_selected)
                    ClickAtRow(m_old_sel_row, mod_keys);
            }
            break;
        }

        case WndEvent::LButtonUp: {
            m_old_sel_row = -1;
            break;
        }

        case WndEvent::LClick: {
            if (m_old_sel_row >= 0 && InClient(pt)) {
                int sel_row = RowUnderPt(pt);
                if (sel_row == m_old_sel_row) {
                    if (!(m_style & LIST_NOSEL))
                        ClickAtRow(sel_row, mod_keys);
                    else
                        m_caret = sel_row;
                    m_lclick_row = sel_row;
                    LeftClickedSignal(sel_row, m_rows[sel_row], pt);
                }
            }
            break;
        }

        case WndEvent::LDoubleClick: {
            int row = RowUnderPt(pt);
            if (row >= 0 && row == m_lclick_row && InClient(pt)) {
                DoubleClickedSignal(row, m_rows[row]);
                m_old_sel_row = -1;
            } else {
                LClick(pt, mod_keys);
            }
            break;
        }

        case WndEvent::RButtonDown: {
            int row = RowUnderPt(pt);
            if (row < static_cast<int>(m_rows.size()) && InClient(pt)) {
                m_old_rdown_row = row;
            } else {
                m_old_rdown_row = -1;
            }
            break;
        }

        case WndEvent::RClick: {
            int row = RowUnderPt(pt);
            if (row >= 0 && row == m_old_rdown_row && InClient(pt)) {
                m_rclick_row = row;
                RightClickedSignal(row, m_rows[row], pt);
            }
            m_old_rdown_row = -1;
            break;
        }

        case WndEvent::MouseEnter: {
            if (m_style & LIST_BROWSEUPDATES) {
                int sel_row = RowUnderPt(pt);
                if (sel_row >= static_cast<int>(m_rows.size()))
                    sel_row = -1;
                if (m_last_row_browsed != sel_row)
                    BrowsedSignal(m_last_row_browsed = sel_row);
            }
            break;
        }

        case WndEvent::MouseHere:
            break;

        case WndEvent::MouseLeave: {
            if (m_style & LIST_BROWSEUPDATES) {
                if (m_last_row_browsed != -1)
                    BrowsedSignal(m_last_row_browsed = -1);
            }
            break;
        }

        case WndEvent::GainingFocus: {
            GUI::GetGUI()->SetFocusWnd(this);
            break;
        }

        case WndEvent::MouseWheel:
            return false;

        case WndEvent::DragDropEnter:
        case WndEvent::DragDropHere:
        case WndEvent::DragDropLeave:
            HandleEvent(event);
            break;

        case WndEvent::KeyPress:
        case WndEvent::KeyRelease:
        case WndEvent::TimerFiring:
            return false;

        default:
            break;
        }
    }
    return true;
}

int ListBox::Insert(Row* row, int at, bool dropped)
{
    // track the originating row if this is an intra-ListBox drag-and-drop
    int dropped_row_original_index = -1;
    if (dropped) {
        for (unsigned int i = 0; i < m_rows.size(); ++i) {
            if (m_rows[i] == row) {
                dropped_row_original_index = i;
                break;
            }
        }
    }

    int retval = at;

    // the first row inserted into an empty list box defines the number of columns, and initializes the column widths
    if (m_rows.empty() && (m_col_widths.empty() || !m_keep_col_widths)) {
        m_col_widths.resize(row->size(), (ClientSize().x - SCROLL_WIDTH) / row->size());
        // put the remainder in the last column, so the total width == ClientSize().x - SCROLL_WIDTH
        m_col_widths.back() += (ClientSize().x - SCROLL_WIDTH) % row->size();
        Alignment alignment = ALIGN_NONE;
        if (m_style & LIST_LEFT)
            alignment = ALIGN_LEFT;
        if (m_style & LIST_CENTER)
            alignment = ALIGN_CENTER;
        if (m_style & LIST_RIGHT)
            alignment = ALIGN_RIGHT;
        m_col_alignments.resize(row->size(), alignment);
        if (!m_header_row->empty())
            NormalizeRow(m_header_row);
    }

    row->InstallEventFilter(this);
    NormalizeRow(row);

    Pt insertion_pt;
    if (m_rows.empty()) {
        retval = 0;
        m_rows.push_back(row);
    } else {
        if (m_style & LIST_NOSORT) {
            if (at < 0 || at > static_cast<int>(m_rows.size())) // if at is out of bounds, insert item at the end
                at = m_rows.size();
            retval = at;
        } else {
            retval = 0;
            RowSorter less_than(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING);
            while (retval < static_cast<int>(m_rows.size()) && !less_than(row, m_rows[retval]))
                ++retval;
        }
        std::vector<Row*>::iterator insertion_it = m_rows.begin() + retval;
        int y = insertion_it == m_rows.end() ?
            m_rows.back()->RelativeLowerRight().y : (*insertion_it)->RelativeUpperLeft().y;
        insertion_pt = Pt(0, y);
        m_rows.insert(insertion_it, row);
    }
    int row_height = row->Height();

    // adjust any affected integer indices
    IncrementIfGE(m_caret, retval);
    IncrementIfGE(dropped_row_original_index, retval);
    IncrementIfGE(m_old_sel_row, retval);
    IncrementIfGE(m_old_rdown_row, retval);
    IncrementIfGE(m_lclick_row, retval);
    IncrementIfGE(m_rclick_row, retval);
    IncrementIfGE(m_last_row_browsed, retval);

    // "bump" the positions of, and selections on, lower rows down one row
    for (int i = static_cast<int>(m_rows.size() - 1); i > retval; --i) {
        m_rows[i]->OffsetMove(Pt(0, row_height));
        if (m_selections.find(i - 1) != m_selections.end()) {
            m_selections.insert(i);
            m_selections.erase(i - 1);
        }
    }

    Pt original_ul = row->RelativeUpperLeft();
    Wnd* original_parent = row->Parent();
    AttachChild(row);
    row->MoveTo(insertion_pt);

    AdjustScrolls(false);

    if (dropped) {
        // ensure that no one has a problem with this drop in user space (if so, they should throw DontAcceptDrop)
        try {
            DroppedSignal(retval, row);
            if (0 <= dropped_row_original_index && dropped_row_original_index < static_cast<int>(m_rows.size()))
                Erase(dropped_row_original_index, true);
        } catch (const DontAcceptDrop&) {
            // if there is a problem, silently undo the drop
            if (dropped_row_original_index == -1) {
                m_suppress_erase_signal = true;
                Erase(retval);
                m_suppress_erase_signal = false;
            } else {
                Erase(retval, true);
            }
            row->MoveTo(original_ul);
            if (original_parent)
                original_parent->AttachChild(row);
            else
                DetachChild(row);
            throw; // re-throw so that AcceptDrop() knows to return false
        }
    } else {
        InsertedSignal(retval, row);
    }

    return retval;
}

ListBox::Row* ListBox::Erase(int idx, bool removing_duplicate)
{
    if (0 <= idx && idx < static_cast<int>(m_rows.size())) { // remove row
        Row* row = m_rows[idx];
        int row_height = row->Height();
        m_rows.erase(m_rows.begin() + idx);
        if (!removing_duplicate) {
            DetachChild(row);
            row->RemoveEventFilter(this);
        }

        // "bump" all the hiliting and positions up one row
        m_selections.erase(idx);
        for (unsigned int i = idx; i < m_rows.size(); ++i) {
            m_rows[i]->OffsetMove(Pt(0, -row_height));
            if (m_selections.find(i + 1) != m_selections.end()) {
                m_selections.insert(i);
                m_selections.erase(i + 1);
            }
        }

        // adjust any affected integer indices
        DecrementIfGE(m_caret, idx);
        DecrementIfGE(m_old_sel_row, idx);
        DecrementIfGE(m_old_rdown_row, idx);
        DecrementIfGE(m_lclick_row, idx);
        DecrementIfGE(m_rclick_row, idx);
        DecrementIfGE(m_last_row_browsed, idx);

        AdjustScrolls(false);

        if (!removing_duplicate && !m_suppress_erase_signal)
            ErasedSignal(idx, row);

        return row;
    } else {
        return 0;
    }
}

void ListBox::BringCaretIntoView()
{
    BringRowIntoView(m_caret);
}

void ListBox::RecreateScrolls()
{
    delete m_vscroll;
    delete m_hscroll;
    m_vscroll = m_hscroll = 0;
    AdjustScrolls(false);
}

void ListBox::ResetAutoScrollVars()
{
    m_auto_scrolling_up = false;
    m_auto_scrolling_down = false;
    m_auto_scrolling_left = false;
    m_auto_scrolling_right = false;
    m_auto_scroll_timer.Stop();
}

void ListBox::Resort()
{
    Row* caret = GetPtrAtIndex(m_caret, m_rows);
    std::set<Row*> selections;
    for (std::set<int>::const_iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
        selections.insert(m_rows[*it]);
    }
    m_selections.clear();
    Row* old_sel_row = GetPtrAtIndex(m_old_sel_row, m_rows);
    Row* old_rdown_row = GetPtrAtIndex(m_old_rdown_row, m_rows);
    Row* lclick_row = GetPtrAtIndex(m_lclick_row, m_rows);
    Row* rclick_row = GetPtrAtIndex(m_rclick_row, m_rows);
    Row* last_row_browsed = GetPtrAtIndex(m_last_row_browsed, m_rows);

    std::stable_sort(m_rows.begin(), m_rows.end(),
                     RowSorter(m_sort_cmp, m_sort_col, m_style & LIST_SORTDESCENDING));

    int y = 0;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        Row* row = m_rows[i];
        if (caret == row)
            m_caret = i;
        if (selections.find(row) != selections.end())
            m_selections.insert(i);
        if (old_sel_row == row)
            m_old_sel_row = i;
        if (old_rdown_row == row)
            m_old_rdown_row = i;
        if (lclick_row == row)
            m_lclick_row = i;
        if (rclick_row == row)
            m_rclick_row = i;
        if (last_row_browsed == row)
            m_last_row_browsed = i;

        row->MoveTo(Pt(0, y));
        y += row->Height();
    }

    m_first_row_shown = 0;
}

void ListBox::ConnectSignals()
{
    if (m_vscroll)
        Connect(m_vscroll->ScrolledSignal, &ListBox::VScrolled, this);
    if (m_hscroll)
        Connect(m_hscroll->ScrolledSignal, &ListBox::HScrolled, this);
}

void ListBox::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & LIST_LEFT) ++dup_ct;
    if (m_style & LIST_RIGHT) ++dup_ct;
    if (m_style & LIST_CENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LIST_LEFT by default
        m_style &= ~(LIST_RIGHT | LIST_CENTER);
        m_style |= LIST_LEFT;
    }
    dup_ct = 0;
    if (m_style & LIST_TOP) ++dup_ct;
    if (m_style & LIST_BOTTOM) ++dup_ct;
    if (m_style & LIST_VCENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LIST_VCENTER by default
        m_style &= ~(LIST_TOP | LIST_BOTTOM);
        m_style |= LIST_VCENTER;
    }
    dup_ct = 0;
    if (m_style & LIST_NOSEL) ++dup_ct;
    if (m_style & LIST_SINGLESEL) ++dup_ct;
    if (m_style & LIST_QUICKSEL) ++dup_ct;
    if (dup_ct > 1)  // at most one of these may be picked; when multiples are picked, disable all of them
        m_style &= ~(LIST_NOSEL | LIST_SINGLESEL | LIST_QUICKSEL);
}

void ListBox::AdjustScrolls(bool adjust_for_resize)
{
    // this client area calculation disregards the thickness of scrolls
    Pt cl_sz = (LowerRight() - Pt(BORDER_THICK, BORDER_THICK)) -
        (UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->empty() ? 0 : m_header_row->Height())));

    int total_x_extent = std::accumulate(m_col_widths.begin(), m_col_widths.end(), 0);
    int total_y_extent = 0;
    if (!m_rows.empty())
        total_y_extent = m_rows.back()->LowerRight().y - m_rows.front()->UpperLeft().y;

    bool vertical_needed =
        m_rows.size() && (total_y_extent > cl_sz.y ||
                          (total_y_extent > cl_sz.y - SCROLL_WIDTH &&
                           total_x_extent > cl_sz.x - SCROLL_WIDTH));
    bool horizontal_needed =
        m_rows.size() && (total_x_extent > cl_sz.x ||
                          (total_x_extent > cl_sz.x - SCROLL_WIDTH &&
                           total_y_extent > cl_sz.y - SCROLL_WIDTH));

    boost::shared_ptr<StyleFactory> style = GetStyleFactory();

    if (m_vscroll) { // if scroll already exists...
        if (!vertical_needed) { // remove scroll
            DeleteChild(m_vscroll);
            m_vscroll = 0;
        } else { // ensure vertical scroll has the right logical and physical dimensions
            int scroll_x = cl_sz.x - SCROLL_WIDTH;
            int scroll_y = 0;
            if (adjust_for_resize)
                m_vscroll->SizeMove(Pt(scroll_x, scroll_y), Pt(scroll_x + SCROLL_WIDTH, scroll_y + cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0)));
            total_y_extent += VerticalScrollPadding(cl_sz.y);
            m_vscroll->SizeScroll(0, total_y_extent - 1, cl_sz.y / 8, cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0));
            MoveChildUp(m_vscroll);
        }
    } else if (!m_vscroll && vertical_needed) { // if scroll doesn't exist but is needed
        m_vscroll = style->NewListBoxVScroll(cl_sz.x - SCROLL_WIDTH, 0, SCROLL_WIDTH, cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0), m_color, CLR_SHADOW);
        total_y_extent += VerticalScrollPadding(cl_sz.y);
        m_vscroll->SizeScroll(0, total_y_extent - 1, cl_sz.y / 8, cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0));
        AttachChild(m_vscroll);
        Connect(m_vscroll->ScrolledSignal, &ListBox::VScrolled, this);
    }

    if (m_hscroll) { // if scroll already exists...
        if (!horizontal_needed) { // remove scroll
            DeleteChild(m_hscroll);
            m_hscroll = 0;
        } else { // ensure horizontal scroll has the right logical and physical dimensions
            int scroll_x = 0;
            int scroll_y = cl_sz.y - SCROLL_WIDTH;
            if (adjust_for_resize)
                m_hscroll->SizeMove(Pt(scroll_x, scroll_y), Pt(scroll_x + cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0), scroll_y + SCROLL_WIDTH));
            total_x_extent += HorizontalScrollPadding(cl_sz.x);
            m_hscroll->SizeScroll(0, total_x_extent - 1, cl_sz.x / 8, cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0));
            MoveChildUp(m_hscroll);
        }
    } else if (!m_hscroll && horizontal_needed) { // if scroll doesn't exist but is needed
        m_hscroll = style->NewListBoxHScroll(0, cl_sz.y - SCROLL_WIDTH, cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0), SCROLL_WIDTH, m_color, CLR_SHADOW);
        total_x_extent += HorizontalScrollPadding(cl_sz.x);
        m_hscroll->SizeScroll(0, total_x_extent - 1, cl_sz.x / 8, cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0));
        AttachChild(m_hscroll);
        Connect(m_hscroll->ScrolledSignal, &ListBox::HScrolled, this);
    }

    assert(!m_vscroll || m_vscroll->PageSize() == ClientHeight());
    assert(!m_hscroll || m_hscroll->PageSize() == ClientWidth());
}

void ListBox::VScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_row_shown = 0;
    int accum = 0;
    int position = 0;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        int row_height = m_rows[i]->Height();
        if (tab_low < accum + row_height / 2) {
            m_first_row_shown = i;
            position = -accum;
            break;
        }
        accum += row_height;
    }
    int initial_x = m_rows.empty() ? 0 : m_rows[0]->RelativeUpperLeft().x;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        m_rows[i]->MoveTo(Pt(initial_x, position));
        position += m_rows[i]->Height();
    }
}

void ListBox::HScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_col_shown = 0;
    int accum = 0;
    int position = 0;
    for (unsigned int i = 0; i < m_col_widths.size(); ++i) {
        int col_width = m_col_widths[i];
        if (tab_low < accum + col_width / 2) {
            m_first_col_shown = i;
            position = -accum;
            break;
        }
        accum += col_width;
    }
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        m_rows[i]->MoveTo(Pt(position, m_rows[i]->RelativeUpperLeft().y));
    }
    m_header_row->MoveTo(Pt(position, m_header_row->RelativeUpperLeft().y));
}

void ListBox::ClickAtRow(int row, Flags<ModKey> mod_keys)
{
    std::set<int> previous_selections = m_selections;
    if (m_style & LIST_SINGLESEL) { // no special keys are being used; just clear all previous selections, select this row, set the caret here
        m_selections.clear();
        m_selections.insert(row);
        m_caret = row;
    } else {
        if (mod_keys & MOD_KEY_CTRL) { // control key depressed
            if (mod_keys & MOD_KEY_SHIFT && m_caret != -1) { // both shift and control keys depressed
                int low  = m_caret < row ? m_caret : row;
                int high = m_caret < row ? row : m_caret;
                bool erase = m_selections.find(m_caret) == m_selections.end();
                for (int i = low; i <= high; ++i) {
                    if (erase)
                        m_selections.erase(i);
                    else
                        m_selections.insert(i);
                }
            } else { // just the control key is depressed: toggle the item selected, adjust the caret
                if (m_old_sel_row_selected)
                    m_selections.erase(row);
                else
                    m_selections.insert(row);
                m_caret = row;
            }
        } else if (mod_keys & MOD_KEY_SHIFT) { // shift key depressed
            bool erase = m_selections.find(m_caret) == m_selections.end();
            if (!(m_style & LIST_QUICKSEL))
                m_selections.clear();
            if (m_caret == -1) { // no previous caret exists; select this row, mark it as the caret
                m_selections.insert(row);
                m_caret = row;
            } else { // select all rows between the caret and this row (inclusive), don't move the caret
                int low  = m_caret < row ? m_caret : row;
                int high = m_caret < row ? row : m_caret;
                for (int i = low; i <= high; ++i) {
                    if (erase)
                        m_selections.erase(i);
                    else
                        m_selections.insert(i);
                }
            }
        } else { // unless LIST_QUICKSEL is used, this is treated just like LIST_SINGLESEL above
            if (m_style & LIST_QUICKSEL) {
                if (m_old_sel_row_selected)
                    m_selections.erase(row);
                else
                    m_selections.insert(row);
                m_caret = row;
            } else {
                m_selections.clear();
                m_selections.insert(row);
                m_caret = row;
            }
        }
    }
    if (previous_selections != m_selections)
        SelChangedSignal(m_selections);
}

void ListBox::NormalizeRow(Row* row)
{
    row->resize(m_col_widths.size());
    row->SetColWidths(m_col_widths);
    row->SetColAlignments(m_col_alignments);
    row->SetMargin(m_cell_margin);
    row->Resize(Pt(std::accumulate(m_col_widths.begin(), m_col_widths.end(), 0), row->Height()));
}

int ListBox::FirstRowShownWhenBottomIs(int bottom_row, int client_height)
{
    if (bottom_row < 0)
        return 0;
    int available_space = client_height - m_rows[bottom_row]->Height();
    int i = bottom_row;
    while (0 < i && m_rows[i - 1]->Height() <= available_space) {
        available_space -= m_rows[--i]->Height();
    }
    return i;
}

int ListBox::FirstColShownWhenRightIs(int right_col, int client_width)
{
    if (right_col < 0)
        return 0;
    int available_space = client_width - m_col_widths[right_col];
    int i = right_col;
    while (0 < i && m_col_widths[i - 1] <= available_space) {
        available_space -= m_col_widths[--i];
    }
    return i;
}
