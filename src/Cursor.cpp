/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2007 T. Zachary Laine

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
   
#include <GG/Cursor.h>

#include <GG/DrawUtil.h>
#include <GG/Texture.h>


using namespace GG;

namespace {
    const bool OUTLINE_CURSOR = false;
}

Cursor::Cursor()
{}

Cursor::~Cursor()
{}

TextureCursor::TextureCursor(const boost::shared_ptr<Texture>& texture,
                             const Pt& hotspot/* = Pt()*/) :
    m_subtexture(texture),
    m_hotspot(hotspot)
{
    m_hotspot.x = std::max(X0, std::min(m_hotspot.x, m_subtexture.Width() - 1));
    m_hotspot.y = std::max(Y0, std::min(m_hotspot.y, m_subtexture.Height() - 1));
}

TextureCursor::TextureCursor(const SubTexture& subtexture,
                             const Pt& hotspot/* = Pt()*/) :
    m_subtexture(subtexture),
    m_hotspot(hotspot)
{
    m_hotspot.x = std::max(X0, std::min(m_hotspot.x, m_subtexture.Width() - 1));
    m_hotspot.y = std::max(Y0, std::min(m_hotspot.y, m_subtexture.Height() - 1));
}

const SubTexture& TextureCursor::GetSubTexture() const
{ return m_subtexture; }

const Pt& TextureCursor::Hotspot() const
{ return m_hotspot; }

void TextureCursor::Render(const Pt& pt)
{
    assert(!m_subtexture.Empty());
    Pt ul = pt - m_hotspot;
    if (OUTLINE_CURSOR) {
        Pt lr = ul + Pt(m_subtexture.Width(), m_subtexture.Height());
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINE_LOOP);
        glColor3ub(255, 0, 0);
        glVertex(lr.x, ul.y);
        glVertex(ul.x, ul.y);
        glVertex(ul.x, lr.y);
        glVertex(lr.x, lr.y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
    glColor3ub(255, 255, 255);
    m_subtexture.OrthoBlit(ul);
}
