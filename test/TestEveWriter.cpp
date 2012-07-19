#include <GG/AdamParser.h>
#include <GG/EveParser.h>
#include <GG/Wnd.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/eve_parser.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/SDL/SDLGUI.h>

#include <fstream>
#include <iostream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


// Currently, the Adam writer has some limitations: It does not preserve
// comments, and (at least partially because it does not preserve comments) it
// does not preserve line position information.
#define REQUIRE_EXACT_MATCH 0

const char* g_eve_file = 0;
const char* g_adam_file = 0;

bool instrument_positions = false;

namespace GG {

    struct StoreAddViewParams
    {
        StoreAddViewParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        boost::any operator()(const boost::any& parent,
                              const adobe::line_position_t& parse_location,
                              adobe::name_t name,
                              const adobe::array_t& parameters,
                              const std::string& brief,
                              const std::string& detailed)
        {
            // Note that we are forced to ignore parent

            if (instrument_positions) {
                std::cerr << parse_location.stream_name() << ":"
                          << parse_location.line_number_m << ":"
                          << parse_location.line_start_m << ":"
                          << parse_location.position_m << ":";
                if (parse_location.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + parse_location.line_start_m,
                                                      m_str.begin() + parse_location.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
#if REQUIRE_EXACT_MATCH
                push_back(m_array, parse_location.stream_name());
                push_back(m_array, parse_location.line_number_m);
                push_back(m_array, std::size_t(parse_location.line_start_m));
                push_back(m_array, std::size_t(parse_location.position_m));
#endif
            }
            push_back(m_array, name);
            push_back(m_array, parameters);
#if REQUIRE_EXACT_MATCH
            push_back(m_array, brief);
            push_back(m_array, detailed);
#endif

            return parent;
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddCellParams
    {
        StoreAddCellParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::eve_callback_suite_t::cell_type_t type,
                        adobe::name_t name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& initializer,
                        const std::string& brief,
                        const std::string& detailed)
        {
            std::string type_str;
            switch (type)
            {
            case adobe::eve_callback_suite_t::constant_k: type_str = "constant_k";
            case adobe::eve_callback_suite_t::interface_k: type_str = "interface_k";
            }
            push_back(m_array, type_str);
            push_back(m_array, name);
            if (instrument_positions) {
                std::cerr << position.stream_name() << ":"
                          << position.line_number_m << ":"
                          << position.line_start_m << ":"
                          << position.position_m << ":";
                if (position.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                      m_str.begin() + position.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
#if REQUIRE_EXACT_MATCH
                push_back(m_array, position.stream_name());
                push_back(m_array, position.line_number_m);
                push_back(m_array, std::size_t(position.line_start_m));
                push_back(m_array, std::size_t(position.position_m));
#endif
            }
            push_back(m_array, initializer);
#if REQUIRE_EXACT_MATCH
            push_back(m_array, brief);
            push_back(m_array, detailed);
#endif
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

}

std::string getline_proc (adobe::name_t, std::streampos)
{ return std::string(); }

class MinimalGGApp : public GG::SDLGUI
{
public:
    MinimalGGApp() : SDLGUI(1024, 768, false, "") {}

    virtual void Enter2DMode() {}
    virtual void Exit2DMode() {}

private:
    virtual void GLInit() {}
    virtual void Initialize()
        {
            std::string file_contents = read_file(g_eve_file);

            adobe::array_t new_parse;

            adobe::eve_callback_suite_t new_parse_callbacks;
            new_parse_callbacks.add_view_proc_m = GG::StoreAddViewParams(new_parse, file_contents);
            new_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(new_parse, file_contents);

            std::cout << "layout:\"\n" << file_contents << "\n\"\n"
                      << "filename: " << g_eve_file << '\n';
            bool new_parse_failed = !GG::Parse(file_contents, g_eve_file, boost::any(), new_parse_callbacks);
            std::cout << "new:      <parse " << (new_parse_failed ? "failure" : "success") << ">\n";

            adobe::vm_lookup_t vm_lookup;
            adobe::sheet_t sheet;
            vm_lookup.attach_to(sheet);
            vm_lookup.attach_to(sheet.machine_m);
            std::string sheet_contents = read_file(g_adam_file);
            if (!GG::Parse(sheet_contents, g_adam_file, adobe::bind_to_sheet(sheet)))
                throw std::logic_error("Adam parse failed.");
            sheet.update();
            std::istringstream iss(file_contents);
            adobe::behavior_t root_behavior(true);
            adobe::auto_ptr<adobe::eve_client_holder> eve_holder(
                make_view("",
                          adobe::line_position_t::getline_proc_t(
                              new adobe::line_position_t::getline_proc_impl_t(&getline_proc)
                          ),
                          iss,
                          sheet,
                          root_behavior,
                          vm_lookup,
                          adobe::button_notifier_t(),
                          adobe::button_notifier_t(),
                          adobe::signal_notifier_t(),
                          adobe::row_factory_t(),
                          adobe::size_enum_t(),
                          adobe::default_widget_factory_proc(),
                          adobe::platform_display_type())
            );
            std::stringstream os;
            eve_holder->layout_sheet_m.print(os);
            adobe::array_t round_trip_parse;
            adobe::eve_callback_suite_t round_trip_parse_callbacks;
            round_trip_parse_callbacks.add_view_proc_m = GG::StoreAddViewParams(round_trip_parse, os.str());
            round_trip_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(round_trip_parse, os.str());
            bool round_trip_parse_pass =
                GG::Parse(os.str(), g_eve_file, boost::any(), round_trip_parse_callbacks);
            bool pass =
                !round_trip_parse_pass && new_parse_failed ||
                round_trip_parse == new_parse;

            std::cout << "Round-trip parse: " << (pass ? "PASS" : "FAIL") << "\n\n";

            if (!pass) {
                std::cout << "rewritten layout:\"\n" << os.str() << "\n\"\n";
                std::cout << "initial (verbose):\n";
                verbose_dump(new_parse);
                std::cout << "roud-trip (verbose):\n";
                verbose_dump(round_trip_parse);
            }

            BOOST_CHECK(pass);

            Exit(0);
        }
    virtual void FinalCleanup() {}
};

BOOST_AUTO_TEST_CASE( eve_writer )
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
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
