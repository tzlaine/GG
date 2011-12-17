/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP
#define ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP

/****************************************************************************************************/

#include <GG/adobe/array.hpp>
#include <GG/adobe/config.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/ClrConstants.h>

#include <string>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct edit_text_ctor_block_t
{
    edit_text_ctor_block_t() :
        read_only_m(false),
        terminal_style_m(false),
        wrap_m(true),
        scrollable_m(false),
        password_m(false),
        min_characters_m(10),
        max_characters_m(0),
        num_lines_m(1),
        color_m(GG::CLR_GRAY),
        text_color_m(GG::CLR_BLACK),
        interior_color_m(GG::CLR_ZERO),
        label_color_m(GG::CLR_BLACK),
        popup_color_m(GG::CLR_GRAY)
    {}

    std::string name_m;
    std::string alt_text_m;
    bool        read_only_m;
    bool        terminal_style_m;
    bool        wrap_m;
    bool        scrollable_m;
    bool        password_m;
    long        min_characters_m;
    long        max_characters_m; // 0 here means unlimited. Only matters when num_lines_m == 1
    long        num_lines_m;
    GG::Clr     color_m;
    GG::Clr     text_color_m;
    GG::Clr     interior_color_m;
    GG::Clr     label_color_m;
    GG::Clr     popup_color_m;
    name_t      signal_id_m;
};

/*************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP
#endif

/****************************************************************************************************/
