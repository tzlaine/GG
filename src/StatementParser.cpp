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
   
#include <GG/StatementParser.h>

#include <GG/adobe/implementation/token.hpp>


namespace {

    adobe::aggregate_name_t constant_k = { "constant" };
    adobe::aggregate_name_t return_k   = { "return" };

    struct array_t_push_back_
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void, typename Arg4 = void>
        struct result;

        template <typename Arg2, typename Arg3, typename Arg4>
        struct result<adobe::array_t, Arg2, Arg3, Arg4>
        { typedef void type; };

        template <typename Arg2>
        void operator()(adobe::array_t& array, Arg2 arg2) const
            { adobe::push_back(array, arg2); }

        template <typename Arg2, typename Arg3>
        void operator()(adobe::array_t& array, Arg2 arg2, Arg3 arg3) const
            {
                adobe::push_back(array, arg2);
                adobe::push_back(array, arg3);
            }

        template <typename Arg2, typename Arg3, typename Arg4>
        void operator()(adobe::array_t& array, Arg2 arg2, Arg3 arg3, Arg4 arg4) const
            {
                adobe::push_back(array, arg2);
                adobe::push_back(array, arg3);
                adobe::push_back(array, arg4);
            }
    };

    const boost::phoenix::function<array_t_push_back_> push;

}

using namespace GG;

statement_parser_rules::statement_parser_rules(
    const lexer& tok,
    const expression_parser_rules::expression_rule& expression
) {
    namespace qi = boost::spirit::qi;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
    using qi::_val;
    using qi::lit;

    const boost::spirit::lex::token_def<adobe::name_t>& constant =
        tok.keywords.find(constant_k)->second;
    const boost::spirit::lex::token_def<adobe::name_t>& return__ =
        tok.keywords.find(return_k)->second;

    const_declaration
        =     constant
        >     tok.identifier [_a = _1]
        >   - (
                   lit(':')
                >  expression(_b)
              )
        >     lit(';')
              [
                  push(_r1, _a, _b, adobe::const_decl_k)
              ]
        ;

    declaration
        =     tok.identifier [_a = _1]
        >>  - (
                   lit(':')
                >  expression(_b)
              )
        >>    lit(';')
              [
                  push(_r1, _a, _b, adobe::decl_k)
              ]
        ;

    assignment
        =     tok.identifier [_a = _1]
        >>    lit('=') [push(_r1, _a, adobe::lvalue_k)]
        >     expression(_r1)
        >     lit(';')
              [
                  push(_r1, adobe::assign_k)
              ]
        ;

    return_
        =     return__
        >     expression(_r1)
        >     lit(';')
              [
                  push(_r1, adobe::return_k)
              ]
        ;

    statement
        =     const_declaration(_val)
        |     declaration(_val)
        |     assignment(_val)
        |     return_(_val)
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(statement);
    NAME(const_declaration);
    NAME(declaration);
    NAME(assignment);
    NAME(return_);
#undef NAME

    qi::on_error<qi::fail>(statement, report_error(_1, _2, _3, _4));
}
