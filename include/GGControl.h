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

#ifndef _GGControl_h_
#define _GGControl_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

namespace GG {

/** This is an abstract base class for all control classes.
   Each control has (like all windows) coordinates offset from the upper-left corner of it's parent's client area.  
   All controls may be disabled.  By default, a Control passes keyboard input to its parent Wnd.  Any class derived
   from Control should do the same with any keyboard input it does not need for its own use.  For instance, an Edit
   control needs to know about arrow key keyboard input, but it should pass other key presses like 'ESC' to its parent.*/
class Control : public Wnd
{
public:
   /** \name Accessors */ //@{
   Clr            Color() const              {return m_color;}    ///< returns the color of the control
   bool           Disabled() const           {return m_disabled;} ///< returns true if the control is disabled, false otherwise

   virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a Control object
   //@}
   
   /** \name Mutators */ //@{
   virtual int    Render() = 0;

   virtual int    Keypress(Key key, Uint32 key_mods) {return (Parent() ? Parent()->Keypress(key, key_mods) : 1);}

   virtual void   SetColor(Clr c)            {m_color = c;}       ///< sets the color of the control
   virtual void   Disable(bool b = true)     {m_disabled = b;}    ///< disables/enables the control; disabled controls appear greyed
   //@}
   
protected:
   /** \name Structors */ //@{
   Control(int x, int y, int w, int h, Uint32 flags = CLICKABLE) : Wnd(x, y, w, h, flags), m_disabled(false) {} ///< default ctor
   Control(const XMLElement& elem); ///< ctor that constructs a Control object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Control object
   //@}

   Clr      m_color;      ///< the color of the control
   bool     m_disabled;   ///< whether or not this control is disabled
};

} // namespace GG

#endif // _GGControl_h_

