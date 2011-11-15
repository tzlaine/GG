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

/** \file TextControl.h \brief Contains the TextControl class, a control which
    represents a certain text string in a certain font, justification, etc. */

#ifndef _GG_TextControl_h_
#define _GG_TextControl_h_

#include <GG/ClrConstants.h>
#include <GG/Control.h>
#include <GG/Font.h>

#include <boost/lexical_cast.hpp>


namespace GG {

/** \brief Displays a piece of text.

    TextControl's know how to center, left- or right-justify, etc. themselves
    within their window areas.  The format flags used with TextControl are
    defined in the TextFormat flag type.

    <br>TextControl is based on pre-rendered font glyphs.  The text is
    rendered character by character from a prerendered font. The font used is
    gotten from the GUI's font manager.  Since a shared_ptr to the font is
    kept, the font is guaranteed to exist at least as long as the TextControl
    object that refers to it.  This also means that if the font is explicitly
    released from the font manager but is still held by at least one
    TextControl object, it will not be destroyed, due to the shared_ptr.  Note
    that if "" is supplied as the font_filename parameter, no text will be
    rendered, but a valid TextControl object will be constructed, which may
    later contain renderable text. TextControl objects support text with
    formatting tags. See GG::Font for details.*/
class GG_API TextControl : public Control
{
public:
    using Wnd::SetMinSize;

    /** \name Structors */ ///@{
    /** Ctor. */
    TextControl(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                Flags<WndFlag> flags = Flags<WndFlag>());

    /** Ctor that does not require window size.  Window size is determined
        from the string and font; the window will be large enough to fit the
        text as rendered, and no larger.  The private member FitToText() will
        also return true. \see TextControl::SetText() */
    TextControl(X x, Y y, const std::string& str, const boost::shared_ptr<Font>& font,
                Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                Flags<WndFlag> flags = Flags<WndFlag>());
    //@}

    /** \name Accessors */ ///@{
    virtual Pt        MinUsableSize(X width = X0) const;

    /** Returns the text displayed in this control. */
    const std::string& Text() const;

    /** Returns the text format (vertical and horizontal justification, use of
        word breaks and line wrapping, etc.) */
    Flags<TextFormat> GetTextFormat() const;

    /** Returns the text color (this may differ from the Control::Color()). */
    Clr               TextColor() const;

    /** Returns true iff the text control clips its text to its client area;
        by default this is not done. */
    bool              ClipText() const;

    /** Returns true iff the text control sets its MinSize() when the bounds
        of its text change because of a call to SetText() or SetTextFormat();
        by default this is not done.  The minimum size of the control in each
        dimension will be the larger of the text size and the current
        MinSize(), if any has been set.  Note that this operates independently
        of fit-to-text behavior, which sets the window size, not its minimum
        size. */
    bool              SetMinSize() const;

    /** Returns the value of the control's text, interpreted as an object of
        type T.  If the control's text can be interpreted as an object of type
        T by boost::lexical_cast (and thus by a stringstream), then GetValue()
        will do so.  Because lexical_cast attempts to convert the entire
        contents of the string to a single value, a TextControl containing the
        string "4.15 3.8" will throw, even though there is a perfectly valid
        4.15 value that occurs first in the string.  \throw
        boost::bad_lexical_cast boost::lexical_cast throws
        boost::bad_lexical_cast when it cannot perform a requested cast. */
    template <class T> T GetValue() const;

    bool   Empty() const;   ///< Returns true iff text string equals "".
    CPSize Length() const;  ///< Returns the number of code points in the text.

    /** Returns the upper-left corner of the text as it is would be rendered
        if it were not bound to the dimensions of this control. */
    Pt TextUpperLeft() const;

    /** Returns the lower-right corner of the text as it is would be rendered
        if it were not bound to the dimensions of this control. */
    Pt TextLowerRight() const;

    /** Returns the line data for the text in this TextControl. */
    const std::vector<Font::LineData>& GetLineData() const;

    /** Returns the Font used by this TextControl to render its text. */
    const boost::shared_ptr<Font>& GetFont() const;

    /** Returns true iff this TextControl was constructed using the ctor
        without width and height parameters.  \see TextControl::SetText() */
    bool FitToText() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();

    /** Sets the text displayed in this control to \a str.  May resize the
        window.  If FitToText() returns true (i.e. if the second ctor was
        used), calls to this function cause the window to be resized to
        whatever space the newly rendered text occupies. */
    void SetText(const std::string& str);

    virtual void SizeMove(const Pt& ul, const Pt& lr);

    /** Sets the text format; ensures that the flags are sane. */
    void         SetTextFormat(Flags<TextFormat> format);

    /** Sets the text color. */
    void         SetTextColor(Clr color);

    /** Just like Control::SetColor(), except that this one also adjusts the
        text color. */
    virtual void SetColor(Clr c);

    /** Enables/disables text clipping to the client area. */
    void         ClipText(bool b);

    /** Enables/disables setting the minimum size of the window to be the text
        size. */
    void         SetMinSize(bool b);

    /** Sets the value of the control's text, interpreted as an object of type
        T.  If the control's text can be interpreted as an object of type T by
        boost::lexical_cast (and thus by a stringstream), then SetValue() will
        do so.  \throw boost::bad_lexical_cast boost::lexical_cast throws
        boost::bad_lexical_cast when it cannot perform a requested cast. */
    template <class T> void SetValue(const T& t);

    /** Sets text to the empty string. */
    void         Clear();

    /** Inserts \a c at position \a pos within the text.  \note Just as with
        most string parameters throughout GG, \a c must be a valid UTF-8
        sequence. */
    void         Insert(CPSize pos, char c);

    /** Inserts \a s at position \a pos within the text. */
    void         Insert(CPSize pos, const std::string& s);
    
    /** Erases \a num code points from the text starting at position \a
        pos. */
    void         Erase(CPSize pos, CPSize num = CP1);

    /** Inserts \a c at text position \a pos within line \a line.  \note Just
        as with most string parameters throughout GG, \a c must be a valid
        UTF-8 sequence. */
    void         Insert(std::size_t line, CPSize pos, char c);

    /** Inserts \a s at text position \a pos within line \a line. */
    void         Insert(std::size_t line, CPSize pos, const std::string& s);

    /** Erases \a num code points from the text starting at position \a
        pos within line \a line. */
    void         Erase(std::size_t line, CPSize pos, CPSize num = CP1);

    using Control::KeyPress;
    //@}

protected:
    /** \name Structors */ ///@{
    TextControl(); ///< Default ctor.
    //@}

private:
    void ValidateFormat();      ///< ensures that the format flags are consistent
    void AdjustMinimumSize();
    void RecomputeTextBounds(); ///< recalculates m_text_ul and m_text_lr

    std::string                 m_text;
    Flags<TextFormat>           m_format;      ///< the formatting used to display the text (vertical and horizontal alignment, etc.)
    Clr                         m_text_color;  ///< the color of the text itself (may differ from GG::Control::m_color)
    bool                        m_clip_text;
    bool                        m_set_min_size;
    std::vector<boost::shared_ptr<Font::TextElement> >
                                m_text_elements;
    std::vector<Font::LineData> m_line_data;
    CPSize                      m_code_points;
    boost::shared_ptr<Font>     m_font;
    bool                        m_fit_to_text; ///< when true, this window will maintain a minimum width and height that encloses the text
    Pt                          m_text_ul;     ///< stored relative to the control's UpperLeft()
    Pt                          m_text_lr;     ///< stored relative to the control's UpperLeft()
    mutable X                   m_last_min_width;
    mutable X                   m_last_min_usable_width;
    mutable Pt                  m_min_usable_size_for_min_width;
    mutable Pt                  m_min_usable_size_for_min_usable_width;
    mutable Pt                  m_min_usable_size;
};

} // namespace GG

// template implementations
template <class T>
T GG::TextControl::GetValue() const
{ return boost::lexical_cast<T, std::string>(m_text); }

template <class T>
void GG::TextControl::SetValue(const T& t)
{ SetText(boost::lexical_cast<std::string>(t)); }

#endif
