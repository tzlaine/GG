/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */

#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/ProgressBar.h>


using namespace GG;

namespace {
    const int BORDER_THICKNESS = 1;
    const int INTERIOR_MARGIN = 3;
}


ProgressBar::ProgressBar(X x, Y y, X w, Y h, Orientation orientation,
                         unsigned int bar_width, Clr color,
                         Clr bar_color/* = CLR_SHADOW*/,
                         Clr interior_color/* = CLR_ZERO*/) :
    Control(x, y, w, h, Flags<WndFlag>()),
    m_pos(0.0),
    m_orientation(orientation),
    m_bar_width(bar_width),
    m_bar_color(bar_color),
    m_interior_color(interior_color)
{ m_color = color; }

Pt ProgressBar::MinUsableSize() const
{
    const Pt MIN_SIZE = MinSize();
    return Pt(m_orientation == VERTICAL ? std::max(MIN_SIZE.x, X(m_bar_width)) : Size().x,
              m_orientation == VERTICAL ? Size().y : std::max(MIN_SIZE.y, Y(m_bar_width)));
}

Clr ProgressBar::BarColor() const
{ return m_bar_color; }

Clr ProgressBar::InteriorColor() const
{ return m_interior_color; }

void ProgressBar::Render()
{
    Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    Clr interior_color_to_use = Disabled() ? DisabledColor(m_interior_color) : m_interior_color;

    const Pt UL = UpperLeft();
    const Pt LR = LowerRight();
    Pt ul = UpperLeft(), lr = LowerRight();
    if (m_orientation == VERTICAL) {
        ul.x = ((LR.x + UL.x) - static_cast<int>(m_bar_width)) / 2;
        lr.x = ul.x + static_cast<int>(m_bar_width);
    } else {
        ul.y = ((LR.y + UL.y) - static_cast<int>(m_bar_width)) / 2;
        lr.y = ul.y + static_cast<int>(m_bar_width);
    }
    FlatRectangle(ul, lr, interior_color_to_use, color_to_use, BORDER_THICKNESS);

    ul.x += INTERIOR_MARGIN;
    ul.y += INTERIOR_MARGIN;
    lr.x -= INTERIOR_MARGIN;
    lr.y -= INTERIOR_MARGIN;

    Clr bar_color_to_use = Disabled() ? DisabledColor(m_bar_color) : m_bar_color;

    if (m_orientation == VERTICAL) {
        Y y_pos(ul.y + (lr.y - ul.y) * (1.0 - m_pos) + Y_d(0.5));
        if (y_pos != ul.y)
            FlatRectangle(Pt(ul.x, y_pos), lr, bar_color_to_use, CLR_ZERO, 0);
    } else {
        X x_pos(ul.x + (lr.x - ul.x) * m_pos + X_d(0.5));
        if (x_pos != ul.x)
            FlatRectangle(ul, Pt(x_pos, lr.y), bar_color_to_use, CLR_ZERO, 0);
    }
}

void ProgressBar::SetPosition(double pos)
{ m_pos = std::max(0.0, std::min(pos, 1.0)); }
