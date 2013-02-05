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
#include <GG/adobe/future/modal_dialog_interface.hpp>

#include <boost/filesystem.hpp>

#include <fstream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"
#include "Action.h"


const char* g_eve_file = 0;
const char* g_adam_file = 0;
const char* g_output_dir = 0;
std::vector<Actions> g_inputs;
bool g_generate_variants = false;
bool g_dont_exit = false;
bool g_test_signals = false;
bool g_drags = false;

struct GenerateEvents
{
    GenerateEvents(GG::Wnd* dialog, const std::string& input_stem) :
        m_iteration(0),
        m_dialog(dialog),
        m_input_stem(input_stem)
        {}

    void operator()(unsigned int, GG::Timer* timer)
        {
            if (m_iteration <= g_inputs.size()) {
                GG::GUI::GetGUI()->SaveWndAsPNG(m_dialog, OutputFilename());
                if (m_iteration) {
                    Actions& actions = g_inputs[m_iteration - 1];
                    for (std::size_t i_ = 0; i_ < actions.size(); ++i_) {
                        typedef void (GG::GUI::*EventFunction)(GG::GUI::EventType, GG::Key, boost::uint32_t, GG::Flags<GG::ModKey>, const GG::Pt&, const GG::Pt&);
                        const std::size_t i = (i_ + 1) % actions.size();
                        EventFunction handle = i ? &GG::GUI::QueueGGEvent : &GG::GUI::HandleGGEvent;
                        Action& action = actions[i];
                        if (!action.m_keys.empty()) {
                            for (std::vector<GG::Key>::const_iterator it = action.m_keys.begin(); it != action.m_keys.end(); ++it) {
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::KEYPRESS, *it, boost::uint32_t(*it), GG::Flags<GG::ModKey>(), GG::Pt(), GG::Pt());
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::KEYRELEASE, *it, boost::uint32_t(*it), GG::Flags<GG::ModKey>(), GG::Pt(), GG::Pt());
                            }
                        } else if (action.m_semantic == Action::Drag) {
                            (GG::GUI::GetGUI()->*handle)(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                            (GG::GUI::GetGUI()->*handle)(GG::GUI::MOUSEMOVE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, action.m_pt_2 - action.m_pt);
                            (GG::GUI::GetGUI()->*handle)(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt_2, GG::Pt());
                        } else {
                            if (action.m_semantic != Action::RightClick) {
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                            } else {
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::RPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                                (GG::GUI::GetGUI()->*handle)(GG::GUI::RRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                            }
                            if (action.m_semantic == Action::DoubleClick) {
                                GG::GUI::GetGUI()->QueueGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                                GG::GUI::GetGUI()->QueueGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), action.m_pt, GG::Pt());
                            }
                        }
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
    GG::GUI::GetGUI()->SetMinDragTime(0);

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
        GG::Connect(timer.FiredSignal, GenerateEvents(eve_dialog.get(), input_stem));
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

std::ostream& operator<<(std::ostream& os, const Action& action)
{
    if (!action.m_keys.empty()) {
        os << "keys{" << action.m_keys << "}";
    } else {
        switch (action.m_semantic) {
        case Action::LeftClick:
            os << action.m_pt.x << ',' << action.m_pt.y;
            break;
        case Action::RightClick:
            os << action.m_pt.x << ',' << action.m_pt.y << 'r';
            break;
        case Action::DoubleClick:
            os << action.m_pt.x << ',' << action.m_pt.y << 'b';
            break;
        case Action::Drag:
            os << action.m_pt.x << ',' << action.m_pt.y << ',' << action.m_pt_2.x << ',' << action.m_pt_2.y << 'd';
            break;
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Actions& actions)
{
    for (std::size_t i = 0; i < actions.size(); ++i) {
        if (i)
            os << ',';
        os << actions[i];
    }
    return os;
}

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
    g_eve_file = argv[1];
    g_adam_file = argv[2];
    g_output_dir = argv[3];
    int i = 4;
    std::string token;
    if (i < argc && (token = argv[i], token == "test_signals")) {
        g_test_signals = true;
        ++i;
    }
    for (; i < argc; ++i) {
        const Actions& actions = ParseActions(argv[i]);
        if (actions.empty())
            break;
        g_inputs.push_back(actions);
    }
    g_generate_variants = !g_test_signals && 4 < i;
    if (!g_inputs.empty())
        g_inputs.push_back(Actions(1, Action(0, 0, Action::LeftClick)));
    if (i < argc)
        g_dont_exit = true;
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
