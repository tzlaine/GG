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

#ifndef COLOR_BUTTON_FACTORY_HPP
#define COLOR_BUTTON_FACTORY_HPP

#include <GG/adobe/dictionary.hpp>

namespace adobe {

    struct widget_node_t;
    struct factory_token_t;
    class widget_factory_t;

    namespace implementation {

        widget_node_t make_color_button(const dictionary_t& parameters,
                                        const widget_node_t& parent,
                                        const factory_token_t& token,
                                        const widget_factory_t& factory);

    }

}

#endif
