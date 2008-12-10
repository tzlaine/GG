// -*- C++ -*-
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

/** \file Font.h
    Contains the Font class, a class that encapsulates the rendering of a
    single FreeType-compatible font in italics, with underlining, left-,
    right-, or center- justified, etc., and the FontManager class which
    provides application-wide management of Font objects. */

#ifndef _GG_Font_h_
#define _GG_Font_h_

#include <GG/AlignmentFlags.h>
#include <GG/Texture.h>
#include <GG/UnicodeCharsets.h>

#include <set>
#include <stack>

#include <boost/unordered_map.hpp>
#include <boost/serialization/access.hpp>

struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;

namespace GG {

/** Returns a string of the form "<rgba r g b a>" from a Clr object with color
    channels r, b, g, a.*/
GG_API std::string RgbaTag(const Clr& c);

/** Text formatting flags. */
GG_FLAG_TYPE(TextFormat);
extern GG_API const TextFormat FORMAT_NONE;        ///< Default format selected.
extern GG_API const TextFormat FORMAT_VCENTER;     ///< Centers text vertically.
extern GG_API const TextFormat FORMAT_TOP;         ///< Top-justifies text.
extern GG_API const TextFormat FORMAT_BOTTOM;      ///< Justifies the text to the bottom of the rectangle.
extern GG_API const TextFormat FORMAT_CENTER;      ///< Centers text horizontally in the rectangle.
extern GG_API const TextFormat FORMAT_LEFT;        ///< Aligns text to the left. 
extern GG_API const TextFormat FORMAT_RIGHT;       ///< Aligns text to the right. 
extern GG_API const TextFormat FORMAT_WORDBREAK;   ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle. (As always, a '\\n' also breaks the line.)
extern GG_API const TextFormat FORMAT_LINEWRAP;    ///< Lines are automatically broken when the next glyph would be drawn outside the the text rectangle.
extern GG_API const TextFormat FORMAT_IGNORETAGS;  ///< Text formatting tags (e.g. <rgba 0 0 0 255>) are treated as regular text.


/** This class creates one or more 16-bpp OpenGL textures that contain
    rendered glyphs from a requested font file at the requested point size,
    including only the requested ranges of code points.  Once the textures have
    been created, text is rendered to the display by rendering quads textured
    with portions of the glyph textures.  The glyphs are rendered to the
    textures in white, with alpha blending used for antialiasing.  The user
    should set the desired text color with a call to glColor*() before any
    call to RenderText().  When text is rendered, DetermineLines() is called
    to determine where the line breaks are, so that text can be rendered
    centered, right-justified, or whatever.  To cut down on this computation,
    when the text is not changing very rapidly (ie not every frame),
    DetermineLines() can be called by the user once, and the result supplied
    to RenderText() repeatedly.  When this is done, the iteration through the
    text to determine line breaks is not necessary at render time.  The user
    is responsible for ensuring that the line data applies to the text string
    supplied to RenderText().  See UnicodeCharsets.h for the ranges of
    code points available, including a function that allow one to determine
    which ranges are necessary for rendering a certain string.  Point sizes
    above 250 are not supported.

    <br><h3>Text Formatting Tags</h3>

    GG::Font supports a few text formatting tags for convenience.  These tags
    are similar to HTML or XML tags; there is an opening version "<tag>" and a
    closing version "</tag>" of each tag.  Tags can be nested.  For instance,
    consider the use of the italics tag \<i> here:

    \verbatim
      <i>some text <i>and </i>some more </i>text \endverbatim

    In this example, everything is italicized except for "text".  Each \<i>
    tag establishes that italics should be used for all further text until the
    next \a matching \</i> tag.  The first \<i> tag matches the second \</i>
    tag, and the two inner tags are matched.  Note that unmatched close-tags
    (e.g. \</i>) are ignored by the text parser Font uses to find tags.  The
    text justification tags are used on a per-line basis, since it makes no
    sense to, for instance, right-justify only a part of a line and center the
    rest. When more than one justification tag appears on a line, the last one
    is used.  A justification close-tag indicates that a line is to be the
    last one with that justification, and only applies if that justification
    is active.

    <br>The supported tags are:
    - \verbatim<i></i> \endverbatim                 Italics
    - \verbatim<u></u> \endverbatim                 Underline
    - \verbatim<rgba r g b a></rgba> \endverbatim   Color. Sets current rendering color to that specified by parameters.  Parameters may be either floating point values between 0.0 and 1.0 inclusive, or 8-bit integer values between 0 and 255 inclusive.  All parameters must be in one format or the other.  The \</rgba> tag does not restore the previously set \<rgba> color, but instead restores the default color used to render the text.  (Example tag: <rgba 0.4 0.5 0.6 0.7>)
    - \verbatim<left></left> \endverbatim           Left-justified text.
    - \verbatim<center></center> \endverbatim       Centered text.
    - \verbatim<right></right> \endverbatim         Right-justified text.
    - \verbatim<pre></pre> \endverbatim             Preformatted.  Similar to HTML \<pre\> tag, except this one only causes all tags to be ignored until a subsequent \</pre\> tag is seen.  Note that due to their semantics, \<pre> tags cannot be nested.

    <p>Users of Font may wish to create their own tags as well.  Though Font
    will know nothing about the semantics of the new tags, it is possible to
    let Font know about them, in order for Font to render them invisible as it
    does with the tags listed above.  See the static methods
    RegisterKnownTag(), RemoveKnownTag(), and ClearKnownTags() for details.
    It is not possible to remove the built-in tags using these methods.  If
    you wish not to use tags at all, call DetermineLines() and RenderText()
    with the format parameter containing FORMAT_IGNORETAGS, or include a
    \<pre\> tag at the beginning of the text to be rendered.
   */
class GG_API Font
{
public:
    /** A range of pointers into a std::string that defines a substring found
        in a string being rendered by Font. */
    class Substring :
        private std::pair<std::string::const_iterator, std::string::const_iterator>
    {
    public:
        typedef std::pair<std::string::const_iterator, std::string::const_iterator> Base;

        /** Default ctor.  Sets .first and .second to be invalid, but
            nonsingular, iterators, with .first == .second. */
        Substring();

        /** Ctor.  \a first_ must be <= \a second_. */
        Substring(std::string::const_iterator first_,
                  std::string::const_iterator second_);

        /** Implicit conversion from base.  \a pair.first must be <= \a
            pair.second. */
        Substring(const Base& pair);

        /** Returns an iterator to the beginning of the substring. */
        std::string::const_iterator begin() const;

        /** Returns an iterator to one-past-the-end of the substring. */
        std::string::const_iterator end() const;

        /** True iff .first == .second. */
        bool empty() const;

        /** Length in original string chars, of the substring. */
        std::size_t size() const;

        /** Implicit conversion to std::string. */
        operator std::string() const;

        /** Comparison with std::string. */
        bool operator==(const std::string& rhs) const;

        /** Comparison with std::string. */
        bool operator!=(const std::string& rhs) const;

        /** Assignment from base.  \a rhs.first must be <= \a rhs.second. */
        Substring& operator=(const Base& rhs);

        /** Concatenation with base.  \a rhs.first must be <= \a rhs.second.
            *this and \a rhs must be contiguous. */
        Substring& operator+=(const Base& rhs);

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** Used to encapsulate a token-like piece of text to be rendered using
        GG::Font. */
    struct GG_API TextElement
    {
        /** The types of token-like entities that can be represented by a
            TextElement. */
        enum TextElementType {
            OPEN_TAG,   ///< An opening text formatting tag (eg "<rgba 0 0 0 255>").
            CLOSE_TAG,  ///< A closing text formatting tag (eg "</rgba>").
            TEXT,       ///< Some non-whitespace text (eg "The").
            WHITESPACE, ///< Some whitespace text (eg "  \n").
            NEWLINE     ///< A newline.  Newline TextElements represent the newline code point when it is encountered in a rendered string, though they do not contain the actual newline character -- their \a text members are always "").
        };

        TextElement(bool ws, bool nl); ///< Ctor.  \a ws indicates that the element contains only whitespace; \a nl indicates that it is a newline element.
        virtual ~TextElement(); ///< Virtual dtor.

        X Width() const;                      ///< Returns the width of the element, in pixels
        virtual TextElementType Type() const; ///< Returns the TextElementType of the element.
        virtual std::size_t OriginalStringChars() const; ///< Returns the number of characters in the original string that the element represents.

        Substring           text;       ///< The text represented by the element, or the name of the tag, if the element is a FormattingTag.
        std::vector<X>      widths;     ///< The widths of the glyphs in \a text.
        const bool          whitespace; ///< True iff this is a whitespace element.
        const bool          newline;    ///< True iff this is a newline element.

    protected:
        TextElement();

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** The type of TextElement that represents a text formatting tag. */
    struct GG_API FormattingTag : TextElement
    {
        FormattingTag(bool close); ///< Ctor.  \a close indicates that the tag is a close-tag (e.g. "</rgba>").

        virtual TextElementType Type() const;
        virtual std::size_t OriginalStringChars() const;

        std::vector<Substring> params;            ///< The parameter strings within the tag, eg "0", "0", "0", and "255" for the tag "<rgba 0 0 0 255>".
        Substring              original_tag_text; ///< The text as it appears in the original string.
        const bool             close_tag;         ///< True iff this is a close-tag.

    private:
        FormattingTag();
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** Holds the essential data on each line that a string occupies when
        rendered with given format flags.  \a char_data contains the visible
        glyphs for each line, plus any text formatting tags present on
        that line as well. */
    struct GG_API LineData
    {
        LineData(); ///< Default ctor.

        /** Contains the extent in pixels, the index into the original string,
            and the text formatting tags that should be applied before
            rendering of a visible glyph. */
        struct GG_API CharData
        {
            /** Defauilt ctor. */
            CharData();

            /** Ctor. */
            CharData(X extent_, std::size_t str_index,
                     const std::vector<boost::shared_ptr<TextElement> >& tags_);

            X           extent;           ///< The furthest-right extent in pixels of this glyph as it appears on the line.
            std::size_t string_index;     ///< The position in the original string of the start of this glyph.
            std::vector<boost::shared_ptr<FormattingTag> >
                        tags;             ///< The text formatting tags that should be applied before rendering this glyph.

        private:
            friend class boost::serialization::access;
            template <class Archive>
            void serialize(Archive& ar, const unsigned int version);
        };

        X           Width() const; ///< returns the width of the line, in pixels
        bool        Empty() const; ///< returns true iff char_data has size 0

        std::vector<CharData> char_data;     ///< Data on each individual glyph.
        Alignment             justification; ///< FORMAT_LEFT, FORMAT_CENTER, or FORMAT_RIGHT; derived from text format flags and/or formatting tags in the text

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    /** Holds the state of tags during rendering of text.  By keeping track of
        this state across multiple calls to RenderText(), the user can
        preserve the functionality of the text formatting tags, if present.*/
    struct GG_API RenderState
    {
        RenderState(); ///< Default ctor.

        /** The count of open \<i> tags seen since the last \</i> seen. */
        std::size_t     use_italics;

        /** The count of open \<u> tags seen since the last \</u> seen. */
        std::size_t     draw_underline;

        /** The stack of text colors (as set by previous tags). */
        std::stack<Clr> colors;
    };

    /** \name Structors */ ///@{
    /** ctor.  Construct a font using only the printable characters in the
        extended ASCII set.  \throw Font::Exception Throws a subclass of
        Exception if the condition specified for the subclass is met. */
    Font(const std::string& font_filename, unsigned int pts);

    /** ctor.  Construct a font using all the code points in the
        UnicodeCharsets in the range [first, last).  \throw Font::Exception
        Throws a subclass of Exception if the condition specified for the
        subclass is met. */
    template <class CharSetIter>
    Font(const std::string& font_filename, unsigned int pts, CharSetIter first, CharSetIter last);

    ~Font(); ///< Dtor.
    //@}

    /** \name Accessors */ ///@{
    const std::string&FontName() const;   ///< returns the name of the file from which this font was created
    unsigned int      PointSize() const;  ///< returns the point size in which the characters in the font object are rendered

    /** returns the range(s) of code points rendered in the font */
    const std::vector<UnicodeCharset>&
                      UnicodeCharsets() const;

    Y                 Ascent() const;     ///< returns the maximum amount above the baseline the text can go, in pixels
    Y                 Descent() const;    ///< returns the maximum amount below the baseline the text can go, in pixels
    Y                 Height() const;     ///< returns (Ascent() - Descent()), in pixels
    Y                 Lineskip() const;   ///< returns the distance that should be placed between lines, in pixels.  This is usually not equal to Height().
    X                 SpaceWidth() const; ///< returns the width in pixels of the glyph for the space character
    X                 RenderGlyph(const Pt& pt, char c) const; ///< renders glyph for \a c and returns advance of glyph rendered
    X                 RenderGlyph(const Pt& pt, boost::uint32_t c) const; ///< renders glyph for \a c and returns advance of glyph rendered
    X                 RenderText(const Pt& pt, const std::string& text) const; ///< unformatted text rendering; repeatedly calls RenderGlyph, then returns advance of entire string
    void              RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>* line_data = 0, RenderState* render_state = 0) const; ///< formatted text rendering
    void              RenderText(const Pt& pt1, const Pt& pt2, const std::string& text, Flags<TextFormat>& format, const std::vector<LineData>& line_data, RenderState& render_state, std::size_t begin_line, std::size_t begin_char, std::size_t end_line, std::size_t end_char) const; ///< formatted text rendering over a subset of lines and code points
    void              ProcessTagsBefore(const std::vector<LineData>& line_data, RenderState& render_state, std::size_t begin_line, std::size_t begin_char) const; ///< Sets \a render_state as if all the text before (<i>begin_line</i>, <i>begin_char</i>) had just been rendered.
    Pt                DetermineLines(const std::string& text, Flags<TextFormat>& format, X box_width, std::vector<LineData>& line_data) const; ///< returns the maximum dimensions of the string in x and y
    Pt                TextExtent(const std::string& text, Flags<TextFormat> format = FORMAT_NONE, X box_width = X0) const; ///< returns the maximum dimensions of the string in x and y.  Provided as a convenience; it just calls DetermineLines with the given parameters.
    Pt                TextExtent(const std::string& text, const std::vector<LineData>& line_data) const; ///< returns the maximum dimensions of the text in x and y.
    //@}

    static void       RegisterKnownTag(const std::string& tag); ///< adds \a tag to the list of embedded tags that Font should not print when rendering text.  Passing "foo" will cause Font to treat "<foo>", \<foo [arg1 [arg2 ...]]>, and "</foo>" as tags.
    static void       RemoveKnownTag(const std::string& tag);   ///< removes \a tag from the known tag list.  Does not remove the built in tags: \<i>, \<u>, \<rgba r g b a>, and \<pre\>.
    static void       ClearKnownTags();                         ///< removes all tags from the known tag list.  Does not remove the built in tags: \<i>, \<u>, \<rgba r g b a>, and \<pre\>.

    /** \name Exceptions */ ///@{
    /** The base class for Font exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when valid font data cannot be read from a file. */
    GG_CONCRETE_EXCEPTION(BadFile, GG::Font, Exception);

    /** Thrown when a nonpositive font size is requested. */
    GG_CONCRETE_EXCEPTION(InvalidPointSize, GG::Font, Exception);

    /** Thrown when a FreeType font could be loaded, but the resulting font is
        not scalable, making it unusable by GG. */
    GG_CONCRETE_EXCEPTION(UnscalableFont, GG::Font, Exception);

    /** Thrown when an attempt is made to create a glyph from null font face
        object. */
    GG_CONCRETE_EXCEPTION(BadFace, GG::Font, Exception);

    /** Thrown when an attempt to set the size of a FreeType font face
        fails. */
    GG_CONCRETE_EXCEPTION(BadPointSize, GG::Font, Exception);

    /** Thrown when FreeType is unable to fulfill a request to load or render
        a glpyh. */
    GG_CONCRETE_EXCEPTION(BadGlyph, GG::Font, Exception);
    //@}

    /** Throws a BadGlyph exception, with \a c converted to a printable ASCII
        character (if possible), or as a Unicode code point.  \a format_str
        should contain the Boost.Format positional notation formatting tag
        "%1%" where the code point should appear. */
    static void ThrowBadGlyph(const std::string& format_str, boost::uint32_t c);

protected:
    /** \name Structors */ ///@{
    Font(); ///< default ctor
    //@}

private:
    /** This just holds the essential data necessary to render a glyph from
        the OpenGL texture(s) created at GG::Font creation time.*/
    struct Glyph
    {
        Glyph(); ///< default ctor
        Glyph(const boost::shared_ptr<Texture>& texture, const Pt& ul, const Pt& lr, X lb, X adv); ///< ctor

        SubTexture  sub_texture;   ///< the subtexture containing just this glyph
        X           left_bearing;  ///< the space that should remain before the glyph
        X           advance;       ///< the amount of space the glyph should occupy, including glyph graphic and inter-glyph spacing
        X           width;         ///< the width of the glyph only
    };

    typedef boost::unordered_map<boost::uint32_t, Glyph> GlyphMap;

    void              Init(const std::string& font_filename, unsigned int pts);
    bool              GenerateGlyph(FT_Face font, boost::uint32_t ch);
    void              ValidateFormat(Flags<TextFormat>& format) const;
    inline X          RenderGlyph(const Pt& pt, const Glyph& glyph, const RenderState* render_state) const;
    void              HandleTag(const boost::shared_ptr<FormattingTag>& tag, double* orig_color, RenderState& render_state) const;

    std::string          m_font_filename;
    unsigned int         m_pt_sz;
    std::vector<UnicodeCharset>
                         m_charsets;    ///< the sets of glyphs that are covered by this font object
    Y                    m_ascent;      ///< maximum amount above the baseline the text can go
    Y                    m_descent;     ///< maximum amount below the baseline the text can go
    Y                    m_height;      ///< ascent - descent
    Y                    m_lineskip;    ///< distance that should be placed between lines
    double               m_underline_offset; ///< amount below the baseline that the underline sits
    double               m_underline_height; ///< height (thickness) of underline
    double               m_italics_offset;   ///< amount that the top of an italicized glyph is left of the bottom
    X                    m_space_width; ///< the width in pixels of the glyph for the space character
    GlyphMap             m_glyphs;      ///< the locations of the images of each glyph within the textures
    std::vector<boost::shared_ptr<Texture> >
                         m_textures;    ///< the OpenGL texture objects in which the glyphs can be found

    static std::set<std::string>   s_action_tags; ///< embedded tags that Font must act upon when rendering are stored here
    static std::set<std::string>   s_known_tags;  ///< embedded tags that Font knows about but should not act upon are stored here

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Stream ooutput operator for Font::Substring. */
std::ostream& operator<<(std::ostream& os, const Font::Substring& substr);


/** This singleton class is essentially a very thin wrapper around a map of
    Font smart pointers, keyed on font filename/point size pairs.  The user
    need only request a font through GetFont(); if the font at the requested
    size needs to be created, the font is created at the requestd size, a
    shared_ptr to it is kept, and a copy of the shared_ptr is returned.  If
    the font has been created at the desired size, but the request includes
    code point range(s) not already created, the font at the requested size is
    created with the union of the reqested and existing ranges, stored, and
    returned as above; the only difference is that the original shared_ptr is
    destroyed.  Due to the shared_ptr semantics, the object pointed to by the
    shared_ptr is deleted if and only if the last shared_ptr that refers to it
    is deleted.  So any requested font can be used as long as the caller
    desires, even when another caller tells the FontManager to free the
    font.*/
class GG_API FontManager
{
private:
    /// This GG::FontManager-private struct is used as a key type for the map
    /// of rendered fonts.
    struct GG_API FontKey
    {
        FontKey(const std::string& str, unsigned int pts); ///< ctor
        bool operator<(const FontKey& rhs) const; ///< lexocograhpical ordering on filename then points

        std::string  filename;   ///< the name of the file from which this font was created
        unsigned int points;     ///< the point size in which this font was rendered
    };

public:
    /** \name Mutators */ ///@{
    /** returns a shared_ptr to the requested font, supporting all printable
        ASCII characters.  \note May load font if unavailable at time of
        request. */
    boost::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts);

    /** returns a shared_ptr to the requested font, supporting all the
        code points in the UnicodeCharsets in the range [first, last).  \note
        May load font if unavailable at time of request. */
    template <class CharSetIter>
    boost::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                    CharSetIter first, CharSetIter last);

    /** removes the indicated font from the font manager.  Due to shared_ptr
        semantics, the font may not be deleted until much later. */
    void                    FreeFont(const std::string& font_filename, unsigned int pts);
    //@}

private:
    FontManager();

    std::map<FontKey, boost::shared_ptr<Font> > m_rendered_fonts;

    static const boost::shared_ptr<Font> EMPTY_FONT;

    friend GG_API FontManager& GetFontManager();
};

/** Returns the singleton FontManager instance. */
GG_API FontManager& GetFontManager();

/** Thrown when initialization of the FreeType library fails. */
GG_EXCEPTION(FailedFTLibraryInit);

} // template GG

// template implementations
template <class Archive>
void GG::Font::Substring::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(first)
        & BOOST_SERIALIZATION_NVP(second);
}

template <class Archive>
void GG::Font::TextElement::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(text)
        & BOOST_SERIALIZATION_NVP(widths)
        & boost::serialization::make_nvp("whitespace", const_cast<bool&>(whitespace))
        & boost::serialization::make_nvp("newline", const_cast<bool&>(newline));
}

template <class Archive>
void GG::Font::FormattingTag::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TextElement)
        & BOOST_SERIALIZATION_NVP(params)
        & BOOST_SERIALIZATION_NVP(original_tag_text)
        & boost::serialization::make_nvp("close_tag", const_cast<bool&>(close_tag));
}

template <class Archive>
void GG::Font::LineData::CharData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(extent)
        & BOOST_SERIALIZATION_NVP(string_index)
        & BOOST_SERIALIZATION_NVP(tags);
}

template <class Archive>
void GG::Font::LineData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(char_data)
        & BOOST_SERIALIZATION_NVP(justification);
}

template <class CharSetIter>
GG::Font::Font(const std::string& font_filename, unsigned int pts, CharSetIter first, CharSetIter last) :
    m_font_filename(font_filename),
    m_pt_sz(pts),
    m_charsets(first, last),
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
        Init(font_filename, pts);
}

template <class Archive>
void GG::Font::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_font_filename)
        & BOOST_SERIALIZATION_NVP(m_pt_sz);

    if (Archive::is_loading::value) {
        if (!m_font_filename.empty() && 0 < m_pt_sz) {
            try {
                Init(m_font_filename, m_pt_sz);
            } catch (const Exception& e) {
                // take no action; the Font must have been uninitialized when saved
            }
        }
    }
}

template <class CharSetIter>
boost::shared_ptr<GG::Font>
GG::FontManager::GetFont(const std::string& font_filename, unsigned int pts,
                         CharSetIter first, CharSetIter last)
{
    FontKey key(font_filename, pts);
    std::map<FontKey, boost::shared_ptr<Font> >::iterator it = m_rendered_fonts.find(key);
    if (it == m_rendered_fonts.end()) { // if no such font has been created, create it now
        if (font_filename == "") {
            return EMPTY_FONT; // keeps this function from throwing; "" is the only invalid font filename that shouldn't throw
        } else {
            m_rendered_fonts[key] = boost::shared_ptr<Font>(new Font(font_filename, pts, first, last));
            return m_rendered_fonts[key];
        }
    // if a font like this has been created, but it doesn't have all the right glyphs, release it and create a new one
    } else {
        std::set<UnicodeCharset> requested_charsets(first, last);
        std::set<UnicodeCharset> found_charsets(it->second->UnicodeCharsets().begin(), it->second->UnicodeCharsets().end());
        if (requested_charsets != found_charsets) {
            std::vector<UnicodeCharset> united_charsets;
            std::set_union(requested_charsets.begin(), requested_charsets.end(),
                           found_charsets.begin(), found_charsets.end(),
                           std::back_inserter(united_charsets));
            m_rendered_fonts.erase(it);
            m_rendered_fonts[key] =
                boost::shared_ptr<Font>(new Font(font_filename, pts, united_charsets.begin(), united_charsets.end()));
            return m_rendered_fonts[key];
        } else { // otherwise, the font we found works, so just return it
            return it->second;
        }
    }
}

#endif // _GG_Font_h_
