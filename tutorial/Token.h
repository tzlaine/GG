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

/** \file Token.h TODO. */

#ifndef _GG_Token_h_
#define _GG_Token_h_

#include <boost/spirit/home/lex/lexer/terminals.hpp>
#include <boost/spirit/home/lex/lexer/lexertl/token.hpp>


namespace GG {

template <typename Iter = const char*,
          typename AttributeTypes = boost::mpl::vector0<>,
          typename HasState = boost::mpl::true_>
struct position_tracking_token :
    boost::spirit::lex::lexertl::token<Iter, AttributeTypes, HasState>
{
    typedef boost::spirit::lex::lexertl::token<Iter, AttributeTypes, HasState> base;
    typedef typename base::iterator_type iterator_type;
    typedef typename base::has_state has_state;
    typedef typename base::id_type id_type;
    typedef typename base::token_value_type token_value_type;

    //  default constructed tokens correspond to EOI tokens
    position_tracking_token() :
        base(),
        m_matched_range(),
        m_line_number(std::numeric_limits<std::size_t>::max()),
        m_line_start()
        {}

    //  construct an invalid token
    explicit position_tracking_token(int i) :
        base(i),
        m_matched_range(),
        m_line_number(std::numeric_limits<std::size_t>::max()),
        m_line_start()
        {}

    position_tracking_token(id_type id, std::size_t size) :
        base(id, size),
        m_matched_range(),
        m_line_number(std::numeric_limits<std::size_t>::max()),
        m_line_start()
        {}

    position_tracking_token(id_type id, std::size_t size, token_value_type tvt) :
        base(id, size, tvt),
        m_matched_range(),
        m_line_number(std::numeric_limits<std::size_t>::max()),
        m_line_start()
        {}

    position_tracking_token(id_type id, std::size_t size, const Iter& first, const Iter& last) :
        base(id, size, first, last),
        m_matched_range(first, last),
        m_line_number(std::numeric_limits<std::size_t>::max()),
        m_line_start()
        {}

    const char* filename() const
        { return s_filename; }

    const std::pair<Iter, Iter>& matched_range() const
        { return m_matched_range; }

    std::size_t line_number() const
        {
            line_start();
            return m_line_number;
        }

    Iter line_start() const
        {
            if (m_line_number == std::numeric_limits<std::size_t>::max()) {
                m_line_number = std::count(s_begin, m_matched_range.first, '\n');
                Iter it = m_matched_range.first;
                for (; it != s_begin; --it) {
                    if (*it == '\n') {
                        ++it;
                        break;
                    }
                }
                m_line_start = it;
            }
            return m_line_start;
        }

    static const char* s_filename;
    static Iter s_begin;

private:
    std::pair<Iter, Iter> m_matched_range;
    mutable std::size_t m_line_number;
    mutable Iter m_line_start;
};

template <typename Iter, typename AttributeTypes, typename HasState>
const char* position_tracking_token<Iter, AttributeTypes, HasState>::s_filename = 0;

template <typename Iter, typename AttributeTypes, typename HasState>
Iter position_tracking_token<Iter, AttributeTypes, HasState>::s_begin;

template <typename Iter, typename AttributeTypes, typename HasState>
inline bool 
operator==(const position_tracking_token<Iter, AttributeTypes, HasState>& lhs, 
           const position_tracking_token<Iter, AttributeTypes, HasState>& rhs)
{ return lhs.id() == rhs.id(); }

template <typename Iter, typename AttributeTypes, typename HasState>
inline bool 
token_is_valid(const position_tracking_token<Iter, AttributeTypes, HasState>& t)
{ return t.is_valid(); }

}

namespace boost { namespace spirit { namespace traits
{
    template <typename Attribute, typename Iter, typename AttributeTypes, typename HasState>
    struct assign_to_attribute_from_value<Attribute,
                                          GG::position_tracking_token<Iter, AttributeTypes, HasState> >
    {
        static void
        call(const GG::position_tracking_token<Iter, AttributeTypes, HasState>& t, Attribute& attr)
        {
            if (0 == t.value().which()) {
                typedef iterator_range<Iter> iterpair_type;
                const iterpair_type& ip = get<iterpair_type>(t.value());
                spirit::traits::assign_to(ip.begin(), ip.end(), attr);
                typedef GG::position_tracking_token<Iter, AttributeTypes, HasState> token_type;
                const_cast<token_type&>(t).value() = attr;
            } else {
                spirit::traits::assign_to(get<Attribute>(t.value()), attr);
            }
        }
    };

    template <typename Attribute, typename Iter, typename HasState>
    struct assign_to_attribute_from_value<Attribute,
                                          GG::position_tracking_token<Iter, mpl::vector0<>, HasState> >
    {
        static void
        call(const GG::position_tracking_token<Iter, mpl::vector0<>, HasState>& t, Attribute& attr)
        { spirit::traits::assign_to(t.value().begin(), t.value().end(), attr); }
    };

    template <typename Attribute, typename Iter, typename HasState>
    struct assign_to_attribute_from_value<Attribute,
                                          GG::position_tracking_token<Iter, mpl::vector<>, HasState> >
    {
        static void
        call(const GG::position_tracking_token<Iter, mpl::vector<>, HasState>& t, Attribute& attr)
        { spirit::traits::assign_to(t.value().begin(), t.value().end(), attr); }
    };

    template <typename Attribute, typename Iter, typename HasState>
    struct assign_to_attribute_from_value<Attribute,
                                          GG::position_tracking_token<Iter, lex::omit, HasState> >
    {
        static void
        call(const GG::position_tracking_token<Iter, lex::omit, HasState>& t, Attribute& attr)
        {}
    };

    template <typename Iter, typename AttributeTypes, typename HasState>
    struct assign_to_attribute_from_value<
        fusion::vector2<std::size_t, iterator_range<Iter> >,
        GG::position_tracking_token<Iter, AttributeTypes, HasState> >
    {
        static void
        call(const GG::position_tracking_token<Iter, AttributeTypes, HasState>& t,
             fusion::vector2<std::size_t, iterator_range<Iter> >& attr)
        {
            typedef iterator_range<Iter> iterpair_type;
            typedef fusion::vector2<std::size_t, iterator_range<Iter> > attribute_type;
            const iterpair_type& ip = get<iterpair_type>(t.value());
            attr = attribute_type(t.id(), get<iterpair_type>(t.value()));
        }
    };

    template <typename Iter, typename Attribute, typename HasState>
    struct token_printer_debug<GG::position_tracking_token<Iter, Attribute, HasState> >
    {
        typedef GG::position_tracking_token<Iter, Attribute, HasState> token_type;
        template <typename Out>
        static void print(Out& out, const token_type& val)
        {
            out << '<';
            spirit::traits::print_token(out, val.value());
            out << '>';
        }
    };

} } }

#endif // _GG_Token_h_
