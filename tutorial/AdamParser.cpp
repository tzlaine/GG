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

#include "AdamParser.h"

#include "ExpressionParser.h"
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


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

    static lexer s_lexer(s_keywords, s_keywords + s_num_keywords);

    return s_lexer;    
}

const AdamExpressionParserRule& GG::AdamExpressionParser()
{
    using boost::spirit::qi::token;
    using boost::spirit::qi::_1;
    using boost::spirit::qi::_val;

    lexer& tok = const_cast<lexer&>(AdamLexer());
    assert(tok.keywords.size() == 10u);
    const boost::spirit::lex::token_def<adobe::name_t>& input = tok.keywords[input_k];
    const boost::spirit::lex::token_def<adobe::name_t>& output = tok.keywords[output_k];
    const boost::spirit::lex::token_def<adobe::name_t>& interface = tok.keywords[interface_k];
    const boost::spirit::lex::token_def<adobe::name_t>& logic = tok.keywords[logic_k];
    const boost::spirit::lex::token_def<adobe::name_t>& constant = tok.keywords[constant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& invariant = tok.keywords[invariant_k];
    const boost::spirit::lex::token_def<adobe::name_t>& sheet = tok.keywords[sheet_k];
    const boost::spirit::lex::token_def<adobe::name_t>& unlink = tok.keywords[unlink_k];
    const boost::spirit::lex::token_def<adobe::name_t>& when = tok.keywords[when_k];
    const boost::spirit::lex::token_def<adobe::name_t>& relate = tok.keywords[relate_k];
    assert(tok.keywords.size() == 10u);

    static expression_parser_rules::name_rule adam_keywords =
        input[_val = _1]
      | output[_val = _1]
      | interface[_val = _1]
      | logic[_val = _1]
      | constant[_val = _1]
      | invariant[_val = _1]
      | sheet[_val = _1]
      | unlink[_val = _1]
      | when[_val = _1]
      | relate[_val = _1]
        ;
    adam_keywords.name("keyword");

    static const expression_parser_rules s_parser(GG::AdamLexer(), adam_keywords);

    return s_parser.expression;
}
