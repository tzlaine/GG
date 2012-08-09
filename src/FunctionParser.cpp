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
   
#include <GG/FunctionParser.h>

#include <GG/adobe/implementation/token.hpp>


namespace {

    adobe::aggregate_name_t function_k = { "function" };

}

using namespace GG;

function_parser_rules::function_parser_rules(
    const lexer& tok,
    const expression_parser_rules::expression_rule& expression
) :
    statement_parser(tok, expression)
{
    namespace qi = boost::spirit::qi;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_r1;
    using qi::_r2;
    using qi::_r3;

    const boost::spirit::lex::token_def<adobe::name_t>& function_ =
        tok.keywords.find(function_k)->second;

    function
        =     function_
        >     tok.identifier [_r1 = _1]
        >     '('
        >     ((tok.identifier [push_back(_r2, _1)]) % ',')
        >     ')'
        >     '{'
        >   * statement_parser.statement [push_back(_r3, _1)]
        >     '}'
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(function);
#undef NAME

    qi::on_error<qi::fail>(function, report_error(_1, _2, _3, _4));
}
