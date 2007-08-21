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

#include <GG/Font.h>

#include <GG/GUI.h>
#include <GG/Base.h>
#include <GG/DrawUtil.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boost/lexical_cast.hpp>
#include <boost/spirit.hpp>

#include <cmath>
#include <numeric>
#include <sstream>

#define DEBUG_DETERMINELINES 0

using namespace GG;

namespace {
    int NextPowerOfTwo(int input)
    {
        int value = 1;
        while (value < input)
            value <<= 1;
        return value;
    }

    inline FT_ULong CharToFT_ULong(char c)
    {
        return static_cast<FT_ULong>(c < 0 ? static_cast<unsigned char>(256 + c) : static_cast<unsigned char>(c));
    }

    /** This is used to collect data on the glyphs as they are recorded into buffers, for use in creating Glyph objects at the end
        of Font's constructor.*/
    struct TempGlyphData
    {
        TempGlyphData() {} ///< default ctor
        TempGlyphData(int i, int _x1, int _y1, int _x2, int _y2, int lb, int a) : idx(i), x1(_x1), y1(_y1), x2(_x2), y2(_y2), left_b(lb), adv(a) {} ///< ctor
        int      idx;              ///< index into m_textures of texture that contains this glyph
        int      x1, y1, x2, y2;   ///< area of glyph subtexture within texture
        int      left_b;           ///< left bearing (see Glyph)
        int      adv;              ///< advance of glyph (see Glyph)
    };

    struct FTLibraryWrapper
    {
        FTLibraryWrapper() : library(0)
        {
            if (!library && FT_Init_FreeType(&library)) // if no library exists and we can't create one...
                throw FailedFTLibraryInit("Unable to initialize FreeType font library object");
        }
        ~FTLibraryWrapper() {FT_Done_FreeType(library);}
        FT_Library library;
    } g_library;

    struct AppendToken
    {
        AppendToken(std::vector<std::string>& token_vec) : tokens(token_vec) {}
        void operator()(const char* first, const char* last) const {tokens.push_back(std::string(first, last));}
    private:
        std::vector<std::string>& tokens;
    };

    struct HandlePreTagFunctor
    {
        HandlePreTagFunctor(std::vector<boost::shared_ptr<Font::TextElement> >& text_elements, bool& ignore_tag_status, bool close_tag) :
            elements(text_elements), ignore_tags(ignore_tag_status), close(close_tag) {}
        void operator()(const char* first, const char* last) const
            {
                if (ignore_tags && !close) {
                    boost::shared_ptr<Font::TextElement> element(new Font::TextElement(false, false));
                    element->text = std::string(first, last);
                    elements.push_back(element);
                } else {
                    boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(close));
                    element->text = "pre";
                    element->original_tag_text = std::string(first, last);
                    elements.push_back(element);
                    ignore_tags = !close;
                }
            }
    private:
        std::vector<boost::shared_ptr<Font::TextElement> >& elements;
        bool& ignore_tags;
        const bool close;
    };

    struct HandleTextFunctor
    {
        HandleTextFunctor(std::vector<boost::shared_ptr<Font::TextElement> >& text_elements) : elements(text_elements) {}
        void operator()(const char* first, const char* last) const
            {
                boost::shared_ptr<Font::TextElement> element(new Font::TextElement(false, false));
                element->text = std::string(first, last);
                elements.push_back(element);
            }
    private:
        std::vector<boost::shared_ptr<Font::TextElement> >& elements;
    };

    boost::spirit::rule<> ws_p = (*boost::spirit::blank_p >> (boost::spirit::eol_p | '\n' | '\r' | '\f')) | +boost::spirit::blank_p;

    struct HandleWhitespaceFunctor
    {
        HandleWhitespaceFunctor(std::vector<boost::shared_ptr<Font::TextElement> >& text_elements) : elements(text_elements) {}
        void operator()(const char* first, const char* last) const
            {
                std::vector<std::string> string_vec;
                using namespace boost::spirit;
                std::string ws_str = std::string(first, last);
                parse(ws_str.c_str(), ws_p[AppendToken(string_vec)]);
                for (unsigned int i = 0; i < string_vec.size(); ++i) {
                    boost::shared_ptr<Font::TextElement> element(new Font::TextElement(true, false));
                    element->text = std::string(first, last);
                    elements.push_back(element);
                    if (string_vec[i].substr(string_vec[i].size() - 1).find_first_of("\n\f\r") != std::string::npos) {
                        boost::shared_ptr<Font::TextElement> element(new Font::TextElement(false, true));
                        elements.push_back(element);
                    }
                }
            }
    private:
        std::vector<boost::shared_ptr<Font::TextElement> >& elements;
    };

    void SetJustification(bool& last_line_of_curr_just, Font::LineData& line_data, Alignment orig_just, Alignment prev_just)
    {
        if (last_line_of_curr_just) {
            line_data.justification = orig_just;
            last_line_of_curr_just = false;
        } else {
            line_data.justification = prev_just;
        }
    }

    const double ITALICS_SLANT_ANGLE = 12; // degrees
    const double ITALICS_FACTOR = 1.0 / tan((90 - ITALICS_SLANT_ANGLE) * 3.1415926 / 180.0); // factor used to shear glyphs ITALICS_SLANT_ANGLE degrees CW from straight up

    const int FT_MAGIC_NUMBER = 4; // taken from the ftview FreeType demo (I have no idea....)
}


///////////////////////////////////////
// function GG::RgbaTag
///////////////////////////////////////
std::string GG::RgbaTag(const Clr& c)
{
    std::stringstream stream;
    stream << "<rgba " << static_cast<int>(c.r) << " " << static_cast<int>(c.g) << " " << 
        static_cast<int>(c.b) << " " << static_cast<int>(c.a) << ">";
    return stream.str();
}


///////////////////////////////////////
// TextFormat
///////////////////////////////////////
const TextFormat GG::FORMAT_NONE         (0);
const TextFormat GG::FORMAT_VCENTER      (1 << 0);
const TextFormat GG::FORMAT_TOP          (1 << 1);
const TextFormat GG::FORMAT_BOTTOM       (1 << 2);
const TextFormat GG::FORMAT_CENTER       (1 << 3);
const TextFormat GG::FORMAT_LEFT         (1 << 4);
const TextFormat GG::FORMAT_RIGHT        (1 << 5);
const TextFormat GG::FORMAT_WORDBREAK    (1 << 6);
const TextFormat GG::FORMAT_LINEWRAP     (1 << 7);
const TextFormat GG::FORMAT_IGNORETAGS   (1 << 8);

GG_FLAGSPEC_IMPL(TextFormat);

namespace {
    bool RegisterTextFormats()
    {
        FlagSpec<TextFormat>& spec = FlagSpec<TextFormat>::instance();
        spec.insert(FORMAT_NONE, "FORMAT_NONE", true);
        spec.insert(FORMAT_VCENTER, "FORMAT_VCENTER", true);
        spec.insert(FORMAT_TOP, "FORMAT_TOP", true);
        spec.insert(FORMAT_BOTTOM, "FORMAT_BOTTOM", true);
        spec.insert(FORMAT_CENTER, "FORMAT_CENTER", true);
        spec.insert(FORMAT_LEFT, "FORMAT_LEFT", true);
        spec.insert(FORMAT_RIGHT, "FORMAT_RIGHT", true);
        spec.insert(FORMAT_WORDBREAK, "FORMAT_WORDBREAK", true);
        spec.insert(FORMAT_LINEWRAP, "FORMAT_LINEWRAP", true);
        spec.insert(FORMAT_IGNORETAGS, "FORMAT_IGNORETAGS", true);
        return true;
    }
    bool dummy = RegisterTextFormats();
}


///////////////////////////////////////
// class GG::Font::TextElement
///////////////////////////////////////
Font::TextElement::TextElement() :
    whitespace(false),
    newline(false)
{
}

Font::TextElement::TextElement(bool ws, bool nl) :
    whitespace(ws),
    newline(nl)
{
}

Font::TextElement::~TextElement()
{
}

int Font::TextElement::Width() const
{
    return std::accumulate(widths.begin(), widths.end(), 0);
}

Font::TextElement::TextElementType Font::TextElement::Type() const
{
    return newline ? NEWLINE : (whitespace ? WHITESPACE : TEXT);
}

int Font::TextElement::OriginalStringChars() const
{
    return text.size();
}


///////////////////////////////////////
// class GG::Font::FormattingTag
///////////////////////////////////////
Font::FormattingTag::FormattingTag() :
    TextElement(),
    close_tag(false)
{
}

Font::FormattingTag::FormattingTag(bool close) :
    TextElement(false, false),
    close_tag(close)
{
}

Font::FormattingTag::TextElementType Font::FormattingTag::Type() const
{
    return close_tag ? CLOSE_TAG : OPEN_TAG;
}

int Font::FormattingTag::OriginalStringChars() const
{
    return original_tag_text.size();
}


///////////////////////////////////////
// class GG::Font::LineData
///////////////////////////////////////
Font::LineData::LineData() :
    justification(ALIGN_CENTER)
{
}

int Font::LineData::Width() const
{
    return char_data.empty() ? 0 : char_data.back().extent;
}

bool Font::LineData::Empty() const
{
    return char_data.empty();
}

///////////////////////////////////////
// class GG::Font::RenderState
///////////////////////////////////////
Font::RenderState::RenderState() :
    ignore_tags(false),
    use_italics(false),
    draw_underline(false),
    color_set(false)
{}


///////////////////////////////////////
// class GG::Font::LineData::CharData
///////////////////////////////////////
Font::LineData::CharData::CharData() :
    extent(0),
    original_char_index(0)
{
}

Font::LineData::CharData::CharData(int extent_, int original_index, const std::vector<boost::shared_ptr<TextElement> >& tags_) :
    extent(extent_),
    original_char_index(original_index),
    tags()
{
    for (unsigned int i = 0; i < tags_.size(); ++i) {
        tags.push_back(boost::dynamic_pointer_cast<FormattingTag>(tags_[i]));
    }
}


///////////////////////////////////////
// struct GG::Font::HandleTagFunctor
///////////////////////////////////////
struct Font::HandleTagFunctor
{
    HandleTagFunctor(std::vector<boost::shared_ptr<Font::TextElement> >& text_elements, const bool& ignore_tag_status, bool close_tag) :
        elements(text_elements), ignore_tags(ignore_tag_status), close(close_tag) {}
    void operator()(const char* first, const char* last) const
        {
            std::string tag_str = std::string(first, last);
            using namespace boost::spirit;
            std::vector<std::string> param_vec;
            rule<> tag_begin = ch_p('<');
            if (close)
                tag_begin = str_p("</");
            rule<> tag_token_parser = tag_begin >> +(*space_p >> (+(anychar_p - (space_p | '>')))[AppendToken(param_vec)]) >> '>';
            parse(tag_str.c_str(), tag_token_parser);
            if (!ignore_tags && Font::s_known_tags.find(param_vec.front()) != Font::s_known_tags.end()) {
                boost::shared_ptr<Font::FormattingTag> element(new Font::FormattingTag(close));
                element->text = param_vec.front();
                if (!close)
                    element->params.insert(element->params.end(), param_vec.begin() + 1, param_vec.end());
                element->original_tag_text = tag_str;
                elements.push_back(element);
            } else {
                boost::shared_ptr<Font::TextElement> text_element(new Font::TextElement(false, false));
                text_element->text = tag_str;
                elements.push_back(text_element);
            }
        }
private:
    std::vector<boost::shared_ptr<Font::TextElement> >& elements;
    const bool& ignore_tags;
    const bool close;
};


///////////////////////////////////////
// class GG::Font
///////////////////////////////////////
std::set<std::string> Font::s_action_tags;
std::set<std::string> Font::s_known_tags;

Font::Font() :
    m_pt_sz(0),
    m_glyph_range(0),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_space_width(0)
{
}

Font::Font(const std::string& font_filename, int pts, unsigned int range/* = ALL_DEFINED_RANGES*/) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_glyph_range(range),
    m_ascent(0),
    m_descent(0),
    m_height(0),
    m_lineskip(0),
    m_underline_offset(0.0),
    m_underline_height(0.0),
    m_italics_offset(0.0),
    m_space_width(0)
{
    if (font_filename != "")
        Init(font_filename, pts, range);
}

Font::~Font()
{
}

const std::string& Font::FontName() const     
{
    return m_font_filename;
}

int Font::PointSize() const    
{
    return m_pt_sz;
}

int Font::GetGlyphRange() const
{
    return m_glyph_range;
}

int Font::Ascent() const       
{
    return m_ascent;
}

int Font::Descent() const      
{
    return m_descent;
}

int Font::Height() const       
{
    return m_height;
}

int Font::Lineskip() const     
{
    return m_lineskip;
}

int Font::SpaceWidth() const   
{
    return m_space_width;
}

int Font::RenderGlyph(const Pt& pt, char c) const
{
    return RenderGlyph(pt.x, pt.y, c);
}

int Font::RenderGlyph(int x, int y, char c) const
{
    std::map<FT_ULong, Glyph>::const_iterator it = m_glyphs.find(CharToFT_ULong(c));
    if (it == m_glyphs.end())
        it = m_glyphs.find(CharToFT_ULong(' ')); // print a space when an unrendered glyph is requested
    return RenderGlyph(x, y, it->second, 0);
}

int Font::RenderText(const Pt& pt, const std::string& text) const
{
    return RenderText(pt.x, pt.y, text);
}

int Font::RenderText(int x, int y, const std::string& text) const
{
    int orig_x = x;
    for (unsigned int i = 0; i < text.length(); ++i) {
        x += RenderGlyph(x, y, text[i]);
    }
    return x - orig_x;
}

void Font::RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>* line_data/* = 0*/,
                      RenderState* render_state/* = 0*/) const
{
    RenderText(pt1.x, pt1.y, pt2.x, pt2.y, text, format, line_data, render_state);
}

void Font::RenderText(int x1, int y1, int x2, int y2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>* line_data/* = 0*/,
                      RenderState* render_state/* = 0*/) const
{
    RenderState state;
    if (!render_state)
        render_state = &state;

    // get breakdown of how text is divided into lines
    std::vector<LineData> lines;
    if (!line_data) {
        DetermineLines(text, format, x2 - x1, lines);
        line_data = &lines;
    }

    RenderText(x1, y1, x2, y2, text, format, *line_data, *render_state, 0, 0, line_data->size(), line_data->back().char_data.size());
}

void Font::RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>& line_data, RenderState& render_state,
                      int begin_line, int begin_char, int end_line, int end_char) const
{
    RenderText(pt1.x, pt1.y, pt2.x, pt2.y, text, format, line_data, render_state, begin_line, begin_char, end_line, end_char);
}

void Font::RenderText(int x1, int y1, int x2, int y2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>& line_data, RenderState& render_state,
                      int begin_line, int begin_char, int end_line, int end_char) const
{
    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);
    
    if (render_state.color_set)
        glColor(render_state.curr_color);

    int y_origin = y1; // default value for FORMAT_TOP
    if (format & FORMAT_BOTTOM)
        y_origin = y2 - ((end_line - begin_line - 1) * m_lineskip + m_height);
    else if (format & FORMAT_VCENTER)
        y_origin = y1 + static_cast<int>(((y2 - y1) - ((end_line - begin_line - 1) * m_lineskip + m_height)) / 2.0);

    for (int i = begin_line; i < end_line; ++i) {
        const LineData& line = line_data[i];
        int x_origin = x1; // default value for FORMAT_LEFT
        if (line.justification == ALIGN_RIGHT)
            x_origin = x2 - line.Width();
        else if (line.justification == ALIGN_CENTER)
            x_origin = x1 + static_cast<int>(((x2 - x1) - line.Width()) / 2.0);
        int y = y_origin + (i - begin_line) * m_lineskip;
        int x = x_origin;

        for (int j = ((i == begin_line) ? begin_char : 0); j < ((i == end_line - 1) ? end_char : static_cast<int>(line.char_data.size())); ++j) {
            for (unsigned int k = 0; k < line.char_data[j].tags.size(); ++k) {
                HandleTag(line.char_data[j].tags[k], orig_color, render_state);
            }
            if (text[line.char_data[j].original_char_index] == '\n')
                continue;
            FT_ULong c = CharToFT_ULong(text[line.char_data[j].original_char_index]);
            std::map<FT_ULong, Glyph>::const_iterator it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                x = x_origin + line.char_data[j].extent; // move forward by the extent of the character when a whitespace or unprintable glyph is requested
            else
                x += RenderGlyph(x, y, it->second, &render_state);
        }
    }

    glColor4dv(orig_color);
}

Pt Font::DetermineLines(const std::string& text, Flags<TextFormat>& format, int box_width, std::vector<LineData>& line_data) const
{
    Pt retval;

    // correct any disagreements in the format flags
    int dup_ct = 0;   // duplication count
    if (format & FORMAT_LEFT) ++dup_ct;
    if (format & FORMAT_RIGHT) ++dup_ct;
    if (format & FORMAT_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_LEFT by default
        format &= ~(FORMAT_RIGHT | FORMAT_CENTER);
        format |= FORMAT_LEFT;
    }
    dup_ct = 0;
    if (format & FORMAT_TOP) ++dup_ct;
    if (format & FORMAT_BOTTOM) ++dup_ct;
    if (format & FORMAT_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use FORMAT_TOP by default
        format &= ~(FORMAT_BOTTOM | FORMAT_VCENTER);
        format |= FORMAT_TOP;
    }
    if ((format & FORMAT_WORDBREAK) && (format & FORMAT_LINEWRAP))   // only one of these can be picked; FORMAT_WORDBREAK overrides FORMAT_LINEWRAP
        format &= ~FORMAT_LINEWRAP;

#if DEBUG_DETERMINELINES
    std::cout << "Font::DetermineLines(text=\"" << text << "\" format=" << format << " box_width=" << box_width << ")" << std::endl;
#endif

    std::vector<boost::shared_ptr<TextElement> > text_elements;
    using namespace boost::spirit;
    rule<> open_pre_tag_p = str_p("<pre>");
    rule<> close_pre_tag_p = str_p("</pre>");
    rule<> close_tag_p = str_p("</") >> +(anychar_p - '>') >> '>';
    rule<> open_tag_p = ch_p('<') >> +(anychar_p - '>') >> '>';
    rule<> text_p = +(anychar_p - (close_tag_p | open_tag_p | space_p));
    if (format & FORMAT_IGNORETAGS) {
        text_p = +(anychar_p - space_p);
        rule<> lines_parser = *(text_p[HandleTextFunctor(text_elements)] |
                                ws_p[HandleWhitespaceFunctor(text_elements)]);
        parse(text.c_str(), lines_parser);
    } else {
        bool ignore_tags = false;
        rule<> lines_parser = *(open_pre_tag_p[HandlePreTagFunctor(text_elements, ignore_tags, false)] |
                                close_pre_tag_p[HandlePreTagFunctor(text_elements, ignore_tags, true)] |
                                close_tag_p[HandleTagFunctor(text_elements, ignore_tags, true)] |
                                open_tag_p[HandleTagFunctor(text_elements, ignore_tags, false)] |
                                text_p[HandleTextFunctor(text_elements)] |
                                ws_p[HandleWhitespaceFunctor(text_elements)]);
        parse(text.c_str(), lines_parser);
    }

    // fill in the widths of characters in the element std::strings
    for (unsigned int i = 0; i < text_elements.size(); ++i) {
        text_elements[i]->widths.resize(text_elements[i]->text.size());
        for (unsigned int j = 0; j < text_elements[i]->text.size(); ++j) {
            if (text_elements[i]->text[j] == '\n') {
                text_elements[i]->widths[j] = 0;
                continue;
            }
            FT_ULong c = text_elements[i]->text[j];
            std::map<FT_ULong, Glyph>::const_iterator it = m_glyphs.find(c);
            if (it == m_glyphs.end())
                it = m_glyphs.find(CharToFT_ULong(' ')); // use a space when an unrendered glyph is requested (the space chararacter is always renderable)
            text_elements[i]->widths[j] = it->second.advance;
        }
    }

#if DEBUG_DETERMINELINES
    std::cout << "parsing results:\n";
    for (unsigned int i = 0; i < text_elements.size(); ++i) {
        if (boost::shared_ptr<FormattingTag> tag_elem = boost::dynamic_pointer_cast<FormattingTag>(text_elements[i])) {
            std::cout << "FormattingTag\n    text=\"" << tag_elem->text << "\"\n    widths=" << StringFromContainer(tag_elem->widths)
                      << "\n    whitespace=" << tag_elem->whitespace << "\n    newline=" << tag_elem->newline << "\n    params=\n";
            for (unsigned int j = 0; j < tag_elem->params.size(); ++j) {
                std::cout << "        \"" << tag_elem->params[j] << "\"\n";
            }
            std::cout << "    original_tag_text=\"" << tag_elem->original_tag_text << "\"\n    close_tag=" << tag_elem->close_tag << "\n";
        } else {
            boost::shared_ptr<TextElement> elem = text_elements[i];
            std::cout << "TextElement\n    text=\"" << elem->text << "\"\n    widths=" << StringFromContainer(elem->widths)
                      << "\n    whitespace=" << elem->whitespace << "\n    newline=" << elem->newline << "\n";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
#endif

    RenderState render_state;
    int tab_width = 8; // default tab width
    int tab_pixel_width = tab_width * m_space_width; // get the length of a tab stop
    bool expand_tabs = format & FORMAT_LEFT; // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
    Alignment orig_just = ALIGN_NONE;
    if (format & FORMAT_LEFT)
        orig_just = ALIGN_LEFT;
    if (format & FORMAT_CENTER)
        orig_just = ALIGN_CENTER;
    if (format & FORMAT_RIGHT)
        orig_just = ALIGN_RIGHT;
    bool last_line_of_curr_just = false; // is this the last line of the current justification? (for instance when a </right> tag is encountered)

    line_data.clear();
    line_data.push_back(LineData());
    line_data.back().justification = orig_just;

    int x = 0;
    int original_string_offset = 0; // the position within the original string of the current TextElement
    std::vector<boost::shared_ptr<TextElement> > pending_formatting_tags;
    for (unsigned int i = 0; i < text_elements.size(); ++i) {
        boost::shared_ptr<TextElement> elem = text_elements[i];
        if (elem->Type() == TextElement::NEWLINE) { // if a newline is explicitly requested, start a new one
            line_data.push_back(LineData());
            SetJustification(last_line_of_curr_just, line_data.back(), orig_just, line_data[line_data.size() - 2].justification);
            x = 0; // reset the x-position to 0
        } else if (elem->Type() == TextElement::WHITESPACE) {
            for (unsigned int j = 0; j < elem->text.size(); ++j) {
                FT_ULong c = CharToFT_ULong(elem->text[j]);
                if (c != '\r' && c != '\f') {
                    int advance_position = x + m_space_width;
                    if (c == '\t' && expand_tabs) {
                        advance_position = (((x / tab_pixel_width) + 1) * tab_pixel_width);
                    } else if (c == '\n') {
                        advance_position = x;
                    }
                    int advance = advance_position - x;
                    if ((format & FORMAT_LINEWRAP) && box_width < advance_position) { // if we're using linewrap and this space won't fit on this line,
                        if (!x && box_width < advance) {
                            // if the space is larger than the line and alone on the line, let the space overrun this
                            // line and then start a new one
                            line_data.push_back(LineData());
                            x = 0; // reset the x-position to 0
                            SetJustification(last_line_of_curr_just, line_data.back(), orig_just, line_data[line_data.size() - 2].justification);
                        } else { // otherwise start a new line and put the space there:
                            line_data.push_back(LineData());
                            x = advance;
                            line_data.back().char_data.push_back(LineData::CharData(x, original_string_offset + j, pending_formatting_tags));
                            pending_formatting_tags.clear();
                            SetJustification(last_line_of_curr_just, line_data.back(), orig_just, line_data[line_data.size() - 2].justification);
                        }
                    } else { // there's room for the space, or we're not using linewrap
                        x += advance;
                        line_data.back().char_data.push_back(LineData::CharData(x, original_string_offset + j, pending_formatting_tags));
                        pending_formatting_tags.clear();
                    }
                }
            }
        } else if (elem->Type() == TextElement::TEXT) {
            if (format & FORMAT_WORDBREAK) {
                if (box_width < x + elem->Width() && x) { // if the text "word" overruns this line, and isn't alone on this line, move it down to the next line
                    line_data.push_back(LineData());
                    x = 0;
                    SetJustification(last_line_of_curr_just, line_data.back(), orig_just, line_data[line_data.size() - 2].justification);
                }
                for (unsigned int j = 0; j < elem->text.size(); ++j) {
                    x += elem->widths[j];
                    line_data.back().char_data.push_back(LineData::CharData(x, original_string_offset + j, pending_formatting_tags));
                    pending_formatting_tags.clear();
                }
            } else {
                for (unsigned int j = 0; j < elem->text.size(); ++j) {
                    if ((format & FORMAT_LINEWRAP) && box_width < x + elem->widths[j] && x) { // if the char overruns this line, and isn't alone on this line, move it down to the next line
                        line_data.push_back(LineData());
                        x = elem->widths[j];
                        line_data.back().char_data.push_back(LineData::CharData(x, original_string_offset + j, pending_formatting_tags));
                        pending_formatting_tags.clear();
                        SetJustification(last_line_of_curr_just, line_data.back(), orig_just, line_data[line_data.size() - 2].justification);
                    } else { // there's room for this char on this line, or there's no wrapping in use
                        x += elem->widths[j];
                        line_data.back().char_data.push_back(LineData::CharData(x, original_string_offset + j, pending_formatting_tags));
                        pending_formatting_tags.clear();
                    }
                }
            }
        } else if (elem->Type() == TextElement::OPEN_TAG) {
            if (elem->text == "left")
                line_data.back().justification = ALIGN_LEFT;
            else if (elem->text == "center")
                line_data.back().justification = ALIGN_CENTER;
            else if (elem->text == "right")
                line_data.back().justification = ALIGN_RIGHT;
            else if (elem->text != "pre")
                pending_formatting_tags.push_back(elem);
            last_line_of_curr_just = false;
        } else if (elem->Type() == TextElement::CLOSE_TAG) {
            if ((elem->text == "left" && line_data.back().justification == ALIGN_LEFT) ||
                (elem->text == "center" && line_data.back().justification == ALIGN_CENTER) ||
                (elem->text == "right" && line_data.back().justification == ALIGN_RIGHT))
                last_line_of_curr_just = true;
            else if (elem->text != "pre")
                pending_formatting_tags.push_back(elem);
        }
        original_string_offset += elem->OriginalStringChars();
    }
    // disregard the final pending formatting tag, if any, since this is the end of the text, and so it cannot have any effect

#if DEBUG_DETERMINELINES
    std::cout << "Line breakdown:\n";
    for (unsigned int i = 0; i < line_data.size(); ++i) {
        std::cout << "Line " << i << ":\n    extents=";
        for (unsigned int j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << line_data[i].char_data[j].extent << " ";
        }
        std::cout << "\n    chars on line: \"";
        for (unsigned int j = 0; j < line_data[i].char_data.size(); ++j) {
            std::cout << text[line_data[i].char_data[j].original_char_index];
        }
        std::cout << "\"\n" << std::endl;
        for (unsigned int j = 0; j < line_data[i].char_data.size(); ++j) {
            for (unsigned int k = 0; k < line_data[i].char_data[j].tags.size(); ++k) {
                if (boost::shared_ptr<FormattingTag> tag_elem = line_data[i].char_data[j].tags[k]) {
                    std::cout << "FormattingTag @" << j << "\n    text=\"" << tag_elem->text << "\"\n    widths="
                              << StringFromContainer(tag_elem->widths) << "\n    whitespace=" << tag_elem->whitespace
                              << "\n    newline=" << tag_elem->newline << "\n    params=\n";
                    for (unsigned int l = 0; l < tag_elem->params.size(); ++l) {
                        std::cout << "        \"" << tag_elem->params[l] << "\"\n";
                    }
                    std::cout << "    original_tag_text=\"" << tag_elem->original_tag_text << "\"\n    close_tag="
                              << tag_elem->close_tag << std::endl;
                }
            }
        }
        std::cout << "\n    justification=" << line_data[i].justification << std::endl;
    }
#endif

    for (unsigned int i = 0; i < line_data.size(); ++i) {
        if (retval.x < line_data[i].Width())
            retval.x = line_data[i].Width();
    }
    retval.y = text.empty() ? 0 : (static_cast<int>(line_data.size()) - 1) * m_lineskip + m_height;

#if DEBUG_DETERMINELINES
    std::cout << "String Size:(" << retval.x << ", " << retval.y << ")\n" << std::endl;
#endif

    return retval;
}

Pt Font::TextExtent(const std::string& text, Flags<TextFormat> format/* = FORMAT_NONE*/, int box_width/* = 0*/) const
{
    std::vector<LineData> lines;
    return DetermineLines(text, format, box_width ? box_width : 1 << 15, lines);
}

void Font::RegisterKnownTag(const std::string& tag)
{
    s_known_tags.insert(tag);
}

void Font::RemoveKnownTag(const std::string& tag)
{
    if (s_action_tags.find(tag) == s_action_tags.end())
        s_known_tags.erase(tag);
}

void Font::ClearKnownTags()
{
    s_action_tags.clear();
    s_action_tags.insert("i");
    s_action_tags.insert("u");
    s_action_tags.insert("rgba");
    s_action_tags.insert("left");
    s_action_tags.insert("center");
    s_action_tags.insert("right");
    s_action_tags.insert("pre");
    s_known_tags = s_action_tags;
}

void Font::Init(const std::string& font_filename, int pts, unsigned int range)
{
    if (s_action_tags.empty()) // if this is the first Font to get initialized, it needs to initialize some static members
        ClearKnownTags();

    FT_Error error;
    FT_Face face;
    FT_Fixed scale;

    // Open the font and create ancillary data
    error = FT_New_Face(g_library.library, font_filename.c_str(), 0, &face);
    if (error || !face)
        throw BadFile("Face object created from \'" + font_filename + "\' was invalid");
    if (pts <= 0)
        throw InvalidPointSize("Attempted to create font \'" + font_filename + "\' with non-positive point size");
    if (range > ALL_CHARS)
        throw InvalidRangeFlags("Attempted to create font \'" + font_filename + "\' with invalid range flags");
    if (!FT_IS_SCALABLE(face))
        throw UnscalableFont("Attempted to create font \'" + font_filename + "\' with uscalable font face");
    // Set the character size and use default 72 DPI
    if (FT_Set_Char_Size(face, 0, pts * 64, 0, 0)) // if error is returned
        throw BadPointSize("Could not set font size while attempting to create font \'" + font_filename + "\'");

    // Get the scalable font metrics for this font
    scale = face->size->metrics.y_scale;
    m_ascent  = static_cast<int>(face->size->metrics.ascender / 64.0); // convert from fixed-point 26.6 format
    m_descent  = static_cast<int>(face->size->metrics.descender / 64.0); // convert from fixed-point 26.6 format
    m_height  = m_ascent - m_descent + 1;
    m_lineskip = static_cast<int>(face->size->metrics.height / 64.0);
    // underline info
    m_underline_offset = std::floor(FT_MulFix(face->underline_position, scale) / 64.0);
    m_underline_height = std::ceil(FT_MulFix(face->underline_thickness, scale) / 64.0);
    if (m_underline_height < 1.0) {
        m_underline_height = 1.0;
    }
    // italics info
    m_italics_offset = ITALICS_FACTOR * m_height / 2.0;

    // these are the ranges of interest in the ASCII table that correspond to the values of GlyphRange
    enum {NUMBER_START = 0x30, NUMBER_STOP = 0x3A, ALPHA_UPPER_START = 0x41, ALPHA_UPPER_STOP = 0x5B, ALPHA_LOWER_START = 0x61,
          ALPHA_LOWER_STOP = 0x7B, SYMBOL_START1 = 0x21, SYMBOL_STOP1 = 0x30, SYMBOL_START2 = 0x3A, SYMBOL_STOP2 = 0x41,
          SYMBOL_START3 = 0x5B, SYMBOL_STOP3 = 0x61, SYMBOL_START4 = 0x7B, SYMBOL_STOP4 = 0x7F};

    std::vector<unsigned int> range_vec;
    if (range < ALL_CHARS) { // if certain specified ranges of glyphs are to be used
        range_vec.push_back(0x20); // the space character is always needed
        range_vec.push_back(0x21);
        if (range & NUMBER) {
            range_vec.push_back(NUMBER_START);        // '0'
            range_vec.push_back(NUMBER_STOP);         // '9' + 1
        }
        if (range & ALPHA_UPPER) {
            range_vec.push_back(ALPHA_UPPER_START);   // 'A'
            range_vec.push_back(ALPHA_UPPER_STOP);    // 'Z' + 1
        }
        if (range & ALPHA_LOWER) {
            range_vec.push_back(ALPHA_LOWER_START);   // 'a'
            range_vec.push_back(ALPHA_LOWER_STOP);    // 'z' + 1
        }
        if (range & SYMBOL) {
            range_vec.push_back(SYMBOL_START1);       // '!'
            range_vec.push_back(SYMBOL_STOP1);        // '/' + 1
            range_vec.push_back(SYMBOL_START2);       // ':'
            range_vec.push_back(SYMBOL_STOP2);        // '@' + 1
            range_vec.push_back(SYMBOL_START3);       // '['
            range_vec.push_back(SYMBOL_STOP3);        // '`' + 1
            range_vec.push_back(SYMBOL_START4);       // '{'
            range_vec.push_back(SYMBOL_STOP4);        // '~' + 1
        }
    } else { // all ASCII glyphs are to be used, even non-printable ones
        range_vec.push_back(0x00);
        range_vec.push_back(0xFF);
    }

    // define default image buffer size
    const int BUF_WIDTH = 256;
    const int BUF_HEIGHT = 256;
    const int BUF_SZ = BUF_WIDTH * BUF_HEIGHT;

    // declare std::vector of image buffers into which we will copy glyph images and create first buffer
    std::vector<unsigned short*> buffer_vec; // 16 bpp: we are creating a luminance + alpha image
    std::vector<Pt> buffer_sizes;
    std::map<FT_ULong, TempGlyphData> temp_glyph_data;
    typedef unsigned short ushort;
    unsigned short* temp_buf = new ushort[BUF_SZ]; // 16 bpp: we are creating a luminance + alpha image
    for (int i = 0; i < BUF_SZ; ++i)
        temp_buf[i] = 0;
    buffer_vec.push_back(temp_buf);
    buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));

    int x = 0, y = 0, max_x = 0;
    while (!range_vec.empty()) {
        unsigned int high = range_vec.back();
        range_vec.pop_back();
        unsigned int low = range_vec.back();
        range_vec.pop_back();

        // copy glyph images
        for (unsigned int c = low; c < high; ++c) {
            if (GenerateGlyph(face, static_cast<FT_ULong>(c))) {
                const FT_Bitmap& glyph_bitmap = face->glyph->bitmap;

                if (x + glyph_bitmap.width >= BUF_WIDTH) { // start a new row of glyph images
                    if (x > max_x) max_x = x;
                    x = 0;
                    y += m_height;
                    if (y + m_height >= BUF_HEIGHT) { // if there's not enough room for another row, create a new buffer
                        // cut off bottom portion of buffer just written, if it is possible to do so and maintain power-of-two height
                        int pow_of_2_x = NextPowerOfTwo(max_x), pow_of_2_y = NextPowerOfTwo(y + m_height);
                        if (pow_of_2_y < buffer_sizes.back().y)
                            buffer_sizes.back().y = pow_of_2_y;
                        if (pow_of_2_x < buffer_sizes.back().x)
                            buffer_sizes.back().x = pow_of_2_x;
                        x = y = 0;
                        typedef unsigned short ushort;
                        temp_buf = new ushort[BUF_SZ];
                        for (int i = 0; i < BUF_SZ; ++i)
                            temp_buf[i] = 0;
                        buffer_vec.push_back(temp_buf);
                        buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));
                    }
                }

                unsigned char*  src_start = glyph_bitmap.buffer;
                unsigned short* dst_start = buffer_vec.back() + y * BUF_WIDTH + x;

                int y_offset = m_height - 1 + m_descent - face->glyph->bitmap_top + FT_MAGIC_NUMBER;

                for (int row = 0; row < glyph_bitmap.rows; ++row) {
                    unsigned char*  src = src_start + row * glyph_bitmap.pitch;
                    unsigned short* dst = dst_start + (row + y_offset) * BUF_WIDTH;
                    for (int col = 0; col < glyph_bitmap.width; ++col) {
#ifdef __BIG_ENDIAN__
                        *dst++ = *src++ | (255 << 8); // big-endian uses different byte ordering
#else
                        *dst++ = (*src++ << 8) | 255; // alpha is the value from glyph_bitmap; luminance is always 100% white
#endif
                    }
                }

                // record info on how to find and use this glyph later
                temp_glyph_data[c] = TempGlyphData(static_cast<int>(buffer_vec.size()) - 1,
                                                   x, y + FT_MAGIC_NUMBER, x + glyph_bitmap.width, y + m_height + FT_MAGIC_NUMBER,
                                                   static_cast<int>((std::ceil(face->glyph->metrics.horiBearingX / 64.0))), // convert from 26.6 fixed point format and round up
                                                   static_cast<int>((std::ceil(face->glyph->metrics.horiAdvance / 64.0)))); // convert from 26.6 fixed point format and round up

                // advance buffer write-position
                x += glyph_bitmap.width;
            }
        }
    }

    // cut off bottom portion of last buffer, if it is possible to do so and maintain power-of-two height
    if (x > max_x) max_x = x;
    int pow_of_2_x = NextPowerOfTwo(max_x), pow_of_2_y = NextPowerOfTwo(y + m_height);
    if (pow_of_2_y < buffer_sizes.back().y)
        buffer_sizes.back().y = pow_of_2_y;
    if (pow_of_2_x < buffer_sizes.back().x)
        buffer_sizes.back().x = pow_of_2_x;

    // create opengl texture from buffer(s) and release buffer(s)
    for (unsigned int i = 0; i < buffer_vec.size(); ++i) {
        boost::shared_ptr<Texture> temp_texture(new Texture);
        temp_texture->Init(0, 0, buffer_sizes[i].x, buffer_sizes[i].y, BUF_WIDTH, (unsigned char*)(buffer_vec[i]), GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2);
        m_textures.push_back(temp_texture);
        delete [] buffer_vec[i];
    }

    // create Glyph objects from temp glyph data
    for (std::map<FT_ULong, TempGlyphData>::iterator it = temp_glyph_data.begin(); it != temp_glyph_data.end(); ++it)
        m_glyphs[it->first] = Glyph(m_textures[it->second.idx], it->second.x1, it->second.y1, it->second.x2, it->second.y2, it->second.left_b, it->second.adv);

    // record the width of the space character
    std::map<FT_ULong, Glyph>::const_iterator glyph_it = m_glyphs.find(CharToFT_ULong(' '));
    assert(glyph_it != m_glyphs.end());
    m_space_width = glyph_it->second.advance;
}

bool Font::GenerateGlyph(FT_Face face, FT_ULong ch)
{
    bool retval = true;

    // load the glyph
    if (!face)
        throw BadFace("GG::Font::GetGlyphBitmap : invalid font or font face");

    using boost::lexical_cast;
    FT_UInt index = FT_Get_Char_Index(face, ch);
    if (index) {
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT))
            throw BadGlyph((std::string("GG::Font::GetGlyphBitmap : Freetype could not load the glyph for character \'") + 
                            (ch < 256 ? lexical_cast<std::string>(char(ch)) : lexical_cast<std::string>(ch))) + "\'");

        FT_GlyphSlot glyph = face->glyph;

        // render the glyph
        if (FT_Render_Glyph(glyph, ft_render_mode_normal))
            throw BadGlyph((std::string("GG::Font::GetGlyphBitmap : Freetype could not render the glyph for character \'") + 
                            (ch < 256 ? lexical_cast<std::string>(char(ch)) : lexical_cast<std::string>(ch))) + "\'");
    } else {
        retval = false;
    }

    return retval;
}

int Font::RenderGlyph(int x, int y, const Glyph& glyph, const Font::RenderState* render_state) const
{
    if (render_state && render_state->use_italics) {
        // render subtexture to tilted rhombus instead of rectangle
        glBindTexture(GL_TEXTURE_2D, glyph.sub_texture.GetTexture()->OpenGLId());
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[1]);
        glVertex2d(x + glyph.left_bearing + m_italics_offset, y);
        glTexCoord2f(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[1]);
        glVertex2d(x + glyph.left_bearing + m_italics_offset + glyph.sub_texture.Width(), y);
        glTexCoord2f(glyph.sub_texture.TexCoords()[0], glyph.sub_texture.TexCoords()[3]);
        glVertex2d(x + glyph.left_bearing - m_italics_offset, y + glyph.sub_texture.Height());
        glTexCoord2f(glyph.sub_texture.TexCoords()[2], glyph.sub_texture.TexCoords()[3]);
        glVertex2d(x + glyph.left_bearing - m_italics_offset + glyph.sub_texture.Width(), y + glyph.sub_texture.Height());
        glEnd();
    } else {
        glyph.sub_texture.OrthoBlit(Pt(x + glyph.left_bearing, y));
    }
    if (render_state && render_state->draw_underline) {
        double x1 = x;
        double y1 = y + m_height + m_descent - m_underline_offset;
        double x2 = x1 + glyph.advance;
        double y2 = y1 + m_underline_height;
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glVertex2d(x1, y2);
        glVertex2d(x1, y1);
        glVertex2d(x2, y1);
        glVertex2d(x2, y2);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
    return glyph.advance;
}

void Font::HandleTag(const boost::shared_ptr<FormattingTag>& tag, double* orig_color, RenderState& render_state) const
{
    using boost::lexical_cast;
    if (tag->text == "i") {
        render_state.use_italics = !tag->close_tag;
    } else if (tag->text == "u") {
        render_state.draw_underline = !tag->close_tag;
    } else if (tag->text == "rgba") {
        if (tag->close_tag) {
            glColor4dv(orig_color);
            render_state.color_set = false;
        } else {
            bool well_formed_tag = true;
            if (4 <= tag->params.size()) {
                try {
                    int temp_color[4];
                    GLubyte color[4];
                    temp_color[0] = lexical_cast<int>(tag->params[0]);
                    temp_color[1] = lexical_cast<int>(tag->params[1]);
                    temp_color[2] = lexical_cast<int>(tag->params[2]);
                    temp_color[3] = lexical_cast<int>(tag->params[3]);
                    if (0 <= temp_color[0] && temp_color[0] <= 255 && 0 <= temp_color[1] && temp_color[1] <= 255 &&
                        0 <= temp_color[2] && temp_color[2] <= 255 && 0 <= temp_color[3] && temp_color[3] <= 255) {
                        color[0] = temp_color[0];
                        color[1] = temp_color[1];
                        color[2] = temp_color[2];
                        color[3] = temp_color[3];
                        glColor4ubv(color);
                        render_state.curr_color = Clr(color[0], color[1], color[2], color[3]);
                        render_state.color_set = true;
                    } else {
                        well_formed_tag = false;
                    }
                } catch (boost::bad_lexical_cast) {
                    try {
                        double color[4];
                        color[0] = lexical_cast<double>(tag->params[0]);
                        color[1] = lexical_cast<double>(tag->params[1]);
                        color[2] = lexical_cast<double>(tag->params[2]);
                        color[3] = lexical_cast<double>(tag->params[3]);
                        if (0.0 <= color[0] && color[0] <= 1.0 && 0.0 <= color[1] && color[1] <= 1.0 &&
                            0.0 <= color[2] && color[2] <= 1.0 && 0.0 <= color[3] && color[3] <= 1.0) {
                            glColor4dv(color);
                            render_state.curr_color = FloatClr(color[0], color[1], color[2], color[3]);
                            render_state.color_set = true;
                        } else {
                            well_formed_tag = false;
                        }
                    } catch (boost::bad_lexical_cast) {
                        well_formed_tag = false;
                    }
                }
            } else {
                well_formed_tag = false;
            }
            if (!well_formed_tag)
                std::cerr << "GG::Font : Encountered malformed <rgba> formatting tag: " << tag->original_tag_text;
        }
    }
}


///////////////////////////////////////
// class GG::FontManager
///////////////////////////////////////
// FontKey 
FontManager::FontKey::FontKey(const std::string& str, int pts) :
    filename(str),
    points(pts)
{
}

bool FontManager::FontKey::operator<(const FontKey& rhs) const
{
    return (filename < rhs.filename || (filename == rhs.filename && points < rhs.points));
}

// FontManager
FontManager::FontManager()
{}

boost::shared_ptr<Font> FontManager::GetFont(const std::string& font_filename, int pts, unsigned int range/* = Font::ALL_DEFINED_RANGES*/)
{
    static const boost::shared_ptr<Font> EMPTY_FONT(new Font("", 0));
    FontKey key(font_filename, pts);
    std::map<FontKey, boost::shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it == m_rendered_fonts.end()) { // if no such font has been created, create it now
        if (font_filename == "")
            return EMPTY_FONT; // keeps this function from throwing; "" is the only invalid font filename that shouldn't throw
        else
            return (m_rendered_fonts[key] = boost::shared_ptr<Font>(new Font(font_filename, pts, range)));
        // if a font like this has been created, but it doesn't have all the right glyphs, release it and create a new one
    } else if ((it->second->GetGlyphRange() & range) != range) {
        range |= it->second->GetGlyphRange();
        m_rendered_fonts.erase(it);
        return (m_rendered_fonts[key] = boost::shared_ptr<Font>(new Font(font_filename, pts, range)));
    } else { // otherwise, the font we found works, so just return it
        return it->second;
    }
}

void FontManager::FreeFont(const std::string& font_filename, int pts)
{
    FontKey key(font_filename, pts);
    std::map<FontKey, boost::shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it != m_rendered_fonts.end())
        m_rendered_fonts.erase(it);
}

FontManager& GG::GetFontManager()
{
    static FontManager manager;
    return manager;
}
