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

#ifndef _GGStaticGraphic_h_
#define _GGStaticGraphic_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

#ifndef _GGTexture_h_
#include "GGTexture.h"
#endif

namespace GG {

/** This is a simple, non-interactive window that displays a GG::SubTexture.  Though the SubTexture displayed in a 
    StaticGraphic is fixed, its size is not; the image can be scaled (proportionately or not) to fit in the StaticGraphic's
    window area. \see StaticGraphicStyle*/
class StaticGraphic : public Control
{
public:
    /** \name Structors */ //@{
    /** creates a StaticGraphic from a pre-existing Texture.
        \warning Calling code <b>must not</b> delete \a texture; \a texture becomes the property of a shared_ptr inside 
        a SubTexture. */
    StaticGraphic(int x, int y, int w, int h, const Texture* texture, Uint32 style = 0, Uint32 flags = 0);
    StaticGraphic(int x, int y, int w, int h, const shared_ptr<Texture>& texture, Uint32 style = 0, Uint32 flags = 0); ///< creates a StaticGraphic from a pre-existing Texture.
    StaticGraphic(int x, int y, int w, int h, const SubTexture& subtexture, Uint32 style = 0, Uint32 flags = 0); ///< creates a StaticGraphic from a pre-existing SubTexture.
    StaticGraphic(const XMLElement& elem); ///< ctor that constructs a StaticGraphic object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a StaticGraphic object
    //@}

    /** \name Accessors */ //@{
    Uint32   Style() const  {return m_style;}   ///< returns the style of the StaticGraphic \see StaticGraphicStyle
    //@}
   
    /** \name Mutators */ //@{
    virtual  int Render();

    void     SetStyle(Uint32 style)  {m_style = style; ValidateStyle();} ///< sets the style flags, and perfroms sanity checking \see StaticGraphicStyle

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a StaticGraphic object

    virtual XMLElementValidator XMLValidator() const; ///< creates a Validator object that can validate changes in the XML representation of this object
    //@}

private:
    void     Init(const SubTexture& subtexture); ///< initializes a StaticGraphic from a SubTexture
    void     ValidateStyle();   ///< ensures that the style flags are consistent

    SubTexture  m_graphic;
    Uint32      m_style;        ///< position of texture wrt the window area
};

} // namespace GG

#endif // _GGStaticGraphic_h_

