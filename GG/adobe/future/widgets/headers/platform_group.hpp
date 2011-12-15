/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_GROUP_HPP
#define ADOBE_WIDGET_GROUP_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/extents.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <GG/Clr.h>

#include <string>


namespace GG {
    class GroupBox;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct group_t
{
    group_t(const std::string& name,
            const std::string& alt_text,
            GG::Clr            color,
            GG::Clr            text_color,
            GG::Clr            interior_color);

    void measure(extents_t& result);

    void place(const place_data_t& place_data);

    GG::GroupBox*        control_m;
    std::string          name_m;
    std::string          alt_text_m;
    GG::Clr              color_m;
    GG::Clr              text_color_m;
    GG::Clr              interior_color_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
