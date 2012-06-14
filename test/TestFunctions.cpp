#include <GG/EveLayout.h>

#include <GG/EveGlue.h>
#include <GG/EveParser.h>
#include <GG/GUI.h>
#include <GG/Timer.h>
#include <GG/Wnd.h>
#include <GG/SDL/SDLGUI.h>
#include <GG/adobe/adam.hpp>

#include <boost/filesystem.hpp>

#include <fstream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


adobe::any_regular_t increment(const adobe::dictionary_t& parameters)
{
    double retval;
    get_value(parameters, adobe::static_name_t("value"), retval);
    ++retval;
    return adobe::any_regular_t(retval);
}

struct Test
{
    Test() {}
    Test(const char* adam_filename,  const adobe::any_regular_t& expected_result) :
        m_adam_filename(adam_filename),
        m_expected_result(expected_result)
        {}
    const char* m_adam_filename;
    adobe::any_regular_t m_expected_result;
};

const std::vector<Test>& Tests()
{
    static std::vector<Test> retval;
    if (retval.empty()) {
        {
            adobe::dictionary_t result;
            result[adobe::static_name_t("one")] = adobe::any_regular_t(2.0);
            result[adobe::static_name_t("two")] = adobe::any_regular_t(3.0);
            retval.push_back(Test("function_test_transform_dictionary.adm", adobe::any_regular_t(result)));
        }
        // TODO: Add more tests!
    }
    return retval;
}

void CheckResult(const GG::EveDialog& eve_dialog, const adobe::any_regular_t& expected_result)
{
    GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), GG::Pt(GG::X(48), GG::Y(22)), GG::Pt());
    GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), GG::Pt(GG::X(48), GG::Y(22)), GG::Pt());
    BOOST_CHECK(eve_dialog.TerminatingAction());
    BOOST_CHECK(eve_dialog.Result().find(adobe::static_name_t("value")) != eve_dialog.Result().end());
    BOOST_CHECK(eve_dialog.Result().find(adobe::static_name_t("value"))->second == expected_result);
}

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return name == adobe::static_name_t("ok") || name == adobe::static_name_t("cancel"); }

void RunTest(std::size_t i)
{
    const Test& test = Tests()[i];
    boost::filesystem::path eve("function_test_dialog.eve");
    boost::filesystem::path adam(test.m_adam_filename);
    GG::EveDialog* eve_dialog(GG::MakeEveDialog(eve, adam, &ButtonHandler));
    GG::Timer timer(100);
    GG::Connect(timer.FiredSignal, boost::bind(&CheckResult, boost::cref(*eve_dialog), boost::cref(test.m_expected_result)));
    eve_dialog->Run();
}

class MinimalGGApp : public GG::SDLGUI
{
public:
    MinimalGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

MinimalGGApp::MinimalGGApp() : 
    SDLGUI(1024, 768, false, "Minimal GG App")
{}

void MinimalGGApp::Enter2DMode()
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

void MinimalGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void MinimalGGApp::GLInit()
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

void MinimalGGApp::Initialize()
{
    for (std::size_t i = 0; i < Tests().size(); ++i) {
        RunTest(i);
    }

    Exit(0);
}

void MinimalGGApp::FinalCleanup()
{}

BOOST_AUTO_TEST_CASE( eve_layout )
{
    MinimalGGApp app;
    app();
}

// Most of this is boilerplate cut-and-pasted from Boost.Test.  We need to
// select which test(s) to do, so we can't use it here unmodified.

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
bool init_unit_test()                   {
#else
::boost::unit_test::test_suite*
init_unit_test_suite( int, char* [] )   {
#endif

#ifdef BOOST_TEST_MODULE
    using namespace ::boost::unit_test;
    assign_op( framework::master_test_suite().p_name.value, BOOST_TEST_STRINGIZE( BOOST_TEST_MODULE ).trim( "\"" ), 0 );
    
#endif

#ifdef BOOST_TEST_ALTERNATIVE_INIT_API
    return true;
}
#else
    return 0;
}
#endif

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    GG::RegisterDictionaryFunction(adobe::static_name_t("increment"), &increment);
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
