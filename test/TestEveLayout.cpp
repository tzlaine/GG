#if USE_SDL_BACKEND
#include "SDLBackend.h"
#else
#include "OgreBackend.h"
#endif

#include <GG/EveGlue.h>
#include <GG/EveParser.h>
#include <GG/Filesystem.h>
#include <GG/GUI.h>
#include <GG/Timer.h>
#include <GG/Wnd.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/future/cursor.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>

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
        m_drag(drag),
        m_pressed_for_drag(false),
        m_moved_for_drag(false),
        m_released_for_drag(false)
        {}
    Action(const std::string& keys) :
        m_keys(keys),
        m_drag(false),
        m_pressed_for_drag(false),
        m_moved_for_drag(false),
        m_released_for_drag(false)
        {}
    GG::Pt m_pt_1;
    GG::Pt m_pt_2;
    bool m_drag;
    bool m_pressed_for_drag;
    bool m_moved_for_drag;
    bool m_released_for_drag;
    std::string m_keys;
};

const char* g_eve_file = 0;
const char* g_adam_file = 0;
const char* g_output_dir = 0;
std::vector<Action> g_click_locations;
bool g_generate_variants = false;
bool g_dont_exit = false;
bool g_test_signals = false;
bool g_drags = false;

struct GenerateEvents
{
    GenerateEvents(GG::Timer& timer, GG::Wnd* dialog, const std::string& input_stem) :
        m_iteration(0),
        m_dialog(dialog),
        m_input_stem(input_stem)
        {}
    void operator()(unsigned int, GG::Timer* timer)
        {
            bool increment = true;
            if (m_iteration <= g_click_locations.size()) {
                GG::GUI::GetGUI()->SaveWndAsPNG(m_dialog, OutputFilename());
                if (m_iteration) {
                    Action& action = g_click_locations[m_iteration - 1];
                    if (!action.m_keys.empty()) {
                        for (std::string::const_iterator it = action.m_keys.begin(); it != action.m_keys.end(); ++it) {
                            GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::KEYPRESS, GG::Key(*it), boost::uint32_t(*it), GG::Flags<GG::ModKey>(), GG::Pt(), GG::Pt());
                            GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::KEYRELEASE, GG::Key(*it), boost::uint32_t(*it), GG::Flags<GG::ModKey>(), GG::Pt(), GG::Pt());
                        }
                    } else if (action.m_drag) {
                        if (!action.m_pressed_for_drag) {
                            GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_1, GG::Pt());
                            action.m_pressed_for_drag = true;
                        } else if (!action.m_moved_for_drag) {
                            GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::MOUSEMOVE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, action.m_pt_2 - action.m_pt_1);
                            action.m_moved_for_drag = true;
                        } else if (!action.m_released_for_drag) {
                            GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                            action.m_released_for_drag = true;
                        }
                        increment = action.m_released_for_drag;
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
            if (increment)
                ++m_iteration;
        }
    std::string OutputFilename()
        {
            boost::filesystem::path out(GG::UTF8ToPath(g_output_dir));
            std::string filename = m_input_stem;
            if (g_generate_variants) {
                filename += '_';
                filename += static_cast<char>('a' + m_iteration);
            }
            filename += ".png";
            out /= filename;
            return GG::PathToUTF8(out);
        }
    std::size_t m_iteration;
    GG::Wnd* m_dialog;
    std::string m_input_stem;
};

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return false; }

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
            if (widget_id == adobe::static_name_t("test_id_2"))
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

void CustomInit()
{
    boost::filesystem::path eve(GG::UTF8ToPath(g_eve_file));
    boost::filesystem::path adam(GG::UTF8ToPath(g_adam_file));
    std::auto_ptr<GG::EveDialog> eve_dialog(
        g_test_signals ?
        GG::MakeEveDialog(eve, adam, &ButtonHandler, &SignalTester) :
        GG::MakeEveDialog(eve, adam, &ButtonHandler)
    );

    boost::filesystem::path input(GG::UTF8ToPath(g_eve_file));
    std::string input_stem = input.stem().native();
    GG::Timer timer(g_drags ? 300 : 100);
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

    GG::GUI::GetGUI()->Exit(0);
}

BOOST_AUTO_TEST_CASE( eve_layout )
{
#if USE_SDL_BACKEND
    MinimalSDLGUI::CustomInit = &CustomInit;
    MinimalSDLMain();
#else
    MinimalOgreGUI::CustomInit = &CustomInit;
    MinimalOgreMain();
#endif
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
    std::size_t keys;
    if (i < argc && (token = argv[i], token == "test_signals")) {
        g_test_signals = true;
        ++i;
    }
    while (
        i < argc &&
        (token = argv[i],
         keys = std::string::npos,
         (comma_1 = token.find(',')) != std::string::npos ||
         (keys = token.find("keys{")) == 0)
    ) {
        if (keys != std::string::npos) {
            g_click_locations.push_back(Action(token.substr(5, token.size() - 6)));
        } else {
            g_generate_variants = !g_test_signals;
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
            if (drag)
                g_drags = true;
        }
        ++i;
    }
    if (!g_click_locations.empty())
        g_click_locations.push_back(Action(GG::Pt(), GG::Pt(), false));
    if (i < argc)
        g_dont_exit = true;
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
