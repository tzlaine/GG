/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_progress_bar.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/GUI.h>
#include <GG/ProgressBar.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

progress_bar_t::progress_bar_t(bool is_vertical,
                               const value_range_format_t& format,
                               int length,
                               int width,
                               GG::Clr color,
                               GG::Clr bar_color,
                               GG::Clr interior_color) :
    control_m(0),
    is_vertical_m(is_vertical),
    length_m(length),
    width_m(width),
    color_m(color),
    bar_color_m(bar_color),
    interior_color_m(interior_color),
    last_m(-1.0),
    value_m(0.0),
    format_m(format)
{}

/****************************************************************************************************/

void progress_bar_t::display(const model_type& value)
{
    value_m = value.type_info() == type_info<empty_t>() ? 0 : value.cast<double>();

    double new_position(format_m.find(value) / static_cast<double>(format_m.size()));

    if (new_position != last_m) {
        last_m = new_position;
        control_m->SetPosition(last_m);
    }
}

/*************************************************************************************************/

void progress_bar_t::measure(extents_t& result)
{
    GG::Pt min_size = control_m->MinUsableSize();
    result.width() = Value(min_size.x);
    result.height() = Value(min_size.y);
}

/*************************************************************************************************/

void progress_bar_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

template <>
platform_display_type insert<progress_bar_t>(display_t&              display,
                                             platform_display_type&  parent,
                                             progress_bar_t&         element)
{
    assert(!element.control_m);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();

    const int length = 0 < element.length_m ? element.length_m : 100;
    const int width = 0 < element.width_m ? element.width_m : 14;
    element.control_m =
        style->NewProgressBar(GG::X0, GG::Y0, GG::X(length), GG::Y(length),
                              element.is_vertical_m ? GG::VERTICAL : GG::HORIZONTAL,
                              width, element.color_m, element.bar_color_m, element.interior_color_m);

    element.color_proxy_m.initialize(
        boost::bind(&GG::ProgressBar::SetColor, element.control_m, _1)
    );
    element.bar_color_proxy_m.initialize(
        boost::bind(&GG::ProgressBar::SetBarColor, element.control_m, _1)
    );
    element.interior_color_proxy_m.initialize(
        boost::bind(&GG::ProgressBar::SetInteriorColor, element.control_m, _1)
    );

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe
