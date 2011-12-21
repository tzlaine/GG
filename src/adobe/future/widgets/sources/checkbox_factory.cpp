/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// checkbox.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_checkbox.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/checkbox_factory.hpp>
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
                           bool checked)
{
    adobe::implementation::handle_signal(signal_notifier,
                                         adobe::static_name_t("checkbox"),
                                         adobe::static_name_t("checked"),
                                         widget_id,
                                         sheet,
                                         bind,
                                         expression,
                                         adobe::any_regular_t(checked));
}

/****************************************************************************************************/

}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   checkbox_t*&        checkbox)
{
    std::string          name;
    std::string          alt_text;
    any_regular_t        true_value(true);
    any_regular_t        false_value(false);
    GG::Clr              color(GG::CLR_GRAY);
    GG::Clr              text_color(GG::CLR_BLACK);
    GG::Clr              interior_color(GG::CLR_ZERO);
    name_t               style_name("SBSTYLE_3D_XBOX");
    name_t               signal_id;

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_value_on, true_value);
    get_value(parameters, key_value_off, false_value);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("text_color"), text_color);
    implementation::get_color(parameters, static_name_t("interior_color"), interior_color);
    get_value(parameters, static_name_t("style"), style_name);
    get_value(parameters, static_name_t("signal_id"), signal_id);

    checkbox = new checkbox_t(name,
                              true_value,
                              false_value,
                              color,
                              text_color,
                              interior_color,
                              implementation::name_to_style(style_name),
                              alt_text,
                              signal_id);
}

/****************************************************************************************************/

template <typename Sheet>
void couple_controller_to_cell(checkbox_t&               controller, 
                               name_t                    cell, 
                               Sheet&                    sheet, 
                               eve_client_holder& client_holder, 
                               const dictionary_t&       parameters)

{
    attach_enabler(client_holder.assemblage_m, cell, controller, sheet, parameters);
    attach_monitor(controller, cell, sheet, client_holder, parameters);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(checkbox_t&            control,
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
    control.checked_proc_m = boost::bind(&handle_checked_signal,
                                         token.signal_notifier_m,
                                         control.signal_id_m,
                                         boost::ref(token.sheet_m),
                                         cell,
                                         expression,
                                         _1);
}

/****************************************************************************************************/

widget_node_t make_checkbox(const dictionary_t&     parameters, 
                            const widget_node_t&    parent, 
                            const factory_token_t&  token,
                            const widget_factory_t& factory)
    { return create_and_hookup_widget<checkbox_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("checkbox")), 
        factory.layout_attributes(static_name_t("checkbox"))); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
