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

#include "GGControl.h"

#include <XMLValidators.h>

namespace GG {

////////////////////////////////////////////////
// GG::Control
////////////////////////////////////////////////
Control::Control(const XMLElement& elem) :
        Wnd(elem.Child("GG::Wnd"))
{
    if (elem.Tag() != "GG::Control")
        throw std::invalid_argument("Attempted to construct a GG::Control from an XMLElement that had a tag other than \"GG::Control\"");

    m_color = Clr(elem.Child("m_color").Child("GG::Clr"));
    m_disabled = lexical_cast<bool>(elem.Child("m_disabled").Text());
}

XMLElement Control::XMLEncode() const
{
    XMLElement retval("GG::Control");
    retval.AppendChild(Wnd::XMLEncode());
    retval.AppendChild(XMLElement("m_color", m_color.XMLEncode()));
    retval.AppendChild(XMLElement("m_disabled", lexical_cast<string>(m_disabled)));
    return retval;
}

XMLElementValidator Control::XMLValidator() const
{
    XMLElementValidator retval("GG::Control");
    retval.AppendChild(Wnd::XMLValidator());
    retval.AppendChild(XMLElementValidator("m_color", m_color.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_disabled", new Validator<bool>()));
    return retval;
}

} // namespace GG

