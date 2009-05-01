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

#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

class LinkTextFilter :
    public GG::Wnd
{
public:
    LinkTextFilter(link_t& link) : m_link(link) {}

    virtual bool EventFilter(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::LClick) {
                m_link.hit_proc_m(m_link.off_value_m);
                retval = true;
            }
            return retval;
        }

    link_t& m_link;
};

} // namespace implementation

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

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();

    // TODO: This looks wrong when compared to the windows platform version of link_t.

    element.control_m =
        style->NewTextControl(GG::X0, GG::Y0, GG::X(100), GG::Y(100),
                              "", style->DefaultFont(),
                              GG::CLR_BLACK, GG::FORMAT_NONE, GG::CLICKABLE);
    element.filter_m.reset(new adobe::implementation::LinkTextFilter(element));
    element.control_m->InstallEventFilter(element.filter_m.get());

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    assert(!element.link_icon_m);

    boost::shared_ptr<GG::Texture> link_texture =
        GG::GUI::GetGUI()->GetTexture("link_icon.png");
    element.link_icon_m =
        style->NewStaticGraphic(
            GG::X0, GG::Y0,
            link_texture->DefaultWidth(), link_texture->DefaultHeight(),
            link_texture
        );

    display.insert(parent, element.link_icon_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
