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

Action::Action(const std::vector<GG::Key>& keys) :
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
    using phoenix::static_cast_;
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
        GG::Key (),
        ascii::space_type
    > KeyRule;

    typedef boost::spirit::qi::rule<
        const char*,
        Action(),
        boost::spirit::qi::locals<int, int, int, int, std::vector<GG::Key> >,
        ascii::space_type
    > ActionRule;

    typedef boost::spirit::qi::rule<
        const char*,
        Actions (),
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

    qi::symbols<char, GG::Key> named_keys;
    named_keys.add
#define KEY(x) (#x, GG::GGK_##x)
        KEY(BACKSPACE)
        KEY(TAB)
        KEY(CLEAR)
        KEY(RETURN)
        KEY(PAUSE)
        KEY(ESCAPE)
        KEY(SPACE)
        KEY(EXCLAIM)
        KEY(QUOTEDBL)
        KEY(HASH)
        KEY(DOLLAR)
        KEY(AMPERSAND)
        KEY(QUOTE)
        KEY(LEFTPAREN)
        KEY(RIGHTPAREN)
        KEY(ASTERISK)
        KEY(PLUS)
        KEY(COMMA)
        KEY(MINUS)
        KEY(PERIOD)
        KEY(SLASH)
        KEY(DELETE)
        KEY(WORLD_0)
        KEY(WORLD_1)
        KEY(WORLD_2)
        KEY(WORLD_3)
        KEY(WORLD_4)
        KEY(WORLD_5)
        KEY(WORLD_6)
        KEY(WORLD_7)
        KEY(WORLD_8)
        KEY(WORLD_9)
        KEY(WORLD_10)
        KEY(WORLD_11)
        KEY(WORLD_12)
        KEY(WORLD_13)
        KEY(WORLD_14)
        KEY(WORLD_15)
        KEY(WORLD_16)
        KEY(WORLD_17)
        KEY(WORLD_18)
        KEY(WORLD_19)
        KEY(WORLD_20)
        KEY(WORLD_21)
        KEY(WORLD_22)
        KEY(WORLD_23)
        KEY(WORLD_24)
        KEY(WORLD_25)
        KEY(WORLD_26)
        KEY(WORLD_27)
        KEY(WORLD_28)
        KEY(WORLD_29)
        KEY(WORLD_30)
        KEY(WORLD_31)
        KEY(WORLD_32)
        KEY(WORLD_33)
        KEY(WORLD_34)
        KEY(WORLD_35)
        KEY(WORLD_36)
        KEY(WORLD_37)
        KEY(WORLD_38)
        KEY(WORLD_39)
        KEY(WORLD_40)
        KEY(WORLD_41)
        KEY(WORLD_42)
        KEY(WORLD_43)
        KEY(WORLD_44)
        KEY(WORLD_45)
        KEY(WORLD_46)
        KEY(WORLD_47)
        KEY(WORLD_48)
        KEY(WORLD_49)
        KEY(WORLD_50)
        KEY(WORLD_51)
        KEY(WORLD_52)
        KEY(WORLD_53)
        KEY(WORLD_54)
        KEY(WORLD_55)
        KEY(WORLD_56)
        KEY(WORLD_57)
        KEY(WORLD_58)
        KEY(WORLD_59)
        KEY(WORLD_60)
        KEY(WORLD_61)
        KEY(WORLD_62)
        KEY(WORLD_63)
        KEY(WORLD_64)
        KEY(WORLD_65)
        KEY(WORLD_66)
        KEY(WORLD_67)
        KEY(WORLD_68)
        KEY(WORLD_69)
        KEY(WORLD_70)
        KEY(WORLD_71)
        KEY(WORLD_72)
        KEY(WORLD_73)
        KEY(WORLD_74)
        KEY(WORLD_75)
        KEY(WORLD_76)
        KEY(WORLD_77)
        KEY(WORLD_78)
        KEY(WORLD_79)
        KEY(WORLD_80)
        KEY(WORLD_81)
        KEY(WORLD_82)
        KEY(WORLD_83)
        KEY(WORLD_84)
        KEY(WORLD_85)
        KEY(WORLD_86)
        KEY(WORLD_87)
        KEY(WORLD_88)
        KEY(WORLD_89)
        KEY(WORLD_90)
        KEY(WORLD_91)
        KEY(WORLD_92)
        KEY(WORLD_93)
        KEY(WORLD_94)
        KEY(WORLD_95)
        KEY(KP0)
        KEY(KP1)
        KEY(KP2)
        KEY(KP3)
        KEY(KP4)
        KEY(KP5)
        KEY(KP6)
        KEY(KP7)
        KEY(KP8)
        KEY(KP9)
        KEY(KP_PERIOD)
        KEY(KP_DIVIDE)
        KEY(KP_MULTIPLY)
        KEY(KP_MINUS)
        KEY(KP_PLUS)
        KEY(KP_ENTER)
        KEY(KP_EQUALS)
        KEY(UP)
        KEY(DOWN)
        KEY(RIGHT)
        KEY(LEFT)
        KEY(INSERT)
        KEY(HOME)
        KEY(END)
        KEY(PAGEUP)
        KEY(PAGEDOWN)
        KEY(F1)
        KEY(F2)
        KEY(F3)
        KEY(F4)
        KEY(F5)
        KEY(F6)
        KEY(F7)
        KEY(F8)
        KEY(F9)
        KEY(F10)
        KEY(F11)
        KEY(F12)
        KEY(F13)
        KEY(F14)
        KEY(F15)
        KEY(NUMLOCK)
        KEY(CAPSLOCK)
        KEY(SCROLLOCK)
        KEY(RSHIFT)
        KEY(LSHIFT)
        KEY(RCTRL)
        KEY(LCTRL)
        KEY(RALT)
        KEY(LALT)
        KEY(RMETA)
        KEY(LMETA)
        KEY(LSUPER)
        KEY(RSUPER)
        KEY(MODE)
        KEY(COMPOSE)
        KEY(HELP)
        KEY(PRINT)
        KEY(SYSREQ)
        KEY(BREAK)
        KEY(MENU)
        KEY(POWER)
        KEY(EURO)
        KEY(UNDO)
#undef KEY
        ;

    KeyRule key
        =     named_keys [ _val = _1 ]
        |     (~char_('}')) [ _val = static_cast_<GG::Key>(_1) ]
        ;

    ActionRule keys
        =     "keys{"
        >>    (*key) [ _e = _1 ]
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
