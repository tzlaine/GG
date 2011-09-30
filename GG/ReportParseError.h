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


namespace GG {

    struct GG_API report_error_
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
        static void default_send_error_string(const std::string& str);
        static const char* s_filename;
        static text_iterator s_begin;
        static text_iterator s_end;

    private:
        void generate_error_string(const token_type& first,
                                   const token_type& it,
                                   const boost::spirit::info& rule_name,
                                   bool at_end,
                                   std::string& str) const;
    };

    extern GG_API const boost::phoenix::function<report_error_> report_error;

}

#endif // _GG_ReportParseError_h_
