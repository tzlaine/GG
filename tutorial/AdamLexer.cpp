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

#include "AdamLexer.h"

#include <GG/adobe/name.hpp>


using namespace GG;

namespace {

    adobe::aggregate_name_t input_k      = { "input" };
    adobe::aggregate_name_t output_k     = { "output" };
    adobe::aggregate_name_t interface_k  = { "interface" };
    adobe::aggregate_name_t logic_k      = { "logic" };
    adobe::aggregate_name_t constant_k   = { "constant" };
    adobe::aggregate_name_t invariant_k  = { "invariant" };
    adobe::aggregate_name_t sheet_k      = { "sheet" };
    adobe::aggregate_name_t unlink_k     = { "unlink" };
    adobe::aggregate_name_t when_k       = { "when" };
    adobe::aggregate_name_t relate_k     = { "relate" };

}

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
    and_("&&")
{
    namespace lex = boost::spirit::lex;

    std::string keywords_regex;
    if (first_keyword != last_keyword) {
        keywords_regex = (first_keyword++)->c_str();
        while (first_keyword != last_keyword) {
            keywords_regex += '|';
            keywords_regex += first_keyword->c_str();
            ++first_keyword;
        }
        keyword = keywords_regex;
    }

    self =
        keyword
      | keyword_true_false
      | keyword_empty
      | identifier
      | lead_comment
      | trail_comment
      | quoted_string
      | number
      | eq_op
      | rel_op
      | mul_op
      | define
      | or_
      | and_
      | '+'
      | '-'
      | '!'
      | '?'
      | ':'
      | '.'
      | ','
      | '('
      | ')'
      | '['
      | ']'
      | '{'
      | '}'
      | '@'
      | ';'
        ;

    self("WS") = lex::token_def<>("\\s+");
}

const lexer& GG::AdamLexer()
{
    static const adobe::name_t s_keywords[] = {
        input_k,
        output_k,
        interface_k,
        logic_k,
        constant_k,
        invariant_k,
        sheet_k,
        unlink_k,
        when_k,
        relate_k
    };
    static const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

    static const GG::lexer s_lexer(s_keywords, s_keywords + s_num_keywords);

    return s_lexer;    
}
