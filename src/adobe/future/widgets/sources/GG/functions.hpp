/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2011 T. Zachary Laine

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

#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <GG/adobe/dictionary_fwd.hpp>


namespace adobe {

    namespace implementation {

        any_regular_t color_dialog(const dictionary_t& parameters);
        any_regular_t file_dialog(const dictionary_t& parameters);
        any_regular_t three_button_dialog(const dictionary_t& parameters);
        any_regular_t append(const array_t& parameters);
        any_regular_t prepend(const array_t& parameters);
        any_regular_t insert(const array_t& parameters);
        any_regular_t erase(const array_t& parameters);
        any_regular_t parse(const array_t& parameters);
        any_regular_t size(const array_t& parameters);
        any_regular_t join(const array_t& parameters);
        any_regular_t split(const array_t& parameters);
        any_regular_t to_string(const array_t& parameters);
        any_regular_t to_name(const array_t& parameters);
        any_regular_t print(const array_t& parameters);
        any_regular_t assert_(const array_t& parameters);

    }

}

#endif
