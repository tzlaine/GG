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
   
/** \file ReportParseError.h TODO. */

#ifndef _GG_ReportParseError_h_
#define _GG_ReportParseError_h_

#include <GG/Export.h>
#include <GG/LexerFwd.h>

#include <boost/algorithm/string/replace.hpp>


namespace GG {

    namespace detail {

        inline void default_send_error_string(const std::string& str)
        { std::cerr << str; }

        extern GG_API const char* s_filename;
        extern GG_API text_iterator s_begin;
        extern GG_API text_iterator s_end;

    }

    template <typename TokenType>
    struct report_error_
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result
        { typedef void type; };

        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        void operator()(Arg1 first, Arg2 last, Arg3 it, Arg4 rule_name) const
            {
                std::string error_string;
                generate_error_string(*first, *it, rule_name, first == last, error_string);
                send_error_string(error_string);
            }

        static boost::function<void (const std::string&)> send_error_string;

    private:
        void generate_error_string(const TokenType& first,
                                   const TokenType& it,
                                   const boost::spirit::info& rule_name,
                                   bool at_end,
                                   std::string& str) const
            {
                std::stringstream is;

                bool empty_match = it.matched_.first == it.matched_.second;
                std::size_t line_number;
                if (empty_match)
                    line_number = std::count(detail::s_begin, detail::s_end, '\n') + 1;
                else
                    line_number = boost::spirit::get_line(it.matched_.first);

                is << detail::s_filename << ":" << line_number << ": "
                   << "Parse error: expected " << rule_name;

                if (at_end || empty_match) {
                    is << " before end of input.\n";
                } else {
                    // Use the end of the token's matched range, if its entire match was
                    // whitespace.
                    std::size_t whitespace = 0;
                    const std::string match(it.matched_.first, it.matched_.second);
                    if (match.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
                        whitespace = match.size();

                    text_iterator line_start = boost::spirit::get_line_start(detail::s_begin, it.matched_.second);
                    ++line_start;
                    text_iterator line_end = it.matched_.second;
                    while (line_end != detail::s_end && *line_end != '\n' && *line_end != '\r') {
                        ++line_end;
                    }
                    std::string line(line_start, line_end);
                    boost::algorithm::replace_all(line, "\t", "    ");

                    std::size_t column = boost::spirit::get_column(detail::s_begin, it.matched_.first);

                    std::string spacing;
                    // Spirit reports columns as 1-based, and we only want to print n-1 spaces before the position indicator.
                    if (2u < column)
                        column -= 2;
                    else
                        column = 0;
                    if (column + whitespace)
                        spacing = std::string(column + whitespace, ' ');

                    is << " here:\n"
                       << "  " << line << "\n"
                       << "  " << spacing << std::string(match.size(), '^')
                       << std::endl;
                }

                str = is.str();
            }
    };

    template <typename TokenType>
    boost::function<void (const std::string&)> report_error_<TokenType>::send_error_string =
        &detail::default_send_error_string;

}

#endif // _GG_ReportParseError_h_
