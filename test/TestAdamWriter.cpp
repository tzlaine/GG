#include <GG/AdamParser.h>

#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

#include <fstream>
#include <iostream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>


// Currently, the Adam writer has some limitations: It does not preserve
// comments, and (at least partially because it does not preserve comments) it
// does not preserve line position information.
#define REQUIRE_EXACT_MATCH 0

const char* g_input_file = 0;

std::string read_file (const std::string& filename)
{
    std::string retval;
    std::ifstream ifs(filename.c_str());
    int c;
    while ((c = ifs.get()) != std::ifstream::traits_type::eof()) {
        retval += c;
    }
    return retval;
}

namespace adobe { namespace version_1 {

std::ostream& operator<<(std::ostream& stream, const type_info_t& x)
{
    std::ostream_iterator<char> out(stream);
    serialize(x, out);
    return stream;
}

} }

namespace GG {

    void verbose_dump(const adobe::array_t& array, std::size_t indent = 0);
    void verbose_dump(const adobe::dictionary_t& array, std::size_t indent = 0);

    void verbose_dump(const adobe::array_t& array, std::size_t indent)
    {
        if (array.empty()) {
            std::cout << std::string(4 * indent, ' ') << "[]\n";
            return;
        }

        std::cout << std::string(4 * indent, ' ') << "[\n";
        ++indent;
        for (adobe::array_t::const_iterator it = array.begin(); it != array.end(); ++it)
        {
            const adobe::any_regular_t& any = *it;
            if (any.type_info() == adobe::type_info<adobe::array_t>()) {
                verbose_dump(any.cast<adobe::array_t>(), indent);
            } else if (any.type_info() == adobe::type_info<adobe::dictionary_t>()) {
                verbose_dump(any.cast<adobe::dictionary_t>(), indent);
            } else {
                std::cout << std::string(4 * indent, ' ')
                          << "type: " << any.type_info() << " "
                          << "value: " << any << "\n";
            }
        }
        --indent;
        std::cout << std::string(4 * indent, ' ') << "]\n";
    }

    void verbose_dump(const adobe::dictionary_t& dictionary, std::size_t indent)
    {
        if (dictionary.empty()) {
            std::cout << std::string(4 * indent, ' ') << "{}\n";
            return;
        }

        std::cout << std::string(4 * indent, ' ') << "{\n";
        ++indent;
        for (adobe::dictionary_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it)
        {
            const adobe::pair<adobe::name_t, adobe::any_regular_t>& pair = *it;
            if (pair.second.type_info() == adobe::type_info<adobe::array_t>()) {
                std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
                verbose_dump(pair.second.cast<adobe::array_t>(), indent);
            } else if (pair.second.type_info() == adobe::type_info<adobe::dictionary_t>()) {
                std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
                verbose_dump(pair.second.cast<adobe::dictionary_t>(), indent);
            } else {
                std::cout << std::string(4 * indent, ' ')
                          << "(" << pair.first << ", "
                          << "type: " << pair.second.type_info() << " "
                          << "value: " << pair.second << ")\n";
            }
        }
        --indent;
        std::cout << std::string(4 * indent, ' ') << "}\n";
    }

    bool instrument_positions = false;

    struct StoreAddCellParams
    {
        StoreAddCellParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::adam_callback_suite_t::cell_type_t type,
                        adobe::name_t cell_name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& expr_or_init,
                        const std::string& brief,
                        const std::string& detailed)
        {
            std::string type_str;
            switch (type)
            {
            case adobe::adam_callback_suite_t::input_k: type_str = "input_k";
            case adobe::adam_callback_suite_t::output_k: type_str = "output_k";
            case adobe::adam_callback_suite_t::constant_k: type_str = "constant_k";
            case adobe::adam_callback_suite_t::logic_k: type_str = "logic_k";
            case adobe::adam_callback_suite_t::invariant_k: type_str = "invariant_k";
            }
            push_back(m_array, type_str);
            push_back(m_array, cell_name);
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
            push_back(m_array, expr_or_init);
#if REQUIRE_EXACT_MATCH
            push_back(m_array, brief);
            push_back(m_array, detailed);
#endif
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddRelationParams
    {
        StoreAddRelationParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(const adobe::line_position_t& position,
                        const adobe::array_t& conditional,
                        const adobe::adam_callback_suite_t::relation_t* first,
                        const adobe::adam_callback_suite_t::relation_t* last,
                        const std::string& brief,
                        const std::string& detailed)
        {
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
            push_back(m_array, conditional);
            while (first != last) {
                push_back(m_array, first->name_m);
                if (instrument_positions) {
                    std::cerr << first->position_m.stream_name() << ":"
                              << first->position_m.line_number_m << ":"
                              << first->position_m.line_start_m << ":"
                              << first->position_m.position_m << ":";
                    if (position.line_start_m) {
                        std::cerr << " \"" << std::string(m_str.begin() + first->position_m.line_start_m,
                                                          m_str.begin() + first->position_m.position_m)
                                  << "\"";
                    }
                    std::cerr << "\n";
                } else {
#if REQUIRE_EXACT_MATCH
                    push_back(m_array, first->position_m.stream_name());
                    push_back(m_array, first->position_m.line_number_m);
                    push_back(m_array, std::size_t(first->position_m.line_start_m));
                    push_back(m_array, std::size_t(first->position_m.position_m));
#endif
                }
                push_back(m_array, first->expression_m);
                push_back(m_array, first->detailed_m);
                push_back(m_array, first->brief_m);
                ++first;
            }
#if REQUIRE_EXACT_MATCH
            push_back(m_array, brief);
            push_back(m_array, detailed);
#endif
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddInterfaceParams
    {
        StoreAddInterfaceParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::name_t cell_name,
                        bool linked,
                        const adobe::line_position_t& position1,
                        const adobe::array_t& initializer,
                        const adobe::line_position_t& position2,
                        const adobe::array_t& expression,
                        const std::string& brief,
                        const std::string& detailed)
        {
            push_back(m_array, cell_name);
            push_back(m_array, linked);
            if (instrument_positions) {
                std::cerr << position1.stream_name() << ":"
                          << position1.line_number_m << ":"
                          << position1.line_start_m << ":"
                          << position1.position_m << ":";
                if (position1.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position1.line_start_m,
                                                      m_str.begin() + position1.position_m)
                          << "\"";
                }
                std::cerr << "\n";
            } else {
#if REQUIRE_EXACT_MATCH
                push_back(m_array, position1.stream_name());
                push_back(m_array, position1.line_number_m);
                push_back(m_array, std::size_t(position1.line_start_m));
                push_back(m_array, std::size_t(position1.position_m));
#endif
            }
            push_back(m_array, initializer);
            if (instrument_positions) {
                std::cerr << position2.stream_name() << ":"
                          << position2.line_number_m << ":"
                          << position2.line_start_m << ":"
                          << position2.position_m << ":";
                if (position2.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position2.line_start_m,
                                                      m_str.begin() + position2.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
#if REQUIRE_EXACT_MATCH
                push_back(m_array, position2.stream_name());
                push_back(m_array, position2.line_number_m);
                push_back(m_array, std::size_t(position2.line_start_m));
                push_back(m_array, std::size_t(position2.position_m));
#endif
            }
            push_back(m_array, expression);
#if REQUIRE_EXACT_MATCH
            push_back(m_array, brief);
            push_back(m_array, detailed);
#endif
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

}

BOOST_AUTO_TEST_CASE( adam_writer )
{
    std::string file_contents = read_file(g_input_file);

    adobe::array_t new_parse;

    adobe::adam_callback_suite_t new_parse_callbacks;
    new_parse_callbacks.add_cell_proc_m = GG::StoreAddCellParams(new_parse, file_contents);
    new_parse_callbacks.add_relation_proc_m = GG::StoreAddRelationParams(new_parse, file_contents);
    new_parse_callbacks.add_interface_proc_m = GG::StoreAddInterfaceParams(new_parse, file_contents);

    std::cout << "sheet:\"\n" << file_contents << "\n\"\n"
              << "filename: " << g_input_file << '\n';
    bool new_parse_failed = !GG::Parse(file_contents, g_input_file, new_parse_callbacks);
    std::cout << "new:      <parse " << (new_parse_failed ? "failure" : "success") << ">\n";

    adobe::vm_lookup_t vm_lookup;
    adobe::sheet_t adobe_sheet;
    vm_lookup.attach_to(adobe_sheet);
    vm_lookup.attach_to(adobe_sheet.machine_m);
    GG::Parse(file_contents, g_input_file, adobe::bind_to_sheet(adobe_sheet));
    std::stringstream os;
    adobe_sheet.print(os);
    adobe::array_t round_trip_parse;
    adobe::adam_callback_suite_t round_trip_parse_callbacks;
    round_trip_parse_callbacks.add_cell_proc_m =
        GG::StoreAddCellParams(round_trip_parse, os.str());
    round_trip_parse_callbacks.add_relation_proc_m =
        GG::StoreAddRelationParams(round_trip_parse, os.str());
    round_trip_parse_callbacks.add_interface_proc_m =
        GG::StoreAddInterfaceParams(round_trip_parse, os.str());
    bool round_trip_parse_pass =
        GG::Parse(os.str(), g_input_file, round_trip_parse_callbacks);
    bool pass =
        !round_trip_parse_pass && new_parse_failed ||
        round_trip_parse == new_parse;

    std::cout << "Round-trip parse: " << (pass ? "PASS" : "FAIL") << "\n\n";

    if (!pass) {
        std::cout << "rewritten sheet:\"\n" << os.str() << "\n\"\n";
        std::cout << "initial (verbose):\n";
        GG::verbose_dump(new_parse);
        std::cout << "roud-trip (verbose):\n";
        GG::verbose_dump(round_trip_parse);
    }

    BOOST_CHECK(pass);
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
    g_input_file = argv[1];
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
