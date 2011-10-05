#include "serialization.h"

#include "saveload.h"

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/ListBox.h>
#include <GG/Layout.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/TextControl.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/SDL/SDLGUI.h>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <fstream>
#include <iostream>


// Tutorial 3: Serialization

// This file is part of the third tutorial.  The other files are
// serialization.h, saveload.h, and saveload.cpp.  It extends the Tutorial 2
// by serializing all the controls to a file called test.xml, deleting them,
// and recreating them from the XML file before showing them on the screen.
// This demonstrates how GG serialization works in detail.  For further
// reference material, see the Boost.Serialization documentation.


////////////////////////////////////////////////////////////////////////////////
// Ignore all code until the end of ControlsTestApp::Initialize(); the
// enclosed code is straight from Tutorial 2.
////////////////////////////////////////////////////////////////////////////////
void QuitButtonClicked()
{
    GG::ThreeButtonDlg quit_dlg(GG::X(200), GG::Y(100), "Are you sure... I mean, really sure?", GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(),
                                GG::CLR_GRAY, GG::CLR_GRAY, GG::CLR_GRAY, GG::CLR_WHITE, 2);
    quit_dlg.Run();

    if (quit_dlg.Result() == 0)
        GG::GUI::GetGUI()->Exit(0);
}

struct BrowseFilesFunctor
{
    void operator()()
    {
        GG::FileDlg file_dlg("", "", false, false, GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(),
                             GG::CLR_GRAY, GG::CLR_GRAY);
        file_dlg.Run();
    }
    void operator()(int)
    {operator()();}
};

class TextUpdater
{
public:
    void SetTextControl(GG::TextControl* text_control)
    {
        m_text_control = text_control;
    }
    void SelectText(std::size_t index)
    {
        if (index)
            m_text_control->SetText("Plan to execute: Plan 9!");
        else
            m_text_control->SetText("Plan to execute: Plan 8");
    }
private:
    GG::TextControl* m_text_control;
} g_text_updater;


CustomTextRow::CustomTextRow() :
    Row()
{}

CustomTextRow::CustomTextRow(const std::string& text) :
    Row()
{
    push_back(GG::ListBox::Row::CreateControl(text, GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont(), GG::CLR_WHITE));
}


class ControlsTestApp : public GG::SDLGUI
{
public:
    ControlsTestApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

ControlsTestApp::ControlsTestApp() : 
    SDLGUI(1024, 768, false, "Control-Test GG App")
{
    SetSaveWndFunction(&::SaveWnd);
    SetLoadWndFunction(&::LoadWnd);
}

void ControlsTestApp::Enter2DMode()
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

void ControlsTestApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void ControlsTestApp::Render()
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

    GG::GUI::Render();
}

void ControlsTestApp::GLInit()
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

void ControlsTestApp::Initialize()
{
    SDL_WM_SetCaption("Control-Test GG App", "Control-Test GG App");

    boost::shared_ptr<GG::Font> font = GetStyleFactory()->DefaultFont();

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, AppWidth(), AppHeight(), 1, 1, 10);

    GG::MenuItem menu_contents;
    GG::MenuItem file_menu("File", 0, false, false);
    file_menu.next_level.push_back(
        GG::MenuItem("Browse...", 1, false, false,
                     GG::MenuItem::SelectedSignalType::slot_type(BrowseFilesFunctor())));
    menu_contents.next_level.push_back(file_menu);
    GG::MenuBar* menu_bar =
        new GG::MenuBar(GG::X0, GG::Y0, AppWidth(), font, menu_contents, GG::CLR_WHITE);
    layout->Add(menu_bar, 0, 0, 1, 2, GG::ALIGN_TOP);

    GG::RadioButtonGroup* radio_button_group = new GG::RadioButtonGroup(GG::X(10), GG::Y(10), GG::X(200), GG::Y(25), GG::HORIZONTAL);
    radio_button_group->AddButton("Plan 8", font, GG::FORMAT_LEFT, GG::CLR_GRAY, GG::CLR_WHITE);
    radio_button_group->AddButton("Plan 9", font, GG::FORMAT_LEFT, GG::CLR_GRAY, GG::CLR_WHITE);
    layout->Add(radio_button_group, 1, 0);

    GG::TextControl* plan_text_control =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(150), GG::Y(25), "", font, GG::CLR_WHITE);
    layout->Add(plan_text_control, 1, 1);

    GG::ListBox::Row* row;
    GG::DropDownList* drop_down_list =
        new GG::DropDownList(GG::X0, GG::Y0, GG::X(150), GG::Y(25), GG::Y(150), GG::CLR_GRAY);
    drop_down_list->SetInteriorColor(GG::CLR_GRAY);
    drop_down_list->SetStyle(GG::LIST_NOSORT);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("I always", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("thought", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("\"combo box\"", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("was a lousy", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("way to describe", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("controls", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    row = new GG::ListBox::Row();
    row->push_back(row->CreateControl("like this", font, GG::CLR_WHITE));
    drop_down_list->Insert(row);
    drop_down_list->Select(0);
    layout->Add(drop_down_list, 2, 0);

    GG::Edit* edit = new GG::Edit(GG::X0, GG::Y0, GG::X(100), "Edit me.", font, GG::CLR_GRAY, GG::CLR_WHITE, GG::CLR_SHADOW);
    edit->Resize(GG::Pt(GG::X(100), GG::Y(35)));
    layout->Add(edit, 2, 1);

    GG::ListBox* list_box = new GG::ListBox(GG::X0, GG::Y0, GG::X(300), GG::Y(200), GG::CLR_GRAY);
    list_box->Insert(new CustomTextRow("Item 1"));
    list_box->Insert(new CustomTextRow("Item 2"));
    list_box->Insert(new CustomTextRow("Item 3"));
    list_box->Insert(new CustomTextRow("Item 4"));
    list_box->Insert(new CustomTextRow("Item 5"));
    list_box->Insert(new CustomTextRow("Item 6"));
    list_box->Insert(new CustomTextRow("Item 7"));
    layout->Add(list_box, 3, 0);

    GG::MultiEdit* multi_edit =
        new GG::MultiEdit(GG::X0, GG::Y0, GG::X(300), GG::Y(200), "Edit me\ntoo.", font, GG::CLR_GRAY, GG::MULTI_LINEWRAP, GG::CLR_WHITE, GG::CLR_SHADOW);
    layout->Add(multi_edit, 3, 1);

    GG::Slider<int>* slider =
        new GG::Slider<int>(GG::X0, GG::Y0, GG::X(300), GG::Y(14), 1, 100, GG::HORIZONTAL, GG::RAISED, GG::CLR_GRAY, 10);
    layout->Add(slider, 4, 0);

    GG::Spin<int>* spin_int =
        new GG::Spin<int>(GG::X0, GG::Y0, GG::X(50), 1, 1, -5, 5, false, font, GG::CLR_GRAY, GG::CLR_WHITE);
    spin_int->Resize(GG::Pt(GG::X(50), GG::Y(30)));
    spin_int->SetMaxSize(GG::Pt(GG::X(75), GG::Y(30)));
    layout->Add(spin_int, 5, 0);

    GG::Spin<double>* spin_double =
        new GG::Spin<double>(GG::X0, GG::Y0, GG::X(50), 1, 1.5, -0.5, 16.0, true, font, GG::CLR_GRAY, GG::CLR_WHITE);
    spin_double->Resize(GG::Pt(GG::X(50), GG::Y(30)));
    spin_double->SetMaxSize(GG::Pt(GG::X(75), GG::Y(30)));
    layout->Add(spin_double, 6, 0);

    GG::Scroll* scroll =
        new GG::Scroll(GG::X0, GG::Y0, GG::X(14), GG::Y(200), GG::VERTICAL, GG::CLR_GRAY, GG::CLR_GRAY);
    scroll->SetMaxSize(GG::Pt(GG::X(14), GG::Y(1000)));
    layout->Add(scroll, 4, 1, 3, 1);

    boost::shared_ptr<GG::Texture> circle_texture = GetTexture("tutorial/hatchcircle.png");
    glDisable(GL_TEXTURE_2D);
    GG::DynamicGraphic* dynamic_graphic =
        new GG::DynamicGraphic(GG::X0, GG::Y0, GG::X(64), GG::Y(64), true, GG::X(64), GG::Y(64), 0, std::vector<boost::shared_ptr<GG::Texture> >(1, circle_texture));
    dynamic_graphic->SetMaxSize(GG::Pt(GG::X(64), GG::Y(64)));
    layout->Add(dynamic_graphic, 7, 0);
    GG::StaticGraphic* static_graphic =
        new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(320), GG::Y(128), circle_texture);
    layout->Add(static_graphic, 7, 1);

    GG::Button* quit_button =
        new GG::Button(GG::X0, GG::Y0, GG::X(75), GG::Y(25), "Quit...", font, GG::CLR_GRAY);
    GG::Button* files_button =
        new GG::Button(GG::X0, GG::Y0, GG::X(75), GG::Y(25), "Files...", font, GG::CLR_GRAY);
    layout->Add(quit_button, 8, 0);
    layout->Add(files_button, 8, 1);

////////////////////////////////////////////////////////////////////////////////
// End of old tutorial code.
////////////////////////////////////////////////////////////////////////////////

    // this is not actually displayed; it's just here to make sure that ColorDlg serialization works
    GG::ColorDlg* color_dlg = new GG::ColorDlg(GG::X(100), GG::Y(100), font, GG::CLR_GRAY, GG::CLR_GRAY);

    // Since we're saving to and then immediately reloading from test.xml, we
    // need to enclose the serialiaztion code inside of code blocks, so that
    // the boost::archive::xml_*archive objects will be destroyed when they go
    // out of scope, allowing another archive to be associated with the file.
    // This is because there is no way to explicitly close a boost
    // serialization archive.

    {
        std::ofstream ofs("test.xml");
        boost::archive::xml_oarchive oa(ofs);
        SaveWnd(layout, "layout", oa);

        // Somthing a little odd is happening here.  We're saving these four
        // objects singly, but they're already being saved as children of
        // layout.  Since signals do not survive serialization, we have to
        // wait to connect the signals below.  But this means that since we
        // are going to delete all these objects a few lines below, we must
        // save these objects so that we have pointers to them when we need to
        // connect the signals below.  Note that we are not reconnecting the
        // File->Browse.. menu item to a BrowseFilesFunctor.  Reconnecting
        // this is left as an excercise for the reader.  Also note that since
        // the boost.serialization library is so damn smart, only one copy of
        // each of these objects is saved in the XML file.  This applies to
        // all serialized pointers; if you save 10 pointers to a single
        // object, only one object is actually saved, and all 10 pointers will
        // point to the same object when they are loaded.

        SaveWnd(radio_button_group, "radio_button_group", oa);
        SaveWnd(plan_text_control, "plan_text_control", oa);
        SaveWnd(files_button, "files_button", oa);
        SaveWnd(quit_button, "quit_button", oa);

        // save a color dialog just to make sure it works
        SaveWnd(color_dlg, "color_dialog", oa);
    }

    // Here, we delete the layout (which automatically causes all its children
    // to be freed as well).  After doing this, if we see the windows on the
    // screen, we know they came from the reload of the XML file.
    delete layout;
    layout = 0;
    plan_text_control = 0;

    delete color_dlg;
    color_dlg = 0;

    {
        std::ifstream ifs("test.xml");
        boost::archive::xml_iarchive ia(ifs);
        LoadWnd(layout, "layout", ia);
        LoadWnd(radio_button_group, "radio_button_group", ia);
        LoadWnd(plan_text_control, "plan_text_control", ia);
        LoadWnd(files_button, "files_button", ia);
        LoadWnd(quit_button, "quit_button", ia);

        // re-load the color dialog just to make sure it works
        LoadWnd(color_dlg, "color_dialog", ia);
    }

////////////////////////////////////////////////////////////////////////////////
// More old tutorial code.
////////////////////////////////////////////////////////////////////////////////

    g_text_updater.SetTextControl(plan_text_control);
    GG::Connect(radio_button_group->ButtonChangedSignal, &TextUpdater::SelectText, &g_text_updater);
    GG::Connect(files_button->ClickedSignal, BrowseFilesFunctor());
    GG::Connect(quit_button->ClickedSignal, &QuitButtonClicked);

    Register(layout);
}

void ControlsTestApp::FinalCleanup()
{
}

extern "C"
int main(int argc, char* argv[])
{
    ControlsTestApp app;

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
