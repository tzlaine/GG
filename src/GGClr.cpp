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

#include "GGClr.h"

namespace GG {

////////////////////////////////////////////////
// GG::Clr
////////////////////////////////////////////////
Clr::Clr() : 
    i(0) 
{
}

Clr::Clr(Uint32 clr) : 
    i(clr) 
{
}

Clr::Clr(int _r, int _g, int _b, int _a) 
{
    r = std::min(_r, 255); 
    g = std::min(_g, 255); 
    b = std::min(_b, 255); 
    a = std::min(_a, 255);
}

Clr::Clr(double _r, double _g, double _b, double _a) 
{
    r = Uint8(_r * 255); 
    g = Uint8(_g * 255); 
    b = Uint8(_b * 255); 
    a = Uint8(_a * 255);
}

Clr::Clr(Uint8 arr[]) 
{
    r = arr[0]; 
    g = arr[1]; 
    b = arr[2]; 
    a = arr[3];
}

Clr::Clr(double arr[]) 
{
    r = Uint8(arr[0] * 255); 
    g = Uint8(arr[1] * 255); 
    b = Uint8(arr[2] * 255); 
    a = Uint8(arr[3] * 255);
}

Clr::Clr(const XMLElement& elem)
{
    if (elem.Tag() != "GG::Clr")
	throw std::invalid_argument("Attempted to construct a GG::Clr from an XMLElement that had a tag other than \"GG::Clr\"");
   
    const XMLElement& r_elem = elem.Child("red");
    const XMLElement& g_elem = elem.Child("green");
    const XMLElement& b_elem = elem.Child("blue");
    const XMLElement& a_elem = elem.Child("alpha");
    r = lexical_cast<int>(r_elem.Attribute("value"));
    g = lexical_cast<int>(g_elem.Attribute("value"));
    b = lexical_cast<int>(b_elem.Attribute("value"));
    a = lexical_cast<int>(a_elem.Attribute("value"));
}
   
XMLElement Clr::XMLEncode() const
{
    XMLElement retval("GG::Clr");
    XMLElement temp("red");
    temp.SetAttribute("value", boost::lexical_cast<string>(int(r)));
    retval.AppendChild(temp);
    temp = XMLElement("green");
    temp.SetAttribute("value", boost::lexical_cast<string>(int(g)));
    retval.AppendChild(temp);
    temp = XMLElement("blue");
    temp.SetAttribute("value", boost::lexical_cast<string>(int(b)));
    retval.AppendChild(temp);
    temp = XMLElement("alpha");
    temp.SetAttribute("value", boost::lexical_cast<string>(int(a)));
    retval.AppendChild(temp);
    return retval;
}

} // namespace GG


