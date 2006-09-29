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

/** \file StaticGraphic.h
    Contains the StaticGraphic class, a fixed image control. */

#ifndef _GG_StaticGraphic_h_
#define _GG_StaticGraphic_h_

#ifndef _GG_Control_h_
#include <GG/Control.h>
#endif

#ifndef _GG_Texture_h_
#include <GG/Texture.h>
#endif

namespace GG {

/** This is a simple, non-interactive window that displays a GG::SubTexture.  Though the SubTexture displayed in a
    StaticGraphic is fixed, its size is not; the image can be scaled (proportionately or not) to fit in the
    StaticGraphic's window area. \see StaticGraphicStyle*/
class GG_API StaticGraphic : public Control
{
public:
    /** \name Structors */ //@{
    /** creates a StaticGraphic from a pre-existing Texture.
        \warning Calling code <b>must not</b> delete \a texture; \a texture becomes the property of a shared_ptr inside 
        a SubTexture. */
    StaticGraphic(int x, int y, int w, int h, const boost::shared_ptr<Texture>& texture, Uint32 style = 0, Uint32 flags = 0); ///< creates a StaticGraphic from a pre-existing Texture.
    StaticGraphic(int x, int y, int w, int h, const SubTexture& subtexture, Uint32 style = 0, Uint32 flags = 0); ///< creates a StaticGraphic from a pre-existing SubTexture.
    //@}

    /** \name Accessors */ //@{
    /** returns the style of the StaticGraphic \see StaticGraphicStyle */
    Uint32   Style() const;
    //@}

    /** \name Mutators */ //@{
    virtual void Render();

    /** sets the style flags, and perfroms sanity checking \see StaticGraphicStyle */
    void  SetStyle(Uint32 style);

    virtual void DefineAttributes(WndEditor* editor);
    //@}

protected:
    /** \name Structors */ //@{
    StaticGraphic(); ///< default ctor
    //@}

private:
    void     Init(const SubTexture& subtexture); ///< initializes a StaticGraphic from a SubTexture
    void     ValidateStyle();   ///< ensures that the style flags are consistent

    SubTexture  m_graphic;
    Uint32      m_style;        ///< position of texture wrt the window area

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::StaticGraphic::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_graphic)
        & BOOST_SERIALIZATION_NVP(m_style);

    if (Archive::is_loading::value)
        ValidateStyle();
}

#endif // _GG_StaticGraphic_h_
