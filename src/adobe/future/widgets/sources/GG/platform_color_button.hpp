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

#ifndef COLORBUTTON_HPP
#define COLORBUTTON_HPP

#include <GG/adobe/future/widgets/headers/platform_label.hpp>

#include <boost/noncopyable.hpp>


namespace adobe {

    struct color_button_t : boost::noncopyable
    {
        typedef GG::Clr model_type;
        typedef boost::function<void (model_type)> setter_type;

        color_button_t(const std::string& name,
                       const std::string& alt_text,
                       unsigned int width,
                       unsigned int height,
                       name_t signal_id,
                       GG::Clr border_color,
                       GG::Clr label_color,
                       GG::Clr dialog_color,
                       GG::Clr dialog_border_color,
                       GG::Clr dialog_text_color);

        void measure(extents_t& result);
        void place(const place_data_t& place_data);
        void display(model_type value);
        void monitor(const setter_type& proc);
        void enable(bool make_enabled);

        GG::Control* control_m;
        unsigned int width_m;
        unsigned int height_m;
        label_t name_m;
        std::string alt_text_m;
        bool using_label_m;
        setter_type value_proc_m;
        name_t signal_id_m;
        GG::Clr border_color_m;
        GG::Clr dialog_color_m;
        GG::Clr dialog_border_color_m;
        GG::Clr dialog_text_color_m;

        implementation::color_proxy_t border_color_proxy_m;
    };

}

#endif
