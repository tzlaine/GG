/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

#include "platform_display_text.hpp"

#include <GG/adobe/string.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <GG/TextControl.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>

#include <string>
#include <cassert>
#include <sstream>


namespace {

    std::string field_text(const std::string& label, adobe::any_regular_t value)
    {
        std::stringstream result;
        if (!label.empty())
            result << label << " ";
        if (value != adobe::any_regular_t()) {
            adobe::type_info_t type(value.type_info());
            if (type == adobe::type_info<double>() ||
                type == adobe::type_info<bool>() ||
                type == adobe::type_info<adobe::name_t>()) {
                result << value;
            } else {
                result << value.cast<adobe::string_t>();
            }
        }
        return result.str();
    }

}

namespace adobe {

    display_text_t::display_text_t(const std::string& name, const std::string& alt_text, int characters, GG::Clr color) :
        name_m(name),
        alt_text_m(alt_text),
        characters_m(characters),
        color_m(color)
    {}

    void display_text_t::place(const place_data_t& place_data)
    { implementation::set_control_bounds(window_m, place_data); }

    void display_text_t::display(const model_type& value)
    {
        assert(window_m);
        window_m->SetText(field_text(name_m, value));
    }

    void display_text_t::measure(extents_t& result)
    {
        assert(window_m);

        boost::shared_ptr<GG::Font> font = implementation::DefaultFont();

        extents_t space_extents(metrics::measure_text(std::string(" "), font));
        extents_t label_extents(metrics::measure_text(name_m, font));
        extents_t characters_extents(metrics::measure_text(std::string(characters_m, '0'), font));

        // set up default settings (baseline, etc.)
        result = space_extents;

        // set the width to the label width (if any)
        result.width() = label_extents.width();

        // add a guide for the label
        result.horizontal().guide_set_m.push_back(label_extents.width());

        // if there's a label, add space for a space
        if (label_extents.width() != 0)
            result.width() += space_extents.width();

        // append the character extents (if any)
        result.width() += characters_extents.width();

        // if there are character extents, add space for a space
        if (characters_extents.width() != 0)
            result.width() += space_extents.width();

        assert(result.horizontal().length_m);
    }

    void display_text_t::measure_vertical(extents_t& calculated_horizontal, const place_data_t& placed_horizontal)
    {
        assert(window_m);

        extents_t::slice_t& vert = calculated_horizontal.vertical();
        vert.length_m = Value(implementation::DefaultFont()->Lineskip());
        vert.guide_set_m.push_back(Value(implementation::DefaultFont()->Ascent()));
    }

    template <>
    platform_display_type insert<display_text_t>(display_t& display,
                                                 platform_display_type& parent,
                                                 display_text_t& element)
    {
        element.window_m =
            implementation::Factory().NewTextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                     element.name_m, implementation::DefaultFont(),
                                                     element.color_m, GG::FORMAT_NONE, GG::INTERACTIVE);

        if (!element.alt_text_m.empty())
            implementation::set_control_alt_text(element.window_m, element.alt_text_m);

        return display.insert(parent, get_display(element));
    }

}
