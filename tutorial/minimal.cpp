#if USE_SDL_BACKEND
#include "../test/SDLBackend.h"
#else
#include "../test/OgreBackend.h"
#endif

#include <GG/StyleFactory.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/dialogs/FileDlg.h>

#include <iostream>

// Tutorial 1: Minimal

// This contains the minimal interesting GG application.  It contains 3D as
// well as GUI elements in the same scene, and demonstrates how to use the
// default SDL and Ogre input drivers.


void CustomRender()
{
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

    // DeltaT() returns the time in whole milliseconds since the last frame
    // was rendered (in other words, since this method was last invoked).
    glRotated(GG::GUI::GetGUI()->DeltaT() * DEGREES_PER_MS, 0.0, 1.0, 0.0);

    glBegin(GL_QUADS);

    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(1.0, 1.0, 1.0);

    glColor3d(1.0, 0.5, 0.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0,-1.0);
    glVertex3d(1.0, -1.0,-1.0);

    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);

    glColor3d(1.0, 1.0, 0.0);
    glVertex3d(1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, -1.0);

    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, 1.0);

    glColor3d(1.0, 0.0, 1.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, -1.0);

    glEnd();
}

// This is the launch point for your GG app.  This is where you should place
// your main GG::Wnd(s) that should appear when the application starts, if
// any.
void CustomInit()
{
    // Create a modal dialog and execute it.  This will show GG operating on
    // top of a "real 3D" scene.  Note that if you want "real" 3D objects
    // (i.e. drawn in a non-orthographic space) inside of GG windows, you can
    // add whatever OpenGL calls you like to a GG::Wnd's Render() method,
    // sandwiched between Exit2DMode() and Enter2DMode().

    const std::string message = "Are we Готово yet?"; // That Russian word means "Done", ha.
    const std::set<GG::UnicodeCharset> charsets_ = GG::UnicodeCharsetsToRender(message);
    const std::vector<GG::UnicodeCharset> charsets(charsets_.begin(), charsets_.end());

    const boost::shared_ptr<GG::Font> font =
        GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(12, &charsets[0], &charsets[0] + charsets.size());

    GG::Wnd* quit_dlg =
        new GG::ThreeButtonDlg(GG::X(200), GG::Y(100), message, font, GG::CLR_SHADOW, 
                               GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);
    quit_dlg->Run();

    // Now that we're back from the modal dialog, we can exit normally, since
    // that's what closing the dialog indicates.  Exit() calls all the cleanup
    // methods for GG::SDLGUI.
    GG::GUI::GetGUI()->Exit(0);
}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    // The try-catch block is not strictly necessary, but it sure helps to see
    // what exception crashed your app in the log file.
    try {
#if USE_SDL_BACKEND
        MinimalSDLGUI::CustomInit = &CustomInit;
        MinimalSDLGUI::CustomRender = &CustomRender;
        MinimalSDLMain();
#else
        MinimalOgreGUI::CustomInit = &CustomInit;
        MinimalOgreGUI::CustomRender = &CustomRender;
        MinimalOgreMain();
#endif
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
    }
    return 0;
}
