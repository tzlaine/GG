/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */

#include <GG/Edit.h>

#include <GG/Font.h>
#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>
#include <GG/utf8/checked.h>


using namespace GG;

namespace {
    struct EditedEcho
    {
        EditedEcho(const std::string& name) : m_name(name) {}
        void operator()(const std::string& str)
            { std::cerr << "GG SIGNAL : " << m_name << "(str=" << str << ")\n"; }
        std::string m_name;
    };

    struct InRange
    {
        InRange(CPSize value) : m_value(value) {}
        bool operator()(const std::pair<CPSize, CPSize>& p) const
            { return p.first < m_value && m_value < p.second; }
        const CPSize m_value;
    };

    Y HeightFromFont(const boost::shared_ptr<Font>& font, unsigned int pixel_margin)
    {  return font->Height() + 2 * static_cast<int>(pixel_margin); }
}

////////////////////////////////////////////////
// GG::Edit
////////////////////////////////////////////////
// static(s)
const int Edit::PIXEL_MARGIN = 5;

Edit::Edit(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font, Clr color,
           Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = INTERACTIVE*/) :
    Control(x, y, w, HeightFromFont(font, PIXEL_MARGIN), flags),
    m_text(new TextControl(X0, Y0, w, Height(), str, font, text_color, FORMAT_LEFT | FORMAT_IGNORETAGS)),
    m_cursor_pos(CP0, CP0),
    m_first_char_shown(CP0),
    m_int_color(interior),
    m_hilite_color(CLR_SHADOW),
    m_sel_text_color(CLR_WHITE),
    m_recently_edited(false),
    m_last_button_down_time(0),
    m_in_double_click_mode(false)
{
    SetColor(color);

    if (INSTRUMENT_ALL_SIGNALS) {
        Connect(EditedSignal, EditedEcho("Edit::EditedSignal"));
        Connect(FocusUpdateSignal, EditedEcho("Edit::FocusUpdateSignal"));
    }
}

Edit::~Edit()
{ delete m_text; }

Pt Edit::MinUsableSize() const
{ return Pt(X(4 * PIXEL_MARGIN), HeightFromFont(m_text->GetFont(), PIXEL_MARGIN)); }

Pt Edit::ClientUpperLeft() const
{ return UpperLeft() + Pt(X(PIXEL_MARGIN), Y(PIXEL_MARGIN)); }

Pt Edit::ClientLowerRight() const
{ return LowerRight() - Pt(X(PIXEL_MARGIN), Y(PIXEL_MARGIN)); }

const std::string& Edit::Text() const
{ return m_text->RawText(); }

const std::pair<CPSize, CPSize>& Edit::CursorPosn() const
{ return m_cursor_pos; }

Clr Edit::TextColor() const
{ return m_text->TextColor(); }

Clr Edit::InteriorColor() const
{ return m_int_color; }

Clr Edit::HiliteColor() const
{ return m_hilite_color; }

Clr Edit::SelectedTextColor() const
{ return m_sel_text_color; }

const boost::shared_ptr<Font>& Edit::GetFont() const
{ return m_text->GetFont(); }

bool Edit::PasswordMode() const
{ return m_text->PasswordMode(); }

boost::uint32_t Edit::PasswordCharacter() const
{ return m_text->PasswordCharacter(); }

void Edit::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_int_color) : m_int_color;
    Clr sel_text_color_to_use = Disabled() ? DisabledColor(m_sel_text_color) : m_sel_text_color;
    Clr hilite_color_to_use = Disabled() ? DisabledColor(m_hilite_color) : m_hilite_color;
    Clr text_color_to_use = Disabled() ? DisabledColor(m_text->TextColor()) : m_text->TextColor();

    Pt ul = UpperLeft(), lr = LowerRight();
    Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    BeveledRectangle(ul, lr, int_color_to_use, color_to_use, false, 2);

    BeginScissorClipping(Pt(client_ul.x - 1, client_ul.y), client_lr);

    const std::vector<Font::LineData::CharData>& char_data = m_text->GetLineData()[0].char_data;
    X first_char_offset = FirstCharOffset();
    Y text_y_pos(ul.y + ((lr.y - ul.y) - m_text->GetFont()->Height()) / 2.0 + 0.5);
    CPSize last_visible_char = LastVisibleChar();
    const StrSize INDEX_0 = StringIndexOf(0, m_first_char_shown, m_text->GetLineData());
    const StrSize INDEX_END = StringIndexOf(0, last_visible_char, m_text->GetLineData());
    if (MultiSelected())   { // if one or more chars are selected, hilite, then draw the range in the selected-text color
        CPSize low_cursor_pos  = std::min(m_cursor_pos.first, m_cursor_pos.second);
        CPSize high_cursor_pos = std::max(m_cursor_pos.first, m_cursor_pos.second);

        // draw hiliting
        Pt hilite_ul(client_ul.x + (low_cursor_pos < 1 ? X0 : char_data[Value(low_cursor_pos - 1)].extent) - first_char_offset, client_ul.y);
        Pt hilite_lr(client_ul.x + char_data[Value(high_cursor_pos - 1)].extent - first_char_offset, client_lr.y);
        FlatRectangle(hilite_ul, hilite_lr, hilite_color_to_use, CLR_ZERO, 0);

        // INDEX_0 to INDEX_1 is unhilited, INDEX_1 to
        // INDEX_2 is hilited, and INDEX_2 to INDEX_3 is
        // unhilited; each range may be empty
        const StrSize INDEX_1 = StringIndexOf(0, std::max(low_cursor_pos, m_first_char_shown), m_text->GetLineData());
        const StrSize INDEX_2 = StringIndexOf(0, std::min(high_cursor_pos, last_visible_char), m_text->GetLineData());

        // draw text
        X text_x_pos = ul.x + PIXEL_MARGIN;
        glColor(text_color_to_use);

        // TODO: Use subrange RenderTex()

        text_x_pos +=
            m_text->GetFont()->RenderText(Pt(text_x_pos, text_y_pos),
                                          Text().substr(Value(INDEX_0), Value(INDEX_1 - INDEX_0)));
        glColor(sel_text_color_to_use);
        text_x_pos +=
            m_text->GetFont()->RenderText(Pt(text_x_pos, text_y_pos),
                                          Text().substr(Value(INDEX_1), Value(INDEX_2 - INDEX_1)));
        glColor(text_color_to_use);
        text_x_pos +=
            m_text->GetFont()->RenderText(Pt(text_x_pos, text_y_pos),
                                          Text().substr(Value(INDEX_2), Value(INDEX_END - INDEX_2)));
    } else { // no selected text
        glColor(text_color_to_use);
        m_text->GetFont()->RenderText(Pt(client_ul.x, text_y_pos), Text().substr(Value(INDEX_0), Value(INDEX_END - INDEX_0)));
        if (GUI::GetGUI()->FocusWnd() == this) { // if we have focus, draw the caret as a simple vertical line
            X caret_x = ScreenPosOfChar(m_cursor_pos.second);
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_LINES);
            glVertex(caret_x, client_ul.y);
            glVertex(caret_x, client_lr.y);
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }
    }

    EndScissorClipping();
}

void Edit::SizeMove(const Pt& ul, const Pt& lr)
{
    Control::SizeMove(ul, lr);
    m_text->Resize(Size());
}

void Edit::SetColor(Clr c)
{ Control::SetColor(c); }

void Edit::Disable(bool b/* = true*/)
{
    m_text->Disable(b);
    Control::Disable(b);
}

void Edit::SetText(const std::string& str)
{
    m_text->SetText(str);
    m_cursor_pos.second = m_cursor_pos.first; // eliminate any hiliting

    // make sure the change in text did not make the cursor or view position invalid
    if (str.empty() || m_text->GetLineData().empty() || m_text->GetLineData()[0].char_data.size() < m_cursor_pos.first) {
        m_first_char_shown = CP0;
        m_cursor_pos = std::make_pair(CP0, CP0);
    }

    m_recently_edited = true;
}

void Edit::Clear()
{ m_text->Clear(); }

void Edit::SetInteriorColor(Clr c)
{ m_int_color = c; }

void Edit::SetTextColor(Clr c)
{ m_text->SetTextColor(c); }

void Edit::SetHiliteColor(Clr c)
{ m_hilite_color = c; }

void Edit::SetSelectedTextColor(Clr c)
{ m_sel_text_color = c; }

void Edit::PasswordMode(bool b)
{ m_text->PasswordMode(b); }

void Edit::PasswordCharacter(boost::uint32_t code_point)
{ m_text->PasswordCharacter(code_point); }

void Edit::SelectAll()
{
    m_cursor_pos.first = m_text->Length();
    m_cursor_pos.second = CP0;
    AdjustView();
}

void Edit::SelectRange(CPSize from, CPSize to)
{
    if (from < to) {
        m_cursor_pos.first = std::max(CP0, from);
        m_cursor_pos.second = std::min(to, m_text->Length());
    } else {
        m_cursor_pos.first = std::min(from, m_text->Length());
        m_cursor_pos.second = std::max(CP0, to);
    }
    AdjustView();
}

TextControl* Edit::GetTextControl() const
{ return m_text; }

bool Edit::MultiSelected() const
{ return m_cursor_pos.first != m_cursor_pos.second; }

CPSize Edit::FirstCharShown() const
{ return m_first_char_shown; }

bool Edit::RecentlyEdited() const
{ return m_recently_edited; }

CPSize Edit::CharIndexOf(X x) const
{
    CPSize retval;
    X first_char_offset = FirstCharOffset();
    for (retval = CP0; retval < m_text->Length(); ++retval) {
        X curr_extent;
        if (x + first_char_offset <= (curr_extent = m_text->GetLineData()[0].char_data[Value(retval)].extent)) { // the point falls within the character at index retval
            X prev_extent = retval ? m_text->GetLineData()[0].char_data[Value(retval - 1)].extent : X0;
            X half_way = (prev_extent + curr_extent) / 2;
            if (half_way <= x + first_char_offset) // if the point is more than halfway across the character, put the cursor *after* the character
                ++retval;
            break;
        }
    }
    return retval;
}

X Edit::FirstCharOffset() const
{ return (m_first_char_shown ? m_text->GetLineData()[0].char_data[Value(m_first_char_shown - 1)].extent : X0); }

X Edit::ScreenPosOfChar(CPSize idx) const
{
    X first_char_offset = FirstCharOffset();
    return UpperLeft().x + PIXEL_MARGIN + ((idx ? m_text->GetLineData()[0].char_data[Value(idx - 1)].extent : X0) - first_char_offset);
}

CPSize Edit::LastVisibleChar() const
{
    X first_char_offset = FirstCharOffset();
    CPSize retval = m_first_char_shown;
    for ( ; retval < m_text->Length(); ++retval) {
        if (Size().x - 2 * PIXEL_MARGIN <= (retval ? m_text->GetLineData()[0].char_data[Value(retval - 1)].extent : X0) - first_char_offset)
            break;
    }
    return retval;
}

unsigned int Edit::LastButtonDownTime() const
{ return m_last_button_down_time; }

bool Edit::InDoubleButtonDownMode() const
{ return m_in_double_click_mode; }

std::pair<CPSize, CPSize> Edit::DoubleButtonDownCursorPos() const
{ return m_double_click_cursor_pos; }

void Edit::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        X click_xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // x coord of click within text space
        CPSize idx = CharIndexOf(click_xpos);
        m_cursor_pos.first = m_cursor_pos.second = idx;
        std::pair<CPSize, CPSize> word_indices = GetDoubleButtonDownWordIndices(idx);
        if (word_indices.first != word_indices.second)
            m_cursor_pos = word_indices;
    }
}

void Edit::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        X xpos = ScreenToWindow(pt).x - PIXEL_MARGIN; // x coord for mouse position within text space
        CPSize idx = CharIndexOf(xpos);
        if (m_in_double_click_mode) {
            std::pair<CPSize, CPSize> word_indices =
                GetDoubleButtonDownDragWordIndices(idx);
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

void Edit::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{ ClearDoubleButtonDownMode(); }

void Edit::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ ClearDoubleButtonDownMode(); }

void Edit::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        bool shift_down = mod_keys & (MOD_KEY_LSHIFT | MOD_KEY_RSHIFT);
        bool emit_signal = false;

        switch (key) {
        case GGK_HOME:
            m_first_char_shown = CP0;
            if (shift_down)
                m_cursor_pos.second = CP0;
            else
                m_cursor_pos.second = m_cursor_pos.first = CP0;
            break;
        case GGK_LEFT:
            if (MultiSelected() && !shift_down) {
                m_cursor_pos.second = m_cursor_pos.first = std::min(m_cursor_pos.first, m_cursor_pos.second);
            } else if (0 < m_cursor_pos.second) {
                --m_cursor_pos.second;
                X extent = m_text->GetLineData()[0].char_data[Value(m_cursor_pos.second)].extent;
                while (0 < m_cursor_pos.second && extent == m_text->GetLineData()[0].char_data[Value(m_cursor_pos.second - 1)].extent)
                    --m_cursor_pos.second;
                if (!shift_down)
                    m_cursor_pos.first = m_cursor_pos.second;
            }
            AdjustView();
            break;
        case GGK_RIGHT:
            if (MultiSelected() && !shift_down) {
                m_cursor_pos.second = m_cursor_pos.first = std::max(m_cursor_pos.first, m_cursor_pos.second);
            } else if (m_cursor_pos.second < m_text->Length()) {
                X extent = m_text->GetLineData()[0].char_data[Value(m_cursor_pos.second)].extent;
                while (m_cursor_pos.second < m_text->Length() && extent == m_text->GetLineData()[0].char_data[Value(m_cursor_pos.second)].extent)
                    ++m_cursor_pos.second;
                if (!shift_down)
                    m_cursor_pos.first = m_cursor_pos.second;
            }
            AdjustView();
            break;
        case GGK_END:
            if (shift_down)
                m_cursor_pos.second = m_text->Length();
            else
                m_cursor_pos.second = m_cursor_pos.first = m_text->Length();
            AdjustView();
            break;
        case GGK_BACKSPACE:
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (0 < m_cursor_pos.first) {
                m_cursor_pos.second = --m_cursor_pos.first;
                m_text->Erase(0, m_cursor_pos.first);
                emit_signal = true;
            }
            AdjustView();
            break;
        case GGK_DELETE:
            if (MultiSelected()) {
                ClearSelected();
                emit_signal = true;
            } else if (m_cursor_pos.first < m_text->Length()) {
                m_text->Erase(m_cursor_pos.first);
                emit_signal = true;
            }
            AdjustView();
            break;
        case GGK_RETURN:
        case GGK_KP_ENTER:
            FocusUpdateSignal(Text());
            m_recently_edited = false;
            ForwardEventToParent();
            break;
        default:
            std::string translated_code_point;
            GetTranslatedCodePoint(key, key_code_point, mod_keys, translated_code_point);
            if (!translated_code_point.empty() &&
                !(mod_keys & (MOD_KEY_CTRL | MOD_KEY_ALT | MOD_KEY_META))) {
                if (MultiSelected())
                    ClearSelected();
                m_text->Insert(0, m_cursor_pos.first, translated_code_point);
                m_cursor_pos.second = ++m_cursor_pos.first;
                emit_signal = true;
                if (LastVisibleChar() <= m_cursor_pos.first)
                    AdjustView();
            } else {
                ForwardEventToParent();
            }
            break;
        }
        if (emit_signal) {
            m_recently_edited = true;
            EditedSignal(Text());
        }
    } else {
        ForwardEventToParent();
    }
}

void Edit::GainingFocus()
{ m_recently_edited = false; }

void Edit::LosingFocus()
{
    if (m_recently_edited)
        FocusUpdateSignal(Text());
}

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownWordIndices(CPSize char_index)
{
    unsigned int ticks = GUI::GetGUI()->Ticks();
    if (ticks - m_last_button_down_time <= GUI::GetGUI()->DoubleClickInterval())
        m_in_double_click_mode = true;
    m_last_button_down_time = ticks;
    m_double_click_cursor_pos = std::pair<CPSize, CPSize>(CP0, CP0);
    if (m_in_double_click_mode) {
        std::set<std::pair<CPSize, CPSize> > words =
            GUI::GetGUI()->FindWords(Text());
        std::set<std::pair<CPSize, CPSize> >::const_iterator it =
            std::find_if(words.begin(), words.end(), InRange(char_index));
        if (it != words.end())
            m_double_click_cursor_pos = *it;
    }
    return m_double_click_cursor_pos;
}

std::pair<CPSize, CPSize> Edit::GetDoubleButtonDownDragWordIndices(CPSize char_index)
{
    std::pair<CPSize, CPSize> retval(CP0, CP0);
    std::set<std::pair<CPSize, CPSize> > words =
        GUI::GetGUI()->FindWords(Text());
    std::set<std::pair<CPSize, CPSize> >::const_iterator it =
        std::find_if(words.begin(), words.end(), InRange(char_index));
    if (it != words.end())
        retval = *it;
    return retval;
}

void Edit::ClearDoubleButtonDownMode()
{ m_in_double_click_mode = false; }

void Edit::ClearSelected()
{
    CPSize low = std::min(m_cursor_pos.first, m_cursor_pos.second);
    CPSize high = std::max(m_cursor_pos.first, m_cursor_pos.second);
    if (m_cursor_pos.first < m_cursor_pos.second)
        m_cursor_pos.second = m_cursor_pos.first;
    else
        m_cursor_pos.first = m_cursor_pos.second;
    m_text->Erase(0, low, high - low);

    // make sure deletion has not left m_first_char_shown in an out-of-bounds position
    if (m_text->GetLineData()[0].char_data.empty())
        m_first_char_shown = CP0;
    else if (m_text->GetLineData()[0].char_data.size() <= m_first_char_shown)
        m_first_char_shown = CodePointIndexOf(0, INVALID_CP_SIZE, m_text->GetLineData());
}

void Edit::AdjustView()
{
    X text_space = Size().x - 2 * PIXEL_MARGIN;
    X first_char_offset = FirstCharOffset();
    if (m_cursor_pos.second < m_first_char_shown) { // if the caret is at a place left of the current visible area
        if (m_first_char_shown - m_cursor_pos.second < 5) // if the caret is less than five characters before m_first_char_shown
            m_first_char_shown = (0 <= m_first_char_shown - 5) ? m_first_char_shown - 5 : CP0; // try to move the caret by five characters
        else // if the caret is more than five characters before m_first_char_shown, just move straight to that spot
            m_first_char_shown = m_cursor_pos.second;
    } else if (m_text->Length() && text_space <= (m_cursor_pos.second ? m_text->GetLineData()[0].char_data[Value(m_cursor_pos.second - 1)].extent : X0) - first_char_offset) { // if the caret is moving to a place right of the current visible area
        // try to move the text by five characters, or to the end if caret is at a location before the end - 5th character
        CPSize last_idx_to_use = (m_cursor_pos.second + 5 <= m_text->Length() - 1) ? m_cursor_pos.second + 5 : m_text->Length() - 1;
        const std::vector<Font::LineData::CharData>& char_data = m_text->GetLineData()[0].char_data;
        // number of pixels that the caret position overruns the right side of text area
        X pixels_to_move = (char_data[Value(last_idx_to_use)].extent - first_char_offset) - text_space;
        if (last_idx_to_use == m_text->Length() - 1) // if the caret is at the very end of the string, add the length of some spaces
            pixels_to_move += static_cast<int>(Value(m_cursor_pos.second + 5 - m_text->Length() - 1)) * m_text->GetFont()->SpaceWidth();
        CPSize move_to = m_first_char_shown;
        while (move_to < char_data.size() &&
               char_data[Value(move_to)].extent - first_char_offset < pixels_to_move) {
            ++move_to;
        }
        m_first_char_shown = move_to;
    }
}


////////////////////////////////////////////////////////////
// Free Functions
////////////////////////////////////////////////////////////
void GG::GetTranslatedCodePoint(Key key,
                                boost::uint32_t key_code_point,
                                Flags<ModKey> mod_keys,
                                std::string& translated_code_point)
{
    // only process it if it's a valid code point or a known printable
    // key, and no significant modifiers are in use
    if (key_code_point) {
        try {
            boost::uint32_t chars[] = { key_code_point };
            utf8::utf32to8(chars, chars + 1, std::back_inserter(translated_code_point));
        } catch (const utf8::invalid_code_point&) {
            translated_code_point.clear();
        }
    } else {
        KeypadKeyToPrintable(key, mod_keys);
        if (GGK_DELETE <= key || !isprint(key))
            translated_code_point.clear();
        else
            translated_code_point = key;
    }
}
