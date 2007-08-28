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
   
#ifndef _GG_OgreGUI_h_ 
#define _GG_OgreGUI_h_

#include <OgreDataStream.h>
#include <OgreRenderTargetListener.h>
#include <OgreSharedPtr.h>
#include <OgreTimer.h>
#include <OgreWindowEventUtilities.h>

#include <GG/GUI.h>


#ifdef _MSC_VER
# ifdef GIGI_OGRE_EXPORTS
#  define GG_OGRE_API __declspec(dllexport)
# else
#  define GG_OGRE_API __declspec(dllimport)
# endif
#else
# define GG_OGRE_API
#endif

namespace Ogre {
    class RenderWindow;
}

namespace GG {

class GG_OGRE_API OgreGUI :
    public GUI,
    public Ogre::RenderTargetListener,
    public Ogre::WindowEventListener
{
public:
    OgreGUI(Ogre::RenderWindow* window, const std::string& config_filename = "");
    ~OgreGUI();

    virtual boost::shared_ptr<ModalEventPump> CreateModalEventPump(bool& done);

    virtual int  Ticks() const;
    virtual int  AppWidth() const;
    virtual int  AppHeight() const;

    const Ogre::SharedPtr<Ogre::DataStream>& ConfigFileStream() const;

    virtual void Exit(int code);

    boost::signal<void ()> HandleSystemEventsSignal;
    boost::signal<void (int, int)> WindowResizedSignal;
    boost::signal<void ()> WindowClosedSignal;

    /** allows any code to access the gui framework by calling OgreGUI::GetGUI() */
    static OgreGUI* GetGUI();

protected:
    virtual void RenderBegin();
    virtual void RenderEnd();
    virtual void Run();
    virtual void HandleSystemEvents();
    virtual void Enter2DMode();
    virtual void Exit2DMode();

private:
    virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& event);
    virtual void windowResized(Ogre::RenderWindow* window);
    virtual void windowClosed(Ogre::RenderWindow* window);

    Ogre::RenderWindow*               m_window;
    mutable Ogre::Timer               m_timer;
    Ogre::SharedPtr<Ogre::DataStream> m_config_file_data;
};

} // namespace GG

#endif // _GG_OgreGUI_h_
