// -*- C++ -*-
#ifndef _OgreSDLGUI_h_
#define _OgreSDLGUI_h_

#include <OgreFrameListener.h>
#include <OgreRenderTargetListener.h>

#include <GG/SDL/SDLGUI.h>


namespace Ogre {
    class RenderSystem;
    class RenderWindow;
}

class OgreSDLGUI :
    public SDLGUI,
    public Ogre::FrameListener,
    public Ogre::RenderTargetListener
{
public:
    OgreSDLGUI(Ogre::RenderWindow* window);

    virtual boost::shared_ptr<GG::ModalEventPump> CreateModalEventPump(bool& done);

    virtual void Exit(int code);
    virtual void Enter2DMode();
    virtual void Exit2DMode();
    virtual void Render();

    virtual bool frameStarted(const Ogre::FrameEvent& event);
    virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent& event);
    virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& event);
    virtual void postViewportUpdate(const Ogre::RenderTargetViewportEvent& event);

    void operator()();

protected:
    virtual void RenderBegin();
    virtual void RenderEnd();

private:
    virtual void SDLInit();
    virtual void GLInit();
    virtual void Initialize();
    virtual void HandleNonGGEvent(const SDL_Event& event);
    virtual void FinalCleanup();
    virtual void SDLQuit();
    virtual void Run();

    Ogre::RenderSystem* m_render_system;
    bool m_exit;
};

#endif // _OgreSDLGUI_h_
