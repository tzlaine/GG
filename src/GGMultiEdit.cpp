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

#include "GGMultiEdit.h"
#include "GGApp.h"
#include "GGScroll.h"
#include "GGDrawUtil.h"

namespace GG {

////////////////////////////////////////////////
// GG::MultiEdit
////////////////////////////////////////////////
// static(s)
const int MultiEdit::SCROLL_WIDTH = 14;

MultiEdit::MultiEdit(int x, int y, int w, int h, const string& str, const shared_ptr<Font>& font, Clr color, 
              Uint32 style/* = TF_LINEWRAP*/, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
        Edit(x, y, w, h, str, font, color, text_color, interior, flags),
        m_style(style),
        m_cursor_begin(0, 0),
        m_cursor_end(0, 0),
        m_first_col_shown(0),
        m_first_row_shown(0),
        m_max_lines_history(0),
        m_vscroll(0),
        m_hscroll(0)
{
    SetColor(color);
    Init();
}

MultiEdit::MultiEdit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, 
              Uint32 style/* = TF_LINEWRAP*/, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, 
              Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) : 
        Edit(x, y, w, h, str, font_filename, pts, color, text_color, interior, flags),
        m_style(style),
        m_cursor_begin(0, 0),
        m_cursor_end(0, 0),
        m_first_col_shown(0),
        m_first_row_shown(0),
        m_max_lines_history(0),
        m_vscroll(0),
        m_hscroll(0)
{
    SetColor(color);
    Init();
}

MultiEdit::MultiEdit(const XMLElement& elem) :
        Edit(elem.Child("GG::Edit")),
        m_cursor_begin(0, 0),
        m_cursor_end(0, 0),
        m_first_col_shown(0),
        m_first_row_shown(0),
        m_vscroll(0),
        m_hscroll(0)
{
    if (elem.Tag() != "GG::MultiEdit")
        throw std::invalid_argument("Attempted to construct a GG::MultiEdit from an XMLElement that had a tag other than \"GG::MultiEdit\"");

    const XMLElement* curr_elem = &elem.Child("m_style");
    m_style = lexical_cast<Uint32>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_max_lines_history");
    m_max_lines_history = lexical_cast<int>(curr_elem->Attribute("value"));

    Init();
}

MultiEdit::~MultiEdit()
{
    delete m_vscroll;
    delete m_hscroll;
}

Pt MultiEdit::ClientLowerRight() const
{
    return Edit::ClientLowerRight() - Pt(RightMargin(), BottomMargin());
}

int MultiEdit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
    Clr sel_text_color_to_use = Disabled() ? DisabledColor(SelectedTextColor()) : SelectedTextColor();
    Clr hilite_color_to_use = Disabled() ? DisabledColor(HiliteColor()) : HiliteColor();
    Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt cl_ul = ClientUpperLeft();
    Pt cl_lr = ClientLowerRight();

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, 2);

    // clip text to viewable area, and save old scissor state, if any
    bool disable_scissor = !glIsEnabled(GL_SCISSOR_TEST);
    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    const int GAP = PIXEL_MARGIN - 2;
    glScissor(ul.x + 2, App::GetApp()->AppHeight() - (lr.y - BottomMargin() - 2), ClientDimensions().x + 2 * GAP, ClientDimensions().y + 2 * GAP);

    Font::RenderState state;
    int first_visible_row = FirstVisibleRow();
    int last_visible_row = LastVisibleRow();
    Uint32 text_format = TextFormat() & ~(TF_TOP | TF_BOTTOM) | TF_VCENTER;
    vector<Font::LineData> lines;
    for (int row = first_visible_row; row <= last_visible_row; ++row) {
        int row_y_pos = m_style & TF_TOP ? 
            cl_ul.y + row * GetFont()->Lineskip() - m_first_row_shown : 
            cl_lr.y - (static_cast<int>(GetLineData().size()) - row) * GetFont()->Lineskip() - m_first_row_shown + 
            (m_vscroll && m_hscroll ? BottomMargin() : 0);
        Pt text_pos(cl_ul.x + RowStartX(row), row_y_pos);
        int begin = GetLineData()[row].begin_idx;
        int length = GetLineData()[row].extents.size();
        Pt text_extent = GetFont()->DetermineLines(m_text.substr(begin, length), text_format, 1 << 15, lines, true);
        int initial_text_x_pos = text_pos.x;

        // if one or more chars of this row are selected, hilite, then draw the range in the selected-text color
        pair<int, int> low_cursor_pos  = LowCursorPos();
        pair<int, int> high_cursor_pos = HighCursorPos();
        if (low_cursor_pos.first <= row && row <= high_cursor_pos.first && MultiSelected()) {
            // idx0 to idx1 is unhilited, idx1 to idx2 is hilited, and idx2 to idx3 is unhilited; each range may be empty
            int idx0 = GetLineData()[row].begin_idx;
            int idx1 = std::max(StringIndexOf(low_cursor_pos.first, low_cursor_pos.second), GetLineData()[row].begin_idx);
            int idx2 = std::min(StringIndexOf(high_cursor_pos.first, high_cursor_pos.second), GetLineData()[row].end_idx);
            int idx3 = GetLineData()[row].end_idx;

            // draw text
            text_extent = GetFont()->DetermineLines(m_text.substr(idx0, idx1 - idx0), text_format, 1 << 15, lines, true);
            glColor4ubv(text_color_to_use.v);
            GetFont()->RenderText(text_pos, text_pos + text_extent, m_text.substr(idx0, idx1 - idx0), text_format, &lines, true, &state);
            text_pos.x += text_extent.x;

            text_extent = GetFont()->DetermineLines(m_text.substr(idx1, idx2 - idx1), text_format, 1 << 15, lines, true);
            // draw hiliting
            FlatRectangle(text_pos.x, text_pos.y, (text_pos + text_extent).x, text_pos.y + GetFont()->Lineskip(), hilite_color_to_use, CLR_ZERO, 0);
            // draw hilited text
            glColor4ubv(sel_text_color_to_use.v);
            GetFont()->RenderText(text_pos, text_pos + text_extent, m_text.substr(idx1, idx2 - idx1), text_format, &lines, true, &state);
            text_pos.x += text_extent.x;

            text_extent = GetFont()->DetermineLines(m_text.substr(idx2, idx3 - idx2), text_format, 1 << 15, lines, true);
            glColor4ubv(text_color_to_use.v);
            GetFont()->RenderText(text_pos, text_pos + text_extent, m_text.substr(idx2, idx3 - idx2), text_format, &lines, true, &state);
            text_pos.x += text_extent.x;
        } else { // just draw normal text on this line
            glColor4ubv(text_color_to_use.v);
            GetFont()->RenderText(text_pos, text_pos + text_extent, m_text.substr(begin, length), text_format, &lines, true, &state);
        }
        // if there's no selected text, but this row contains the caret (and READ_ONLY is not in effect)
        if (!MultiSelected() && m_cursor_begin.first == row && !(m_style & READ_ONLY)) {
            int caret_x = CharXOffset(m_cursor_begin.first, m_cursor_begin.second) + initial_text_x_pos;
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            glVertex2i(caret_x, row_y_pos);
            glVertex2i(caret_x, row_y_pos + GetFont()->Lineskip());
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }
    }

    // restore previous state
    if (disable_scissor)
        glDisable(GL_SCISSOR_TEST);
    glPopAttrib();

    return 1;
}

int MultiEdit::LButtonDown(const Pt& pt, Uint32 keys)
{
    // when a button press occurs, record the character position under the cursor, and remove any previous selection range
    if (!Disabled() && !(m_style & READ_ONLY)) {
        m_cursor_begin = m_cursor_end = CharAt(ScreenToClient(pt));
        AdjustView();
    }
    return 1;
}

int MultiEdit::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled() && !(m_style & READ_ONLY)) {
        // when a drag occurs, move m_cursor_end to where the mouse is, which selects a range of characters
        Pt click_pos = ScreenToClient(pt); // coord of click within text space
        m_cursor_end = CharAt(click_pos);
        // if we're dragging past the currently visible text, adjust the view so more text can be selected
        if (click_pos.x < 0 || click_pos.x > ClientDimensions().x || 
            click_pos.y < 0 || click_pos.y > ClientDimensions().y) 
            AdjustView();
    }
    return 1;
}

int MultiEdit::Keypress(Key key, Uint32 key_mods)
{
    if (!Disabled() && !(m_style & READ_ONLY)) {
        bool shift_down = key_mods & (GGKMOD_LSHIFT | GGKMOD_RSHIFT);
        bool emit_signal = false;
        switch (key) {
        case GGK_RETURN:
        case GGK_KP_ENTER: {
            if (MultiSelected())
                ClearSelected();
            Insert(StringIndexOf(m_cursor_begin.first, m_cursor_begin.second), '\n');
            ++m_cursor_begin.first;
            m_cursor_begin.second = 0;
            // the cursor might be off the bottom if the bottom row was just chopped off to satisfy m_max_lines_history
            if (static_cast<int>(GetLineData().size()) - 1 < m_cursor_begin.first) {
                m_cursor_begin.first = static_cast<int>(GetLineData().size()) - 1;
                m_cursor_begin.second = static_cast<int>(GetLineData()[m_cursor_begin.first].extents.size());
            }
            m_cursor_end = m_cursor_begin;
            emit_signal = true;
            break;
        }

        case GGK_LEFT: {
            if (MultiSelected() && !shift_down) {
                m_cursor_begin = m_cursor_end = LowCursorPos();
            } else if (0 < m_cursor_end.second) {
                --m_cursor_end.second;
                int extent = GetLineData()[m_cursor_end.first].extents.size() ? GetLineData()[m_cursor_end.first].extents[m_cursor_end.second] : 0;
                while (0 < m_cursor_end.second && extent == GetLineData()[m_cursor_end.first].extents[m_cursor_end.second - 1])
                    --m_cursor_end.second;
                // if we're at the beginning of a line and there are only tag characters here, go up to the previous line, if any
                if (!m_cursor_end.second && !GetLineData()[m_cursor_end.first].extents[m_cursor_end.second] && 0 < m_cursor_end.first) {
                    --m_cursor_end.first;
                    m_cursor_end.second = GetLineData()[m_cursor_end.first].extents.size();
                    int extent = GetLineData()[m_cursor_end.first].extents.size() ? GetLineData()[m_cursor_end.first].extents[m_cursor_end.second] : 0;
                    while (0 < m_cursor_end.second && extent == GetLineData()[m_cursor_end.first].extents[m_cursor_end.second - 1])
                        --m_cursor_end.second;
                }
            } else if (0 < m_cursor_end.first) {
                --m_cursor_end.first;
                m_cursor_end.second = GetLineData()[m_cursor_end.first].extents.size();
                int extent = GetLineData()[m_cursor_end.first].extents.size() ? GetLineData()[m_cursor_end.first].extents[m_cursor_end.second] : 0;
                while (0 < m_cursor_end.second && extent == GetLineData()[m_cursor_end.first].extents[m_cursor_end.second - 1])
                    --m_cursor_end.second;
            }
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
            break;
        }

        case GGK_RIGHT: {
            int initial_extent = GetLineData()[m_cursor_end.first].extents.size() ? GetLineData()[m_cursor_end.first].extents[m_cursor_end.second] : 0;
            if (MultiSelected() && !shift_down) {
                m_cursor_begin = m_cursor_end = HighCursorPos();
            } else if (m_cursor_end.second < static_cast<int>(GetLineData()[m_cursor_end.first].extents.size())) {
                // if there are all tags on this line, we need to skip down to the next line, if one exists
                if (!GetLineData()[m_cursor_end.first].extents.back()) {
                    if (m_cursor_end.first < static_cast<int>(GetLineData().size() - 1)) {
                        ++m_cursor_end.first;
                        m_cursor_end.second = 0;
                        while (m_cursor_end.second < static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) && 
                               !GetLineData()[m_cursor_end.first].extents[m_cursor_end.second])
                            ++m_cursor_end.second;
                    }
                } else {
                    while (m_cursor_end.second < static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) - 1 && 
                           initial_extent == GetLineData()[m_cursor_end.first].extents[m_cursor_end.second + 1])
                        ++m_cursor_end.second;
                    ++m_cursor_end.second;
                }
            } else if (m_cursor_end.first < static_cast<int>(GetLineData().size()) - 1) {
                ++m_cursor_end.first;
                m_cursor_end.second = 0;
                while (m_cursor_end.second < static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) && 
                       !GetLineData()[m_cursor_end.first].extents[m_cursor_end.second])
                    ++m_cursor_end.second;
            }
            if (!shift_down)
                m_cursor_begin = m_cursor_end;
            break;
        }

        case GGK_UP: {
            if (MultiSelected() && !shift_down) {
                m_cursor_begin = m_cursor_end = LowCursorPos();
            } else if (0 < m_cursor_end.first) {
                int row_start = RowStartX(m_cursor_end.first);
                int char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
                --m_cursor_end.first;
                m_cursor_end.second = CharAt(m_cursor_end.first, row_start + char_offset);
                if (!shift_down)
                    m_cursor_begin = m_cursor_end;
            }
            break;
        }

        case GGK_DOWN: {
            if (MultiSelected() && !shift_down) {
                m_cursor_begin = m_cursor_end = HighCursorPos();
            } else if (m_cursor_end.first < static_cast<int>(GetLineData().size()) - 1) {
                int row_start = RowStartX(m_cursor_end.first);
                int char_offset = CharXOffset(m_cursor_end.first, m_cursor_end.second);
                ++m_cursor_end.first;
                m_cursor_end.second = CharAt(m_cursor_end.first, row_start + char_offset);
                if (!shift_down)
                    m_cursor_begin = m_cursor_end;
            }
            break;
        }

        case GGK_HOME: {
            if (shift_down) {
                m_cursor_end.second = 0;
            } else {
                m_cursor_begin.second = 0;
                m_cursor_end = m_cursor_begin;
            }
            break;
        }

        case GGK_END: {
            if (shift_down) {
                m_cursor_end.second = GetLineData()[m_cursor_end.first].extents.size();
            } else {
                m_cursor_begin.second = GetLineData()[m_cursor_begin.first].extents.size();
                m_cursor_end = m_cursor_begin;
            }
            break;
        }

        case GGK_PAGEUP: {
            if (m_vscroll) {
                m_vscroll->ScrollPageDecr();
                int rows_moved = m_vscroll->PageSize() / GetFont()->Lineskip();
                m_cursor_end.first = std::max(0, m_cursor_end.first - rows_moved);
                if (static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) < m_cursor_end.second)
                    m_cursor_end.second = static_cast<int>(GetLineData()[m_cursor_end.first].extents.size());
                m_cursor_begin = m_cursor_end;
            }
            break;
        }

        case GGK_PAGEDOWN: {
            if (m_vscroll) {
                m_vscroll->ScrollPageIncr();
                int rows_moved = m_vscroll->PageSize() / GetFont()->Lineskip();
                m_cursor_end.first = std::min(m_cursor_end.first + rows_moved, static_cast<int>(GetLineData().size()) - 1);
                if (static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) < m_cursor_end.second)
                    m_cursor_end.second = static_cast<int>(GetLineData()[m_cursor_end.first].extents.size());
                m_cursor_begin = m_cursor_end;
            }
            break;
        }

        case GGK_BACKSPACE: {
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (0 < m_cursor_begin.second) {
                m_cursor_end.second = --m_cursor_begin.second;
                Erase(StringIndexOf(m_cursor_begin.first, m_cursor_begin.second));
                emit_signal = true;
            } else if (0 < m_cursor_begin.first) {
                m_cursor_end.first = --m_cursor_begin.first;
                m_cursor_end.second = m_cursor_begin.second = GetLineData()[m_cursor_begin.first].extents.size();
                Erase(StringIndexOf(m_cursor_begin.first, m_cursor_begin.second));
                emit_signal = true;
            }
            break;
        }

        case GGK_DELETE: {
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (m_cursor_begin.second < static_cast<int>(GetLineData()[m_cursor_begin.first].extents.size())) {
                Erase(StringIndexOf(m_cursor_begin.first, m_cursor_begin.second));
                emit_signal = true;
            } else if (m_cursor_begin.first < static_cast<int>(GetLineData().size()) - 1) {
                int begin = StringIndexOf(m_cursor_begin.first, m_cursor_begin.second);
                int length = StringIndexOf(m_cursor_begin.first + 1, 0) - begin;
                Erase(begin, length);
                emit_signal = true;
            }
            break;
        }

        default: {
            if (isprint(key)) { // only process it if it's a printable character
                if (MultiSelected())
                    ClearSelected();
                // insert the character to the right of the caret
                Insert(StringIndexOf(m_cursor_begin.first, m_cursor_begin.second), key);
                // then move the caret fwd one.
                if (m_cursor_begin.second < static_cast<int>(GetLineData()[m_cursor_begin.first].extents.size())) {
                    ++m_cursor_begin.second;
                } else {
                    ++m_cursor_begin.first;
                    m_cursor_begin.second = 1;
                }
                // the cursor might be off the bottom if the bottom row was just chopped off to satisfy m_max_lines_history
                if (static_cast<int>(GetLineData().size()) - 1 < m_cursor_begin.first) {
                    m_cursor_begin.first = static_cast<int>(GetLineData().size()) - 1;
                    m_cursor_begin.second = static_cast<int>(GetLineData()[m_cursor_begin.first].extents.size());
                }
                m_cursor_end = m_cursor_begin;
                emit_signal = true;
            } else if (Parent()) {
                return Parent()->Keypress(key, key_mods);
            }
            break;
        }
        }
        AdjustView();
        if (emit_signal)
            EditedSignal()(m_text);
    }
    return 1;
}

void MultiEdit::SizeMove(int x1, int y1, int x2, int y2)
{
    if (m_style & INTEGRAL_HEIGHT)
        y2 -= ((y2 - y1) - (2 * PIXEL_MARGIN)) % GetFont()->Lineskip();
    Edit::SizeMove(x1, y1, x2, y2);
    AdjustScrolls();
    AdjustView();
}

void MultiEdit::SelectAll()
{
    m_cursor_begin = pair<int, int>(0, 0);
    m_cursor_end = pair<int, int>(GetLineData().size() - 1, 
                                  GetLineData().size() ? GetLineData()[GetLineData().size() - 1].extents.size() : 0);
}

void MultiEdit::SetText(const string& str)
{
     bool scroll_to_end = (m_style & TERMINAL_STYLE) &&
          (!m_vscroll || m_vscroll->PosnRange().second == m_vscroll->ScrollRange().second + 1);

    // trim the rows, if required by m_max_lines_history
    Pt cl_sz = ClientDimensions();
    Uint32 format = TextFormat();
    if (0 < m_max_lines_history) {
        vector<Font::LineData> lines;
        GetFont()->DetermineLines(str, format, cl_sz.x, lines, true);
        if (m_max_lines_history < static_cast<int>(lines.size())) {
            int first_line = 0;
            int last_line = m_max_lines_history - 1;
            int cursor_begin_idx = -1; // used to correct the cursor range when lines get chopped
            int cursor_end_idx = -1;
            if (m_style & TERMINAL_STYLE) {
                first_line = (lines.size() - 1) - (m_max_lines_history - 1);
                last_line = lines.size() - 1;
                // chopping these lines off the front will invalidate the cursor range unless we do this
                int chars_cut = 0;
                for (int i = 0; i < first_line; ++i) {
                    if (i < static_cast<int>(GetLineData().size()) - 1)
                        chars_cut += GetLineData()[i + 1].begin_idx - GetLineData()[i].begin_idx;
                    else
                        chars_cut += GetLineData()[i].end_idx - GetLineData()[i].begin_idx;
                }
                cursor_begin_idx = StringIndexOf(m_cursor_begin.first, m_cursor_begin.second) - chars_cut;
                cursor_end_idx = StringIndexOf(m_cursor_end.first, m_cursor_end.second) - chars_cut;
            }
            int begin = lines[first_line].begin_idx;
            int length = lines[last_line].end_idx - begin;
            TextControl::SetText(str.substr(begin, length));
            if (cursor_begin_idx != -1 && cursor_end_idx != -1) {
                bool found_cursor_begin = false;
                bool found_cursor_end = false;
                for (unsigned int i = 0; i < GetLineData().size(); ++i) {
                    if (!found_cursor_begin && cursor_begin_idx <= GetLineData()[i].end_idx) {
                        m_cursor_begin.first = i;
                        m_cursor_begin.second = cursor_begin_idx - GetLineData()[i].begin_idx;
                        found_cursor_begin = true;
                    }
                    if (!found_cursor_end && cursor_end_idx <= GetLineData()[i].end_idx) {
                        m_cursor_end.first = i;
                        m_cursor_end.second = cursor_end_idx - GetLineData()[i].begin_idx;
                        found_cursor_end = true;
                    }
                }
            }
        } else {
            TextControl::SetText(str);
        }
    } else {
        TextControl::SetText(str);
    }

    // make sure the change in text did not make the cursor position invalid
    if (GetLineData().empty()) {
        m_cursor_end = pair<int,int>(0,0);
    } else if (static_cast<int>(GetLineData().size()) <= m_cursor_end.first) {
        m_cursor_end.first = static_cast<int>(GetLineData().size()) - 1;
        m_cursor_end.second = static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) - 1;
    } else if (static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) < m_cursor_end.second) {
        m_cursor_end.second = static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) - 1;
    }
    m_cursor_begin = m_cursor_end; // eliminate any hiliting

    m_contents_sz = GetFont()->TextExtent(m_text, format, (format & (TF_WORDBREAK | TF_LINEWRAP)) ? cl_sz.x : 0, true);

    AdjustScrolls();
    AdjustView();
     if (scroll_to_end && m_vscroll)
          m_vscroll->ScrollTo(m_vscroll->ScrollRange().second - m_vscroll->PageSize());
    EditedSignal()(str);
}

XMLElement MultiEdit::XMLEncode() const
{
    XMLElement retval("GG::MultiEdit");
    retval.AppendChild(Edit::XMLEncode());

    XMLElement temp;

    temp = XMLElement("m_style");
    temp.SetAttribute("value", lexical_cast<string>(m_style));
    retval.AppendChild(temp);

    temp = XMLElement("m_max_lines_history");
    temp.SetAttribute("value", lexical_cast<string>(m_max_lines_history));
    retval.AppendChild(temp);

    return retval;
}

bool MultiEdit::MultiSelected() const
{
    return m_cursor_begin != m_cursor_end;
}

int MultiEdit::RightMargin() const
{
    return (m_vscroll ? SCROLL_WIDTH : 0);
}

int MultiEdit::BottomMargin() const
{
    return (m_hscroll ? SCROLL_WIDTH : 0);
}

pair<int, int> MultiEdit::CharAt(const Pt& pt) const
{
    pair<int, int> retval;

    retval.first = RowAt(pt.y);
    if (retval.first < 0)
        retval = pair<int, int>(0, 0);
    else if (static_cast<int>(GetLineData().size()) <= retval.first)
        retval.first = GetLineData().size() - 1;

    if (0 <= retval.first && retval.first < static_cast<int>(GetLineData().size()))
        retval.second = CharAt(retval.first, pt.x);
    if (retval.second < 0)
        retval.second = 0;
    else if (static_cast<int>(GetLineData()[retval.first].extents.size()) <= retval.second)
        retval.second = GetLineData()[retval.first].extents.size();

    return retval;
}

int MultiEdit::StringIndexOf(int row, int char_idx) const
{
    return GetLineData()[row].begin_idx + char_idx;
}

int MultiEdit::RowStartX(int row) const
{
    int retval = -m_first_col_shown;

    Pt cl_sz = ClientDimensions();
    int excess_width = m_contents_sz.x - cl_sz.x;
    if (m_style & TF_RIGHT)
        retval -= excess_width;
    else if (m_style & TF_CENTER)
        retval -= excess_width / 2;

    int format = TextFormat();
    Pt text_extent = GetFont()->TextExtent(m_text.substr(GetLineData()[row].begin_idx, GetLineData()[row].extents.size()), format, 0, true);
    if (GetLineData()[row].justification == TF_LEFT) {
        retval += (m_vscroll && m_hscroll ? RightMargin() : 0);
    } else if (GetLineData()[row].justification == TF_RIGHT) {
        retval += m_contents_sz.x - text_extent.x + (m_vscroll && m_hscroll ? RightMargin() : 0);
    } else if (GetLineData()[row].justification == TF_CENTER) {
        retval += (m_contents_sz.x - text_extent.x + (m_vscroll && m_hscroll ? RightMargin() : 0)) / 2;
    }

    return retval;
}

int MultiEdit::CharXOffset(int row, int idx) const
{
    return (0 < idx ? GetLineData()[row].extents[idx - 1] : 0);
}

int MultiEdit::RowAt(int y) const
{
    int retval = 0;
    Uint32 format = TextFormat();
    y += m_first_row_shown;
    if (format & TF_TOP) {
        retval = y / GetFont()->Lineskip();
    } else { // TF_BOTTOM
        retval = (static_cast<int>(GetLineData().size()) - 1) - 
            (ClientDimensions().y + (m_vscroll && m_hscroll ? BottomMargin() : 0) - y - 1) / GetFont()->Lineskip();
    }
    return retval;
}

int MultiEdit::CharAt(int row, int x) const
{
    int retval = 0;
    x -= RowStartX(row);
    while (retval < static_cast<int>(GetLineData()[row].extents.size()) && GetLineData()[row].extents[retval] < x)
        ++retval;
    if (0 <= retval && retval < static_cast<int>(GetLineData()[row].extents.size())) {
        int prev_extent = retval ? GetLineData()[row].extents[retval - 1] : 0;
        int half_way = (prev_extent + GetLineData()[row].extents[retval]) / 2;
        if (half_way < x) // if the point is more than halfway across the character, put the cursor *after* the character
            ++retval;
    }
    return retval;
}

int MultiEdit::FirstVisibleRow() const
{
    return std::max(0, std::min(RowAt(0), static_cast<int>(GetLineData().size()) - 1));
}

int MultiEdit::LastVisibleRow() const
{
    return std::max(0, std::min(RowAt(ClientDimensions().y), static_cast<int>(GetLineData().size()) - 1));
}

int MultiEdit::FirstFullyVisibleRow() const
{
    int retval = RowAt(0);
    if (m_first_row_shown % GetFont()->Lineskip())
        ++retval;
    return std::max(0, std::min(retval, static_cast<int>(GetLineData().size()) - 1));
}

int MultiEdit::LastFullyVisibleRow() const
{
    int retval = RowAt(ClientDimensions().y);
    if ((m_first_row_shown + ClientDimensions().y + BottomMargin()) % GetFont()->Lineskip())
        --retval;
    return std::max(0, std::min(retval, static_cast<int>(GetLineData().size()) - 1));
}

int MultiEdit::FirstVisibleChar(int row) const
{
    return std::max(0, std::min(CharAt(row, 0), GetLineData()[row].end_idx));
}

int MultiEdit::LastVisibleChar(int row) const
{
    return std::max(0, std::min(CharAt(row, ClientDimensions().x), GetLineData()[row].end_idx));
}

pair<int, int> MultiEdit::HighCursorPos() const
{
    if (m_cursor_begin.first < m_cursor_end.first || 
        (m_cursor_begin.first == m_cursor_end.first && m_cursor_begin.second < m_cursor_end.second))
        return m_cursor_end;
    else
        return m_cursor_begin;
}

pair<int, int> MultiEdit::LowCursorPos() const
{
    if (m_cursor_begin.first < m_cursor_end.first || 
        (m_cursor_begin.first == m_cursor_end.first && m_cursor_begin.second < m_cursor_end.second))
        return m_cursor_begin;
    else
        return m_cursor_end;
}

Scroll* MultiEdit::NewVScroll(bool horz_scroll)
{
    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border
    Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    return new Scroll(cl_sz.x + GAP - SCROLL_WIDTH, -GAP, SCROLL_WIDTH, cl_sz.y + 2 * GAP - (horz_scroll ? SCROLL_WIDTH : 0), Scroll::VERTICAL, m_color, CLR_SHADOW);
}

Scroll* MultiEdit::NewHScroll(bool vert_scroll)
{
    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border
    Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    return new Scroll(-GAP, cl_sz.y + GAP - SCROLL_WIDTH, cl_sz.x + 2 * GAP - (vert_scroll ? SCROLL_WIDTH : 0), SCROLL_WIDTH, Scroll::HORIZONTAL, m_color, CLR_SHADOW);
}

void MultiEdit::RecreateScrolls()
{
    delete m_vscroll;
    delete m_hscroll;
    m_vscroll = m_hscroll = 0;
    AdjustScrolls();
}

void MultiEdit::Init()
{
    ValidateStyle();
    SetTextFormat(m_style);
     SetText(m_text);
    SizeMove(UpperLeft(), LowerRight()); // do this to set up the scrolls, and in case INTEGRAL_HEIGHT is in effect
}

void MultiEdit::ValidateStyle()
{
    if (m_style & TERMINAL_STYLE) {
        m_style &= ~(TF_TOP | TF_VCENTER);
        m_style |= TF_BOTTOM;
    } else {
        m_style &= ~(TF_VCENTER | TF_BOTTOM);
        m_style |= TF_TOP;
    }

    if (!(m_style & (TF_LEFT | TF_CENTER | TF_RIGHT)))
        m_style |= TF_LEFT;

    if (m_style & (TF_LINEWRAP | TF_WORDBREAK)) {
        m_style |= NO_HSCROLL;
    }
}

void MultiEdit::ClearSelected()
{
    int idx_1 = StringIndexOf(m_cursor_begin.first, m_cursor_begin.second);
    int idx_2 = StringIndexOf(m_cursor_end.first, m_cursor_end.second);
    m_cursor_begin = m_cursor_end = LowCursorPos();
    Erase(idx_1 < idx_2 ? idx_1 : idx_2, std::abs(idx_1 - idx_2));
}

void MultiEdit::AdjustView()
{
    Pt cl_sz = ClientDimensions();
    Uint32 format = TextFormat();
    int excess_width = m_contents_sz.x - cl_sz.x;
    int excess_height = m_contents_sz.y - cl_sz.y;
    int horz_min = 0;            // these are default values for TF_LEFT and TF_TOP
    int horz_max = excess_width;
    int vert_min = 0;
    int vert_max = excess_height;
    
    if (format & TF_RIGHT) {
        horz_min = -excess_width;
        horz_max = horz_min + m_contents_sz.x;
    } else if (format & TF_CENTER) {
        horz_min = -excess_width / 2;
        horz_max = horz_min + m_contents_sz.x;
    }
    if (format & TF_BOTTOM) {
        vert_min = -excess_height;
        vert_max = vert_min + m_contents_sz.y;
    }

    // make sure that m_first_row_shown and m_first_col_shown are within sane bounds
    if (GetLineData().empty()) {
        m_first_col_shown = m_first_row_shown = 0;
    } else  {
        if (excess_width <= 0 || !m_hscroll)
            m_first_col_shown = 0;
        else
            m_hscroll->ScrollTo(std::max(horz_min, std::min(m_first_col_shown, horz_max)));

        if (excess_height <= 0 || !m_vscroll)
            m_first_row_shown = 0;
        else
            m_vscroll->ScrollTo(std::max(vert_min, std::min(m_first_row_shown, vert_max)));
    }

    // adjust m_first_row_shown position to bring the cursor into view
    int first_fully_vis_row = FirstFullyVisibleRow();
    if (m_cursor_end.first < first_fully_vis_row && m_vscroll) {
        int diff = (first_fully_vis_row - m_cursor_end.first);
        m_vscroll->ScrollTo(std::max(vert_min, m_first_row_shown - GetFont()->Lineskip() * diff));
    }
    int last_fully_vis_row = LastFullyVisibleRow();
    if (last_fully_vis_row < m_cursor_end.first && m_vscroll) {
        int diff = (m_cursor_end.first - last_fully_vis_row);
        m_vscroll->ScrollTo(std::min(m_first_row_shown + GetFont()->Lineskip() * diff, vert_max));
    }

    // adjust m_first_col_shown position to bring the cursor into view
    int first_visible_char = FirstVisibleChar(m_cursor_end.first);
    int last_visible_char = LastVisibleChar(m_cursor_end.first);
    int client_char_posn = RowStartX(m_cursor_end.first) + CharXOffset(m_cursor_end.first, m_cursor_end.second);
    if (client_char_posn < 0 && m_hscroll) { // if the caret is at a place left of the current visible area
        if (first_visible_char - m_cursor_end.second < 5) { // if the caret is fewer than five characters before first_visible_char
            // try to move the caret by five characters
            int five_char_distance = GetLineData()[m_cursor_end.first].extents[first_visible_char] - 
                                     GetLineData()[m_cursor_end.first].extents[(5 < first_visible_char) ? first_visible_char - 5 : 0];
            m_hscroll->ScrollTo(m_first_col_shown - five_char_distance);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(horz_min + m_first_col_shown + client_char_posn);
        }
    } else if (cl_sz.x <= client_char_posn && m_hscroll) { // if the caret is moving to a place right of the current visible area
        if (m_cursor_end.second - last_visible_char < 5) { // if the caret is fewer than five characters after last_visible_char
            // try to move the caret by five characters
            int last_char_of_line = static_cast<int>(GetLineData()[m_cursor_end.first].extents.size()) - 1;
            int five_char_distance = GetLineData()[m_cursor_end.first].extents[(last_visible_char + 5 < last_char_of_line ? last_visible_char + 5 : last_char_of_line)] - 
                                     GetLineData()[m_cursor_end.first].extents[last_visible_char];
            m_hscroll->ScrollTo(m_first_col_shown + five_char_distance);
        } else { // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_hscroll->ScrollTo(std::min(horz_min + m_first_col_shown + client_char_posn, horz_max));
        }
    }
}

void MultiEdit::AdjustScrolls()
{
    bool need_vert = false, need_horz = false;

    // this client area calculation disregards the thickness of scrolls
    Pt cl_sz = Edit::ClientLowerRight() - Edit::ClientUpperLeft();
    Pt contents_sz = GetFont()->TextExtent(m_text, TextFormat(), (TextFormat() & (TF_WORDBREAK | TF_LINEWRAP)) ? cl_sz.x : 0, true);
    contents_sz.y = GetLineData().size() * GetFont()->Lineskip();
    int excess_width = contents_sz.x - cl_sz.x;

    need_vert = (!(m_style & NO_VSCROLL) && (contents_sz.y > cl_sz.y ||
                 (contents_sz.y > cl_sz.y - SCROLL_WIDTH && contents_sz.x > cl_sz.x - SCROLL_WIDTH)));
    need_horz = (!(m_style & NO_HSCROLL) && (contents_sz.x > cl_sz.x ||
                 (contents_sz.x > cl_sz.x - SCROLL_WIDTH && contents_sz.y > cl_sz.y - SCROLL_WIDTH)));

    Pt orig_cl_sz = ClientDimensions();

    const int GAP = PIXEL_MARGIN - 2; // the space between the client area and the border

    int vscroll_min = (m_style & TERMINAL_STYLE) ? (cl_sz.y - contents_sz.y) : 0;
    int hscroll_min = 0; // default values for TF_LEFT
    if (m_style & TF_RIGHT) {
        hscroll_min = -excess_width;
    } else if (m_style & TF_CENTER) {
        hscroll_min = -excess_width / 2;
    }
    int vscroll_max = vscroll_min + contents_sz.y - 1;
    int hscroll_max = hscroll_min + contents_sz.x - 1;

    if (m_vscroll) { // if scroll already exists...
        if (!need_vert) { // remove scroll
            DeleteChild(m_vscroll);
            m_vscroll = 0;
        } else { // ensure vertical scroll has the right logical and physical dimensions
            m_vscroll->SizeScroll(vscroll_min, vscroll_max, cl_sz.y / 8, cl_sz.y - (need_horz ? SCROLL_WIDTH : 0));
            int scroll_x = cl_sz.x + GAP - SCROLL_WIDTH;
            int scroll_y = -GAP;
            m_vscroll->SizeMove(scroll_x, scroll_y, scroll_x + SCROLL_WIDTH, scroll_y + cl_sz.y + 2 * GAP - (need_horz ? SCROLL_WIDTH : 0));
        }
    } else if (!m_vscroll && need_vert) { // if scroll doesn't exist but is needed
        m_vscroll = NewVScroll(need_horz);
        m_vscroll->SizeScroll(vscroll_min, vscroll_max, cl_sz.y / 8, cl_sz.y - (need_horz ? SCROLL_WIDTH : 0));
        AttachChild(m_vscroll);
        Connect(m_vscroll->ScrolledSignal(), &MultiEdit::VScrolled, this);
    }

    if (m_hscroll) { // if scroll already exists...
        if (!need_horz) { // remove scroll
            DeleteChild(m_hscroll);
            m_hscroll = 0;
        } else { // ensure horizontal scroll has the right logical and physical dimensions
            m_hscroll->SizeScroll(hscroll_min, hscroll_max, cl_sz.x / 8, cl_sz.x - (need_vert ? SCROLL_WIDTH : 0));
            int scroll_x = -GAP;
            int scroll_y = cl_sz.y + GAP - SCROLL_WIDTH;
            m_hscroll->SizeMove(scroll_x, scroll_y, scroll_x + cl_sz.x + 2 * GAP - (need_vert ? SCROLL_WIDTH : 0), scroll_y + SCROLL_WIDTH);
        }
    } else if (!m_hscroll && need_horz) { // if scroll doesn't exist but is needed
        m_hscroll = NewHScroll(need_vert);
        m_hscroll->SizeScroll(hscroll_min, hscroll_max, cl_sz.x / 8, cl_sz.x - (need_vert ? SCROLL_WIDTH : 0));
        AttachChild(m_hscroll);
        Connect(m_hscroll->ScrolledSignal(), &MultiEdit::HScrolled, this);
    }

    // if the new client dimensions changed after adjusting the scrolls, they are unequal to the extent of the text,
    // and there is some kind of wrapping going on, we need to re-SetText()
    Pt new_cl_sz = ClientDimensions();
    if (orig_cl_sz != new_cl_sz && (new_cl_sz.x != contents_sz.x || new_cl_sz.y != contents_sz.y) && 
        (m_style & (TF_WORDBREAK | TF_LINEWRAP))) {
        SetText(m_text);
    }
}

void MultiEdit::VScrolled(int upper, int lower, int range_upper, int range_lower)
{
    m_first_row_shown = upper;
}

void MultiEdit::HScrolled(int upper, int lower, int range_upper, int range_lower)
{
    m_first_col_shown = upper;
}

} // namespace GG

