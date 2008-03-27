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

/** \file Clr.h
    Contains the utility class Clr, which represents colors in GG. */

#ifndef _GG_Clr_h_
#define _GG_Clr_h_

#include <GG/Base.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>


namespace GG {

/** A simple 32-bit structure that can act as a packed 32-bit unsigned integer representation of a RGBA color, a vector
    of the four unsigned bytes that compose an RGBA color, or the individual unsigned bytes "a", "r", "g", and "b", each
    of which represents a color channel.  You should not use literals to initialize Color objects; depending on the
    endian-ness of the machine, 0x00FFFFFF would be transparent white (little-endian) or opaque yellow (big-endian).*/
struct GG_API Clr
{
    /** \name Structors */ ///@{
    Clr();                                                       ///< default ctor
    Clr(GLubyte r_, GLubyte g_, GLubyte b_, GLubyte a_);         ///< ctor that constructs a Clr from four ints that represent the color channels
    //@}

    GLubyte r;   ///< the red channel
    GLubyte g;   ///< the green channel
    GLubyte b;   ///< the blue channel
    GLubyte a;   ///< the alpha channel

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

GG_API Clr FloatClr(float r_, float g_, float b_, float a_); ///< named ctor that constructs a Clr from four floats that represent the color channels (each must be >= 0.0 and <= 1.0)

GG_API bool operator==(const Clr& rhs, const Clr& lhs); ///< returns true iff \a rhs and \a lhs are identical
GG_API bool operator!=(const Clr& rhs, const Clr& lhs); ///< returns true iff \a rhs and \a lhs are different

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

} // namespace GG

// template implementations
template <class Archive>
void GG::Clr::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(r)
        & BOOST_SERIALIZATION_NVP(g)
        & BOOST_SERIALIZATION_NVP(b)
        & BOOST_SERIALIZATION_NVP(a);
}

#endif // _GG_Clr_h_

