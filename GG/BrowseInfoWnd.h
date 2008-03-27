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

/** \file BrowseInfoWnd.h
    Contains the BrowseInfoWnd class, and its subclass TextBoxBrowseInfoWnd.  These classes display info on another
    window. */

#ifndef _GG_BrowseInfoWnd_h_
#define _GG_BrowseInfoWnd_h_

#include <GG/Wnd.h>
#include <GG/Font.h>

#include <boost/serialization/is_abstract.hpp>
#include <boost/serialization/version.hpp>


namespace GG {

class Font;
class TextControl;

/** The abstract base class for all browse-info display windows.  Each frame that a BrowseInfoWnd is displayed, its
    Update() method is called.  The Wnd* parameter passed in this call is the window about which the BrowseInfoWnd is
    displaying info (the target Wnd); the BrowseInfoWnd can collect whatever information it requires from the target Wnd
    before it is rendered.  Note that a BrowseInfoWnd should never be CLICKABLE. */
class GG_API BrowseInfoWnd : public Wnd
{
public:
    /** \name Accessors */ ///@{
    /** Returns true iff \a wnd's browse mode \a mode contains browse info that is usable by this BrowseInfoWnd.  This
        method is used by GUI to determine whether a Wnd w has suitable browse info available; if not, w's ancestors may
        instead be asked for browse info recursively. */
    virtual bool WndHasBrowseInfo(const Wnd* wnd, int mode) const = 0;
    //@}

    /** \name Mutators */ ///@{
    virtual void Render() = 0;

    /** Collects data from \a target that is needed by Render().  Note that the one datum that is always available for
        any Wnd is the text to display for \a mode, accessible through Wnd::BrowseInfoText() (though this may be the
        empty string).  Other data that are provided by a Wnd subclass can be recovered by casting \a target to its
        actual subclass type. */
    void Update(int mode, const Wnd* target);

    /** Sets the current cursor position to the one given. */
    void SetCursorPosition(const Pt& cursor_pos);
    //@}

protected:
    /** \name Structors */ ///@{
    BrowseInfoWnd(); ///< default ctor
    BrowseInfoWnd(int x, int y, int w, int h); ///< basic ctor
    //@}

private:
    Pt m_cursor_pos;

    virtual void UpdateImpl(int mode, const Wnd* target);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** A subclass of BrowseInfoWnd that displays text in a box, optionally with a border.  The typical use case is for
    TextBoxBrowseInfoWnd to get the appropriate text for the current mode by calling BrowseInfoText() on its target Wnd.
    It may also be used to display static text, by setting SetTextFromTarget(false) and setting the desired text with
    SetText(). */
class GG_API TextBoxBrowseInfoWnd : public BrowseInfoWnd
{
public:
    /** \name Structors */ ///@{
    /** basic ctor */
    TextBoxBrowseInfoWnd(int w, const boost::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color,
                         Flags<TextFormat> format = FORMAT_LEFT | FORMAT_WORDBREAK, int border_width = 2, int text_margin = 4);
    //@}

    /** \name Accessors */ ///@{
    virtual bool                   WndHasBrowseInfo(const Wnd* wnd, int mode) const;

    bool                           TextFromTarget() const; ///< returns true iff the text to display will be read from the target wnd
    const std::string&             Text () const;          ///< returns the text currently set for display
    const boost::shared_ptr<Font>& GetFont() const;        ///< returns the Font used to display text
    Clr                            Color() const;          ///< returns the color used to render the text box
    Clr                            BorderColor() const;    ///< returns the color used to render the text box border
    Clr                            TextColor() const;      ///< returns the color used to render the text
    Flags<TextFormat>              GetTextFormat() const;  ///< returns the text format used to render the text
    int                            BorderWidth() const;    ///< returns the width of the text box border
    int                            TextMargin() const;     ///< returns the margin to leave between the text and the text box
    //@}

    /** \name Mutators */ ///@{
    void         SetText (const std::string& str);
    virtual void Render();

    void SetTextFromTarget(bool b);                    ///< sets the text display mode to static (\a b == true) or dynamic (read from the target Wnd, \a b == false)
    void SetFont(const boost::shared_ptr<Font>& font); ///< sets the Font used to display text
    void SetColor(Clr color);                          ///< sets the color used to render the text box
    void SetBorderColor(Clr border_color);             ///< sets the color used to render the text box border
    void SetTextColor(Clr text_color);                 ///< sets the color used to render the text
    void SetTextFormat(Flags<TextFormat> format);      ///< sets the text format used to render the text
    void SetBorderWidth(int border_width);             ///< sets the width of the text box border
    void SetTextMargin(int text_margin);               ///< sets the margin to leave between the text and the text box
    //@}

protected:
    /** \name Structors */ ///@{
    TextBoxBrowseInfoWnd(); ///< default ctor
    //@}

private:
    virtual void UpdateImpl(int mode, const Wnd* target);

    bool                    m_text_from_target;
    boost::shared_ptr<Font> m_font;
    Clr                     m_color;
    Clr                     m_border_color;
    int                     m_border_width;
    int                     m_preferred_width;
    TextControl*            m_text_control;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

BOOST_IS_ABSTRACT(GG::BrowseInfoWnd);
BOOST_CLASS_VERSION(GG::BrowseInfoWnd, 1);
BOOST_CLASS_VERSION(GG::TextBoxBrowseInfoWnd, 1);

// template implementations
template <class Archive>
void GG::BrowseInfoWnd::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd);
    if (1 <= version)
        ar & BOOST_SERIALIZATION_NVP(m_cursor_pos);
}

template <class Archive>
void GG::TextBoxBrowseInfoWnd::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BrowseInfoWnd)
        & BOOST_SERIALIZATION_NVP(m_text_from_target)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_border_color)
        & BOOST_SERIALIZATION_NVP(m_border_width)
        & BOOST_SERIALIZATION_NVP(m_text_control);
    if (1 <= version)
        ar & BOOST_SERIALIZATION_NVP(m_preferred_width);
}

#endif // _GG_BrowseInfoWnd_h_
