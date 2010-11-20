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
   
/** \file Lexer.h TODO. */

#ifndef _GG_Lexer_h_
#define _GG_Lexer_h_

#ifndef GG_API
# ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef min
#  undef max
#  ifdef GiGi_EXPORTS
#   define GG_API __declspec(dllexport)
#  else
#   define GG_API __declspec(dllimport)
#  endif
# else
#  define GG_API
# endif
#endif

#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/implementation/token.hpp>

#define BOOST_SPIRIT_DEBUG // turn on tracking of match locations for tokens
#include <boost/spirit/include/lex_lexertl.hpp>
#undef BOOST_SPIRIT_DEBUG


namespace GG {

typedef std::string::const_iterator text_iterator;

namespace detail {
    struct named_eq_op : adobe::name_t {};
    struct named_rel_op : adobe::name_t {};
    struct named_mul_op : adobe::name_t {};
}

typedef boost::spirit::lex::lexertl::token<
    text_iterator,
    boost::mpl::vector<
        std::pair<adobe::name_t, text_iterator>,
        std::pair<detail::named_eq_op, text_iterator>,
        std::pair<detail::named_rel_op, text_iterator>,
        std::pair<detail::named_mul_op, text_iterator>,
        std::pair<std::string, text_iterator>,
        std::pair<double, text_iterator>,
        std::pair<bool, text_iterator>,
        text_iterator
    >
> token_type;

typedef boost::spirit::lex::lexertl::actor_lexer<token_type> spirit_lexer_base_type;

struct GG_API lexer :
    boost::spirit::lex::lexer<spirit_lexer_base_type>
{
    lexer(const adobe::name_t* first_keyword,
          const adobe::name_t* last_keyword);

    boost::spirit::lex::token_def<std::pair<bool, text_iterator> > keyword_true_false;
    boost::spirit::lex::token_def<text_iterator> keyword_empty;
    boost::spirit::lex::token_def<std::pair<adobe::name_t, text_iterator> > identifier;
    boost::spirit::lex::token_def<std::pair<std::string, text_iterator> > lead_comment;
    boost::spirit::lex::token_def<std::pair<std::string, text_iterator> > trail_comment;
    boost::spirit::lex::token_def<std::pair<std::string, text_iterator> > quoted_string;
    boost::spirit::lex::token_def<std::pair<double, text_iterator> > number;
    boost::spirit::lex::token_def<std::pair<detail::named_eq_op, text_iterator> > eq_op;
    boost::spirit::lex::token_def<std::pair<detail::named_rel_op, text_iterator> > rel_op;
    boost::spirit::lex::token_def<std::pair<detail::named_mul_op, text_iterator> > mul_op;
    boost::spirit::lex::token_def<text_iterator> define;
    boost::spirit::lex::token_def<text_iterator> or_;
    boost::spirit::lex::token_def<text_iterator> and_;
    boost::spirit::lex::token_def<text_iterator> not_;
    boost::spirit::lex::token_def<text_iterator> plus;
    boost::spirit::lex::token_def<text_iterator> minus;
    boost::spirit::lex::token_def<text_iterator> at;
    boost::spirit::lex::token_def<text_iterator> question;
    boost::spirit::lex::token_def<text_iterator> colon;
    boost::spirit::lex::token_def<text_iterator> dot;
    boost::spirit::lex::token_def<text_iterator> comma;
    boost::spirit::lex::token_def<text_iterator> lparen;
    boost::spirit::lex::token_def<text_iterator> rparen;
    boost::spirit::lex::token_def<text_iterator> lbracket;
    boost::spirit::lex::token_def<text_iterator> rbracket;
    boost::spirit::lex::token_def<text_iterator> lbrace;
    boost::spirit::lex::token_def<text_iterator> rbrace;
    boost::spirit::lex::token_def<text_iterator> semicolon;

    std::map<
        adobe::name_t,
        boost::spirit::lex::token_def<std::pair<adobe::name_t, text_iterator> >
    > keywords;

    std::size_t line_number; // 0-based line number of the current token
    text_iterator line_start; // the first character of the line currently being processed
};

typedef lexer::iterator_type token_iterator;

typedef lexer::lexer_def lexer_def;

typedef boost::spirit::qi::in_state_skipper<lexer_def> skipper_type;


namespace detail {

    struct tok_val_
    {
        template <typename Arg>
        struct result;

        template <typename T>
        struct result<std::pair<T, text_iterator> >
        { typedef T type; };

        template <typename T>
        typename result<std::pair<T, text_iterator> >::type
        operator()(const std::pair<T, text_iterator>& t) const
            { return t.first; }
    };

    extern GG_API const boost::phoenix::function<tok_val_> tok_val;

    struct tok_pos_
    {
        template <typename Arg>
        struct result
        { typedef text_iterator type; };

        template <typename T>
        text_iterator operator()(const std::pair<T, text_iterator>& t) const
            { return t.second; }
    };

    extern GG_API const boost::phoenix::function<tok_pos_> tok_pos;

}

}


// These template specializations are required by Spirit.Lex to automatically
// convert an iterator pair to an adobe::name_t in detail::lexer.

namespace boost { namespace spirit { namespace traits
{
    // These template specializations are required by Spirit.Lex to automatically
    // convert an iterator pair to an adobe::name_t in the lexer below.

    template <typename Iter>
    struct assign_to_attribute_from_iterators<adobe::name_t, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            { attr = adobe::name_t(std::string(first, last).c_str()); }
    };

    // HACK! This is only necessary because of a bug in Spirit in Boost
    // versions <= 1.45.
    template <>
    struct GG_API assign_to_attribute_from_iterators<bool, GG::text_iterator, void>
    {
        static void call(const GG::text_iterator& first, const GG::text_iterator& last, bool& attr)
            { attr = *first == 't' ? true : false; }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_eq_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            { attr = *first == '=' ? adobe::equal_k : adobe::not_equal_k; }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<Iter, Iter>
    {
        static void call(const Iter& first, const Iter& last, Iter& attr)
            { attr = first; }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_rel_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            {
                std::ptrdiff_t dist = std::distance(first, last);
                attr =
                    *first == '<' ?
                    (dist == 1 ? adobe::less_k : adobe::less_equal_k) :
                    (dist == 1 ? adobe::greater_k : adobe::greater_equal_k);
            }
    };

    template <typename Iter>
    struct assign_to_attribute_from_iterators<GG::detail::named_mul_op, Iter>
    {
        static void call(const Iter& first, const Iter& last, adobe::name_t& attr)
            {
                attr =
                    *first == '*' ?
                    adobe::multiply_k :
                    (*first == '/' ? adobe::divide_k : adobe::modulus_k);
            }
    };

    template <typename Iter, typename T>
    struct assign_to_attribute_from_iterators<std::pair<T, Iter>, Iter>
    {
        static void call(const Iter& first, const Iter& last, std::pair<T, Iter>& attr)
            {
                using boost::fusion::at_c;
                assign_to_attribute_from_iterators<T, Iter, void>::call(first, last, attr.first);
                attr.second = first;
            }
    };

} } }

#endif // _GG_Lexer_h_
