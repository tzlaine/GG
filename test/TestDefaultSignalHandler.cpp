#include <GG/EveGlue.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE DefaultSignalHandler

#include <boost/test/unit_test.hpp>


bool test_run_flag = false;

struct reset_flag
{
    reset_flag(bool& f) : m_flag(f) { m_flag = false; }
    ~reset_flag() { m_flag = false; }
    bool& m_flag;
};

const adobe::any_regular_t any_any_any_value(adobe::name_t("any_any_any"));
const adobe::any_regular_t any_any_baz_value(adobe::name_t("any_any_baz"));
const adobe::any_regular_t any_bar_any_value(adobe::name_t("any_bar_any"));
const adobe::any_regular_t any_bar_baz_value(adobe::name_t("any_bar_baz"));
const adobe::any_regular_t foo_any_any_value(adobe::name_t("foo_any_any"));
const adobe::any_regular_t foo_any_baz_value(adobe::name_t("foo_any_baz"));
const adobe::any_regular_t foo_bar_any_value(adobe::name_t("foo_bar_any"));
const adobe::any_regular_t foo_bar_baz_value(adobe::name_t("foo_bar_baz"));

const adobe::any_regular_t foo_baz_bar_value(adobe::name_t("foo_baz_bar"));

void any_any_any_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, any_any_any_value);
}

void any_any_baz_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, any_any_baz_value);
}

void any_bar_any_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, any_bar_any_value);
}

void any_bar_baz_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, any_bar_baz_value);
}

void foo_any_any_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, foo_any_any_value);
}

void foo_any_baz_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, foo_any_baz_value);
}

void foo_bar_any_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, foo_bar_any_value);
}

void foo_bar_baz_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, foo_bar_baz_value);
}


void foo_baz_bar_fn(adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t& value)
{
    test_run_flag = true;
    BOOST_CHECK_EQUAL(value, foo_baz_bar_value);
}


const adobe::name_t foo("foo");
const adobe::name_t bar("bar");
const adobe::name_t baz("baz");


BOOST_AUTO_TEST_CASE( single_function )
{
    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler(foo, bar, baz, any_any_any_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler(foo, bar, baz, any_any_baz_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_bar_any_fn);
        default_handler(foo, bar, baz, any_bar_any_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   baz,
                                   any_bar_baz_fn);
        default_handler(foo, bar, baz, any_bar_baz_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_any_any_fn);
        default_handler(foo, bar, baz, foo_any_any_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   foo_any_baz_fn);
        default_handler(foo, bar, baz, foo_any_baz_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(foo,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_bar_any_fn);
        default_handler(foo, bar, baz, foo_bar_any_value);
        BOOST_CHECK(test_run_flag);
    }

    {
        reset_flag rf(test_run_flag);
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(foo,
                                   bar,
                                   baz,
                                   foo_bar_baz_fn);
        default_handler(foo, bar, baz, foo_bar_baz_value);
        BOOST_CHECK(test_run_flag);
    }
}


BOOST_AUTO_TEST_CASE( two_functions )
{
    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_bar_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   baz,
                                   any_bar_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_any_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   foo_any_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(foo,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_bar_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_any_any_fn);
        default_handler.SetHandler(foo,
                                   bar,
                                   baz,
                                   foo_bar_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   any_bar_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   baz,
                                   any_bar_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, any_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_any_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   foo_any_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(foo,
                                   bar,
                                   GG::DefaultSignalHandler::any_widget_id,
                                   foo_bar_any_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_any_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   any_any_baz_fn);
        default_handler.SetHandler(foo,
                                   bar,
                                   baz,
                                   foo_bar_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   baz,
                                   any_bar_baz_fn);
        default_handler.SetHandler(foo,
                                   GG::DefaultSignalHandler::any_signal,
                                   baz,
                                   foo_any_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_any_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }

    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(GG::DefaultSignalHandler::any_widget_type,
                                   bar,
                                   baz,
                                   any_bar_baz_fn);
        default_handler.SetHandler(foo,
                                   bar,
                                   baz,
                                   foo_bar_baz_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
    }


    {
        GG::DefaultSignalHandler default_handler;
        default_handler.SetHandler(foo,
                                   bar,
                                   baz,
                                   foo_bar_baz_fn);
        default_handler.SetHandler(foo,
                                   baz,
                                   bar,
                                   foo_baz_bar_fn);
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, bar, baz, foo_bar_baz_value);
            BOOST_CHECK(test_run_flag);
        }
        {
            reset_flag rf(test_run_flag);
            default_handler(foo, baz, bar, foo_baz_bar_value);
            BOOST_CHECK(test_run_flag);
        }
    }
}
