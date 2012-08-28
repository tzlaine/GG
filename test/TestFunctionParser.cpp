#include <GG/FunctionParser.h>

#include <GG/FunctionWriter.h>

#include <fstream>
#include <iostream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


const char* g_input_file = 0;

bool instrument_positions = false;


BOOST_AUTO_TEST_CASE( function_parser )
{
    std::string file_contents = read_file(g_input_file);

    std::cout << "functions:\"\n" << file_contents << "\n\"\n"
              << "filename: " << g_input_file << '\n';

    GG::AdamFunctions functions;
    bool pass = GG::ParseFunctions(file_contents, g_input_file, functions);
    std::cout << (pass ? "PASS" : "FAIL") << "\n";

    std::cout << functions.size() << " functions\n";
    for (GG::AdamFunctions::iterator it = functions.begin();
         it != functions.end();
         ++it) {
        std::cout << GG::WriteFunction(it->second) << "\n\n";
    }

    std::cout << "\n";

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
