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

/** \file MultiEdit.h
    Contains the MultiEdit class, a multi-line text box control. */

#ifndef _GG_MultiEdit_h_
#define _GG_MultiEdit_h_

#include <GG/Edit.h>


namespace GG {

class Scroll;

/** The styles of display and interaction for a MultiEdit. */
GG_FLAG_TYPE(MultiEditStyle);
extern GG_API const MultiEditStyle MULTI_NONE;             ///< Default style selected.
extern GG_API const MultiEditStyle MULTI_WORDBREAK;        ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle. (As always, a '\\n' also breaks the line.)
extern GG_API const MultiEditStyle MULTI_LINEWRAP;         ///< Lines are automatically broken when the next character (or space) would be drawn outside the the text rectangle.
extern GG_API const MultiEditStyle MULTI_VCENTER;          ///< Vertically centers text. 
extern GG_API const MultiEditStyle MULTI_TOP;              ///< Aligns text to the top. 
extern GG_API const MultiEditStyle MULTI_BOTTOM;           ///< Aligns text to the bottom. 
extern GG_API const MultiEditStyle MULTI_CENTER;           ///< Centers text. 
extern GG_API const MultiEditStyle MULTI_LEFT;             ///< Aligns text to the left. 
extern GG_API const MultiEditStyle MULTI_RIGHT;            ///< Aligns text to the right. 
extern GG_API const MultiEditStyle MULTI_READ_ONLY;        ///< The control is not user-interactive, only used to display text.  Text can still be programmatically altered and selected.
extern GG_API const MultiEditStyle MULTI_TERMINAL_STYLE;   ///< The text in the control is displayed so that the bottom is visible, instead of the top.
extern GG_API const MultiEditStyle MULTI_INTEGRAL_HEIGHT;  ///< The height of the control will always be a multiple of the height of one row (fractions rounded down).
extern GG_API const MultiEditStyle MULTI_NO_VSCROLL;       ///< Vertical scrolling is not available, and there is no vertical scroll bar.
extern GG_API const MultiEditStyle MULTI_NO_HSCROLL;       ///< Horizontal scrolling is not available, and there is no horizontal scroll bar.
extern GG_API const Flags<MultiEditStyle> MULTI_NO_SCROLL; ///< Scrolls are not used for this control.


/** This is a multi-line text input and display control.  MultiEdit is designed to be used as a basic text-input control
    for text longer than one line, or to display large amounts of formatted or unformatted text.  MultiEdit supports
    text formatting tags.  See GG::Font for details.  Several style flags are available.  If the MULTI_TERMINAL_STYLE
    flag is in use, lines that exceed the history limit will be removed from the beginning of the text; otherwise, they
    are removed from the end.  If either MULTI_LINEWRAP of MULTI_WORDBREAK are in use, MULTI_NO_HSCROLL must be in use
    as well.  MULTI_VCENTER is not an allowed style; if it is specified, MULTI_TOP will be used in its place.  The
    justification introduced by text formatting tags is very different from that introduced by the TF_* styles.  The
    former justifies lines within the space taken up by the text.  The latter justifies the entire block of text within
    the client area of the control.  So if you specify MULTI_LEFT and use \<right> formatting tags on the entire text,
    the text will appear to be right-justified, but you will probably only see the extreme left of the text area without
    scrolling.  If none of the no-scroll style flags are in use, the scrolls are created and destroyed automatically, as
    needed. */
class GG_API MultiEdit : public Edit
{
public:
    /** \name Structors */ ///@{
    MultiEdit(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color, 
              Flags<MultiEditStyle> style = MULTI_LINEWRAP, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, 
              Flags<WndFlag> flags = CLICKABLE); ///< ctor
    virtual ~MultiEdit(); ///< dtor
    //@}

    /** \name Accessors */ ///@{
    virtual Pt MinUsableSize() const;
    virtual Pt ClientLowerRight() const;

    Flags<MultiEditStyle> Style() const; ///< returns the style flags for this MultiEdit to \a style

    /** returns the maximum number of lines of text that the control keeps. This number includes the lines that are 
        visible in the control.  A value <= 0 indicates that there is no limit.*/
    int MaxLinesOfHistory() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void   Render();
    virtual void   LButtonDown(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void   LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);
    virtual void   MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys);
    virtual void   KeyPress(Key key, Flags<ModKey> mod_keys);

    virtual void   SizeMove(const Pt& ul, const Pt& lr);

    virtual void   SelectAll();
    virtual void   SetText(const std::string& str);

    void           SetStyle(Flags<MultiEditStyle> style); ///< sets the style flags for this MultiEdit to \a style

    /** sets the maximum number of rows of text that the control will keep */
    void           SetMaxLinesOfHistory(int max);

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ ///@{
    MultiEdit(); ///< defalt ctor
    //@}

    /** \name Accessors */ ///@{
    virtual bool MultiSelected() const;     ///< returns true if >= 1 characters are selected
    int     RightMargin() const;            ///< returns the width of the scrollbar on the right side of the control (0 if none)
    int     BottomMargin() const;           ///< returns the width of the scrollbar at the bottom of the control (0 if none)
    std::pair<int, int>
            CharAt(const Pt& pt) const;     ///< returns row and character index of \a pt, or (0, 0) if \a pt falls outside the text.  \a pt is in client-space coordinates
    std::pair<int, int>
            CharAt(int string_idx) const;   ///< returns row and character index of char at string index, or (0, 0) if \a string_idx falls outside the text, or if \a string_idx refers to a non-visible character

    Pt      ScrollPosition() const;         ///< returns the positions of the scrollbars, in pixels

    /** returns index into WindowText() of position \a char_idx in row \a row, using \a line_data instead of the current
        line data, if it is supplied.  Not range-checked. */
    int     StringIndexOf(int row, int char_idx, const std::vector<Font::LineData>* line_data = 0) const;

    int     RowStartX(int row) const;       ///< returns the the x-coordinate of the beginning of row \a row, in cleint-space coordinates.  Not range-checked.
    int     CharXOffset(int row, int idx) const; ///< returns the distance in pixels from the start of row \a row to the character at index idx.  Not range-checked.
    int     RowAt(int y) const;             ///< returns the line that falls under y coordinate \a y.  \a y must be in client-space coordinates.
    int     CharAt(int row, int x) const;   ///< returns the index of the character in row \a row that falls under x coordinate \a x.  \a x must be in client-space coordinates.
    int     FirstVisibleRow() const;        ///< returns the index of the first visible row, or 0 if none
    int     LastVisibleRow() const;         ///< returns the index of the last visible row, or 0 if none
    int     FirstFullyVisibleRow() const;   ///< returns the index of the first fully visible row, or 0 if none
    int     LastFullyVisibleRow() const;    ///< returns the index of the last fully visible row, or 0 if none
    int     FirstVisibleChar(int row) const;///< returns the index of the first visible character of row \a row, or 0 if none
    int     LastVisibleChar(int row) const; ///< returns the index of the last visible character of row \a row, or 0 if none
    std::pair<int, int>
            HighCursorPos() const;          ///< returns the greater of m_cursor_begin and m_cursor_end
    std::pair<int, int>
            LowCursorPos() const;           ///< returns the lesser of m_cursor_begin and m_cursor_end
    //@}

    /** \name Mutators */ ///@{
    void    RecreateScrolls();              ///< recreates the vertical and horizontal scrolls as needed.

    /** ensures that the next call to SetText() preserves the positioning of the text.  This should only be called if it
        is known that the call to SetText() will not put the text-position in an illegal state.  For instance, if
        creating a MultiEdit that contains hyperlink text then coloring or underlining a link may require a call to
        SetText(), but may be guaranteed not to change the text layout.  Without a call to this function, the scroll
        positions will be reset. */
    void    PreserveTextPositionOnNextSetText();
    //@}

    static const int SCROLL_WIDTH;          ///< the width used to create the control's vertical and horizontal Scrolls
    static const int BORDER_THICK;          ///< the thickness with which to render the border of the control

private:
    void    Init();
    void    ValidateStyle();
    void    ClearSelected();   ///< clears (deletes) selected characters, as when a del, backspace, or character is entered
    void    AdjustView();      ///< makes sure the caret ends up in view after an arbitrary move or SetText()
    void    AdjustScrolls();   ///< sets the sizes of the scroll-space and the screen-space of the scrolls
    void    VScrolled(int upper, int lower, int range_upper, int range_lower);
    void    HScrolled(int upper, int lower, int range_upper, int range_lower);

    Flags<MultiEditStyle> m_style;

    std::pair<int, int>  m_cursor_begin; ///< the row and character index of the first character in the hilited selection
    std::pair<int, int>  m_cursor_end;   ///< the row and character index + 1 of the last character in the hilited selection
                                         // if m_cursor_begin == m_cursor_end, the caret is draw at m_cursor_end

    Pt          m_contents_sz;          ///< the size of the entire text block in the control (not just the visible part)

    int         m_first_col_shown;      ///< the position (counted from the left side of the text) of the first pixel shown
    int         m_first_row_shown;      ///< the position (counted from the top of the text) of the first pixel shown

    int         m_max_lines_history;

    Scroll*     m_vscroll;
    Scroll*     m_hscroll;

    bool        m_preserve_text_position_on_next_set_text;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::MultiEdit::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Edit)
        & BOOST_SERIALIZATION_NVP(m_style)
        & BOOST_SERIALIZATION_NVP(m_cursor_begin)
        & BOOST_SERIALIZATION_NVP(m_cursor_end)
        & BOOST_SERIALIZATION_NVP(m_contents_sz)
        & BOOST_SERIALIZATION_NVP(m_first_col_shown)
        & BOOST_SERIALIZATION_NVP(m_first_row_shown)
        & BOOST_SERIALIZATION_NVP(m_max_lines_history)
        & BOOST_SERIALIZATION_NVP(m_vscroll)
        & BOOST_SERIALIZATION_NVP(m_hscroll);

    if (Archive::is_loading::value)
        ValidateStyle();
}

#endif // _GG_MultiEdit_h_
