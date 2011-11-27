/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/algorithm/find_match.hpp>

#include <GG/adobe/future/widgets/headers/platform_tab_group.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void tab_changed(adobe::tab_group_t& tab_group, std::size_t index)
{
    if (!tab_group.value_proc_m.empty())
        tab_group.value_proc_m(tab_group.items_m[index].value_m);
}

/****************************************************************************************************/

class Placeholder :
    public GG::Wnd
{
public:
    Placeholder() : Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::Flags<GG::WndFlag>()) {}
};

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

tab_group_t::tab_group_t(const tab_t* first,
                         const tab_t* last,
                         theme_t theme) :
    control_m(NULL),
    theme_m(theme),
    value_proc_m(),
    items_m(first, last)
{}

/****************************************************************************************************/

void tab_group_t::measure(extents_t& result)
{
    assert(control_m);

    // REVISIT (fbrereto) : A lot of static metrics values added here

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    for (tab_set_t::iterator first(items_m.begin()), last(items_m.end());
         first != last;
         ++first) {
        GG::X text_width = style->DefaultFont()->TextExtent(first->name_m).x;
        result.width() += Value(text_width) + 18;
    }

    result.height() = Value(control_m->MinUsableSize().y);

    result.vertical().frame_m.first = result.height() + 7;

    result.height() = 5;
}

/****************************************************************************************************/

void tab_group_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

void tab_group_t::display(const any_regular_t& new_value)
{
    assert(control_m);

    tab_set_t::iterator it =
        find_match(items_m, new_value,
                   compare_members(&tab_t::value_m, std::equal_to<any_regular_t>()));

    if (it != items_m.end())
        tab_bar_m->SetCurrentTab(it - items_m.begin());
}

/****************************************************************************************************/

void tab_group_t::monitor(const tab_group_value_proc_t& proc)
{
    assert(control_m);
    if (!value_proc_m)
        value_proc_m = proc;
}

/****************************************************************************************************/

void tab_group_t::enable(bool make_enabled)
{
    assert(control_m);
    tab_bar_m->Disable(!make_enabled);
}

/****************************************************************************************************/

template <>
platform_display_type insert<tab_group_t>(display_t&             display,
                                          platform_display_type& parent,
                                          tab_group_t&           element)
{
    assert (!element.control_m);

    // This probably looks odd.  The interface of tab groups in Eve is wildly
    // different from that of GG's TabWnd.  The most expedient way to get all
    // the Wnds in the right styles and still match the Eve interface was to
    // replace the built-in TabBar inside of TabWnd with one created here.

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    element.tab_bar_m = style->NewTabBar(GG::X0, GG::Y0, GG::X(100),
                                         style->DefaultFont(), GG::CLR_GRAY);
    element.tab_bar_m->SetMinSize(GG::Pt(element.tab_bar_m->MinSize().x, element.tab_bar_m->Height()));

    element.control_m = style->NewTabWnd(GG::X0, GG::Y0, GG::X(100), element.tab_bar_m->Height() + 20,
                                         style->DefaultFont(), GG::CLR_GRAY);

    GG::Layout* layout = element.control_m->DetachLayout();
    delete layout;

    layout = new GG::Layout(GG::X0, GG::Y0,
                            element.control_m->Width(), element.control_m->Height(),
                            2, 1);
    layout->SetRowStretch(1, 1.0);
    layout->Add(element.tab_bar_m, 0, 0);
    layout->Add(new Placeholder, 1, 0);
    element.control_m->SetLayout(layout);

    GG::Connect(element.tab_bar_m->TabChangedSignal,
                boost::bind(&tab_changed, boost::ref(element), _1));

    for (tab_group_t::tab_set_t::iterator
             first(element.items_m.begin()), last(element.items_m.end());
         first != last;
         ++first) {
        element.tab_bar_m->AddTab(first->name_m);
    }

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} //namespace adobe
