/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_popup.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/placeable_concept.hpp>

#include <GG/ClrConstants.h>
#include <GG/DropDownList.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

GG::Y DropHeight(unsigned int lines)
{ return adobe::implementation::RowHeight() * static_cast<int>(lines) + static_cast<int>(2 * GG::ListBox::BORDER_THICK); }

/****************************************************************************************************/

enum metrics {
    gap = 4 // Measured as space from popup to label.
};

/****************************************************************************************************/

void sel_changed_slot(adobe::popup_t& popup, GG::DropDownList::iterator it)
{
    assert(popup.control_m);

    std::size_t new_index(popup.control_m->CurrentItemIndex());

    if (!popup.value_proc_m.empty() || !popup.extended_value_proc_m.empty()) {

        if (popup.custom_m)
            --new_index;

        if (popup.value_proc_m)
            popup.value_proc_m(popup.menu_items_m.at(new_index).find(adobe::key_value)->second);

        if (popup.extended_value_proc_m)
            popup.extended_value_proc_m(popup.menu_items_m.at(new_index).find(adobe::key_value)->second, adobe::modifier_state());
    }

    if (popup.selection_changed_proc_m)
        popup.selection_changed_proc_m(popup.menu_items_m.at(new_index).find(adobe::key_value)->second);
}

/****************************************************************************************************/

void set_menu_item_set(adobe::popup_t& p, const adobe::popup_t::menu_item_t* first, const adobe::popup_t::menu_item_t* last)
{
    p.custom_m = false;

    unsigned int lines = 0;
    for (; first != last; ++first, ++lines)
    {
        // MM: Revisit. Is there a way to have disabled separators in combo boxes?
        // Since I don't know a way I intercept -'s here. (Dashes inidcate separators
        // on the macintosh and also in eve at the moment).

        const adobe::string_t& name = first->find(adobe::key_name)->second.cast<adobe::string_t>();
        if (name != "-" && name != "__separator") 
            p.menu_items_m.push_back(*first);
    }

    if (p.control_m)
        p.control_m->SetDropHeight(DropHeight(lines));
}

/****************************************************************************************************/

void message_menu_item_set(adobe::popup_t& popup)
{
    assert(popup.control_m);

    for (adobe::popup_t::menu_item_set_t::const_iterator
             first = popup.menu_items_m.begin(), last = popup.menu_items_m.end();
         first != last;
         ++first) {
        popup.control_m->Insert(
            adobe::implementation::item_to_row(*first, popup.row_factory_m, popup.item_text_color_m)
        );
    }

    popup.enable(!popup.menu_items_m.empty());

    if (popup.menu_items_m.empty())
        return;

    popup.display(popup.last_m);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

popup_t::popup_t(const std::string& name,
                 const std::string& alt_text,
                 int max_characters,
                 const std::string& custom_item_name,
                 const menu_item_t* first,
                 const menu_item_t* last,
                 GG::Clr            color,
                 GG::Clr            label_color,
                 GG::Clr            item_text_color,
                 name_t             signal_id) :
    control_m(0),
    name_m(name, alt_text, 0, GG::FORMAT_LEFT | GG::FORMAT_TOP, label_color),
    alt_text_m(alt_text),
    max_characters_m(max_characters),
    using_label_m(!name.empty()),
    color_m(color),
    item_text_color_m(item_text_color),
    signal_id_m(signal_id),
    custom_m(false),
    custom_item_name_m(custom_item_name),
    row_factory_m(0)
{ ::set_menu_item_set(*this, first, last); }

/****************************************************************************************************/

void popup_t::measure(extents_t& result)
{
    assert(control_m);

    //
    // The popup_t has multiple text items. We need to find the one with
    // the widest extents (when drawn). Then we can make sure that we get
    // enough space to draw our largest text item.
    //
    const boost::shared_ptr<GG::Font>& font = implementation::DefaultFont();

    menu_item_set_t::iterator first(menu_items_m.begin());
    menu_item_set_t::iterator last(menu_items_m.end());
    GG::Pt largest_extent(font->TextExtent(custom_item_name_m));
    for (; first != last; ++first) {
        GG::Pt extent = font->TextExtent(first->find(key_name)->second.cast<std::string>());
        if (largest_extent.x < extent.x)
            largest_extent = extent;
    }
    largest_extent.x += implementation::CharWidth();
    largest_extent.x = std::min(largest_extent.x, max_characters_m * implementation::CharWidth());

    GG::Pt non_client_size = implementation::NonClientSize(*control_m);

    result.width() = Value(largest_extent.x + non_client_size.x);
    result.height() = original_height_m;
    GG::Y baseline =
        (static_cast<int>(result.height()) - implementation::CharHeight()) / 2 +
        implementation::DefaultFont()->Ascent();
    result.vertical().guide_set_m.push_back(Value(baseline));

    //
    // If we have a label (always on our left side?) then we
    // need to add the size of the label to our result. We try
    // to align the label with the popup by baseline. Which is
    // kind of what Eve does, so it's bad that we do this
    // ourselves, really...
    //
    if (!using_label_m)
        return;
    //
    // We store the height of the label, from this we can
    // figure out how much to offset it when positioning
    // the widgets in set_bounds.
    //
    extents_t label_bounds(measure_text(name_m.name_m, font));
    label_bounds.vertical().guide_set_m.push_back(Value(font->Ascent()));
    //
    // Now we can align the label within the vertical
    // slice of the result. This doesn't do anything if
    // the label is shorter than the popup.
    //
    align_slices(result.vertical(), label_bounds.vertical());
    //
    // Add the width of the label (plus a gap) to the
    // resulting width.
    //
    result.width() += gap + label_bounds.width();

    //
    // Add a point-of-interest where the label ends.
    // We put the label to the left of the popup.
    //
    result.horizontal().guide_set_m.push_back(label_bounds.width());
}

/****************************************************************************************************/

void popup_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    place_data_t local_place_data(place_data);

    if (using_label_m) {
        //
        // The vertical offset of the label is the geometry's
        // baseline - the label's baseline.
        //
        assert(place_data.vertical().guide_set_m.empty() == false);

        place_data_t label_place_data;
        label_place_data.horizontal().position_m = left(place_data);
        label_place_data.vertical().position_m = top(place_data);

        //
        // The width of the label is the first horizontal
        // point of interest.
        //
        assert(place_data.horizontal().guide_set_m.empty() == false);
        width(label_place_data) = place_data.horizontal().guide_set_m[0];
        height(label_place_data) = height(place_data);

        //
        // Set the label dimensions.
        //
        place(name_m, label_place_data);

        //
        // Now we need to adjust the position of the popup
        // widget.
        //
        long width = gap + adobe::width(label_place_data);
        local_place_data.horizontal().position_m += width;
        adobe::width(local_place_data) -= width;
    }

    implementation::set_control_bounds(control_m, local_place_data);
}

/****************************************************************************************************/

void popup_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);

    if (using_label_m) {
        using adobe::enable;
        enable(name_m, make_enabled);
    }
}

/****************************************************************************************************/

void popup_t::set_item_text_color(GG::Clr color)
{
    const bool color_changed = color != item_text_color_m;
    item_text_color_m = color;
    if (color_changed && !menu_items_m.empty()) {
        menu_item_set_t menu_items;
        std::swap(menu_items, menu_items_m);
        reset_menu_item_set(&menu_items.front(), &menu_items.back() + 1);
    }
}

/****************************************************************************************************/

void popup_t::reset_menu_item_set(const menu_item_t* first, const menu_item_t* last)
{
    assert(control_m);
    menu_items_m.clear();
    control_m->Clear();
    ::set_menu_item_set(*this, first, last);
    ::message_menu_item_set(*this);
}

/****************************************************************************************************/

void popup_t::display(const model_type& value)
{
    assert(control_m);

    last_m = value;

    menu_item_set_t::iterator  first(menu_items_m.begin());
    menu_item_set_t::iterator  last(menu_items_m.end());

    for (; first != last; ++first) {
        if (first->find(key_value)->second == value) {
            if (custom_m) {
                custom_m = false;
                control_m->Erase(control_m->begin());
            }
            std::ptrdiff_t index(first - menu_items_m.begin());
            control_m->Select(index);
            return;
        }
    }

    display_custom();
}

/****************************************************************************************************/

void popup_t::display_custom()
{
    if (custom_m)
        return;

    custom_m = true;
    dictionary_t item;
    item[key_name] = any_regular_t(custom_item_name_m);
    item[static_name_t("color")] = any_regular_t(item_text_color_m);
    control_m->Insert(
        implementation::item_to_row(item, row_factory_m, item_text_color_m), control_m->begin()
    );
    control_m->Select(0);
}

/****************************************************************************************************/

void popup_t::select_with_text(const std::string& text)
{
    assert(control_m);

    std::size_t old_index(control_m->CurrentItemIndex());

    std::size_t new_index = std::numeric_limits<std::size_t>::max();
    for (std::size_t i = 0; i < menu_items_m.size(); ++i) {
        if (menu_items_m[i].find(key_name)->second.cast<std::string>().c_str() == text) {
            new_index = i;
            break;
        }
    }

    if (new_index < menu_items_m.size())
        control_m->Select(new_index);

    if (value_proc_m.empty())
        return;

    if (new_index != old_index)
        value_proc_m(menu_items_m.at(new_index).find(adobe::key_value)->second);
}

/****************************************************************************************************/

void popup_t::monitor(const setter_type& proc)
{
    assert(control_m);
    value_proc_m = proc;
}

/****************************************************************************************************/

void popup_t::monitor_extended(const extended_setter_type& proc)
{
    assert(control_m);
    extended_value_proc_m = proc;
}

/****************************************************************************************************/

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/container idiom for event and drawing system.

template <> platform_display_type insert<popup_t>(display_t&             display,
                                                  platform_display_type& parent,
                                                  popup_t&               element)
{
    if (element.using_label_m)
        insert(display, parent, element.name_m);

    assert(!element.control_m);

    int lines = std::min(element.menu_items_m.size(), std::size_t(20u));
    element.control_m =
        implementation::Factory().NewDropDownList(GG::X0, GG::Y0, GG::X1, implementation::StandardHeight(),
                                                  DropHeight(lines), element.color_m);
    element.control_m->SetStyle(GG::LIST_NOSORT);

    element.original_height_m = Value(element.control_m->Height());

    GG::Connect(element.control_m->SelChangedSignal,
                boost::bind(&sel_changed_slot, boost::ref(element), _1));

    if (!element.alt_text_m.empty())
        adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    ::message_menu_item_set(element);

    element.color_proxy_m.initialize(
        boost::bind(&GG::DropDownList::SetColor, element.control_m, _1)
    );
    element.item_text_color_proxy_m.initialize(
        boost::bind(&popup_t::set_item_text_color, &element, _1)
    );

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
