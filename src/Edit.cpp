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

#include <GG/Edit.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    struct InRange
    {
        InRange(int value) : m_value(value) {}
        bool operator()(const std::pair<int, int>& p) const
            { return p.first < m_value && m_value < p.second; }
        const int m_value;
    };

    int HeightFromFont(const boost::shared_ptr<Font>& font, int pixel_margin)
    {  return font->Height() + 2 * pixel_margin; }
}

////////////////////////////////////////////////
// GG::Edit
////////////////////////////////////////////////
// static(s)
const int Edit::PIXEL_MARGIN = 5;

Edit::Edit() :
    TextControl(),
    m_first_char_shown(0),
    m_recently_edited(false),
    m_last_button_down_time(0),
    m_in_double_click_mode(false)
{}

Edit::Edit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = CLICKABLE*/) :
    TextControl(x, y, w, HeightFromFont(font, PIXEL_MARGIN), str, font, text_color, FORMAT_LEFT | FORMAT_IGNORETAGS, flags),
    m_cursor_pos(0, 0),
    m_first_char_shown(0),
    m_int_color(interior),
    m_hilite_color(CLR_SHADOW),
    m_sel_text_color(CLR_WHITE),
    m_recently_edited(false),
    m_last_button_down_time(0),
    m_in_double_click_mode(false)
{
    SetColor(color);
}

Pt Edit::MinUsableSize() const
{ return Pt(4 * PIXEL_MARGIN, HeightFromFont(GetFont(), PIXEL_MARGIN)); }

Pt Edit::ClientUpperLeft() const
{ return UpperLeft() + Pt(PIXEL_MARGIN, PIXEL_MARGIN); }

Pt Edit::ClientLowerRight() const
{ return LowerRight() - Pt(PIXEL_MARGIN, PIXEL_MARGIN); }

const std::pair<int, int>& Edit::CursorPosn() const
{ return m_cursor_pos; }

Clr Edit::InteriorColor() const
{ return m_int_color; }

Clr Edit::HiliteColor() const
{ return m_hilite_color; }

Clr Edit::SelectedTextColor() const
{ return m_sel_text_color; }

void Edit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr sel_text_color_to_use = Disabled() ? DisabledColor(m_sel_text_color) : m_sel_text_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;
    Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, 2);

    BeginScissorClipping(client_ul.x - 1, client_ul.y, client_lr.x, client_lr.y);
    
    const std::vector<Font::LineData::CharData>& char_data = GetLineData()[0].char_data;
    int first_char_offset = FirstCharOffset();
    int text_y_pos = ul.y + static_cast<int>(((lr.y - ul.y) - GetFont()->Height()) / 2.0 + 0.5);
    int last_visible_char = LastVisibleChar();
    if (MultiSelected())   { // if one or more chars are selected, hilite, then draw the range in the selected-text color
        int low_cursor_pos  = std::min(m_cursor_pos.first, m_cursor_pos.second);
        int high_cursor_pos = std::max(m_cursor_pos.first, m_cursor_pos.second);

        // draw hiliting
        Pt hilite_ul(client_ul.x + (low_cursor_pos < 1 ? 0 : static_cast<int>(char_data[low_cursor_pos - 1].extent)) - first_char_offset, client_ul.y),
        hilite_lr(client_ul.x + static_cast<int>(char_data[high_cursor_pos - 1].extent) - first_char_offset, client_lr.y);
        FlatRectangle(hilite_ul.x, hilite_ul.y, hilite_lr.x, hilite_lr.y, hilite_color_to_use, CLR_ZERO, 0);

        // idx0 to idx1 is unhilited, idx1 to idx2 is hilited, and idx2 to idx3 is unhilited; each range may be empty
        int idx0 = m_first_char_shown;
        int idx1 = std::max(low_cursor_pos, m_first_char_shown);
        int idx2 = std::min(high_cursor_pos, last_visible_char);
        int idx3 = last_visible_char;

        // draw text
        int text_x_pos = ul.x + PIXEL_MARGIN;
        glColor(text_color_to_use);
        text_x_pos += GetFont()->RenderText(text_x_pos, text_y_pos, WindowText().substr(idx0, idx1 - idx0));
        glColor(sel_text_color_to_use);
        text_x_pos += GetFont()->RenderText(text_x_pos, text_y_pos, WindowText().substr(idx1, idx2 - idx1));
        glColor(text_color_to_use);
        text_x_pos += GetFont()->RenderText(text_x_pos, text_y_pos, WindowText().substr(idx2, idx3 - idx2));
    } else { // no selected text
        glColor(text_color_to_use);
        GetFont()->RenderText(client_ul.x, text_y_pos, WindowText().substr(m_first_char_shown, last_visible_char - m_first_char_shown));
        if (GUI::GetGUI()->FocusWnd() == this) { // if we have focus, draw the caret as a simple vertical line
            int caret_x = ScreenPosOfChar(m_cursor_pos.second);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            glVertex2i(caret_x, client_ul.y);
            glVertex2i(caret_x, client_lr.y);
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }
    }

    EndScissorClipping();
}

void Edit::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        int click_xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // x coord of click within text space
        int idx = CharIndexOf(click_xpos);
        m_cursor_pos.first = m_cursor_pos.second = idx;
        std::pair<int, int> word_indices = GetDoubleButtonDownWordIndices(idx);
        if (word_indices.first != word_indices.second)
            m_cursor_pos = word_indices;
    }
}

void Edit::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        int xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // x coord for mouse position within text space
        int idx = CharIndexOf(xpos);
        if (m_in_double_click_mode) {
            std::pair<int, int> word_indices = GetDoubleButtonDownDragWordIndices(idx);
            if (word_indices.first == word_indices.second) {
                if (idx < m_double_click_cursor_pos.first) {
                    m_cursor_pos.second = idx;
                    m_cursor_pos.first = m_double_click_cursor_pos.second;
                } else if (m_double_click_cursor_pos.second < idx) {
                    m_cursor_pos.second = idx;
                    m_cursor_pos.first = m_double_click_cursor_pos.first;
                } else {
                    m_cursor_pos = m_double_click_cursor_pos;
                }
            } else {
                if (word_indices.first <= m_double_click_cursor_pos.first) {
                    m_cursor_pos.second = word_indices.first;
                    m_cursor_pos.first = m_double_click_cursor_pos.second;
                } else {
                    m_cursor_pos.second = word_indices.second;
                    m_cursor_pos.first = m_double_click_cursor_pos.first;
                }
            }
        } else {
            // when a single-click drag occurs, move m_cursor_pos.second to where the mouse is, which selects a range of characters
            m_cursor_pos.second = idx;
            if (xpos < 0 || Size().x - 2 * PIXEL_MARGIN < xpos) // if we're dragging past the currently visible text
                AdjustView();
        }
    }
}

void Edit::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ ClearDoubleButtonDownMode(); }

void Edit::KeyPress(Key key, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
        bool emit_signal = false;

        switch (key) {
        case GGK_HOME:
            m_first_char_shown = 0;
            if (shift_down)
                m_cursor_pos.second = 0;
            else
                m_cursor_pos.second = m_cursor_pos.first = 0;
            break;
        case GGK_LEFT:
            if (MultiSelected() && !shift_down) {
                m_cursor_pos.second = m_cursor_pos.first = std::min(m_cursor_pos.first, m_cursor_pos.second);
            } else if (0 < m_cursor_pos.second) {
                --m_cursor_pos.second;
                int extent = GetLineData()[0].char_data[m_cursor_pos.second].extent;
                while (0 < m_cursor_pos.second && extent == GetLineData()[0].char_data[m_cursor_pos.second - 1].extent)
                    --m_cursor_pos.second;
                if (!shift_down)
                    m_cursor_pos.first = m_cursor_pos.second;
            }
            AdjustView();
            break;
        case GGK_RIGHT:
            if (MultiSelected() && !shift_down) {
                m_cursor_pos.second = m_cursor_pos.first = std::max(m_cursor_pos.first, m_cursor_pos.second);
            } else if (m_cursor_pos.second < Length()) {
                int extent = GetLineData()[0].char_data[m_cursor_pos.second].extent;
                while (m_cursor_pos.second < Length() && extent == GetLineData()[0].char_data[m_cursor_pos.second].extent)
                    ++m_cursor_pos.second;
                if (!shift_down)
                    m_cursor_pos.first = m_cursor_pos.second;
            }
            AdjustView();
            break;
        case GGK_END:
            if (shift_down)
                m_cursor_pos.second = Length();
            else
                m_cursor_pos.second = m_cursor_pos.first = Length();
            AdjustView();
            break;
        case GGK_BACKSPACE:
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (0 < m_cursor_pos.first) {
                m_cursor_pos.second = --m_cursor_pos.first;
                Erase(m_cursor_pos.first);
                emit_signal = true;
            }
            AdjustView();
            break;
        case GGK_DELETE:
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (m_cursor_pos.first < Length()) {
                Erase(m_cursor_pos.first);
                emit_signal = true;
            }
            AdjustView();
            break;
        case GGK_RETURN:
        case GGK_KP_ENTER:
            FocusUpdateSignal(WindowText());
            TextControl::KeyPress(key, mod_keys);
            m_recently_edited = false;
            break;
        default:
            // only process it if it's a printable character, and no significant modifiers are in use
            KeypadKeyToPrintable(key, mod_keys);
            if (key < GGK_DELETE && isprint(key) && !(mod_keys & (MOD_KEY_CTRL | MOD_KEY_ALT | MOD_KEY_META | MOD_KEY_MODE))) {
                if (MultiSelected())
                    ClearSelected();
                Insert(m_cursor_pos.first, key);                // insert character after caret
                m_cursor_pos.second = ++m_cursor_pos.first;     // then move the caret fwd one
                emit_signal = true;                             // notify parent that text has changed
                if (LastVisibleChar() <= m_cursor_pos.first)    // when we over-run our writing space with typing, scroll the window
                    AdjustView();
            } else {
                TextControl::KeyPress(key, mod_keys);
            }
            break;
        }
        if (emit_signal)
            EditedSignal(WindowText());
    } else {
        TextControl::KeyPress(key, mod_keys);
    }
}

void Edit::GainingFocus()
{ m_recently_edited = false; }

void Edit::LosingFocus()
{
    if (m_recently_edited)
        FocusUpdateSignal(WindowText());
}

void Edit::SetColor(Clr c)
{ Control::SetColor(c); }

void Edit::SetInteriorColor(Clr c)
{ m_int_color = c; }

void Edit::SetHiliteColor(Clr c)
{ m_hilite_color = c; }

void Edit::SetSelectedTextColor(Clr c)
{ m_sel_text_color = c; }

void Edit::SelectAll()
{
    m_cursor_pos.first = Length(); 
    m_cursor_pos.second = 0;
    AdjustView();
}

void Edit::SelectRange(int from, int to)
{
    if (from < to) {
        m_cursor_pos.first = std::max(0, from);
        m_cursor_pos.second = std::min(to, Length());
    } else {
        m_cursor_pos.first = std::min(from, Length());
        m_cursor_pos.second = std::max(0, to);
    }
    AdjustView();
}

void Edit::SetText(const std::string& str)
{
    TextControl::SetText(str);
    m_cursor_pos.second = m_cursor_pos.first; // eliminate any hiliting

    // make sure the change in text did not make the cursor or view position invalid
    if (str.empty() || GetLineData().empty() || static_cast<int>(GetLineData()[0].char_data.size()) < m_cursor_pos.first) {
        m_first_char_shown = 0;
        m_cursor_pos = std::make_pair(0, 0);
    }

    m_recently_edited = true;

    EditedSignal(str);
}

void Edit::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    editor->Label("Edit");
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Highlighting Color", m_hilite_color);
    editor->Attribute("Selected Text Color", m_sel_text_color);
}

bool Edit::MultiSelected() const
{ return m_cursor_pos.first != m_cursor_pos.second; }

int Edit::FirstCharShown() const
{ return m_first_char_shown; }

bool Edit::RecentlyEdited() const
{ return m_recently_edited; }

int Edit::CharIndexOf(int x) const
{
    int retval;
    int first_char_offset = FirstCharOffset();
    for (retval = 0; retval < Length(); ++retval) {
        int curr_extent;
        if (x + first_char_offset <= (curr_extent = GetLineData()[0].char_data[retval].extent)) { // the point falls within the character at index retval
            int prev_extent = retval ? GetLineData()[0].char_data[retval - 1].extent : 0;
            int half_way = (prev_extent + curr_extent) / 2;
            if (half_way <= x + first_char_offset) // if the point is more than halfway across the character, put the cursor *after* the character
                ++retval;
            break;
        }
    }
    return retval;
}

int Edit::FirstCharOffset() const
{ return (m_first_char_shown ? GetLineData()[0].char_data[m_first_char_shown - 1].extent : 0); }

int Edit::ScreenPosOfChar(int idx) const
{
    int first_char_offset = FirstCharOffset();
    return UpperLeft().x + PIXEL_MARGIN + ((idx ? static_cast<int>(GetLineData()[0].char_data[idx - 1].extent) : 0) - first_char_offset);
}

int Edit::LastVisibleChar() const
{
    int first_char_offset = FirstCharOffset();
    int retval = m_first_char_shown;
    for ( ; retval < Length(); ++retval) {
        if (Size().x - 2 * PIXEL_MARGIN <= (retval ? GetLineData()[0].char_data[retval - 1].extent : 0) - first_char_offset)
            break;
    }
    return retval;
}

int Edit::LastButtonDownTime() const
{ return m_last_button_down_time; }

bool Edit::InDoubleButtonDownMode() const
{ return m_in_double_click_mode; }

std::pair<int, int> Edit::DoubleButtonDownCursorPos() const
{ return m_double_click_cursor_pos; }

std::pair<int, int> Edit::GetDoubleButtonDownWordIndices(int char_index)
{
    int ticks = GUI::GetGUI()->Ticks();
    if (ticks - m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    m_last_button_down_time = ticks;
    m_double_click_cursor_pos = std::pair<int, int>();
    if (m_in_double_click_mode) {
        std::set<std::pair<int, int> > words = GUI::GetGUI()->FindWords(WindowText());
        std::set<std::pair<int, int> >::const_iterator it =
            std::find_if(words.begin(), words.end(), InRange(char_index));
        if (it != words.end())
            m_double_click_cursor_pos = *it;
    }
    return m_double_click_cursor_pos;
}

std::pair<int, int> Edit::GetDoubleButtonDownDragWordIndices(int char_index)
{
    std::pair<int, int> retval;
    std::set<std::pair<int, int> > words = GUI::GetGUI()->FindWords(WindowText());
    std::set<std::pair<int, int> >::const_iterator it =
        std::find_if(words.begin(), words.end(), InRange(char_index));
    if (it != words.end())
        retval = *it;
    return retval;
}

void Edit::ClearDoubleButtonDownMode()
{ m_in_double_click_mode = false; }

void Edit::ClearSelected()
{
    int erase_start = std::min(m_cursor_pos.first, m_cursor_pos.second);
    int erase_amount = abs(m_cursor_pos.second - m_cursor_pos.first);
    if (m_cursor_pos.first < m_cursor_pos.second)
        m_cursor_pos.second = m_cursor_pos.first;
    else
        m_cursor_pos.first = m_cursor_pos.second;
    Erase(erase_start, erase_amount);

    // make sure deletion has not left m_first_char_shown in an out-of-bounds position
    if (GetLineData()[0].char_data.empty())
        m_first_char_shown = 0;
    else if (static_cast<int>(GetLineData()[0].char_data.size()) <= m_first_char_shown)
        m_first_char_shown = GetLineData()[0].char_data.size() - 1;
}

void Edit::AdjustView()
{
    int text_space = Size().x - 2 * PIXEL_MARGIN;
    int first_char_offset = FirstCharOffset();
    if (m_cursor_pos.second < m_first_char_shown) { // if the caret is at a place left of the current visible area
        if (m_first_char_shown - m_cursor_pos.second < 5) // if the caret is less than five characters before m_first_char_shown
            m_first_char_shown = (0 <= m_first_char_shown - 5) ? m_first_char_shown - 5 : 0; // try to move the caret by five characters
        else // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_first_char_shown = m_cursor_pos.second;
    } else if (text_space <= (m_cursor_pos.second ? GetLineData()[0].char_data[m_cursor_pos.second - 1].extent : 0) - first_char_offset) { // if the caret is moving to a place right of the current visible area
        // try to move the text by five characters, or to the end if caret is at a location before the end - 5th character
        int last_idx_to_use = (m_cursor_pos.second + 5 <= Length() - 1) ? m_cursor_pos.second + 5 : Length() - 1; // try to move the caret by five characters
        const std::vector<Font::LineData::CharData>& char_data = GetLineData()[0].char_data;
        // number of pixels that the caret position overruns the right side of text area
        int pixels_to_move = (char_data[last_idx_to_use].extent - first_char_offset) - text_space;
        if (last_idx_to_use == Length() - 1) // if the caret is at the very end of the string, add the length of some spaces
            pixels_to_move += (m_cursor_pos.second + 5 - Length() - 1) * GetFont()->SpaceWidth();
        int move_to = m_first_char_shown;
        while (move_to < static_cast<int>(char_data.size()) &&
               char_data[move_to].extent - first_char_offset < pixels_to_move)
            ++move_to;
        m_first_char_shown = move_to;
    }
}
