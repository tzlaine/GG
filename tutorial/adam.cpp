#include <GG/AdamGlue.h>
#include <GG/Layout.h>
#include <GG/SDL/SDLGUI.h>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/adam_evaluate.hpp>

#include <iostream>


class AdamGGApp :
    public GG::SDLGUI
{
public:
    AdamGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

enum PathTypes {
    NONE,
    PATH_1
};

std::ostream& operator<<(std::ostream& os, PathTypes p);
std::istream& operator>>(std::istream& os, PathTypes& p);

class AdamDialog :
    public GG::Wnd
{
private:
    static const GG::X WIDTH;
    static const GG::Y HEIGHT;

public:
    AdamDialog();

    virtual void Render();

private:
    void OkClicked();

    boost::shared_ptr<GG::Font> m_font;
    GG::Spin<PathTypes>* m_path_spin;
    GG::Edit* m_flatness_edit;
    GG::Button* m_ok;

    adobe::sheet_t m_sheet;
    GG::AdamSheetGlue m_adam_glue;
};

std::ostream& operator<<(std::ostream& os, PathTypes p)
{
    os << (p == NONE ? "None" : "Path_1");
    return os;
}

std::istream& operator>>(std::istream& is, PathTypes& p)
{
    std::string path_str;
    is >> path_str;
    p = path_str == "None" ? NONE : PATH_1;
    return is;
}

const GG::X AdamDialog::WIDTH(250);
const GG::Y AdamDialog::HEIGHT(75);

AdamDialog::AdamDialog() :
    Wnd(AdamGGApp::GetGUI()->AppWidth() / 2 - WIDTH,
        AdamGGApp::GetGUI()->AppHeight() / 2 - HEIGHT,
        WIDTH, HEIGHT, GG::CLICKABLE | GG::MODAL),
    m_font(AdamGGApp::GetGUI()->GetFont("tutorial/Vera.ttf", 12)),
    m_path_spin(new GG::Spin<PathTypes>(GG::X0, GG::Y0, GG::X(50),
                                        NONE, PathTypes(1), NONE, PATH_1,
                                        false, m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_flatness_edit(new GG::Edit(GG::X0, GG::Y0, GG::X(50),
                                 "0.0", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_ok(new GG::Button(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                        "Ok", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_sheet(),
    m_adam_glue(m_sheet)
{
    const char* sheet_spec =
        "sheet clipping_path"
        "{"
        "output:"
        "    result                  <== { path: path, flatness: flatness };"
        ""
        "interface:"
        "    unlink flatness : 0.0   <== (path == 0) ? 0.0 : flatness;"
        "    path            : 1;"
        "}";

    GG::TextControl* path_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Path:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);
    GG::TextControl* flatness_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Flatness:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, WIDTH, HEIGHT, 2u, 3u);
    layout->SetBorderMargin(2);
    layout->SetCellMargin(4);
    layout->Add(path_label, 0, 0);
    layout->Add(m_path_spin, 0, 1);
    layout->Add(m_ok, 0, 2);
    layout->Add(flatness_label, 1, 0);
    layout->Add(m_flatness_edit, 1, 1);
    SetLayout(layout);

    GG::Connect(m_ok->ClickedSignal, &AdamDialog::OkClicked, this);

    m_sheet.machine_m.set_variable_lookup(boost::bind(&adobe::sheet_t::get, &m_sheet, _1));
    std::istringstream is(sheet_spec);
    adobe::parse(is, adobe::line_position_t("m_sheet"), adobe::bind_to_sheet(m_sheet));
    m_sheet.update();

    m_adam_glue.AddCell<double, PathTypes>(*m_path_spin, adobe::name_t("path"));
    m_adam_glue.AddCell<double, double>(*m_flatness_edit, adobe::name_t("flatness"));
}

void AdamDialog::Render()
{ FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_SHADOW, GG::CLR_SHADOW, 1); }

void AdamDialog::OkClicked()
{ m_done = true; }

// implementations

AdamGGApp::AdamGGApp() : 
    SDLGUI(1024, 768, false, "Adam App")
{
}

void AdamGGApp::Enter2DMode()
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

void AdamGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void AdamGGApp::Render()
{
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

    // DeltaT() returns the time in whole milliseconds since the last frame
    // was rendered (in other words, since this method was last invoked).
    glRotated(DeltaT() * DEGREES_PER_MS, 0.0, 1.0, 0.0);

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

    GG::GUI::Render();
}

void AdamGGApp::GLInit()
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

void AdamGGApp::Initialize()
{
    SDL_WM_SetCaption("Adam GG App", "Adam GG App");

    AdamDialog adam_dlg;
    adam_dlg.Run();

    Exit(0);
}

// This gets called as the application is exit()ing, and as the name says,
// performs all necessary cleanup at the end of the app's run.
void AdamGGApp::FinalCleanup()
{
}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    AdamGGApp app;

    try {
        app();
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
    }
    return 0;
}
