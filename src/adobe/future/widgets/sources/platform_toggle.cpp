/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_toggle.hpp>
#include <GG/adobe/future/widgets/headers/button_helper.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/Button.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

const adobe::toggle_t::image_type& current_image(adobe::toggle_t& toggle)
{
    if (!toggle.control_m->Disabled()) {
        if (toggle.last_m == toggle.value_on_m)
            return toggle.image_on_m;
        else
            return toggle.image_off_m;
    } else { // disabled_button
        return toggle.image_disabled_m;
    }
}

/****************************************************************************************************/
#if 0 // TODO
HBITMAP current_bitmap(adobe::toggle_t& toggle)
{
    if (::IsWindowEnabled(toggle.control_m))
    {
        if (toggle.last_m == toggle.value_on_m)
            return toggle.bitmap_on_m;
        else
            return toggle.bitmap_off_m;
    }
    else // disabled_button
    {
        return toggle.bitmap_disabled_m;
    }
}
#endif

/****************************************************************************************************/

void toggle_clicked(adobe::toggle_t& toggle)
{
    if (toggle.setter_proc_m.empty())
        return;

    // toggle it.
    adobe::any_regular_t new_value =
        toggle.last_m == toggle.value_on_m ?
        adobe::any_regular_t(adobe::empty_t()) :
        toggle.value_on_m;

    toggle.setter_proc_m(new_value);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

toggle_t::toggle_t(const std::string&  alt_text,
                   const any_regular_t value_on,
                   const image_type&   image_on,
                   const image_type&   image_off,
                   const image_type&   image_disabled,
                   theme_t             theme) :
    control_m(0),
    theme_m(theme),
    alt_text_m(alt_text),
    image_on_m(image_on),
    image_off_m(image_off),
    image_disabled_m(image_disabled),
    value_on_m(value_on)
#if 0 // TODO
    ,
    texture_on_m(to_texture(image_on)),
    texture_off_m(to_texture(image_off)),
    texture_disabled_m(to_texture(image_disabled))
#endif
{ }

/****************************************************************************************************/

void toggle_t::measure(extents_t& result)
{
    assert(control_m);

    result = extents_t();

    const adobe::toggle_t::image_type& image(current_image(*this));

    result.height() = static_cast<long>(image.height());
    result.width() = static_cast<long>(image.width());
}

/****************************************************************************************************/

void toggle_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

void toggle_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);
#if 0 // TODO
    ::SendMessage(control_m, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) current_bitmap(*this));
#endif
}

/****************************************************************************************************/

void toggle_t::display(const any_regular_t& to_value)
{
    assert(control_m);

    if (last_m == to_value)
        return;

    last_m = to_value;
#if 0 // TODO
    ::SendMessage(control_m, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) current_bitmap(*this));
#endif
}

/****************************************************************************************************/

void toggle_t::monitor(const setter_type& proc)
{
    assert(control_m);
    setter_proc_m = proc;
}

/****************************************************************************************************/

template <>
platform_display_type insert<toggle_t>(display_t&             display,
                                       platform_display_type& parent,
                                       toggle_t&              element)
{
    assert(!element.control_m);

    assert(!"Cannot create toggle_t's until the texture creation issues are resolved");
#if 0 // TODO
    element.control_m = ::CreateWindowExW(  WS_EX_COMPOSITED | WS_EX_TRANSPARENT, L"STATIC",
                                    NULL,
                                    WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,
                                    0, 0, 100, 20,
                                    parent,
                                    0,
                                    ::GetModuleHandle(NULL),
                                    NULL);

    GG::Connect(element.control_m->ClickedSignal,
                boost::bind(&toggle_clicked, boost::ref(element)));
#endif

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
