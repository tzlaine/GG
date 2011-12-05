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

#include "platform_listbox.hpp"

#include "listbox_factory.hpp"

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/EveGlue.h>


namespace {

    bool register_()
    {
        GG::RegisterView(adobe::static_name_t("listbox"), &adobe::implementation::make_listbox);
        return true;
    }

    bool dummy = register_();

}

namespace adobe {

    void create_widget(const dictionary_t& parameters,
                       size_enum_t,
                       listbox_t*& listbox)
    {
        std::string name;
        std::string alt_text;
        long characters(50);
        long rows(0);
        long width(0);
        long height(0);
        array_t items;
        listbox_t::item_set_t item_set;

        get_value(parameters, key_name, name);
        get_value(parameters, key_alt_text, alt_text);
        get_value(parameters, key_characters, characters);
        get_value(parameters, static_name_t("rows"), rows);
        get_value(parameters, static_name_t("width"), width);
        get_value(parameters, static_name_t("height"), height);
        get_value(parameters, key_items, items);

        for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first)
        {
            dictionary_t cur_item(first->cast<dictionary_t>());
            item_set.push_back(
                listbox_t::item_set_t::value_type(
                    get_value(cur_item, key_name).cast<std::string>(), get_value(cur_item, key_value)
                )
            );
        }

        if (!rows && items.empty())
            rows = 8;

        listbox = new listbox_t(name, alt_text, characters, rows, width, height, item_set);
    }

    namespace implementation {

        widget_node_t make_listbox(const dictionary_t& parameters,
                                   const widget_node_t& parent,
                                   const factory_token_t& token,
                                   const widget_factory_t& factory)
        {
            return create_and_hookup_widget<listbox_t, poly_placeable_t>(
                parameters, parent, token,
                factory.is_container(static_name_t("listbox")),
                factory.layout_attributes(static_name_t("listbox")));
        }

    }

}
