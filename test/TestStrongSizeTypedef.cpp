#include <GG/FontFwd.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE StrongSizeTypedef

#include <boost/test/unit_test.hpp>

std::size_t i = 7;
GG::StrSize x(2);

std::size_t temp_size;

#define ARITHMETIC_ITERATION(op)                                   \
    BOOST_CHECK_EQUAL(Value(i op x), i op Value(x));               \
    BOOST_CHECK_EQUAL(Value(x op i), Value(x) op i);               \
    BOOST_CHECK_EQUAL(Value(x op x), Value(x) op Value(x))

#define RESET()                                 \
    i = 7;                                      \
    x = GG::StrSize(2)

#define ASSIGN_ARITHMETIC_ITERATION(op)                                 \
    BOOST_CHECK_EQUAL(Value(x op i), (temp_size = Value(x)) op i); RESET(); \
    BOOST_CHECK_EQUAL(Value(x op x), (temp_size = Value(x)) op (temp_size = Value(x))); RESET()

#define COMPARISON_ITERATION(op)                                 \
    BOOST_CHECK_EQUAL(i op x, i op Value(x));                    \
    BOOST_CHECK_EQUAL(x op i, Value(x) op i);                    \
    BOOST_CHECK_EQUAL(x op x, Value(x) op Value(x))

BOOST_AUTO_TEST_CASE( arithmetic )
{
    ARITHMETIC_ITERATION(+);
    ARITHMETIC_ITERATION(-);
    ARITHMETIC_ITERATION(*);
    BOOST_CHECK_EQUAL(Value(x / i), Value(x) / i);
    BOOST_CHECK_EQUAL(Value(x / x), Value(x) / Value(x));

    ASSIGN_ARITHMETIC_ITERATION(+=);
    ASSIGN_ARITHMETIC_ITERATION(-=);
    ASSIGN_ARITHMETIC_ITERATION(*=);
    ASSIGN_ARITHMETIC_ITERATION(/=);
}

BOOST_AUTO_TEST_CASE( comparison )
{
    COMPARISON_ITERATION(<);
    COMPARISON_ITERATION(>);
    COMPARISON_ITERATION(==);
    COMPARISON_ITERATION(!=);
    COMPARISON_ITERATION(<=);
    COMPARISON_ITERATION(>=);
}

BOOST_AUTO_TEST_CASE( unary )
{
    BOOST_CHECK_EQUAL(++x, ++(temp_size = Value(x)));
    BOOST_CHECK_EQUAL(x++, (temp_size = Value(x))++);
    BOOST_CHECK_EQUAL(x, temp_size);

    BOOST_CHECK_EQUAL(--x, --(temp_size = Value(x)));
    BOOST_CHECK_EQUAL(x--, (temp_size = Value(x))--);
    BOOST_CHECK_EQUAL(x, temp_size);

    BOOST_CHECK_EQUAL(!x, !Value(x));

    BOOST_CHECK_EQUAL(-x, -Value(x));
}
