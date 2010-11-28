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
   
/** \file ExpressionParser.h TODO. */

#ifndef _GG_ExpressionParser_h_
#define _GG_ExpressionParser_h_

#include <GG/Lexer.h>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace GG {

namespace detail {

    struct report_error_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2 last, Arg3 it, Arg4 rule_name) const
            {
                std::string error_string;
                generate_error_string(*first, *it, rule_name, first == last, error_string);
                send_error_string(error_string);
            }

        static boost::function<void (const std::string&)> send_error_string;
        static void default_send_error_string(const std::string& str);

    private:
        void generate_error_string(const token_type& first,
                                   const token_type& it,
                                   const boost::spirit::info& rule_name,
                                   bool at_end,
                                   std::string& str) const;
    };

    extern GG_API const boost::phoenix::function<report_error_> report_error;

}

struct GG_API expression_parser_rules
{
    typedef boost::spirit::qi::rule<
        token_iterator,
        adobe::name_t(),
        skipper_type
    > keyword_rule;

    expression_parser_rules(const lexer& tok, const keyword_rule& keyword_);

    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        boost::spirit::qi::locals<adobe::array_t, adobe::array_t>,
        skipper_type
    > expression_rule;
    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        boost::spirit::qi::locals<adobe::name_t>,
        skipper_type
    > local_name_rule;
    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        boost::spirit::qi::locals<std::size_t>,
        skipper_type
    > local_size_rule;
    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        boost::spirit::qi::locals<adobe::array_t>,
        skipper_type
    > local_array_rule;
    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        skipper_type
    > no_locals_rule;

    // expression grammar
    expression_rule expression;

    local_array_rule or_expression;
    local_array_rule and_expression;
    local_name_rule equality_expression;
    local_name_rule relational_expression;
    local_name_rule additive_expression;
    local_name_rule multiplicative_expression;
    local_name_rule unary_expression;
    no_locals_rule postfix_expression;
    no_locals_rule primary_expression;
    local_name_rule variable_or_function;
    no_locals_rule array;
    no_locals_rule dictionary;
    no_locals_rule argument_expression_list;
    local_size_rule argument_list;
    local_size_rule named_argument_list;
    local_name_rule named_argument;
    no_locals_rule name;
    no_locals_rule boolean;

    // lexical grammar
    boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        boost::spirit::qi::locals<std::string>,
        skipper_type
    > string;
    keyword_rule keyword;
};

}

#endif // _GG_ExpressionParser_h_
