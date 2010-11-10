#include <GG/AdamDlg.h>
#include <GG/Layout.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/SDL/SDLGUI.h>

#include <GG/adobe/dictionary.hpp>

#include <iostream>


class AdamGGApp :
    public GG::SDLGUI
{
public:
    AdamGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

enum PathTypes {
    NONE,
    PATH_1
};

std::ostream& operator<<(std::ostream& os, PathTypes p);
std::istream& operator>>(std::istream& os, PathTypes& p);

#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/implementation/token.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/phoenix/object.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>

#include <boost/algorithm/string/split.hpp> // for testing only
#include <boost/algorithm/string/classification.hpp> // for testing only
#include <fstream> // for testing only

// for testing only
std::string read_file (const std::string& filename)
{
    std::string retval;
    std::ifstream ifs(filename.c_str());
    int c;
    while ((c = ifs.get()) != std::ifstream::traits_type::eof()) {
        retval += c;
    }
    return retval;
}

// for testing only
namespace adobe { namespace version_1 {

std::ostream& operator<<(std::ostream& stream, const type_info_t& x)
{
    std::ostream_iterator<char> out(stream);
    serialize(x, out);
    return stream;
}

} }

namespace GG {

    struct AnySlotImplBase
    {
        virtual AnySlotImplBase* Clone() const = 0;
    };

    template <class Signature>
    struct AnySlotImpl :
        AnySlotImplBase
    {
        AnySlotImpl(const boost::function<Signature>& slot) :
            m_slot(slot)
            {}

        virtual AnySlotImplBase* Clone() const
            { return new AnySlotImpl(*this); }

        boost::function<Signature> m_slot;
    };

    GG_EXCEPTION(BadAnySlotCast);

    class AnySlot
    {
    public:
        AnySlot() :
            m_impl(0)
            {}

        template <class Signature>
        AnySlot(const boost::function<Signature>& slot) :
            m_impl(new AnySlotImpl<Signature>(slot))
            {}

        AnySlot(const AnySlot& rhs) :
            m_impl(rhs.m_impl ? rhs.m_impl->Clone() : 0)
            {}

        AnySlot& operator=(const AnySlot& rhs)
            {
                delete m_impl;
                m_impl = rhs.m_impl ? rhs.m_impl->Clone() : 0;
                return *this;
            }

        ~AnySlot()
            { delete m_impl; }

        template <class Signature>
        boost::function<Signature>& Cast()
            {
                assert(m_impl);
                AnySlotImpl<Signature>* derived_impl =
                    dynamic_cast<AnySlotImpl<Signature>*>(m_impl);
                if (!derived_impl)
                    throw BadAnySlotCast();
                return derived_impl->m_slot;
            }

    private:
        AnySlotImplBase* m_impl;
    };

    struct AnySignalImplBase
    {
        virtual AnySignalImplBase* Clone() const = 0;
        virtual boost::signals::connection Connect(AnySlot& slot) = 0;
    };

    template <class Signature>
    struct AnySignalImpl :
        AnySignalImplBase
    {
        AnySignalImpl(boost::signal<Signature>& signal) :
            m_signal(&signal)
            {}

        virtual AnySignalImplBase* Clone() const
            { return new AnySignalImpl(*this); }

        virtual boost::signals::connection Connect(AnySlot& slot)
            {
                boost::function<Signature>& fn = slot.Cast<Signature>();
                return m_signal->connect(fn);
            }

        boost::signal<Signature>* m_signal;
    };

    class AnySignal
    {
    public:
        AnySignal() :
            m_impl(0)
            {}

        template <class Signature>
        AnySignal(boost::signal<Signature>& signal) :
            m_impl(new AnySignalImpl<Signature>(signal))
            {}

        AnySignal(const AnySignal& rhs) :
            m_impl(rhs.m_impl ? rhs.m_impl->Clone() : 0)
            {}

        AnySignal& operator=(const AnySignal& rhs)
            {
                delete m_impl;
                m_impl = rhs.m_impl ? rhs.m_impl->Clone() : 0;
                return *this;
            }

        ~AnySignal()
            { delete m_impl; }

        boost::signals::connection Connect(AnySlot& slot)
            {
                assert(m_impl);
                return m_impl->Connect(slot);
            }

    private:
        AnySignalImplBase* m_impl;
    };

    struct Wnd_
    {
        typedef adobe::closed_hash_map<adobe::name_t, AnySignal> SignalsMap;
        typedef adobe::closed_hash_map<adobe::name_t, AnySlot> SlotsMap;

        SignalsMap m_signals_map;
        SlotsMap m_slots_map;
    };

    struct ControlA :
        public Wnd_
    {
        ControlA()
            {
                m_signals_map[adobe::name_t("foo_signal")] = AnySignal(foo_signal);
                m_signals_map[adobe::name_t("bar_signal")] = AnySignal(bar_signal);
                m_signals_map[adobe::name_t("baz_signal")] = AnySignal(baz_signal);
            }

        boost::signal<void (const int, double)> foo_signal;
        boost::signal<void (const int&, double&)> bar_signal;
        boost::signal<void (const int*, double*)> baz_signal;
    };

    struct ControlB :
        public Wnd_
    {
        ControlB()
            {
                m_slots_map[adobe::name_t("foo")] =
                    AnySlot(
                        boost::function<void (const int, double)>(
                            boost::bind(&ControlB::foo, this, _1, _2)
                        )
                    );
                m_slots_map[adobe::name_t("bar")] =
                    AnySlot(
                        boost::function<void (const int&, double&)>(
                            boost::bind(&ControlB::bar, this, _1, _2)
                        )
                    );
                m_slots_map[adobe::name_t("baz")] =
                    AnySlot(
                        boost::function<void (const int*, double*)>(
                            boost::bind(&ControlB::baz, this, _1, _2)
                        )
                    );
            }

        void foo(const int i, double d)
            { std::cerr << "foo(" << i << ", " << d << ")\n"; }
        void bar(const int& i, double& d)
            { std::cerr << "bar(" << i << ", " << d << ")\n"; }
        void baz(const int* i, double* d)
            { std::cerr << "baz(" << *i << ", " << *d << ")\n"; }
    };

    boost::signals::connection
    Connect(Wnd_& signal_wnd, adobe::name_t signal_name, Wnd_& slot_wnd, adobe::name_t slot_name)
    {
        AnySignal signal = signal_wnd.m_signals_map[signal_name];
        AnySlot slot = slot_wnd.m_slots_map[slot_name];
        return signal.Connect(slot);
    }

    bool TestNewConnections()
    {
        ControlA* control_a = new ControlA;
        ControlB* control_b = new ControlB;

#if 0
        boost::signals::connection old_style_foo_connection =
            Connect(control_a->foo_signal, &ControlB::foo, control_b);
#endif

        boost::signals::connection foo_connection =
            Connect(*control_a, adobe::name_t("foo_signal"), *control_b, adobe::name_t("foo"));
        boost::signals::connection bar_connection =
            Connect(*control_a, adobe::name_t("bar_signal"), *control_b, adobe::name_t("bar"));
        boost::signals::connection baz_connection =
            Connect(*control_a, adobe::name_t("baz_signal"), *control_b, adobe::name_t("baz"));

        int i = 180;
        double d = 3.14159;

        control_a->foo_signal(i, d);
        control_a->bar_signal(i, d);
        control_a->baz_signal(&i, &d);

        return false;
    }

    //bool dummy = TestNewConnections();

    struct make_name_t_
    {
        template <typename Arg>
        struct result
        { typedef adobe::name_t type; };

        adobe::name_t operator()(const std::string& arg1) const
            { return adobe::name_t(arg1.c_str()); }
    };

    boost::phoenix::function<make_name_t_> make_name_t;

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

    struct report_error_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 _1, Arg2 _2, Arg3 _3, Arg4 _4) const
            {
                if (_3 == _2) {
                    std::cout
                        << "Parse error: expected "
                        << _4
                        << " before end of expression inupt."
                        << std::endl;
                } else {
                    std::cout
                        << "Parse error: expected "
                        << _4
                        << " here:"
                        << "\n  "
                        << std::string(_1, _2)
                        << "\n  "
                        << std::string(std::distance(_1, _3), '~')
                        << '^'
                        << std::endl;
                }
            }
    };

    template <typename Iter>
    struct expression_parser :
        boost::spirit::qi::grammar<Iter, void(), boost::spirit::ascii::space_type>
    {
        typedef boost::spirit::ascii::space_type space_type;

        expression_parser(const std::vector<adobe::name_t>& keywords, adobe::array_t& stack) :
            expression_parser::base_type(start, "expression")
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

            start = expression(&stack);

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
             >> *( "||" > and_expression(&_a) )[push(*_r1, _a, adobe::or_k)];

            and_expression =
                equality_expression(_r1)
             >> *( "&&" > equality_expression(&_a) )[push(*_r1, _a, adobe::and_k)];

            equality_expression =
                relational_expression(_r1)
             >> *( eq_op[_a = _1] > relational_expression(_r1) )[push(*_r1, _a)];

            relational_expression =
                additive_expression(_r1)
             >> *( rel_op[_a = _1] > additive_expression(_r1) )[push(*_r1, _a)];

            additive_expression =
                multiplicative_expression(_r1)
             >> *( add_op[_a = _1] > multiplicative_expression(_r1) )[push(*_r1, _a)];

            multiplicative_expression =
                unary_expression(_r1)
             >> *( mul_op[_a = _1] > unary_expression(_r1) )[push(*_r1, _a)];

            unary_expression =
                postfix_expression(_r1)
              | ( unary_op[_a = _1] > unary_expression(_r1) )[if_(_a)[push(*_r1, _a)]];

            // omitting unary_operator

            postfix_expression =
                primary_expression(_r1)
             >> *( ("[" > expression(_r1) > "]") | ("." > identifier(_r1)) )[
                 push(*_r1, adobe::index_k)
             ];

            primary_expression =
                ( "(" >> expression(_r1) > ")" )
              | name(_r1)
              | number(_r1)
              | boolean(_r1)
              | string(_r1)
              | empty(_r1)
              | array(_r1)
              | dictionary(_r1)
              | variable_or_function(_r1);

            variable_or_function = function(_r1) | variable(_r1);

            array = "[" >> (argument_list(_r1) | eps[push(*_r1, adobe::array_t())]) > "]";

            dictionary = "{" >> (named_argument_list(_r1) | eps[push(*_r1, adobe::dictionary_t())]) > "}";

            argument_expression_list = named_argument_list(_r1) | argument_list(_r1);

            argument_list =
                (expression(_r1)[_a = 1] >> *( "," > expression(_r1)[++_a] ))[
                    push(*_r1, static_cast_<double>(_a), adobe::array_k)
                ];

            named_argument_list =
                (named_argument(_r1)[_a = 1] >> *( "," > named_argument(_r1)[++_a] ))[
                    push(*_r1, static_cast_<double>(_a), adobe::dictionary_k)
                ];

            named_argument =
                identifier_string[_a = make_name_t(_1)] >> lit(":")[push(*_r1, _a)] > expression(_r1);

            name = "@" > ( identifier(_r1) | keyword(_r1) );

            boolean = bool_[push(*_r1, _1)];


            // lexical grammar

            // omitting simple_token

            // omitting compound_token

            string = (quoted_string[_a = _1] >> *(quoted_string[_a += _1]))[push(*_r1, _a)];

            lead_comment %= ("/*" >> lexeme[*char_ - "*/"] >> "*/");

            trail_comment %= ("//" >> lexeme[*char_ >> eol]);

            identifier = (!keyword_string >> identifier_string)[push(*_r1, make_name_t(_1))];

            keyword = keyword_string[push(*_r1, _1)];

            number = !(lit("-") | "+") >> double_[push(*_r1, _1)];

            quoted_string %=
                lexeme['"' >> *(char_ - '"') >> '"']
              | lexeme['\'' >> *(char_ - '\'') >> '\''];

            // omitting digits


            // convenience rules
            identifier_string %= (alpha | "_") >> *(alpha | "_" | digit);

            empty = lit("empty")[push(*_r1, adobe::any_regular_t())];

            function =
                (identifier_string[_a = make_name_t(_1)] >>
                 (
                     "(" >> (argument_expression_list(_r1) | eps[push(*_r1, adobe::array_t())]) > ")"
                 )
                )[push(*_r1, _a, adobe::function_k)];

            variable = identifier(_r1)[push(*_r1, adobe::variable_k)];


            // symbol tables

            keyword_string.add
                ("empty", adobe::name_t("empty"))
                ("true", adobe::name_t("true"))
                ("false", adobe::name_t("false"))
                ;

            const std::size_t num_keywords = keywords.size();
            for (std::size_t i = 0; i < num_keywords; ++i) {
                keyword_string.add(keywords[i].c_str(), keywords[i]);
            }

            eq_op.add
                ("==", adobe::equal_k)
                ("!=", adobe::not_equal_k)
                ;

            rel_op.add
                ("<",  adobe::less_k)
                ("<=", adobe::less_equal_k)
                (">",  adobe::greater_k)
                (">=", adobe::greater_equal_k)
                ;

            add_op.add
                ("+", adobe::add_k)
                ("-", adobe::subtract_k)
                ;

            mul_op.add
                ("*", adobe::multiply_k)
                ("/", adobe::divide_k)
                ("%", adobe::modulus_k)
                ;

            unary_op.add
                ("+", adobe::name_t())
                ("-", adobe::unary_negate_k)
                ("!", adobe::not_k)
                ;

            // define names for rules, to be used in error reporting
#define NAME(x) x.name(#x)
            NAME(start);
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
            NAME(lead_comment);
            NAME(trail_comment);
            NAME(identifier);
            NAME(keyword);
            NAME(number);
            NAME(quoted_string);
            NAME(identifier_string);
            NAME(empty);
            NAME(function);
            NAME(variable);
#undef NAME

            qi::on_error<qi::fail>(start, report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<Iter, std::string(), space_type> string_rule;
        typedef boost::spirit::qi::rule<
            Iter,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::name_t>,
            space_type
        > local_name_rule;
        typedef boost::spirit::qi::rule<
            Iter,
            void(adobe::array_t*),
            boost::spirit::qi::locals<std::size_t>,
            space_type
        > local_size_rule;
        typedef boost::spirit::qi::rule<
            Iter,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::array_t>,
            space_type
        > local_array_rule;
        typedef boost::spirit::qi::rule<Iter, void(adobe::array_t*), space_type> no_locals_rule;

        boost::spirit::qi::rule<Iter, void(), space_type> start;

        // expression grammar
        boost::spirit::qi::rule<
            Iter,
            void(adobe::array_t*),
            boost::spirit::qi::locals<adobe::array_t, adobe::array_t>,
            space_type
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
        no_locals_rule variable_or_function;
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
            Iter,
            void(adobe::array_t*),
            boost::spirit::qi::locals<std::string>,
            space_type
        > string;
        string_rule lead_comment;
        string_rule trail_comment;
        no_locals_rule identifier;
        no_locals_rule keyword;
        no_locals_rule number;
        boost::spirit::qi::rule<Iter, std::string(), space_type> quoted_string;

        // convenience rules
        string_rule identifier_string;
        boost::spirit::qi::rule<
            Iter,
            void(adobe::array_t*),
            space_type
        > empty;
        local_name_rule function;
        no_locals_rule variable;
        boost::spirit::qi::symbols<char, adobe::name_t> keyword_string;
        boost::spirit::qi::symbols<char, adobe::name_t> eq_op;
        boost::spirit::qi::symbols<char, adobe::name_t> rel_op;
        boost::spirit::qi::symbols<char, adobe::name_t> add_op;
        boost::spirit::qi::symbols<char, adobe::name_t> mul_op;
        boost::spirit::qi::symbols<char, adobe::name_t> unary_op;

        boost::phoenix::function<array_t_push_back_> push;
        boost::phoenix::function<report_error_> report_error;
    };

    void verbose_dump(const adobe::array_t& array, std::size_t indent = 0);
    void verbose_dump(const adobe::dictionary_t& array, std::size_t indent = 0);

    void verbose_dump(const adobe::array_t& array, std::size_t indent)
    {
        if (array.empty()) {
            std::cout << std::string(4 * indent, ' ') << "[]\n";
            return;
        }

        std::cout << std::string(4 * indent, ' ') << "[\n";
        ++indent;
        for (adobe::array_t::const_iterator it = array.begin(); it != array.end(); ++it)
        {
            const adobe::any_regular_t& any = *it;
            if (any.type_info() == adobe::type_info<adobe::array_t>()) {
                verbose_dump(any.cast<adobe::array_t>(), indent);
            } else if (any.type_info() == adobe::type_info<adobe::dictionary_t>()) {
                verbose_dump(any.cast<adobe::dictionary_t>(), indent);
            } else {
                std::cout << std::string(4 * indent, ' ')
                          << "type: " << any.type_info() << " "
                          << "value: " << any << "\n";
            }
        }
        --indent;
        std::cout << std::string(4 * indent, ' ') << "]\n";
    }

    void verbose_dump(const adobe::dictionary_t& dictionary, std::size_t indent)
    {
        if (dictionary.empty()) {
            std::cout << std::string(4 * indent, ' ') << "{}\n";
            return;
        }

        std::cout << std::string(4 * indent, ' ') << "{\n";
        ++indent;
        for (adobe::dictionary_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it)
        {
            const adobe::pair<adobe::name_t, adobe::any_regular_t>& pair = *it;
            if (pair.second.type_info() == adobe::type_info<adobe::array_t>()) {
                std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
                verbose_dump(pair.second.cast<adobe::array_t>(), indent);
            } else if (pair.second.type_info() == adobe::type_info<adobe::dictionary_t>()) {
                std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
                verbose_dump(pair.second.cast<adobe::dictionary_t>(), indent);
            } else {
                std::cout << std::string(4 * indent, ' ')
                          << "(" << pair.first << ", "
                          << "type: " << pair.second.type_info() << " "
                          << "value: " << pair.second << ")\n";
            }
        }
        --indent;
        std::cout << std::string(4 * indent, ' ') << "}\n";
    }

    bool TestExpressionParser(const expression_parser<std::string::const_iterator>& expression_p,
                              adobe::array_t& new_parsed_expression,
                              const std::string& expression)
    {
        std::cout << "expression: \"" << expression << "\"\n";
        adobe::array_t original_parsed_expression;
        bool original_parse_failed = false;
        try {
            original_parsed_expression = adobe::parse_adam_expression(expression);
        } catch (const adobe::stream_error_t&) {
            original_parse_failed = true;
        }
        if (original_parse_failed)
            std::cout << "original: <parse failure>\n";
        else
            std::cout << "original: " << original_parsed_expression << "\n";
        using boost::spirit::ascii::space;
        using boost::spirit::qi::phrase_parse;
        bool new_parse_failed =
            !phrase_parse(expression.begin(), expression.end(), expression_p, space);
        if (new_parse_failed)
            std::cout << "new:      <parse failure>\n";
        else
            std::cout << "new:      " << new_parsed_expression << "\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parsed_expression == original_parsed_expression;
        std::cout << (pass ? "PASS" : "FAIL") << "\n";

        if (!pass) {
            std::cout << "original (verbose):\n";
            verbose_dump(original_parsed_expression);
            std::cout << "new (verbose):\n";
            verbose_dump(new_parsed_expression);
        }

        std::cout << "\n";
        new_parsed_expression.clear();

        return pass;
    }

    bool TestExpressionParser()
    {
        adobe::array_t stack;
        std::vector<adobe::name_t> keywords;
        keywords.push_back(adobe::name_t("input"));
        keywords.push_back(adobe::name_t("output"));
        keywords.push_back(adobe::name_t("interface"));
        keywords.push_back(adobe::name_t("logic"));
        keywords.push_back(adobe::name_t("constant"));
        keywords.push_back(adobe::name_t("invariant"));
        keywords.push_back(adobe::name_t("sheet"));
        keywords.push_back(adobe::name_t("unlink"));
        keywords.push_back(adobe::name_t("when"));
        keywords.push_back(adobe::name_t("relate"));
        expression_parser<std::string::const_iterator> expression_p(keywords, stack);

        std::string expressions_file_contents = read_file("test_expressions");
        std::vector<std::string> expressions;
        using boost::algorithm::split;
        using boost::algorithm::is_any_of;
        split(expressions, expressions_file_contents, is_any_of("\n"));

        std::size_t passes = 0;
        std::size_t failures = 0;
        for (std::size_t i = 0; i < expressions.size(); ++i) {
            if (!expressions[i].empty()) {
                if (TestExpressionParser(expression_p, stack, expressions[i]))
                    ++passes;
                else
                    ++failures;
            }
        }

        std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";

        exit(0);

        return false;
    }

    bool dummy2 = TestExpressionParser();

}

class AdamDialog :
    public GG::Wnd
{
private:
    static const GG::X WIDTH;
    static const GG::Y HEIGHT;

public:
    AdamDialog();

    adobe::dictionary_t Result();

    virtual void Render();
    virtual bool Run();

private:
    bool HandleActions (adobe::name_t name, const adobe::any_regular_t&);

    boost::shared_ptr<GG::Font> m_font;
    GG::Spin<PathTypes>* m_path_spin;
    GG::Edit* m_flatness_edit;
    GG::Button* m_ok;

    GG::AdamModalDialog m_adam_modal_dialog;
};


// implementations

std::ostream& operator<<(std::ostream& os, PathTypes p)
{
    os << (p == NONE ? "None" : "Path_1");
    return os;
}

std::istream& operator>>(std::istream& is, PathTypes& p)
{
    std::string path_str;
    is >> path_str;
    p = path_str == "None" ? NONE : PATH_1;
    return is;
}

const GG::X AdamDialog::WIDTH(250);
const GG::Y AdamDialog::HEIGHT(75);

AdamDialog::AdamDialog() :
    Wnd((AdamGGApp::GetGUI()->AppWidth() - WIDTH) / 2,
        (AdamGGApp::GetGUI()->AppHeight() - HEIGHT) / 2,
        WIDTH, HEIGHT, GG::INTERACTIVE | GG::MODAL),
    m_font(AdamGGApp::GetGUI()->GetStyleFactory()->DefaultFont()),
    m_path_spin(new GG::Spin<PathTypes>(GG::X0, GG::Y0, GG::X(50),
                                        NONE, PathTypes(1), NONE, PATH_1,
                                        false, m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_flatness_edit(new GG::Edit(GG::X0, GG::Y0, GG::X(50),
                                 "0.0", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_ok(new GG::Button(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                        "Ok", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_adam_modal_dialog("sheet clipping_path"
                        "{"
                        "output:"
                        "    result                  <== { path: path, flatness: flatness };"
                        ""
                        "interface:"
                        "    unlink flatness : 0.0   <== (path == 0) ? 0.0 : flatness;"
                        "    path            : 1;"
                        "}",
                        adobe::dictionary_t(),
                        adobe::dictionary_t(),
                        GG::ADAM_DIALOG_DISPLAY_ALWAYS,
                        this,
                        boost::bind(&AdamDialog::HandleActions, this, _1, _2),
                        boost::filesystem::path())
{
    GG::TextControl* path_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Path:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);
    GG::TextControl* flatness_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Flatness:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, WIDTH, HEIGHT, 2u, 3u);
    layout->SetBorderMargin(2);
    layout->SetCellMargin(4);
    layout->Add(path_label, 0, 0);
    layout->Add(m_path_spin, 0, 1);
    layout->Add(m_ok, 0, 2);
    layout->Add(flatness_label, 1, 0);
    layout->Add(m_flatness_edit, 1, 1);
    SetLayout(layout);

    m_adam_modal_dialog.BindCell<double, PathTypes>(*m_path_spin, adobe::name_t("path"));
    m_adam_modal_dialog.BindCell<double, double>(*m_flatness_edit, adobe::name_t("flatness"));

    GG::Connect(m_ok->ClickedSignal,
                boost::bind(boost::ref(m_adam_modal_dialog.DialogActionSignal),
                            adobe::name_t("ok"),
                            adobe::any_regular_t()));
}

adobe::dictionary_t AdamDialog::Result()
{ return m_adam_modal_dialog.Result().m_result_values; }

void AdamDialog::Render()
{ FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_SHADOW, GG::CLR_SHADOW, 1); }

bool AdamDialog::Run()
{
    if (m_adam_modal_dialog.NeedUI())
        return Wnd::Run();
    return true;
}

bool AdamDialog::HandleActions (adobe::name_t name, const adobe::any_regular_t&)
{ return name == adobe::static_name_t("ok"); }


AdamGGApp::AdamGGApp() : 
    SDLGUI(1024, 768, false, "Adam App")
{}

void AdamGGApp::Enter2DMode()
{
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void AdamGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void AdamGGApp::Render()
{
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

    // DeltaT() returns the time in whole milliseconds since the last frame
    // was rendered (in other words, since this method was last invoked).
    glRotated(DeltaT() * DEGREES_PER_MS, 0.0, 1.0, 0.0);

    glBegin(GL_QUADS);

    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(1.0, 1.0, 1.0);

    glColor3d(1.0, 0.5, 0.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0,-1.0);
    glVertex3d(1.0, -1.0,-1.0);

    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);

    glColor3d(1.0, 1.0, 0.0);
    glVertex3d(1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, -1.0);

    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, 1.0);

    glColor3d(1.0, 0.0, 1.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, -1.0);

    glEnd();

    GG::GUI::Render();
}

void AdamGGApp::GLInit()
{
    double ratio = Value(AppWidth() * 1.0) / Value(AppHeight());

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);
    gluLookAt(0.0, 0.0, 5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

void AdamGGApp::Initialize()
{
    SDL_WM_SetCaption("Adam GG App", "Adam GG App");

    AdamDialog adam_dlg;
    adam_dlg.Run();

    adobe::dictionary_t dictionary = adam_dlg.Result();

    std::ostringstream results_str;
    results_str << "result:\n"
                << "path = " << dictionary[adobe::name_t("path")].cast<double>() << "\n"
                << "flatness = " << dictionary[adobe::name_t("flatness")].cast<double>();

    GG::ThreeButtonDlg results_dlg(GG::X(200), GG::Y(100), results_str.str(),
                                   GetStyleFactory()->DefaultFont(), GG::CLR_SHADOW, 
                                   GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);
    results_dlg.Run();

    Exit(0);
}

// This gets called as the application is exit()ing, and as the name says,
// performs all necessary cleanup at the end of the app's run.
void AdamGGApp::FinalCleanup()
{}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    AdamGGApp app;

    try {
        app();
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
    }
    return 0;
}
