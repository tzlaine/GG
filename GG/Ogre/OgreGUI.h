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

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <OgreRenderTargetListener.h>
#include <OgreTimer.h>
#include <OgreWindowEventUtilities.h>

#include <GG/GUI.h>


namespace Ogre { class RenderWindow; }
namespace OIS { class InputManager; }

namespace GG {

class OgreGUI :
    public GUI,
    public Ogre::RenderTargetListener,
    public Ogre::WindowEventListener,
    public OIS::MouseListener,
    public OIS::KeyListener
{
public:
    OgreGUI(Ogre::RenderWindow* window);
    ~OgreGUI();

    virtual boost::shared_ptr<ModalEventPump> CreateModalEventPump(bool& done);

    virtual int  Ticks() const;
    virtual int  AppWidth() const;
    virtual int  AppHeight() const;

    virtual void Exit(int code);
    virtual void HandleSystemEvents();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void RenderBegin();
    virtual void RenderEnd();
    virtual void Run();

private:
    struct CleanQuit {};

    virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& event);
    virtual void windowResized(Ogre::RenderWindow* window);
    virtual void windowClosed(Ogre::RenderWindow* window);

    virtual bool mouseMoved(const OIS::MouseEvent &event);
    virtual bool mousePressed(const OIS::MouseEvent &event, OIS::MouseButtonID id);
    virtual bool mouseReleased(const OIS::MouseEvent &event, OIS::MouseButtonID id);

    virtual bool keyPressed(const OIS::KeyEvent& event);
    virtual bool keyReleased(const OIS::KeyEvent& event);

    void CleanupInputManager();

    Ogre::RenderWindow* m_window;
    mutable Ogre::Timer m_timer;
	OIS::InputManager*  m_input_manager;
	OIS::Mouse*         m_mouse;
	OIS::Keyboard*      m_keyboard;
};

} // namespace GG

#endif // _GG_OgreGUI_h_
