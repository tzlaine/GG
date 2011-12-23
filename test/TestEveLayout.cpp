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


struct Action
{
    Action()
        {}
    Action(const GG::Pt& pt_1, const GG::Pt& pt_2, bool drag) :
        m_pt_1(pt_1),
        m_pt_2(pt_2),
        m_drag(drag)
        {}
    GG::Pt m_pt_1;
    GG::Pt m_pt_2;
    bool m_drag;
};

const char* g_eve_file = 0;
const char* g_adam_file = 0;
const char* g_output_dir = 0;
std::vector<Action> g_click_locations;
bool g_generate_variants = false;
bool g_dont_exit = false;
bool g_test_signals = false;

struct GenerateEvents
{
    GenerateEvents(GG::Timer& timer, GG::Wnd* dialog, const std::string& input_stem) :
        m_iteration(0),
        m_dialog(dialog),
        m_input_stem(input_stem)
        {}
    void operator()(unsigned int, GG::Timer* timer)
        {
            if (m_iteration <= g_click_locations.size()) {
                GG::GUI::GetGUI()->SaveWndAsPNG(m_dialog, OutputFilename());
                if (m_iteration) {
                    const Action& action = g_click_locations[m_iteration - 1];
                    if (action.m_drag) {
                        GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_1, GG::Pt());
                        GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::MOUSEMOVE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                        GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                    } else {
                        if (action.m_pt_1 != action.m_pt_2) {
                            GG::GUI::GetGUI()->QueueGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                            GG::GUI::GetGUI()->QueueGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                        }
                        GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_1, GG::Pt());
                        GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_1, GG::Pt());
                    }
                }
                timer->Reset();
            } else {
                m_dialog->EndRun();
            }
            ++m_iteration;
        }
    std::string OutputFilename()
        {
            boost::filesystem::path out(g_output_dir);
            std::string filename = m_input_stem;
            if (g_generate_variants) {
                filename += '_';
                filename += static_cast<char>('a' + m_iteration);
            }
            filename += ".png";
            out /= filename;
            return out.string();
        }
    std::size_t m_iteration;
    GG::Wnd* m_dialog;
    std::string m_input_stem;
};

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

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return name == adobe::static_name_t("ok") || name == adobe::static_name_t("cancel"); }

void SignalTester(adobe::name_t widget_type, adobe::name_t signal, adobe::name_t widget_id, const adobe::any_regular_t& value)
{
#define INSTRUMENT 0
#if INSTRUMENT
    std::cerr << "Testing unbound signals ...\n";
#endif
    if (widget_type == adobe::static_name_t("button")) {
#if INSTRUMENT
        std::cerr << "Testing unbound button signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("clicked"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(std::string("button value 1")));
        else
            BOOST_CHECK(value == adobe::any_regular_t(std::string("button value 2")));
    } else if (widget_type == adobe::static_name_t("checkbox")) {
#if INSTRUMENT
        std::cerr << "Testing unbound checkbox signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("checked"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(std::string("checkbox value 1")));
        else
            BOOST_CHECK(value == adobe::any_regular_t(std::string("checkbox value 2")));
    } else if (widget_type == adobe::static_name_t("radio_button")) {
#if INSTRUMENT
        std::cerr << "Testing unbound radio_button signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("checked"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(std::string("radio button value 1")));
        else
            BOOST_CHECK(value == adobe::any_regular_t(std::string("radio button value 2")));
    } else if (widget_type == adobe::static_name_t("slider")) {
#if INSTRUMENT
        std::cerr << "Testing unbound slider signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("slid") ||
                    signal == adobe::static_name_t("slid_and_stopped"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id) {
            adobe::dictionary_t dict;
            dict[adobe::static_name_t("slider_pos")] = adobe::any_regular_t(25);
            dict[adobe::static_name_t("slider_min")] = adobe::any_regular_t(0);
            dict[adobe::static_name_t("slider_max")] = adobe::any_regular_t(99);
            adobe::any_regular_t expected_value(dict);
            BOOST_CHECK(value == expected_value);
        } else {
            adobe::dictionary_t dict;
            dict[adobe::static_name_t("slider_pos")] = adobe::any_regular_t(75);
            dict[adobe::static_name_t("slider_min")] = adobe::any_regular_t(0);
            dict[adobe::static_name_t("slider_max")] = adobe::any_regular_t(99);
            adobe::any_regular_t expected_value(dict);
            BOOST_CHECK(value == expected_value);
        }
    } else if (widget_type == adobe::static_name_t("edit_text")) {
#if INSTRUMENT
        std::cerr << "Testing unbound edit_text signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("edited") ||
                    signal == adobe::static_name_t("focus_update"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(std::string("f")));
        else
            BOOST_CHECK(value == adobe::any_regular_t(std::string("g")));
    } else if (widget_type == adobe::static_name_t("edit_number")) {
#if INSTRUMENT
        std::cerr << "Testing unbound edit_number signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("edited") ||
                    signal == adobe::static_name_t("focus_update") ||
                    signal == adobe::static_name_t("unit_changed"));
        BOOST_CHECK(!widget_id ||
                    widget_id == adobe::static_name_t("test_id_1") ||
                    widget_id == adobe::static_name_t("test_id_2"));
        if (signal == adobe::static_name_t("edited") ||
            signal == adobe::static_name_t("focus_update")) {
            if (!widget_id)
                BOOST_CHECK(value == adobe::any_regular_t(1));
            else if (widget_id == adobe::static_name_t("test_id_1"))
                BOOST_CHECK(value == adobe::any_regular_t(2));
        } else {
            if (widget_id == adobe::static_name_t("test_id_1"))
                BOOST_CHECK(value == adobe::any_regular_t(std::string("cm")));
        }
    } else if (widget_type == adobe::static_name_t("popup")) {
#if INSTRUMENT
        std::cerr << "Testing unbound popup signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("selection_changed"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(2));
        else
            BOOST_CHECK(value == adobe::any_regular_t(3));
    } else if (widget_type == adobe::static_name_t("tab_group")) {
#if INSTRUMENT
        std::cerr << "Testing unbound tab_group signals ...\n";
#endif
        BOOST_CHECK(signal == adobe::static_name_t("tab_changed"));
        BOOST_CHECK(!widget_id || widget_id == adobe::static_name_t("test_id"));
        if (!widget_id)
            BOOST_CHECK(value == adobe::any_regular_t(2));
        else
            BOOST_CHECK(value == adobe::any_regular_t(3));
    }
#undef INSTRUMENT
}

void MinimalGGApp::Initialize()
{
    boost::filesystem::path eve(g_eve_file);
    boost::filesystem::path adam(g_adam_file);
    std::auto_ptr<GG::EveDialog> eve_dialog(
        g_test_signals ?
        GG::MakeEveDialog(eve, adam, &ButtonHandler, &SignalTester) :
        GG::MakeEveDialog(eve, adam, &ButtonHandler)
    );

    boost::filesystem::path input(g_eve_file);
    std::string input_stem =
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
        input.stem().native()
#else
        input.stem()
#endif
        ;
    GG::Timer timer(100);
    if (!g_dont_exit)
        GG::Connect(timer.FiredSignal, GenerateEvents(timer, eve_dialog.get(), input_stem));
    eve_dialog->Run();

    std::cout << "Terminating action: " << eve_dialog->TerminatingAction() << "\n"
              << "Result:";

    if (eve_dialog->Result().empty())
        std::cout << " <no result set>\n";
    else
        std::cout << "\n" << eve_dialog->Result();
    std::cout << std::endl;

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
    g_eve_file = argv[1];
    g_adam_file = argv[2];
    g_output_dir = argv[3];
    int i = 4;
    std::string token;
    std::size_t comma_1;
    while (i < argc && (token = argv[i], comma_1 = token.find(',')) != std::string::npos) {
        g_generate_variants = true;
        std::size_t comma_2 = token.find(',', comma_1 + 1);
        GG::Pt point_1(GG::X(boost::lexical_cast<int>(token.substr(0, comma_1))),
                       GG::Y(boost::lexical_cast<int>(token.substr(comma_1 + 1, comma_2 - comma_1 - 1))));
        GG::Pt point_2 = point_1;
        bool drag = false;
        if (comma_2 != std::string::npos) {
            std::size_t comma_3 = token.find(',', comma_2 + 1);
            std::size_t end = token.find('d', comma_3 + 1);
            drag = end != std::string::npos;
            point_2 = GG::Pt(GG::X(boost::lexical_cast<int>(token.substr(comma_2 + 1, comma_3 - comma_2 - 1))),
                             GG::Y(boost::lexical_cast<int>(token.substr(comma_3 + 1, end - comma_3 - 1))));
        }
        g_click_locations.push_back(Action(point_1, point_2, drag));
        ++i;
    }
    if (i < argc && (token = argv[i], token == "test_signals")) {
        g_test_signals = true;
        ++i;
    }
    if (i < argc)
        g_dont_exit = true;
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
