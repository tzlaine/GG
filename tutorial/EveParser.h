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
   
/** \file EveParser.h TODO. */

#ifndef _GG_EveParser_h_
#define _GG_EveParser_h_

#include <GG/Lexer.h>
#include <GG/adobe/array_fwd.hpp>
#include <GG/adobe/name_fwd.hpp>

#include <boost/spirit/home/qi/char/char_class.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>

#include <string>


namespace adobe {
    struct eve_callback_suite_t;
}

namespace GG {

GG_API const lexer& EveLexer();

/** The type of Spirit 2 parser returned by EveExpressionParser(). */
typedef boost::spirit::qi::rule<
    token_iterator,
    void(adobe::array_t&),
    boost::spirit::qi::locals<adobe::array_t, adobe::array_t>,
    skipper_type
> EveExpressionParserRule;

/** Returns a Spirit 2 parser that can be used to parse Eve expressions. */
GG_API const EveExpressionParserRule& EveExpressionParser();

GG_API bool Parse(const std::string& sheet,
                  const std::string& filename,
                  const boost::any& position,
                  const adobe::eve_callback_suite_t& callbacks);

}

#endif // _GG_EveParser_h_