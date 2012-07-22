/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// tab_group.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_tab_group.hpp>

#include <GG/ClrConstants.h>
#include <GG/adobe/future/widgets/headers/tab_group_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/localization.hpp>

/*************************************************************************************************/

/*************************************************************************************************/

namespace {

/****************************************************************************************************/

GG::TabBarStyle name_to_style(adobe::name_t style_name)
{
    if (style_name == adobe::static_name_t("attached"))
        return GG::TAB_BAR_ATTACHED;
    else if (style_name == adobe::static_name_t("detached"))
        return GG::TAB_BAR_DETACHED;
    else
        throw std::runtime_error("Unknown tab bar style.");
}

/****************************************************************************************************/

void handle_tab_changed_signal(adobe::signal_notifier_t signal_notifier,
                               adobe::name_t widget_id,
                               adobe::sheet_t& sheet,
                               adobe::name_t bind,
                               adobe::array_t expression,
                               const adobe::any_regular_t& value)
{
    adobe::implementation::handle_signal(signal_notifier,
                                         adobe::static_name_t("tab_group"),
                                         adobe::static_name_t("tab_changed"),
                                         widget_id,
                                         sheet,
                                         bind,
                                         expression,
                                         value);
}

/****************************************************************************************************/

}

/*************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         size,
                   tab_group_t*&       widget)
{
    array_t items;
    GG::Clr color(GG::CLR_GRAY);
    GG::Clr text_color(GG::CLR_BLACK);
    name_t  style_name("attached");
    name_t  signal_id;

    get_value(parameters, key_items, items);
    implementation::get_color(parameters, static_name_t("color"), color);
    implementation::get_color(parameters, static_name_t("text_color"), text_color);
    get_value(parameters, static_name_t("style"), style_name);
    get_value(parameters, static_name_t("signal_id"), signal_id);

    std::vector<tab_group_t::tab_t>  tabs;
    array_t::const_iterator          first(items.begin());
    array_t::const_iterator          last(items.end());

    for (; first != last; ++first)
    {
        tab_group_t::tab_t new_tab;

        new_tab.name_m = localization_invoke(get_value((*first).cast<dictionary_t>(), key_name).cast<std::string>());
        new_tab.value_m = get_value((*first).cast<dictionary_t>(), key_value);

        tabs.push_back(new_tab);
    }

    tab_group_t::tab_t* first_tab(tabs.empty() ? 0 : &tabs[0]);

    widget = new tab_group_t(first_tab,
                             first_tab + tabs.size(),
                             color,
                             text_color,
                             name_to_style(style_name),
                             signal_id);
}

/****************************************************************************************************/

template <>
void attach_view_and_controller(tab_group_t&           control,
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

    any_regular_t tab_changed_binding;
    name_t cell;
    array_t expression;
    if (get_value(parameters, static_name_t("bind_tab_changed_signal"), tab_changed_binding))
        implementation::cell_and_expression(tab_changed_binding, cell, expression);
    control.tab_changed_proc_m =
        boost::bind(&handle_tab_changed_signal,
                    token.signal_notifier_m, control.signal_id_m,
                    boost::ref(token.sheet_m), cell, expression, _1);

#define BIND_COLOR(name)                                                \
    adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
    BIND_COLOR(color);
    BIND_COLOR(text_color);
#undef BIND_COLOR
}

/****************************************************************************************************/

widget_node_t make_tab_group(const dictionary_t&     parameters,
                             const widget_node_t&    parent,
                             const factory_token_t&  token,
                             const widget_factory_t& factory)
{ 
    return create_and_hookup_widget<tab_group_t, poly_placeable_t>(
        parameters, parent, token, 
        factory.is_container(static_name_t("tab_group")), 
        factory.layout_attributes(static_name_t("tab_group")));
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
