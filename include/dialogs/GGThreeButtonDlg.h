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

#ifndef _GGThreeButtonDlg_h_
#define _GGThreeButtonDlg_h_

#ifndef _GGWnd_h_
#include "../GGWnd.h"
#endif

namespace GG {

class Button;

/** a general pop-up message or user input box with one, two, or three buttons.  This is designed to be used as a 
    generic message window, with just and "ok" button, or for any input consisting of only two or three choices, such 
    as "yes" and "no", "abort", "retry", and "fail", etc.  The enter key can be pressed to select the default button;
    the first button is always the default, unless the user sets a different one via SetDefault().  Similarly, the 
    escape key can be pressed to select the button that will get the user out of the dialog without taking any action,
    if one exists; the last button is always the escape button, unless a different
    one is set via SetEscape().  Note that this means that in a one-button dialog both enter and escape do the same 
    thing.  The default text for the buttons depends on the number of buttons.  For a one-button dialog, the default
    is "ok"; for a two-button dialog, the defults are "ok" and "cancel"; and for a three-button dialog, the defults 
    are "yes", "no", and "cancel".*/
class ThreeButtonDlg : public Wnd
{
public:
    /** \name Structors */ //@{
    /** basic ctor*/
    ThreeButtonDlg(int x, int y, int w, int h, const string& msg, const string& font_filename, int pts, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color = CLR_BLACK, int buttons = 3, Button* zero = 0, 
                   Button* one = 0, Button* two = 0);

    /** ctor that automatically centers the dialog in the app's area*/
    ThreeButtonDlg(int w, int h, const string& msg, const string& font_filename, int pts, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color = CLR_BLACK, int buttons = 3, Button* zero = 0, 
                   Button* one = 0, Button* two = 0);
                  
    /** basic ctor*/
    ThreeButtonDlg(int x, int y, int w, int h, const string& msg, const string& font_filename, int pts, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color, int buttons, const string& zero, 
                   const string& one = "", const string& two = "");

    /** ctor that automatically centers the dialog in the app's area*/
    ThreeButtonDlg(int w, int h, const string& msg, const string& font_filename, int pts, Clr color, 
                   Clr border_color, Clr button_color, Clr text_color, int buttons, const string& zero, 
                   const string& one = "", const string& two = "");

    ThreeButtonDlg(const XMLElement& elem); ///< ctor that constructs an ThreeButtonDlg object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ThreeButtonDlg object
    //@}

    /** \name Accessors */ //@{
    Clr ButtonColor() const    {return m_button_color;}///< returns the color of the buttons in the dialog
    int Result() const         {return m_result;}      ///< returns 0, 1, or 2, depending on which buttoon was clicked
    int DefaultButton() const  {return m_default;}     ///< returns the number of the button that will be chosen by default if the user hits enter (-1 if none)
    int EscapeButton() const   {return m_escape;}      ///< returns the number of the button that will be chosen by default if the user hits ESC (-1 if none)

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from an ThreeButtonDlg object
    //@}

    /** \name Mutators */ //@{
    virtual int Render();
    virtual int Keypress(Key key, Uint32 key_mods);
   
    void SetButtonColor(Clr color);  ///< sets the color used to render the dialog's buttons
    void SetDefaultButton(int i);    ///< sets the number of the button that will be chosen by default if the user hits enter (-1 to disable)
    void SetEscapeButton(int i);     ///< sets the number of the button that will be chosen by default if the user hits ESC (-1 to disable)
    //@}

private:
    int NumButtons() const;
    void AttachSignalChildren();
    void DetachSignalChildren();
    void Init(const string& msg, const string& font_filename, int pts, int buttons,
              const string& zero = "", const string& one = "", const string& two = "");
    void Button0Clicked()   {m_done = true; m_result = 0;}
    void Button1Clicked()   {m_done = true; m_result = 1;}
    void Button2Clicked()   {m_done = true; m_result = 2;}
   
    Clr      m_color;
    Clr      m_border_color;
    Clr      m_text_color;
    Clr      m_button_color;
    int      m_default;
    int      m_escape;
    int      m_result;
    Button*  m_button_0;
    Button*  m_button_1;
    Button*  m_button_2;
};

} // namespace GG

#endif // _GGThreeButtonDlg_h_

