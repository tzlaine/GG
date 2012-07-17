/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_group.hpp>

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/GroupBox.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

group_t::group_t(const std::string& name,
                 const std::string& alt_text,
                 GG::Clr            color,
                 GG::Clr            text_color,
                 GG::Clr            interior_color) :
    control_m(0),
    name_m(name),
    alt_text_m(alt_text),
    color_m(color),
    text_color_m(text_color),
    interior_color_m(interior_color)
{ }

/****************************************************************************************************/

void group_t::measure(extents_t& result)
{
    assert(control_m);

    if (name_m.empty()) {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    result = metrics::measure_text(name_m, implementation::DefaultFont());

    result.width() += 15;

    result.vertical().frame_m.first = result.height() + 7;

    result.height() = 5;
}

/****************************************************************************************************/

void group_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

template <>
platform_display_type insert<group_t>(display_t&             display,
                                      platform_display_type& parent,
                                      group_t&               element)
{
    element.control_m =
        implementation::Factory().NewGroupBox(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                              element.name_m,
                                              implementation::DefaultFont(),
                                              element.color_m,
                                              element.text_color_m,
                                              element.interior_color_m);
    element.control_m->SetClientCornersEqualToBoxCorners(true);

    element.color_proxy_m.initialize(
        boost::bind(&GG::GroupBox::SetColor, element.control_m, _1)
    );
    element.text_color_proxy_m.initialize(
        boost::bind(&GG::GroupBox::SetTextColor, element.control_m, _1)
    );
    element.interior_color_proxy_m.initialize(
        boost::bind(&GG::GroupBox::SetInteriorColor, element.control_m, _1)
    );

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
