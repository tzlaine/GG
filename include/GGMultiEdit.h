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

#ifndef _GGMultiEdit_h_
#define _GGMultiEdit_h_

#ifndef _GGEdit_h_
#include "GGEdit.h"
#endif

namespace GG {

class Scroll;

/** This is a multi-line text input control.
    If the TERMINAL_STYLE flag is in use, rows that exceed the history limit will be removed from the beginning of the 
    text; otherwise, they are removed from the end.
    If none of the no-scroll style flags are in use, the scrolls are created and destroyed automatically, as needed.
    If either TF_LINEWRAP of TF_WORDBREAK are in use, NO_HSCROLL must be in use as well.
    MultiEdit supports text formatting tags.  See GG::Font for details.
    TF_VCENTER is not an allowed style; if it is specified, TF_TOP will be used in its place. */
class MultiEdit : public Edit
{
public:
    using Wnd::SizeMove;

    /** the styles of display and interaction for a MultiEdit.  The TF_WORDBREAK and TF_LINEWRAP GG::TextFormat flags also 
        apply.*/
    enum Styles {READ_ONLY =       1 << 10, ///< the control is not user-interactive, only used to display text.  Text can still be programmatically altered and selected.
                 TERMINAL_STYLE =  1 << 11, ///< the text in the control is displayed so that the bottom is visible, instead of the top
                 INTEGRAL_HEIGHT = 1 << 12, ///< the height of the control will always be a multiple of the height of one row (fractions rounded down)
                 NO_VSCROLL =      1 << 13, ///< vertical scrolling is not available, and there is no vertical scroll bar
                 NO_HSCROLL =      1 << 14, ///< horizontal scrolling is not available, and there is no horizontal scroll bar
                 NO_SCROLL = NO_VSCROLL | NO_HSCROLL ///< scrolls are not used for this control
                };

    /** \name Structors */ //@{
    MultiEdit(int x, int y, int w, int h, const string& str, const shared_ptr<Font>& font, Clr color, 
              Uint32 style = TF_LINEWRAP, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, 
              Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor
    MultiEdit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, 
              Uint32 style = TF_LINEWRAP, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, 
              Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor
    MultiEdit(const XMLElement& elem); ///< ctor that constructs an MultiEdit object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a MultiEdit object
    virtual ~MultiEdit(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    virtual Pt ClientLowerRight() const;

    /** returns the maximum number of lines of text that the control keeps. This number includes the lines that are 
        visible in the control.  A value <= 0 indicates that there is no limit.*/
    int MaxLinesOfHistory() const {return m_max_lines_history;}

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an MultiEdit object
    //@}

    /** \name Mutators */ //@{
    virtual int    Render();
    virtual int    LButtonDown(const Pt& pt, Uint32 keys);
    virtual int    LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual int    Keypress(Key key, Uint32 key_mods);

    virtual void   SizeMove(int x1, int y1, int x2, int y2);

    virtual void   SelectAll();
    virtual void   SetText(const string& str);

    /** sets the maximum number of rows of text that the control will keep around */
    void           SetMaxLinesOfHistory(int max) {m_max_lines_history = max; SetText(m_text);}
    //@}

protected:
    static const int SCROLL_WIDTH;

private:
    void    Init();
    void    ValidateStyle();
    bool    MultiSelected() const;      ///< returns true if >= 1 characters are selected
    void    ClearSelected();            ///< clears (deletes) selected characters, as when a del, backspace, or character is entered
    void    AdjustView();               ///< makes sure the caret ends up in view after an arbitrary move
    void    AdjustScrolls();            ///< sets the sizes of the scroll-space and the sizes of the screen-space of the scrolls
    int     RightMargin() const;
    int     BottomMargin() const;
    pair<int, int> CharAt(const Pt& pt) const; ///< returns row and character index of \a posn; returns (-1, -1) if \a pt does not fall on the text
    int     StringIndexOf(int row, int char_idx) const; ///< returns row and character index of position \a char_idx in row \a row
    int     ScreenXPosOfChar(int row, int idx) const; ///< returns the screen x-coordinate of the left side of the character at index idx in m_text
    int     RowAt(int y) const;             ///< returns the line that falls under y coordinate \a y (in client coordinates)
    int     CharAt(int row, int x) const;   ///< returns the index of the character in row \a row that falls under x coordinate \a x (in client coordinates)
    int     FirstVisibleRow() const;        ///< returns the index of the first visible row (may return an out-of-bounds result)
    int     LastVisibleRow() const;         ///< returns the index of the last visible row (may return an out-of-bounds result)
    int     FirstFullyVisibleRow() const;   ///< returns the index of the first fully visible row (may return an out-of-bounds result)
    int     LastFullyVisibleRow() const;    ///< returns the index of the last fully visible row (may return an out-of-bounds result)
    int     FirstVisibleChar(int row) const;///< returns the index of the first visible character of row \a row (may return an out-of-bounds result)
    int     LastVisibleChar(int row) const; ///< returns the index of the last visible character of row \a row (may return an out-of-bounds result)
    pair<int, int> HighCursorPos() const;
    pair<int, int> LowCursorPos() const;
    void    VScrolled(int upper, int lower, int range_upper, int range_lower);
    void    HScrolled(int upper, int lower, int range_upper, int range_lower);

    Uint32  m_style;

    pair<int, int> m_cursor_begin;      ///< the row and character index of the first character in the hilited selection
    pair<int, int> m_cursor_end;        ///< the row and character index + 1 of the last character in the hilited selection
                                        // if m_cursor_begin == m_cursor_end, the caret is draw at m_cursor_end

    int         m_first_col_shown;      ///< the position (counted from the left side of the text) of the first pixel shown
    int         m_first_row_shown;      ///< the position (counted from the top of the text) of the first pixel shown

    int         m_max_lines_history;

    Scroll*     m_vscroll;
    Scroll*     m_hscroll;
};

} // namespace GG

#endif // _GGMultiEdit_h_

