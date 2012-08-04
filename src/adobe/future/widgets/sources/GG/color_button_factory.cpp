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

#include "platform_color_button.hpp"

#include "color_button_factory.hpp"

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/EveGlue.h>


namespace adobe {

    bool register_()
    {
        GG::RegisterView(adobe::static_name_t("color_button"), &adobe::implementation::make_color_button);
        return true;
    }

    bool dummy = register_();

}

namespace adobe {

    void create_widget(const dictionary_t& parameters,
                       size_enum_t,
                       color_button_t*& color_button)
    {
        std::string name;
        std::string alt_text;
        unsigned int width(Value(adobe::implementation::StandardHeight() * 3));
        unsigned int height(Value(adobe::implementation::StandardHeight()));
        name_t signal_id;
        GG::Clr border_color(GG::CLR_WHITE);
        GG::Clr label_color(GG::CLR_BLACK);
        GG::Clr dialog_color(GG::CLR_GRAY);
        GG::Clr dialog_border_color(GG::CLR_GRAY);
        GG::Clr dialog_text_color(GG::CLR_BLACK);

        get_value(parameters, key_name, name);
        get_value(parameters, key_alt_text, alt_text);
        get_value(parameters, static_name_t("width"), width);
        get_value(parameters, static_name_t("height"), height);
        get_value(parameters, static_name_t("signal_id"), signal_id);
        get_value(parameters, static_name_t("border_color"), border_color);
        get_value(parameters, static_name_t("label_color"), label_color);
        get_value(parameters, static_name_t("dialog_color"), dialog_color);
        get_value(parameters, static_name_t("dialog_border_color"), dialog_border_color);
        get_value(parameters, static_name_t("dialog_text_color"), dialog_text_color);

        color_button = new color_button_t(name,
                                          alt_text,
                                          width,
                                          height,
                                          signal_id,
                                          border_color,
                                          label_color,
                                          dialog_color,
                                          dialog_border_color,
                                          dialog_text_color);
    }

    template <>
    void attach_view_and_controller(color_button_t&        control,
                                    const dictionary_t&    parameters,
                                    const factory_token_t& token,
                                    adobe::name_t,
                                    adobe::name_t,
                                    adobe::name_t)
    {
        basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);

        if (parameters.count(key_bind) != 0) {
            name_t cell(get_value(parameters, key_bind).cast<name_t>());

            attach_view_direct(control, parameters, token, cell);

            // is the cell in the layout sheet or the Adam sheet?
            if (layout_sheet.count_interface(cell) != 0)
                couple_controller_to_cell(control, cell, layout_sheet, token, parameters);
            else
                couple_controller_to_cell(control, cell, token.sheet_m, token, parameters);
        }

#define BIND_COLOR(name)                                                \
        adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
        BIND_COLOR(border_color);
#undef BIND_COLOR
    }

    namespace implementation {

        widget_node_t make_color_button(const dictionary_t& parameters,
                                        const widget_node_t& parent,
                                        const factory_token_t& token,
                                        const widget_factory_t& factory)
        {
            return create_and_hookup_widget<color_button_t, poly_placeable_t>(
                parameters, parent, token,
                factory.is_container(static_name_t("color_button")),
                factory.layout_attributes(static_name_t("color_button"))
            );
        }

    }

}
