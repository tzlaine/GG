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

#include "platform_display_text.hpp"

#include "display_text_factory.hpp"

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/EveGlue.h>


namespace {

    bool register_()
    {
        GG::RegisterView(adobe::static_name_t("display_text"), &adobe::implementation::make_display_text);
        return true;
    }

    bool dummy = register_();

}

namespace adobe {

    void create_widget(const dictionary_t& parameters,
                       size_enum_t,
                       display_text_t*& display_text)
    {
        std::string name;
        std::string alt_text;
        long characters(5);

        get_value(parameters, key_name, name);
        get_value(parameters, key_alt_text, alt_text);
        get_value(parameters, key_characters, characters);

        display_text = new display_text_t(name, alt_text, characters);
    }

    template <typename Sheet, typename FactoryToken>
    inline void couple_controller_to_cell(display_text_t&,
                                          name_t,
                                          Sheet&,
                                          const FactoryToken&,
                                          const dictionary_t&)

    {}

    namespace implementation {

        widget_node_t make_display_text(const dictionary_t& parameters,
                                        const widget_node_t& parent,
                                        const factory_token_t& token,
                                        const widget_factory_t& factory)
        {
            return create_and_hookup_widget<display_text_t, poly_placeable_twopass_t>(
                parameters, parent, token,
                factory.is_container(static_name_t("display_text")),
                factory.layout_attributes(static_name_t("display_text")));
        }

    }

}
