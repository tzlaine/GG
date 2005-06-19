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

/* $Header$ */

/** \file GGPtRect.h
    Contains the utility classes Pt and Rect. */

#ifndef _GGPtRect_h_
#define _GGPtRect_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

/** a GG screen coordinate class */
struct GG_API Pt
{
    /** \name Structors */ //@{
    Pt();                       ///< default ctor
    Pt(int x_, int y_);         ///< ctor that creates a Pt ( \a _x , \a y )
    Pt(const XMLElement& elem); ///< ctor that creates a Pt as defined in XMLElement elem. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Pt object
    //@}

    /** \name Accessors */ //@{
    /** returns true if x < \a rhs.x or returns true if x == \a rhs.x and y <\a rhs.y.  This is useful for sorting Pts 
        in STL containers and algorithms*/
    bool Less(const Pt& rhs) const {return x < rhs.x ? true : (x == rhs.x ? (y < rhs.y ? true : false) : false);}

    XMLElement XMLEncode() const; ///< returns an XMLElement that encodes this Pt
    XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Pt
    //@}

    /** \name Mutators */ //@{
    void  operator+=(const Pt& rhs)      {x += rhs.x; y += rhs.y;} ///< adds \a rhs to Pt
    void  operator-=(const Pt& rhs)      {x -= rhs.x; y -= rhs.y;} ///< subtracts \a rhs to Pt
    Pt    operator-() const              {return Pt(-x, -y);}      ///< negates Pt
    //@}

    int x; ///< the x component
    int y; ///< the y component
};

/** a GG rectangle class. this is essentially just two points that bound the rectangle*/
struct GG_API Rect
{
    /** \name Structors */ //@{
    Rect();                                ///< default ctor
    Rect(const Pt& pt1, const Pt& pt2);    ///< ctor that constructs a Rect from two corners; any two opposing corners will do
    Rect(int x1, int y1, int x2, int y2);  ///< ctor taht constructs a Rect from its left, upper, right, and bottom boundaries
    Rect(const XMLElement& elem);          ///< ctor that creates a Rect as defined in XMLElement elem. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Rect object
    //@}

    /** \name Accessors */ //@{
    int   Left() const         {return ul.x;}        ///< returns the left boundary of the Rect
    int   Right() const        {return lr.x;}        ///< returns the right boundary of the Rect
    int   Top() const          {return ul.y;}        ///< returns the top boundary of the Rect
    int   Bottom() const       {return lr.y;}        ///< returns the bottom boundary of the Rect
    Pt    UpperLeft() const    {return ul;}          ///< returns the upper-left corner of the Rect
    Pt    LowerRight() const   {return lr;}          ///< returns the lower-right corner of the Rect
    int   Width() const        {return lr.x - ul.x;} ///< returns the width of the Rect
    int   Height() const       {return lr.y - ul.y;} ///< returns the height of the Rect

    bool  Contains(const Pt& pt) const; ///< returns true iff \a pt falls inside the Rect

    XMLElement XMLEncode() const; ///< returns an XMLElement that encodes this Rect
    XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Rect
    //@}

    /** \name Mutators */ //@{
    void operator+=(const Pt& pt)      {ul += pt; lr += pt;} ///< shifts the Rect by adding \a pt to each corner
    void operator-=(const Pt& pt)      {ul -= pt; lr -= pt;} ///< shifts the Rect by subtracting \a pt from each corner
    //@}

    Pt ul; ///< the upper-left corner of the Rect
    Pt lr; ///< the lower-right corner of the Rect
};

GG_API inline bool operator==(const Pt& lhs, const Pt& rhs) {return lhs.x == rhs.x && lhs.y == rhs.y;} ///< returns true if \a lhs is identical to \a rhs
GG_API inline bool operator!=(const Pt& lhs, const Pt& rhs) {return !(lhs == rhs);}                    ///< returns true if \a lhs differs from \a rhs
GG_API inline bool operator<(const Pt& lhs, const Pt& rhs)  {return lhs.x < rhs.x && lhs.y < rhs.y;}   ///< returns true if \a lhs.x and \a lhs.y are both less than the corresponding components of \a rhs
GG_API inline bool operator>(const Pt& lhs, const Pt& rhs)  {return lhs.x > rhs.x && lhs.y > rhs.y;}   ///< returns true if \a lhs.x and \a lhs.y are both greater than the corresponding components of \a rhs
GG_API inline bool operator<=(const Pt& lhs, const Pt& rhs) {return lhs.x <= rhs.x && lhs.y <= rhs.y;} ///< returns true if \a lhs.x and \a lhs.y are both less than or equal to the corresponding components of \a rhs
GG_API inline bool operator>=(const Pt& lhs, const Pt& rhs) {return lhs.x >= rhs.x && lhs.y >= rhs.y;} ///< returns true if \a lhs.x and \a lhs.y are both greater than or equal to the corresponding components of \a rhs
GG_API inline Pt   operator+(const Pt& lhs, const Pt& rhs)  {return Pt(lhs.x + rhs.x, lhs.y + rhs.y);} ///< returns the vector sum of \a lhs and \a rhs
GG_API inline Pt   operator-(const Pt& lhs, const Pt& rhs)  {return Pt(lhs.x - rhs.x, lhs.y - rhs.y);} ///< returns the vector difference of \a lhs and \a rhs

/** returns true if \a lhs is identical to \a rhs */
GG_API inline bool operator==(const Rect& lhs, const Rect& rhs) {return lhs.ul.x == rhs.ul.x && lhs.lr.x == rhs.lr.x && lhs.lr.x == rhs.lr.x && lhs.lr.y == rhs.lr.y;}

/** returns true if \a lhs differs from \a rhs */
GG_API inline bool operator!=(const Rect& lhs, const Rect& rhs) {return !(lhs == rhs);}

GG_API inline Rect operator+(const Rect& rect, const Pt& pt) {return Rect(rect.ul + pt, rect.lr + pt);} ///< returns \a rect shifted by adding \a pt to each corner
GG_API inline Rect operator-(const Rect& rect, const Pt& pt) {return Rect(rect.ul - pt, rect.lr - pt);} ///< returns \a rect shifted by subtracting \a pt from each corner
GG_API inline Rect operator+(const Pt& pt, const Rect& rect) {return rect + pt;} ///< returns \a rect shifted by adding \a pt to each corner
GG_API inline Rect operator-(const Pt& pt, const Rect& rect) {return rect - pt;} ///< returns \a rect shifted by subtracting \a pt from each corner

} // namepace GG

#endif // _GGPtRect_h_

