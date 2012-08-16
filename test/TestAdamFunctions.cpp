#if USE_SDL_BACKEND
#include "SDLBackend.h"
#else
#include "OgreBackend.h"
#endif

#include <GG/EveGlue.h>
#include <GG/EveParser.h>
#include <GG/Filesystem.h>
#include <GG/FunctionParser.h>
#include <GG/GUI.h>
#include <GG/Timer.h>
#include <GG/Wnd.h>
#include <GG/adobe/adam.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <fstream>

#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "TestingUtils.h"


struct Test
{
    Test() {}
    Test(const char* expression,
         const adobe::any_regular_t& expected_result,
         bool expect_exception = false) :
        m_expression(expression),
        m_expected_result(expected_result),
        m_expect_exception(expect_exception)
        {}
    const char* m_expression;
    adobe::any_regular_t m_expected_result;
    bool m_expect_exception;
};

const GG::AdamFunctions& AdamFunctions()
{
    static GG::AdamFunctions retval;
    if (retval.empty()) {
        std::string functions = read_file("function_parser_test.fn");
        bool parse_result =
            GG::ParseFunctions(functions, "function_parser_test.fn", retval);
        BOOST_CHECK(parse_result);
    }
    return retval;
}

const std::vector<Test>& Tests()
{
    static std::vector<Test> retval;
    if (retval.empty()) {
        retval.push_back(Test("empty_fn()", adobe::any_regular_t()));
        retval.push_back(Test("simple_number_fn()", adobe::any_regular_t(1)));
        retval.push_back(Test("decl_fn()", adobe::any_regular_t(adobe::static_name_t("none"))));
        retval.push_back(Test("const_decl_fn()", adobe::any_regular_t(adobe::static_name_t("none"))));
        retval.push_back(Test("const_cell_fn()", adobe::any_regular_t(adobe::static_name_t("const_cell_contents"))));
        retval.push_back(Test("interface_cell_fn()", adobe::any_regular_t(adobe::static_name_t("interface_cell_contents"))));
        retval.push_back(Test("assign_cell_1_fn()", adobe::any_regular_t()));
        retval.push_back(Test("assign_cell_2_fn()", adobe::any_regular_t()));
        retval.push_back(Test("shadowed_param_1_fn()", adobe::any_regular_t()));
        retval.push_back(Test("shadowed_param_1_fn(empty)", adobe::any_regular_t()));
        retval.push_back(Test("shadowed_param_1_fn(1)", adobe::any_regular_t(1)));
        retval.push_back(Test("shadowed_param_2_fn()", adobe::any_regular_t(adobe::array_t())));
        retval.push_back(Test("shadowed_param_3_fn()", adobe::any_regular_t(adobe::array_t())));
        retval.push_back(Test("assignment_fn()", adobe::any_regular_t(adobe::dictionary_t())));
        retval.push_back(Test("simple_if_1_fn(true)", adobe::any_regular_t(1)));
        retval.push_back(Test("simple_if_1_fn(false)", adobe::any_regular_t()));
        retval.push_back(Test("simple_if_2_fn(true)", adobe::any_regular_t(1)));
        retval.push_back(Test("simple_if_2_fn(false)", adobe::any_regular_t(0)));
        retval.push_back(Test("nested_ifs_fn(true, true, true)", adobe::any_regular_t(0)));
        retval.push_back(Test("nested_ifs_fn(true, true, false)", adobe::any_regular_t(1)));
        retval.push_back(Test("nested_ifs_fn(true, false, true)", adobe::any_regular_t(2)));
        retval.push_back(Test("nested_ifs_fn(true, false, false)", adobe::any_regular_t(3)));
        retval.push_back(Test("nested_ifs_fn(false, true, true)", adobe::any_regular_t(4)));
        retval.push_back(Test("nested_ifs_fn(false, true, false)", adobe::any_regular_t(5)));
        retval.push_back(Test("nested_ifs_fn(false, false, true)", adobe::any_regular_t(6)));
        retval.push_back(Test("nested_ifs_fn(false, false, false)", adobe::any_regular_t(7)));
        retval.push_back(Test("chained_ifs_fn(true, true, true)", adobe::any_regular_t(0)));
        retval.push_back(Test("chained_ifs_fn(false, true, true)", adobe::any_regular_t(1)));
        retval.push_back(Test("chained_ifs_fn(false, false, true)", adobe::any_regular_t(2)));
        retval.push_back(Test("chained_ifs_fn(false, false, false)", adobe::any_regular_t(3)));
        retval.push_back(Test("slow_size([])", adobe::any_regular_t(0)));
        retval.push_back(Test("slow_size([0])", adobe::any_regular_t(1)));
        retval.push_back(Test("slow_size([0, @two])", adobe::any_regular_t(2)));
        retval.push_back(Test("slow_size({})", adobe::any_regular_t(0)));
        retval.push_back(Test("slow_size({one: 0})", adobe::any_regular_t(1)));
        retval.push_back(Test("slow_size({one: 0, two: @two})", adobe::any_regular_t(2)));
    }
    return retval;
}

void CheckResult(const GG::EveDialog& eve_dialog, const adobe::any_regular_t& expected_result)
{
    GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LPRESS, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), GG::Pt(GG::X(48), GG::Y(22)), GG::Pt());
    GG::GUI::GetGUI()->HandleGGEvent(GG::GUI::LRELEASE, GG::Key(), boost::uint32_t(), GG::Flags<GG::ModKey>(), GG::Pt(GG::X(48), GG::Y(22)), GG::Pt());
    BOOST_CHECK(eve_dialog.TerminatingAction());
    BOOST_CHECK(eve_dialog.Result().find(adobe::static_name_t("const_cell"))->second ==
                adobe::any_regular_t(adobe::static_name_t("const_cell_contents")));
    BOOST_CHECK(eve_dialog.Result().find(adobe::static_name_t("interface_cell"))->second ==
                adobe::any_regular_t(adobe::static_name_t("interface_cell_contents")));
    BOOST_CHECK(eve_dialog.Result().find(adobe::static_name_t("value"))->second ==
                expected_result);
    if (eve_dialog.Result().find(adobe::static_name_t("value"))->second != expected_result) {
        std::cout << eve_dialog.Result().find(adobe::static_name_t("value"))->second << " != "
                  << expected_result << '\n';
    }
}

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return false; }

void RunTest(std::size_t i)
{
    const Test& test = Tests()[i];
    boost::filesystem::path eve("function_test_dialog.eve");
    std::string adam_str = "sheet function_test_dialog {\n";
    adam_str += "    constant: const_cell: @const_cell_contents;\n";
    adam_str += "    interface: interface_cell: @interface_cell_contents;\n";
    adam_str += "    output: result <== { value: ";
    adam_str += test.m_expression;
    adam_str += ", const_cell: const_cell, interface_cell: interface_cell };\n";
    adam_str += "}";
    boost::filesystem::ifstream eve_stream(eve);
    std::istringstream adam_stream(adam_str);
    if (test.m_expect_exception) {
        BOOST_CHECK_THROW(
            (GG::MakeEveDialog(eve_stream,
                               "function_test_dialog.eve",
                               adam_stream,
                               "inline Adam expression",
                               GG::DictionaryFunctions(),
                               GG::ArrayFunctions(),
                               AdamFunctions(),
                               &ButtonHandler)),
            adobe::stream_error_t
        );
        return;
    }
    GG::EveDialog* eve_dialog =
        GG::MakeEveDialog(eve_stream,
                          "function_test_dialog.eve",
                          adam_stream,
                          "inline Adam expression",
                          GG::DictionaryFunctions(),
                          GG::ArrayFunctions(),
                          AdamFunctions(),
                          &ButtonHandler);
    GG::Timer timer(25);
    GG::Connect(timer.FiredSignal,
                boost::bind(&CheckResult,
                            boost::cref(*eve_dialog),
                            boost::cref(test.m_expected_result)));
    eve_dialog->Run();
}

void CustomInit()
{
    for (std::size_t i = 0; i < Tests().size(); ++i) {
        RunTest(i);
    }

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
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
