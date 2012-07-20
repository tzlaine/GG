/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

// button.hpp needs to come before widget_factory to hook the overrides
#include <GG/adobe/future/widgets/headers/platform_button.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/button_factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/window_server.hpp>

#include <GG/Button.h>
#include <GG/ClrConstants.h>
#include <GG/EveGlue.h>

#include "GG/functions.hpp"


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

struct button_item_t
{
    button_item_t() : modifier_set_m(adobe::modifiers_none_s)
    { }

    button_item_t(const button_item_t& default_item, const adobe::dictionary_t& parameters) :
        name_m(default_item.name_m),
        alt_text_m(default_item.alt_text_m),
        bind_m(default_item.bind_m),
        bind_output_m(default_item.bind_output_m),
        action_m(default_item.action_m),
        bind_signal_m(default_item.bind_signal_m),
        expression_m(default_item.expression_m),
        value_m(default_item.value_m),
        contributing_m(default_item.contributing_m),
        modifier_set_m(default_item.modifier_set_m),
        signal_id_m(default_item.signal_id_m)
    {
        adobe::implementation::get_localized_string(parameters, adobe::key_name, name_m);
        adobe::implementation::get_localized_string(parameters, adobe::key_alt_text, alt_text_m);
        get_value(parameters, adobe::key_bind, bind_m);
        get_value(parameters, adobe::key_bind_output, bind_output_m);
        get_value(parameters, adobe::key_action, action_m);
        get_value(parameters, adobe::key_value, value_m);
        get_value(parameters, adobe::static_name_t("signal_id"), signal_id_m);

        // modifers can be a name or array

        /*
            REVISIT (sparent) : Old way was with a single modifers key word with multiple
            modifers encoded in a single name - new way with a name or array under the
            new keyword modifier_set.
        */
        adobe::dictionary_t::const_iterator iter(parameters.find(adobe::key_modifiers));

        if (iter == parameters.end())
            iter = parameters.find(adobe::key_modifier_set);

        if (iter != parameters.end())
            modifier_set_m |= adobe::value_to_modifier(iter->second);

        adobe::any_regular_t clicked_binding;
        if (get_value(parameters, adobe::static_name_t("bind_clicked_signal"), clicked_binding)) {
            adobe::implementation::cell_and_expression(clicked_binding, bind_signal_m, expression_m);
        }

        if (!action_m)
            action_m = signal_id_m;
        if (!signal_id_m)
            signal_id_m = action_m;
    }

    std::string          name_m;
    std::string          alt_text_m;
    adobe::name_t        bind_m;
    adobe::name_t        bind_output_m;
    adobe::name_t        action_m;
    adobe::name_t        bind_signal_m;
    adobe::array_t       expression_m;
    adobe::any_regular_t value_m;
    adobe::dictionary_t  contributing_m;
    adobe::modifiers_t   modifier_set_m;
    adobe::name_t        signal_id_m;
};

/*************************************************************************************************/

void proxy_button_hit(const adobe::factory_token_t&  token,
                      adobe::name_t                  bind,
                      adobe::name_t                  bind_output,
                      adobe::name_t                  action,
                      const adobe::any_regular_t&    value,
                      const adobe::dictionary_t&     contributing,
                      const adobe::widget_factory_t& factory)
{
    adobe::sheet_t& sheet = token.sheet_m;
    adobe::behavior_t& behavior = token.client_holder_m.root_behavior_m;
    adobe::vm_lookup_t& vm_lookup = token.vm_lookup_m;
    const adobe::button_notifier_t& button_notifier = token.button_notifier_m;
    const adobe::button_notifier_t& top_level_button_notifier =
        token.top_level_button_notifier_m;
    const adobe::signal_notifier_t& signal_notifier = token.signal_notifier_m;
    const adobe::row_factory_t* row_factory = token.row_factory_m;

    if (bind_output) {
        //touch(); // REVISIT (sparent) : We should have per item touch!
        sheet.set(bind_output, value);
        sheet.update();
    } else if (button_notifier) {
        if (bind) {
            adobe::dictionary_t result;
            result.insert(std::make_pair(adobe::key_value, value));
            result.insert(std::make_pair(adobe::key_contributing, adobe::any_regular_t(contributing)));
            button_notifier(action, adobe::any_regular_t(result));
        } else {
            if (action == adobe::static_name_t("dialog")) {
                std::string eve_script;
                if (value.cast<std::string>(eve_script)) {
                    adobe::window_server_t window_server(sheet,
                                                         behavior,
                                                         vm_lookup,
                                                         top_level_button_notifier,
                                                         signal_notifier,
                                                         row_factory,
                                                         factory);
                    window_server.run(eve_script.c_str());
                } else {
                    std::string adam_script;
                    std::string eve_script;
                    adobe::name_t name;
                    adobe::name_t bind_result;
                    adobe::dictionary_t dialog_parameters;
                    const adobe::dictionary_t& parameters =
                        value.cast<adobe::dictionary_t>();
                    get_value(parameters, adobe::static_name_t("adam_script"), adam_script);
                    get_value(parameters, adobe::static_name_t("eve_script"), eve_script);
                    get_value(parameters, adobe::static_name_t("name"), name);
                    get_value(parameters, adobe::static_name_t("bind_result"), bind_result);
                    get_value(parameters, adobe::static_name_t("dialog_parameters"), dialog_parameters);

                    if (eve_script.empty() == !name)
                        throw std::runtime_error("Exactly one of eve_script and name must be defined");
                    if (!name != !bind_result)
                        throw std::runtime_error("Both or neither of name and bind_result must be defined");

                    if (name) {
                        adobe::any_regular_t result;
                        if (name == adobe::static_name_t("color_dialog")) {
                            result = adobe::implementation::color_dialog(dialog_parameters);
                        } else if (name == adobe::static_name_t("file_dialog")) {
                            result = adobe::implementation::file_dialog(dialog_parameters);
                        } else if (name == adobe::static_name_t("three_button_dialog")) {
                            result = adobe::implementation::three_button_dialog(dialog_parameters);
                        } else {
                            throw std::runtime_error("Unknown builtin dialog type specified");
                        }
                        sheet.set(bind_result, result);
                        sheet.update();
                    } else if (!adam_script.empty() && !eve_script.empty()) {
                        GG::DictionaryFunctions df;
                        for (adobe::vm_lookup_t::dictionary_function_map_t::const_iterator
                                 it = vm_lookup.dictionary_functions().begin();
                             it != vm_lookup.dictionary_functions().end();
                             ++it) {
                            df[it->first] = it->second;
                        }
                        GG::ArrayFunctions af;
                        for (adobe::vm_lookup_t::array_function_map_t::const_iterator
                                 it = vm_lookup.array_functions().begin();
                             it != vm_lookup.array_functions().end();
                             ++it) {
                            af[it->first] = it->second;
                        }
                        const GG::ModalDialogResult& result =
                            GG::ExecuteModalDialog(boost::filesystem::path(eve_script),
                                                   boost::filesystem::path(adam_script),
                                                   df,
                                                   af,
                                                   top_level_button_notifier,
                                                   signal_notifier,
                                                   row_factory ? *row_factory : GG::RowFactory());
                        if (bind_result && result.m_terminating_action == adobe::static_name_t("ok")) {
                            sheet.set(bind_result, adobe::any_regular_t(result.m_result));
                            sheet.update();
                        }
                    }
                }
            } else {
                button_notifier(action, value);
            }
        }
    }
}

/****************************************************************************************************/

void handle_clicked_signal(adobe::signal_notifier_t signal_notifier,
                           adobe::name_t widget_id,
                           adobe::sheet_t& sheet,
                           adobe::name_t bind,
                           adobe::array_t expression,
                           const adobe::any_regular_t& value)
{
    adobe::implementation::handle_signal(signal_notifier,
                                         adobe::static_name_t("button"),
                                         adobe::static_name_t("clicked"),
                                         widget_id,
                                         sheet,
                                         bind,
                                         expression,
                                         value);
}

/*************************************************************************************************/

void state_set_push_back(adobe::button_t& button,
                         const adobe::factory_token_t& token,
                         const button_item_t& temp,
                         const adobe::widget_factory_t& factory)
{
    button.state_set_m.push_back(adobe::button_state_descriptor_t());

    button.state_set_m.back().name_m         = temp.name_m;
    button.state_set_m.back().alt_text_m     = temp.alt_text_m;
    button.state_set_m.back().modifier_set_m = temp.modifier_set_m;
    button.state_set_m.back().hit_proc_m     =
        boost::bind(&proxy_button_hit,
                    token,
                    temp.bind_m,
                    temp.bind_output_m,
                    temp.action_m,
                    _1,
                    _2,
                    boost::cref(factory));

    button.state_set_m.back().clicked_proc_m =
        boost::bind(&handle_clicked_signal,
                    token.signal_notifier_m,
                    temp.signal_id_m,
                    boost::ref(token.sheet_m),
                    temp.bind_signal_m,
                    temp.expression_m,
                    _1);

    button.state_set_m.back().value_m        = temp.value_m;
    button.state_set_m.back().contributing_m = temp.contributing_m;
}

/****************************************************************************************************/

void connect_button_state(adobe::button_t&          control,
                          adobe::assemblage_t&      assemblage,
                          adobe::sheet_t&           sheet,
                          const button_item_t&      temp,
                          adobe::eve_client_holder& /*client_holder*/)
{
    if (!temp.bind_m)
        return;

    /*
        REVISIT (sparent) : Don't currently propogate modifier mask past this point.
        Not yet wired up.
    */

    adobe::display_compositor_t<adobe::button_t, adobe::modifiers_t>*
        current_display(adobe::make_display_compositor(control, temp.modifier_set_m));

    assemblage.cleanup(boost::bind(adobe::delete_ptr<adobe::display_compositor_t<adobe::button_t, adobe::modifiers_t>*>(), current_display));
    assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, 
                                    sheet.monitor_invariant_dependent(
                                     temp.bind_m, boost::bind(&adobe::button_t::enable, 
                                                               boost::ref(control), _1))));

    adobe::attach_view(assemblage, temp.bind_m, *current_display, sheet);
    
    
    /*
        REVISIT (sparent) : Filtering the contributing code here isn't necessarily the right thing -
        I think monitor_contributing should remove the filter functionality if possible. For
        now I'm passing in an empty dictionary for no filtering.
    */

    assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, 
                                    sheet.monitor_contributing(temp.bind_m, adobe::dictionary_t(), 
                                                               boost::bind(&adobe::button_t::set_contributing, 
                                                                   boost::ref(control), 
                                                                   temp.modifier_set_m, _1))));
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

button_t* create_button_widget(const dictionary_t&     parameters,
                               const factory_token_t&  token,
                               size_enum_t             size,
                               const widget_factory_t& factory)
{
    bool               is_cancel(false);
    bool               is_default(false);
    modifiers_t        modifier_mask(modifiers_none_s);
    array_t            items;
    button_item_t      item;
    GG::Clr            color(GG::CLR_GRAY);
    GG::Clr            text_color(GG::CLR_BLACK);
    dictionary_t       image;

    implementation::get_localized_string(parameters, key_name,        item.name_m);
    implementation::get_localized_string(parameters, key_alt_text,    item.alt_text_m);
    get_value(parameters, key_bind,        item.bind_m);
    get_value(parameters, key_bind_output, item.bind_output_m);
    get_value(parameters, key_action,      item.action_m);
    get_value(parameters, key_value,       item.value_m);
    get_value(parameters, adobe::static_name_t("signal_id"), item.signal_id_m);
    get_value(parameters, key_items,       items);
    get_value(parameters, key_default,     is_default);
    get_value(parameters, key_cancel,      is_cancel);
    get_color(parameters, static_name_t("color"), color);
    get_color(parameters, static_name_t("text_color"), text_color);
    get_value(parameters, key_image,       image);

    GG::SubTexture unpressed;
    GG::SubTexture pressed;
    GG::SubTexture rollover;

    get_subtexture(image, static_name_t("unpressed"), unpressed);
    get_subtexture(image, static_name_t("pressed"), pressed);
    get_subtexture(image, static_name_t("rollover"), rollover);

    adobe::any_regular_t clicked_binding;
    if (get_value(parameters, adobe::static_name_t("bind_clicked_signal"), clicked_binding)) {
        adobe::implementation::cell_and_expression(clicked_binding, item.bind_signal_m, item.expression_m);
    }

    button_t* result = new button_t(is_default,
                                    is_cancel,
                                    color,
                                    text_color,
                                    unpressed,
                                    pressed,
                                    rollover);

    result->color_proxy_m.initialize(set_button_color(false, *result));
    result->text_color_proxy_m.initialize(set_button_color(true, *result));

    for (array_t::const_iterator iter(items.begin()), last(items.end()); iter != last; ++iter) {
        state_set_push_back(*result,
                            token,
                            button_item_t(item, iter->cast<dictionary_t>()),
                            factory);
    }

    bool state_set_originally_empty(result->state_set_m.empty());

    if (state_set_originally_empty)
        state_set_push_back(*result, token, item, factory);

    for (button_state_set_t::const_iterator first(result->state_set_m.begin()), last(result->state_set_m.end()); first != last; ++first)
        modifier_mask |= first->modifier_set_m;

    result->modifier_mask_m = modifier_mask;

    for (array_t::const_iterator iter(items.begin()), last(items.end()); iter != last; ++iter) {
        button_item_t temp(item, iter->cast<dictionary_t>());

        connect_button_state(*result,
                             token.client_holder_m.assemblage_m,
                             token.sheet_m,
                             temp,
                             token.client_holder_m);
    }

    if (state_set_originally_empty) {
        connect_button_state(*result,
                             token.client_holder_m.assemblage_m,
                             token.sheet_m,
                             item,
                             token.client_holder_m);
    }

#define BIND_COLOR(name)                                                \
    adobe::attach_view(result->name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
    BIND_COLOR(color);
    BIND_COLOR(text_color);
#undef BIND_COLOR

    return result;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

widget_node_t make_button(const dictionary_t&     parameters, 
                          const widget_node_t&    parent, 
                          const factory_token_t&  token,
                          const widget_factory_t& factory)
{ 
    size_enum_t   size(parameters.count(key_size) ?
                       implementation::enumerate_size(get_value(parameters, key_size).cast<name_t>()) :
                       parent.size_m);

    button_t* widget =
        implementation::create_button_widget(parameters, token, size, factory);
    token.client_holder_m.assemblage_m.cleanup(boost::bind(delete_ptr<button_t*>(),widget));
   
    //
    // Call display_insertion to embed the new widget within the view heirarchy
    //
    platform_display_type display_token(insert(get_main_display(), parent.display_token_m, *widget));

    // set up key handler code. We do this all the time because we want the button to be updated
    // when modifier keys are pressed during execution of the dialog.

    keyboard_t::iterator keyboard_token(
        token.client_holder_m.keyboard_m.insert(parent.keyboard_token_m, poly_key_handler_t(boost::ref(*widget))));
    
    //
    // As per SF.net bug 1428833, we want to attach the poly_placeable_t
    // to Eve before we attach the controller and view to the model
    //

    eve_t::iterator eve_token;
    eve_token = attach_placeable<poly_placeable_t>(parent.eve_token_m, *widget, parameters, 
        token, factory.is_container(static_name_t("button")),
        factory.layout_attributes(static_name_t("button")));

    //
    // Return the widget_node_t that comprises the tokens created for this widget by the various components
    //
    return widget_node_t(size, eve_token, display_token, keyboard_token);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
