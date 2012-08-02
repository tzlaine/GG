// -*- C++ -*-
#ifndef _OgreBackend_h_
#define _OgreBackend_h_

#include <GG/Ogre/OgreGUI.h>

#ifdef OGRE_STATIC_LIB
#include "OgrePlugins/OgreCgPlugin.h"
#include "OgrePlugins/OgreOctreePlugin.h"
#include "OgrePlugins/OgreParticleFXPlugin.h"
#include "OgrePlugins/OgreGLPlugin.h"
#include <GG/Ogre/Plugins/OISInput.h>
#elif defined(__APPLE__)
#include <GG/Ogre/Plugins/OISInput.h>
#endif

#include <OgreCamera.h>
#include <OgreLogManager.h>
#include <OgreRenderSystem.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreViewport.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>


#ifndef OGRE_STATIC_LIB
#  ifdef FREEORION_WIN32
const std::string OGRE_INPUT_PLUGIN_NAME("GiGiOgrePlugin_OIS.dll");
#  else
const std::string OGRE_INPUT_PLUGIN_NAME("libGiGiOgrePlugin_OIS.so");
#  endif
#endif


class MinimalOgreGUI :
    public GG::OgreGUI
{
public:
    MinimalOgreGUI(Ogre::Root* root,
                   Ogre::RenderWindow* window,
                   const std::string& ois_input_cfg_file_name) :
        OgreGUI(window, ois_input_cfg_file_name),
        m_root(root)
        { WindowClosedSignal.connect(boost::bind(&MinimalOgreGUI::Exit, this, 0)); }

    virtual void Enter2DMode()
        {
            Ogre::RenderWindow* window = m_root->getAutoCreatedWindow();
            unsigned int width, height, c;
            int left, top;
            window->getMetrics(width, height, c, left, top);

            OgreGUI::Enter2DMode();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);

            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHT3);
            glDisable(GL_LIGHT4);
            glDisable(GL_LIGHT5);
            glDisable(GL_LIGHT6);
            glDisable(GL_LIGHT7);

            float ambient_light[] = {0.2f, 0.2f, 0.2f, 1.0f};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glViewport(0, 0, width, height);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            // set up coordinates with origin in upper-left and +x and +y directions right and down, respectively
            // the depth of the viewing volume is only 1 (from 0.0 to 1.0)
            glOrtho(0.0, width, height, 0.0, 0.0, width);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    virtual void Render()
        {
            if (CustomRender)
                CustomRender();
            GUI::Render();
        }

    static boost::function<void ()> CustomInit;
    static boost::function<void ()> CustomRender;

private:
    virtual void Initialize()
        {
            RenderCursor(true);
            if (CustomInit)
                CustomInit();
        }

    Ogre::Root* m_root;
};

boost::function<void ()> MinimalOgreGUI::CustomInit;
boost::function<void ()> MinimalOgreGUI::CustomRender;

int MinimalOgreMain()
{
    Ogre::LogManager*       log_manager = 0;
    Ogre::Root*             root = 0;
#ifdef __APPLE__
    OISInput*               ois_input_plugin = 0;
#elif defined(OGRE_STATIC_LIB)
    OISInput*               ois_input_plugin = 0;
    Ogre::OctreePlugin*     octree_plugin = 0;
    Ogre::GLPlugin*         gl_plugin = 0;
#endif

    try {
        using namespace Ogre;

        log_manager = new LogManager();
        log_manager->createLog("ogre.log", true, false);

        root = new Root("ogre_plugins.cfg");

#if defined(OGRE_STATIC_LIB)
        octree_plugin = new Ogre::OctreePlugin;
        gl_plugin = new Ogre::GLPlugin;
        root->installPlugin(octree_plugin);
        root->installPlugin(gl_plugin);
#endif

        RenderSystem* selected_render_system = root->getRenderSystemByName("OpenGL Rendering Subsystem");
        if (selected_render_system == 0)
            throw std::runtime_error("Failed to find an Ogre GL render system.");

        root->setRenderSystem(selected_render_system);

        int color_depth = 32;
        bool fullscreen = false;
        int width(1024), height(768);

        selected_render_system->setConfigOption("Full Screen", fullscreen ? "Yes" : "No");
        std::string video_mode_str =
            boost::io::str(boost::format("%1% x %2% @ %3%-bit colour") %
                           width %
                           height %
                           color_depth);
        selected_render_system->setConfigOption("Video Mode", video_mode_str);

        RenderWindow* window = root->initialise(true, "GiGi (Ogre backend)");

        SceneManager* scene_manager = root->createSceneManager("OctreeSceneManager", "SceneMgr");

        Camera* camera = scene_manager->createCamera("Camera");
        camera->setPosition(Vector3(0, 0, 500));    // Position it at 500 in Z direction
        camera->lookAt(Vector3(0, 0, -300));        // Look back along -Z
        camera->setNearClipDistance(5);

        Viewport* viewport = window->addViewport(camera);
        viewport->setBackgroundColour(ColourValue(0, 0, 0));

        MinimalOgreGUI app(root, window, "OISInput.cfg");

#ifdef __APPLE__
        ois_input_plugin = new OISInput;
        root->installPlugin(ois_input_plugin);
#elif defined(OGRE_STATIC_LIB)
        ois_input_plugin = new OISInput;
        root->installPlugin(ois_input_plugin);
#else
        root->loadPlugin(OGRE_INPUT_PLUGIN_NAME);
#endif

        // run rendering loop
        app();  // calls GUI::operator() which calls OgreGUI::Run() which starts rendering loop
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
    } catch (const  boost::io::format_error& e) {
        std::cerr << "main() caught exception(boost::io::format_error): " << e.what() << std::endl;
    } catch (const GG::ExceptionBase& e) {
        std::cerr << "main() caught exception(" << e.type() << "): " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "main() caught unknown exception." << std::endl;
    }

    if (root) {
#ifdef __APPLE__
        root->uninstallPlugin(ois_input_plugin);
        delete ois_input_plugin;
#elif defined(OGRE_STATIC_LIB)
        root->uninstallPlugin(ois_input_plugin);
        root->uninstallPlugin(octree_plugin);
        root->uninstallPlugin(gl_plugin);
        delete ois_input_plugin;
        delete octree_plugin;
        delete gl_plugin;
#else
        root->unloadPlugin(OGRE_INPUT_PLUGIN_NAME);
#endif
    }

    delete root;
    delete log_manager;

    return 0;
}

#endif
