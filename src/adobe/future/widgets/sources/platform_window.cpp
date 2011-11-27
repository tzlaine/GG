/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_window.hpp>

#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/keyboard.hpp>

#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

window_t::window_t(const std::string&     name,
                   GG::Flags<GG::WndFlag> flags,
                   theme_t                theme) :
    window_m(0),
    flags_m(flags),
    name_m(name),
    theme_m(theme),
    debounce_m(false),
    placed_once_m(false)
{ }

/****************************************************************************************************/

window_t::~window_t()
{ delete window_m; }

/****************************************************************************************************/

void window_t::measure(extents_t& result)
{
    assert(window_m);

    if (name_m.empty())
    {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    result = metrics::measure_text(name_m, style->DefaultFont());

    result.width() = static_cast<long>(result.width() * 1.5);
}

/****************************************************************************************************/

void window_t::place(const place_data_t& place_data)
{
    assert(window_m);

    if (placed_once_m)
    {
        set_size(point_2d_t(width(place_data), height(place_data)));
    }
    else
    {
        placed_once_m = true;

        place_data_m = place_data;

        GG::Pt ul(GG::X(left(place_data)), GG::Y(top(place_data)));
        GG::Pt size(
            GG::Pt(GG::X(width(place_data)), GG::Y(height(place_data))) +
            implementation::NonClientSize(*window_m)
        );

        window_m->SetMinSize(size);

        window_m->SizeMove(ul, ul + size);
    }
}

/****************************************************************************************************/

void window_t::set_size(const point_2d_t& size)
{
    assert(window_m);

    if (debounce_m)
        return;

    debounce_m = true;

    width(place_data_m) = size.x_m;
    height(place_data_m) = size.y_m;

    window_m->Resize(
        GG::Pt(GG::X(width(place_data_m)), GG::Y(height(place_data_m))) +
        implementation::NonClientSize(*window_m)
    );

    debounce_m = false;
}

/****************************************************************************************************/

void window_t::reposition()
{
    assert(window_m);

    const GG::X width(window_m->Width());
    const GG::Y height(window_m->Height());

    GG::X app_width(GG::GUI::GetGUI()->AppWidth());
    GG::Y app_height(GG::GUI::GetGUI()->AppHeight());
    
    GG::X left(std::max(GG::X(10), (app_width - width) / 2));
    GG::Y top(std::max(GG::Y(10), (app_height - height) / 2));

    window_m->MoveTo(GG::Pt(left, top));
}

/****************************************************************************************************/

void window_t::set_visible(bool make_visible)
{
    assert(window_m);

    if (!window_m->Visible())
        reposition();

    // It's necessary not to recursively show or hide children here, as it
    // messes up the operation of panel_t.
    if (make_visible)
        window_m->Show(false);
    else
        window_m->Hide(false);
}

/****************************************************************************************************/

void window_t::monitor_resize(const window_resize_proc_t& proc)
{ resize_proc_m = proc; }

/****************************************************************************************************/

any_regular_t window_t::underlying_handler()
{ return any_regular_t(window_m); }

/****************************************************************************************************/

bool window_t::handle_key(key_type key, bool pressed, modifiers_t modifiers)
{ return false; }

/****************************************************************************************************/

template <>
platform_display_type insert<window_t>(display_t&             display,
                                       platform_display_type& parent,
                                       window_t&              element)
{
    assert(!element.window_m);
    assert(!parent);

    element.window_m = new GG::Window(element);

    return display.insert(parent, element.window_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
