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

#include <GG/DrawUtil.h>

#include <GG/GUI.h>

#include <valarray>

namespace { // file-scope constants and functions
    using namespace GG;

    const double   PI = 3.14159426;
    const double   SQRT2OVER2 = std::sqrt(2.0) / 2.0;

    /// a stack of the currently-active clipping rects, in GG coordinates, not OpenGL scissor coordinates
    std::vector<Rect> g_scissor_clipping_rects;

    /// whenever points on the unit circle are calculated with expensive sin() and cos() calls, the results are cached here
    std::map<int, std::valarray<double> > unit_circle_coords;
    /// this doesn't serve as a cache, but does allow us to prevent numerous constructions and destructions of Clr valarrays.
    std::map<int, std::valarray<Clr> > color_arrays;

    void Rectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color1, Clr border_color2, int bevel_thick,
                   bool bevel_left, bool bevel_top, bool bevel_right, bool bevel_bottom)
    {
        glDisable(GL_TEXTURE_2D);

        int inner_x1 = x1 + (bevel_left ? bevel_thick : 0), inner_y1 = y1 + (bevel_top ? bevel_thick : 0),
            inner_x2 = x2 - (bevel_right ? bevel_thick : 0), inner_y2 = y2 - (bevel_bottom ? bevel_thick : 0);

        int vertices[] = {inner_x2, inner_y1, x2, y1, inner_x1, inner_y1, x1, y1, inner_x1, inner_y2, x1, y2,
                          inner_x2, inner_y2, x2, y2, inner_x2, inner_y1, x2, y1};

        // draw beveled edges
        if (bevel_thick && (border_color1 != CLR_ZERO || border_color2 != CLR_ZERO)) {
            glColor(border_color1);
            if (border_color1 == border_color2) {
                glBegin(GL_QUAD_STRIP);
                for (int i = 0; i < 10; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
            } else {
                glBegin(GL_QUAD_STRIP);
                for (int i = 0; i < 6; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
                glColor(border_color2);
                glBegin(GL_QUAD_STRIP);
                for (int i = 4; i < 10; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
            }
        }

        // draw interior of rectangle
        if (color != CLR_ZERO) {
            glVertexPointer(2, GL_INT, 2 * 2 * sizeof(GL_INT), vertices);
            glColor(color);
            glBegin(GL_QUADS);
            glVertex2i(inner_x2, inner_y1);
            glVertex2i(inner_x1, inner_y1);
            glVertex2i(inner_x1, inner_y2);
            glVertex2i(inner_x2, inner_y2);
            glEnd();
        }

        glEnable(GL_TEXTURE_2D);
    }

    void Check(int x1, int y1, int x2, int y2, Clr color1, Clr color2, Clr color3)
    {
        int wd = x2 - x1, ht = y2 - y1;
        glDisable(GL_TEXTURE_2D);

        // all vertices
        double verts[][2] = {{-0.2f, 0.2f}, {-0.6f, -0.2f}, {-0.6f, 0.0f}, {-0.2f, 0.4f}, {-0.8f, 0.0f},
                             {-0.2f, 0.6f}, { 0.8f, -0.4f}, {0.6f, -0.4f}, {0.8f, -0.8f}};

        glPushMatrix();
        const double sf = 1.25f; // just a scale factor; the check wasn't the right size as drawn originally
        glTranslated(x1 + wd / 2.0f, y1 + ht / 2.0 * sf, 0.0); // move origin to the center of the rectangle
        glScaled(wd / 2.0 * sf, ht / 2.0 * sf, 1.0);          // map the range [-1,1] to the rectangle in both directions

        glColor(color3);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[2]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[7]);
        glEnd();

        glColor(color2);
        glBegin(GL_QUADS);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[6]);
        glEnd();

        glColor(color1);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[6]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[3]);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void X(int x1, int y1, int x2, int y2, Clr color1, Clr color2, Clr color3)
    {
        int wd = x2 - x1, ht = y2 - y1;
        glDisable(GL_TEXTURE_2D);

        // all vertices
        double verts[][2] = {{-0.4f, -0.6f}, {-0.6f, -0.4f}, {-0.4f, -0.4f}, {-0.2f, 0.0f}, {-0.6f, 0.4f},
                             {-0.4f, 0.6f}, {-0.4f, 0.4f}, {0.0f, 0.2f}, {0.4f, 0.6f}, {0.6f, 0.4f},
                             {0.4f, 0.4f}, {0.2f, 0.0f}, {0.6f, -0.4f}, {0.4f, -0.6f}, {0.4f, -0.4f},
                             {0.0f, -0.2f}, {0.0f, 0.0f}};

        glPushMatrix();
        const double sf = 1.75f; // just a scale factor; the check wasn't the right size as drawn originally
        glTranslatef(x1 + wd / 2.0f, y1 + ht / 2.0f, 0.0f);   // move origin to the center of the rectangle
        glScalef(wd / 2.0f * sf, ht / 2.0f * sf, 1.0f);       // map the range [-1,1] to the rectangle in both directions

        glColor(color1);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[12]);
        glVertex2dv(verts[13]);
        glVertex2dv(verts[14]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[15]);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[9]);
        glVertex2dv(verts[11]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[10]);
        glEnd();

        glColor(color2);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[2]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[13]);
        glVertex2dv(verts[15]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[14]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[16]);
        glEnd();

        glColor(color3);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[9]);
        glVertex2dv(verts[10]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[14]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[11]);
        glVertex2dv(verts[12]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[10]);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void BubbleArc(int x1, int y1, int x2, int y2, Clr color1, Clr color2, Clr color3, double theta1, double theta2)
    {
        int wd = x2 - x1, ht = y2 - y1;
        glDisable(GL_TEXTURE_2D);

        // correct theta* values to range [0, 2pi)
        if (theta1 < 0)
            theta1 += (int(-theta1 / (2 * PI)) + 1) * 2 * PI;
        else if (theta1 >= 2 * PI)
            theta1 -= int(theta1 / (2 * PI)) * 2 * PI;
        if (theta2 < 0)
            theta2 += (int(-theta2 / (2 * PI)) + 1) * 2 * PI;
        else if (theta2 >= 2 * PI)
            theta2 -= int(theta2 / (2 * PI)) * 2 * PI;

        const int      SLICES = std::min(3 + std::max(wd, ht), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
        const double   HORZ_THETA = (2 * PI) / SLICES;

        std::valarray<double>& unit_vertices = unit_circle_coords[SLICES];
        std::valarray<Clr>&    colors = color_arrays[SLICES];
        bool calc_vertices = unit_vertices.size() == 0;
        if (calc_vertices) {
            unit_vertices.resize(2 * (SLICES + 1), 0.0);
            double theta = 0.0f;
            for (int j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
                unit_vertices[j*2] = cos(-theta);
                unit_vertices[j*2+1] = sin(-theta);
            }
            colors.resize(SLICES + 1, Clr()); // create but don't initialize (this is essentially just scratch space, since the colors are different call-to-call)
        }
        int first_slice_idx = int(theta1 / HORZ_THETA + 1);
        int last_slice_idx = int(theta2 / HORZ_THETA - 1);
        if (theta1 >= theta2)
            last_slice_idx += SLICES;
        for (int j = first_slice_idx; j <= last_slice_idx; ++j) { // calculate the color value for each needed point
            int X = (j > SLICES ? (j - SLICES) : j) * 2, Y = X + 1;
            double color_scale_factor = (SQRT2OVER2 * (unit_vertices[X] + unit_vertices[Y]) + 1) / 2; // this is essentially the dot product of (x,y) with (sqrt2over2,sqrt2over2), the direction of the light source, scaled to the range [0,1]
            colors[j] = Clr(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                            GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                            GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                            GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        }

        glPushMatrix();
        glTranslatef(x1 + wd / 2.0f, y1 + ht / 2.0f, 0.0f);   // move origin to the center of the rectangle
        glScalef(wd / 2.0f, ht / 2.0f, 1.0f);                 // map the range [-1,1] to the rectangle in both (x- and y-) directions

        glColor(color1);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        // point on circle at angle theta1
        double x = cos(-theta1),
            y = sin(-theta1);
        double color_scale_factor = (SQRT2OVER2 * (x + y) + 1) / 2;
        glColor4ub(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                   GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                   GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                   GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glVertex2f(x, y);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glColor(colors[i]);
            glVertex2f(unit_vertices[X], unit_vertices[Y]);
        }
        // theta2
        x = cos(-theta2);
        y = sin(-theta2);
        color_scale_factor = (SQRT2OVER2 * (x + y) + 1) / 2;
        glColor4ub(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                   GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                   GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                   GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glVertex2f(x, y);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void CircleArc(int x1, int y1, int x2, int y2, Clr color, Clr border_color1, Clr border_color2, int bevel_thick, double theta1, double theta2)
    {
        int wd = x2 - x1, ht = y2 - y1;
        glDisable(GL_TEXTURE_2D);

        // correct theta* values to range [0, 2pi)
        if (theta1 < 0)
            theta1 += (int(-theta1 / (2 * PI)) + 1) * 2 * PI;
        else if (theta1 >= 2 * PI)
            theta1 -= int(theta1 / (2 * PI)) * 2 * PI;
        if (theta2 < 0)
            theta2 += (int(-theta2 / (2 * PI)) + 1) * 2 * PI;
        else if (theta2 >= 2 * PI)
            theta2 -= int(theta2 / (2 * PI)) * 2 * PI;

        const int      SLICES = std::min(3 + std::max(wd, ht), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
        const double   HORZ_THETA = (2 * PI) / SLICES;

        std::valarray<double>& unit_vertices = unit_circle_coords[SLICES];
        std::valarray<Clr>&    colors = color_arrays[SLICES];
        bool calc_vertices = unit_vertices.size() == 0;
        if (calc_vertices) {
            unit_vertices.resize(2 * (SLICES + 1), 0.0);
            double theta = 0.0f;
            for (int j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
                unit_vertices[j*2] = cos(-theta);
                unit_vertices[j*2+1] = sin(-theta);
            }
            colors.resize(SLICES + 1, Clr()); // create but don't initialize (this is essentially just scratch space, since the colors are different call-to-call)
        }
        int first_slice_idx = int(theta1 / HORZ_THETA + 1);
        int last_slice_idx = int(theta2 / HORZ_THETA - 1);
        if (theta1 >= theta2)
            last_slice_idx += SLICES;
        for (int j = first_slice_idx; j <= last_slice_idx; ++j) { // calculate the color value for each needed point
            int X = (j > SLICES ? (j - SLICES) : j) * 2, Y = X + 1;
            double color_scale_factor = (SQRT2OVER2 * (unit_vertices[X] + unit_vertices[Y]) + 1) / 2; // this is essentially the dot product of (x,y) with (sqrt2over2,sqrt2over2), the direction of the light source, scaled to the range [0,1]
            colors[j] = Clr(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                            GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                            GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                            GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        }

        glPushMatrix();
        glTranslatef(x1 + wd / 2.0f, y1 + ht / 2.0f, 0.0f);   // move origin to the center of the rectangle
        glScalef(wd / 2.0f, ht / 2.0f, 1.0f);                 // map the range [-1,1] to the rectangle in both (x- and y-) directions

        double inner_radius = (std::min(wd, ht) - 2.0 * bevel_thick) / std::min(wd, ht);
        glColor(color);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        // point on circle at angle theta1
        double theta1_x = cos(-theta1), theta1_y = sin(-theta1);
        glVertex2f(theta1_x * inner_radius, theta1_y * inner_radius);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glVertex2f(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
        }      // theta2
        double theta2_x = cos(-theta2), theta2_y = sin(-theta2);
        glVertex2f(theta2_x * inner_radius, theta2_y * inner_radius);
        glEnd();
        glBegin(GL_QUAD_STRIP);
        // point on circle at angle theta1
        double color_scale_factor = (SQRT2OVER2 * (theta1_x + theta1_y) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glVertex2f(theta1_x, theta1_y);
        glVertex2f(theta1_x * inner_radius, theta1_y * inner_radius);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glColor(colors[i]);
            glVertex2f(unit_vertices[X], unit_vertices[Y]);
            glVertex2f(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
        }
        // theta2
        color_scale_factor = (SQRT2OVER2 * (theta2_x + theta2_y) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glVertex2f(theta2_x, theta2_y);
        glVertex2f(theta2_x * inner_radius, theta2_y * inner_radius);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void RoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color1, Clr border_color2, int corner_radius, int thick)
    {
        int circle_diameter = corner_radius * 2;
        CircleArc(x2 - circle_diameter, y1, x2, y1 + circle_diameter, color, border_color2, border_color1, thick, 0, 0.5 * PI);  // ur corner
        CircleArc(x1, y1, x1 + circle_diameter, y1 + circle_diameter, color, border_color2, border_color1, thick, 0.5 * PI, PI); // ul corner
        CircleArc(x1, y2 - circle_diameter, x1 + circle_diameter, y2, color, border_color2, border_color1, thick, PI, 1.5 * PI); // ll corner
        CircleArc(x2 - circle_diameter, y2 - circle_diameter, x2, y2, color, border_color2, border_color1, thick, 1.5 * PI, 0);  // lr corner

        glDisable(GL_TEXTURE_2D);

        // top
        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glBegin(GL_QUADS);
        glVertex2i(x2 - corner_radius, y1);
        glVertex2i(x1 + corner_radius, y1);
        glVertex2i(x1 + corner_radius, y1 + thick);
        glVertex2i(x2 - corner_radius, y1 + thick);
        glEnd();

        // left (uses color scale factor (SQRT2OVER2 * (1 + 0) + 1) / 2, which equals that of top
        glBegin(GL_QUADS);
        glVertex2i(x1 + thick, y1 + corner_radius);
        glVertex2i(x1, y1 + corner_radius);
        glVertex2i(x1, y2 - corner_radius);
        glVertex2i(x1 + thick, y2 - corner_radius);
        glEnd();

        // right
        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glBegin(GL_QUADS);
        glVertex2i(x2, y1 + corner_radius);
        glVertex2i(x2 - thick, y1 + corner_radius);
        glVertex2i(x2 - thick, y2 - corner_radius);
        glVertex2i(x2, y2 - corner_radius);
        glEnd();

        // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of left
        glBegin(GL_QUADS);
        glVertex2i(x2 - corner_radius, y2 - thick);
        glVertex2i(x1 + corner_radius, y2 - thick);
        glVertex2i(x1 + corner_radius, y2);
        glVertex2i(x2 - corner_radius, y2);
        glEnd();

        // middle
        glColor(color);
        glBegin(GL_QUADS);
        glVertex2i(x2 - corner_radius, y1 + thick);
        glVertex2i(x1 + corner_radius, y1 + thick);
        glVertex2i(x1 + corner_radius, y2 - thick);
        glVertex2i(x2 - corner_radius, y2 - thick);

        glVertex2i(x2 - thick, y1 + corner_radius);
        glVertex2i(x2 - corner_radius, y1 + corner_radius);
        glVertex2i(x2 - corner_radius, y2 - corner_radius);
        glVertex2i(x2 - thick, y2 - corner_radius);

        glVertex2i(x1 + thick, y1 + corner_radius);
        glVertex2i(x1 + corner_radius, y1 + corner_radius);
        glVertex2i(x1 + corner_radius, y2 - corner_radius);
        glVertex2i(x1 + thick, y2 - corner_radius);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

    void BubbleRectangle(int x1, int y1, int x2, int y2, Clr color1, Clr color2, Clr color3, int corner_radius)
    {
        int circle_diameter = corner_radius * 2;
        BubbleArc(x2 - circle_diameter, y1, x2, y1 + circle_diameter, color1, color3, color2, 0, 0.5 * PI);  // ur corner
        BubbleArc(x1, y1, x1 + circle_diameter, y1 + circle_diameter, color1, color3, color2, 0.5 * PI, PI); // ul corner
        BubbleArc(x1, y2 - circle_diameter, x1 + circle_diameter, y2, color1, color3, color2, PI, 1.5 * PI); // ll corner
        BubbleArc(x2 - circle_diameter, y2 - circle_diameter, x2, y2, color1, color3, color2, 1.5 * PI, 0);  // lr corner

        glDisable(GL_TEXTURE_2D);

        // top
        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        Clr scaled_color(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                         GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                         GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                         GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glBegin(GL_QUADS);
        glColor(scaled_color);
        glVertex2i(x2 - corner_radius, y1);
        glVertex2i(x1 + corner_radius, y1);
        glColor(color1);
        glVertex2i(x1 + corner_radius, y1 + corner_radius);
        glVertex2i(x2 - corner_radius, y1 + corner_radius);
        glEnd();

        // left (uses color scale factor (SQRT2OVER2 * (1 + 0) + 1) / 2, which equals that of top
        glBegin(GL_QUADS);
        glColor(scaled_color);
        glVertex2i(x1, y1 + corner_radius);
        glVertex2i(x1, y2 - corner_radius);
        glColor(color1);
        glVertex2i(x1 + corner_radius, y2 - corner_radius);
        glVertex2i(x1 + corner_radius, y1 + corner_radius);
        glEnd();

        // right
        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        scaled_color = Clr(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                           GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                           GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                           GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex2i(x2 - corner_radius, y1 + corner_radius);
        glVertex2i(x2 - corner_radius, y2 - corner_radius);
        glColor(scaled_color);
        glVertex2i(x2, y2 - corner_radius);
        glVertex2i(x2, y1 + corner_radius);
        glEnd();

        // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of left
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex2i(x2 - corner_radius, y2 - corner_radius);
        glVertex2i(x1 + corner_radius, y2 - corner_radius);
        glColor(scaled_color);
        glVertex2i(x1 + corner_radius, y2);
        glVertex2i(x2 - corner_radius, y2);
        glEnd();

        // middle
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex2i(x2 - corner_radius, y1 + corner_radius);
        glVertex2i(x1 + corner_radius, y1 + corner_radius);
        glVertex2i(x1 + corner_radius, y2 - corner_radius);
        glVertex2i(x2 - corner_radius, y2 - corner_radius);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
} // namespace


namespace GG {

    void glColor(Clr clr)
    { glColor4ub(clr.r, clr.g, clr.b, clr.a); }

    Clr LightColor(Clr clr)
    {
        const double scale_factor = 2.0;   // factor by which the color is lightened
        Clr retval = clr;
        retval.r = std::min(static_cast<int>(retval.r * scale_factor), 255);
        retval.g = std::min(static_cast<int>(retval.g * scale_factor), 255);
        retval.b = std::min(static_cast<int>(retval.b * scale_factor), 255);
        return retval;
    }

    Clr DarkColor(Clr clr)
    {
        const double scale_factor = 2.0;   // factor by which the color is darkened
        Clr retval = clr;
        retval.r = static_cast<int>(retval.r / scale_factor);
        retval.g = static_cast<int>(retval.g / scale_factor);
        retval.b = static_cast<int>(retval.b / scale_factor);
        return retval;
    }

    Clr DisabledColor(Clr clr)
    {
        Clr retval = clr;
        const double gray_factor = 0.75; // amount to move clr in the direction of gray
        retval.r = static_cast<int>(retval.r + (CLR_GRAY.r - retval.r) * gray_factor);
        retval.g = static_cast<int>(retval.g + (CLR_GRAY.g - retval.g) * gray_factor);
        retval.b = static_cast<int>(retval.b + (CLR_GRAY.b - retval.b) * gray_factor);
        return retval;
    }

    void BeginScissorClipping(Pt ul, Pt lr)
    {
        BeginScissorClipping(ul.x, ul.y, lr.x ,lr.y);
    }

    void BeginScissorClipping(int x1, int y1, int x2, int y2)
    {
        if (g_scissor_clipping_rects.empty()) {
            // save old scissor state
            glPushAttrib(GL_SCISSOR_BIT);
            glEnable(GL_SCISSOR_TEST);
        } else {
            const Rect& r = g_scissor_clipping_rects.back();
            x1 = std::max(r.Left(), std::min(x1, r.Right()));
            y1 = std::max(r.Top(), std::min(y1, r.Bottom()));
            x2 = std::max(r.Left(), std::min(x2, r.Right()));
            y2 = std::max(r.Top(), std::min(y2, r.Bottom()));
        }
        glScissor(x1, GUI::GetGUI()->AppHeight() - y2, x2 - x1, y2 - y1);
        g_scissor_clipping_rects.push_back(Rect(x1, y1, x2, y2));
    }

    void EndScissorClipping()
    {
        g_scissor_clipping_rects.pop_back();
        if (g_scissor_clipping_rects.empty()) {
            // restore previous scissor-clipping state
            glPopAttrib();
        } else {
            const Rect& r = g_scissor_clipping_rects.back();
            glScissor(r.Left(), GUI::GetGUI()->AppHeight() - r.Bottom(), r.Width(), r.Height());
        }
    }

    void FlatRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int border_thick/* = 2*/)
    {
        Rectangle(x1, y1, x2, y2, color, border_color, border_color, border_thick, true, true, true, true);
    }

    void BeveledRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int bevel_thick/* = 2*/,
                          bool bevel_left/* = true*/, bool bevel_top/* = true*/, bool bevel_right/* = true*/, bool bevel_bottom/* = true*/)
    {
        Rectangle(x1, y1, x2, y2, color, (up ? LightColor(border_color) : DarkColor(border_color)), (up ? DarkColor(border_color) : LightColor(border_color)), bevel_thick, bevel_left, bevel_top, bevel_right, bevel_bottom);
    }

    void FlatRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int corner_radius/* = 5*/, int border_thick/* = 2*/)
    {
        RoundedRectangle(x1, y1, x2, y2, color, border_color, border_color, corner_radius, border_thick);
    }

    void BeveledRoundedRectangle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up, int corner_radius/* = 5*/, int bevel_thick/* = 2*/)
    {
        RoundedRectangle(x1, y1, x2, y2, color, (up ? LightColor(border_color) : DarkColor(border_color)), (up ? DarkColor(border_color) : LightColor(border_color)), corner_radius, bevel_thick);
    }

    void FlatCheck(int x1, int y1, int x2, int y2, Clr color)
    {
        Check(x1, y1, x2, y2, color, color, color);
    }

    void BeveledCheck(int x1, int y1, int x2, int y2, Clr color)
    {
        Check(x1, y1, x2, y2, color, LightColor(color), DarkColor(color));
    }

    void FlatX(int x1, int y1, int x2, int y2, Clr color)
    {
        X(x1, y1, x2, y2, color, color, color);
    }

    void BeveledX(int x1, int y1, int x2, int y2, Clr color)
    {
        X(x1, y1, x2, y2, color, LightColor(color), DarkColor(color));
    }

    void Bubble(int x1, int y1, int x2, int y2, Clr color, bool up/* = true*/)
    {
        BubbleArc(x1, y1, x2, y2, color, (up ? DarkColor(color) : LightColor(color)), (up ? LightColor(color) : DarkColor(color)), 0, 0);
    }

    void FlatCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, int thick/* = 2*/)
    {
        CircleArc(x1, y1, x2, y2, color, border_color, border_color, thick, 0, 0);
    }

    void BeveledCircle(int x1, int y1, int x2, int y2, Clr color, Clr border_color, bool up/* = true*/, int bevel_thick/* = 2*/)
    {
        CircleArc(x1, y1, x2, y2, color, (up ? DarkColor(border_color) : LightColor(border_color)), (up ? LightColor(border_color) : DarkColor(border_color)), bevel_thick, 0, 0);
    }

    void BubbleRectangle(int x1, int y1, int x2, int y2, Clr color, bool up, int corner_radius/* = 5*/)
    {
        ::BubbleRectangle(x1, y1, x2, y2, color, (up ? LightColor(color) : DarkColor(color)), (up ? DarkColor(color) : LightColor(color)), corner_radius);
    }

} // namespace GG
