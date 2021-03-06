/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_button.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>

/****************************************************************************************************/

namespace {

void button_clicked(adobe::button_t& m_button)
{
    adobe::button_state_set_t::iterator state(
        adobe::button_modifier_state(m_button.state_set_m,
                                     m_button.modifier_mask_m,
                                     m_button.modifiers_m)
    );

    if (state == m_button.state_set_m.end())
        state = adobe::button_default_state(m_button.state_set_m);

    if (!state->hit_proc_m.empty())
        state->hit_proc_m(state->value_m, state->contributing_m);

    if (!state->clicked_proc_m.empty())
        state->clicked_proc_m(state->value_m);
}

}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

set_button_color::set_button_color(bool text, button_t& button) :
    text_m(text),
    color_m(text ? button.text_color_m : button.color_m),
    button_m(button.control_m)
{}

/****************************************************************************************************/

void set_button_color::operator()(GG::Clr c) const
{
    color_m = c;
    if (button_m) {
        if (text_m)
            button_m->SetTextColor(c);
        else
            button_m->SetColor(c);
    }
}

/****************************************************************************************************/

button_t::button_t(bool                             is_default,
                   bool                             is_cancel,
                   GG::Clr                          color,
                   GG::Clr                          text_color,
                   const GG::SubTexture&            unpressed,
                   const GG::SubTexture&            pressed,
                   const GG::SubTexture&            rollover) :
    control_m(0),
    modifier_mask_m(modifiers_none_s),
    modifiers_m(modifiers_none_s),
    is_default_m(is_default),
    is_cancel_m(is_cancel),
    enabled_m(true),
    color_m(color),
    text_color_m(text_color),
    unpressed_m(unpressed),
    pressed_m(pressed),
    rollover_m(rollover)
{ }

/****************************************************************************************************/

void button_t::measure(extents_t& result)
{
    result = metrics::measure(control_m);

    button_state_set_t::iterator state(button_modifier_state(state_set_m,
                                                             modifier_mask_m,
                                                             modifiers_m));

    boost::shared_ptr<GG::Font> font = implementation::DefaultFont();

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    extents_t cur_text_extents(metrics::measure_text(state->name_m, font));

    result.width() -= cur_text_extents.width();
    result.height() = Value(control_m->Height());

    long width_additional(0);
    
    for (button_state_set_t::iterator iter(state_set_m.begin()), last(state_set_m.end()); iter != last; ++iter)
    {
        extents_t tmp(metrics::measure_text(iter->name_m, font));
        width_additional = (std::max)(width_additional, tmp.width());
    }

    result.width() += width_additional;
    result.width() += Value(2 * implementation::CharWidth());

    result.width() = (std::max)(result.width(), 70L);
}

/****************************************************************************************************/

void button_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

void button_t::enable(bool make_enabled)
{
    enabled_m = make_enabled;
    if (control_m)
        control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void button_t::set(modifiers_t modifiers, const model_type& value)
{
    button_state_set_t::iterator state(button_modifier_state(state_set_m, modifier_mask_m, modifiers));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    if (state->value_m != value)
        state->value_m = value;
}

/****************************************************************************************************/

void button_t::set_contributing(modifiers_t modifiers, const dictionary_t& value)
{
    button_state_set_t::iterator state(button_modifier_state(state_set_m, modifier_mask_m, modifiers));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    state->contributing_m = value;
}

/****************************************************************************************************/

bool button_t::handle_key(key_type key, bool pressed, modifiers_t /* modifiers */)
{
    if (pressed == false)
        return false;

    modifiers_m = modifier_state();

    //
    // Look up the state which this modifier should trigger.
    //
    button_state_set_t::iterator state(button_modifier_state(state_set_m,
                                                             modifier_mask_m,
                                                             modifiers_m));

    if (state == state_set_m.end())
        state = button_default_state(state_set_m);

    //
    // Set the window text.
    //
    control_m->SetText(state->name_m);

    //
    // Set the alt text if need be.
    //
    if (!state->alt_text_m.empty())
        implementation::set_control_alt_text(control_m, state->alt_text_m);

    if (state->hit_proc_m.empty() || enabled_m == false)
        return false;

    if ((key.first == GG::GGK_RETURN || key.first == GG::GGK_KP_ENTER) && is_default_m)
        state->hit_proc_m(state->value_m, state->contributing_m);
    else if (key.first == GG::GGK_ESCAPE && is_cancel_m)
        state->hit_proc_m(state->value_m, state->contributing_m);
    else
        return false;

    return true;
}

/****************************************************************************************************/

template <>
platform_display_type insert<button_t>(display_t&             display,
                                       platform_display_type& parent,
                                       button_t&              element)
{
    assert(element.control_m == 0);

    button_state_set_t::iterator state(button_default_state(element.state_set_m));

    element.control_m =
        implementation::Factory().NewButton(GG::X0, GG::Y0, GG::X1, implementation::StandardHeight(),
                                            state->name_m, implementation::DefaultFont(),
                                            element.color_m, element.text_color_m);

    element.control_m->SetUnpressedGraphic(element.unpressed_m);
    element.control_m->SetPressedGraphic(element.pressed_m);
    element.control_m->SetRolloverGraphic(element.rollover_m);

    const GG::X max_width =
        std::max(element.unpressed_m.Empty() ? GG::X0 : element.unpressed_m.Width(),
                 std::max(element.pressed_m.Empty() ? GG::X0 : element.pressed_m.Width(),
                          element.rollover_m.Empty() ? GG::X0 : element.rollover_m.Width()));
    const GG::Y max_height =
        std::max(element.unpressed_m.Empty() ? GG::Y0 : element.unpressed_m.Height(),
                 std::max(element.pressed_m.Empty() ? GG::Y0 : element.pressed_m.Height(),
                          element.rollover_m.Empty() ? GG::Y0 : element.rollover_m.Height()));
    const GG::X min_width =
        std::min(element.unpressed_m.Empty() ? GG::X0 : element.unpressed_m.Width(),
                 std::min(element.pressed_m.Empty() ? GG::X0 : element.pressed_m.Width(),
                          element.rollover_m.Empty() ? GG::X0 : element.rollover_m.Width()));
    const GG::Y min_height =
        std::min(element.unpressed_m.Empty() ? GG::Y0 : element.unpressed_m.Height(),
                 std::min(element.pressed_m.Empty() ? GG::Y0 : element.pressed_m.Height(),
                          element.rollover_m.Empty() ? GG::Y0 : element.rollover_m.Height()));
    if (max_width && max_height && min_width && min_height) {
        element.control_m->SetMaxSize(GG::Pt(max_width, max_height));
        element.control_m->SetMinSize(GG::Pt(min_width, min_height));
    }

    GG::Connect(element.control_m->ClickedSignal,
                boost::bind(&button_clicked, boost::ref(element)));

    if (!state->alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, state->alt_text_m);

    element.control_m->Disable(!element.enabled_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
