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

#include "OgreGUI.h"

#include <GG/EventPump.h>

#include <OgreRenderSystem.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>

#include <OIS/OIS.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <GL/gl.h>
#include "glext.h"
#else
#include <GL/glx.h>
#endif

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#include <cctype>

#include <iostream> // TODO: remove -- temporary use only


using namespace GG;

namespace {
    class OgreModalEventPump : public ModalEventPump
    {
    public:
        OgreModalEventPump(const bool& done) : ModalEventPump(done) {}
        virtual void operator()()
            {
                GUI* gui = GUI::GetGUI();
                EventPumpState& state = State();
                Ogre::Root& root = Ogre::Root::getSingleton();
                while (!m_done) {
                    Ogre::WindowEventUtilities::messagePump();
                    LoopBody(gui, state, true, false);
                    gui->HandleSystemEvents();
                    if (!root.renderOneFrame())
                        break;
                }
            }
    };

    Uint32 GetKeyMods(OIS::Keyboard* keyboard)
    {
        Uint32 retval = 0;
        if (keyboard->isKeyDown(OIS::KC_LSHIFT))   retval |= GGKMOD_LSHIFT;
        if (keyboard->isKeyDown(OIS::KC_RSHIFT))   retval |= GGKMOD_RSHIFT;
        if (keyboard->isKeyDown(OIS::KC_LCONTROL)) retval |= GGKMOD_LCTRL;
        if (keyboard->isKeyDown(OIS::KC_RCONTROL)) retval |= GGKMOD_RCTRL;
        if (keyboard->isKeyDown(OIS::KC_LMENU))    retval |= GGKMOD_LALT;
        if (keyboard->isKeyDown(OIS::KC_RMENU))    retval |= GGKMOD_RALT;
        if (keyboard->isKeyDown(OIS::KC_LWIN))     retval |= GGKMOD_LMETA;
        if (keyboard->isKeyDown(OIS::KC_RWIN))     retval |= GGKMOD_RMETA;
        if (keyboard->isKeyDown(OIS::KC_NUMLOCK))  retval |= GGKMOD_NUM;
        if (keyboard->isKeyDown(OIS::KC_CAPITAL))  retval |= GGKMOD_CAPS;
        return retval;
    }

    Key GGKeyFromOISKey(const OIS::KeyEvent& event, Uint32 mods,
                        OIS::Keyboard::TextTranslationMode translation_mode)
    {
        Key retval = GGK_UNKNOWN;

        switch (event.key) {
        case OIS::KC_UNASSIGNED:   retval = GGK_UNKNOWN; break;
        case OIS::KC_ESCAPE:       retval = GGK_ESCAPE; break;
        case OIS::KC_1:            retval = GGK_1; break;
        case OIS::KC_2:            retval = GGK_2; break;
        case OIS::KC_3:            retval = GGK_3; break;
        case OIS::KC_4:            retval = GGK_4; break;
        case OIS::KC_5:            retval = GGK_5; break;
        case OIS::KC_6:            retval = GGK_6; break;
        case OIS::KC_7:            retval = GGK_7; break;
        case OIS::KC_8:            retval = GGK_8; break;
        case OIS::KC_9:            retval = GGK_9; break;
        case OIS::KC_0:            retval = GGK_0; break;
        case OIS::KC_MINUS:        retval = GGK_MINUS; break;
        case OIS::KC_EQUALS:       retval = GGK_EQUALS; break;
        case OIS::KC_BACK:         retval = GGK_BACKSPACE; break;
        case OIS::KC_TAB:          retval = GGK_TAB; break;
        case OIS::KC_Q:            retval = GGK_q; break;
        case OIS::KC_W:            retval = GGK_w; break;
        case OIS::KC_E:            retval = GGK_e; break;
        case OIS::KC_R:            retval = GGK_r; break;
        case OIS::KC_T:            retval = GGK_t; break;
        case OIS::KC_Y:            retval = GGK_y; break;
        case OIS::KC_U:            retval = GGK_u; break;
        case OIS::KC_I:            retval = GGK_i; break;
        case OIS::KC_O:            retval = GGK_o; break;
        case OIS::KC_P:            retval = GGK_p; break;
        case OIS::KC_LBRACKET:     retval = GGK_LEFTBRACKET; break;
        case OIS::KC_RBRACKET:     retval = GGK_RIGHTBRACKET; break;
        case OIS::KC_RETURN:       retval = GGK_RETURN; break;
        case OIS::KC_LCONTROL:     retval = GGK_LCTRL; break;
        case OIS::KC_A:            retval = GGK_a; break;
        case OIS::KC_S:            retval = GGK_s; break;
        case OIS::KC_D:            retval = GGK_d; break;
        case OIS::KC_F:            retval = GGK_f; break;
        case OIS::KC_G:            retval = GGK_g; break;
        case OIS::KC_H:            retval = GGK_h; break;
        case OIS::KC_J:            retval = GGK_j; break;
        case OIS::KC_K:            retval = GGK_k; break;
        case OIS::KC_L:            retval = GGK_l; break;
        case OIS::KC_SEMICOLON:    retval = GGK_SEMICOLON; break;
        case OIS::KC_APOSTROPHE:   retval = GGK_QUOTE; break;
        case OIS::KC_GRAVE:        retval = GGK_BACKQUOTE; break;
        case OIS::KC_LSHIFT:       retval = GGK_LSHIFT; break;
        case OIS::KC_BACKSLASH:    retval = GGK_BACKSLASH; break;
        case OIS::KC_Z:            retval = GGK_z; break;
        case OIS::KC_X:            retval = GGK_x; break;
        case OIS::KC_C:            retval = GGK_c; break;
        case OIS::KC_V:            retval = GGK_v; break;
        case OIS::KC_B:            retval = GGK_b; break;
        case OIS::KC_N:            retval = GGK_n; break;
        case OIS::KC_M:            retval = GGK_m; break;
        case OIS::KC_COMMA:        retval = GGK_COMMA; break;
        case OIS::KC_PERIOD:       retval = GGK_PERIOD; break;
        case OIS::KC_SLASH:        retval = GGK_SLASH; break;
        case OIS::KC_RSHIFT:       retval = GGK_RSHIFT; break;
        case OIS::KC_MULTIPLY:     retval = GGK_KP_MULTIPLY; break;
        case OIS::KC_LMENU:        retval = GGK_LALT; break;
        case OIS::KC_SPACE:        retval = GGK_SPACE; break;
        case OIS::KC_CAPITAL:      retval = GGK_CAPSLOCK; break;
        case OIS::KC_F1:           retval = GGK_F1; break;
        case OIS::KC_F2:           retval = GGK_F2; break;
        case OIS::KC_F3:           retval = GGK_F3; break;
        case OIS::KC_F4:           retval = GGK_F4; break;
        case OIS::KC_F5:           retval = GGK_F5; break;
        case OIS::KC_F6:           retval = GGK_F6; break;
        case OIS::KC_F7:           retval = GGK_F7; break;
        case OIS::KC_F8:           retval = GGK_F8; break;
        case OIS::KC_F9:           retval = GGK_F9; break;
        case OIS::KC_F10:          retval = GGK_F10; break;
        case OIS::KC_NUMLOCK:      retval = GGK_NUMLOCK; break;
        case OIS::KC_SCROLL:       retval = GGK_SCROLLOCK; break;
        case OIS::KC_NUMPAD7:      retval = GGK_KP7; break;
        case OIS::KC_NUMPAD8:      retval = GGK_KP8; break;
        case OIS::KC_NUMPAD9:      retval = GGK_KP9; break;
        case OIS::KC_SUBTRACT:     retval = GGK_KP_MINUS; break;
        case OIS::KC_NUMPAD4:      retval = GGK_KP4; break;
        case OIS::KC_NUMPAD5:      retval = GGK_KP5; break;
        case OIS::KC_NUMPAD6:      retval = GGK_KP6; break;
        case OIS::KC_ADD:          retval = GGK_KP_PLUS; break;
        case OIS::KC_NUMPAD1:      retval = GGK_KP1; break;
        case OIS::KC_NUMPAD2:      retval = GGK_KP2; break;
        case OIS::KC_NUMPAD3:      retval = GGK_KP3; break;
        case OIS::KC_NUMPAD0:      retval = GGK_KP0; break;
        case OIS::KC_DECIMAL:      retval = GGK_KP_PERIOD; break;
        case OIS::KC_OEM_102:      retval = GGK_UNKNOWN; break;
        case OIS::KC_F11:          retval = GGK_F11; break;
        case OIS::KC_F12:          retval = GGK_F12; break;
        case OIS::KC_F13:          retval = GGK_F13; break;
        case OIS::KC_F14:          retval = GGK_F14; break;
        case OIS::KC_F15:          retval = GGK_F15; break;
        case OIS::KC_KANA:         retval = GGK_UNKNOWN; break;
        case OIS::KC_ABNT_C1:      retval = GGK_SLASH; break;
        case OIS::KC_CONVERT:      retval = GGK_UNKNOWN; break;
        case OIS::KC_NOCONVERT:    retval = GGK_UNKNOWN; break;
        case OIS::KC_YEN:          retval = GGK_UNKNOWN; break;
        case OIS::KC_ABNT_C2:      retval = GGK_KP_PERIOD; break;
        case OIS::KC_NUMPADEQUALS: retval = GGK_KP_EQUALS; break;
        case OIS::KC_PREVTRACK:    retval = GGK_UNKNOWN; break;
        case OIS::KC_AT:           retval = GGK_AT; break;
        case OIS::KC_COLON:        retval = GGK_COLON; break;
        case OIS::KC_UNDERLINE:    retval = GGK_UNDERSCORE; break;
        case OIS::KC_KANJI:        retval = GGK_UNKNOWN; break;
        case OIS::KC_STOP:         retval = GGK_UNKNOWN; break;
        case OIS::KC_AX:           retval = GGK_UNKNOWN; break;
        case OIS::KC_UNLABELED:    retval = GGK_UNKNOWN; break;
        case OIS::KC_NEXTTRACK:    retval = GGK_UNKNOWN; break;
        case OIS::KC_NUMPADENTER:  retval = GGK_KP_ENTER; break;
        case OIS::KC_RCONTROL:     retval = GGK_RCTRL; break;
        case OIS::KC_MUTE:         retval = GGK_UNKNOWN; break;
        case OIS::KC_CALCULATOR:   retval = GGK_UNKNOWN; break;
        case OIS::KC_PLAYPAUSE:    retval = GGK_UNKNOWN; break;
        case OIS::KC_MEDIASTOP:    retval = GGK_UNKNOWN; break;
        case OIS::KC_VOLUMEDOWN:   retval = GGK_UNKNOWN; break;
        case OIS::KC_VOLUMEUP:     retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBHOME:      retval = GGK_UNKNOWN; break;
        case OIS::KC_NUMPADCOMMA:  retval = GGK_KP_EQUALS; break;
        case OIS::KC_DIVIDE:       retval = GGK_KP_DIVIDE; break;
        case OIS::KC_SYSRQ:        retval = GGK_SYSREQ; break;
        case OIS::KC_RMENU:        retval = GGK_RALT; break;
        case OIS::KC_PAUSE:        retval = GGK_PAUSE; break;
        case OIS::KC_HOME:         retval = GGK_HOME; break;
        case OIS::KC_UP:           retval = GGK_UP; break;
        case OIS::KC_PGUP:         retval = GGK_PAGEUP; break;
        case OIS::KC_LEFT:         retval = GGK_LEFT; break;
        case OIS::KC_RIGHT:        retval = GGK_RIGHT; break;
        case OIS::KC_END:          retval = GGK_END; break;
        case OIS::KC_DOWN:         retval = GGK_DOWN; break;
        case OIS::KC_PGDOWN:       retval = GGK_PAGEDOWN; break;
        case OIS::KC_INSERT:       retval = GGK_INSERT; break;
        case OIS::KC_DELETE:       retval = GGK_DELETE; break;
        case OIS::KC_LWIN:         retval = GGK_LMETA; break;
        case OIS::KC_RWIN:         retval = GGK_RMETA; break;
        case OIS::KC_APPS:         retval = GGK_UNKNOWN; break;
        case OIS::KC_POWER:        retval = GGK_POWER; break;
        case OIS::KC_SLEEP:        retval = GGK_UNKNOWN; break;
        case OIS::KC_WAKE:         retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBSEARCH:    retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBFAVORITES: retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBREFRESH:   retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBSTOP:      retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBFORWARD:   retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBBACK:      retval = GGK_UNKNOWN; break;
        case OIS::KC_MYCOMPUTER:   retval = GGK_UNKNOWN; break;
        case OIS::KC_MAIL:         retval = GGK_UNKNOWN; break;
        case OIS::KC_MEDIASELECT:  retval = GGK_UNKNOWN; break;
        }

        // this code works because GG::Key maps (at least partially) to the
        // printable ASCII characters
        bool shift = mods & GGKMOD_SHIFT;
        bool caps_lock = mods & GGKMOD_CAPS;
        if (shift || caps_lock) {
            if (shift != caps_lock && ('a' <= retval && retval <= 'z')) {
                retval = Key(std::toupper(retval));
            } else if (shift) { // the caps lock key should not affect these
                // this assumes a US keyboard layout
                switch (retval) {
                case '`': retval = Key('~'); break;
                case '1': retval = Key('!'); break;
                case '2': retval = Key('@'); break;
                case '3': retval = Key('#'); break;
                case '4': retval = Key('$'); break;
                case '5': retval = Key('%'); break;
                case '6': retval = Key('^'); break;
                case '7': retval = Key('&'); break;
                case '8': retval = Key('*'); break;
                case '9': retval = Key('('); break;
                case '0': retval = Key(')'); break;
                case '-': retval = Key('_'); break;
                case '=': retval = Key('+'); break;
                case '[': retval = Key('{'); break;
                case ']': retval = Key('}'); break;
                case '\\': retval = Key('|'); break;
                case ';': retval = Key(':'); break;
                case '\'': retval = Key('"'); break;
                case ',': retval = Key('<'); break;
                case '.': retval = Key('>'); break;
                case '/': retval = Key('?'); break;
                default: break;
                }
            }
        }

        return retval;
    }
}

OgreGUI::OgreGUI(Ogre::RenderWindow* window) :
    GUI(""),
    m_window(window),
    m_timer(),
	m_input_manager(0),
	m_mouse(0),
	m_keyboard(0)
{
    window->addListener(this);

    OIS::ParamList param_list;
    std::size_t window_handle = 0;
    window->getCustomAttribute("WINDOW", &window_handle);
    param_list.insert(std::make_pair(std::string("WINDOW"),
                                     boost::lexical_cast<std::string>(window_handle)));
    m_input_manager = OIS::InputManager::createInputSystem(param_list);
    m_keyboard = boost::polymorphic_downcast<OIS::Keyboard*>(
        m_input_manager->createInputObject(OIS::OISKeyboard, true));
    m_keyboard->setEventCallback(this);
    m_mouse = boost::polymorphic_downcast<OIS::Mouse*>(
        m_input_manager->createInputObject(OIS::OISMouse, true));
    m_mouse->setEventCallback(this);

    Ogre::WindowEventUtilities::addWindowEventListener(m_window, this);
}

OgreGUI::~OgreGUI()
{
    Ogre::WindowEventUtilities::removeWindowEventListener(m_window, this);
    CleanupInputManager();
}

boost::shared_ptr<ModalEventPump> OgreGUI::CreateModalEventPump(bool& done)
{ return boost::shared_ptr<ModalEventPump>(new OgreModalEventPump(done)); }

int OgreGUI::Ticks() const
{ return m_timer.getMilliseconds(); }

int OgreGUI::AppWidth() const
{ return m_window->getWidth(); }

int OgreGUI::AppHeight() const
{ return m_window->getHeight(); }

void OgreGUI::Exit(int code)
{
    if (code == 0)
        throw CleanQuit();
    else
        std::exit(code);
}

void OgreGUI::HandleSystemEvents()
{
    assert(m_mouse->buffered());
    assert(m_keyboard->buffered());
    m_mouse->capture();
    m_keyboard->capture();
}

void OgreGUI::Enter2DMode()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

	using namespace Ogre;

    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();

    // TODO: These calls are designed to set up OpenGL in a GG-friendly state.  This code works for Ogre 1.2-RC1 on
    // Linux, but not Windows.  It has not been tried with other versions of Ogre.  Therefore, users may need to set up
    // the state themselves, perhaps through direct OpenGL calls, with state pushed/popped to preserve it.

    // set-up matrices
    render_system->_setWorldMatrix(Matrix4::IDENTITY);
    render_system->_setViewMatrix(Matrix4::IDENTITY);
    render_system->_setProjectionMatrix(Matrix4::IDENTITY);

    glOrtho(0.0, AppWidth(), AppHeight(), 0.0, 0.0, AppWidth());

    // initialise render settings
    render_system->setLightingEnabled(false);
    render_system->_setDepthBufferParams(false, false);
    render_system->_setCullingMode(CULL_NONE);
    render_system->_setFog(FOG_NONE);
    render_system->_setColourBufferWriteEnabled(true, true, true, true);
    render_system->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
    render_system->unbindGpuProgram(GPT_VERTEX_PROGRAM);
    render_system->setShadingType(SO_GOURAUD);
    render_system->_setPolygonMode(PM_SOLID);

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
    render_system->_setTextureCoordCalculation(0, TEXCALC_NONE);
    render_system->_setTextureCoordSet(0, 0);
    render_system->_setTextureUnitFiltering(0, FO_LINEAR, FO_LINEAR, FO_POINT);
    render_system->_setTextureAddressingMode(0, uvw_address_mode);
    render_system->_setTextureMatrix(0, Matrix4::IDENTITY);
    render_system->_setAlphaRejectSettings(CMPF_ALWAYS_PASS, 0);
    render_system->_setTextureBlendMode(0, colour_blend_mode);
    render_system->_setTextureBlendMode(0, alpha_blend_mode);
    render_system->_disableTextureUnitsFrom(1);

    // enable alpha blending
    render_system->_setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);

    typedef void (*BindBufferARBFn)(GLenum, GLuint);
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    BindBufferARBFn glBindBufferARB = (BindBufferARBFn)wglGetProcAddress("glBindBufferARB");
#else
    BindBufferARBFn glBindBufferARB = (BindBufferARBFn)glXGetProcAddress((const GLubyte* )"glBindBufferARB");
#endif
    if (glBindBufferARB) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}

void OgreGUI::Exit2DMode()
{ glPopAttrib(); }

void OgreGUI::RenderBegin() {}
void OgreGUI::RenderEnd() {}

void OgreGUI::Run()
{
    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::RenderSystem* active_renderer = root.getRenderSystem();
    assert(active_renderer);
    active_renderer->_initRenderTargets();
    root.clearEventTimes();
    try {
        while (1) {
            Ogre::WindowEventUtilities::messagePump();
            HandleSystemEvents();
            if (!root.renderOneFrame())
                break;
        }
    } catch (const CleanQuit&) {}
}

void OgreGUI::postRenderTargetUpdate(const Ogre::RenderTargetEvent& event)
{ Render(); }

void OgreGUI::windowResized(Ogre::RenderWindow* window)
{
    if (window == m_window) {
        unsigned int width, height, depth;
        int left, top;
        window->getMetrics(width, height, depth, left, top);

        const OIS::MouseState& mouse_state = m_mouse->getMouseState();
        mouse_state.width = width;
        mouse_state.height = height;
    }
}

void OgreGUI::windowClosed(Ogre::RenderWindow* window)
{
    if (window == m_window) {
        CleanupInputManager();
        Exit(0);
    }
}

bool OgreGUI::mouseMoved(const OIS::MouseEvent &event)
{
    Pt mouse_pos(event.state.X.abs, event.state.Y.abs);
    Pt mouse_rel(event.state.X.rel, event.state.Y.rel);
    HandleGGEvent(MOUSEMOVE, GGK_UNKNOWN, GetKeyMods(m_keyboard), mouse_pos, mouse_rel);
    return true;
}

bool OgreGUI::mousePressed(const OIS::MouseEvent &event, OIS::MouseButtonID id)
{
    Pt mouse_pos(event.state.X.abs, event.state.Y.abs);
    EventType gg_event = IDLE;
    switch (id) {
    case OIS::MB_Left:   gg_event = LPRESS; break;
    case OIS::MB_Right:  gg_event = RPRESS; break;
    case OIS::MB_Middle: gg_event = MPRESS; break;
    default: break;
    }
    if (gg_event != IDLE)
        HandleGGEvent(gg_event, GGK_UNKNOWN, GetKeyMods(m_keyboard), mouse_pos, Pt());
    return true;
}

bool OgreGUI::mouseReleased(const OIS::MouseEvent &event, OIS::MouseButtonID id)
{
    Pt mouse_pos(event.state.X.abs, event.state.Y.abs);
    EventType gg_event = IDLE;
    switch (id) {
    case OIS::MB_Left:   gg_event = LRELEASE; break;
    case OIS::MB_Right:  gg_event = RRELEASE; break;
    case OIS::MB_Middle: gg_event = MRELEASE; break;
    default: break;
    }
    if (gg_event != IDLE)
        HandleGGEvent(gg_event, GGK_UNKNOWN, GetKeyMods(m_keyboard), mouse_pos, Pt());
    return true;
}

bool OgreGUI::keyPressed(const OIS::KeyEvent& event)
{
    Uint32 mods = GetKeyMods(m_keyboard);
    Key key = GGKeyFromOISKey(event, mods, m_keyboard->getTextTranslation());
    if (key != GGK_UNKNOWN)
        HandleGGEvent(KEYPRESS, key, mods, Pt(), Pt());
    return true;
}

bool OgreGUI::keyReleased(const OIS::KeyEvent& event)
{
    Uint32 mods = GetKeyMods(m_keyboard);
    Key key = GGKeyFromOISKey(event, mods, m_keyboard->getTextTranslation());
    if (key != GGK_UNKNOWN)
        HandleGGEvent(KEYRELEASE, key, mods, Pt(), Pt());
    return true;
}

void OgreGUI::CleanupInputManager()
{
    if (m_input_manager) {
        m_input_manager->destroyInputObject(m_mouse);
        m_input_manager->destroyInputObject(m_keyboard);
        OIS::InputManager::destroyInputSystem(m_input_manager);
        m_input_manager = 0;
    }
}
