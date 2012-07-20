/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_group.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/group_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>


namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   group_t*&           group)
{
    std::string    name;
    std::string    alt_text;
    GG::Clr        color(GG::CLR_GRAY);
    GG::Clr        text_color(GG::CLR_BLACK);
    GG::Clr        interior_color(GG::CLR_ZERO);

    implementation::get_localized_string(parameters, key_name, name);
    implementation::get_localized_string(parameters, key_alt_text, alt_text);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("text_color"), text_color);
    implementation::get_color(parameters, static_name_t("interior_color"), interior_color);

    group = new group_t(name, alt_text, color, text_color, interior_color);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(group_t&               control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    // no adam interaction

#define BIND_COLOR(name)                                                \
    adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
    BIND_COLOR(color);
    BIND_COLOR(text_color);
    BIND_COLOR(interior_color);
#undef BIND_COLOR
}

/****************************************************************************************************/

widget_node_t make_group(const dictionary_t&     parameters, 
                         const widget_node_t&    parent, 
                         const factory_token_t&  token,
                         const widget_factory_t& factory)
    {
        return create_and_hookup_widget<group_t, poly_placeable_t>(
            parameters, parent, token,
            factory.is_container(static_name_t("group")),
            factory.layout_attributes(static_name_t("group"))
        );
    }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
