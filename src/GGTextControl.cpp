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

#include "GGTextControl.h"
#include "GGApp.h"
#include "GGDrawUtil.h"

namespace GG {

////////////////////////////////////////////////
// GG::TextControl
////////////////////////////////////////////////
TextControl::TextControl(int x, int y, int w, int h, const string& str, const shared_ptr<Font>& font, Uint32 text_fmt/* = 0*/,
                         Clr color/* = CLR_BLACK*/, Uint32 flags/* = 0*/) :
        Control(x, y, w, h, flags),
        m_format(text_fmt),
        m_text_color(color),
        m_font(font),
        m_fit_to_text(false)
{
    ValidateFormat();
    Control::m_text = str;
    if (m_font) m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
}

TextControl::TextControl(int x, int y, int w, int h, const string& str, const string& font_filename, int pts,
                         Uint32 text_fmt/* = 0*/, Clr color/* = CLR_BLACK*/, Uint32 flags/* = 0*/) :
        Control(x, y, w, h, flags),
        m_format(text_fmt),
        m_text_color(color),
        m_font(App::GetApp()->GetFont(font_filename, pts)),
        m_fit_to_text(false)
{
    ValidateFormat();
    Control::m_text = str;
    if (m_font) m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
}

TextControl::TextControl(int x, int y, const string& str, const shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                         Uint32 flags/* = 0*/) :
        Control(x, y, 0, 0, flags),
        m_format(0),
        m_text_color(color),
        m_font(font),
        m_fit_to_text(true)
{
    ValidateFormat();
    Control::m_text = str;
    if (m_font) {
        Pt text_sz = m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
        Resize(text_sz);
    }
}

TextControl::TextControl(int x, int y, const string& str, const string& font_filename, int pts, Clr color/* = CLR_BLACK*/,
                         Uint32 flags/* = 0*/) :
        Control(x, y, 0, 0, flags),
        m_format(0),
        m_text_color(color),
        m_font(App::GetApp()->GetFont(font_filename, pts)),
        m_fit_to_text(true)
{
    ValidateFormat();
    Control::m_text = str;
    if (m_font) {
        Pt text_sz = m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
        Resize(text_sz);
    }
}

TextControl::TextControl(const XMLElement& elem) :
        Control(elem.Child("GG::Control"))
{
    if (elem.Tag() != "GG::TextControl")
        throw std::invalid_argument("Attempted to construct a GG::TextControl from an XMLElement that had a tag other than \"GG::TextControl\"");

    const XMLElement* curr_elem = &elem.Child("m_format");
    m_format = lexical_cast<Uint32>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_text_color");
    m_text_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_font").Child("GG::Font");
    string font_filename = curr_elem->Child("m_font_filename").Text();
    int pts = lexical_cast<int>(curr_elem->Child("m_pt_sz").Attribute("value"));
    m_font = App::GetApp()->GetFont(font_filename, pts);

    curr_elem = &elem.Child("m_fit_to_text");
    m_fit_to_text = lexical_cast<bool>(curr_elem->Attribute("value"));

    if (m_font) {
        Pt text_sz = m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
        if (m_fit_to_text)
            Resize(text_sz);
    }
}

XMLElement TextControl::XMLEncode() const
{
    XMLElement retval("GG::TextControl");
    retval.AppendChild(Control::XMLEncode());

    XMLElement temp;

    temp = XMLElement("m_format");
    temp.SetAttribute("value", lexical_cast<string>(m_format));
    retval.AppendChild(temp);

    temp = XMLElement("m_text_color");
    temp.AppendChild(m_text_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_font");
    temp.AppendChild(m_font->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_fit_to_text");
    temp.SetAttribute("value", lexical_cast<string>(m_fit_to_text));
    retval.AppendChild(temp);

    return retval;
}

int TextControl::Render()
{
    Clr clr_to_use = Disabled() ? DisabledColor(TextColor()) : TextColor();
    glColor4ubv(clr_to_use.v);
    if (m_font) m_font->RenderText(UpperLeft(), LowerRight(), m_text, m_format, &m_line_data, true);
    return 1;
}

void TextControl::SetText(const string& str)
{
    Control::m_text = str;
    if (m_font) {
        Pt text_sz = m_font->DetermineLines(WindowText(), m_format, ClientDimensions().x, m_line_data, true);
        if (m_fit_to_text)
            Resize(text_sz);
    }
}

void TextControl::ValidateFormat()
{
    int dup_ct = 0;   // duplication count
    if (m_format & TF_LEFT) ++dup_ct;
    if (m_format & TF_RIGHT) ++dup_ct;
    if (m_format & TF_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use TF_CENTER by default
        m_format &= ~(TF_RIGHT | TF_LEFT);
        m_format |= TF_CENTER;
    }
    dup_ct = 0;
    if (m_format & TF_TOP) ++dup_ct;
    if (m_format & TF_BOTTOM) ++dup_ct;
    if (m_format & TF_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use TF_VCENTER by default
        m_format &= ~(TF_TOP | TF_BOTTOM);
        m_format |= TF_VCENTER;
    }
    if ((m_format & TF_WORDBREAK) && (m_format & TF_LINEWRAP))   // only one of these can be picked; TF_WORDBREAK overrides TF_LINEWRAP
        m_format &= ~TF_LINEWRAP;
}

} // namespace GG

