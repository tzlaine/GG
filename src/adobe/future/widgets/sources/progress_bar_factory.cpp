/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// tab_group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_progress_bar.hpp>

#include <GG/adobe/future/widgets/headers/progress_bar_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   progress_bar_t*&    widget)
{
    name_t orientation(key_horizontal);
    int length(100);
    int width(14);
    GG::Clr color(GG::CLR_BLACK);
    GG::Clr bar_color(GG::CLR_SHADOW);
    GG::Clr interior_color(GG::CLR_ZERO);

    get_value(parameters, key_orientation, orientation);
    get_value(parameters, static_name_t("length"), length);
    get_value(parameters, static_name_t("width"), width);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("bar_color"), bar_color);
    implementation::get_color(parameters, static_name_t("interior_color"), interior_color);

    widget = new progress_bar_t(orientation == key_vertical,
                                length,
                                width,
                                color,
                                bar_color,
                                interior_color);
}

/*************************************************************************************************/

template <typename Sheet, typename FactoryToken>
inline void couple_controller_to_cell(progress_bar_t&,
                                      name_t,
                                      Sheet&,
                                      const FactoryToken&,
                                      const dictionary_t&)
{
    // no adam interaction
}

/****************************************************************************************************/

widget_node_t make_progress_bar(const dictionary_t&     parameters,
                                const widget_node_t&    parent,
                                const factory_token_t&  token,
                                const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<progress_bar_t, poly_placeable_t>(
        parameters, parent, token, 
        factory.is_container(static_name_t("progress_bar")), 
        factory.layout_attributes(static_name_t("progress_bar"))
    );
}

/*************************************************************************************************/

} // namespace adobe
