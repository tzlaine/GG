#if USE_SDL_BACKEND
#include "../test/SDLBackend.h"
#else
#include "../test/OgreBackend.h"
#endif

#include <GG/StyleFactory.h>
#include <GG/EveGlue.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/adobe/name.hpp>
#include <GG/adobe/any_regular.hpp>

#include <iostream>


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

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return false; }

void CustomInit()
{
    std::istringstream eve(
"layout alert_dialog\n"
"{\n"
"    view dialog(name: 'Alert', placement: place_row)\n"
"    {\n"
"        image(image: 'stop.png');     \n"
"\n"
"        column(vertical: align_fill)\n"
"        {\n"
"            static_text(name: 'Unfortunately, something drastic has happened. If you would like we can try to continue with the operation, but there is a chance you will blow up your computer. Would you like to try?', characters: 25);\n"
"            row(vertical: align_bottom, horizontal: align_right)\n"
"            {\n"
"                button(name: 'Cancel', action: @cancel, cancel: true);\n"
"                button(name: 'OK', bind: @result, action: @ok, default: true);\n"
"            }\n"
"        }\n"
"    }\n"
"}");

    std::istringstream adam(
"sheet alert_dialog\n"
"{\n"
"output:\n"
"    result <== { dummy_value: 42 };\n"
"}");

    GG::ExecuteModalDialog(eve, "inline eve code", adam, "inline adam code", &ButtonHandler);

    GG::GUI::GetGUI()->Exit(0);
}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
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
