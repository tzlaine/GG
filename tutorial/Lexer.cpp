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

#include "Lexer.h"

#include <GG/adobe/name.hpp>

#include <boost/spirit/home/phoenix.hpp>


using namespace GG;

namespace {

    struct newline_count_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef std::size_t type; };

        std::size_t operator()(text_iterator first, text_iterator last) const
            { return std::count(first, last, '\n'); }
    };

    const boost::phoenix::function<newline_count_> newline_count;

}

namespace GG { namespace detail {
    const boost::phoenix::function<tok_val_> tok_val;
    const boost::phoenix::function<tok_pos_> tok_pos;
} }

lexer::lexer(const adobe::name_t* first_keyword,
             const adobe::name_t* last_keyword) :
    keyword_true_false("true|false"),
    keyword_empty("empty"),
    identifier("[a-zA-Z]\\w*"),
    lead_comment("\\/\\*[^*]*\\*+([^/*][^*]*\\*+)*\\/"),
    trail_comment("\\/\\/.*$"),
    quoted_string("\\\"[^\\\"]*\\\"|'[^']*'"),
    number("\\d+(\\.\\d*)?"),
    eq_op("==|!="),
    rel_op("<|>|<=|>="),
    mul_op("\\*|\\/|%"),
    define("<=="),
    or_("\"||\""),
    and_("&&"),
    not_('!'),
    plus('+'),
    minus('-'),
    at('@'),
    question('?'),
    colon(':'),
    dot('.'),
    comma(','),
    lparen('('),
    rparen(')'),
    lbracket('['),
    rbracket(']'),
    lbrace('{'),
    rbrace('}'),
    semicolon(';'),
    line_number(0),
    line_start()
{
    namespace lex = boost::spirit::lex;
    using boost::phoenix::ref;
    using lex::_start;
    using lex::_end;
    using lex::lit;

    typedef lex::token_def<text_iterator> empty_token;

    self =
        keyword_true_false
      | keyword_empty;

    while (first_keyword != last_keyword) {
        const lex::token_def<std::pair<adobe::name_t, text_iterator> >& keyword_token =
            keywords[*first_keyword] =
            lex::token_def<std::pair<adobe::name_t, text_iterator> >(first_keyword->c_str());
        self += keyword_token;
        ++first_keyword;
    }

    self +=
        identifier
      | lead_comment[ref(line_number) += newline_count(_start, _end)]
      | trail_comment
      | quoted_string
      | number
      | eq_op
      | rel_op
      | mul_op
      | define
      | or_
      | and_
      | plus
      | minus
      | not_
      | at
      | question
      | colon
      | dot
      | comma
      | lparen
      | rparen
      | lbracket
      | rbracket
      | lbrace
      | rbrace
      | semicolon
        ;

#undef SAVE_TOKEN_POS

    self("WS") =
        empty_token("\n")
        [
            ++ref(line_number),
            ref(line_start) = _end
        ]
      | empty_token("[\x20\t\f\r\v]+");
}
