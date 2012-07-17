/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_DISPLAY_NUMBER_HPP
#define ADOBE_DISPLAY_NUMBER_HPP

/****************************************************************************************************/

#include <GG/adobe/memory.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/number_unit.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/Clr.h>
#include <GG/PtRect.h>

#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>

#include <string>


namespace GG {
    class TextControl;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct display_number_t  : boost::noncopyable 
{
    typedef double model_type;

    typedef std::vector<unit_t> unit_set_t;

    template <class ForwardIterator>
    display_number_t(const std::string& name,
                     const std::string& alt_text,
                     ForwardIterator    first,
                     ForwardIterator    last,
                     GG::Clr            color,
                     GG::Clr            label_color,
                     int                characters);

    GG::TextControl*         window_m;
    std::string              name_m;
    std::string              alt_text_m;
    unit_set_t               unit_set_m;
    GG::Clr                  color_m;
    GG::Clr                  label_color_m;
    int                      characters_m;
    double                   value_m;

    void measure(extents_t& result);
    void measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal);       
    void place(const place_data_t& place_data);
    void display(const model_type& value);
    void set_label_color(GG::Clr color);

    implementation::color_proxy_t color_proxy_m;
    implementation::color_proxy_t label_color_proxy_m;
};

/****************************************************************************************************/

template <typename ForwardIterator>
display_number_t::display_number_t(
    const std::string& name,
    const std::string& alt_text,
    ForwardIterator    first,
    ForwardIterator    last, 
    GG::Clr            color,
    GG::Clr            label_color,
    int                characters) :
    name_m(name),
    alt_text_m(alt_text),
    unit_set_m(first, last),
    color_m(color),
    label_color_m(label_color),
    characters_m(characters),
    value_m(0)
{}

/****************************************************************************************************/

inline GG::TextControl* get_display(display_number_t& widget)
{ return widget.window_m; }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
