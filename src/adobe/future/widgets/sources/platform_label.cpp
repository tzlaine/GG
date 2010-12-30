/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_label.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/placeable_concept.hpp>

#include <string>
#include <cassert>

#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

label_t::label_t(const std::string& name,
                 const std::string& alt_text,
                 std::size_t        characters,
                 theme_t            theme) :
    window_m(0),
    theme_m(theme),
    name_m(name),
    alt_text_m(alt_text),
    characters_m(characters)
{ }

/****************************************************************************************************/

void place(label_t& value, const place_data_t& place_data)
{
    implementation::set_control_bounds(value.window_m, place_data);
    if (!value.alt_text_m.empty())
        implementation::set_control_alt_text(value.window_m, value.alt_text_m);
}

/****************************************************************************************************/

void measure(label_t& value, extents_t& result)
{
    assert(value.window_m);
    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    GG::Pt size =
        style->DefaultFont()->TextExtent(value.characters_m ?
                                         std::string(value.characters_m, '0') :
                                         value.name_m,
                                         value.window_m->GetTextFormat());
    result.horizontal().length_m = Value(size.x);
    assert(result.horizontal().length_m);
}

/****************************************************************************************************/

void measure_vertical(label_t& value, extents_t& calculated_horizontal, 
                      const place_data_t& placed_horizontal)
{
    assert(value.window_m);
    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    GG::Pt size =
        style->DefaultFont()->TextExtent(value.name_m, value.window_m->GetTextFormat(),
                                         GG::X(width(placed_horizontal)));
    calculated_horizontal.vertical().length_m = Value(size.y);
    calculated_horizontal.vertical().guide_set_m.push_back(
        Value(style->DefaultFont()->Ascent()));
}

/****************************************************************************************************/

void enable(label_t& value, bool make_enabled)
{ value.window_m->Disable(make_enabled); }

/****************************************************************************************************/

extents_t measure_text(const std::string& text, const boost::shared_ptr<GG::Font>& font)
{
    extents_t result;
    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    GG::Pt size = font->TextExtent(text);
    result.horizontal().length_m = Value(size.x);
    result.vertical().length_m = Value(size.y);
    return result;
}

/****************************************************************************************************/

std::string get_control_string(const label_t& widget)
{ return widget.window_m->Text(); }

/****************************************************************************************************/

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/
//              container idiom for event and drawing system.

template <>
platform_display_type insert<label_t>(display_t& display,
                                      platform_display_type& parent,
                                      label_t& element)
{
    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    element.window_m = style->NewTextControl(GG::X0, GG::Y0, GG::X(100), GG::Y(100),
                                             element.name_m, style->DefaultFont(),
                                             GG::CLR_BLACK, GG::FORMAT_WORDBREAK);

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.window_m, element.alt_text_m);

    return display.insert(parent, get_display(element));
}


/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
