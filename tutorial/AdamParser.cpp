#include "AdamParser.h"

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>


using namespace GG;

namespace {

    struct make_name_t_
    {
        template <typename Arg>
        struct result
        { typedef adobe::name_t type; };

        adobe::name_t operator()(const std::string& arg1) const
            { return adobe::name_t(arg1.c_str()); }
    };

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

    struct strip_quotes_
    {
        template <typename Arg>
        struct result
        { typedef std::string type; };

        std::string operator()(const std::string& arg1) const
            { return arg1.substr(1, arg1.size() - 2); }
    };

    struct report_error_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2 last, Arg3 it, Arg4 name) const
            {
                if (it == last) {
                    std::cout
                        << "Parse error: expected "
                        << name
                        << " before end of expression inupt."
                        << std::endl;
                } else {
                    text_iterator text_end =
                        boost::next(first, std::distance(first, last) - 1)->matched_.second;
                    std::cout
                        << "Parse error: expected "
                        << name
                        << " here:\n"
                        << "  "
                        << std::string(first->matched_.first, text_end)
                        << "\n"
                        << "  "
                        << std::string(std::distance(first->matched_.first, it->matched_.first), '~')
                        << '^'
                        << std::endl;
                }
            }
    };

    struct expression_parser_rules
    {
        typedef boost::spirit::qi::rule<
            GG::token_iterator,
            adobe::name_t(),
            GG::skipper_type
        > name_rule;

        expression_parser_rules(const GG::lexer& tok, const name_rule& keyword_) :
            keyword(keyword_)
        {
            namespace ascii = boost::spirit::ascii;
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;
            using ascii::char_;
            using phoenix::construct;
            using phoenix::if_;
            using phoenix::static_cast_;
            using phoenix::val;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_a;
            using qi::_b;
            using qi::_r1;
            using qi::_val;
            using qi::alpha;
            using qi::bool_;
            using qi::digit;
            using qi::double_;
            using qi::eol;
            using qi::eps;
            using qi::lexeme;
            using qi::lit;

            expression =
                or_expression(_r1)
             >> -(
                    "?"
                  > expression(&_a)
                  > ":"
                  > expression(&_b)
                 )[push(*_r1, _a, _b, adobe::ifelse_k)];

            or_expression =
                and_expression(_r1)
             >> *( tok.or_ > and_expression(&_a) )[push(*_r1, _a, adobe::or_k)];

            and_expression =
                equality_expression(_r1)
             >> *( tok.and_ > equality_expression(&_a) )[push(*_r1, _a, adobe::and_k)];

            equality_expression =
                relational_expression(_r1)
             >> *( tok.eq_op[_a = _1] > relational_expression(_r1) )[push(*_r1, _a)];

            relational_expression =
                additive_expression(_r1)
             >> *( tok.rel_op[_a = _1] > additive_expression(_r1) )[push(*_r1, _a)];

            additive_expression =
                multiplicative_expression(_r1)
                >> *(
                        (
                            lit('+')[_a = adobe::add_k]
                          | lit('-')[_a = adobe::subtract_k]
                        )
                      > multiplicative_expression(_r1)
                    )[push(*_r1, _a)];

            multiplicative_expression =
                unary_expression(_r1)
             >> *( tok.mul_op[_a = _1] > unary_expression(_r1) )[push(*_r1, _a)];

            unary_expression =
                postfix_expression(_r1)
              | (
                  (
                      lit('+')
                    | lit('-')[_a = adobe::unary_negate_k]
                    | lit('!')[_a = adobe::not_k]
                  )
                > unary_expression(_r1)
                )[if_(_a)[push(*_r1, _a)]];

            // omitting unary_operator

            postfix_expression =
                primary_expression(_r1)
             >> *( ('[' > expression(_r1) > ']') | ('.' > tok.identifier[push(*_r1, _1)]) )[
                 push(*_r1, adobe::index_k)
             ];

            primary_expression =
                ( '(' >> expression(_r1) > ')' )
              | name(_r1)
              | tok.number[push(*_r1, _1)]
              | boolean(_r1)
              | string(_r1)
              | tok.keyword_empty[push(*_r1, adobe::any_regular_t())]
              | array(_r1)
              | dictionary(_r1)
              | variable_or_function(_r1);

            variable_or_function =
                (tok.identifier[_a = _1] >>
                 (
                     "(" >> (argument_expression_list(_r1) | eps[push(*_r1, adobe::array_t())]) > ")"
                 )
                )[push(*_r1, _a, adobe::function_k)]
              | tok.identifier[push(*_r1, _1, adobe::variable_k)];

            array = '[' >> (argument_list(_r1) | eps[push(*_r1, adobe::array_t())]) > ']';

            dictionary = '{' >> (named_argument_list(_r1) | eps[push(*_r1, adobe::dictionary_t())]) > '}';

            argument_expression_list = named_argument_list(_r1) | argument_list(_r1);

            argument_list =
                (expression(_r1)[_a = 1] >> *( ',' > expression(_r1)[++_a] ))[
                    push(*_r1, static_cast_<double>(_a), adobe::array_k)
                ];

            named_argument_list =
                (named_argument(_r1)[_a = 1] >> *( ',' > named_argument(_r1)[++_a] ))[
                    push(*_r1, static_cast_<double>(_a), adobe::dictionary_k)
                ];

            named_argument =
                tok.identifier[_a = _1] >> lit(':')[push(*_r1, _a)] > expression(_r1);

            name = '@' > ( tok.identifier[push(*_r1, _1)] | keyword[push(*_r1, _1)] );

            boolean = tok.keyword_true_false[push(*_r1, _1)];


            // lexical grammar not covered by lexer

            string =
                (tok.quoted_string[_a = strip_quotes(_1)]
              >> *(tok.quoted_string[_a += strip_quotes(_1)]))[push(*_r1, _a)];

            keyword =
                tok.keyword_true_false[_val = if_else(_1, adobe::true_k, adobe::false_k)]
              | tok.keyword_empty[_val = adobe::empty_k]
              | keyword_[_val = _1];


            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(expression);
            NAME(or_expression);
            NAME(and_expression);
            NAME(equality_expression);
            NAME(relational_expression);
            NAME(additive_expression);
            NAME(multiplicative_expression);
            NAME(unary_expression);
            NAME(postfix_expression);
            NAME(primary_expression);
            NAME(variable_or_function);
            NAME(array);
            NAME(dictionary);
            NAME(argument_expression_list);
            NAME(argument_list);
            NAME(named_argument_list);
            NAME(named_argument);
            NAME(name);
            NAME(boolean);
            NAME(string);
#undef NAME

            qi::on_error<qi::fail>(expression, report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::name_t>,
            GG::skipper_type
        > local_name_rule;
        typedef boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            boost::spirit::qi::locals<std::size_t>,
            GG::skipper_type
        > local_size_rule;
        typedef boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::array_t>,
            GG::skipper_type
        > local_array_rule;
        typedef boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            GG::skipper_type
        > no_locals_rule;

        // expression grammar
        boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::array_t, adobe::array_t>,
            GG::skipper_type
        > expression;

        local_array_rule or_expression;
        local_array_rule and_expression;
        local_name_rule equality_expression;
        local_name_rule relational_expression;
        local_name_rule additive_expression;
        local_name_rule multiplicative_expression;
        local_name_rule unary_expression;
        no_locals_rule postfix_expression;
        no_locals_rule primary_expression;
        local_name_rule variable_or_function;
        no_locals_rule array;
        no_locals_rule dictionary;
        no_locals_rule argument_expression_list;
        local_size_rule argument_list;
        local_size_rule named_argument_list;
        local_name_rule named_argument;
        no_locals_rule name;
        no_locals_rule boolean;

        // lexical grammar
        boost::spirit::qi::rule<
            GG::token_iterator,
            void(adobe::array_t*),
            boost::spirit::qi::locals<std::string>,
            GG::skipper_type
        > string;
        name_rule keyword;

        boost::phoenix::function<make_name_t_> make_name_t;
        boost::phoenix::function<array_t_push_back_> push;
        boost::phoenix::function<strip_quotes_> strip_quotes;
        boost::phoenix::function<report_error_> report_error;
    };

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
