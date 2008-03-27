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

/** \file TextControl.h
    Contains the TextControl class, a control which represents a certain text string in a certain font, justification, etc. */

#ifndef _GG_TextControl_h_
#define _GG_TextControl_h_

#include <GG/Control.h>
#include <GG/Font.h>

#include <boost/lexical_cast.hpp>


namespace GG {

/** The name says it all.  All TextControl objects know how to center, left- or right-justify, etc. themselves within
    their window areas.  The format flags used with TextControl are defined in enum GG::TextFormat in
    GGBase.h. TextControl has std::string-like operators and functions that allow the m_text member string to be
    manipulated directly.  In addition, the << and >> operators allow virtually any type (int, float, char, etc.) to be
    read from a Text object as if it were an input or output stream, thanks to boost::lexical_cast.  Note that the Text
    stream operators only read the first instance of the specified type from m_text, and overwrite the entire m_text
    string when writing to it; both operators may throw.  This is a text control based on pre-rendered font glyphs.  The
    text is rendered character by character from a prerendered font. The font used is gotten from the GUI's font
    manager.  Since a shared_ptr to the font is kept, the font is guaranteed to live at least as long as the TextControl
    object that refers to it.  This also means that if the font is explicitly released from the font manager but is
    still held by at least one TextControl object, it will not be destroyed, due to the shared_ptr.  Note that if "" is
    supplied as the font_filename parameter, no text will be rendered, but a valid TextControl object will be
    constructed, which may later contain renderable text. TextControl objects support text with formatting tags. See
    GG::Font for details.*/
class GG_API TextControl : public Control
{
public:
    using Wnd::SetMinSize;

    /** \name Structors */ ///@{
    TextControl(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE, Flags<WndFlag> flags = Flags<WndFlag>()); ///< ctor taking a font directly

    /** ctor that does not require window size.
        Window size is determined from the string and font; the window will be large enough to fit the text as rendered, 
        and no larger.  The private member m_fit_to_text is also set to true. \see TextControl::SetText() */
    TextControl(int x, int y, const std::string& str, const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE, Flags<WndFlag> flags = Flags<WndFlag>());
    //@}

    /** \name Accessors */ ///@{
    virtual Pt        MinUsableSize() const;

    /** returns the text format (vertical and horizontal justification, use of word breaks and line wrapping, etc.) */
    Flags<TextFormat> GetTextFormat() const;

    /** returns the text color (this may differ from the Control::Color() in some subclasses) */
    Clr               TextColor() const;

    /** returns true iff the text control clips its text to its client area; by default this is not done */
    bool              ClipText() const;

    /** returns true iff the text control sets its MinSize() when the bounds of its text change because of a call to
        SetText() or SetTextFormat(); by default this is not done.  The minimum size of the control in each dimension
        will be the larger of the text size and the current MinSize(), if any has been set.  Note that this operates
        independently of fit-to-text behavior, which sets the window size, not its minimum size. */
    bool              SetMinSize() const;

    /** sets the value of \a t to the interpreted value of the control's text.
        If the control's text can be interpreted as an object of type T by boost::lexical_cast (and thus by a stringstream), 
        then the >> operator will do so.  Note that the return type is void, so multiple >> operations cannot be strung 
        together.  Also, because lexical_cast attempts to convert the entire contents of the string to a single value, a 
        TextControl containing the string "4.15 3.8" will fill a float with 0.0 (the default construction of float), 
        even though there is a perfectly valid 4.15 value that occurs first in the string.  \note boost::lexical_cast 
        usually throws boost::bad_lexical_cast when it cannot perform a requested cast, though >> will return a 
        default-constructed T if one cannot be deduced from the control's text. */
    template <class T> void operator>>(T& t) const;

    /** returns the value of the control's text, interpreted as an object of type T.
        If the control's text can be interpreted as an object of type T by boost::lexical_cast (and thus by a stringstream), 
        then GetValue() will do so.  Because lexical_cast attempts to convert the entire contents of the string to a 
        single value, a TextControl containing the string "4.15 3.8" will throw, even though there is a perfectly 
        valid 4.15 value that occurs first in the string.  \throw boost::bad_lexical_cast boost::lexical_cast throws 
        boost::bad_lexical_cast when it cannot perform a requested cast. This is handy for validating data in a dialog box;
        Otherwise, using operator>>(), you may get the default value, even though the text in the control may not be the 
        default value at all, but garbage. */
    template <class T> T GetValue() const;

    operator const std::string&() const; ///< returns the control's text; allows TextControl's to be used as std::string's

    bool  Empty() const;   ///< returns true when text string equals ""
    int   Length() const;  ///< returns length of text string

    /** returns the upper-left corner of the text as it is would be rendered if it were not bound to the dimensions of
        this control. */
    Pt    TextUpperLeft() const;

    /** returns the lower-right corner of the text as it is would be rendered if it were not bound to the dimensions of
        this control. */
    Pt    TextLowerRight() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();

    /** sets the text to \a str; may resize the window.  If the private member m_fit_to_text is true (i.e. if the second 
        ctor type was used), calls to this function cause the window to be resized to whatever space the newly rendered 
        text occupies. */
    virtual void   SetText(const std::string& str);
    virtual void   SetText(const char* str);
    virtual void   SizeMove(const Pt& ul, const Pt& lr);
    void           SetTextFormat(Flags<TextFormat> format); ///< sets the text format; ensures that the flags are sane
    void           SetTextColor(Clr color);      ///< sets the text color
    virtual void   SetColor(Clr c);              ///< just like Control::SetColor(), except that this one also adjusts the text color
    void           ClipText(bool b);             ///< enables/disables text clipping to the client area
    void           SetMinSize(bool b);           ///< enables/disables setting the minimum size of the window to be the text size

    /** Sets the value of the control's text to the stringified version of t.
        If t can be converted to a string representation by a boost::lexical_cast (and thus by a stringstream), then the << operator
        will do so, eg double(4.15) to string("4.15").  Note that the return type is void, so multiple << operations cannot be 
        strung together.  \throw boost::bad_lexical_cast boost::lexical_cast throws boost::bad_lexical_cast when it is confused.*/
    template <class T> void operator<<(T t);

    void  operator+=(const std::string& str); ///< appends \a str to text string by way of SetText()
    void  operator+=(const char* str);        ///< appends \a str to text string by way of SetText()
    void  operator+=(char ch);                ///< appends \a ch to text string by way of SetText()
    void  Clear();                            ///< sets text string to ""
    void  Insert(int pos, char ch);           ///< allows access to text string much as a std::string
    void  Erase(int pos, int num = 1);        ///< allows access to text string much as a std::string

    virtual void DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ ///@{
    TextControl(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    const std::vector<Font::LineData>&  GetLineData() const;
    const boost::shared_ptr<Font>&      GetFont() const;
    bool                                FitToText() const;
    bool                                DirtyLoad() const;
    //@}

private:
    void ValidateFormat();      ///< ensures that the format flags are consistent
    void AdjustMinimumSize();
    void RecomputeTextBounds(); ///< recalculates m_text_ul and m_text_lr

    Flags<TextFormat>           m_format;      ///< the formatting used to display the text (vertical and horizontal alignment, etc.)
    Clr                         m_text_color;  ///< the color of the text itself (may differ from GG::Control::m_color)
    bool                        m_clip_text;
    bool                        m_set_min_size;
    std::vector<Font::LineData> m_line_data;
    boost::shared_ptr<Font>     m_font;
    bool                        m_fit_to_text; ///< when true, this window will maintain a minimum width and height that encloses the text
    Pt                          m_text_ul;     ///< stored relative to the control's UpperLeft()
    Pt                          m_text_lr;     ///< stored relative to the control's UpperLeft()

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
    // true iff the object has just been loaded from a serialized form, meaning changes may have been made that leave
    // the control in an inconsistent state; this must be remedied the next time Render() is called
    bool m_dirty_load;
};

} // namespace GG

// template implementations
template <class T>
void GG::TextControl::operator>>(T& t) const
{
    try {
        t = boost::lexical_cast<T>(Control::m_text);
    } catch (boost::bad_lexical_cast) {
        t = T();
    }
}

template <class T>
T GG::TextControl::GetValue() const
{
    return boost::lexical_cast<T, std::string>(Control::m_text);
}

template <class T>
void GG::TextControl::operator<<(T t)
{
    SetText(boost::lexical_cast<std::string>(t));
}

template <class Archive>
void GG::TextControl::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_format)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_clip_text)
        & BOOST_SERIALIZATION_NVP(m_set_min_size)
        & BOOST_SERIALIZATION_NVP(m_line_data)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_fit_to_text)
        & BOOST_SERIALIZATION_NVP(m_text_ul)
        & BOOST_SERIALIZATION_NVP(m_text_lr);

    if (Archive::is_loading::value) {
        ValidateFormat();
        m_dirty_load = true;
    }
}

#endif // _GG_TextControl_h_
