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

#ifndef _GGEdit_h_
#define _GGEdit_h_

#ifndef _GGTextControl_h_
#include "GGTextControl.h"
#endif

namespace GG {

/** This is a single-line text input control.
   This is a simple edit box control.  It inherits from TextControl, so it 
   can be treated in many ways very similarly to a std::string.  
   Note that the second set of constructors determine the height of 
   the control based on the height of the font used and the value of the constant PIXEL_MARGIN.  There are two
   types of signals emitted by an Edit control.  The first is EditedSignal(); this is emitted every time the
   contents of the Edit change.  Sometimes, however, you don't want that.  For instance, say you want to keep
   the value of the text in the Edit to between (numerical values) 100 and 300.  If the Edit currently reads "200", 
   the user may decide to highlight the "2", hit delete, then type a "1".  If updates are immediate, you will
   receive notification that the Edit says "00" (an invalid value), when that is just a temporary value you 
   don't care about.  In such situations the other signal, FocusUpdateSignal(), should be useful.  It is only 
   emitted when the Edit is losing the input focus and the contents have changed since it gained the input 
   focus.  So you would only receive a single update, namely "100", which is a valid number for that control, 
   and you would receive it only when it is certain that the user is finished editing the text (when the focus
   changes).  Note that both signals may be used at the same time, if desired.*/
class GG_API Edit : public TextControl
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (const string&)> EditedSignalType; ///< emitted whenever the contents of the Edit are altered (keypresses, deletes, etc.); provides the new contents of the Edit
    typedef boost::signal<void (const string&)> FocusUpdateSignalType; ///< emitted whenever the Edit loses the input focus, and its contents have changed since it gained the focus; provides the new contents of the Edit
    //@}

    /** \name Slot Types */ //@{
    typedef EditedSignalType::slot_type       EditedSlotType;      ///< type of functor(s) invoked on a EditedSignalType
    typedef FocusUpdateSignalType::slot_type  FocusUpdateSlotType; ///< type of functor(s) invoked on a FocusUpdateSignalType
    //@}

    /** \name Structors */ //@{
    Edit(int x, int y, int w, int h, const string& str, const shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor
    Edit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor
    Edit(int x, int y, int w, const string& str, const shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor that does not required height. Height is determined from the font and point size used.
    Edit(int x, int y, int w, const string& str, const string& font_filename, int pts, Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER); ///< ctor that does not required height. Height is determined from the font and point size used.
    Edit(const XMLElement& elem); ///< ctor that constructs an Edit object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Edit object
    //@}

    /** \name Accessors */ //@{
    virtual Pt     ClientUpperLeft() const;
    virtual Pt     ClientLowerRight() const;

    const pair<int, int>& CursorPosn() const        {return m_cursor_pos;}     ///< returns the current position of the cursor (first selected character to the last + 1 selected one)
    Clr                   InteriorColor() const     {return m_int_color;}      ///< returns the interior color of the control
    Clr                   HiliteColor() const       {return m_hilite_color;}   ///< returns the color used to render hiliting around selected text
    Clr                   SelectedTextColor() const {return m_sel_text_color;} ///< returns the color used to render selected text

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an Edit object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object

    EditedSignalType&       EditedSignal() const      {return m_edited_sig;}        ///< returns the edited signal object for this Edit
    FocusUpdateSignalType&  FocusUpdateSignal() const {return m_focus_update_sig;}  ///< returns the focus update signal object for this Edit
    //@}
   
    /** \name Mutators */ //@{
    virtual bool   Render();
    virtual void   LButtonDown(const Pt& pt, Uint32 keys);
    virtual void   LDrag(const Pt& pt, const Pt& move, Uint32 keys);
    virtual void   Keypress(Key key, Uint32 key_mods);
    virtual void   GainingFocus();
    virtual void   LosingFocus();

    virtual void   SetColor(Clr c)                {Control::SetColor(c);}
    void           SetInteriorColor(Clr c)        {m_int_color = c;}         ///< sets the interior color of the control
    void           SetHiliteColor(Clr c)          {m_hilite_color = c;}      ///< sets the color used to render hiliting around selected text
    void           SetSelectedTextColor(Clr c)    {m_sel_text_color = c;}    ///< sets the color used to render selected text

    /** selects all text in the given range.  When \a from == \a to, this function just places the caret at \a from.  Note that it is legal to
        pass values such that \a to < \a from.  The difference is that \a from < \a to simulates a drag-selection from left to right, and \a to < 
        \a from simulates one from right to left.  The direction of the simulated drag affects which part of the text is visible at the end of 
        the function call. */
    virtual void   SelectRange(int from, int to);

    /** selects all text in the entire control.  This function leaves the beginning of the text in view; see SelectRange(). */
    virtual void   SelectAll();

    virtual void   SetText(const string& str);
    //@}
   
protected:
    /** \name Accessors */ //@{
    virtual bool            MultiSelected() const   {return m_cursor_pos.first != m_cursor_pos.second;} ///< returns true if >= 1 characters selected
    int                     FirstCharShown() const  {return m_first_char_shown;}///< returns the index of the first character visible in the Edit
    const string&           PreviousText() const    {return m_previous_text;}   ///< returns the text that was in the edit at the time of the last focus gain
    int                     CharIndexOf(int x) const;  ///< returns index into WindowText() of the character \a x pixels from left edge of visible portion of string
    int                     FirstCharOffset() const;   ///< returns the pixel distance from the beginning of the string to just before the first visible character
    int                     ScreenPosOfChar(int idx) const;  ///< returns the screen x-coordinate of the left side of the character at index \a idx in WindowText()
    int                     LastVisibleChar() const;   ///< actually, this returns the last + 1 visible char, for use in "for (i=0;i<last_vis_char;++i)", etc.
    //@}

    static const int PIXEL_MARGIN; ///< the number of pixels to leave between the text and the control's frame

private:
    void     ClearSelected();           ///< clears (deletes) selected characters, as when a del, backspace, or character is entered
    void     AdjustView();              ///< makes sure the caret ends up in view after an arbitrary move

    pair<int, int> m_cursor_pos;        ///< if .first == .second, the caret is drawn before character at m_cursor_pos.first
                                        //   otherwise, the range is selected (when range is selected, caret is considered at .second)
    int         m_first_char_shown;     ///< index into the string of the first character on the left end of the control's viewable area
    Clr         m_int_color;            ///< color of background inside text box
    Clr         m_hilite_color;         ///< color behind selected range
    Clr         m_sel_text_color;       ///< color of selected text

    string      m_previous_text;    ///< the contents when the focus was last gained
   
    mutable EditedSignalType      m_edited_sig;
    mutable FocusUpdateSignalType m_focus_update_sig;
};

} // namespace GG

#endif // _GGEdit_h_

