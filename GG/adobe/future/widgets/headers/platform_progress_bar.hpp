/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_PROGRESS_BAR_HPP
#define ADOBE_PROGRESS_BAR_HPP

#include <GG/adobe/config.hpp>

#include <boost/utility.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/extents.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/value_range_format.hpp>


namespace GG {
    class ProgressBar;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct progress_bar_t : boost::noncopyable
{
    typedef progress_bar_t widget_type_t;
    typedef any_regular_t model_type;

    progress_bar_t(bool is_vertical,
                   const value_range_format_t& format,
                   int length,
                   int width,
                   GG::Clr color,
                   GG::Clr bar_color,
                   GG::Clr interior_color);

    void measure(extents_t& result);
    void place(const place_data_t& place_data);
    void display(const model_type& value);

    GG::ProgressBar*     control_m;
    bool                 is_vertical_m;
    int                  length_m;
    int                  width_m;
    GG::Clr              color_m;
    GG::Clr              bar_color_m;
    GG::Clr              interior_color_m;
    double               last_m;
    double               value_m;
    value_range_format_t format_m;

    implementation::color_proxy_t color_proxy_m;
    implementation::color_proxy_t bar_color_proxy_m;
    implementation::color_proxy_t interior_color_proxy_m;
};

/****************************************************************************************************/

}

/****************************************************************************************************/

#endif
