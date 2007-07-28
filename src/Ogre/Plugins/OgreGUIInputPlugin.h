// -*- C++ -*-
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
   whatwasthataddress@hotmail.com */
   
#ifndef _GG_OgreGUIInputPlugin_h_ 
#define _GG_OgreGUIInputPlugin_h_

#include <OgrePlugin.h>

#include <boost/signals.hpp>


namespace Ogre { class RenderWindow; }

namespace GG {

class OgreGUIInputPlugin :
    public Ogre::Plugin
{
public:
    OgreGUIInputPlugin();
    ~OgreGUIInputPlugin();

    /// for overriding use of autocreated Root window
    static void SetRenderWindow(Ogre::RenderWindow* window);
    static Ogre::RenderWindow* GetRenderWindow();

protected:
    void ConnectHandlers();
    void DisconnectHandlers();

private:
    virtual void HandleSystemEvents() = 0;
    virtual void HandleWindowResize(int width, int height);
    virtual void HandleWindowClose();

    boost::signals::connection m_handle_events_connection;
    boost::signals::connection m_resize_connection;
    boost::signals::connection m_close_connection;

    static Ogre::RenderWindow* s_render_window;
};

} // namespace GG

#endif // _GG_OgreGUIInputPlugin_h_
