/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// radio_button.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_radio_button.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/radio_button_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/*************************************************************************************************/

namespace {

/****************************************************************************************************/

void handle_checked_signal(adobe::signal_notifier_t signal_notifier,
                           adobe::name_t widget_id,
                           adobe::sheet_t& sheet,
                           adobe::name_t bind,
                           adobe::array_t expression,
                           const adobe::any_regular_t& value)
{
    adobe::implementation::handle_signal(signal_notifier,
                                         adobe::static_name_t("radio_button"),
                                         adobe::static_name_t("checked"),
                                         widget_id,
                                         sheet,
                                         bind,
                                         expression,
                                         value);
}

/****************************************************************************************************/

}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   radio_button_t*&     widget)
{
    std::string   name;
    std::string   alt_text;
    any_regular_t set_value;
    GG::Clr       color(GG::CLR_GRAY);
    GG::Clr       text_color(GG::CLR_BLACK);
    GG::Clr       interior_color(GG::CLR_ZERO);
    name_t        style_name("radio");
    name_t        signal_id;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_value, set_value);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("text_color"), text_color);
    implementation::get_color(parameters, static_name_t("interior_color"), interior_color);
    get_value(parameters, static_name_t("style"), style_name);
    get_value(parameters, static_name_t("signal_id"), signal_id);

    widget = new radio_button_t(name,
                                alt_text,
                                set_value,
                                color,
                                text_color,
                                interior_color,
                                implementation::name_to_style(style_name),
                                signal_id);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(radio_button_t&        control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    if (parameters.count(key_bind) != 0) {
        name_t cell(get_value(parameters, key_bind).cast<name_t>());
        attach_view_and_controller_direct(control, parameters, token, cell);
    }

    any_regular_t checked_binding;
    name_t cell;
    array_t expression;
    if (get_value(parameters, static_name_t("bind_checked_signal"), checked_binding)) {
        implementation::cell_and_expression(checked_binding, cell, expression);
    }
    control.checked_proc_m =
        boost::bind(&handle_checked_signal,
                    token.signal_notifier_m,
                    control.signal_id_m,
                    boost::ref(token.sheet_m),
                    cell,
                    expression,
                    _1);

#define BIND_COLOR(name)                                                \
    adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
    BIND_COLOR(color);
    BIND_COLOR(text_color);
    BIND_COLOR(interior_color);
#undef BIND_COLOR
}

/****************************************************************************************************/

widget_node_t make_radio_button(const dictionary_t&     parameters,
                                const widget_node_t&    parent,
                                const factory_token_t&  token,
                                const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<radio_button_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("radio_button")), 
        factory.layout_attributes(static_name_t("radio_button"))); 
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
