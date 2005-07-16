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

using namespace GG;

extern const Clr GG::CLR_ZERO(0, 0, 0, 0);
extern const Clr GG::CLR_BLACK(0, 0, 0, 255);
extern const Clr GG::CLR_WHITE(255, 255, 255, 255);
extern const Clr GG::CLR_GRAY(127, 127, 127, 255);
extern const Clr GG::CLR_SHADOW(127, 127, 127, 127);
extern const Clr GG::CLR_RED(255, 0, 0, 255);
extern const Clr GG::CLR_GREEN(0, 255, 0, 255);
extern const Clr GG::CLR_BLUE(0, 0, 255, 255);
extern const Clr GG::CLR_CYAN(0, 255, 255, 255);
extern const Clr GG::CLR_YELLOW(255, 255, 0, 255);
extern const Clr GG::CLR_MAGENTA(255, 0, 255, 255);

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

////////////////////////////////////////////////
// free function(s)
////////////////////////////////////////////////
bool GG::operator==(const Clr& rhs, const Clr& lhs) 
{
    return rhs.i == lhs.i;
}
