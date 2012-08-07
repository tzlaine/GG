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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

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

adobe::any_regular_t push_back(const adobe::dictionary_t& parameters)
{
    adobe::any_regular_t state;
    get_value(parameters, adobe::static_name_t("state"), state);
    adobe::array_t& array = state.cast<adobe::array_t>();
    adobe::any_regular_t value;
    get_value(parameters, adobe::static_name_t("value"), value);
    array.push_back(value);
    return state;
}

adobe::any_regular_t push_back_key(const adobe::dictionary_t& parameters)
{
    adobe::any_regular_t state;
    get_value(parameters, adobe::static_name_t("state"), state);
    adobe::array_t& array = state.cast<adobe::array_t>();
    adobe::any_regular_t key;
    get_value(parameters, adobe::static_name_t("key"), key);
    array.push_back(key);
    return state;
}

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

const std::vector<Test>& Tests()
{
    static std::vector<Test> retval;
    if (retval.empty()) {
        {
            adobe::dictionary_t result;
            result[adobe::static_name_t("one")] = adobe::any_regular_t(2.0);
            result[adobe::static_name_t("two")] = adobe::any_regular_t(3.0);
            retval.push_back(Test("transform({one: 1, two: 2}, @increment)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(4.0));
            result.push_back(adobe::any_regular_t(5.0));
            retval.push_back(Test("transform([3, 4], @increment)", adobe::any_regular_t(result)));
        }
        retval.push_back(Test("transform(5, @increment)", adobe::any_regular_t(6.0)));

        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(adobe::static_name_t("one")));
            result.push_back(adobe::any_regular_t(adobe::static_name_t("two")));
            result.push_back(adobe::any_regular_t(adobe::static_name_t("three")));
            retval.push_back(Test("fold({one: 1, two: 2, three: 3}, [], @push_back_key)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(4.0));
            result.push_back(adobe::any_regular_t(5.0));
            result.push_back(adobe::any_regular_t(6.0));
            retval.push_back(Test("fold([4, 5, 6], [], @push_back)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            retval.push_back(Test("fold(7, [], @push_back)", adobe::any_regular_t(result)));
        }

        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(3.0));
            result.push_back(adobe::any_regular_t(2.0));
            result.push_back(adobe::any_regular_t(1.0));
            retval.push_back(Test("foldr({one: 1, two: 2, three: 3}, [], @push_back)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(6.0));
            result.push_back(adobe::any_regular_t(5.0));
            result.push_back(adobe::any_regular_t(4.0));
            retval.push_back(Test("foldr([4, 5, 6], [], @push_back)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            retval.push_back(Test("foldr(7, [], @push_back)", adobe::any_regular_t(result)));
        }

        retval.push_back(Test("append()", adobe::any_regular_t(), true));
        {
            adobe::array_t result;
            retval.push_back(Test("append([])", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            retval.push_back(Test("append([], 7)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            result.push_back(adobe::any_regular_t(std::string("8")));
            retval.push_back(Test("append([7], '8')", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            result.push_back(adobe::any_regular_t(std::string("8")));
            result.push_back(adobe::any_regular_t(adobe::name_t("nine")));
            retval.push_back(Test("append([7], '8', @nine)", adobe::any_regular_t(result)));
        }

        retval.push_back(Test("prepend()", adobe::any_regular_t(), true));
        {
            adobe::array_t result;
            retval.push_back(Test("prepend([])", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            retval.push_back(Test("prepend([], 7)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            result.push_back(adobe::any_regular_t(std::string("8")));
            retval.push_back(Test("prepend(['8'], 7)", adobe::any_regular_t(result)));
        }
        {
            adobe::array_t result;
            result.push_back(adobe::any_regular_t(7.0));
            result.push_back(adobe::any_regular_t(std::string("8")));
            result.push_back(adobe::any_regular_t(adobe::name_t("nine")));
            retval.push_back(Test("prepend([@nine], 7, '8')", adobe::any_regular_t(result)));
        }
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
{ return false; }

void RunTest(std::size_t i)
{
    const Test& test = Tests()[i];
    boost::filesystem::path eve("function_test_dialog.eve");
    std::string adam_str =
        "sheet function_test_dialog { output: result <== { value: ";
    adam_str += test.m_expression;
    adam_str += " }; }";
    boost::filesystem::ifstream eve_stream(eve);
    std::istringstream adam_stream(adam_str);
    if (test.m_expect_exception) {
        BOOST_CHECK_THROW(
            (GG::MakeEveDialog(eve_stream,
                               "function_test_dialog.eve",
                               adam_stream,
                               "inline Adam expression",
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
                          &ButtonHandler);
    GG::Timer timer(100);
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
    GG::RegisterDictionaryFunction(adobe::static_name_t("increment"), &increment);
    GG::RegisterDictionaryFunction(adobe::static_name_t("push_back"), &push_back);
    GG::RegisterDictionaryFunction(adobe::static_name_t("push_back_key"), &push_back_key);
    return ::boost::unit_test::unit_test_main( &init_unit_test, argc, argv );
}
