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
   
/** \file StatementParser.h TODO. */

#ifndef _GG_StatementParser_h_
#define _GG_StatementParser_h_

#include <GG/ExpressionParser.h>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace GG {

struct GG_API statement_parser_rules
{
    statement_parser_rules(
        const lexer& tok,
        const expression_parser_rules::expression_rule& expression
    );

    typedef boost::spirit::qi::rule<
        token_iterator,
        adobe::array_t(),
        boost::spirit::qi::locals<
            adobe::name_t,
            adobe::array_t,
            adobe::array_t,
            adobe::array_t
        >,
        skipper_type
    > statement_rule;
    typedef boost::spirit::qi::rule<
        token_iterator,
        void(adobe::array_t&),
        skipper_type
    > statements_rule;

    // statement grammar
    statement_rule statement;

    statement_rule const_declaration;
    statement_rule declaration;
    statement_rule assignment;
    statements_rule block;
    statements_rule if_statements;
    statement_rule if_;
    statement_rule return_;
};

}

#endif
