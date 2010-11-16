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
   
/** \file AdamLexer.h TODO. */

#ifndef _GG_AdamLexer_h_
#define _GG_AdamLexer_h_

#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/lex_lexertl.hpp>


namespace GG {

typedef std::string::const_iterator text_iterator;

typedef boost::spirit::lex::lexertl::token<
    text_iterator,
    boost::mpl::vector<
        adobe::name_t,
        std::string,
        double,
        bool
    >
> token_type;

typedef boost::spirit::lex::lexertl::actor_lexer<token_type> spirit_lexer_base_type;

namespace detail {
    struct named_eq_op : adobe::name_t {};
    struct named_rel_op : adobe::name_t {};
    struct named_mul_op : adobe::name_t {};
}

struct lexer :
    boost::spirit::lex::lexer<spirit_lexer_base_type>
{
    lexer(const adobe::name_t* first_keyword,
          const adobe::name_t* last_keyword);

    boost::spirit::lex::token_def<adobe::name_t> keyword;
    boost::spirit::lex::token_def<bool> keyword_true_false;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> keyword_empty;
    boost::spirit::lex::token_def<adobe::name_t> identifier;
    boost::spirit::lex::token_def<std::string> lead_comment;
    boost::spirit::lex::token_def<std::string> trail_comment;
    boost::spirit::lex::token_def<std::string> quoted_string;
    boost::spirit::lex::token_def<double> number;
    boost::spirit::lex::token_def<detail::named_eq_op> eq_op;
    boost::spirit::lex::token_def<detail::named_rel_op> rel_op;
    boost::spirit::lex::token_def<detail::named_mul_op> mul_op;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> define;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> or_;
    boost::spirit::lex::token_def<boost::spirit::lex::omit> and_;
};

typedef lexer::iterator_type token_iterator;

typedef lexer::lexer_def lexer_def;

typedef boost::spirit::qi::in_state_skipper<lexer_def> skipper_type;

const lexer& AdamLexer();

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

    template <>
    struct assign_to_attribute_from_iterators<bool, GG::text_iterator, void>
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

} } }

#endif // _GG_AdamLexer_h_
