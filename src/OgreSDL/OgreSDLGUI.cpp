#include "OgreSDLGUI.h"

#include <OgreRenderSystem.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>

#include <GG/EventPump.h>


namespace {
    class OgreModalEventPump : public GG::ModalEventPump
    {
    public:
        OgreModalEventPump(const bool& done) : ModalEventPump(done) {}
        virtual void operator()()
        {
            GG::GUI* gui = GG::GUI::GetGUI();
            GG::EventPumpState& state = State();
            Ogre::Root& root = Ogre::Root::getSingleton();
            while (!m_done) {
                if (!root._fireFrameStarted())
                    break;
                root._updateAllRenderTargets();
                if (!root._fireFrameEnded())
                    break;
                LoopBody(gui, state, true, true);
            }
        }
    };
}

OgreSDLGUI::OgreSDLGUI(Ogre::RenderWindow* window) :
    SDLGUI(window->getWidth(), window->getHeight(), false, ""),
    m_render_system(Ogre::Root::getSingleton().getRenderSystem()),
    m_exit(false)
{
    Ogre::Root::getSingleton().addFrameListener(this);
    window->addListener(this);
}

boost::shared_ptr<GG::ModalEventPump> OgreSDLGUI::CreateModalEventPump(bool& done)
{
    return boost::shared_ptr<GG::ModalEventPump>(new OgreModalEventPump(done));
}

void OgreSDLGUI::Exit(int code)
{
    m_exit = true;
}

void OgreSDLGUI::Enter2DMode()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	using namespace Ogre;

    // TODO: These calls are designed to set up OpenGL in a GG-friendly state.  This code works for Ogre 1.2-RC1 on
    // Linux, but not Windows.  It has not been tried with other versions of Ogre.  Therefore, users may need to set up
    // the state themselves, perhaps through direct OpenGL calls, with state pushed/popped to preserve it.

    // set-up matrices
    m_render_system->_setWorldMatrix(Matrix4::IDENTITY);
    m_render_system->_setViewMatrix(Matrix4::IDENTITY);
    m_render_system->_setProjectionMatrix(Matrix4::IDENTITY);
    glOrtho(0.0, AppWidth(), AppHeight(), 0.0, 0.0, AppWidth());

    // initialise render settings
    m_render_system->setLightingEnabled(false);
    m_render_system->_setDepthBufferParams(false, false);
    m_render_system->_setCullingMode(CULL_NONE);
    m_render_system->_setFog(FOG_NONE);
    m_render_system->_setColourBufferWriteEnabled(true, true, true, true);
    m_render_system->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
    m_render_system->unbindGpuProgram(GPT_VERTEX_PROGRAM);
    m_render_system->setShadingType(SO_GOURAUD);
    m_render_system->_setPolygonMode(PM_SOLID);

    Ogre::LayerBlendModeEx colour_blend_mode;
    colour_blend_mode.blendType = Ogre::LBT_COLOUR;
    colour_blend_mode.source1 = Ogre::LBS_TEXTURE;
    colour_blend_mode.source2 = Ogre::LBS_DIFFUSE;
    colour_blend_mode.operation = Ogre::LBX_MODULATE;
    Ogre::LayerBlendModeEx alpha_blend_mode;
    alpha_blend_mode.blendType = Ogre::LBT_ALPHA;
    alpha_blend_mode.source1 = Ogre::LBS_TEXTURE;
    alpha_blend_mode.source2 = Ogre::LBS_DIFFUSE;
    alpha_blend_mode.operation = Ogre::LBX_MODULATE;
    Ogre::TextureUnitState::UVWAddressingMode uvw_address_mode;
    uvw_address_mode.u = Ogre::TextureUnitState::TAM_CLAMP;
    uvw_address_mode.v = Ogre::TextureUnitState::TAM_CLAMP;
    uvw_address_mode.w = Ogre::TextureUnitState::TAM_CLAMP;

    // initialise texture settings
    m_render_system->_setTextureCoordCalculation(0, TEXCALC_NONE);
    m_render_system->_setTextureCoordSet(0, 0);
    m_render_system->_setTextureUnitFiltering(0, FO_LINEAR, FO_LINEAR, FO_POINT);
    m_render_system->_setTextureAddressingMode(0, uvw_address_mode);
    m_render_system->_setTextureMatrix(0, Matrix4::IDENTITY);
    m_render_system->_setAlphaRejectSettings(CMPF_ALWAYS_PASS, 0);
    m_render_system->_setTextureBlendMode(0, colour_blend_mode);
    m_render_system->_setTextureBlendMode(0, alpha_blend_mode);
    m_render_system->_disableTextureUnitsFrom(1);

    // enable alpha blending
    m_render_system->_setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);

#if 0
    typedef void (*BindBufferARBFn)(GLenum, GLuint);
    BindBufferARBFn glBindBufferARB = (BindBufferARBFn)SDL_GL_GetProcAddress("glBindBufferARB");
    if (glBindBufferARB) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
#endif
}

void OgreSDLGUI::Exit2DMode()
{
    glPopAttrib();
}

void OgreSDLGUI::Render()
{
    GUI::Render();
}

bool OgreSDLGUI::frameStarted(const Ogre::FrameEvent& ogre_event)
{
    if (m_exit)
        return false;
    HandleSystemEvents();
    return !m_exit;
}

void OgreSDLGUI::preRenderTargetUpdate(const Ogre::RenderTargetEvent& event)
{}

void OgreSDLGUI::postRenderTargetUpdate(const Ogre::RenderTargetEvent& event)
{
    Render();
}

void OgreSDLGUI::postViewportUpdate(const Ogre::RenderTargetViewportEvent& event)
{
    int w = event.source->getActualWidth();
    int h = event.source->getActualHeight();
    if (w != AppWidth() || h != AppHeight())
        SetAppSize(GG::Pt(w, h));
}

void OgreSDLGUI::operator()()
{
    Run();
}

void OgreSDLGUI::RenderBegin()
{}

void OgreSDLGUI::RenderEnd()
{}

void OgreSDLGUI::SDLInit()
{}

void OgreSDLGUI::GLInit()
{}

void OgreSDLGUI::Initialize()
{
#if defined(GG_USE_NET) && GG_USE_NET
    if (SDLNet_Init() < 0) {
        std::cerr << "SDL Net initialization failed: " << SDLNet_GetError();
        Exit(1);
    }

    if (FE_Init() < 0) {
        std::cerr << "FastEvents initialization failed: " << FE_GetError();
        Exit(1);
    }

    if (NET2_Init() < 0) {
        std::cerr << "SDL Net2 initialization failed: " << NET2_GetError();
        Exit(1);
    }
#endif // GG_USE_NET
}

void OgreSDLGUI::HandleNonGGEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_QUIT:
        m_exit = true;
        break;
    }
}

void OgreSDLGUI::FinalCleanup()
{}

void OgreSDLGUI::SDLQuit()
{
#if defined(GG_USE_NET) && GG_USE_NET
    NET2_Quit();
    FE_Quit();
    SDLNet_Quit();
#endif // GG_USE_NET
}

void OgreSDLGUI::Run()
{
    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::RenderSystem* active_renderer = root.getRenderSystem();
    assert(active_renderer);
    root.clearEventTimes();
    while (1) {
        if (!root._fireFrameStarted())
            break;
        root._updateAllRenderTargets();
        if (!root._fireFrameEnded())
            break;
    }
}
