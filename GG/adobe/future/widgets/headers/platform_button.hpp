/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_UI_CORE_BUTTON_HPP
#define ADOBE_UI_CORE_BUTTON_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <boost/noncopyable.hpp>

#include <GG/Texture.h>


namespace GG {
    class Button;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct button_t : boost::noncopyable
{
    typedef any_regular_t model_type;

    button_t(bool                             is_default,
             bool                             is_cancel,
             modifiers_t                      modifier_mask,
             const GG::Clr&                   color,
             const GG::Clr&                   text_color,
             const GG::SubTexture&            unpressed,
             const GG::SubTexture&            pressed,
             const GG::SubTexture&            rollover,
             const button_state_descriptor_t* first,
             const button_state_descriptor_t* last);

    void measure(extents_t& result);

    void place(const place_data_t& place_data);

    void enable(bool make_enabled);

    void display(const any_regular_t& item);

    void set(modifiers_t modifiers, const model_type& value);

    void set_contributing(modifiers_t modifiers, const dictionary_t& value);

    any_regular_t underlying_handler() { return any_regular_t(control_m); }

    bool handle_key(key_type key, bool pressed, modifiers_t modifiers);

    GG::Button*        control_m;
    button_state_set_t state_set_m;
    modifiers_t        modifier_mask_m;
    modifiers_t        modifiers_m;
    bool               is_default_m;
    bool               is_cancel_m;
    bool               enabled_m;
    GG::Clr            color_m;
    GG::Clr            text_color_m;
    GG::SubTexture     unpressed_m;
    GG::SubTexture     pressed_m;
    GG::SubTexture     rollover_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
