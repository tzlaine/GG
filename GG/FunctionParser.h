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
   
/** \file FunctionParser.h TODO. */

#ifndef _GG_FunctionParser_h_
#define _GG_FunctionParser_h_

#include <GG/StatementParser.h>
#include <GG/adobe/adam_function.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace GG {

/** The map of functions used to evaluate user-defined Adam functions in Adam
    and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef adobe::closed_hash_map<adobe::name_t, adobe::adam_function> AdamFunctions;

struct GG_API function_parser_rules
{
    function_parser_rules(
        const lexer& tok,
        const expression_parser_rules::expression_rule& expression
    );

    typedef boost::spirit::qi::rule<
        token_iterator,
        void(AdamFunctions&),
        boost::spirit::qi::locals<
            adobe::name_t,
            std::vector<adobe::name_t>,
            std::vector<adobe::array_t>
        >,
        skipper_type
    > function_rule;

    // function grammar
    function_rule function;

    statement_parser_rules statement_parser;
};

GG_API bool ParseFunctions(const std::string& functions,
                           const std::string& filename,
                           AdamFunctions&);

}

#endif
