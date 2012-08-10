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

#include <GG/AdamParser.h>

#include <GG/adobe/implementation/token.hpp>


using namespace GG;

namespace {

    adobe::aggregate_name_t function_k = { "function" };

    struct add_function_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        void operator()(
            AdamFunctions& arg1,
            adobe::name_t arg2,
            const std::vector<adobe::name_t>& arg3,
            const std::vector<adobe::array_t>& arg4
        ) const
            { arg1[arg2] = adobe::adam_function_t(arg2, arg3, arg4); }
    };

    const boost::phoenix::function<add_function_> add_function;

}

function_parser_rules::function_parser_rules(
    const lexer& tok,
    const expression_parser_rules::expression_rule& expression
) :
    statement_parser(tok, expression)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using phoenix::construct;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_r1;
    using qi::_val;
    using qi::lit;

    function
        =     tok.identifier [_a = _1]
        >     '('
        >   - ((tok.identifier [push_back(_b, _1)]) % ',')
        >     ')'
        >     '{'
        >   * statement_parser.statement [push_back(_c, _1)]
        >     lit('}')
              [
                  add_function(_r1, _a, _b, _c)
              ]
        ;


    // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
    NAME(function);
#undef NAME

    qi::on_error<qi::fail>(function, report_error(_1, _2, _3, _4));
}

bool GG::ParseFunctions(const std::string& functions,
                        const std::string& filename,
                        AdamFunctions& retval)
{
    using boost::spirit::qi::phrase_parse;
    text_iterator it(functions.begin());
    detail::s_text_it = &it;
    detail::s_begin = it;
    detail::s_end = text_iterator(functions.end());
    detail::s_filename = filename.c_str();
    token_iterator iter = AdamLexer().begin(it, detail::s_end);
    token_iterator end = AdamLexer().end();
    function_parser_rules rules(AdamLexer(), AdamExpressionParser());
    bool result =
        phrase_parse(iter,
                     end,
                     *rules.function(boost::phoenix::ref(retval)),
                     boost::spirit::qi::in_state("WS")[AdamLexer().self]);
    return result && iter == end;
}
