#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "Action.h"


Action::Action()
{}

Action::Action(int x, int y, Semantic semantic) :
    m_pt(GG::X(x), GG::Y(y)),
    m_semantic(semantic),
    m_pressed_for_drag(false),
    m_moved_for_drag(false),
    m_released_for_drag(false)
{}

Action::Action(int x1, int y1, int x2, int y2) :
    m_pt(GG::X(x1), GG::Y(y1)),
    m_pt_2(GG::X(x2), GG::Y(y2)),
    m_semantic(Drag),
    m_pressed_for_drag(false),
    m_moved_for_drag(false),
    m_released_for_drag(false)
{}

Action::Action(const std::string& keys) :
    m_keys(keys),
    m_pressed_for_drag(false),
    m_moved_for_drag(false),
    m_released_for_drag(false)
{}

Actions ParseActions(const std::string& str)
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    using ascii::char_;
    using phoenix::construct;
    using phoenix::push_back;
    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_d;
    using qi::_e;
    using qi::_val;
    using qi::as_string;
    using qi::int_;
    using qi::lit;

    typedef boost::spirit::qi::rule<
        const char*,
        Action(),
        boost::spirit::qi::locals<int, int, int, int, std::string>,
        ascii::space_type
    > ActionRule;

    typedef boost::spirit::qi::rule<
        const char*,
        Actions(),
        ascii::space_type
    > ActionsRule;

    ActionRule left_click
        =     int_ [ _a = _1 ]
        >>    ','
        >>    int_ [ _b = _1 ]
        [ _val = construct<Action>(_a, _b, Action::LeftClick) ]
        ;

    ActionRule right_click
        =     int_ [ _a = _1 ]
        >>    ','
        >>    int_ [ _b = _1 ]
        >>    lit('r')
        [ _val = construct<Action>(_a, _b, Action::RightClick) ]
        ;

    ActionRule double_click
        =     int_ [ _a = _1 ]
        >>    ','
        >>    int_ [ _b = _1 ]
        >>    lit('b')
        [ _val = construct<Action>(_a, _b, Action::DoubleClick) ]
        ;

    ActionRule drag
        =     int_ [ _a = _1 ]
        >>    ','
        >>    int_ [ _b = _1 ]
        >>    ','
        >>    int_ [ _c = _1 ]
        >>    ','
        >>    int_ [ _d = _1 ]
        >>    lit('d')
        [ _val = construct<Action>(_a, _b, _c, _d) ]
        ;

    ActionRule keys
        =     "keys{"
        >>    as_string[*~char_('}')] [ _e = _1 ]
        >>    lit('}')
        [ _val = construct<Action>(_e) ]
        ;

    ActionRule any_action;
    any_action
        %=     drag | right_click | double_click | left_click | keys
        ;

    ActionsRule actions
        =     any_action [ push_back(_val, _1) ]
        >>  * (
                  ','
               >> any_action [ push_back(_val, _1) ]
              )
        ;

    Actions retval;
    const char* str_ = str.c_str();
    phrase_parse(str_, str_ + str.size(), actions, ascii::space, retval);

    return retval;
}
