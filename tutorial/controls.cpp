#include "GGButton.h"
#include "GGDropDownList.h"
#include "GGDynamicGraphic.h"
#include "GGEdit.h"
#include "GGListBox.h"
#include "GGLayout.h"
#include "GGMenu.h"
#include "GGMultiEdit.h"
#include "GGScroll.h"
#include "GGSlider.h"
#include "GGSpin.h"
#include "GGStaticGraphic.h"
#include "GGTextControl.h"
#include "GGFileDlg.h"
#include "GGThreeButtonDlg.h"
#include "SDLGGApp.h"

// Tutorial 2: Controls
// Below you'll find the basic framework from Tutorial 1, plus one of every kind of GG Control.  Note that these
// controls are pretty ugly, especially the ones with the circles I drew myself in 2 minutes.  Yech.


// This is a free function that is called in response to the signal emitted by the quit button when it is clicked.  See
// below for the signal-connection code.
void QuitButtonClicked()
{
    GG::ThreeButtonDlg quit_dlg(200, 100, "Are you sure... I mean, really sure?", "Vera.ttf", 12, GG::CLR_GRAY,
                                GG::CLR_GRAY, GG::CLR_GRAY, GG::CLR_WHITE, 2);
    quit_dlg.Run();

    if (quit_dlg.Result() == 0)
        GG::App::GetApp()->Exit(0);
}

// This is a functor that is invoked when the files button is clicked, or when the Browse Files menu option is selected.
// Note that in order to accoplish this, the functor must provide two interfaces, one a void () signature, and one a
// void (int) signature.
struct BrowseFilesFunctor
{
    void operator()()
    {
        GG::FileDlg file_dlg("", "", false, false, "Vera.ttf", 12, GG::CLR_GRAY,
                             GG::CLR_GRAY);
        file_dlg.Run();
    }
    void operator()(int)
    {operator()();}
};

// This is a full-fledged class that has one of its member functions invoked when the radio buttons are clicked.
class TextUpdater
{
public:
    void SetTextControl(GG::TextControl* text_control)
    {
        m_text_control = text_control;
    }
    void SelectText(int index)
    {
        if (index)
            m_text_control->SetText("Plan to execute: Plan 9!");
        else
            m_text_control->SetText("Plan to execute: Plan 8");
    }
private:
    GG::TextControl* m_text_control;
} g_text_updater;


// A custom GG::ListBox::Row type used in the list box; more on this below.
struct CustomTextRow : GG::ListBox::Row
{
    CustomTextRow() :
        Row()
    {}

    CustomTextRow(const std::string& text) :
        Row()
    {
        push_back(GG::ListBox::Row::CreateControl(text, GG::App::GetApp()->GetFont("Vera.ttf", 12), GG::CLR_WHITE));
    }
};


////////////////////////////////////////////////////////////////////////////////
// Ignore all code until ControlsTestGGApp::Initialize(); the enclosed code is
// straight from Tutorial 1.
////////////////////////////////////////////////////////////////////////////////
class ControlsTestGGApp : public SDLGGApp
{
public:
    ControlsTestGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

ControlsTestGGApp::ControlsTestGGApp() : 
    SDLGGApp(1024, 768, false, "Control-Test GG App")
{
}

void ControlsTestGGApp::Enter2DMode()
{
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, AppWidth(), AppHeight());

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, AppWidth(), AppHeight(), 0.0, 0.0, AppWidth());

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void ControlsTestGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void ControlsTestGGApp::Render()
{
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

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

    GG::App::Render();
}

void ControlsTestGGApp::GLInit()
{
    double ratio = AppWidth() / (float)(AppHeight());

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, AppWidth(), AppHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);
    gluLookAt(0.0, 0.0, 5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}
////////////////////////////////////////////////////////////////////////////////
// End of old tutorial code.
////////////////////////////////////////////////////////////////////////////////

void ControlsTestGGApp::Initialize()
{
    SDL_WM_SetCaption("Control-Test GG App", "Control-Test GG App");

    boost::shared_ptr<GG::Font> font = GetFont("Vera.ttf", 12);

    // We're creating a layout for this window, so that we don't have to come up with position coordinates for all the
    // Controls.
    GG::Layout* layout = new GG::Layout(0, 0, AppWidth(), AppHeight(), 1, 1, 10);

    // Create a menu bar for the top of the app.
    GG::MenuItem menu_contents;
    GG::MenuItem file_menu("File", 0, false, false);
    // Notice that the menu item can be directly attached to a slot right in its ctor.  In this case, the slot is a
    // functor.
    file_menu.next_level.push_back(GG::MenuItem("Browse...", 1, false, false, BrowseFilesFunctor()));
    menu_contents.next_level.push_back(file_menu);
    GG::MenuBar* menu_bar =
        new GG::MenuBar(0, 0, AppWidth(), font, menu_contents, GG::CLR_WHITE);
    layout->Add(menu_bar, 0, 0, 1, 2, GG::ALIGN_TOP);

    // Here we create a RadioButtonGroup, then create two StateButtons and add them to the group.  The only signal that
    // needs to be handled to respond to changes to the StateButtons is the group's ButtonChangedSignal; the group
    // handles the click signals of its member radio buttons.
    GG::RadioButtonGroup* radio_button_group = new GG::RadioButtonGroup(10, 10);
    // Note that even though the overall app has a layout, the positions of the buttons within the RadioButtonGroup must
    // be hand-coded.
    GG::StateButton* state_button8 =
        new GG::StateButton(0,   0, 100, 25, "Plan 8", font, GG::TF_LEFT, GG::CLR_GRAY, GG::CLR_WHITE,
                            GG::CLR_ZERO, GG::StateButton::SBSTYLE_3D_RADIO);
    GG::StateButton* state_button9 =
        new GG::StateButton(100, 0, 100, 25, "Plan 9", font, GG::TF_LEFT, GG::CLR_GRAY, GG::CLR_WHITE,
                            GG::CLR_ZERO, GG::StateButton::SBSTYLE_3D_RADIO);
    radio_button_group->AddButton(state_button8);
    radio_button_group->AddButton(state_button9);
    layout->Add(radio_button_group, 1, 0);

    // A text control to display the result of clicking the radio buttons above.
    GG::TextControl* plan_text_control =
        new GG::TextControl(0, 0, 150, 25, "", font, GG::CLR_WHITE);
    layout->Add(plan_text_control, 1, 1);

    // A drop-down list, otherwise known as a "combo box".  What a stupid name.
    GG::ListBox::Row* row;
    GG::DropDownList* drop_down_list =
        new GG::DropDownList(0, 0, 150, 25, 150, GG::CLR_GRAY, GG::CLR_GRAY);
    // Here we add the rows we want to appear in the DropDownList one at a time.
    drop_down_list->SetStyle(GG::LB_NOSORT);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("I always", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("thought", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("\"combo box\"", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("was a lousy", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("way to describe", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("controls", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(GG::ListBox::Row::CreateControl("like this", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    drop_down_list->Select(0);
    layout->Add(drop_down_list, 2, 0);

    // A basic edit-control.
    GG::Edit* edit = new GG::Edit(0, 0, 100, 35, "Edit me.", font, GG::CLR_GRAY, GG::CLR_WHITE, GG::CLR_SHADOW);
    layout->Add(edit, 2, 1);

    // A list box.  Here, instead of adding GG::ListBox::Row's to the list and populating each one with a text control,
    // we create CustomTextRow objects.  CustomTextRow is a subclass of GG::ListBox::Row that contains a single text
    // control.  It is only a slight gain in code readability to use it instead of hand-coding each row as above in the
    // DropDownList example, but in general the ability to add arbitrary GG::ListBox::Row subclasses to a ListBox is
    // very powerful.  The most important use is to have each row contain a reference to the object that it represents
    // in the list.
    GG::ListBox* list_box = new GG::ListBox(0, 0, 300, 200, GG::CLR_GRAY);
    list_box->Insert(new CustomTextRow("Item 1"));
    list_box->Insert(new CustomTextRow("Item 2"));
    list_box->Insert(new CustomTextRow("Item 3"));
    list_box->Insert(new CustomTextRow("Item 4"));
    list_box->Insert(new CustomTextRow("Item 5"));
    list_box->Insert(new CustomTextRow("Item 6"));
    list_box->Insert(new CustomTextRow("Item 7"));
    layout->Add(list_box, 3, 0);

    // A multi-line edit control.
    GG::MultiEdit* multi_edit =
        new GG::MultiEdit(0, 0, 300, 200, "Edit me\ntoo.", font, GG::CLR_GRAY, GG::TF_LINEWRAP, GG::CLR_WHITE, GG::CLR_SHADOW);
    layout->Add(multi_edit, 3, 1);

    // A numerical value slider control.
    GG::Slider* slider =
        new GG::Slider(0, 0, 300, 14, 1, 100, GG::Slider::HORIZONTAL, GG::Slider::RAISED, GG::CLR_GRAY, 10);
    layout->Add(slider, 4, 0);

    // An integer spin box.
    GG::Spin<int>* spin_int =
        new GG::Spin<int>(0, 0, 50, 30, 1, 1, -5, 5, false, font, GG::CLR_GRAY, GG::CLR_WHITE);
    spin_int->SetMaxSize(75, 30);
    layout->Add(spin_int, 5, 0);

    // A double spin box.  Note that this Spin is editable, but the values must be multiples of 1.5 between -0.5 and
    // 16.0; values typed into the spin will be clamped to the nearest value in this range.
    GG::Spin<double>* spin_double =
        new GG::Spin<double>(0, 0, 50, 30, 1.0, 1.5, -0.5, 16.0, true, font, GG::CLR_GRAY, GG::CLR_WHITE);
    spin_double->SetMaxSize(75, 30);
    layout->Add(spin_double, 6, 0);

    // A scrollbar control
    GG::Scroll* scroll =
        new GG::Scroll(0, 0, 14, 200, GG::Scroll::VERTICAL, GG::CLR_GRAY, GG::CLR_GRAY);
    scroll->SetMaxSize(14, 1000);
    layout->Add(scroll, 4, 1, 7, 2);

    // These two lines load my crappy image of circles used for the next two controls, and then restores the state of
    // GL_TEXTURE_2D, which is changed in the process.
    boost::shared_ptr<GG::Texture> circle_texture = GetTexture("hatchcircle.png");
    glDisable(GL_TEXTURE_2D);

    // A slideshow-type changing graphic control.
    GG::DynamicGraphic* dynamic_graphic =
        new GG::DynamicGraphic(0, 0, 64, 64, true, 64, 64, 0, circle_texture);
    dynamic_graphic->SetMaxSize(64, 64);
    layout->Add(dynamic_graphic, 7, 0);
    // An unchanging image control.
    GG::StaticGraphic* static_graphic =
        new GG::StaticGraphic(0, 0, 320, 128, circle_texture);
    layout->Add(static_graphic, 7, 1);

    // A couple of buttons.
    GG::Button* quit_button =
        new GG::Button(0, 0, 75, 25, "Quit...", font, GG::CLR_GRAY);
    GG::Button* files_button =
        new GG::Button(0, 0, 75, 25, "Files...", font, GG::CLR_GRAY);
    layout->Add(quit_button, 8, 0);
    layout->Add(files_button, 8, 1);

    // Here we connect three signals to three slots.  The first signal is connected to an object's member function; the
    // second to a functor; and the third to a free function.  Note that the syntax is very similar for each, and all
    // are equally easy.
    g_text_updater.SetTextControl(plan_text_control);
    GG::Connect(radio_button_group->ButtonChangedSignal, &TextUpdater::SelectText, &g_text_updater);
    GG::Connect(files_button->ClickedSignal, BrowseFilesFunctor());
    GG::Connect(quit_button->ClickedSignal, &QuitButtonClicked);

    // This registers our Layout (which is a Wnd subclass) with the App, causing it to be displayed.
    Register(layout);
}

////////////////////////////////////////////////////////////////////////////////
// More old tutorial code.
////////////////////////////////////////////////////////////////////////////////
void ControlsTestGGApp::FinalCleanup()
{
}

extern "C"
int main(int argc, char* argv[])
{
    ControlsTestGGApp app;

    try {
        app();
    } catch (const std::invalid_argument& e) {
        app.Logger().errorStream() << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        app.Logger().errorStream() << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        app.Logger().errorStream() << "main() caught exception(std::exception): " << e.what();
    }
    return 0;
}