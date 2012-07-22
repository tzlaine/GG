// -*- C++ -*-
#ifndef _SDLBackend_h_
#define _SDLBackend_h_

#include <GG/SDL/SDLGUI.h>


class MinimalSDLGUI : public GG::SDLGUI
{
public:
    MinimalSDLGUI() : 
        SDLGUI(1024, 768, false, "GiGi (SDL backend)")
        {}

    virtual void Enter2DMode()
        {
            glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();

            glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
        }
    virtual void Exit2DMode()
        {
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glPopAttrib();
        }

    static boost::function<void ()> CustomInit;

private:
    virtual void GLInit()
        {
            double ratio = Value(AppWidth() * 1.0) / Value(AppHeight());

            glEnable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0, 0, 0, 0);
            glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(50.0, ratio, 1.0, 10.0);
            gluLookAt(0.0, 0.0, 5.0,
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            glMatrixMode(GL_MODELVIEW);
        }

    virtual void Initialize()
        {
            RenderCursor(true);
            if (CustomInit)
                CustomInit();
        }
};

boost::function<void ()> MinimalSDLGUI::CustomInit;

int MinimalSDLMain()
{
    MinimalSDLGUI gui;
    gui();
    return 0;
}

#endif
