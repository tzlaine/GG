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

#include "GGControl.h"

#include <GGWndEditor.h>

using namespace GG;

////////////////////////////////////////////////
// GG::Control
////////////////////////////////////////////////
Control::Control() :
    Wnd ()
{
}

Control::Control(int x, int y, int w, int h, Uint32 flags/* = CLICKABLE*/) :
    Wnd(x, y, w, h, flags),
    m_disabled(false)
{
}

Clr Control::Color() const
{
    return m_color;
}

bool Control::Disabled() const
{
    return m_disabled;
}

void Control::MouseWheel(const Pt& pt, int move, Uint32 keys)
{
    if (Parent())
        Parent()->MouseWheel(pt, move, keys);
}

void Control::Keypress(Key key, Uint32 key_mods)
{
    if (Parent())
        Parent()->Keypress(key, key_mods);
}

void Control::SetColor(Clr c)
{
    m_color = c;
}

void Control::Disable(bool b/* = true*/)
{
    m_disabled = b;
}

void Control::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Wnd::DefineAttributes(editor);
    editor->Label("Control");
    editor->Attribute("Color", m_color);
    editor->Attribute("Disabled", m_disabled);
}
