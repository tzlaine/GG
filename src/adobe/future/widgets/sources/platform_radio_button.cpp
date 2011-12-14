/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_radio_button.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void radio_button_clicked(adobe::radio_button_t& button, bool)
{
    if (!button.hit_proc_m.empty())
        button.hit_proc_m(button.set_value_m);

    if (!button.checked_proc_m.empty())
        button.checked_proc_m(button.set_value_m);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

radio_button_t::radio_button_t(const std::string&   name,
                               const std::string&   alt_text,
                               const any_regular_t& set_value,
                               const GG::Clr&       color,
                               const GG::Clr&       text_color,
                               const GG::Clr&       interior_color,
                               GG::StateButtonStyle style,
                               name_t               signal_id) :
    control_m(0),
    name_m(name),
    alt_text_m(alt_text),
    set_value_m(set_value),
    color_m(color),
    text_color_m(text_color),
    interior_color_m(interior_color),
    style_m(style),
    signal_id_m(signal_id)
{ }

/****************************************************************************************************/

void radio_button_t::measure(extents_t& result)
{
    GG::Pt min_usable_size = control_m->MinUsableSize();

    result.width() = Value(min_usable_size.x);
    result.height() = Value(min_usable_size.y);

    result.vertical().guide_set_m.push_back(
        Value((min_usable_size.y - implementation::DefaultFont()->Lineskip()) / 2 +
              implementation::DefaultFont()->Ascent())
    );
}

/****************************************************************************************************/

void radio_button_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

void radio_button_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void radio_button_t::display(const any_regular_t& value)
{
    assert(control_m);

    if (last_m != value) {
        last_m = value;
        control_m->SetCheck(last_m == set_value_m);
    }
}

/****************************************************************************************************/

void radio_button_t::monitor(const setter_type& proc)
{
    assert(control_m);
    hit_proc_m = proc;
}

/****************************************************************************************************/

template <>
platform_display_type insert<radio_button_t>(display_t&             display,
                                             platform_display_type& parent,
                                             radio_button_t&        element)
{
    element.control_m =
        implementation::Factory().NewStateButton(GG::X0, GG::Y0, GG::X1,
                                                 implementation::StandardHeight(), element.name_m,
                                                 implementation::DefaultFont(),
                                                 element.color_m, element.text_color_m,
                                                 element.interior_color_m, element.style_m);

    GG::Connect(element.control_m->CheckedSignal,
                boost::bind(&radio_button_clicked, boost::ref(element), _1));

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
