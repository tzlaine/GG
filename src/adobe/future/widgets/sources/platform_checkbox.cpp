/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_checkbox.hpp>

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>


#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void checkbox_checked(adobe::checkbox_t& checkbox, bool checked)
{
    if (checkbox.hit_proc_m)
        checkbox.hit_proc_m(checked ? checkbox.true_value_m : checkbox.false_value_m);

    if (checkbox.checked_proc_m)
        checkbox.checked_proc_m(checked ? checkbox.true_value_m : checkbox.false_value_m);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

checkbox_t::checkbox_t( const std::string&                      name,
                        const any_regular_t&                    true_value,
                        const any_regular_t&                    false_value,
                        GG::Clr                                 color,
                        GG::Clr                                 text_color,
                        GG::Clr                                 interior_color,
                        GG::StateButtonStyle                    style,
                        const std::string&                      alt_text,
                        name_t                                  signal_id) :
    control_m(0),
    true_value_m(true_value),
    false_value_m(false_value),
    name_m(name),
    color_m(color),
    text_color_m(text_color),
    interior_color_m(interior_color),
    style_m(style),
    alt_text_m(alt_text),
    signal_id_m(signal_id)
{ }

/****************************************************************************************************/

void checkbox_t::measure(extents_t& result)
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

void checkbox_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

void checkbox_t::enable(bool make_enabled)
{
    if (control_m)
        control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void checkbox_t::display(const any_regular_t& new_value)
{
    assert(control_m);

    if (current_value_m == new_value) return;

    current_value_m = new_value;

    if (current_value_m == true_value_m)
        control_m->SetCheck(true);
    else if (current_value_m == false_value_m)
        control_m->SetCheck(false);
    else
        assert(!"checkbox_t::display() : non-true and non-false value set");
}

/****************************************************************************************************/

void checkbox_t::monitor(const setter_type& proc)
{
    assert(control_m);

    hit_proc_m = proc;
}

/****************************************************************************************************/

template <>
platform_display_type insert<checkbox_t>(display_t&             display,
                                         platform_display_type& parent,
                                         checkbox_t&            element)
{
    assert(element.control_m == 0);

    element.control_m =
        implementation::Factory().NewStateButton(GG::X0, GG::Y0, GG::X1, implementation::StandardHeight(),
                                                 element.name_m, implementation::DefaultFont(),
                                                 element.color_m, element.text_color_m,
                                                 element.interior_color_m, element.style_m);

    GG::Connect(element.control_m->CheckedSignal,
                boost::bind(&checkbox_checked, boost::ref(element), _1));

    if (!element.alt_text_m.empty())
        adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    element.color_proxy_m.initialize(
        boost::bind(&GG::StateButton::SetColor, element.control_m, _1)
    );
    element.text_color_proxy_m.initialize(
        boost::bind(&GG::StateButton::SetTextColor, element.control_m, _1)
    );
    element.interior_color_proxy_m.initialize(
        boost::bind(&GG::StateButton::SetInteriorColor, element.control_m, _1)
    );

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
