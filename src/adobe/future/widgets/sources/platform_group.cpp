/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_group.hpp>

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

group_t::group_t(const std::string& name,
                 const std::string& alt_text,
                 theme_t            theme) :
    control_m(0),
    name_m(name),
    alt_text_m(alt_text),
    theme_m(theme)
{ }

/****************************************************************************************************/

void group_t::measure(extents_t& result)
{
    assert(control_m);

    if (name_m.empty())
    {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    result = metrics::measure_text(name_m, style->DefaultFont());

    result.width() += 15;

    result.vertical().frame_m.first = result.height() + 7;

    result.height() = 5;
}

/****************************************************************************************************/

void group_t::place(const place_data_t& place_data)
{
    assert(control_m);
    implementation::set_control_bounds(control_m, place_data);
}

/****************************************************************************************************/

template <>
platform_display_type insert<group_t>(display_t&             display,
                                      platform_display_type& parent,
                                      group_t&               element)
{
    assert(!"GG has no group control yet");
#if 0
    HWND parent_hwnd(parent);

    element.control_m = ::CreateWindowExW(WS_EX_CONTROLPARENT | WS_EX_COMPOSITED | WS_EX_TRANSPARENT,
                                          L"BUTTON",
                                          hackery::convert_utf(element.name_m).c_str(),
                                          WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                                          0, 0, 10, 10,
                                          parent_hwnd,
                                          0,
                                          ::GetModuleHandle(NULL),
                                          NULL);
#endif

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
