/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// reveal.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_reveal.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/reveal_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t&  parameters, 
                   size_enum_t          size,
                   reveal_t*&           widget)
{
    std::string             name;
    std::string             alt_text;
    any_regular_t           show_value(true);
    GG::Clr                 text_color(GG::CLR_BLACK);
    dictionary_t            show_image;
    dictionary_t            hide_image;

    get_value(parameters, key_value_on, show_value);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_name, name);
    implementation::get_color(parameters, static_name_t("text_color"), text_color);

    get_value(parameters, static_name_t("showing_image"), show_image);
    GG::SubTexture show_unpressed;
    GG::SubTexture show_pressed;
    GG::SubTexture show_rollover;
    implementation::get_subtexture(show_image, static_name_t("unpressed"), show_unpressed);
    implementation::get_subtexture(show_image, static_name_t("pressed"), show_pressed);
    implementation::get_subtexture(show_image, static_name_t("rollover"), show_rollover);

    get_value(parameters, static_name_t("hiding_image"), hide_image);
    GG::SubTexture hide_unpressed;
    GG::SubTexture hide_pressed;
    GG::SubTexture hide_rollover;
    implementation::get_subtexture(hide_image, static_name_t("unpressed"), hide_unpressed);
    implementation::get_subtexture(hide_image, static_name_t("pressed"), hide_pressed);
    implementation::get_subtexture(hide_image, static_name_t("rollover"), hide_rollover);

    widget = new reveal_t(name, show_value, alt_text, text_color,
                          show_unpressed, show_pressed, show_rollover,
                          hide_unpressed, hide_pressed, hide_rollover);
}

/****************************************************************************************************/

template <typename Sheet>
void couple_controller_to_cell(reveal_t&                 controller,
                               name_t                    cell, 
                               Sheet&                    sheet, 
                               const factory_token_t& token, 
                               const dictionary_t&       parameters)
{
    attach_monitor(controller, cell, sheet, token, parameters);
}
    
/****************************************************************************************************/

widget_node_t make_reveal(const dictionary_t&     parameters,
                          const widget_node_t&    parent,
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<reveal_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("reveal")), 
        factory.layout_attributes(static_name_t("reveal"))); 
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
