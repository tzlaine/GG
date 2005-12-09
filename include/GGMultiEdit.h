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

/** \file GGMultiEdit.h
    Contains the MultiEdit class, a multi-line text box control. */

#ifndef _GGMultiEdit_h_
#define _GGMultiEdit_h_

#ifndef _GGEdit_h_
#include "GGEdit.h"
#endif

namespace GG {

class Scroll;

/** This is a multi-line text input and display control.  MultiEdit is designed to be used as a basic text-input control
    for text longer than one line, or to display large amounts of formatted or unformatted text.  MultiEdit supports
    text formatting tags.  See GG::Font for details.  Several style flags are available.  If the TERMINAL_STYLE flag is
    in use, rows that exceed the history limit will be removed from the beginning of the text; otherwise, they are
    removed from the end.  If either TF_LINEWRAP of TF_WORDBREAK are in use, NO_HSCROLL must be in use as well.
    TF_VCENTER is not an allowed style; if it is specified, TF_TOP will be used in its place.  The justification
    introduced by text formatting tags is very different from that introduced by the TF_* styles.  The former justifies
    lines within the space taken up by the text.  The latter justifies the entire block of text within the client area
    of the control.  So if you specify TF_LEFT and use \<right> formatting tags on the entire text, the text will appear
    to be right-justified, but you will probably only see the extreme left of the text area without scrolling.  If none
    of the no-scroll style flags are in use, the scrolls are created and destroyed automatically, as needed. */
class GG_API MultiEdit : public Edit
{
public:
    /** the styles of display and interaction for a MultiEdit.  The TF_WORDBREAK, TF_LINEWRAP, TF_LEFT, TF_CENTER, and 
        TF_RIGHT GG::TextFormat flags also apply.*/
    enum Styles {
        READ_ONLY =       1 << 10, ///< the control is not user-interactive, only used to display text.  Text can still be programmatically altered and selected.
        TERMINAL_STYLE =  1 << 11, ///< the text in the control is displayed so that the bottom is visible, instead of the top
        INTEGRAL_HEIGHT = 1 << 12, ///< the height of the control will always be a multiple of the height of one row (fractions rounded down)
        NO_VSCROLL =      1 << 13, ///< vertical scrolling is not available, and there is no vertical scroll bar
        NO_HSCROLL =      1 << 14, ///< horizontal scrolling is not available, and there is no horizontal scroll bar
        NO_SCROLL = NO_VSCROLL | NO_HSCROLL ///< scrolls are not used for this control
    };

    /** \name Structors */ //@{
    MultiEdit(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color, 
              Uint32 style = TF_LINEWRAP, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, 
              Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor
    virtual ~MultiEdit(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    virtual Pt ClientLowerRight() const;

    /** returns the maximum number of lines of text that the control keeps. This number includes the lines that are 
        visible in the control.  A value <= 0 indicates that there is no limit.*/
    int MaxLinesOfHistory() const;
    //@}

    /** \name Mutators */ //@{
    virtual void   Render();
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);
    virtual void   LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual void   MouseWheel(const Pt& pt, int move, Uint32 keys);
    virtual void   Keypress(Key key, Uint32 key_mods);

    virtual void   SizeMove(const Pt& ul, const Pt& lr);

    virtual void   SelectAll();
    virtual void   SetText(const std::string& str);

    /** sets the maximum number of rows of text that the control will keep */
    void           SetMaxLinesOfHistory(int max);

    virtual void   DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ //@{
    MultiEdit(); ///< defalt ctor
    //@}

    /** \name Accessors */ //@{
    virtual bool MultiSelected() const;     ///< returns true if >= 1 characters are selected
    int     RightMargin() const;            ///< returns the width of the scrollbar on the right side of the control (0 if none)
    int     BottomMargin() const;           ///< returns the width of the scrollbar at the bottom of the control (0 if none)
    std::pair<int, int>
            CharAt(const Pt& pt) const;     ///< returns row and character index of \a pt, or (0, 0) if \a pt falls outside the text.  \a pt is in client-space coordinates
    int     StringIndexOf(int row, int char_idx) const; ///< returns index into WindowText() of position \a char_idx in row \a row.  Not range-checked.
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

    /** \name Mutators */ //@{
    void            RecreateScrolls();              ///< recreates the vertical and horizontal scrolls as needed.
    //@}

    static const int SCROLL_WIDTH;          ///< the width used to create the control's vertical and horizontal Scrolls

private:
    void    Init();
    void    ValidateStyle();
    void    ClearSelected();   ///< clears (deletes) selected characters, as when a del, backspace, or character is entered
    void    AdjustView();      ///< makes sure the caret ends up in view after an arbitrary move or SetText()
    void    AdjustScrolls();   ///< sets the sizes of the scroll-space and the screen-space of the scrolls
    void    VScrolled(int upper, int lower, int range_upper, int range_lower);
    void    HScrolled(int upper, int lower, int range_upper, int range_lower);

    Uint32  m_style;

    std::pair<int, int> m_cursor_begin; ///< the row and character index of the first character in the hilited selection
    std::pair<int, int> m_cursor_end;   ///< the row and character index + 1 of the last character in the hilited selection
                                        // if m_cursor_begin == m_cursor_end, the caret is draw at m_cursor_end

    Pt          m_contents_sz;          ///< the size of the entire text block in the control (not just the visible part)

    int         m_first_col_shown;      ///< the position (counted from the left side of the text) of the first pixel shown
    int         m_first_row_shown;      ///< the position (counted from the top of the text) of the first pixel shown

    int         m_max_lines_history;

    Scroll*     m_vscroll;
    Scroll*     m_hscroll;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// define EnumMap and stream operators for MultiEdit::Style
GG_ENUM_MAP_BEGIN(MultiEdit::Styles)
    GG_ENUM_MAP_INSERT(MultiEdit::READ_ONLY)
    GG_ENUM_MAP_INSERT(MultiEdit::TERMINAL_STYLE)
    GG_ENUM_MAP_INSERT(MultiEdit::INTEGRAL_HEIGHT)
    GG_ENUM_MAP_INSERT(MultiEdit::NO_VSCROLL)
    GG_ENUM_MAP_INSERT(MultiEdit::NO_HSCROLL)
    GG_ENUM_MAP_INSERT(MultiEdit::NO_SCROLL)
GG_ENUM_MAP_END

GG_ENUM_STREAM_IN(MultiEdit::Styles)
GG_ENUM_STREAM_OUT(MultiEdit::Styles)

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

#endif // _GGMultiEdit_h_
