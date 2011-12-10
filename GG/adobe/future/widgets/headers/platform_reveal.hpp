/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_REVEAL_HPP
#define ADOBE_WIDGET_REVEAL_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>

#include <GG/Texture.h>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>

#include <boost/function.hpp>


namespace GG {
    class Button;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct reveal_t : boost::noncopyable
{
    typedef any_regular_t model_type;

    typedef boost::function<void (const model_type&)> setter_type;

    reveal_t(const std::string&    name,
             const any_regular_t&  show_value,
             const std::string&    alt_text,
             const GG::Clr&        text_color,
             const GG::SubTexture& show_unpressed,
             const GG::SubTexture& show_pressed,
             const GG::SubTexture& show_rollover,
             const GG::SubTexture& hide_unpressed,
             const GG::SubTexture& hide_pressed,
             const GG::SubTexture& hide_rollover);

    void measure(extents_t& result);

    void place(const place_data_t& place_data);

    void display(const any_regular_t& value);

    void monitor(const setter_type& proc);

    GG::Button*         control_m;
    GG::Clr             text_color_m;
    GG::SubTexture      show_unpressed_m;
    GG::SubTexture      show_pressed_m;
    GG::SubTexture      show_rollover_m;
    GG::SubTexture      hide_unpressed_m;
    GG::SubTexture      hide_pressed_m;
    GG::SubTexture      hide_rollover_m;
    label_t             name_m;
    bool                using_label_m;
    setter_type         hit_proc_m;
    any_regular_t       show_value_m;
    any_regular_t       current_value_m;
    std::string         alt_text_m;
};

/****************************************************************************************************/

namespace view_implementation {

/****************************************************************************************************/

void set_value_from_model(reveal_t& value, const any_regular_t& new_value);

/****************************************************************************************************/

} // namespace view_implementation

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
