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

#include "GGEdit.h"
#include "GGApp.h"
#include "GGDrawUtil.h"

namespace GG {

namespace {
const int PIXEL_MARGIN = 5;
}

////////////////////////////////////////////////
// GG::Edit
////////////////////////////////////////////////
Edit::Edit(int x, int y, int w, int h, const string& str, const shared_ptr<Font>& font, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        TextControl(x, y, w, h, str, TF_LEFT, text_color, flags),
        m_cursor_pos(0, 0),
        m_first_char_shown(0),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_sel_text_color(CLR_WHITE),
        m_previous_text(str),
        m_font(font)
{
    SetColor(color);
    m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);
}

Edit::Edit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        TextControl(x, y, w, h, str, TF_LEFT, text_color, flags),
        m_cursor_pos(0, 0),
        m_first_char_shown(0),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_sel_text_color(CLR_WHITE),
        m_previous_text(str),
        m_font(App::GetApp()->GetFont(font_filename, pts))
{
    SetColor(color);
    m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);
}

Edit::Edit(int x, int y, int w, const string& str, const shared_ptr<Font>& font, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        TextControl(x, y, w, font->Height() + 2 * PIXEL_MARGIN, str, TF_LEFT, text_color, flags),
        m_cursor_pos(0, 0),
        m_first_char_shown(0),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_sel_text_color(CLR_WHITE),
        m_previous_text(str),
        m_font(font)
{
    SetColor(color);
    m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);
}

Edit::Edit(int x, int y, int w, const string& str, const string& font_filename, int pts, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Uint32 flags/* = CLICKABLE | DRAG_KEEPER*/) :
        TextControl(x, y, w, App::GetApp()->GetFont(font_filename, pts)->Height() + 2 * PIXEL_MARGIN, str, TF_LEFT, text_color, flags),
        m_cursor_pos(0, 0),
        m_first_char_shown(0),
        m_int_color(interior),
        m_hilite_color(CLR_SHADOW),
        m_sel_text_color(CLR_WHITE),
        m_previous_text(str),
        m_font(App::GetApp()->GetFont(font_filename, pts))
{
    SetColor(color);
    m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);
}

Edit::Edit(const XMLElement& elem) :
        TextControl(elem.Child("GG::TextControl"))
{
    if (elem.Tag() != "GG::Edit")
        throw std::invalid_argument("Attempted to construct a GG::Edit from an XMLElement that had a tag other than \"GG::Edit\"");

    const XMLElement* curr_elem = &elem.Child("m_cursor_pos");
    m_cursor_pos.first = lexical_cast<int>(curr_elem->Attribute("first"));
    m_cursor_pos.second = lexical_cast<int>(curr_elem->Attribute("second"));

    curr_elem = &elem.Child("m_first_char_shown");
    m_first_char_shown = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_int_color");
    m_int_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_hilite_color");
    m_hilite_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_sel_text_color");
    m_sel_text_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_font").Child("GG::Font");
    string font_filename = curr_elem->Child("m_font_filename").Text();
    int pts = lexical_cast<int>(curr_elem->Child("m_pt_sz").Attribute("value"));
    m_font = App::GetApp()->GetFont(font_filename, pts);

    if (m_font)
        m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);

    m_previous_text = WindowText();
}

Pt Edit::ClientUpperLeft() const
{
    return UpperLeft() + Pt(PIXEL_MARGIN, PIXEL_MARGIN);
}

Pt Edit::ClientLowerRight() const
{
    return LowerRight() - Pt(PIXEL_MARGIN, PIXEL_MARGIN);
}

int Edit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr sel_text_color_to_use = Disabled() ? DisabledColor(m_sel_text_color) : m_sel_text_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;
    Clr text_color_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, 2);

    // clip text to viewable area, and save old scissor state, if any
    bool disable_scissor = !glIsEnabled(GL_SCISSOR_TEST);
    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(ul.x + PIXEL_MARGIN, App::GetApp()->AppHeight() - lr.y, lr.x - ul.x - 2 * PIXEL_MARGIN, lr.y - ul.y);

    vector<int>& extents = m_line_data[0].extents;
    int first_char_offset = FirstCharOffset();
    int text_y_pos = ul.y + int(((lr.y - ul.y) - m_font->Height()) / 2.0f + 0.5f);
    int last_visible_char = LastVisibleChar();
    if (MultiSelected())   { // if one or more chars are selected, hilite, then draw the range in the selected-text color
        int low_cursor_pos  = (m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.first : m_cursor_pos.second);
        int high_cursor_pos = (m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.second : m_cursor_pos.first);

        // draw hiliting
        Pt hilite_ul(client_ul.x + (low_cursor_pos < 1 ? 0 : int(extents[low_cursor_pos - 1])) - first_char_offset, client_ul.y),
        hilite_lr(client_ul.x + int(extents[high_cursor_pos - 1]) - first_char_offset, client_lr.y);
        FlatRectangle(hilite_ul.x, hilite_ul.y, hilite_lr.x, hilite_lr.y, hilite_color_to_use, CLR_ZERO, 0);

        // idx0 to idx1 is unhilited, idx1 to idx2 is hilited, and idx2 to idx3 is unhilited; each range may be empty
        int idx0 = m_first_char_shown;
        int idx1 = std::max(low_cursor_pos, m_first_char_shown);
        int idx2 = std::min(high_cursor_pos, last_visible_char);
        int idx3 = last_visible_char;

        // draw text
        int text_x_pos = ul.x + PIXEL_MARGIN;
        glColor4ubv(text_color_to_use.v);
        text_x_pos += m_font->RenderText(text_x_pos, text_y_pos, m_text.substr(idx0, idx1 - idx0));
        glColor4ubv(sel_text_color_to_use.v);
        text_x_pos += m_font->RenderText(text_x_pos, text_y_pos, m_text.substr(idx1, idx2 - idx1));
        glColor4ubv(text_color_to_use.v);
        text_x_pos += m_font->RenderText(text_x_pos, text_y_pos, m_text.substr(idx2, idx3 - idx2));
    } else { // no selected text
        glColor4ubv(text_color_to_use.v);
        m_font->RenderText(client_ul.x, text_y_pos, m_text.substr(m_first_char_shown, last_visible_char - m_first_char_shown));
        if (App::GetApp()->FocusWnd() == this) { // if we have focus, draw the caret as a simple vertical line
            int caret_x = ScreenPosOfChar(m_cursor_pos.first);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            glVertex2i(caret_x, client_ul.y);
            glVertex2i(caret_x, client_lr.y);
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

int Edit::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
        // when a button press occurs, record the character position under the cursor, and remove any previous selection range
        int click_xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // x coord of click within text space
        int idx = CharIndexOf(click_xpos);
        m_cursor_pos.first = m_cursor_pos.second = idx;
    }
    return 1;
}

int Edit::LDrag(const Pt& pt, const Pt& move, Uint32 keys)
{
    if (!Disabled()) {
        // when a drag occurs, move m_cursor_pos.second to where the mouse is, which selects a range of characters
        int click_xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // screen-pixel x coord for click
        m_cursor_pos.second = CharIndexOf(click_xpos);
        if (click_xpos < 0 || click_xpos > WindowDimensions().x - 2 * PIXEL_MARGIN) // if we're dragging past the currently visible text
            AdjustView();
    }
    return 1;
}

int Edit::Keypress(Key key, Uint32 key_mods)
{
    bool shift_down = key_mods & (KMOD_LSHIFT | KMOD_RSHIFT);
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
            m_cursor_pos.second = m_cursor_pos.first = m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.first : m_cursor_pos.second;
        } else if (m_cursor_pos.second > 0) {
            --m_cursor_pos.second;
            int extent = m_line_data[0].extents[m_cursor_pos.second];
            while (m_cursor_pos.second > 0 && extent == m_line_data[0].extents[m_cursor_pos.second - 1])
                --m_cursor_pos.second;
            if (!shift_down)
                m_cursor_pos.first = m_cursor_pos.second;
        }
        AdjustView();
        break;
    case GGK_RIGHT:
        if (MultiSelected() && !shift_down) {
            m_cursor_pos.second = m_cursor_pos.first = m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.second : m_cursor_pos.first;
        } else if (m_cursor_pos.second < Length()) {
            int extent = m_line_data[0].extents[m_cursor_pos.second];
            while (extent == m_line_data[0].extents[++m_cursor_pos.second]) ;
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
        } else if (m_cursor_pos.first > 0) {
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
    default:
        if (isprint(key)) { // only process it if it's a printable character
            if (MultiSelected()) ClearSelected();
            Insert(m_cursor_pos.first, key);                // insert character after caret
            m_cursor_pos.second = ++m_cursor_pos.first;     // then move the caret fwd one
            emit_signal = true;                             // notify parent that text has changed
            if (m_cursor_pos.first >= LastVisibleChar())    // when we over-run our writing space with typing, scroll the window
                AdjustView();
        } else if (Parent()) {
            Parent()->Keypress(key, key_mods);
        }
        break;
    }
    if (emit_signal) m_edited_sig(m_text);

    return 1;
}

int Edit::GainingFocus()
{
    m_previous_text = WindowText();
    return 1;
}

int Edit::LosingFocus()
{
    if (m_previous_text != WindowText())
        m_focus_update_sig(WindowText());
    return 1;
}

void Edit::SetText(const string& str)
{
    Control::m_text = str;
    m_cursor_pos.second = m_cursor_pos.first; // eliminate any hiliting
    m_font->DetermineLines(WindowText(), m_format, WindowDimensions().x, m_line_data);

    // make sure the change in text did not make the cursor or view position invalid
    if (m_line_data.empty() || m_cursor_pos.first  > static_cast<int>(m_line_data[0].extents.size())) {
        m_cursor_pos = pair<int,int>(0,0);
        AdjustView();
    }

    m_edited_sig(str);
}

XMLElement Edit::XMLEncode() const
{
    XMLElement retval("GG::Edit");
    retval.AppendChild(TextControl::XMLEncode());

    XMLElement temp;

    temp = XMLElement("m_cursor_pos");
    temp.SetAttribute("first", lexical_cast<string>(m_cursor_pos.first));
    temp.SetAttribute("second", lexical_cast<string>(m_cursor_pos.second));
    retval.AppendChild(temp);

    temp = XMLElement("m_first_char_shown");
    temp.SetAttribute("value", lexical_cast<string>(m_first_char_shown));
    retval.AppendChild(temp);

    temp = XMLElement("m_int_color");
    temp.AppendChild(m_int_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_hilite_color");
    temp.AppendChild(m_hilite_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_sel_text_color");
    temp.AppendChild(m_sel_text_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_line_data"); // leaving this empty; it will be recalculated at recreation time
    retval.AppendChild(temp);

    temp = XMLElement("m_font");
    temp.AppendChild(m_font->XMLEncode());
    retval.AppendChild(temp);

    return retval;
}

void Edit::ClearSelected()
{
    int erase_start = m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.first : m_cursor_pos.second;
    int erase_amount = abs(m_cursor_pos.second - m_cursor_pos.first);
    m_cursor_pos.first < m_cursor_pos.second ? m_cursor_pos.second = m_cursor_pos.first : m_cursor_pos.first = m_cursor_pos.second;
    Erase(erase_start, erase_amount);

    // make sure deletion has not left m_first_char_shown in an out-of-bounds position
    if (m_line_data.empty())
        m_first_char_shown = 0;
    else if (m_first_char_shown >= static_cast<int>(m_line_data[0].extents.size()))
        m_first_char_shown = m_line_data[0].extents.size() ? m_line_data[0].extents.size() - 1 : 0;
}

void Edit::AdjustView()
{
    int text_space = WindowDimensions().x - 2 * PIXEL_MARGIN;
    int first_char_offset = FirstCharOffset();
    if (m_cursor_pos.second < m_first_char_shown) { // if the caret is at a place left of the current visible area
        if (m_first_char_shown - m_cursor_pos.second < 5) // if the caret is less than five characters before m_first_char_shown
            m_first_char_shown = (m_first_char_shown - 5 >= 0) ? m_first_char_shown - 5 : 0; // try to move the caret by five characters
        else // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_first_char_shown = m_cursor_pos.second;
    } else if ((m_cursor_pos.second ? m_line_data[0].extents[m_cursor_pos.second - 1] : 0) - first_char_offset >= text_space) { // if the caret is moving to a place right of the current visible area
        // try to move the text by five characters, or to the end if caret is at a location before the end - 5th character
        int last_idx_to_use = (m_cursor_pos.second + 5 <= Length() - 1) ? m_cursor_pos.second + 5 : Length() - 1; // try to move the caret by five characters
        vector<int>& extents = m_line_data[0].extents;
        // number of pixels that the caret position overruns the right side of text area
        int pixels_to_move = (extents[last_idx_to_use] - first_char_offset) - text_space;
        if (last_idx_to_use == Length() - 1) // if the caret is at the very end of the string, add the length of some spaces
            pixels_to_move += (m_cursor_pos.second + 5 - Length() - 1) * m_font->SpaceWidth();
        int move_to = m_first_char_shown;
        while (pixels_to_move > extents[move_to] - first_char_offset)
            ++move_to;
        m_first_char_shown = move_to;
    }
}

int Edit::CharIndexOf(int x) const
{
    int retval;
    int first_char_offset = FirstCharOffset();
    for (retval = 0; retval < Length(); ++retval) {
        int curr_extent;
        if (x + first_char_offset <= (curr_extent = m_line_data[0].extents[retval])) { // the point falls within the character at index retval
            int prev_extent = retval ? m_line_data[0].extents[retval - 1] : 0;
            int half_way = (prev_extent + curr_extent) / 2;
            if (x + first_char_offset >= half_way) // if the point is more than halfway across the character, put the cursor *after* the character
                ++retval;
            break;
        }
    }
    return retval;
}

int Edit::FirstCharOffset() const
{
    return (m_first_char_shown ? m_line_data[0].extents[m_first_char_shown - 1] : 0);
}

int Edit::ScreenPosOfChar(int idx) const
{
    int first_char_offset = FirstCharOffset();
    return UpperLeft().x + PIXEL_MARGIN + ((idx ? int(m_line_data[0].extents[idx - 1]) : 0) - first_char_offset);
}

int Edit::LastVisibleChar() const
{
    int first_char_offset = FirstCharOffset();
    int retval;
    for (retval = m_first_char_shown; retval < Length(); ++retval)
        if ((retval ? m_line_data[0].extents[retval - 1] : 0) - first_char_offset >= WindowDimensions().x - 2 * PIXEL_MARGIN)
            break;
    return retval;
}

} // namespace GG

