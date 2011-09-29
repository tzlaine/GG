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

#include <GG/ReportParseError.h>

#include <boost/algorithm/string/replace.hpp>


namespace GG {

    const boost::phoenix::function<report_error_> report_error;

}

using namespace GG;

void report_error_::default_send_error_string(const std::string& str)
{ std::cerr << str; }

void report_error_::generate_error_string(const token_type& first,
                                          const token_type& it,
                                          const boost::spirit::info& rule_name,
                                          bool at_end,
                                          std::string& str) const
{
    std::stringstream is;

    is << s_filename << ":" << (boost::spirit::get_line(it.matched_.first.position()) + 1) << ": "
       << "Parse error: expected " << rule_name;

    if (at_end) {
        is << " before end of input.\n";
    } else {
        std::string match(it.matched_.first, it.matched_.second);
        text_iterator it_start = it.matched_.first;

        // Use the end of the token's matched range, if its entire match was
        // whitespace.
        if (match.find_first_not_of(" \t\n\r\f\v") == std::string::npos)
            it_start = it.matched_.second;

        text_iterator line_start = boost::spirit::get_line_start(s_begin, it.matched_.first);
        if (it_start.base() < line_start.base())
            it_start = line_start;

        std::string line_start_though_it_start(line_start, it_start);
        boost::algorithm::replace_all(line_start_though_it_start, "\t", "    ");

        std::string position_indicator(line_start_though_it_start.size(), '~');

        is << " here:\n"
           << "  " << line_start_though_it_start << match << "\n"
           << "  " << position_indicator << '^'
           << std::endl;
    }

    str = is.str();
}

boost::function<void (const std::string&)> report_error_::send_error_string =
    &report_error_::default_send_error_string;

const char* report_error_::s_filename = 0;

text_iterator report_error_::s_begin;
