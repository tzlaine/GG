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

/** \file DrawUtil.h
    Contains numerous 2D rendering convenience functions, for rendering rectangles, circles, etc. */

#ifndef _GG_DrawUtil_h_
#define _GG_DrawUtil_h_

#include <GG/Base.h>


namespace GG {

    /** Calls the appropriate version of glColor*() with \a clr.*/
    GG_API void glColor(Clr clr);

    /** Returns the lightened version of color clr.  LightColor leaves the alpha channel unchanged, and multiplies the
        other channels by a some factor.  (The factor is defined within LightColor().)*/
    GG_API Clr LightColor(Clr clr);

    /** Returns the darkened version of color clr.  DarkColor leaves the alpha channel unchanged, and divides the other
        channels by a some factor.  (The factor is defined within DarkColor().)*/
    GG_API Clr DarkColor(Clr clr);

    /** Returns the "disabled" (grayed) version of color clr.  DisabledColor leaves the alpha channel unchanged, and
        adjusts the other channels in the direction of gray (GG_CLR_GRAY) by a factor between 0.0 and 1.0.  (The factor
        is defined within DisabledColor().)  This is used throughout the GG classes to render disabled controls.*/
    GG_API Clr DisabledColor(Clr clr);

    /** Sets up a GL scissor box, so that everything outside of the screen region defined by points \a pt1 and \a pt2 is
        clipped out.  These coordinates should be in GG screen coordinates, with +y downward, instead of GL's screen
        coordinates.  \note Failing to call EndScissorClipping() after calling this function and before the next
        unmatched glPopAttrib() call may produce unexpected results.*/
    GG_API void BeginScissorClipping(Pt ul, Pt lr);

    /** Sets up a GL scissor box, so that everything outside of the screen region defined by points (<i>x1</i>,
        <i>y1</i>) and (<i>x2</i>, <i>y2</i>) is clipped out.  These coordinates should be in GG screen coordinates,
        with +y downward, instead of GL's screen coordinates.  \note Failing to call EndScissorClipping() after calling
        this function and before the next unmatched glPopAttrib() call may produce unexpected results.*/
    GG_API void BeginScissorClipping(int x1, int y1, int x2, int y2);

    /** Ends the current GL scissor box, restoring GL scissor state to what it was before the corresponding call to
        BeginScissorClipping().  \note If there is not an outstanding call to BeginScissorClipping() when this function
        is called, this function does nothing.*/
    GG_API void EndScissorClipping();

    /** Renders a rectangle starting at (x1,y1) and ending just before (x2,y2), and assumes that OpenGL in in a "2D"
        state.  The border is drawn in the desired thickness and color, then whatever is space is left inside that is
        filled with color \a color.  No checking is done to make sure that \a border_thick * 2 is <= \a x2 - \a x1 (or
        <= \a y2 - \a y1, for that matter).  This method of drawing and the 2D requirements are true for all functions
        that follow.*/
    GG_API void FlatRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int border_thick = 2);

    /** Like FlatRectangle(), but with a "beveled" appearance.  The border_color used to create a lighter and a darker
        version of border_color, which are used to draw beveled edges around the inside of the rectangle to the desired
        thickness.  If \a up is true, the beveled edges are lighter on the top and left, darker on the bottom and right,
        effecting a raised appearance.  If \a up is false, the opposite happens, and the rectangle looks depressed.
        This is true of all the Beveled*() functions.*/
    GG_API void BeveledRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int bevel_thick = 2,
                                 bool bevel_left = true, bool bevel_top = true, bool bevel_right = true, bool bevel_bottom = true);

    /** Draws a checkmark used to draw state buttons. */
    GG_API void FlatCheck(int x1, int y1, int x2, int y2, Clr color);

    /** Like FlatCheck(), but with a raised appearance. */
    GG_API void BeveledCheck(int x1, int y1, int x2, int y2, Clr color);

    /** Draws an X-mark used to draw state buttons. */
    GG_API void FlatX(int x1, int y1, int x2, int y2, Clr color);

    /** Like FlatX(), but with a raised appearance. */
    GG_API void BeveledX(int x1, int y1, int x2, int y2, Clr color);

    /** Draws a disk that appears to be a portion of a lit sphere.  The portion may appear raised or depressed.*/
    GG_API void Bubble(int x1, int y1, int x2, int y2, Clr color, bool up = true);

    /** Draws a circle of thick pixels thickness in the color specified. */
    GG_API void FlatCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int thick = 2);

    /** Draws a circle of \a thick pixels thickness in the color specified.  The circle appears to be beveled, and may
        be beveled in such a way as to appear raised or depressed.*/
    GG_API void BeveledCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up = true, int bevel_thick = 2);

    /** Draws a rounded rectangle of the specified thickness. The radius of the circles used to draw the corners is
        specified by \a corner_radius.  Note that this means the rectangle should be at least 2 * \a corner_radius on a
        side, but as with all the other functions, no such check is performed.*/
    GG_API void FlatRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int corner_radius = 5, int border_thick = 2);

    /** Like the FlatRoundedRectangle() function, but beveled (raised or depressed). */
    GG_API void BeveledRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int corner_radius = 5, int bevel_thick = 2);

    /** Using the same techniques as in Bubble(), creates a rounded, bubbly rectangle. */
    GG_API void BubbleRectangle(int x1, int y1, int x2, int y2, Clr color, bool up, int corner_radius = 5);

}

#endif // _GG_DrawUtil_h_
