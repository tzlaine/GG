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

#include <GG/Clr.h>

using namespace GG;

const Clr GG::CLR_ZERO(0, 0, 0, 0);
const Clr GG::CLR_BLACK(0, 0, 0, 255);
const Clr GG::CLR_WHITE(255, 255, 255, 255);
const Clr GG::CLR_GRAY(127, 127, 127, 255);
const Clr GG::CLR_SHADOW(127, 127, 127, 127);
const Clr GG::CLR_RED(255, 0, 0, 255);
const Clr GG::CLR_GREEN(0, 255, 0, 255);
const Clr GG::CLR_BLUE(0, 0, 255, 255);
const Clr GG::CLR_CYAN(0, 255, 255, 255);
const Clr GG::CLR_YELLOW(255, 255, 0, 255);
const Clr GG::CLR_MAGENTA(255, 0, 255, 255);

////////////////////////////////////////////////
// GG::Clr
////////////////////////////////////////////////
Clr::Clr() :
    r(0),
    g(0),
    b(0),
    a(0)
{}

Clr::Clr(GLubyte r_, GLubyte g_, GLubyte b_, GLubyte a_) :
    r(r_),
    g(g_),
    b(b_),
    a(a_)
{}


////////////////////////////////////////////////
// free function(s)
////////////////////////////////////////////////
Clr GG::FloatClr(float r, float g, float b, float a)
{
    return Clr(static_cast<GLubyte>(r * 255),
               static_cast<GLubyte>(g * 255),
               static_cast<GLubyte>(b * 255),
               static_cast<GLubyte>(a * 255));
}

bool GG::operator==(const Clr& rhs, const Clr& lhs) 
{ return rhs.r == lhs.r && rhs.g == lhs.g && rhs.b == lhs.b && rhs.a == lhs.a; }

bool GG::operator!=(const Clr& rhs, const Clr& lhs) 
{ return !(rhs == lhs); }
