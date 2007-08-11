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

#include <GG/TextControl.h>

#include <GG/DrawUtil.h>
#include <GG/WndEditor.h>

#include <boost/assign/list_of.hpp>


using namespace GG;

namespace {
    struct SetFontAction : AttributeChangedAction<boost::shared_ptr<Font> >
    {
        SetFontAction(TextControl* text_control) : m_text_control(text_control) {}
        void operator()(const boost::shared_ptr<Font>&) {m_text_control->SetText(m_text_control->WindowText());}
    private:
        TextControl* m_text_control;
    };

    struct SetFormatAction : AttributeChangedAction<Flags<TextFormat> >
    {
        SetFormatAction(TextControl* text_control) : m_text_control(text_control) {}
        void operator()(const Flags<TextFormat>& format) {m_text_control->SetTextFormat(format);}
    private:
        TextControl* m_text_control;
    };

    struct FitToTextAction : AttributeChangedAction<bool>
    {
        FitToTextAction(TextControl* text_control) : m_text_control(text_control) {}
        void operator()(const bool&) {m_text_control->SetText(m_text_control->WindowText());}
    private:
        TextControl* m_text_control;
    };

    struct SetMinSizeAction : AttributeChangedAction<bool>
    {
        SetMinSizeAction(TextControl* text_control) : m_text_control(text_control) {}
        void operator()(const bool& set_min_size) {m_text_control->SetMinSize(set_min_size);}
    private:
        TextControl* m_text_control;
    };
}

////////////////////////////////////////////////
// GG::TextControl
////////////////////////////////////////////////
TextControl::TextControl() :
    Control(),
    m_format(FORMAT_NONE),
    m_clip_text(false),
    m_set_min_size(false),
    m_fit_to_text(false),
    m_dirty_load(false)
{}

TextControl::TextControl(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, w, h, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_font(font),
    m_fit_to_text(false),
    m_dirty_load(false)
{
    ValidateFormat();
    SetText(str);
}

TextControl::TextControl(int x, int y, const std::string& str, const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) :
    Control(x, y, 0, 0, flags),
    m_format(format),
    m_text_color(color),
    m_clip_text(false),
    m_set_min_size(false),
    m_font(font),
    m_fit_to_text(true),
    m_dirty_load(false)
{
    ValidateFormat();
    SetText(str);
}

Pt TextControl::MinUsableSize() const
{
    return m_font ?
        m_font->TextExtent(WindowText(), m_format, ClientSize().x) :
        Pt();
}

Flags<TextFormat> TextControl::GetTextFormat() const
{
    return m_format;
}

Clr TextControl::TextColor() const
{
    return m_text_color;
}

bool TextControl::ClipText() const
{
    return m_clip_text;
}

bool TextControl::SetMinSize() const
{
    return m_set_min_size;
}

TextControl::operator const std::string&() const
{
    return Control::m_text;
}

bool TextControl::Empty() const
{
    return Control::m_text.empty();
}

int TextControl::Length() const
{
    return Control::m_text.length();
}

Pt TextControl::TextUpperLeft() const
{
    return UpperLeft() + m_text_ul;
}

Pt TextControl::TextLowerRight() const
{
    return UpperLeft() + m_text_lr;
}

void TextControl::Render()
{
    if (m_dirty_load)
        SetText(WindowText());
    Clr clr_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();
    glColor(clr_to_use);
    if (m_font) {
        if (m_clip_text)
            BeginClipping();
        m_font->RenderText(UpperLeft(), LowerRight(), m_text, m_format, &m_line_data);
        if (m_clip_text)
            EndClipping();
    }
}

void TextControl::SetText(const std::string& str)
{
    Control::m_text = str;
    if (m_font) {
        Pt text_sz = m_font->DetermineLines(WindowText(), m_format, ClientSize().x, m_line_data);
        m_text_ul = Pt();
        m_text_lr = text_sz;
        AdjustMinimumSize();
        if (m_fit_to_text) {
            Resize(text_sz);
        } else {
            RecomputeTextBounds();
        }
    }
    m_dirty_load = false;
}

void TextControl::SetText(const char* str)
{
    SetText(std::string(str));
}

void TextControl::SizeMove(const Pt& ul, const Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    RecomputeTextBounds();
}

void TextControl::SetTextFormat(Flags<TextFormat> format)
{
    m_format = format;
    ValidateFormat();
    SetText(WindowText());
}

void TextControl::SetTextColor(Clr color)
{
    m_text_color = color;
}

void TextControl::SetColor(Clr c)
{
    Control::SetColor(c);
    m_text_color = c;
}

void TextControl::ClipText(bool b)
{
    m_clip_text = b;
}

void TextControl::SetMinSize(bool b)
{
    m_set_min_size = b;
    AdjustMinimumSize();
}

void TextControl::operator+=(const std::string& str)
{
    SetText(Control::m_text + str);
}

void TextControl::operator+=(const char* str)
{
    SetText(Control::m_text + str);
}

void TextControl::operator+=(char ch)
{
    SetText(Control::m_text + ch);
}

void TextControl::Clear()
{
    SetText("");
}

void TextControl::Insert(int pos, char ch)
{
    Control::m_text.insert(pos, 1, ch);
    SetText(Control::m_text);
}

void TextControl::Erase(int pos, int num/* = 1*/)
{
    Control::m_text.erase(pos, num);
    SetText(Control::m_text);
}

void TextControl::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("TextControl");
    boost::shared_ptr<SetFontAction> set_font_action(new SetFontAction(this));
    editor->Attribute<boost::shared_ptr<Font> >("Font", m_font, set_font_action);
    boost::shared_ptr<SetFormatAction> set_format_action(new SetFormatAction(this));
    editor->BeginFlags<TextFormat>(m_format, set_format_action);
    typedef std::vector<TextFormat> FlagVec;
    using boost::assign::list_of;
    editor->FlagGroup("V. Alignment", FlagVec() = list_of(FORMAT_TOP)(FORMAT_VCENTER)(FORMAT_BOTTOM));
    editor->FlagGroup("H. Alignment", FlagVec() = list_of(FORMAT_LEFT)(FORMAT_CENTER)(FORMAT_RIGHT));
    editor->Flag("Word-break", FORMAT_WORDBREAK);
    editor->Flag("Line-wrap", FORMAT_LINEWRAP);
    editor->Flag("Ignore Tags", FORMAT_IGNORETAGS);
    editor->EndFlags();
    editor->Attribute("Text Color", m_text_color);
    editor->Attribute("Clip Text", m_clip_text);
    boost::shared_ptr<FitToTextAction> fit_to_text_action(new FitToTextAction(this));
    editor->Attribute<bool>("Fit Size to Text", m_fit_to_text, fit_to_text_action);
    boost::shared_ptr<SetMinSizeAction> min_size_action(new SetMinSizeAction(this));
    editor->Attribute<bool>("Set Min Size", m_set_min_size, min_size_action);
    editor->ConstAttribute("Text Upper Left", m_text_ul);
    editor->ConstAttribute("Text Lower Right", m_text_lr);
}

const std::vector<Font::LineData>& TextControl::GetLineData() const
{
    return m_line_data;
}

const boost::shared_ptr<Font>& TextControl::GetFont() const
{
    return m_font;
}

bool TextControl::FitToText() const
{
    return m_fit_to_text;
}

bool TextControl::DirtyLoad() const
{
    return m_dirty_load;
}

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
    m_text_ul.y = 0; // default value for FORMAT_TOP
    if (m_format & FORMAT_BOTTOM)
        m_text_ul.y = Size().y - text_sz.y;
    else if (m_format & FORMAT_VCENTER)
        m_text_ul.y = static_cast<int>((Size().y - text_sz.y) / 2.0);
    m_text_ul.x = 0; // default for FORMAT_LEFT
    if (m_format & FORMAT_RIGHT)
        m_text_ul.x = Size().x - text_sz.x;
    else if (m_format & FORMAT_CENTER)
        m_text_ul.x = static_cast<int>((Size().x - text_sz.x) / 2.0);
    m_text_lr = m_text_ul + text_sz;
}
