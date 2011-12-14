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

#ifndef DISPLAY_TEXT_HPP
#define DISPLAY_TEXT_HPP

#include <GG/Clr.h>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/noncopyable.hpp>

#include <string>


namespace GG {
    class TextControl;
}

namespace adobe {

    struct display_text_t : boost::noncopyable
    {
        typedef any_regular_t model_type;

        display_text_t(const std::string& name,
                       const std::string& alt_text,
                       int characters,
                       GG::Clr color);

        GG::TextControl* window_m;
        std::string name_m;
        std::string alt_text_m;
        int characters_m;
        GG::Clr color_m;

        void measure(extents_t& result);
        void measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal);
        void place(const place_data_t& place_data);
        void display(const model_type& value);
    };

    inline GG::TextControl* get_display(display_text_t& widget)
    { return widget.window_m; }

}

#endif
