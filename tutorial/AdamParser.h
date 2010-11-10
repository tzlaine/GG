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
   
/** \file AdamExpressionParser.h TODO. */

#ifndef _GG_AdamParser_h_
#define _GG_AdamParser_h_

#include <GG/Base.h>
#include <GG/adobe/array_fwd.hpp>

#include <boost/spirit/home/qi/char/char_class.hpp>
#include <boost/spirit/home/qi/nonterminal/grammar.hpp>

#include <string>


namespace GG {

/** The type of Spirit 2 parser returned by GetAdamExpressionParser(). */
typedef boost::spirit::qi::grammar<
    std::string::const_iterator,
    void(),
    boost::spirit::ascii::space_type
> AdamExpressionParser;

/** Returns a Spirit 2 parser that can be used to parse Adam expressions.  The
    returned parser will fill in \a expression with the results of the
    parse. */
GG_API const AdamExpressionParser& GetAdamExpressionParser(adobe::array_t& expression);

}

#endif // _GG_AdamParser_h_
