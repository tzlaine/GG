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

#ifndef _GGDrawUtil_h_
#define _GGDrawUtil_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

/** Returns the lightened version of color clr.  
    LightColor leaves the alpha channel unchanged, and multiplies the other channels by a 
    some factor.  (The factor is defined within LightColor().)*/
Clr LightColor(Clr clr);

/** Returns the darkened version of color clr.  
    DarkColor leaves the alpha channel unchanged, and divides the other channels by a 
    some factor.  (The factor is defined within DarkColor().)*/
Clr DarkColor(Clr clr);

/** Returns the "disabled" (grayed) version of color clr.  
    DisabledColor leaves the alpha channel unchanged, and adjusts the other channels in the direction of gray (GG_CLR_GRAY) 
    by a factor between 0.0f and 1.0f.  (The factor is defined within DisabledColor().)  This is used throughout the GG classes 
    to render disabled controls.*/
Clr DisabledColor(Clr clr);

/** Renders a rectangle starting at (x1,y1) and ending just before (x2,y2), and assumes that OpenGL in in a "2D" state.  
    The border is drawn in the desired thickness and color, then whatever is space is left inside that is filled with color 
    \a color.  No checking is done to make sure that \a border_thick * 2 is <= \a x2 - \a x1 (or <= \a y2 - \a y1, for that 
    matter).  This method of drawing and the 2D requirements are true for all functions that follow.*/
void FlatRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int border_thick = 2);

/** Like FlatRectangle(), but with a "beveled" appearance.  
    The border_color used to create a lighter and a darker version of border_color, which are used to draw beveled edges around 
    the inside of the rectangle to the desired thickness.  If \a up is true, the beveled edges are lighter on the top and left, 
    darker on the bottom and right, effecting a raised appearance.  If \a up is false, the opposite happens, and the rectangle 
    looks depressed.  This is true of all the Beveled*() functions.*/
void BeveledRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int bevel_thick = 2);

/** Draws a checkmark used to draw state buttons. */
void FlatCheck(int x1, int y1, int x2, int y2, Clr color);

/** Like FlatCheck(), but with a raised appearance. */
void BeveledCheck(int x1, int y1, int x2, int y2, Clr color);

/** Draws an X-mark used to draw state buttons. */
void FlatX(int x1, int y1, int x2, int y2, Clr color);

/** Like FlatX(), but with a raised appearance. */
void BeveledX(int x1, int y1, int x2, int y2, Clr color);

/** Draws a disk that appears to be a portion of a lit sphere.  
    The portion may appear raised or depressed.*/
void Bubble(int x1, int y1, int x2, int y2, Clr color, bool up = true);

/** Draws a circle of thick pixels thickness in the color specified. */
void FlatCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int thick = 2);

/** Draws a circle of \a thick pixels thickness in the color specified.  
    The circle appears to be beveled, and may be beveled in such a way as to appear raised or depressed.*/
void BeveledCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up = true, int bevel_thick = 2);

/** Draws a rounded rectangle of the specified thickness. 
    The radius of the circles used to draw the corners is specified by \a corner_radius.  Note that this means the rectangle 
    should be at least 2 * \a corner_radius on a side, but as with all the other functions, no such check is performed.*/
void FlatRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int corner_radius = 5, int border_thick = 2);

/** Like the FlatRoundedRectangle() function, but beveled (raised or depressed). */
void BeveledRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int corner_radius = 5, int bevel_thick = 2);

/** Using the same techniques as in Bubble(), creates a rounded, bubbly rectangle. */
void BubbleRectangle(int x1, int y1, int x2, int y2, Clr color, bool up, int corner_radius = 5);

} // namespace GG

#endif // _GGDrawUtil_h_


