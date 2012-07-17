/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// popup.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_popup.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/popup_common.hpp>
#include <GG/adobe/future/widgets/headers/popup_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

/*************************************************************************************************/

namespace {

/****************************************************************************************************/

void handle_selection_changed_signal(adobe::signal_notifier_t signal_notifier,
                                     adobe::name_t widget_id,
                                     adobe::sheet_t& sheet,
                                     adobe::name_t bind,
                                     adobe::array_t expression,
                                     const adobe::any_regular_t& value)
{
    adobe::implementation::handle_signal(signal_notifier,
                                         adobe::static_name_t("popup"),
                                         adobe::static_name_t("selection_changed"),
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
                   popup_t*&           widget)
{
    std::string              name;
    std::string              alt_text;
    int                      max_characters(25);
    std::string              custom_item_name("Custom");
    array_t                  items;
    popup_t::menu_item_set_t item_set;
    name_t                   signal_id;
    GG::Clr                  color(GG::CLR_GRAY);
    GG::Clr                  label_color(GG::CLR_BLACK);
    GG::Clr                  item_text_color(GG::CLR_BLACK);

    get_value(parameters, key_name, name);
    get_value(parameters, key_alt_text, alt_text);
    get_value(parameters, key_max_characters, max_characters);
    get_value(parameters, key_items, items);
    get_value(parameters, key_custom_item_name, custom_item_name);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("label_color"), label_color);
    implementation::get_color(parameters, static_name_t("item_text_color"), item_text_color);
    get_value(parameters, static_name_t("signal_id"), signal_id);

    for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first) {
        item_set.push_back(first->cast<dictionary_t>());
        get_value(item_set.back(), key_value);
    }

    // REVISIT (fbrereto) : Should be called 'first' but for the fact that MSVC 7.1 complains.
    popup_t::menu_item_set_t::value_type* first_value(item_set.empty() ? 0 : &item_set[0]);

    widget = new popup_t(name,
                         alt_text,
                         max_characters,
                         custom_item_name,
                         first_value,
                         first_value + item_set.size(),
                         color,
                         label_color,
                         item_text_color,
                         signal_id);
}

/*************************************************************************************************/

template <>
void attach_view_and_controller(popup_t&               control,
                                const dictionary_t&    parameters,
                                const factory_token_t& token,
                                adobe::name_t,
                                adobe::name_t,
                                adobe::name_t)
{
    basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);
    assemblage_t&  assemblage(token.client_holder_m.assemblage_m);

    if (parameters.count(key_bind) != 0) {
        name_t cell(get_value(parameters, key_bind).cast<name_t>());
        attach_view_and_controller_direct(control, parameters, token, cell);
    }

    if (parameters.count(key_items) && 
        get_value(parameters, key_items).type_info() == type_info<name_t>()) {
        // dynamically bind to the cell instead of getting a static list of popup items

        name_t cell(get_value(parameters, key_items).cast<name_t>());

        if (layout_sheet.count_interface(cell))
            attach_popup_menu_item_set(control, cell, layout_sheet, assemblage, token.client_holder_m);
        else
            attach_popup_menu_item_set(control, cell, token.sheet_m, assemblage, token.client_holder_m);
    }

    any_regular_t selection_changed_binding;
    name_t cell;
    array_t expression;
    if (get_value(parameters, static_name_t("bind_selection_changed_signal"), selection_changed_binding))
        implementation::cell_and_expression(selection_changed_binding, cell, expression);
    control.selection_changed_proc_m =
        boost::bind(&handle_selection_changed_signal,
                    token.signal_notifier_m, control.signal_id_m,
                    boost::ref(token.sheet_m), cell, expression, _1);

    control.row_factory_m = token.row_factory_m;

#define BIND_COLOR(name)                                                \
    adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
    BIND_COLOR(color);
    BIND_COLOR(item_text_color);
#undef BIND_COLOR
}

/****************************************************************************************************/

widget_node_t make_popup(const dictionary_t&     parameters,
                         const widget_node_t&    parent,
                         const factory_token_t&  token,
                         const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<popup_t, poly_placeable_t>(parameters, parent, token, 
        factory.is_container(static_name_t("popup")), 
        factory.layout_attributes(static_name_t("popup"))); 
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
