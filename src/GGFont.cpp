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

#include "GGFont.h"

#include <GGApp.h>
#include <GGBase.h>
#include <XMLValidators.h>

#include <cmath>
#include <boost/spirit.hpp>


namespace GG {

string RgbaTag(const Clr& c)
{
    stringstream stream;
    stream << "<rgba " << static_cast<int>(c.r) << " " << static_cast<int>(c.g) << " " << 
        static_cast<int>(c.b) << " " << static_cast<int>(c.a) << ">";
    return stream.str();
}


///////////////////////////////////////
// class GG::Font
///////////////////////////////////////
set<string> Font::s_action_tags;
set<string> Font::s_known_tags;

namespace {
int NextPowerOfTwo(int input)
{
    int value = 1;
    while (value < input)
        value <<= 1;
    return value;
}

/** This is used to collect data on the glyphs as they are recorded into buffers, for use in creating Glyph objects at the end
   of Font's constructor.*/
struct TempGlyphData
{
    TempGlyphData() {} ///< default ctor
    TempGlyphData(int i, int _x1, int _y1, int _x2, int _y2, int lb, int a) : idx(i), x1(_x1), y1(_y1), x2(_x2), y2(_y2), left_b(lb), adv(a) {} ///< ctor
    int      idx;              ///< index into m_textures of texture that contains this glyph
    int      x1, y1, x2, y2;   ///< area of glyph subtexture within texture
    int	     left_b;	       ///< left bearing (see Glyph)
    int      adv;              ///< advance of glyph (see Glyph)
};

struct FTLibraryWrapper
{
    GGEXCEPTION(FTLibraryWrapperException);   ///< exception class \see GG::GGEXCEPTION
    FTLibraryWrapper() : library(0)
    {
        if (!library && FT_Init_FreeType(&library)) // if no library exists and we can't create one...
            throw FTLibraryWrapperException("Unable to initialize FreeType font library object");
    }
    ~FTLibraryWrapper() {FT_Done_FreeType(library);}
    FT_Library library;
} g_library;

const double ITALICS_SLANT_ANGLE = 12; // degrees
const double ITALICS_FACTOR = 1.0 / tan((90 - ITALICS_SLANT_ANGLE) * 3.1415926 / 180.0); // factor used to shear glyphs ITALICS_SLANT_ANGLE degrees CW from straight up
}

Font::Font(const string& font_filename, int pts, Uint32 range/* = ALL_DEFINED_RANGES*/) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_glyph_range(range)
{
    if (font_filename != "")
	Init(font_filename, pts, range);
}

Font::Font(const XMLElement& elem)
{
    if (elem.Tag() != "GG::Font")
        throw std::invalid_argument("Attempted to construct a GG::Font from an XMLElement that had a tag other than \"GG::Font\"");

    string font_filename = elem.Child("m_font_filename").Text();
    int pts = lexical_cast<int>(elem.Child("m_pt_sz").Text());
    int range = lexical_cast<int>(elem.Child("m_glyph_range").Text());

    if (font_filename != "")
	Init(font_filename, pts, range);
}

int Font::RenderGlyph(int x, int y, char c) const
{
    std::map<char, Glyph>::const_iterator it = m_glyphs.find(c);
    if (it == m_glyphs.end())
        it = m_glyphs.find(' '); // print a space when an unrendered glyph is requested
    return RenderGlyph(x, y, it->second, 0);
}

int Font::RenderText(int x, int y, const string& text) const
{
    int orig_x = x;
    for (unsigned int i = 0; i < text.length(); ++i) {
        x += RenderGlyph(x, y, text[i]);
    }
    return x - orig_x;
}

void Font::RenderText(int x1, int y1, int x2, int y2, const string& text, Uint32& format, const vector<LineData>* line_data/* = 0*/, 
                      bool tags/* = false*/, Font::RenderState* render_state/* = 0*/) const
{
    RenderState state;
    if (!render_state) {
        render_state = &state;
    }

    double orig_color[4];
    glGetDoublev(GL_CURRENT_COLOR, orig_color);
    
    if (render_state->color_set)
        glColor4ubv(render_state->curr_color.v);

    // get breakdown of how text is divided into lines
    vector<LineData> lines;
    if (!line_data) {
        DetermineLines(text, format, x2 - x1, lines, tags);
        line_data = &lines;
    }

    // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
    bool expand_tabs = format & TF_LEFT;

    // get the width of a space character in pixels, and the spacing of tab stops in spaces and pixels
    std::map<char, Glyph>::const_iterator it = m_glyphs.find(' ');
    int tab_width = (format >> 16) & 0xFF; // width of tabs is embedded in bits 16-23
    if (!tab_width)
        tab_width = 8; // default tab width
    int tab_pixel_width = tab_width * m_space_width;  // get the length of a tab stop

    int rows_rendered = 0;
    int y_origin = y1; // default value for TF_TOP
    if (format & TF_BOTTOM)
	y_origin = y2 - (static_cast<int>(line_data->size()) - 1) * m_lineskip + m_height;
    else if (format & TF_VCENTER)
        y_origin = y1 + static_cast<int>(((y2 - y1) - ((static_cast<int>(line_data->size()) - 1) * m_lineskip + m_height)) / 2.0);
    int x = x1, y = y_origin;
    for (std::vector<LineData>::const_iterator cit = line_data->begin(); cit != line_data->end(); ++cit) {
        const LineData& curr_line = *cit;
        x = x1; // default value for TF_LEFT
        if (curr_line.justification == TF_RIGHT)
            x = x2 - curr_line.Width();
        else if (curr_line.justification == TF_CENTER)
            x = x1 + static_cast<int>(((x2 - x1) - curr_line.Width()) / 2.0);
        y = y_origin + rows_rendered * m_lineskip;
        for (int i = curr_line.begin_idx; i < curr_line.end_idx; ++i) {
            char c = text[i];
            if (c == '\t' && expand_tabs) {
                int curr_tab_location = (x - x1) / tab_pixel_width;   // find the nearest previous tab stop
                x = x1 + (curr_tab_location + 1) * tab_pixel_width;   // then find the location of the next tab stop
            } else {
                Tag tag;
                if (tags)
                    FindFormatTag(text, i, tag, render_state->ignore_tags);
                if (tag.char_length) {
                    HandleTag(tag, x, y, orig_color, *render_state);
                    i += tag.char_length - 1;
                } else {
                    it = m_glyphs.find(c);
                    if (it == m_glyphs.end())
                        it = m_glyphs.find(' '); // print a space when an unrendered glyph is requested
                    x += RenderGlyph(x, y, it->second, render_state);
                }
            }
        }
        ++rows_rendered;
    }

    glColor4dv(orig_color);
}

Pt Font::DetermineLines(const string& text, Uint32& format, int box_width, vector<LineData>& line_data, bool tags/* = false*/) const
{
    Pt retval;

    // correct any disagreements in the format flags
    int dup_ct = 0;   // duplication count
    if (format & TF_LEFT) ++dup_ct;
    if (format & TF_RIGHT) ++dup_ct;
    if (format & TF_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use TF_LEFT by default
        format &= ~(TF_RIGHT | TF_CENTER);
        format |= TF_LEFT;
    }
    dup_ct = 0;
    if (format & TF_TOP) ++dup_ct;
    if (format & TF_BOTTOM) ++dup_ct;
    if (format & TF_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use TF_TOP by default
        format &= ~(TF_BOTTOM | TF_VCENTER);
        format |= TF_TOP;
    }
    if ((format & TF_WORDBREAK) && (format & TF_LINEWRAP))   // only one of these can be picked; TF_WORDBREAK overrides TF_LINEWRAP
        format &= ~TF_LINEWRAP;

    RenderState render_state;
    int tab_width = (format >> 16) & 0xFF; // width of tabs is embedded in bits 16-23
    if (!tab_width) tab_width = 8;            // default tab width
    int tab_pixel_width = tab_width * m_space_width;   // get the length of a tab stop
    int x = 0;
    bool expand_tabs = format & TF_LEFT; // tab expansion only takes place when the lines are left-justified (otherwise, tabs are just spaces)
    Uint32 orig_just = format & (TF_LEFT | TF_CENTER | TF_RIGHT); // original justification as comes from the format
    bool last_line_of_curr_just = false; // is this the last line of the current justification? (for instance when a </right> tag is encountered)

    line_data.clear();
    line_data.push_back(LineData());
    line_data.back().begin_idx = 0;
    line_data.back().justification = orig_just;

    for (unsigned int i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == '\n') {                // if a newline is explicitly requested,
            line_data.back().end_idx = i;       // end the current line
            line_data.push_back(LineData());    // start a new one
            line_data.back().begin_idx = i + 1;
            if (last_line_of_curr_just) {
                line_data.back().justification = orig_just;
                last_line_of_curr_just = false;
            } else {
                line_data.back().justification = line_data[line_data.size() - 2].justification;
            }
            x = 0;                              // reset the x-position to 0
        } else if (c == '\t' && expand_tabs) {
            int curr_tab_location = x / tab_pixel_width;                         // find the nearest previous tab stop
            int next_tab_position = (curr_tab_location + 1) * tab_pixel_width;   // then find the location of the next tab stop
            if ((format & TF_LINEWRAP) && next_tab_position > box_width) { // if we're using linewrap and this tab won't fit on this line,
                if (!x && tab_pixel_width > box_width) { // if the tab is larger than the line and alone on the line,
                    line_data.back().end_idx = i + 1;   // let the tab overrun this line (so we don't skip a blank line, then have the tab overrun the next line anyway)
                    line_data.push_back(LineData());    // and then start a new one
                    line_data.back().begin_idx = i + 1;
                    x = 0;                              // and reset the x-position to 0
                    if (last_line_of_curr_just) {
                        line_data.back().justification = orig_just;
                        last_line_of_curr_just = false;
                    } else {
                        line_data.back().justification = line_data[line_data.size() - 2].justification;
                    }
                } else { // otherwise start a new line and put the tab there:
                    line_data.back().end_idx = i;             // end the current line
                    line_data.push_back(LineData());          // start a new one
                    line_data.back().begin_idx = i;
                    line_data.back().extents.push_back(x = tab_pixel_width); // advance the x-position to the first tab stop
                    if (last_line_of_curr_just) {
                        line_data.back().justification = orig_just;
                        last_line_of_curr_just = false;
                    } else {
                        line_data.back().justification = line_data[line_data.size() - 2].justification;
                    }
                }
            } else { // there's room for the tab, or we're not using linewrap
                line_data.back().extents.push_back(x = next_tab_position);
            }
        } else if (c != '\r' && c != '\f') { // c is a letter, number, symbol (whether included in the pre-rendered range of this font or not (see constructor)), or unexpanded tab (a space is substituted), except linefeed or carriage return
            if (tags) {
                Tag tag;
                FindFormatTag(text, i, tag, render_state.ignore_tags);
                if (tag.char_length) {
                    HandleTag(tag, 0, 0, 0, render_state);
                    if (tag.tokens[0] == "left") {
                        if (tag.close_tag) {
                            if (line_data.back().justification == TF_LEFT)
                                last_line_of_curr_just = true;
                        } else {
                            line_data.back().justification = TF_LEFT;
                            last_line_of_curr_just = false;
                        }
                    } else if (tag.tokens[0] == "center") {
                        if (tag.close_tag) {
                            if (line_data.back().justification == TF_CENTER)
                                last_line_of_curr_just = true;
                        } else {
                            line_data.back().justification = TF_CENTER;
                            last_line_of_curr_just = false;
                        }
                    } else if (tag.tokens[0] == "right") {
                        if (tag.close_tag) {
                            if (line_data.back().justification == TF_RIGHT)
                                last_line_of_curr_just = true;
                        } else {
                            line_data.back().justification = TF_RIGHT;
                            last_line_of_curr_just = false;
                        }
                    }
                    i += tag.char_length - 1; // "- 1" because i gets incremented at the end of each loop iteration
                    // now represent all these skipped characters with the same extent value
                    int extent = line_data.back().extents.empty() ? 0 : line_data.back().extents.back();
                    for (int j = 0; j < tag.char_length; ++j) {
                        line_data.back().extents.push_back(extent);
                    }
                    continue;
                }
            }

            std::map<char, Glyph>::const_iterator it = m_glyphs.find(c);
            if (it == m_glyphs.end()) {
                it = m_glyphs.find(' ');   // use a space when an unrendered glyph is requested (the space char is always rendered)
                c = ' ';                   // and reflect this choice in the value of c
            }
            int char_advance = it->second.advance;
            int char_width = it->second.width;
            if (format & TF_LINEWRAP) { // any symbol that doesn't fit on the rest of the current line gets moved down to the next line*
                if (x + char_width > box_width && (x || char_width <= box_width)) {// *unless it's greater than the box width and alone on the line (so we don't skip a blank line, then have the character overrun the next line anyway)
                    line_data.back().end_idx = i;                         // end the current line
                    line_data.push_back(LineData());                      // start a new one
                    line_data.back().begin_idx = i;
                    line_data.back().extents.push_back(x = char_advance); // advance the x-position the width of the character
                    if (last_line_of_curr_just) {
                        line_data.back().justification = orig_just;
                        last_line_of_curr_just = false;
                    } else {
                        line_data.back().justification = line_data[line_data.size() - 2].justification;
                    }
                } else { // the symbol will fit on the rest of this line
                    line_data.back().extents.push_back(x += char_advance);
                }
            } else if (format & TF_WORDBREAK) { // lines are wrapped, but words are not broken
                int word_start = i;
                int word_advance = 0;
                int word_width = 0;
                vector<int> word_extents;
                // first get the characters of the word itself
                while (i < text.size() && !isspace(c = text[i])) {
                    it = m_glyphs.find(c);
                    if (it == m_glyphs.end()) {
                        it = m_glyphs.find(' ');
                        c = ' ';
                    }
                    word_extents.push_back(word_advance += it->second.advance);
                    word_width = word_advance - (it->second.advance - it->second.width);
                    ++i;
                }
                // then get all the spaces after the word (but not tabs and newlines, unless tabs are being treated as spaces)
                while (i < text.size() && ((c = text[i]) == ' ' || (!expand_tabs && c == '\t'))) {
                    it = m_glyphs.find(c);
                    if (it == m_glyphs.end()) {
                        it = m_glyphs.find(' ');
                        c = ' ';
                    }
                    word_extents.push_back(word_advance += it->second.advance);
                    ++i;
                }
                // if the word exceeds the rest of the space on the line AND it's not alone on the line, or it could fit all on one line,
                // move the word to the next line
                if (x + word_width > box_width && (x || word_width <= box_width)) {
                    line_data.back().end_idx = word_start; // end the current line before this word
                    line_data.push_back(LineData());       // start a new one
                    line_data.back().begin_idx = word_start;
                    line_data.back().extents.insert(line_data.back().extents.end(), word_extents.begin(), word_extents.end());
                    x = word_advance;                      // advance the x-position the width of the character
                    if (last_line_of_curr_just) {
                        line_data.back().justification = orig_just;
                        last_line_of_curr_just = false;
                    } else {
                        line_data.back().justification = line_data[line_data.size() - 2].justification;
                    }
                } else {
                    for (unsigned int i = 0; i < word_extents.size(); ++i) {
                        line_data.back().extents.push_back(x + word_extents[i]); // must add x, since the extents start with 0 being the start of the word
                    }
                    x += word_advance;
                }
                if (word_advance) // if we found the end of a word, the last character after the last character of the word was not really processed
                    --i;
            } else { // no wrapping at all; everything goes on a single line unless there are '\n' characters in the text
                line_data.back().extents.push_back(x += char_advance);
            }
        }
    }
    line_data.back().end_idx = text.size();

    // find text height, and longest line for width
    retval.y = (static_cast<int>(line_data.size()) - 1) * m_lineskip + m_height;
    for (unsigned int i = 0; i < line_data.size(); ++i)
        if (!line_data[i].extents.empty() && line_data[i].extents.back() > retval.x)
            retval.x = line_data[i].extents.back();

    return retval;
}

Pt Font::TextExtent(const string& text, Uint32 format/* = TF_NONE*/, int box_width/* = 0*/, bool tags/* = false*/) const
{
    vector<LineData> lines;
    return DetermineLines(text, format, box_width ? box_width : 1 << 15, lines, tags);
}

XMLElement Font::XMLEncode() const
{
    XMLElement retval("GG::Font");
    retval.AppendChild(XMLElement("m_font_filename", m_font_filename));
    retval.AppendChild(XMLElement("m_pt_sz", lexical_cast<string>(m_pt_sz)));
    retval.AppendChild(XMLElement("m_glyph_range", lexical_cast<string>(m_glyph_range)));
    return retval;
}

XMLElementValidator Font::XMLValidator() const
{
    XMLElementValidator retval("GG::Font");
    retval.AppendChild(XMLElementValidator("m_font_filename"));
    retval.AppendChild(XMLElementValidator("m_pt_sz", new Validator<int>()));
    retval.AppendChild(XMLElementValidator("m_glyph_range", new Validator<int>()));
    return retval;
}

void Font::RegisterKnownTag(const string& tag)
{
    s_known_tags.insert(tag);
}

void Font::RemoveKnownTag(const string& tag)
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

void Font::FindFormatTag(const string& text, int idx, Tag& tag, bool ignore_tags/* = false*/)
{
    tag = Tag();
    if (text[idx] == '<') {
        int close_brace_posn = text.find('>', idx);
        if (close_brace_posn != static_cast<int>(string::npos)) {
            tag.close_tag = text[idx + 1] == '/';
            int tag_text_start_posn = idx + (tag.close_tag ? 2 : 1);
            string tag_str;
            if (tag_text_start_posn < close_brace_posn) {
                tag_str = text.substr(tag_text_start_posn, close_brace_posn - tag_text_start_posn);
                using namespace boost::spirit;
                parse(tag_str.c_str(), *(*space_p >> (+(anychar_p - space_p))[append(tag.tokens)]));
                if (s_known_tags.find(tag.tokens[0]) != s_known_tags.end() &&
                    (!ignore_tags || (tag.tokens[0] == "pre" && tag.close_tag))) // a known tag
                    tag.char_length = close_brace_posn - idx + 1;
            }
        }
    }
}

void Font::Init(const string& font_filename, int pts, Uint32 range)
{
    if (s_action_tags.empty()) // if this is the first Font to get initialized, it needs to initialize some static members
        ClearKnownTags();

    FT_Error error;
    FT_Face face;
    FT_Fixed scale;

    // Open the font and create ancillary data
    error = FT_New_Face(g_library.library, font_filename.c_str(), 0, &face);
    if (error || !face)
        throw FontException("Face object created from \'" + font_filename + "\' was invalid");
    if (pts <= 0)
        throw FontException("Attempted to create font \'" + font_filename + "\' with non-positive point size");
    if (range > ALL_CHARS)
        throw FontException("Attempted to create font \'" + font_filename + "\' with invalid range flags");
    if (!FT_IS_SCALABLE(face))
        throw FontException("Attempted to create font \'" + font_filename + "\' with uscalable font face");
    // Set the character size and use default 72 DPI
    if (FT_Set_Char_Size(face, 0, pts * 64, 0, 0)) // if error is returned
        throw FontException("Could not set font size while attempting to create font \'" + font_filename + "\'");

    // Get the scalable font metrics for this font
    scale = face->size->metrics.y_scale;
    m_ascent  = int(std::ceil(FT_MulFix(face->bbox.yMax, scale) / 64.0)); // convert from fixed-point 26.6 format
    m_descent = int(std::ceil(FT_MulFix(face->bbox.yMin, scale) / 64.0));
    m_height  = m_ascent - m_descent + 1;
    m_lineskip = int(std::ceil(FT_MulFix(face->height, scale) / 64.0));
    // underline info
    m_underline_offset = std::ceil(FT_MulFix(face->underline_position, scale) / 64.0);
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

    vector<int> range_vec;
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

    // declare vector of image buffers into which we will copy glyph images and create first buffer
    vector<Uint16*> buffer_vec; // 16 bpp: we are creating a luminance + alpha image
    vector<Pt> buffer_sizes;
    map<char, TempGlyphData> temp_glyph_data;
    Uint16* temp_buf = new Uint16[BUF_SZ]; // 16 bpp: we are creating a luminance + alpha image
    for (int i = 0; i < BUF_SZ; ++i)
        temp_buf[i] = 0;
    buffer_vec.push_back(temp_buf);
    buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));

    int x = 0, y = 0, max_x = 0;
    while (!range_vec.empty()) {
        int high = range_vec.back();
        range_vec.pop_back();
        int low = range_vec.back();
        range_vec.pop_back();

        // copy glyph images
        for (int c = low; c < high; ++c) {
            if (GenerateGlyph(face, c)) {
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
                        temp_buf = new Uint16[BUF_SZ];
                        for (int i = 0; i < BUF_SZ; ++i)
                            temp_buf[i] = 0;
                        buffer_vec.push_back(temp_buf);
                        buffer_sizes.push_back(Pt(BUF_WIDTH, BUF_HEIGHT));
                    }
                }

                Uint8*  src_start = glyph_bitmap.buffer;
                Uint16* dst_start = buffer_vec.back() + y * BUF_WIDTH + x;

                int y_offset = m_height - 1 + m_descent - face->glyph->bitmap_top;

                for (int row = 0; row < glyph_bitmap.rows; ++row) {
                    Uint8*  src = src_start + row * glyph_bitmap.pitch;
                    Uint16* dst = dst_start + (row + y_offset) * BUF_WIDTH;
                    for (int col = 0; col < glyph_bitmap.width; ++col)
                        *dst++ = (*src++ << 8) | 255; // alpha is the value from glyph_bitmap; luminance is always 100% white
                }

                // record info on how to find and use this glyph later
                temp_glyph_data[c] = TempGlyphData(static_cast<int>(buffer_vec.size()) - 1,
                                                   x, y, x + glyph_bitmap.width, y + m_height,
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
        shared_ptr<Texture> temp_texture(new Texture);
        temp_texture->Init(0, 0, buffer_sizes[i].x, buffer_sizes[i].y, BUF_WIDTH, (unsigned char*)(buffer_vec[i]), 2);
        m_textures.push_back(temp_texture);
        delete [] buffer_vec[i];
    }

    // create Glyph objects from temp glyph data
    for (std::map<char, TempGlyphData>::iterator it = temp_glyph_data.begin(); it != temp_glyph_data.end(); ++it)
        m_glyphs[it->first] = Glyph(m_textures[it->second.idx], it->second.x1, it->second.y1, it->second.x2, it->second.y2, it->second.left_b, it->second.adv);

    // record the width of the space character
    std::map<char, Glyph>::const_iterator glyph_it = m_glyphs.find(' ');
    m_space_width = glyph_it->second.advance;
}

bool Font::GenerateGlyph(FT_Face face, char ch)
{
    bool retval = true;

    // load the glyph
    if (!face)
        throw FontException("GG::Font::GetGlyphBitmap : invalid font or font face");

    FT_UInt index = FT_Get_Char_Index(face, ch);
    if (index) {
        if (FT_Load_Glyph(face, index, FT_LOAD_DEFAULT))
            throw FontException((string("GG::Font::GetGlyphBitmap : Freetype could not load the glyph for character \'") + ch) + "\'");

        FT_GlyphSlot glyph = face->glyph;

        // render the glyph
        if (FT_Render_Glyph(glyph, ft_render_mode_normal))
            throw FontException((string("GG::Font::GetGlyphBitmap : Freetype could not render the glyph for character \'") + ch) + "\'");
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
        glyph.sub_texture.OrthoBlit(x + glyph.left_bearing, y, false);
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

void Font::HandleTag(const Tag& tag, int x, int y, const double* orig_color, Font::RenderState& render_state) const
{
    bool rendering = orig_color; // if orig_color == 0, we're not rendering the text, just getting lines and extents
    if (tag.tokens[0] == "i" && rendering) {
        if (!render_state.use_italics && !tag.close_tag)
            render_state.use_italics = true;
        else if (render_state.use_italics && tag.close_tag)
            render_state.use_italics = false;
    } else if (tag.tokens[0] == "u" && rendering) {
        if (!tag.close_tag) {
            render_state.draw_underline = true;
        } else {
            render_state.draw_underline = false;
        }
    } else if (tag.tokens[0] == "rgba" && rendering) {
        if (tag.close_tag) {
            glColor4dv(orig_color);
            render_state.color_set = false;
        } else {
            bool well_formed_tag = true;
            if (5 <= tag.tokens.size()) {
                try {
                    int temp_color[4];
                    Uint8 color[4];
                    temp_color[0] = lexical_cast<int>(tag.tokens[1]);
                    temp_color[1] = lexical_cast<int>(tag.tokens[2]);
                    temp_color[2] = lexical_cast<int>(tag.tokens[3]);
                    temp_color[3] = lexical_cast<int>(tag.tokens[4]);
                    if (0 <= temp_color[0] && temp_color[0] <= 255 && 0 <= temp_color[1] && temp_color[1] <= 255 &&
                        0 <= temp_color[2] && temp_color[2] <= 255 && 0 <= temp_color[3] && temp_color[3] <= 255) {
                        color[0] = temp_color[0];
                        color[1] = temp_color[1];
                        color[2] = temp_color[2];
                        color[3] = temp_color[3];
                        glColor4ubv(color);
                        render_state.curr_color = Clr(color);
                        render_state.color_set = true;
                    } else {
                        well_formed_tag = false;
                    }
                } catch (boost::bad_lexical_cast) {
                    try {
                        double color[4];
                        color[0] = lexical_cast<double>(tag.tokens[1]);
                        color[1] = lexical_cast<double>(tag.tokens[2]);
                        color[2] = lexical_cast<double>(tag.tokens[3]);
                        color[3] = lexical_cast<double>(tag.tokens[4]);
                        if (0.0 <= color[0] && color[0] <= 1.0 && 0.0 <= color[1] && color[1] <= 1.0 &&
                            0.0 <= color[2] && color[2] <= 1.0 && 0.0 <= color[3] && color[3] <= 1.0) {
                            glColor4dv(color);
                            render_state.curr_color = Clr(color);
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
            if (!well_formed_tag) {
                string error_msg = "GG::Font : Encountered malformed <rgba> formatting tag: ";
                error_msg += (tag.close_tag ? "</" : "<");
                for (unsigned int i = 0; i < tag.tokens.size(); ++i) {
                    error_msg += tag.tokens[i];
                    if (i < tag.tokens.size() - 1)
                        error_msg += " ";
                }
                error_msg += ">";
                App::GetApp()->Logger().error(error_msg);
            }
        }
    } else if (tag.tokens[0] == "pre") {
        render_state.ignore_tags = !tag.close_tag;
    }
}



///////////////////////////////////////
// class GG::FontManager
///////////////////////////////////////
// static member(s)
bool FontManager::s_created = false;

FontManager::FontManager()
{
    if (s_created)
        throw FontManagerException("Attempted to create a second instance of GG::FontManager");
    s_created = true;
}

shared_ptr<Font> FontManager::GetFont(const string& font_filename, int pts, Uint32 range/* = GGFont::ALL_DEFINED_RANGES*/)
{
    static const shared_ptr<Font> EMPTY_FONT(new Font("", 0));
    FontKey key(font_filename, pts);
    std::map<FontKey, shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it == m_rendered_fonts.end()) { // if no such font has been created, create it now
        if (font_filename == "")
            return EMPTY_FONT; // keeps this function from throwing; "" is the only invalid font filename that shouldn't throw
        else
            return (m_rendered_fonts[key] = shared_ptr<Font>(new Font(font_filename, pts, range)));
        // if a font like this has been created, but it doesn't have all the right glyphs, release it and create a new one
    } else if ((it->second->GetGlyphRange() & range) != range) {
        range |= it->second->GetGlyphRange();
        m_rendered_fonts.erase(it);
        return (m_rendered_fonts[key] = shared_ptr<Font>(new Font(font_filename, pts, range)));
    } else { // otherwise, the font we found works, so just return it
        return it->second;
    }
}

void FontManager::FreeFont(const string& font_filename, int pts)
{
    FontKey key(font_filename, pts);
    std::map<FontKey, shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it != m_rendered_fonts.end())
        m_rendered_fonts.erase(it);
}

} // namespace GG

