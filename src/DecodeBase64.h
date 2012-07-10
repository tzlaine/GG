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
   whatwasthataddress@gmail.com
*/

#ifndef _DecodeBase64_h_
#define _DecodeBase64_h_


namespace detail {

    inline void DecodeBase64(std::vector<unsigned char>& data,
                             const char* str,
                             std::size_t len)
    {
        static std::vector<unsigned int> table(256, 0);

        if (!table['A']) {
            for (unsigned char c = 'A'; c <= 'Z'; ++c) {
                table[c] = c - 'A';
            }
            for (unsigned char c = 'a'; c <= 'z'; ++c) {
                table[c] = 26 + c - 'a';
            }
            for (unsigned char c = '0'; c <= '9'; ++c) {
                table[c] = 52 + c - '0';
            }
            table['+'] = 62;
            table['/'] = 63;
        }

        std::size_t groups = len / 4;
        data.resize(groups * 3);

        std::size_t data_posn = 0;
        std::size_t str_posn = 0;
        for (std::size_t i = 0; i < groups - 1; ++i) {
            boost::uint32_t group_value =
                  (table[str[str_posn + 0]] << 18)
                | (table[str[str_posn + 1]] << 12)
                | (table[str[str_posn + 2]] << 6)
                | (table[str[str_posn + 3]] << 0);
            data[data_posn + 0] = group_value << 8 >> 24;
            data[data_posn + 1] = group_value << 16 >> 24;
            data[data_posn + 2] = group_value << 24 >> 24;
            data_posn += 3;
            str_posn += 4;
        }
        boost::uint32_t group_value =
              (table[str[str_posn + 0]] << 18)
            | (table[str[str_posn + 1]] << 12)
            | (table[str[str_posn + 2]] << 6)
            | (table[str[str_posn + 3]] << 0);
        if (data.size() - data_posn == 3) {
            data[data_posn + 0] = group_value << 8 >> 24;
            data[data_posn + 1] = group_value << 16 >> 24;
            data[data_posn + 2] = group_value << 24 >> 24;
        } else if (data.size() - data_posn == 2) {
            data[data_posn + 0] = group_value << 8 >> 24;
            data[data_posn + 1] = group_value << 16 >> 24;
        } else if (data.size() - data_posn == 1) {
            data[data_posn + 0] = group_value << 8 >> 24;
        }
    }

}

#endif
