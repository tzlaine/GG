/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_link.hpp>
#include <GG/adobe/algorithm/copy.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <GG/Control.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

#if 0 // TODO
HBITMAP link_bitmap()
{
    static HBITMAP bitmap_s(0);

    if (bitmap_s == 0)
    {
        boost::gil::rgba8_image_t image;

        adobe::image_slurp("link_icon.tga", image);

        bitmap_s = adobe::to_bitmap(image);
    }

    return bitmap_s;
}
#endif

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

link_t::link_t(const std::string&   alt_text,
               const any_regular_t& on_value,
               const any_regular_t& off_value,
               long                 count,
               theme_t              theme) :
    control_m(0),
    link_icon_m(0),
    alt_text_m(alt_text),
    on_value_m(on_value),
    off_value_m(off_value),
    count_m(count),
    theme_m(theme)
{ }

/****************************************************************************************************/

void link_t::measure(extents_t& result)
{
    result.width() = 15;
    result.height() = 5;

    for (long i(0); i < count_m; ++i)
        result.vertical().guide_set_m.push_back(2);
}

/****************************************************************************************************/

void link_t::place(const place_data_t& place_data)
{
    assert(control_m);

    tl_m.x_m = left(place_data);
    tl_m.y_m = top(place_data);

    implementation::set_control_bounds(control_m, place_data);

    prongs_m.erase(prongs_m.begin(), prongs_m.end());

    copy(place_data.vertical().guide_set_m, std::back_inserter(prongs_m));
}

/****************************************************************************************************/

void link_t::enable(bool make_enabled)
{
    assert(control_m);
    
    control_m->Disable(!make_enabled);

    bool do_visible(value_m == on_value_m && make_enabled);

    set_control_visible(link_icon_m, do_visible);
    set_control_visible(control_m, do_visible);
}

/****************************************************************************************************/

void link_t::display(const any_regular_t& new_value)
{
    assert(control_m);
    
    value_m = new_value;

    bool do_visible(value_m == on_value_m && !control_m->Disabled());

    set_control_visible(link_icon_m, do_visible);
    set_control_visible(control_m, do_visible);
}

/****************************************************************************************************/

void link_t::monitor(const setter_type& proc)
{
    assert(control_m);

    hit_proc_m = proc;
}

/****************************************************************************************************/

template <>
platform_display_type insert<link_t>(display_t&             display,
                                     platform_display_type& parent,
                                     link_t&                element)
{
    assert(!element.control_m);

    assert(!"Cannot create link_t's until the texture loading issue is resolved");
#if 0 // TODO
    element.control_m = ::CreateWindowExW(WS_EX_COMPOSITED, L"STATIC",
                                          NULL,
                                          WS_CHILD | WS_VISIBLE,
                                          0, 0, 50, 50,
                                          parent_hwnd,
                                          0,
                                          ::GetModuleHandle(NULL),
                                          NULL);
#endif

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

#if 0 // TODO
    element.link_icon_m = ::CreateWindowExW(WS_EX_COMPOSITED | WS_EX_TRANSPARENT, L"STATIC",
                                            NULL,
                                            WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,
                                            0, 0, 9, 16,
                                            parent_hwnd,
                                            0,
                                            ::GetModuleHandle(NULL),
                                            NULL);

    ::SendMessage(element.link_icon_m, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) link_bitmap());
#endif

    display.insert(parent, element.link_icon_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
