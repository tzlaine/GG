// -*- C++ -*-
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

#ifndef _GGFont_h_
#define _GGFont_h_

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

#ifndef __FREETYPE_H__
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace GG {

/** returns a string of the form "<rgba r g b a>" from a Clr object with color channels r, b, g, a.*/
GG_API string RgbaTag(const Clr& c);


/** This class creates one or more 16-bpp OpenGL textures that contain rendered text from a requested font file at the 
    requested point size, including only the requested ranges of characters.  
    Once the textures have been created, text is rendered to the display by rendering polygons textured with portions of the 
    rendered textures that contain individual character images.  The characters are rendered 
    to the textures in white, with alpha blending used for antialiasing.  The user should set the desired text color with a 
    call to glColor*() before any call to RenderText().  When text is rendered, DetermineLines() is called to determine 
    where the line breaks are, so that text can be rendered centered, right-justified, or whatever.  To cut down on this 
    computation, when the text is not changing very rapidly 
    (ie not every frame), DetermineLines() can be called by the user once, and the result supplied to RenderText() repeatedly.  
    When this is done, the iteration through the text to determine line breaks is not necessary at render time.  The user is 
    responsible for ensuring that the line data applies to the text string supplied to RenderText().  The total range of 
    characters 
    covered by ALL_DEFINED_RANGES is 0x20 (' ') to 0x7E ('~').  SYMBOL covers everything in that range that isn't a letter or 
    digit.  ALL_CHARS covers all 256 ASCII character codes from 0x00 to 0xFF.  The other ranges are self-explanatory.  Point 
    sizes above 250 are not supported. 
    <br><h3>Text Formatting Tags</h3>
    GG::Font supports a few text formatting tags for convenience.  These tags are similar to HTML or XML tags; there is an
    opening version "<tag>" and a closing version "</tag>" of each tag.  It is important to note that GG::Font tags represent
    state change, and so cannot be meaningfully nested.  For instance, consider the use of the italics tag \<i> here:
    \verbatim
      <i>some text <i>and </i>some more text</i>\endverbatim
    In this example, everything is italicized except for "some more text".  Each \<i> tag establishes that italics should be 
    used for all further text until the next \</i> tag.  So the second \<i> tag is redundant, as is the second \</i> tag.  Each 
    respectively activates or deactivates italics when that is already the current state.  The text justification tags are used
    on a per-line basis, since it makes no sense to right-justify only a part of a line and center the rest, for instance. 
    When more than one justification tag appears on a line, the last one is used.  A justification close-tag indicates that 
    a line is to be the last one with that justification, and only applies if that justification is active.
    <br>The supported tags are:
    - \verbatim<i></i>\endverbatim                 Italics
    - \verbatim<u></u>\endverbatim                 Underline
    - \verbatim<rgba r g b a></rgba>\endverbatim   Color. Sets current rendering color to that specified by parameters.  Parameters may be either floating point values between 0.0 and 1.0 inclusive, or 8-bit integer values between 0 and 255 inclusive.  All parameters must be in one format or the other.  The \</rgba> tag does not restore the previously set \<rgba> color, but instead restores the default color used to render the text.  (Example tag: <rgba 0.4 0.5 0.6 0.7>)
    - \verbatim<left></left>\endverbatim           Left-justified text.
    - \verbatim<center></center>\endverbatim       Centered text.
    - \verbatim<right></right>\endverbatim         Right-justified text.
    - \verbatim<pre></pre>\endverbatim             Preformatted.  Similar to HTML \<pre\> tag, except this one only causes all tags to be ignored until a subsequent \</pre\> tag is seen.
    <p>
    Users of Font may wish to create their own tags as well.  Though Font will not be able to handle new tags without reworking
    the Font code, it is possible to let Font know about other tags, in order for Font to render them invisible as it does 
    with the tags listed above.  See the static methods RegisterKnownTag(), RemoveKnownTag(), and ClearKnownTags() for details.
    It is not possible to accidentally or intentionally remove the built-in tags using these methods.  If you wish not to use 
    tags at all, call DetermineLines() and RenderText() with the parameter tags set to false (this is the default value), or 
    include a \<pre\> tag at the beginning of the text to be rendered.
   */
class GG_API Font
{
public:
    /** This struct is used to render GG:Font text.  
        It holds vital data on each line that a string occupies when rendered with given format flags.  Each value 
        extents[i] is the furthest that the right side of the i-th character extends from the beginning of the line.*/
    struct GG_API LineData
    {
        int Width() const {return (!extents.empty() ? extents.back() : 0);}; ///< returns the width of the line, in pixels

        int         begin_idx;      ///< the index into the text string of the first character of the line
        int         end_idx;        ///< the index of the last + 1 character of the line
        vector<int> extents;        ///< the cummulative extents of the characters on this line of the text (0 extent is the left side)
        Uint32      justification;  ///< TF_LEFT, TF_CENTER, or TF_RIGHT; derived from text format flags and/or formatting tags in the text
    };

    /** This just holds the essential data necessary to represent a text formatting tag.*/
    struct GG_API Tag
    {
        Tag() : close_tag(false), char_length(0) {} ///< default ctor
 
        vector<string> tokens;        ///< the tokens of the tag (tokens are broken up by ' ' and '\\t' whitespace characters)
        bool           close_tag;     ///< true if this is a "close" tag (eg "</i>")
        int            char_length;   ///< the total number of characters that comprise the tag, including brackets and whitespace.  0 indicates that this is not a valid tag.
    };

    /** This holds the state of tags during rendering of text.  By keeping track of this state across multiple calls to
        RenderText(), the user can preserve the functionality of the text formatting tags, if present.*/
    struct GG_API RenderState
    {
        RenderState() : ignore_tags(false), use_italics(false), draw_underline(false), color_set(false) {} ///< default ctor

        bool    ignore_tags;        ///< set to true upon encountering a \<pre> tag, and to false when a \</pre> tag is seen
        bool    use_italics;        ///< set to true upon encountering an \<i> tag, and to false when an \</i> tag is seen
        bool    draw_underline;     ///< set to true upon encountering an \<u> tag, and to false when an \</u> tag is seen
        bool    color_set;          ///< true when a tag has set the current color
        Clr     curr_color;         ///< the current text color (as set by a tag)
    };

    GGEXCEPTION(FontException);   ///< exception class \see GG::GGEXCEPTION

    /** the ranges of character glyphs to be rendered in this font*/
    enum GlyphRange {NUMBER = 1,                ///< only the numbers ('0' - '9')
                     ALPHA_UPPER = 2,           ///< only the uppercase alphabet ('A' - 'Z')
                     ALPHA_LOWER = 4,           ///< only the lowercase alphabet ('a' - 'z')
                     SYMBOL = 8,                ///< everything printable not covered above
                     ALL_DEFINED_RANGES = 15,   ///< everything above (all printable characters)
                     ALL_CHARS = 31             ///< all characters (0x00 - 0xFF)
                    };
      
    /** \name Structors */ //@{
    Font(const string& font_filename, int pts, Uint32 range = ALL_DEFINED_RANGES); ///< ctor. \throw FontException This constructor may throw if a valid font file is not found, or a Font cannot be created for some other reason.
    Font(const XMLElement& elem); ///< ctor that constructs a Font object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Font object
    virtual ~Font() {} ///< virtual dtor
    //@}

    /** \name Accessors */ //@{
    const string&     FontName() const     {return m_font_filename;}  ///< returns the name of the file from which this font was created
    int               PointSize() const    {return m_pt_sz;}          ///< returns the point size in which the characters in the font object are rendered
    int               GetGlyphRange() const{return m_glyph_range;}    ///< returns the range(s) of characters rendered in the font \see GlyphRange
    int               Ascent() const       {return m_ascent;}         ///< returns the maximum amount above the baseline the text can go, in pixels
    int               Descent() const      {return m_descent;}        ///< returns the maximum amount below the baseline the text can go, in pixels
    int               Height() const       {return m_height;}         ///< returns (Ascent() - Descent()), in pixels
    int               Lineskip() const     {return m_lineskip;}       ///< returns the distance that should be placed between lines, in pixels.  This is usually not equal to Height().
    int               SpaceWidth() const{return m_space_width;}    ///< returns the width in pixels of the glyph for the space character
    int               RenderGlyph(const Pt& pt, char c) const {return RenderGlyph(pt.x, pt.y, c);}   ///< renders glyph for \a c and returns advance of glyph rendered
    int               RenderGlyph(int x, int y, char c) const;     ///< renders glyph for \a c and returns advance of glyph rendered
    int               RenderText(const Pt& pt, const string& text) const {return RenderText(pt.x, pt.y, text);}  ///< unformatted text rendering; repeatedly calls RenderGlyph, then returns advance of entire string; treats formatting tags as regular text unless \a tags == true
    int               RenderText(int x, int y, const string& text) const; ///< unformatted text rendering; repeatedly calls RenderGlyph, then returns advance of entire string; treats formatting tags as regular text unless \a tags == true
    void              RenderText(const Pt& pt1, const Pt& pt2, const string& text, Uint32& format, const vector<LineData>* line_data = 0, bool tags = false, RenderState* render_state = 0) const {RenderText(pt1.x, pt1.y, pt2.x, pt2.y, text, format, line_data, tags, render_state);} ///< formatted text rendering; treats formatting tags as regular text unless \a tags == true
    void              RenderText(int x1, int y1, int x2, int y2, const string& text, Uint32& format, const vector<LineData>* line_data = 0, bool tags = false, RenderState* render_state = 0) const; ///< formatted text rendering; treats formatting tags as regular text unless \a tags == true
    Pt                DetermineLines(const string& text, Uint32& format, int box_width, vector<LineData>& lines, bool tags = false) const; ///< returns the maximum dimensions of the string in x and y; treats formatting tags as regular text unless \a tags == true
    Pt                TextExtent(const string& text, Uint32 format = TF_NONE, int box_width = 0, bool tags = false) const; ///< returns the maximum dimensions of the string in x and y.  Provided as a convenience; it just calls DetermineLines with the given parameters.

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Font object
    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Font
    //@}
   
    static void       RegisterKnownTag(const string& tag);   ///< adds \a tag to the list of embedded tags that Font should not print when rendering text.  Passing "foo" will cause Font to treat "<foo>", \<foo [arg1 [arg2 ...]]>, and "</foo>" as tags.
    static void       RemoveKnownTag(const string& tag);     ///< removes \a tag from the known tag list
    static void       ClearKnownTags();                      ///< removes all tags from the known tag list.  Does not remove the built in tags: \<i>, \<u>, \<rgba r g b a>, and \<pre>.
    static void       FindFormatTag(const string& text, int idx, Tag& tag, bool ignore_tags = false); ///< fills \a tag with the data on the first tag found from location \a idx in \a text

private:
    /** This just holds the essential data necessary to render a glyph from the OpenGL texture(s) 
        created at GG::Font creation time.*/
    struct Glyph
    {
        Glyph() : advance(0) {} ///< default ctor
        Glyph(const shared_ptr<Texture>& texture, int x1, int y1, int x2, int y2, int lb, int adv) : 
            sub_texture(texture, x1, y1, x2, y2), left_bearing(lb), advance(adv), width(x2 - x1) {} ///< ctor

        SubTexture  sub_texture;   ///< the subtexture containing just this glyph
        int	    left_bearing;  ///< the space that should remain before the glyph
        int         advance;       ///< the amount of space the glyph should occupy, including glyph graphic and inter-glyph spacing
        int         width;         ///< the width of the glyph only
    };

    void              Init(const string& font_filename, int pts, Uint32 range);
    bool              GenerateGlyph(FT_Face font, char ch);
    inline int        RenderGlyph(int x, int y, const Glyph& glyph, const RenderState* render_state) const;
    void              HandleTag(const Tag& tag, int x, int y, const double* orig_color, RenderState& render_state) const;

    string               m_font_filename;
    int                  m_pt_sz;
    int                  m_glyph_range; ///< the (bitwise or'ed) GlyphRanges that are covered by this font object
    int                  m_ascent;      ///< maximum amount above the baseline the text can go
    int                  m_descent;     ///< maximum amount below the baseline the text can go
    int                  m_height;      ///< ascent - descent
    int                  m_lineskip;    ///< distance that should be placed between lines
    double               m_underline_offset; ///< amount below the baseline that the underline sits
    double               m_underline_height; ///< height (thickness) of underline
    double               m_italics_offset;   ///< amount that the top of an italicized glyph is left of the bottom
    int                  m_space_width; ///< the width in pixels of the glyph for the space character
    map<char, Glyph>     m_glyphs;      ///< the locations of the images of each glyph within the textures
    vector<shared_ptr<Texture> >
                         m_textures;    ///< the OpenGL texture objects in which the glyphs can be found
   
    static set<string>   s_action_tags; ///< embedded tags that Font must act upon when rendering are stored here
    static set<string>   s_known_tags;  ///< embedded tags that Font knows about but should not act upon are stored here
};


// define EnumMap and stream operators for Font::GlyphRange
ENUM_MAP_BEGIN(Font::GlyphRange)
    ENUM_MAP_INSERT(Font::NUMBER)
    ENUM_MAP_INSERT(Font::ALPHA_UPPER)
    ENUM_MAP_INSERT(Font::ALPHA_LOWER)
    ENUM_MAP_INSERT(Font::SYMBOL)
    ENUM_MAP_INSERT(Font::ALL_DEFINED_RANGES)
    ENUM_MAP_INSERT(Font::ALL_CHARS)
ENUM_MAP_END

ENUM_STREAM_IN(Font::GlyphRange)
ENUM_STREAM_OUT(Font::GlyphRange)


/** This singleton class is essentially a very thin wrapper around a map of Font smart pointers, keyed on font filename/point size pairs.  
    The user need only request a font through GetFont(); if the font at the requested size needs to be created, the font is 
    created at the requestd size, a shared_ptr to it is kept, and a copy of the shared_ptr is returned.  If the font has been 
    created at the desired size, but the request includes character range(s) not already created, the font at the requested 
    size is created with the union of the reqested and existing ranges, stored, and returned as above; the only difference is 
    that the original shared_ptr is destroyed.  Due to the shared_ptr semantics, the object pointed to by the shared_ptr is deleted 
    if and only if the last shared_ptr that refers to it is deleted.  So any requested font can be used as long as the caller 
    desires, even when another caller tells the FontManager to free the font.*/
class GG_API FontManager
{
private:
    /// This GG::FontManager-private struct is used as a key type for the map of rendered fonts.
    struct FontKey 
    {
        FontKey(const string& str, int pts) : filename(str), points(pts) {} ///< ctor
        bool operator<(const FontKey& rhs) const {return (filename < rhs.filename || (filename == rhs.filename && points < rhs.points));} ///< less-than operator

        string   filename;   ///< the name of the file from which this font was created
        int      points;     ///< the point size in which this font was rendered
    };

public:
    GGEXCEPTION(FontManagerException);   ///< exception class \see GG::GGEXCEPTION

    /** \name Structors */ //@{
    FontManager(); ///< ctor
    //@}

    /** \name Mutators */ //@{
    shared_ptr<Font> GetFont(const string& font_filename, int pts, Uint32 range = Font::ALL_DEFINED_RANGES); ///< returns a shared_ptr to the requested font.  May load font if unavailable at time of request.
    void             FreeFont(const string& font_filename, int pts); ///< removes the indicated font from the font manager.  Due to shared_ptr semantics, the font may not be deleted until much later.
    //@}

private:
    static bool                      s_created;
    map<FontKey, shared_ptr<Font> >  m_rendered_fonts;
};

} // namespace GG

#endif // _GGFont_h_

