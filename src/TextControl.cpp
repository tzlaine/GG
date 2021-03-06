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

#include <GG/TextControl.h>

#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>

#include <boost/assign/list_of.hpp>


using namespace GG;

namespace {
    const Pt INVALID_USABLE_SIZE(-X1, -Y1);
}

////////////////////////////////////////////////
// GG::TextControl
////////////////////////////////////////////////
TextControl::TextControl(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, w, h, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_code_points(0),
    m_font(font),
    m_fit_to_text(false),
    m_password_mode(false),
    m_password_character("*")
{
    ValidateFormat();
    SetText(str);
}

TextControl::TextControl(X x, Y y, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, X0, Y0, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_code_points(0),
    m_font(font),
    m_fit_to_text(true),
    m_password_mode(false),
    m_password_character("*")
{
    ValidateFormat();
    SetText(str);
}

Pt TextControl::MinUsableSize() const
{
    Pt min_size = MinSize();
    if (Text().empty()) {
        m_min_usable_size = Pt();
    } else if (m_fit_to_text) {
        m_min_usable_size = m_text_lr - m_text_ul;
    } else if (0 < min_size.x) {
        if (min_size.x != m_last_min_width) {
            m_min_usable_size = m_font->TextExtent(Text(), m_format, min_size.x);
            m_last_min_width = min_size.x;
        }
    } else {
        std::size_t min_chars = 8;
        if (!m_line_data.empty())
            min_chars = std::min(min_chars, m_line_data[0].char_data.size());
        m_min_usable_size = Pt(static_cast<int>(min_chars) * m_font->SpaceWidth(), m_font->Height());
    }
    return m_min_usable_size;
}

const std::string& TextControl::Text() const
{ return m_password_mode ? m_password_text : m_text; }

const std::string& TextControl::RawText() const
{ return m_text; }

Flags<TextFormat> TextControl::GetTextFormat() const
{ return m_format; }

Clr TextControl::TextColor() const
{ return m_text_color; }

bool TextControl::ClipText() const
{ return m_clip_text; }

bool TextControl::SetMinSize() const
{ return m_set_min_size; }

bool TextControl::PasswordMode() const
{ return m_password_mode; }

boost::uint32_t TextControl::PasswordCharacter() const
{
    boost::uint32_t retval;
    utf8::utf8to32(m_password_character.begin(), m_password_character.end(), &retval);
    return retval;
}

bool TextControl::Empty() const
{ return m_text.empty(); }

CPSize TextControl::Length() const
{ return m_code_points; }

Pt TextControl::TextUpperLeft() const
{ return UpperLeft() + m_text_ul; }

Pt TextControl::TextLowerRight() const
{ return UpperLeft() + m_text_lr; }

void TextControl::Render()
{
    Clr clr_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();
    glColor(clr_to_use);
    if (m_font) {
        ChildClippingMode old_clipping_mode = GetChildClippingMode();
        SetChildClippingMode(ClipToClient);
        if (m_clip_text)
            BeginClipping();
        m_font->RenderText(UpperLeft(), LowerRight(), Text(), m_format, &m_line_data);
        if (m_clip_text)
            EndClipping();
        SetChildClippingMode(old_clipping_mode);
    }
}

void TextControl::SetText(const std::string& str)
{
    m_text = str;

    CPSize str_code_points = CPSize(utf8::distance(str.begin(), str.end()));
    if (m_password_mode) {
        assert(m_password_text.size() % m_password_character.size() == 0);
        CPSize password_code_points(m_password_text.size() / m_password_character.size());
        if (str_code_points < password_code_points) {
            m_text.resize(Value(str_code_points) * m_password_character.size());
        } else {
            for (CPSize i = CP0; i < str_code_points - password_code_points; ++i) {
                m_password_text += m_password_character;
            }
        }
    }

    if (m_font) {
        m_code_points = str_code_points;
        m_text_elements.clear();
        Pt text_sz =
            m_font->DetermineLines(Text(), m_format, ClientSize().x, m_line_data, m_text_elements);
        m_text_ul = Pt();
        m_text_lr = text_sz;
        AdjustMinimumSize();
        if (m_fit_to_text) {
            Resize(text_sz);
        } else {
            RecomputeTextBounds();
        }
    }
}

void TextControl::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    bool redo_determine_lines = false;
    X client_width = ClientSize().x;
    if (!m_fit_to_text && (m_format | FORMAT_WORDBREAK || m_format | FORMAT_LINEWRAP)) {
        X text_width = m_text_lr.x - m_text_ul.x;
        redo_determine_lines =
            client_width < text_width ||
            text_width < client_width && 1u < m_line_data.size();
    }
    if (redo_determine_lines) {
        Pt text_sz;
        if (m_text_elements.empty()) {
            text_sz =
                m_font->DetermineLines(Text(), m_format, client_width, m_line_data, m_text_elements);
        } else {
            text_sz =
                m_font->DetermineLines(Text(), m_format, client_width, m_text_elements, m_line_data);
        }
        m_text_ul = Pt();
        m_text_lr = text_sz;
        AdjustMinimumSize();
    }
    RecomputeTextBounds();
}

void TextControl::SetTextFormat(Flags<TextFormat> format)
{
    m_format = format;
    ValidateFormat();
    if (m_format != format)
        SetText(m_text);
}

void TextControl::SetTextColor(Clr color)
{ m_text_color = color; }

void TextControl::SetColor(Clr c)
{
    Control::SetColor(c);
    m_text_color = c;
}

void TextControl::ClipText(bool b)
{ m_clip_text = b; }

void TextControl::SetMinSize(bool b)
{
    m_set_min_size = b;
    AdjustMinimumSize();
}

void TextControl::PasswordMode(bool b)
{
    bool need_set_text = !m_password_mode && m_password_mode != b;
    m_password_mode = b;
    if (need_set_text)
        SetText(m_text);
}

void TextControl::PasswordCharacter(boost::uint32_t code_point)
{
    CPSize password_code_points;
    if (m_password_mode) {
        assert(m_password_text.size() % m_password_character.size() == 0);
        password_code_points = CPSize(m_password_text.size() / m_password_character.size());
    }
    boost::uint32_t chars[] = { code_point };
    utf8::utf32to8(chars, chars + 1, std::back_inserter(m_password_character));
    m_password_text.clear();
    for (CPSize i = CP0; i < password_code_points; ++i) {
        m_password_text += m_password_character;
    }
}

void TextControl::Clear()
{ SetText(""); }

void TextControl::Insert(CPSize pos, char c)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Insert(line, pos, c);
}

void TextControl::Insert(CPSize pos, const std::string& s)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Insert(line, pos, s);
}

void TextControl::Erase(CPSize pos, CPSize num/* = CP1*/)
{
    std::size_t line;
    boost::tie(line, pos) = LinePositionOf(pos, m_line_data);
    Erase(line, pos, num);
}

void TextControl::Insert(std::size_t line, CPSize pos, char c)
{
    if (!detail::ValidUTFChar<char>()(c))
        throw utf8::invalid_utf8(c);
    m_text.insert(Value(StringIndexOf(line, pos, m_line_data)), 1, c);
    SetText(m_text);
}

void TextControl::Insert(std::size_t line, CPSize pos, const std::string& s)
{
    m_text.insert(Value(StringIndexOf(line, pos, m_line_data)), s);
    SetText(m_text);
}

void TextControl::Erase(std::size_t line, CPSize pos, CPSize num/* = CP1*/)
{
    std::string::iterator it = m_text.begin() + Value(StringIndexOf(line, pos, m_line_data));
    std::string::iterator end_it = m_text.begin() + Value(StringIndexOf(line, pos + num, m_line_data));
    m_text.erase(it, end_it);
    SetText(m_text);
}

const std::vector<Font::LineData>& TextControl::GetLineData() const
{ return m_line_data; }

const boost::shared_ptr<Font>& TextControl::GetFont() const
{ return m_font; }

bool TextControl::FitToText() const
{ return m_fit_to_text; }

void TextControl::ValidateFormat()
{
    int dup_ct = 0;   // duplication count
    if (m_format & FORMAT_LEFT) ++dup_ct;
    if (m_format & FORMAT_RIGHT) ++dup_ct;
    if (m_format & FORMAT_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_CENTER by default
        m_format &= ~(FORMAT_RIGHT | FORMAT_LEFT);
        m_format |= FORMAT_CENTER;
    }
    dup_ct = 0;
    if (m_format & FORMAT_TOP) ++dup_ct;
    if (m_format & FORMAT_BOTTOM) ++dup_ct;
    if (m_format & FORMAT_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_VCENTER by default
        m_format &= ~(FORMAT_TOP | FORMAT_BOTTOM);
        m_format |= FORMAT_VCENTER;
    }
    if ((m_format & FORMAT_WORDBREAK) && (m_format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
        m_format &= ~FORMAT_LINEWRAP;
}

void TextControl::AdjustMinimumSize()
{
    if (m_set_min_size)
        SetMinSize(m_text_lr - m_text_ul);
}

void TextControl::RecomputeTextBounds()
{
    Pt text_sz = TextLowerRight() - TextUpperLeft();
    m_text_ul.y = Y0; // default value for FORMAT_TOP
    if (m_format & FORMAT_BOTTOM)
        m_text_ul.y = Size().y - text_sz.y;
    else if (m_format & FORMAT_VCENTER)
        m_text_ul.y = (Size().y - text_sz.y) / 2.0;
    m_text_ul.x = X0; // default for FORMAT_LEFT
    if (m_format & FORMAT_RIGHT)
        m_text_ul.x = Size().x - text_sz.x;
    else if (m_format & FORMAT_CENTER)
        m_text_ul.x = (Size().x - text_sz.x) / 2.0;
    m_text_lr = m_text_ul + text_sz;
}
