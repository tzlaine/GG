// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

/** \file StrongTypedef.h \brief Contains macros used to create "strong
    typedefs", that is value types that are not mutually interoperable with
    each other or with builtin types for extra type safety. */

#ifndef _GG_StrongTypedef_h_
#define _GG_StrongTypedef_h_

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/serialization/nvp.hpp>

#include <iostream>


#define GG_STRONG_DOUBLE_TYPEDEF(name)                                  \
    class name;                                                         \
    class name ## _d;                                                   \
    double Value(name ## _d x);                                         \
                                                                        \
    class name ## _d                                                    \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        name ## _d() {}                                                 \
        explicit name ## _d(double t) : m_value(t) {}                   \
                                                                        \
        bool operator<(name ## _d rhs) const                            \
        { return m_value < rhs.m_value; }                               \
        bool operator==(name ## _d rhs) const                           \
        { return m_value == rhs.m_value; }                              \
                                                                        \
        bool operator<(double rhs) const                                \
        { return m_value < rhs; }                                       \
        bool operator==(double rhs) const                               \
        { return m_value == rhs; }                                      \
                                                                        \
        operator int ConvertibleToBoolDummy::* () const                 \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        name ## _d operator-() const                                    \
        { return name ## _d(-m_value); }                                \
                                                                        \
        name ## _d& operator++()                                        \
        { ++m_value; return *this; }                                    \
        name ## _d& operator--()                                        \
        { --m_value; return *this; }                                    \
                                                                        \
        name ## _d operator++(int)                                      \
        { name ## _d retval(m_value); ++m_value; return retval; }       \
        name ## _d operator--(int)                                      \
        { name ## _d retval(m_value); --m_value; return retval; }       \
                                                                        \
        name ## _d& operator+=(name ## _d rhs)                          \
        { m_value += rhs.m_value; return *this; }                       \
        name ## _d& operator-=(name ## _d rhs)                          \
        { m_value -= rhs.m_value; return *this; }                       \
        name ## _d& operator*=(name ## _d rhs)                          \
        { m_value *= rhs.m_value; return *this; }                       \
        name ## _d& operator/=(name ## _d rhs)                          \
        { m_value /= rhs.m_value; return *this; }                       \
                                                                        \
        name ## _d& operator+=(double rhs)                              \
        { m_value += rhs; return *this; }                               \
        name ## _d& operator-=(double rhs)                              \
        { m_value -= rhs; return *this; }                               \
        name ## _d& operator*=(double rhs)                              \
        { m_value *= rhs; return *this; }                               \
        name ## _d& operator/=(double rhs)                              \
        { m_value /= rhs; return *this; }                               \
                                                                        \
        name ## _d& operator+=(name rhs);                               \
        name ## _d& operator-=(name rhs);                               \
        name ## _d& operator*=(name rhs);                               \
        name ## _d& operator/=(name rhs);                               \
                                                                        \
        name ## _d operator+(name rhs);                                 \
        name ## _d operator-(name rhs);                                 \
        name ## _d operator*(name rhs);                                 \
        name ## _d operator/(name rhs);                                 \
                                                                        \
    private:                                                            \
        double m_value;                                                 \
                                                                        \
        friend class boost::serialization::access;                      \
        template <class Archive>                                        \
        void serialize(Archive& ar, const unsigned int version)         \
        { ar & BOOST_SERIALIZATION_NVP(m_value); }                      \
                                                                        \
        friend double Value(name ## _d x);                              \
    };                                                                  \
                                                                        \
    inline bool operator>(name ## _d x, name ## _d y)                   \
    { return y < x; }                                                   \
    inline bool operator<=(name ## _d x, name ## _d y)                  \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name ## _d x, name ## _d y)                  \
    { return y < x || x == y; }                                         \
                                                                        \
    inline bool operator!=(name ## _d x, double y)                      \
    { return !(x == y); }                                               \
    inline bool operator>(name ## _d x, double y)                       \
    { return !(x < y || x == y); }                                      \
    inline bool operator<=(name ## _d x, double y)                      \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name ## _d x, double y)                      \
    { return !(x < y); }                                                \
    inline bool operator>(double x, name ## _d y)                       \
    { return y < x; }                                                   \
    inline bool operator==(double x, name ## _d y)                      \
    { return y == x; }                                                  \
    inline bool operator!=(double x, name ## _d y)                      \
    { return y != x; }                                                  \
    inline bool operator<(double x, name ## _d y)                       \
    { return !(y < x || y == x); }                                      \
    inline bool operator<=(double x, name ## _d y)                      \
    { return !(y < x); }                                                \
    inline bool operator>=(double x, name ## _d y)                      \
    { return y < x || y == x; }                                         \
                                                                        \
    inline name ## _d operator+(name ## _d lhs, name ## _d rhs)         \
    { return lhs += rhs; }                                              \
    inline name ## _d operator-(name ## _d lhs, name ## _d rhs)         \
    { return lhs -= rhs; }                                              \
    inline name ## _d operator*(name ## _d lhs, name ## _d rhs)         \
    { return lhs *= rhs; }                                              \
    inline name ## _d operator/(name ## _d lhs, name ## _d rhs)         \
    { return lhs /= rhs; }                                              \
                                                                        \
    inline name ## _d operator+(name ## _d lhs, double rhs)             \
    { return lhs += rhs; }                                              \
    inline name ## _d operator-(name ## _d lhs, double rhs)             \
    { return lhs -= rhs; }                                              \
    inline name ## _d operator*(name ## _d lhs, double rhs)             \
    { return lhs *= rhs; }                                              \
    inline name ## _d operator/(name ## _d lhs, double rhs)             \
    { return lhs /= rhs; }                                              \
                                                                        \
    inline name ## _d operator+(double lhs, name ## _d rhs)             \
    { return rhs += lhs; }                                              \
    inline name ## _d operator-(double lhs, name ## _d rhs)             \
    { return -(rhs -= lhs); }                                           \
    inline name ## _d operator*(double lhs, name ## _d rhs)             \
    { return rhs *= lhs; }                                              \
                                                                        \
    inline double Value(name ## _d x)                                   \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name ## _d x)     \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name ## _d& x)    \
    {                                                                   \
        double t;                                                       \
        os >> t;                                                        \
        x = name ## _d(t);                                              \
        return os;                                                      \
    }                                                                   \
                                                                        \
    void dummy_function_to_force_semicolon()

/** Creates a new type \a name, based on underlying type \a type, which is not
    interconvertible with any other numeric type.  \a type must be an integral
    type.  The resulting type has most of the operations of the underlying
    integral type.  Specifically, the type is totally ordered, incrementable,
    decrementable, and arithmetic.  The type is also interarithemtic with and
    comparable to objects of types \a type and double.  Note that the free
    functions accepting doubles return GG_STRONG_DOUBLE_TYPEDEF's called \a
    name_d.  This allows \a name objects to be used in floating point math. */
#define GG_STRONG_INTEGRAL_TYPEDEF(name, type)                          \
    GG_STRONG_DOUBLE_TYPEDEF(name);                                     \
                                                                        \
    type Value(name x);                                                 \
                                                                        \
    class name                                                          \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        BOOST_STATIC_ASSERT((boost::is_integral<type>::value));         \
                                                                        \
        name() {}                                                       \
        explicit name(type t) : m_value(t) {}                           \
        explicit name(name ## _d t) :                                   \
            m_value(static_cast<type>(Value(t)))                        \
        {}                                                              \
                                                                        \
        name& operator=(name ## _d t)                                   \
        { m_value = static_cast<type>(Value(t)); return *this; }        \
                                                                        \
        bool operator<(name rhs) const                                  \
        { return m_value < rhs.m_value; }                               \
        bool operator==(name rhs) const                                 \
        { return m_value == rhs.m_value; }                              \
                                                                        \
        bool operator<(type rhs) const                                  \
        { return m_value < rhs; }                                       \
        bool operator==(type rhs) const                                 \
        { return m_value == rhs; }                                      \
        bool operator!=(type rhs) const                                 \
        { return m_value != rhs; }                                      \
                                                                        \
        bool operator<(name ## _d rhs) const                            \
        { return m_value < Value(rhs); }                                \
        bool operator==(name ## _d rhs) const                           \
        { return m_value == Value(rhs); }                               \
        bool operator!=(name ## _d rhs) const                           \
        { return m_value != Value(rhs); }                               \
                                                                        \
        bool operator<(double rhs) const                                \
        { return m_value < rhs; }                                       \
        bool operator==(double rhs) const                               \
        { return m_value == rhs; }                                      \
        bool operator!=(double rhs) const                               \
        { return m_value != rhs; }                                      \
                                                                        \
        operator int ConvertibleToBoolDummy::* () const                 \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        name operator-() const                                          \
        { return name(-m_value); }                                      \
                                                                        \
        name& operator++()                                              \
        { ++m_value; return *this; }                                    \
        name& operator--()                                              \
        { --m_value; return *this; }                                    \
                                                                        \
        name operator++(int)                                            \
        { name retval(m_value); ++m_value; return retval; }             \
        name operator--(int)                                            \
        { name retval(m_value); --m_value; return retval; }             \
                                                                        \
        name& operator+=(name rhs)                                      \
        { m_value += rhs.m_value; return *this; }                       \
        name& operator-=(name rhs)                                      \
        { m_value -= rhs.m_value; return *this; }                       \
        name& operator*=(name rhs)                                      \
        { m_value *= rhs.m_value; return *this; }                       \
        name& operator/=(name rhs)                                      \
        { m_value /= rhs.m_value; return *this; }                       \
        name& operator%=(name rhs)                                      \
        { m_value %= rhs.m_value; return *this; }                       \
                                                                        \
        name& operator+=(type rhs)                                      \
        { m_value += rhs; return *this; }                               \
        name& operator-=(type rhs)                                      \
        { m_value -= rhs; return *this; }                               \
        name& operator*=(type rhs)                                      \
        { m_value *= rhs; return *this; }                               \
        name& operator/=(type rhs)                                      \
        { m_value /= rhs; return *this; }                               \
        name& operator%=(type rhs)                                      \
        { m_value %= rhs; return *this; }                               \
                                                                        \
        name& operator+=(name ## _d rhs)                                \
        { return operator+=(Value(rhs)); }                              \
        name& operator-=(name ## _d rhs)                                \
        { return operator-=(Value(rhs)); }                              \
        name& operator*=(name ## _d rhs)                                \
        { return operator*=(Value(rhs)); }                              \
        name& operator/=(name ## _d rhs)                                \
        { return operator/=(Value(rhs)); }                              \
                                                                        \
        name& operator+=(double rhs)                                    \
        { m_value = static_cast<type>(m_value + rhs); return *this; }   \
        name& operator-=(double rhs)                                    \
        { m_value = static_cast<type>(m_value - rhs); return *this; }   \
        name& operator*=(double rhs)                                    \
        { m_value = static_cast<type>(m_value * rhs); return *this; }   \
        name& operator/=(double rhs)                                    \
        { m_value = static_cast<type>(m_value / rhs); return *this; }   \
                                                                        \
    private:                                                            \
        type m_value;                                                   \
                                                                        \
        friend class boost::serialization::access;                      \
        template <class Archive>                                        \
        void serialize(Archive& ar, const unsigned int version)         \
        { ar & BOOST_SERIALIZATION_NVP(m_value); }                      \
                                                                        \
        friend class name ## _d;                                        \
        friend type Value(name x);                                      \
    };                                                                  \
                                                                        \
    inline name operator+(name lhs, name rhs)                           \
    { return lhs += rhs; }                                              \
    inline name operator-(name lhs, name rhs)                           \
    { return lhs -= rhs; }                                              \
    inline name operator*(name lhs, name rhs)                           \
    { return lhs *= rhs; }                                              \
    inline name operator/(name lhs, name rhs)                           \
    { return lhs /= rhs; }                                              \
    inline name operator%(name lhs, name rhs)                           \
    { return lhs %= rhs; }                                              \
                                                                        \
    inline name operator+(name lhs, type rhs)                           \
    { return lhs += rhs; }                                              \
    inline name operator-(name lhs, type rhs)                           \
    { return lhs -= rhs; }                                              \
    inline name operator*(name lhs, type rhs)                           \
    { return lhs *= rhs; }                                              \
    inline name operator/(name lhs, type rhs)                           \
    { return lhs /= rhs; }                                              \
    inline name operator%(name lhs, type rhs)                           \
    { return lhs %= rhs; }                                              \
                                                                        \
    inline name operator+(type lhs, name rhs)                           \
    { return rhs += lhs; }                                              \
    inline name operator-(type lhs, name rhs)                           \
    { return -(rhs -= lhs); }                                           \
    inline name operator*(type lhs, name rhs)                           \
    { return rhs *= lhs; }                                              \
                                                                        \
    inline name ## _d operator+(name x, double y)                       \
    { return name ## _d(Value(x)) + y; }                                \
    inline name ## _d operator+(double x, name y)                       \
    { return x + name ## _d(Value(y)); }                                \
                                                                        \
    inline name ## _d operator-(name x, double y)                       \
    { return name ## _d(Value(x)) - y; }                                \
    inline name ## _d operator-(double x, name y)                       \
    { return x - name ## _d(Value(y)); }                                \
                                                                        \
    inline name ## _d operator*(name x, double y)                       \
    { return name ## _d(Value(x)) * y; }                                \
    inline name ## _d operator*(double x, name y)                       \
    { return x * name ## _d(Value(y)); }                                \
                                                                        \
    inline name ## _d operator/(name x, double y)                       \
    { return name ## _d(Value(x)) / y; }                                \
                                                                        \
    inline bool operator!=(name x, name y)                              \
    { return !(x == y); }                                               \
    inline bool operator>(name x, name y)                               \
    { return y < x; }                                                   \
    inline bool operator<=(name x, name y)                              \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, name y)                              \
    { return y < x || x == y; }                                         \
                                                                        \
    inline bool operator>(name x, type y)                               \
    { return !(x < y || x == y); }                                      \
    inline bool operator<=(name x, type y)                              \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, type y)                              \
    { return !(x < y); }                                                \
    inline bool operator==(type x, name y)                              \
    { return y == x; }                                                  \
    inline bool operator!=(type x, name y)                              \
    { return y != x; }                                                  \
    inline bool operator>(type x, name y)                               \
    { return y < x; }                                                   \
    inline bool operator<(type x, name y)                               \
    { return !(y < x || y == x); }                                      \
    inline bool operator<=(type x, name y)                              \
    { return !(y < x); }                                                \
    inline bool operator>=(type x, name y)                              \
    { return y < x || y == x; }                                         \
                                                                        \
    inline bool operator>(name x, name ## _d y)                         \
    { return !(x < y || x == y); }                                      \
    inline bool operator<=(name x, name ## _d y)                        \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, name ## _d y)                        \
    { return !(x < y); }                                                \
    inline bool operator!=(name ## _d x, name y)                        \
    { return Value(y) != Value(x); }                                    \
    inline bool operator==(name ## _d x, name y)                        \
    { return Value(y) == Value(x); }                                    \
    inline bool operator>(name ## _d x, name y)                         \
    { return y < x; }                                                   \
    inline bool operator<(name ## _d x, name y)                         \
    { return !(y < x || y == x); }                                      \
    inline bool operator<=(name ## _d x, name y)                        \
    { return !(y < x); }                                                \
    inline bool operator>=(name ## _d x, name y)                        \
    { return y < x || x == y; }                                         \
                                                                        \
    inline bool operator>(name x, double y)                             \
    { return !(x < y || x == y); }                                      \
    inline bool operator<=(name x, double y)                            \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, double y)                            \
    { return !(x < y); }                                                \
    inline bool operator>(double x, name y)                             \
    { return y < x; }                                                   \
    inline bool operator<(double x, name y)                             \
    { return !(y < x || y == x); }                                      \
    inline bool operator<=(double x, name y)                            \
    { return !(y < x); }                                                \
    inline bool operator>=(double x, name y)                            \
    { return y < x || x == y; }                                         \
                                                                        \
    inline type Value(name x)                                           \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name x)           \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name& x)          \
    {                                                                   \
        type t;                                                         \
        os >> t;                                                        \
        x = name(t);                                                    \
        return os;                                                      \
    }                                                                   \
                                                                        \
    inline name ## _d& name ## _d::operator+=(name rhs)                 \
    { m_value += rhs.m_value; return *this; }                           \
    inline name ## _d& name ## _d::operator-=(name rhs)                 \
    { m_value -= rhs.m_value; return *this; }                           \
    inline name ## _d& name ## _d::operator*=(name rhs)                 \
    { m_value *= rhs.m_value; return *this; }                           \
    inline name ## _d& name ## _d::operator/=(name rhs)                 \
    { m_value /= rhs.m_value; return *this; }                           \
                                                                        \
    inline name ## _d operator+(name lhs, name ## _d rhs)               \
    { return rhs += lhs; }                                              \
    inline name ## _d operator-(name lhs, name ## _d rhs)               \
    { return lhs - Value(rhs); }                                        \
    inline name ## _d operator*(name lhs, name ## _d rhs)               \
    { return rhs *= lhs; }                                              \
    inline name ## _d operator/(name lhs, name ## _d rhs)               \
    { return lhs / Value(rhs); }                                        \
                                                                        \
    inline name ## _d name ## _d::operator+(name rhs)                   \
    { return name ## _d(m_value + rhs.m_value); }                       \
    inline name ## _d name ## _d::operator-(name rhs)                   \
    { return name ## _d(m_value - rhs.m_value); }                       \
    inline name ## _d name ## _d::operator*(name rhs)                   \
    { return name ## _d(m_value * rhs.m_value); }                       \
    inline name ## _d name ## _d::operator/(name rhs)                   \
    { return name ## _d(m_value / rhs.m_value); }                       \
                                                                        \
    void dummy_function_to_force_semicolon()

/** Creates a new type \a name, based on underlying type std::size_t, which is
    not interconvertible with any other numeric type.  The resulting type has
    most of the operations of std::size_t.  Specifically, the type is totally
    ordered, incrementable, decrementable, and arithmetic.  The type is also
    interarithemtic with and comparable to objects of type std::size_t. */
#define GG_STRONG_SIZE_TYPEDEF(name)                                    \
    class name;                                                         \
    std::size_t Value(name x);                                          \
                                                                        \
    class name                                                          \
    {                                                                   \
    private:                                                            \
        struct ConvertibleToBoolDummy {int _;};                         \
                                                                        \
    public:                                                             \
        name() {}                                                       \
        explicit name(std::size_t t) : m_value(t) {}                    \
                                                                        \
        bool operator<(name rhs) const                                  \
        { return m_value < rhs.m_value; }                               \
        bool operator==(name rhs) const                                 \
        { return m_value == rhs.m_value; }                              \
                                                                        \
        bool operator<(std::size_t rhs) const                           \
        { return m_value < rhs; }                                       \
        bool operator==(std::size_t rhs) const                          \
        { return m_value == rhs; }                                      \
                                                                        \
        operator int ConvertibleToBoolDummy::* () const                 \
        { return m_value ? &ConvertibleToBoolDummy::_ : 0; }            \
                                                                        \
        name operator-() const                                          \
        { return name(-m_value); }                                      \
                                                                        \
        name& operator++()                                              \
        { ++m_value; return *this; }                                    \
        name& operator--()                                              \
        { --m_value; return *this; }                                    \
                                                                        \
        name operator++(int)                                            \
        { name retval(m_value); ++m_value; return retval; }             \
        name operator--(int)                                            \
        { name retval(m_value); --m_value; return retval; }             \
                                                                        \
        name& operator+=(name rhs)                                      \
        { m_value += rhs.m_value; return *this; }                       \
        name& operator-=(name rhs)                                      \
        { m_value -= rhs.m_value; return *this; }                       \
        name& operator*=(name rhs)                                      \
        { m_value *= rhs.m_value; return *this; }                       \
        name& operator/=(name rhs)                                      \
        { m_value /= rhs.m_value; return *this; }                       \
        name& operator%=(name rhs)                                      \
        { m_value %= rhs.m_value; return *this; }                       \
                                                                        \
        name& operator+=(std::size_t rhs)                               \
        { m_value += rhs; return *this; }                               \
        name& operator-=(std::size_t rhs)                               \
        { m_value -= rhs; return *this; }                               \
        name& operator*=(std::size_t rhs)                               \
        { m_value *= rhs; return *this; }                               \
        name& operator/=(std::size_t rhs)                               \
        { m_value /= rhs; return *this; }                               \
        name& operator%=(std::size_t rhs)                               \
        { m_value %= rhs; return *this; }                               \
                                                                        \
    private:                                                            \
        std::size_t m_value;                                            \
                                                                        \
        friend class boost::serialization::access;                      \
        template <class Archive>                                        \
        void serialize(Archive& ar, const unsigned int version)         \
        { ar & BOOST_SERIALIZATION_NVP(m_value); }                      \
                                                                        \
        friend class name ## _d;                                        \
        friend std::size_t Value(name x);                               \
    };                                                                  \
                                                                        \
    inline bool operator>(name x, name y)                               \
    { return y < x; }                                                   \
    inline bool operator<=(name x, name y)                              \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, name y)                              \
    { return y < x || x == y; }                                         \
                                                                        \
    inline bool operator!=(name x, std::size_t y)                       \
    { return !(x == y); }                                               \
    inline bool operator>(name x, std::size_t y)                        \
    { return !(x < y || x == y); }                                      \
    inline bool operator<=(name x, std::size_t y)                       \
    { return x < y || x == y; }                                         \
    inline bool operator>=(name x, std::size_t y)                       \
    { return !(x < y); }                                                \
    inline bool operator==(std::size_t x, name y)                       \
    { return y == x; }                                                  \
    inline bool operator!=(std::size_t x, name y)                       \
    { return y != x; }                                                  \
    inline bool operator>(std::size_t x, name y)                        \
    { return y < x; }                                                   \
    inline bool operator<(std::size_t x, name y)                        \
    { return !(y < x || y == x); }                                      \
    inline bool operator<=(std::size_t x, name y)                       \
    { return !(y < x); }                                                \
    inline bool operator>=(std::size_t x, name y)                       \
    { return y < x || y == x; }                                         \
                                                                        \
    inline name operator+(name lhs, name rhs)                           \
    { return lhs += rhs; }                                              \
    inline name operator-(name lhs, name rhs)                           \
    { return lhs -= rhs; }                                              \
    inline name operator*(name lhs, name rhs)                           \
    { return lhs *= rhs; }                                              \
    inline name operator/(name lhs, name rhs)                           \
    { return lhs /= rhs; }                                              \
    inline name operator%(name lhs, name rhs)                           \
    { return lhs %= rhs; }                                              \
                                                                        \
    inline name operator+(name lhs, std::size_t rhs)                    \
    { return lhs += rhs; }                                              \
    inline name operator-(name lhs, std::size_t rhs)                    \
    { return lhs -= rhs; }                                              \
    inline name operator*(name lhs, std::size_t rhs)                    \
    { return lhs *= rhs; }                                              \
    inline name operator/(name lhs, std::size_t rhs)                    \
    { return lhs /= rhs; }                                              \
    inline name operator%(name lhs, std::size_t rhs)                    \
    { return lhs %= rhs; }                                              \
                                                                        \
    inline name operator+(std::size_t lhs, name rhs)                    \
    { return rhs += lhs; }                                              \
    inline name operator-(std::size_t lhs, name rhs)                    \
    { return -(rhs -= lhs); }                                           \
    inline name operator*(std::size_t lhs, name rhs)                    \
    { return rhs *= lhs; }                                              \
                                                                        \
    inline std::size_t Value(name x)                                    \
    { return x.m_value; }                                               \
                                                                        \
    inline std::ostream& operator<<(std::ostream& os, name x)           \
    { os << Value(x); return os; }                                      \
                                                                        \
    inline std::istream& operator>>(std::istream& os, name& x)          \
    {                                                                   \
        std::size_t t;                                                  \
        os >> t;                                                        \
        x = name(t);                                                    \
        return os;                                                      \
    }                                                                   \
                                                                        \
    void dummy_function_to_force_semicolon()

#endif // _GG_StrongTypedef_h_
