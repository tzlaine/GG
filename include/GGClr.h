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

#ifndef _GGClr_h_
#define _GGClr_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

/** A simple 32-bit structure that can act as a packed 32-bit unsigned integer representation of a RGBA color, a vector of the four
    unsigned bytes that compose an RGBA color, or the individual unsigned bytes "a", "r", "g", and "b", each of which represents a 
    color channel.  You should not use literals to initialize Color objects; depending on the endian-ness of the machine, 
    0x00FFFFFF would be transparent white (little-endian) or opaque yellow (big-endian).*/
struct GG_API Clr
{
    /** \name Structors */ //@{
    Clr();                                                    ///< default ctor
    Clr(Uint32 clr);                                          ///< ctor that constructs a Clr from a 32-bit value packed with the 4 color channels
    explicit Clr(int _r, int _g, int _b, int _a);             ///< ctor that constructs a Clr from four Uint8s that represent the color channels
    explicit Clr(double _r, double _g, double _b, double _a); ///< ctor that constructs a Clr from four doubles that represent the color channels (each must be >= 0.0 and <= 1.0)
    Clr(Uint8 arr[]);                                         ///< ctor that constructs a Clr from an array of at least four Uint8s that represent the color channels
    Clr(double arr[]);                                        ///< ctor that constructs a Clr from an array of at least four doubles that represent the color channels (each must be >= 0.0 and <= 1.0)
    Clr(const XMLElement& elem); ///< ctor that creates a Clr as defined in XMLElement elem. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Clr object
    //@}
   
    /** \name Accessors */ //@{
    bool operator==(const Clr& rhs) {return i == rhs.i;} ///< returns true iff *this and rhs are identical

    XMLElement XMLEncode() const; ///< returns an XMLElement that encodes this Clr
    XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this Clr
    //@}
   
    union {
        Uint32 i;     ///< represents Clr as a packed RGBA color
        Uint8  v[4];  ///< represents Clr as a vector of RGBA components
        struct {
            Uint8 r;   ///< the red channel
            Uint8 g;   ///< the green channel
            Uint8 b;   ///< the blue channel
            Uint8 a;   ///< the alpha channel
        };
    };
};

// some useful color constants
extern GG_API const Clr CLR_ZERO;
extern GG_API const Clr CLR_BLACK;
extern GG_API const Clr CLR_WHITE;
extern GG_API const Clr CLR_GRAY;
extern GG_API const Clr CLR_SHADOW;
extern GG_API const Clr CLR_RED;
extern GG_API const Clr CLR_GREEN;
extern GG_API const Clr CLR_BLUE;
extern GG_API const Clr CLR_CYAN;
extern GG_API const Clr CLR_YELLOW;
extern GG_API const Clr CLR_MAGENTA;

} // namepace GG

#endif // _GGClr_h_

