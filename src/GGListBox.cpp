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

#include "GGListBox.h"
#include "GGApp.h"
#include "GGScroll.h"
#include "GGTextControl.h"
#include "GGStaticGraphic.h"
#include "GGDrawUtil.h"

namespace GG {

namespace {
const int SCROLL_WIDTH = 14;

class RowSorter // used to sort rows by a certain column (which may contain some empty cells)
{
public:
    RowSorter(int col, bool less) : sort_col(col), return_less(less) {}

    bool operator()(const ListBox::Row* lr, const ListBox::Row* rr)
    {
        return return_less ? lr->SortKey(sort_col) < rr->SortKey(sort_col) : lr->SortKey(sort_col) > rr->SortKey(sort_col);
    }

private:
    const int  sort_col;
    const bool return_less;
};
}

////////////////////////////////////////////////
// GG::ListBox::Row
////////////////////////////////////////////////
ListBox::Row::Row(const XMLElement& elem)
{
    if (elem.Tag() != "GG::ListBox::Row")
        throw std::invalid_argument("Attempted to construct a GG::ListBox::Row from an XMLElement that had a tag other than \"GG::ListBox::Row\"");

    const XMLElement* curr_elem = &elem.Child("data_type");
    data_type = curr_elem->Text();

    curr_elem = &elem.Child("height");
    height = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("alignment");
    alignment = lexical_cast<Uint32>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("indentation");
    indentation = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("cells");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        if (curr_elem->Child(i).Tag() == "GG::Control" && !curr_elem->Child(i).NumChildren())
            push_back(0);
        else
            push_back(dynamic_cast<Control*>(GG::App::GetApp()->GenerateWnd(curr_elem->Child(i))));
    }

    curr_elem = &elem.Child("sub_rows");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        sub_rows.push_back(new Row(curr_elem->Child(i)));
    }
}

ListBox::Row::~Row()
{
    // note that this does not clean up the contents of the row (the Controls); these will be freed as children of the ListBox
    for (unsigned int i = 0; i < sub_rows.size(); ++i)
        delete sub_rows[i];
}

int ListBox::Row::Height() const
{
    int retval = height;
    if (!sub_rows.empty())
        for (unsigned int i = 0; i < sub_rows.size(); ++i)
            retval += sub_rows[i]->height;
    return retval;
}

const string& ListBox::Row::SortKey(int column) const
{
    return at(column)->WindowText();
}

XMLElement ListBox::Row::XMLEncode() const
{
    XMLElement retval("GG::ListBox::Row");

    XMLElement temp;

    temp = XMLElement("data_type", data_type);
    retval.AppendChild(temp);

    temp = XMLElement("height");
    temp.SetAttribute("value", lexical_cast<string>(height));
    retval.AppendChild(temp);

    temp = XMLElement("alignment");
    temp.SetAttribute("value", lexical_cast<string>(alignment));
    retval.AppendChild(temp);

    temp = XMLElement("indentation");
    temp.SetAttribute("value", lexical_cast<string>(indentation));
    retval.AppendChild(temp);

    temp = XMLElement("cells");
    for (unsigned int i = 0; i < size(); ++i) {
        if (operator[](i))
            temp.AppendChild(operator[](i)->XMLEncode());
        else
            temp.AppendChild(XMLElement("GG::Control"));
    }
    retval.AppendChild(temp);

    temp = XMLElement("sub_rows");
    for (std::vector<Row*>::const_iterator it = sub_rows.begin(); it != sub_rows.end(); ++it) {
        temp.AppendChild((*it)->XMLEncode());
    }
    retval.AppendChild(temp);

    return retval;
}

void ListBox::Row::push_back(const string& str, const shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/)
{
    push_back(CreateControl(str, font, color));
}

void ListBox::Row::push_back(const string& str, const string& font_filename, int pts, Clr color/* = CLR_BLACK*/)
{
    push_back(CreateControl(str, App::GetApp()->GetFont(font_filename, pts), color));
}

void ListBox::Row::push_back(const SubTexture& st)
{
    push_back(CreateControl(st));
}

Control* ListBox::Row::CreateControl(const string& str, const shared_ptr<Font>& font, Clr color)
{
    return new TextControl(0, 0, str, font, color);
}

Control* ListBox::Row::CreateControl(const SubTexture& st)
{
    return new StaticGraphic(0, 0, st.Width(), st.Height(), st, SG_SHRINKFIT);
}

////////////////////////////////////////////////
// GG::ListBox
////////////////////////////////////////////////
// static(s)
const int ListBox::BORDER_THICK = 2;

ListBox::ListBox(int x, int y, int w, int h, Clr color, Clr interior/* = CLR_ZERO*/,
                 Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        Control(x, y, w, h, flags),
        m_vscroll(0),
        m_hscroll(0),
        m_caret(-1),
        m_old_sel_row(-1),
        m_old_rdown_row(-1),
        m_lclick_row(-1),
        m_rclick_row(-1),
        m_row_drag_offset(Pt(-1, -1)),
        m_last_row_browsed(-1),
        m_first_row_shown(0),
        m_first_col_shown(0),
        m_cell_margin(2),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_style(0),
        m_header_row(0),
        m_row_height(22),
        m_keep_col_widths(false),
        m_clip_cells(false),
        m_sort_col(0)
{
    SetColor(color);
    ValidateStyle();
    m_header_row = new Row;
}

ListBox::ListBox(int x, int y, int w, int h, Clr color, const vector<int>& col_widths,
                 Clr interior/* = CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        Control(x, y, w, h, flags),
        m_vscroll(0),
        m_hscroll(0),
        m_caret(-1),
        m_old_sel_row(-1),
        m_old_rdown_row(-1),
        m_lclick_row(-1),
        m_rclick_row(-1),
        m_row_drag_offset(Pt(-1, -1)),
        m_last_row_browsed(-1),
        m_first_row_shown(0),
        m_first_col_shown(0),
        m_col_widths(col_widths),
        m_cell_margin(2),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_style(0),
        m_header_row(0),
        m_row_height(22),
        m_keep_col_widths(false),
        m_clip_cells(false),
        m_sort_col(0)
{
    SetColor(color);
    ValidateStyle();
    m_header_row = new Row;
}

ListBox::ListBox(const XMLElement& elem) :
        Control(elem.Child("GG::Control")),
        m_vscroll(0),
        m_hscroll(0),
        m_old_sel_row(-1),
        m_old_rdown_row(-1),
        m_lclick_row(-1),
        m_rclick_row(-1),
        m_row_drag_offset(Pt(-1, -1)),
        m_last_row_browsed(-1),
        m_header_row(0)
{
    if (elem.Tag() != "GG::ListBox")
        throw std::invalid_argument("Attempted to construct a GG::ListBox from an XMLElement that had a tag other than \"GG::ListBox\"");

    const XMLElement* curr_elem = &elem.Child("m_caret");
    m_caret = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_selections");
    for (int i = 0; i < curr_elem->NumAttributes(); ++i) {
        m_selections.insert(lexical_cast<int>(curr_elem->Attribute("index" + lexical_cast<string>(i))));
    }

    curr_elem = &elem.Child("m_rows");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        if (Row* row = dynamic_cast<Row*>(App::GetApp()->GenerateWnd(curr_elem->Child(i))))
            m_rows.push_back(row);
        else
            throw std::runtime_error("ListBox::ListBox : Attempted to use a non-Row object as a ListBox row.");
    }

    curr_elem = &elem.Child("m_first_row_shown");
    m_first_row_shown = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_first_col_shown");
    m_first_col_shown = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_col_widths");
    for (int i = 0; i < curr_elem->NumAttributes(); ++i) {
        m_col_widths.push_back(lexical_cast<int>(curr_elem->Attribute("index" + lexical_cast<string>(i))));
    }

    curr_elem = &elem.Child("m_col_alignments");
    for (int i = 0; i < curr_elem->NumAttributes(); ++i) {
        m_col_alignments.push_back(lexical_cast<Uint32>(curr_elem->Attribute("index" + lexical_cast<string>(i))));
    }

    curr_elem = &elem.Child("m_cell_margin");
    m_cell_margin = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_int_color");
    m_int_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_hilite_color");
    m_hilite_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_style");
    m_style = lexical_cast<Uint32>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_header_row");
    if (curr_elem->NumChildren()) {
        if (Row* row = dynamic_cast<Row*>(App::GetApp()->GenerateWnd(curr_elem->Child(0))))
            m_header_row = row;
        else
            throw std::runtime_error("ListBox::ListBox : Attempted to use a non-Row object as the header row.");
    } else {
        m_header_row = new Row;
    }

    curr_elem = &elem.Child("m_row_height");
    m_row_height = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_keep_col_widths");
    m_keep_col_widths = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_clip_cells");
    m_clip_cells = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_sort_col");
    m_sort_col = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_allowed_types");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        m_allowed_types.insert(curr_elem->Child("index" + lexical_cast<string>(i)).Text());
    }

    for (unsigned int i = 0; i < m_rows.size(); ++i)
        AttachRowChildren(m_rows[i]);

    AttachRowChildren(m_header_row);

    AdjustScrolls();
}

ListBox::~ListBox()
{
    // note that this does not clean up the contents of the row (the Controls); these will be freed as children of the ListBox
    for (unsigned int i = 0; i < m_rows.size(); ++i)
        delete m_rows[i];
}

Pt ListBox::ClientUpperLeft() const
{
    return UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->size() ? m_header_row->Height() : 0));
}

Pt ListBox::ClientLowerRight() const
{
    return LowerRight() - Pt(BORDER_THICK + RightMargin(), BORDER_THICK + BottomMargin());
}

int ListBox::SortCol() const
{
    return (m_sort_col >= 0 && m_sort_col < static_cast<int>(m_col_widths.size()) ? m_sort_col : 0);
}

int ListBox::Render()
{
    // draw beveled rectangle around client area
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt cl_ul = ClientUpperLeft(), cl_lr = ClientLowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, BORDER_THICK);

    // clip rows to viewable area, and save old scissor state, if any
    bool disable_scissor = !glIsEnabled(GL_SCISSOR_TEST);
    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(ul.x + BORDER_THICK, App::GetApp()->AppHeight() - lr.y + BORDER_THICK,
              lr.x - ul.x - 2 * BORDER_THICK - RightMargin(),
              lr.y - ul.y - 2 * BORDER_THICK - BottomMargin());

    int last_visible_row = LastVisibleRow();
    int last_visible_col = LastVisibleCol();

    // draw selection hiliting
    int prev_sel = -1;
    int top = cl_ul.y, bottom = 0;
    for (std::set<int>::iterator  it = m_selections.begin(); it != m_selections.end(); ++it) {
            int curr_sel = *it;
            if (curr_sel >= m_first_row_shown && curr_sel <= last_visible_row){
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
            (App::GetApp()->FocusWnd() == this || App::GetApp()->FocusWnd() == m_vscroll || App::GetApp()->FocusWnd() == m_hscroll)) {
        int top = cl_ul.y;
        for (int i = m_first_row_shown; i < m_caret; ++i)
            top += m_rows[i]->Height();
        bottom = top + m_rows[m_caret]->Height();
        FlatRectangle(cl_ul.x, top, cl_lr.x, bottom, CLR_ZERO, CLR_SHADOW, 2);
    }

    // draw column headings
    if (m_header_row->size() && static_cast<int>(m_header_row->size()) == NumCols())
        RenderRow(m_header_row, cl_ul.x, cl_ul.y - m_header_row->Height(), last_visible_col);

    // draw data in cells
    top = cl_ul.y;
    for (int i = m_first_row_shown; i <= last_visible_row; ++i) {
        RenderRow(m_rows[i], cl_ul.x, top, last_visible_col);
        top += m_rows[i]->Height();
    }

    // restore previous scissor-clipping state
    if (disable_scissor)
        glDisable(GL_SCISSOR_TEST);
    glPopAttrib();

    // draw dragged line item, if one exists
    if (m_old_sel_row != -1 && m_row_drag_offset != Pt(-1, -1)) {
        Pt drag_row_ul = App::GetApp()->MousePosition() - m_row_drag_offset;
        RenderRow(m_rows[m_old_sel_row], drag_row_ul.x, drag_row_ul.y, last_visible_col);
    }

    return 1;
}

int ListBox::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        m_old_sel_row = RowUnderPt(pt);
        if (m_old_sel_row >= static_cast<int>(m_rows.size()) || !InClient(pt)) {
            m_old_sel_row = -1;
        } else {
            m_old_sel_row_selected = m_selections.find(m_old_sel_row) != m_selections.end();
            if (!(m_style & LB_NOSEL) && !m_old_sel_row_selected)
                ClickAtRow(m_old_sel_row, keys);
            if (m_style & LB_DRAGDROP && (((m_style & LB_NOSEL) || m_selections.find(m_old_sel_row) != m_selections.end())))
                m_row_drag_offset = DragOffset(pt);
        }
    }
    return 1;
}

int ListBox::LButtonUp(const Pt& pt, Uint32 keys)
{
    if (!Disabled() && (m_style & LB_DRAGDROP) && m_old_sel_row != -1 && m_row_drag_offset != Pt(-1, -1)) {
        Wnd* tmp = App::GetApp()->GetWindowUnder(pt);
        ListBox* drop_target_wnd;
        if ((drop_target_wnd = dynamic_cast<ListBox*>(tmp)) && 
            drop_target_wnd->AcceptsDropType(m_rows[m_old_sel_row]->data_type)) {
            // this will drop the item selected by the user's mouse position (or all selected items) into the place, 
            // if no sorting is done on target listbox.
            int ins_row = drop_target_wnd->RowUnderPt(pt);
            if (m_selections.empty()) {
                drop_target_wnd->Insert(m_rows[m_old_sel_row], ins_row, true);
                Delete(m_old_sel_row);
            } else {
                for (set<int>::iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
                    drop_target_wnd->Insert(m_rows[*it], ins_row++, true);
                }
                set<int> sels = m_selections;
                for (set<int>::reverse_iterator it = sels.rbegin(); it != sels.rend(); ++it) {
                    Delete(*it);
                }
            }
            if (m_caret > static_cast<int>(m_rows.size()) - 1)
                m_caret = static_cast<int>(m_rows.size()) - 1;
        }
    }
    m_old_sel_row = -1;
    m_row_drag_offset = Pt(-1, -1);
    return 1;
}

int ListBox::LClick(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        if (m_old_sel_row >= 0 && InClient(pt)) {
            int sel_row = RowUnderPt(pt);
            if (sel_row == m_old_sel_row) {
                if (!(m_style & LB_NOSEL))
                    ClickAtRow(sel_row, keys);
                m_lclick_row = sel_row;
            } else if (m_row_drag_offset != Pt(-1, -1) && (m_style & LB_DRAGDROP) && (m_style & LB_NOSORT) && 
                       (m_selections.empty() || (m_style & LB_SINGLESEL))) {
                // allow arbitrary rearrangement of unsorted lists that have dragging and dropping enabled (NOSEL and SINGLESEL only)
                int old_sel_row = m_old_sel_row;
                Insert(m_rows[old_sel_row], sel_row, true);
                Delete(old_sel_row + (sel_row <= old_sel_row ? 1 : 0));
            }
        }
    }
    m_old_sel_row = -1;
    m_row_drag_offset = Pt(-1, -1);
    return 1;
}

int ListBox::LDoubleClick(const Pt& pt, Uint32 keys)
{
    int row = RowUnderPt(pt);
    if (!Disabled() && row >= 0 && row == m_lclick_row && InClient(pt)) {
        m_double_clicked_sig(row, m_rows[row]);
    } else {
        LClick(pt, keys);
    }
    return 1;
}

int ListBox::RButtonDown(const Pt& pt, Uint32 keys)
{
    int row = RowUnderPt(pt);
    if (!Disabled() && row < static_cast<int>(m_rows.size()) && InClient(pt)) {
        m_old_rdown_row = row;
    } else {
        m_old_rdown_row = -1;
    }
    return 1;
}

int ListBox::RClick(const Pt& pt, Uint32 keys)
{
    int row = RowUnderPt(pt);
    if (!Disabled() && row >= 0 && row == m_old_rdown_row && InClient(pt)) {
        m_rclick_row = row;
        m_rclicked_sig(row, m_rows[row], pt);
    }
    m_old_rdown_row = -1;
    return 1;
}

int ListBox::Keypress(Key key, Uint32 key_mods)
{
    int retval = 1;

    switch (key) {
    case GGK_SPACE: // space bar (selects item under caret like a mouse click)
        if (m_caret != -1)
            retval = ClickAtRow(m_caret, key_mods);
        break;
    case GGK_DELETE: // delete key
        if (m_style & LB_USERDELETE) {
            set<int>::reverse_iterator r_it;
            while ((r_it = m_selections.rbegin()) != m_selections.rend())
                Delete(*r_it);
            m_selections.clear();
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
            int space = ClientDimensions().y;
            while (m_caret >= 1 && (space -= m_rows[m_caret - 1]->Height()) > 0)
                --m_caret;
        }
        break;
    case GGK_PAGEDOWN: // page down key (not numpad key)
        if (m_caret != -1) {
            int space = ClientDimensions().y;
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
            if (std::accumulate(m_col_widths.begin(), m_col_widths.begin() + last_fully_visible_col, 0) > ClientDimensions().x)
                --last_fully_visible_col;
            if (last_fully_visible_col < static_cast<int>(m_col_widths.size()) - 1) {
                ++m_first_col_shown;
                m_hscroll->ScrollTo(std::accumulate(m_col_widths.begin(), m_col_widths.begin() + m_first_col_shown, 0));
            }
            break;}

        // any other key gets passed along to the parent
    default:
        if (Parent())
            retval = Parent()->Keypress(key, key_mods);
    }

    if (key != GGK_SPACE && key != GGK_DELETE && key != GGK_LEFT && key != GGK_RIGHT)
        BringCaretIntoView();

    return retval;
}

int ListBox::MouseHere(const Pt& pt, Uint32 keys)
{
    if (!Disabled() && (m_style & LB_BROWSEUPDATES)) {
        int sel_row = RowUnderPt(pt);
        if (sel_row >= static_cast<int>(m_rows.size()))
            sel_row = -1;
        if (m_last_row_browsed != sel_row)
            m_browsed_sig(m_last_row_browsed = sel_row);
    }
    return 1;
}

int ListBox::MouseLeave(const Pt& pt, Uint32 keys)
{
    if (!Disabled() && (m_style & LB_BROWSEUPDATES)) {
        if (m_last_row_browsed != -1)
            m_browsed_sig(m_last_row_browsed = -1);
    }
    return 1;
}

void ListBox::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    DeleteChild(m_vscroll);
    m_vscroll = 0;
    DeleteChild(m_hscroll);
    m_hscroll = 0;
    AdjustScrolls(); // fix those pesky scrollbars
}

int ListBox::Insert(Row* row, int at/*= -1*/)
{
    return Insert(row, at, false);
}

void ListBox::Delete(int idx)
{
    if (idx >= 0 && idx < static_cast<int>(m_rows.size())) { // remove row
        // "bump" all the hiliting up one row
        m_selections.erase(idx);
        for (int i = idx; i < NumRows() - 1; ++i) {
            if (m_selections.find(i + 1) != m_selections.end()) {
                m_selections.insert(i);
                m_selections.erase(i + 1);
            }
        }
    }

    DetachRowChildren(m_rows[idx]);
    m_rows.erase(m_rows.begin() + idx);

    if (idx <= m_caret) // move caret up, if needed
        --m_caret;

    AdjustScrolls();

    m_deleted_sig(idx);
}

void ListBox::Clear()
{
    bool signal = !m_rows.empty(); // inhibit signal if the list box was already empty
    for (unsigned int i = 0; i < m_rows.size(); ++i)
        delete m_rows[i];
    m_rows.clear();
    m_caret = -1;
    DeleteChildren();
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

    AdjustScrolls();

    if (signal)
        m_cleared_sig();
}

void ListBox::ClearSelection()
{
    bool emit_signal = false;
    if (!m_selections.empty())
        emit_signal = true;
    m_selections.clear();
    m_caret = -1;
    if (emit_signal)
        m_sel_changed_sig(m_selections);
}

void ListBox::ClearRow(int n)
{
    if (n >= 0 && n < static_cast<int>(m_rows.size()) && m_selections.find(n) != m_selections.end()) {
        m_selections.erase(n);
        m_sel_changed_sig(m_selections);
    }
}

void ListBox::SelectRow(int n)
{
    if (n >= 0 && n < static_cast<int>(m_rows.size()) && m_selections.find(n) == m_selections.end()) {
        bool emit_signal = m_selections.find(n) == m_selections.end();
        if (m_style & LB_SINGLESEL)
            m_selections.clear();
        m_selections.insert(n);
        if (emit_signal)
            m_sel_changed_sig(m_selections);
    }
}

void ListBox::IndentRow(int n, int i)
{
    if (n >= 0 && n < static_cast<int>(m_rows.size()) && i >= 0)
        m_rows[i]->indentation = i;
}

void ListBox::SetStyle(Uint32 s)
{
    // if we're going from an unsorted style to a sorted one, do the sorting now
    if (m_style & LB_NOSORT) {
        if (!(s & LB_NOSORT))
            std::stable_sort(m_rows.begin(), m_rows.end(), RowSorter(m_sort_col, !(s & LB_SORTDESCENDING)));
    } else { // if we're changing the sorting order of a sorted list, reverse the contents
        if (((m_style & LB_SORTDESCENDING) && !(s & LB_SORTDESCENDING)) ||
                (!(m_style & LB_SORTDESCENDING) && (s & LB_SORTDESCENDING)))
            std::reverse(m_rows.begin(), m_rows.end());
    }
    m_style = s;
    ValidateStyle();
}

void ListBox::SetColHeaders(Row* r)
{
    delete m_header_row;
    m_header_row = r;
    // if this column header is being added to an empty listbox, the listbox takes on some of the
    // attributes of the header, similarly to the insertion of a row into an empty listbox; see Insert()
    if (m_rows.empty() && m_col_widths.empty()) {
        m_col_widths.resize(m_header_row->size(), (ClientDimensions().x - SCROLL_WIDTH) / m_header_row->size());
        // put the remainder in the last column, so the total width == ClientDimensions().x - SCROLL_WIDTH
        m_col_widths.back() += (ClientDimensions().x - SCROLL_WIDTH) % m_header_row->size();
        m_col_alignments.resize(m_header_row->size(), (m_style & (LB_LEFT | LB_CENTER | LB_RIGHT)));
        AttachRowChildren(m_header_row);
        NormalizeRow(m_header_row);
    }
}

void ListBox::SetNumCols(int n)
{
    if (n > 0) {
        if (m_col_widths.size()) {
            m_col_widths.resize(n);
            m_col_alignments.resize(n);
        } else {
            m_col_widths.resize(n, ClientDimensions().x / n);
            m_col_widths.back() += ClientDimensions().x % n;
            m_col_alignments.resize(n, (m_style & (LB_LEFT | LB_CENTER | LB_RIGHT)));
        }
        if (m_sort_col <= n)
            m_sort_col = 0;
    }
}

void ListBox::SetSortCol(int n)
{
    if (m_sort_col != n && !(m_style & LB_NOSORT))
        std::stable_sort(m_rows.begin(), m_rows.end(), RowSorter(n, !(m_style & LB_SORTDESCENDING)));
    m_sort_col = n;
}

XMLElement ListBox::XMLEncode() const
{
    XMLElement retval("GG::ListBox");
    retval.AppendChild(Control::XMLEncode());

    // remove children; they will be recreated at reload time
    while (retval.Child("GG::Control").Child("GG::Wnd").Child("m_children").NumChildren())
        retval.Child("GG::Control").Child("GG::Wnd").Child("m_children").RemoveChild(0);

    XMLElement temp;

    temp = XMLElement("m_caret");
    temp.SetAttribute("value", lexical_cast<string>(m_caret));
    retval.AppendChild(temp);

    int i = 0;
    temp = XMLElement("m_selections");
    for (std::set<int>::const_iterator it = m_selections.begin(); it != m_selections.end(); ++it) {
            temp.SetAttribute("index" + lexical_cast<string>(i++), lexical_cast<string>(*it));
        }
    retval.AppendChild(temp);

    temp = XMLElement("m_rows");
    for (std::vector<Row*>::const_iterator it = m_rows.begin(); it != m_rows.end(); ++it) {
        temp.AppendChild((*it)->XMLEncode());
    }
    retval.AppendChild(temp);

    temp = XMLElement("m_first_row_shown");
    temp.SetAttribute("value", lexical_cast<string>(m_first_row_shown));
    retval.AppendChild(temp);

    temp = XMLElement("m_first_col_shown");
    temp.SetAttribute("value", lexical_cast<string>(m_first_col_shown));
    retval.AppendChild(temp);

    i = 0;
    temp = XMLElement("m_col_widths");
    for (std::vector<int>::const_iterator it = m_col_widths.begin(); it != m_col_widths.end(); ++it) {
        temp.SetAttribute("index" + lexical_cast<string>(i++), lexical_cast<string>(*it));
    }
    retval.AppendChild(temp);

    i = 0;
    temp = XMLElement("m_col_alignments");
    for (std::vector<Uint32>::const_iterator it = m_col_alignments.begin(); it != m_col_alignments.end(); ++it) {
        temp.SetAttribute("index" + lexical_cast<string>(i++), lexical_cast<string>(*it));
    }
    retval.AppendChild(temp);

    temp = XMLElement("m_cell_margin");
    temp.SetAttribute("value", lexical_cast<string>(m_cell_margin));
    retval.AppendChild(temp);

    temp = XMLElement("m_int_color");
    temp.AppendChild(m_int_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_hilite_color");
    temp.AppendChild(m_hilite_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_style");
    temp.SetAttribute("value", lexical_cast<string>(m_style));
    retval.AppendChild(temp);

    temp = XMLElement("m_header_row");
    if (m_header_row)
        temp.AppendChild(m_header_row->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_row_height");
    temp.SetAttribute("value", lexical_cast<string>(m_row_height));
    retval.AppendChild(temp);

    temp = XMLElement("m_keep_col_widths");
    temp.SetAttribute("value", lexical_cast<string>(m_keep_col_widths));
    retval.AppendChild(temp);

    temp = XMLElement("m_clip_cells");
    temp.SetAttribute("value", lexical_cast<string>(m_clip_cells));
    retval.AppendChild(temp);

    temp = XMLElement("m_sort_col");
    temp.SetAttribute("value", lexical_cast<string>(m_sort_col));
    retval.AppendChild(temp);

    i = 0;
    temp = XMLElement("m_allowed_types");
    for (std::set<string>::const_iterator it = m_allowed_types.begin(); it != m_allowed_types.end(); ++it) {
            XMLElement child("index" + lexical_cast<string>(i++), *it);
            temp.AppendChild(child);
    }
    retval.AppendChild(temp);

    return retval;
}

int ListBox::RightMargin() const
{
    return (m_vscroll ? SCROLL_WIDTH : 0);
}

int ListBox::BottomMargin() const
{
    return (m_hscroll ? SCROLL_WIDTH : 0);
}

bool ListBox::AcceptsDropType(const string& str) const
{
    bool retval = false;
    if (m_style & LB_DRAGDROP) {
        if (m_allowed_types.find("") != m_allowed_types.end() ||
                m_allowed_types.find(str) != m_allowed_types.end())
            retval = true;
    }
    return retval;
}

int ListBox::LastVisibleRow() const
{
    int visible_pixels = ClientDimensions().y;
    int acc = 0, i;
    for (i = m_first_row_shown; i < static_cast<int>(m_rows.size()); ++i) {
        acc += m_rows[i]->Height();
        if (visible_pixels <= acc) break;
    }
    if (i >= static_cast<int>(m_rows.size()))
        i = static_cast<int>(m_rows.size()) - 1;
    return i;
}

int ListBox::LastVisibleCol() const
{
    int retval = m_first_col_shown;
    int right_boundary = m_col_widths.empty() ? ClientDimensions().x : m_col_widths[m_first_col_shown];
    while (right_boundary <= ClientLowerRight().x && retval < static_cast<int>(m_col_widths.size()) - 1)
        right_boundary += m_col_widths[++retval];
    return retval;
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

Pt ListBox::DragOffset(const Pt& pt) const
{
    Pt retval(-1, -1);
    Pt cl_ul = ClientUpperLeft();
    if (InClient(pt)) {
        retval.x = pt.x - cl_ul.x; // we know the x-offset immediately
        int acc = cl_ul.y;
        for (unsigned int i = m_first_row_shown; i < m_rows.size(); ++i) {
            acc += m_rows[i]->Height();
            if (pt.y <= acc) {
                retval.y = pt.y - (acc - m_rows[i]->Height());
                break;
            }
        }
        if (retval.y == -1)
            retval.x = -1;
    }
    return retval;
}

int ListBox::Insert(Row* row, int at, bool dropped)
{
    int retval = at;

    // the first row inserted into an empty list box defines the number of columns, and initializes the column widths
    if (m_rows.empty() && (m_col_widths.empty() || !m_keep_col_widths)) {
        m_col_widths.resize(row->size(), (ClientDimensions().x - SCROLL_WIDTH) / row->size());
        // put the remainder in the last column, so the total width == ClientDimensions().x - SCROLL_WIDTH
        m_col_widths.back() += (ClientDimensions().x - SCROLL_WIDTH) % row->size();
        m_col_alignments.resize(row->size(), (m_style & (LB_LEFT | LB_CENTER | LB_RIGHT)));
        if (m_header_row->size())
            NormalizeRow(m_header_row);
    }

    if (m_style & LB_NOSORT || m_rows.empty()) {
        if (at < 0 || at > static_cast<int>(m_rows.size())) // if at is out of bounds, insert item at the end
            at = m_rows.size();
        retval = at;
        m_rows.insert(m_rows.begin() + retval, row);
    } else { // otherwise, put it into the list in the right spot
        const string& row_str = (*row)[m_sort_col]->WindowText();
        retval = 0;
        while ((retval < static_cast<int>(m_rows.size())) &&
                ((m_style & LB_SORTDESCENDING) ? (row_str <= (*m_rows[retval])[m_sort_col]->WindowText()) :
                 (row_str >= (*m_rows[retval])[m_sort_col]->WindowText())))
            ++retval;
        m_rows.insert(m_rows.begin() + retval, row);
    }

    // add controls as children and adjust row heights and number of row item cells, if not already set
    AttachRowChildren(m_rows[retval]);
    NormalizeRow(m_rows[retval]);

    // "bump" the hiliting down one row
    if (retval != -1) {
        for (int i = NumRows() - 1; i > retval; --i) {
            if (m_selections.find(i - 1) != m_selections.end()) {
                m_selections.insert(i);
                m_selections.erase(i - 1);
            }
        }
    }

    if (retval <= m_caret) // move caret down, if needed
        ++m_caret;

    AdjustScrolls();

    if (dropped)
        m_dropped_sig(retval, row);
    else
        m_inserted_sig(retval, row);

    return retval;
}

void ListBox::BringCaretIntoView()
{
    int old_first_row = m_first_row_shown;
    if (m_caret >= 0 && m_caret < m_first_row_shown) {
        m_first_row_shown = m_caret;
    } else if (m_caret > LastVisibleRow()) { // find the row that preceeds the caret by about ClientDimensions().y pixels, and make it the first row shown
        int avail_space = ClientDimensions().y - m_rows[m_caret]->Height();
        m_first_row_shown = m_caret;
        while (m_first_row_shown > 0 && (avail_space -= m_rows[m_first_row_shown - 1]->Height()) > 0)
            --m_first_row_shown;
    }
    if (m_first_row_shown != old_first_row && m_vscroll) {
        int acc = 0;
        for (int i = 0; i < m_first_row_shown; ++i)
            acc += m_rows[i]->Height();
        m_vscroll->ScrollTo(acc);
    }
}

Scroll* ListBox::NewVScroll(bool horz_scroll)
{
    Pt cl_sz = ((LowerRight() - Pt(BORDER_THICK, BORDER_THICK)) -
                (UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->size() ? m_header_row->Height() : 0))));
    return new Scroll(cl_sz.x - SCROLL_WIDTH, 0, SCROLL_WIDTH, cl_sz.y - (horz_scroll ? SCROLL_WIDTH : 0), Scroll::VERTICAL, m_color, CLR_SHADOW);
}

Scroll* ListBox::NewHScroll(bool vert_scroll)
{
    Pt cl_sz = ((LowerRight() - Pt(BORDER_THICK, BORDER_THICK)) -
                (UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->size() ? m_header_row->Height() : 0))));
    return new Scroll(0, cl_sz.y - SCROLL_WIDTH, cl_sz.x - (vert_scroll ? SCROLL_WIDTH : 0), SCROLL_WIDTH, Scroll::HORIZONTAL, m_color, CLR_SHADOW);
}

void ListBox::RecreateScrolls()
{
    delete m_vscroll;
    delete m_hscroll;
    m_vscroll = m_hscroll = 0;
    AdjustScrolls();
}

void ListBox::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & LB_LEFT) ++dup_ct;
    if (m_style & LB_RIGHT) ++dup_ct;
    if (m_style & LB_CENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LB_LEFT by default
        m_style &= ~(LB_RIGHT | LB_CENTER);
        m_style |= LB_LEFT;
    }
    dup_ct = 0;
    if (m_style & LB_TOP) ++dup_ct;
    if (m_style & LB_BOTTOM) ++dup_ct;
    if (m_style & LB_VCENTER) ++dup_ct;
    if (dup_ct != 1) {  // exactly one must be picked; when none or multiples are picked, use LB_VCENTER by default
        m_style &= ~(LB_TOP | LB_BOTTOM);
        m_style |= LB_VCENTER;
    }
    dup_ct = 0;
    if (m_style & LB_NOSEL) ++dup_ct;
    if (m_style & LB_SINGLESEL) ++dup_ct;
    if (m_style & LB_QUICKSEL) ++dup_ct;
    if (dup_ct > 1)  // at most one of these may be picked; when multiples are picked, disable all of them
        m_style &= ~(LB_NOSEL | LB_SINGLESEL | LB_QUICKSEL);
}

void ListBox::AdjustScrolls()
{
    bool vertical_needed = false;
    bool horizontal_needed = false;

    // this client area calculation disregards the thickness of scrolls
    Pt cl_sz = ((LowerRight() - Pt(BORDER_THICK, BORDER_THICK)) -
                (UpperLeft() + Pt(BORDER_THICK, BORDER_THICK + (m_header_row->size() ? m_header_row->Height() : 0))));

    int total_x_extent = 0, total_y_extent = 0;
    for (unsigned int i = 0; i < m_col_widths.size(); ++i)
        total_x_extent += m_col_widths[i];
    for (unsigned int i = 0; i < m_rows.size(); ++i)
        total_y_extent += m_rows[i]->Height();

    vertical_needed = (total_y_extent > cl_sz.y ||
                       (total_y_extent > cl_sz.y - SCROLL_WIDTH && total_x_extent > cl_sz.x - SCROLL_WIDTH));
    horizontal_needed = (total_x_extent > cl_sz.x ||
                         (total_x_extent > cl_sz.x - SCROLL_WIDTH && total_y_extent > cl_sz.y - SCROLL_WIDTH));

    if (m_vscroll) { // if scroll already exists...
        if (!vertical_needed) { // remove scroll
            DeleteChild(m_vscroll);
            m_vscroll = 0;
        } else { // ensure vertical scroll has the right logical and physcal dimensions
            m_vscroll->SizeScroll(0, total_y_extent - 1, cl_sz.y / 8, cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0));
            int scroll_x = cl_sz.x - SCROLL_WIDTH;
            int scroll_y = 0;
            m_vscroll->SizeMove(scroll_x, scroll_y, scroll_x + SCROLL_WIDTH, scroll_y + cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0));
        }
    } else if (!m_vscroll && vertical_needed) { // if scroll doesn't exist but is needed
        m_vscroll = NewVScroll(horizontal_needed);
        m_vscroll->SizeScroll(0, total_y_extent - 1, cl_sz.y / 8, cl_sz.y - (horizontal_needed ? SCROLL_WIDTH : 0));
        AttachChild(m_vscroll);
        Connect(m_vscroll->ScrolledSignal(), &ListBox::VScrolled, this);
    }

    if (m_hscroll) { // if scroll already exists...
        if (!horizontal_needed) { // remove scroll
            DeleteChild(m_hscroll);
            m_hscroll = 0;
        } else { // ensure horizontal scroll has the right logical and physcal dimensions
            m_hscroll->SizeScroll(0, total_x_extent - 1, cl_sz.x / 8, cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0));
            int scroll_x = 0;
            int scroll_y = cl_sz.y - SCROLL_WIDTH;
            m_hscroll->SizeMove(scroll_x, scroll_y, scroll_x + cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0), scroll_y + SCROLL_WIDTH);
        }
    } else if (!m_hscroll && horizontal_needed) { // if scroll doesn't exist but is needed
        m_hscroll = NewHScroll(vertical_needed);
        m_hscroll->SizeScroll(0, total_x_extent - 1, cl_sz.x / 8, cl_sz.x - (vertical_needed ? SCROLL_WIDTH : 0));
        AttachChild(m_hscroll);
        Connect(m_hscroll->ScrolledSignal(), &ListBox::HScrolled, this);
    }
}

void ListBox::VScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_row_shown = 0;
    for (unsigned int i = 0; i < m_rows.size(); ++i) {
        if (tab_low > 0) {
            tab_low -= m_rows[i]->Height();
            ++m_first_row_shown;
        }
    }
}

void ListBox::HScrolled(int tab_low, int tab_high, int low, int high)
{
    m_first_col_shown = 0;
    for (unsigned int i = 0; i < m_col_widths.size(); ++i) {
        if (tab_low > 0) {
            tab_low -= m_col_widths[i];
            ++m_first_col_shown;
        }
    }
}

void ListBox::RenderRow(const Row* row, int left, int top, int last_col)
{
    // draw this row's controls on the -1 iteration, then each of its subrows on iterations 0 through sub_rows.size() - 1
    for (int j = -1; j < static_cast<int>(row->sub_rows.size()); ++j) {
        const Row* subrow_to_use = (j == -1 ? row : row->sub_rows[j]); // use main (non sub_row) data for the -1 iteration
        RenderSubRow(subrow_to_use, left, top, last_col);
        top += subrow_to_use->height;
    }
}

void ListBox::RenderSubRow(const Row* subrow, int left, int top, int last_col)
{
    int right = left;
    int bottom = top + subrow->height;
    // draw each control in turn
    for (int i = m_first_col_shown; i <= last_col; ++i) {
        if ((*subrow)[i]) {
            left = right;
            right += m_col_widths[i];
            int x = 0, y = 0;
            Pt control_sz = (*subrow)[i]->WindowDimensions();
            Uint32 alignment = subrow->alignment | m_col_alignments[i];

            if (alignment & LB_LEFT) {
                x = left + m_cell_margin;
                if (!i)
                    x += subrow->indentation; // indentation only applies to left-justified items in the first cell
            } else if (alignment & LB_CENTER) {
                x = left + m_cell_margin + (m_col_widths[i] - 2 * m_cell_margin - control_sz.x) / 2;
            } else { // (m_style & LB_RIGHT)
                x = right - m_cell_margin - control_sz.x;
            }

            if (alignment & LB_TOP) {
                y = top + m_cell_margin;
            } else if (alignment & LB_VCENTER) {
                y = top + m_cell_margin + (subrow->height - 2 * m_cell_margin - control_sz.y) / 2;
            } else { // (m_style & LB_BOTTOM)
                y = bottom - m_cell_margin - control_sz.y;
            }

            (*subrow)[i]->MoveTo(Pt(x, y) - ClientUpperLeft());
            (*subrow)[i]->Render();
        }
    }
}

int ListBox::ClickAtRow(int row, Uint32 keys)
{
    std::set<int> previous_selections = m_selections;
    if (m_style & LB_SINGLESEL) { // no special keys are being used; just clear all previous selections, select this row, set the caret here
        m_selections.clear();
        m_selections.insert(row);
        m_caret = row;
    } else {
        if (keys & GGKMOD_CTRL) { // control key depressed
            if (keys & GGKMOD_SHIFT && m_caret != -1) { // both shift and control keys depressed
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
                if (m_selections.find(row) != m_selections.end())
                    m_selections.erase(row);
                else
                    m_selections.insert(row);
                m_caret = row;
            }
        } else if (keys & GGKMOD_SHIFT) { // shift key depressed
            bool erase = m_selections.find(m_caret) == m_selections.end();
            if (!(m_style & LB_QUICKSEL))
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
        } else { // unless LB_QUICKSEL is used, this is treated just like LB_SINGLESEL above
            if (m_style&LB_QUICKSEL) {
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
        m_sel_changed_sig(m_selections);
    return 1;
}

void ListBox::NormalizeRow(Row* row)
{
    row->resize(m_col_widths.size());
    if (!row->height)
        row->height = m_row_height;
    for (unsigned int i = 0; i < row->sub_rows.size(); ++i) {
        row->sub_rows[i]->resize(m_col_widths.size());
        if (!row->sub_rows[i]->height)
            row->sub_rows[i]->height = row->height;
    }
}

void ListBox::AttachRowChildren(Row* row)
{
    // attach controls in row to the list box, but hide them; we want to control how they're rendered,
    // but want them to be fully interactive, just as they would be any where else
    for (unsigned int i = 0; i < row->size(); ++i) {
        AttachChild((*row)[i]);
        (*row)[i]->Hide();
    }    
    for (unsigned int i = 0; i < row->sub_rows.size(); ++i) {
        AttachRowChildren(row->sub_rows[i]);
    }
}

void ListBox::DetachRowChildren(Row* row)
{
    for (unsigned int i = 0; i < row->size(); ++i)
        DetachChild((*row)[i]);
    for (unsigned int i = 0; i < row->sub_rows.size(); ++i)
        DetachRowChildren(row->sub_rows[i]);
}


} // namespace GG

