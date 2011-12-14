/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>

#include <GG/GUI.h>
#include <GG/Texture.h>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

name_t state_cell_unique_name()
{
    // this takes an increasing counter and converts it to base 26,
    // turning it into a string, and returning the string. It prepends
    // a ZUID to dramatically increase the avoidance of a collision with
    // a cell already named in the sheet.
    //
    // (I'd use a zuid here, but there are symbols missing on Mac OS X:)
    //      _sprintf$LDBLStub
    //      _sscanf$LDBLStub

    static long count(-1);
    std::string result("__22c8c184-fb4e-1fff-8f9e-8dc7baa26187_");
    long        tmp(++count);

    while(true)
    {
        long n(tmp / 26);
        long r(tmp % 26);

        tmp = n;        

        result += static_cast<char>('a' + r);

        if (tmp == 0)
            break;
    }

    return name_t(result.c_str());
}

/****************************************************************************************************/

void align_slices(extents_t::slice_t& slice_one, extents_t::slice_t slice_two)
{
    //
    // Make sure that we have points of interest to align by.
    //
    // REVISIT (fbrereto) Are we sure we want to assert here?
    //

    assert(!slice_one.guide_set_m.empty());   
    assert(!slice_two.guide_set_m.empty());

    //
    // The eventual point of interest (or baseline) is the maxiumum
    // of the two we have.
    //

    long poi = (std::max)(slice_one.guide_set_m[0], slice_two.guide_set_m[0]);

    //
    // The length of the slice is the maximum amount before the
    // point of interest (ascent above baseline) plus the maximum
    // amount after the point of interest (descent below the
    // baseline).
    //
    // The maximum before the point of interest is obviously the
    // the point of interest.
    //

    long length = poi + (std::max)( slice_one.length_m - slice_one.guide_set_m[0],
                                    slice_two.length_m - slice_two.guide_set_m[0]);

    //
    // Set our new values into the first slice.
    //

    slice_one.guide_set_m[0] = poi;
    slice_one.length_m = length;
}

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

bool get_color(const dictionary_t& parameters, name_t name, GG::Clr& color)
{
    dictionary_t colors;
    if (!get_value(parameters, name, colors))
        return false;

    unsigned int r = 0, g = 0, b = 0, a = 255;
    bool color_set = false;
    color_set |= get_value(colors, static_name_t("r"), r);
    color_set |= get_value(colors, static_name_t("g"), g);
    color_set |= get_value(colors, static_name_t("b"), b);
    color_set |= get_value(colors, static_name_t("a"), a);

    if (color_set)
        color = GG::Clr(r, g, b, a);

    return color_set;
}

/****************************************************************************************************/

bool get_subtexture(const dictionary_t& parameters, name_t name, GG::SubTexture& subtexture)
{
    any_regular_t value;
    if (!get_value(parameters, name, value))
        return false;

    return get_subtexture(value, subtexture);
}

/****************************************************************************************************/

bool get_subtexture(const any_regular_t& value, GG::SubTexture& subtexture)
{
    bool retval = false;

    if (value.cast(subtexture)) {
        retval = true;
    } else {
        boost::shared_ptr<GG::Texture> texture;
        std::string texture_name;
        if (value.cast(texture_name)) {
            try {
                texture = GG::GUI::GetGUI()->GetTexture(texture_name);
                texture->SetFilters(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
            } catch (...) {}
            retval = true;
        } else if (value.cast(texture)) {
            retval = true;
        }
        subtexture = GG::SubTexture(texture);
    }

    return retval;
}

/****************************************************************************************************/

GG::StateButtonStyle name_to_style(adobe::name_t name)
{
#define CASE(x) if (name == adobe::static_name_t(#x)) return GG::x

    CASE(SBSTYLE_3D_XBOX);
    else CASE(SBSTYLE_3D_CHECKBOX);
    else CASE(SBSTYLE_3D_RADIO);
    else CASE(SBSTYLE_3D_BUTTON);
    else CASE(SBSTYLE_3D_ROUND_BUTTON);
    else CASE(SBSTYLE_3D_TOP_ATTACHED_TAB);
    else CASE(SBSTYLE_3D_TOP_DETACHED_TAB);
    else throw std::runtime_error("Unknown StateButtonStyle name");

#undef CASE
}

/****************************************************************************************************/

void replace_placeholder(array_t& expression, name_t name, const any_regular_t& value)
{
    for (std::size_t i = 0; i < expression.size(); ++i) {
        name_t element_name;
        if (expression[i].cast<name_t>(element_name) && element_name == name)
            expression[i] = value;
    }
}

/****************************************************************************************************/

void replace_placeholders(array_t& expression,
                          const any_regular_t& _,
                          const any_regular_t& _1,
                          const any_regular_t& _2/* = any_regular_t()*/,
                          const any_regular_t& _3/* = any_regular_t()*/,
                          const any_regular_t& _4/* = any_regular_t()*/)
{
    replace_placeholder(expression, static_name_t("_"), _);
    if (!empty(_1))
        replace_placeholder(expression, static_name_t("_1"), _1);
    if (!empty(_2))
        replace_placeholder(expression, static_name_t("_2"), _2);
    if (!empty(_3))
        replace_placeholder(expression, static_name_t("_3"), _3);
    if (!empty(_4))
        replace_placeholder(expression, static_name_t("_4"), _4);
}

/****************************************************************************************************/

void handle_signal(signal_notifier_t signal_notifier,
                   name_t widget_name,
                   name_t signal_name,
                   name_t widget_id,
                   sheet_t& sheet,
                   name_t bind,
                   array_t expression,
                   const any_regular_t& _1,
                   const any_regular_t& _2/* = any_regular_t()*/,
                   const any_regular_t& _3/* = any_regular_t()*/,
                   const any_regular_t& _4/* = any_regular_t()*/)
{
    if (!bind && !signal_notifier)
        return;

    any_regular_t _;
    {
        dictionary_t dict;

        if (!empty(_1))
            dict[static_name_t("_1")] = _1;
        if (!empty(_2))
            dict[static_name_t("_2")] = _2;
        if (!empty(_3))
            dict[static_name_t("_3")] = _3;
        if (!empty(_4))
            dict[static_name_t("_4")] = _4;

        _ = any_regular_t(dict);
    }

    any_regular_t value;
    if (expression.empty()) {
        value = _;
    } else {
        replace_placeholders(expression, _, _1, _2, _3, _4);
        value = sheet.inspect(expression);
    }

    if (bind) {
        sheet.set(bind, value);
        sheet.update();
    } else if (signal_notifier) {
        signal_notifier(widget_name, signal_name, widget_id, value);
    }
}

/****************************************************************************************************/

void cell_and_expression(const any_regular_t& value,
                         name_t& cell,
                         array_t& expression)
{
    array_t cell_and_expression;
    value.cast<name_t>(cell);
    if (!cell && value.cast<array_t>(cell_and_expression)) {
        cell = cell_and_expression[0].cast<name_t>();
        const std::string& expression_string = cell_and_expression[0].cast<std::string>();
        expression = parse_adam_expression(expression_string);
    }
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
