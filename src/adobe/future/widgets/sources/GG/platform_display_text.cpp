/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

#include "platform_display_text.hpp"

#include <GG/adobe/dictionary.hpp>
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

    std::string field_text(const std::string& label, const adobe::any_regular_t& value, GG::Clr label_color)
    {
        std::stringstream result;
        if (!label.empty())
            result << GG::RgbaTag(label_color) << label << "</rgba> ";
        if (value != adobe::any_regular_t()) {
            adobe::type_info_t type(value.type_info());
            if (type == adobe::type_info<double>() ||
                type == adobe::type_info<bool>() ||
                type == adobe::type_info<adobe::name_t>()) {
                result << value;
            } else if (type == adobe::type_info<adobe::array_t>()) {
                result << '[';
                const adobe::array_t& array = value.cast<adobe::array_t>();
                for (adobe::array_t::const_iterator it = array.begin(), end_it = array.end();
                     it != end_it;
                     ++it) {
                    result << field_text("", *it, label_color);
                    if (boost::next(it) != end_it)
                        result << ',';
                }
                result << ']';
            } else if (type == adobe::type_info<adobe::dictionary_t>()) {
                result << '{';
                const adobe::dictionary_t& dictionary = value.cast<adobe::dictionary_t>();
                for (adobe::dictionary_t::const_iterator it = dictionary.begin(), end_it = dictionary.end();
                     it != end_it;
                     ++it) {
                    result << it->first << ": " << field_text("", it->second, label_color);
                    if (boost::next(it) != end_it)
                        result << ',';
                }
                result << '}';
            } else {
                result << value.cast<adobe::string_t>();
            }
        }
        return result.str();
    }

}

namespace adobe {

    display_text_t::display_text_t(const std::string& name,
                                   const std::string& alt_text,
                                   int characters,
                                   GG::Clr color,
                                   GG::Clr label_color) :
        name_m(name),
        alt_text_m(alt_text),
        characters_m(characters),
        color_m(color),
        label_color_m(label_color)
    {}

    void display_text_t::place(const place_data_t& place_data)
    { implementation::set_control_bounds(window_m, place_data); }

    void display_text_t::display(const model_type& value)
    {
        assert(window_m);
        value_m = value;
        window_m->SetText(field_text(name_m, value_m, label_color_m));
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

    void display_text_t::set_label_color(GG::Clr color)
    {
        label_color_m = color;
        display(value_m);
    }

    template <>
    platform_display_type insert<display_text_t>(display_t& display,
                                                 platform_display_type& parent,
                                                 display_text_t& element)
    {
        element.window_m =
            implementation::Factory().NewTextControl(GG::X0, GG::Y0, GG::X1, GG::Y1,
                                                     element.name_m, implementation::DefaultFont(),
                                                     element.color_m, GG::FORMAT_LEFT | GG::FORMAT_TOP,
                                                     GG::INTERACTIVE);

        if (!element.alt_text_m.empty())
            implementation::set_control_alt_text(element.window_m, element.alt_text_m);

        element.color_proxy_m.initialize(
            boost::bind(&GG::TextControl::SetColor, element.window_m, _1)
        );
        element.label_color_proxy_m.initialize(
            boost::bind(&display_text_t::set_label_color, &element, _1)
        );

        return display.insert(parent, get_display(element));
    }

}
