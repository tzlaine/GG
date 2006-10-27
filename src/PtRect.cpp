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

#include <GG/PtRect.h>


using namespace GG;

////////////////////////////////////////////////
// GG::Pt
////////////////////////////////////////////////
Pt::Pt() :
    x(0),
    y(0)
{
}

Pt::Pt(int x_, int y_) :
    x(x_),
    y(y_)
{
}

std::ostream& GG::operator<<(std::ostream& os, const Pt& pt)
{
    os << "(" << pt.x << ", " << pt.y << ")";
    return os;
}


////////////////////////////////////////////////
// GG::Rect
////////////////////////////////////////////////
Rect::Rect()
{
}

Rect::Rect(const Pt& pt1, const Pt& pt2)
{
    ul.x = std::min(pt1.x, pt2.x);
    ul.y = std::min(pt1.y, pt2.y);
    lr.x = std::max(pt1.x, pt2.x);
    lr.y = std::max(pt1.y, pt2.y);
}

Rect::Rect(int x1, int y1, int x2, int y2) :
    ul(Pt(x1, y1)),
    lr(Pt(x2, y2))
{
}

bool Rect::Contains(const Pt& pt) const 
{
    return (ul <= pt && pt < lr);
}

std::ostream& GG::operator<<(std::ostream& os, const Rect& rect)
{
    os << "[" << rect.ul << " - " << rect.lr << "]";
    return os;
}
