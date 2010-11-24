#include "Lexer.h"

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

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_variable.hpp>
#include <boost/spirit/home/phoenix/object.hpp>
#include <boost/spirit/home/phoenix/statement/if.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/spirit/home/phoenix.hpp>

#include <GG/adobe/adam_parser.hpp> // for testing only
#include "AdamParser.h" // for testing only
#include "ExpressionWriter.h" // for testing only
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

    struct lexer_test_grammar 
        : boost::spirit::qi::grammar<GG::token_iterator, GG::skipper_type>
    {
        lexer_test_grammar(GG::lexer& tok) :
            lexer_test_grammar::base_type(start)
            {
                using boost::spirit::qi::_1;
                using boost::spirit::qi::lit;
                using boost::phoenix::at_c;
                using boost::phoenix::val;

#define DUMP_TOK(x) tok.x[std::cout << val(#x" -- ") << _1 << std::endl]
#define DUMP_LIT(x) lit(x)[std::cout << val("'") << val(x) << val("'") << std::endl]
#define DUMP_UNATTRIBUTED(x) tok.x[std::cout << val(#x) << std::endl]
#define DUMP_KEYWORD_TOK(x) x[std::cout << val("keyword -- ") << _1 << std::endl]

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

                start =
                    +(
                        DUMP_KEYWORD_TOK(input)
                      | DUMP_KEYWORD_TOK(output)
                      | DUMP_KEYWORD_TOK(interface)
                      | DUMP_KEYWORD_TOK(logic)
                      | DUMP_KEYWORD_TOK(constant)
                      | DUMP_KEYWORD_TOK(invariant)
                      | DUMP_KEYWORD_TOK(sheet)
                      | DUMP_KEYWORD_TOK(unlink)
                      | DUMP_KEYWORD_TOK(when)
                      | DUMP_KEYWORD_TOK(relate)
                      | DUMP_TOK(identifier)
                      | DUMP_TOK(lead_comment)
                      | DUMP_TOK(trail_comment)
                      | DUMP_TOK(quoted_string)
                      | DUMP_TOK(number)
                      | DUMP_TOK(eq_op)
                      | DUMP_TOK(rel_op)
                      | DUMP_TOK(mul_op)
                      | DUMP_TOK(keyword_true_false)
                      | DUMP_UNATTRIBUTED(keyword_empty)
                      | DUMP_UNATTRIBUTED(define)
                      | DUMP_UNATTRIBUTED(or_)
                      | DUMP_UNATTRIBUTED(and_)
                      | DUMP_LIT('-')
                      | DUMP_LIT('+')
                      | DUMP_LIT('!')
                      | DUMP_LIT('?')
                      | DUMP_LIT(':')
                      | DUMP_LIT('.')
                      | DUMP_LIT(',')
                      | DUMP_LIT('(')
                      | DUMP_LIT(')')
                      | DUMP_LIT('[')
                      | DUMP_LIT(']')
                      | DUMP_LIT('{')
                      | DUMP_LIT('}')
                      | DUMP_LIT('@')
                      | DUMP_LIT(';')
                    )
                    ;

#undef DUMP_TOK
#undef DUMP_LIT
            }

        boost::spirit::qi::rule<GG::token_iterator, GG::skipper_type> start;
    };

    bool TestLexer()
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
        const std::size_t s_num_keywords = sizeof(s_keywords) / sizeof(s_keywords[0]);

        GG::lexer lexer(s_keywords, s_keywords + s_num_keywords);
        lexer_test_grammar test_grammar(lexer);

        const std::string str = read_file("test_expressions");

        std::string::const_iterator it = str.begin();
        token_iterator iter = lexer.begin(it, str.end());
        token_iterator end = lexer.end();

        boost::spirit::qi::phrase_parse(iter,
                                        end,
                                        test_grammar,
                                        boost::spirit::qi::in_state("WS")[lexer.self]);

        exit(0);

        return false;
    }

    //bool dummy2 = TestLexer();

    struct expression_parser :
        boost::spirit::qi::grammar<GG::token_iterator, void(), GG::skipper_type>
    {
        expression_parser(adobe::array_t& stack, const AdamExpressionParserRule& expression) :
            expression_parser::base_type(start)
        {
            start = expression(boost::phoenix::ref(stack));
        }

        boost::spirit::qi::rule<GG::token_iterator, void(), GG::skipper_type> start;
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

    bool TestExpressionParser(const expression_parser& expression_p,
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
        using boost::spirit::qi::phrase_parse;
        std::string::const_iterator it = expression.begin();
        token_iterator iter = AdamLexer().begin(it, expression.end());
        token_iterator end = AdamLexer().end();
        bool new_parse_failed =
            !phrase_parse(iter,
                          end,
                          expression_p,
                          boost::spirit::qi::in_state("WS")[AdamLexer().self]);
        if (new_parse_failed)
            std::cout << "new:      <parse failure>\n";
        else
            std::cout << "new:      " << new_parsed_expression << "\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parsed_expression == original_parsed_expression;
        std::cout << (pass ? "PASS" : "FAIL") << "\n";

        if (pass) {
            std::cout << GG::WriteExpression(new_parsed_expression) << '\n';
        } else {
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
        expression_parser expression_p(stack, AdamExpressionParser());

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

    //bool dummy3 = TestExpressionParser();


    bool instrument_positions = false;

    struct StoreAddCellParams
    {
        StoreAddCellParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::adam_callback_suite_t::cell_type_t type,
                        adobe::name_t cell_name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& expr_or_init,
                        const std::string& brief,
                        const std::string& detailed)
        {
            std::string type_str;
            switch (type)
            {
            case adobe::adam_callback_suite_t::input_k: type_str = "input_k";
            case adobe::adam_callback_suite_t::output_k: type_str = "output_k";
            case adobe::adam_callback_suite_t::constant_k: type_str = "constant_k";
            case adobe::adam_callback_suite_t::logic_k: type_str = "logic_k";
            case adobe::adam_callback_suite_t::invariant_k: type_str = "invariant_k";
            }
            push_back(m_array, type_str);
            push_back(m_array, cell_name);
            if (instrument_positions) {
                std::cerr << position.stream_name() << ":"
                          << position.line_number_m << ":"
                          << position.line_start_m << ":"
                          << position.position_m << ":";
                if (position.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                      m_str.begin() + position.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position.stream_name());
                push_back(m_array, position.line_number_m);
                push_back(m_array, std::size_t(position.line_start_m));
                push_back(m_array, std::size_t(position.position_m));
            }
            push_back(m_array, expr_or_init);
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddRelationParams
    {
        StoreAddRelationParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(const adobe::line_position_t& position,
                        const adobe::array_t& conditional,
                        const adobe::adam_callback_suite_t::relation_t* first,
                        const adobe::adam_callback_suite_t::relation_t* last,
                        const std::string& brief,
                        const std::string& detailed)
        {
            if (instrument_positions) {
                std::cerr << position.stream_name() << ":"
                          << position.line_number_m << ":"
                          << position.line_start_m << ":"
                          << position.position_m << ":";
                if (position.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position.line_start_m,
                                                      m_str.begin() + position.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position.stream_name());
                push_back(m_array, position.line_number_m);
                push_back(m_array, std::size_t(position.line_start_m));
                push_back(m_array, std::size_t(position.position_m));
            }
            push_back(m_array, conditional);
            while (first != last) {
                push_back(m_array, first->name_m);
                if (instrument_positions) {
                    std::cerr << first->position_m.stream_name() << ":"
                              << first->position_m.line_number_m << ":"
                              << first->position_m.line_start_m << ":"
                              << first->position_m.position_m << ":";
                    if (position.line_start_m) {
                        std::cerr << " \"" << std::string(m_str.begin() + first->position_m.line_start_m,
                                                          m_str.begin() + first->position_m.position_m)
                                  << "\"";
                    }
                    std::cerr << "\n";
                } else {
                    push_back(m_array, first->position_m.stream_name());
                    push_back(m_array, first->position_m.line_number_m);
                    push_back(m_array, std::size_t(first->position_m.line_start_m));
                    push_back(m_array, std::size_t(first->position_m.position_m));
                }
                push_back(m_array, first->expression_m);
                push_back(m_array, first->detailed_m);
                push_back(m_array, first->brief_m);
                ++first;
            }
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    struct StoreAddInterfaceParams
    {
        StoreAddInterfaceParams(adobe::array_t& array, const std::string& str) :
            m_array(array),
            m_str(str)
            {}

        void operator()(adobe::name_t cell_name,
                        bool linked,
                        const adobe::line_position_t& position1,
                        const adobe::array_t& initializer,
                        const adobe::line_position_t& position2,
                        const adobe::array_t& expression,
                        const std::string& brief,
                        const std::string& detailed)
        {
            push_back(m_array, cell_name);
            push_back(m_array, linked);
            if (instrument_positions) {
                std::cerr << position1.stream_name() << ":"
                          << position1.line_number_m << ":"
                          << position1.line_start_m << ":"
                          << position1.position_m << ":";
                if (position1.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position1.line_start_m,
                                                      m_str.begin() + position1.position_m)
                          << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position1.stream_name());
                push_back(m_array, position1.line_number_m);
                push_back(m_array, std::size_t(position1.line_start_m));
                push_back(m_array, std::size_t(position1.position_m));
            }
            push_back(m_array, initializer);
            if (instrument_positions) {
                std::cerr << position2.stream_name() << ":"
                          << position2.line_number_m << ":"
                          << position2.line_start_m << ":"
                          << position2.position_m << ":";
                if (position2.line_start_m) {
                    std::cerr << " \"" << std::string(m_str.begin() + position2.line_start_m,
                                                      m_str.begin() + position2.position_m)
                              << "\"";
                }
                std::cerr << "\n";
            } else {
                push_back(m_array, position2.stream_name());
                push_back(m_array, position2.line_number_m);
                push_back(m_array, std::size_t(position2.line_start_m));
                push_back(m_array, std::size_t(position2.position_m));
            }
            push_back(m_array, expression);
            push_back(m_array, brief);
            push_back(m_array, detailed);
        }

        adobe::array_t& m_array;
        const std::string& m_str;
    };

    bool TestAdamParser(adobe::array_t& new_parse,
                        adobe::array_t& old_parse,
                        const std::string& filename,
                        const std::string& sheet)
    {
        adobe::adam_callback_suite_t old_parse_callbacks;
        old_parse_callbacks.add_cell_proc_m = StoreAddCellParams(old_parse, sheet);
        old_parse_callbacks.add_relation_proc_m = StoreAddRelationParams(old_parse, sheet);
        old_parse_callbacks.add_interface_proc_m = StoreAddInterfaceParams(old_parse, sheet);

        adobe::adam_callback_suite_t new_parse_callbacks;
        new_parse_callbacks.add_cell_proc_m = StoreAddCellParams(new_parse, sheet);
        new_parse_callbacks.add_relation_proc_m = StoreAddRelationParams(new_parse, sheet);
        new_parse_callbacks.add_interface_proc_m = StoreAddInterfaceParams(new_parse, sheet);

        std::cout << "sheet:\"\n" << sheet << "\n\"\n"
                  << "filename: " << filename << '\n';
        bool original_parse_failed = false;
        try {
            std::stringstream ss(sheet);
            adobe::parse(ss, adobe::line_position_t(filename.c_str()), old_parse_callbacks);
        } catch (const adobe::stream_error_t& e) {
#if 0
            for (std::size_t i = 0; i < e.line_position_set().size(); ++i) {
                std::cout << "Line position: \"" << e.line_position_set()[i] << "\"\n";
            }
#endif
            original_parse_failed = true;
        }
        if (original_parse_failed)
            std::cout << "original: <parse failure>\n";
        else
            std::cout << "original: " << old_parse << "\n";
        bool new_parse_failed = !Parse(sheet, filename, new_parse_callbacks);
        if (new_parse_failed)
            std::cout << "new:      <parse failure>\n";
        else
            std::cout << "new:      " << new_parse << "\n";
        bool pass =
            original_parse_failed && new_parse_failed ||
            new_parse == old_parse;
        std::cout << (pass ? "PASS" : "FAIL") << "\n";

        if (!pass) {
            std::cout << "original (verbose):\n";
            verbose_dump(old_parse);
            std::cout << "new (verbose):\n";
            verbose_dump(new_parse);
        }

        std::cout << "\n";
        new_parse.clear();
        old_parse.clear();
        return pass;
    }

    bool TestAdamParser()
    {
        adobe::array_t old_parse;
        adobe::array_t new_parse;

        std::size_t passes = 0;
        std::size_t failures = 0;

        namespace fs = boost::filesystem;
        fs::path current_path = fs::current_path();
        fs::directory_iterator it(current_path);
        fs::directory_iterator end_it;
        for (; it != end_it; ++it) {
            if (boost::algorithm::ends_with(it->string(), "adm")) {
                std::string file_contents = read_file(it->string());
                if (!file_contents.empty()) {
                    if (TestAdamParser(new_parse, old_parse, it->string(), file_contents))
                        ++passes;
                    else
                        ++failures;
                }
            }
        }

        std::cout << "Summary: " << passes << " passed, " << failures << " failed\n";

        exit(0);

        return false;
    }

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
    if (1 < argc)
        GG::instrument_positions = true;

    GG::TestAdamParser();

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
