/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2011 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include "platform_listbox.hpp"

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/ClrConstants.h>
#include <GG/GUI.h>
#include <GG/ListBox.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


namespace {

    GG::Y Height(unsigned int lines)
    { return adobe::implementation::RowHeight() * static_cast<int>(lines) + static_cast<int>(2 * GG::ListBox::BORDER_THICK); }

    enum metrics {
        gap = 4 // Measured as space from listbox to label.
    };

    void clear_items(adobe::listbox_t& control)
    {
        assert(control.control_m);
        control.items_m.clear();
        control.control_m->Clear();
    }

    void listbox_selection_changed(adobe::listbox_t& listbox, const GG::ListBox::SelectionSet& selections)
    {
        assert(listbox.control_m);

        if (listbox.value_proc_m) {
            adobe::array_t array;
            for (GG::ListBox::SelectionSet::const_iterator it = selections.begin(), end_it = selections.end();
                 it != end_it;
                 ++it) {
                std::ptrdiff_t index = std::distance(listbox.control_m->begin(), *it);
                array.push_back(
                    listbox.items_m[index].cast<adobe::dictionary_t>().find(adobe::static_name_t("value"))->second
                );
            }
            listbox.value_proc_m(listbox.debounce_m = adobe::any_regular_t(array));
        }

        if (listbox.selection_changed_proc_m)
            listbox.selection_changed_proc_m(listbox, selections);
    }

    void listbox_dropped(adobe::listbox_t& listbox, GG::ListBox::iterator row)
    {
        const bool internal_drag_and_drop =
            std::count(listbox.control_m->begin(), listbox.control_m->end(), *row) == 2u;
        GG::ListBox::iterator other_row =
            internal_drag_and_drop ?
            std::find(listbox.control_m->begin(), row, *row) :
            listbox.control_m->end();
        const bool dropped_before_self = other_row == row;
        if (internal_drag_and_drop && dropped_before_self) {
            other_row = std::find(++other_row, listbox.control_m->end(), *row);
            assert(other_row != row);
            assert(other_row != listbox.control_m->end());
        }
        const std::size_t i = std::distance(listbox.control_m->begin(), row);
        listbox.items_m.insert(listbox.items_m.begin() + i, adobe::any_regular_t((*row)->Item()));
        if (internal_drag_and_drop) {
            std::size_t old_i = std::distance(listbox.control_m->begin(), other_row);
            listbox.items_m.erase(listbox.items_m.begin() + old_i);
        }
        if (listbox.item_set_view_controller_m.value_proc_m)
            listbox.item_set_view_controller_m.value_proc_m(listbox.items_m);
        if (listbox.dropped_proc_m)
            listbox.dropped_proc_m(listbox, row);
    }

    void listbox_drop_acceptable(adobe::listbox_t& listbox, const GG::ListBox::Row& row, GG::ListBox::const_iterator it)
    {
        if (listbox.drop_acceptable_proc_m)
            listbox.drop_acceptable_proc_m(listbox, row, it);
    }

    void listbox_left_clicked(adobe::listbox_t& listbox, GG::ListBox::iterator row, const GG::Pt& pt)
    {
        if (listbox.left_clicked_proc_m)
            listbox.left_clicked_proc_m(listbox, row, pt);
    }

    void listbox_right_clicked(adobe::listbox_t& listbox, GG::ListBox::iterator row, const GG::Pt& pt)
    {
        if (listbox.right_clicked_proc_m)
            listbox.right_clicked_proc_m(listbox, row, pt);
    }

    void listbox_double_clicked(adobe::listbox_t& listbox, GG::ListBox::iterator row)
    {
        if (listbox.double_clicked_proc_m)
            listbox.double_clicked_proc_m(listbox, row);
    }

    void listbox_erased(adobe::listbox_t& listbox, GG::ListBox::iterator row)
    {
        const std::size_t i = std::distance(listbox.control_m->begin(), row);
        listbox.items_m.erase(listbox.items_m.begin() + i);
        if (listbox.item_set_view_controller_m.value_proc_m)
            listbox.item_set_view_controller_m.value_proc_m(listbox.items_m);
        if (listbox.erased_proc_m)
            listbox.erased_proc_m(listbox, row);
    }

    void listbox_browsed(adobe::listbox_t& listbox, GG::ListBox::iterator row)
    {
        if (listbox.browsed_proc_m)
            listbox.browsed_proc_m(listbox, row);
    }

    void message_item_set(adobe::listbox_t& listbox)
    {
        assert(listbox.control_m);

        for (adobe::listbox_t::item_set_t::const_iterator
                 first = listbox.items_m.begin(), last = listbox.items_m.end();
             first != last;
             ++first) {
            listbox.control_m->Insert(
                adobe::implementation::item_to_row(first->cast<adobe::dictionary_t>(),
                                                   listbox.row_factory_m,
                                                   listbox.item_text_color_m)
            );
        }
    }

    void sort_items(adobe::listbox_t& listbox)
    {
        if (listbox.style_m & GG::LIST_NOSORT)
            return;

        typedef std::map<std::string, adobe::dictionary_t*> sorted_item_map;
        sorted_item_map sorted_items;
        adobe::listbox_t::item_set_t sorted_item_set(
            listbox.items_m.size(),
            adobe::any_regular_t(adobe::dictionary_t())
        );
        for (adobe::listbox_t::item_set_t::iterator
                 it = listbox.items_m.begin(), end_it = listbox.items_m.end();
             it != end_it;
             ++it) {
            std::string name;
            adobe::dictionary_t& d = it->cast<adobe::dictionary_t>();
            adobe::implementation::get_localized_string(d, adobe::key_name, name);
            sorted_items[name] = &d;
        }
        std::size_t i = 0;
        if (listbox.style_m & GG::LIST_SORTDESCENDING) {
            for (sorted_item_map::reverse_iterator
                     it = sorted_items.rbegin(), end_it = sorted_items.rend();
                 it != end_it;
                 ++it) {
                std::swap(sorted_item_set[i++].cast<adobe::dictionary_t>(), *it->second);
            }
        } else {
            for (sorted_item_map::iterator
                     it = sorted_items.begin(), end_it = sorted_items.end();
                 it != end_it;
                 ++it) {
                std::swap(sorted_item_set[i++].cast<adobe::dictionary_t>(), *it->second);
            }
        }
        std::swap(listbox.items_m, sorted_item_set);
    }

}

namespace adobe {

    listbox_t::item_set_view_controller_t::item_set_view_controller_t() :
        listbox_m(0)
    {}

    void listbox_t::item_set_view_controller_t::initialize(listbox_t& listbox)
    { listbox_m = &listbox; }

    void listbox_t::item_set_view_controller_t::display(const model_type& value)
    {
        assert(listbox_m);
        if (value != listbox_m->items_m)
            listbox_m->reset_item_set(value);
    }

    void listbox_t::item_set_view_controller_t::monitor(const setter_type& proc)
    { value_proc_m = proc; }

    void listbox_t::item_set_view_controller_t::enable(bool)
    {}

    listbox_t::listbox_t(const std::string& name,
                         const std::string& alt_text,
                         int characters,
                         int rows,
                         GG::Flags<GG::ListBoxStyle> style,
                         const item_set_t& items,
                         GG::Clr color,
                         GG::Clr interior_color,
                         GG::Clr hilite_color,
                         GG::Clr label_color,
                         GG::Clr item_text_color,
                         const std::vector<std::string>& drop_types,
                         name_t signal_id) :
        control_m(0),
        name_m(name, alt_text, 0, GG::FORMAT_LEFT | GG::FORMAT_TOP, label_color),
        alt_text_m(alt_text),
        using_label_m(!name.empty()),
        items_m(items),
        characters_m(characters),
        rows_m(rows),
        style_m(style),
        color_m(color),
        interior_color_m(interior_color),
        hilite_color_m(hilite_color),
        item_text_color_m(item_text_color),
        drop_types_m(drop_types),
        signal_id_m(signal_id),
        row_factory_m(0)
    { ::sort_items(*this); }

    void listbox_t::measure(extents_t& result)
    {
        assert(control_m);

        //
        // The listbox_t has multiple text items. We need to find the one with
        // the widest extents (when drawn). Then we can make sure that we get
        // enough space to draw our largest text item.
        //
        const boost::shared_ptr<GG::Font>& font = implementation::DefaultFont();

        GG::X width = implementation::CharWidth() * characters_m;

        GG::Pt non_client_size = implementation::NonClientSize(*control_m);

        result.width() = Value(width + non_client_size.x);
        result.height() = original_height_m;
        GG::Y baseline_offset =
            control_m->ClientUpperLeft().y - control_m->UpperLeft().y + 2; // the +2 comes from the default ListBox::Row margin
        result.vertical().guide_set_m.push_back(Value(baseline_offset));

        //
        // If we have a label (always on our left side?) then we
        // need to add the size of the label to our result. We try
        // to align the label with the listbox by baseline. Which is
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
        label_bounds.vertical().guide_set_m.push_back(0);
        //
        // Now we can align the label within the vertical
        // slice of the result. This doesn't do anything if
        // the label is shorter than the listbox.
        //
        align_slices(result.vertical(), label_bounds.vertical());
        //
        // Add the width of the label (plus a gap) to the
        // resulting width.
        //
        result.width() += gap + label_bounds.width();

        //
        // Add a point-of-interest where the label ends.
        // We put the label to the left of the listbox.
        //
        result.horizontal().guide_set_m.push_back(label_bounds.width());
    }

    void listbox_t::place(const place_data_t& place_data)
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
            long baseline = place_data.vertical().guide_set_m[0];

            place_data_t label_place_data;
            label_place_data.horizontal().position_m = left(place_data);
            label_place_data.vertical().position_m = top(place_data) + baseline;

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
            // Now we need to adjust the position of the listbox
            // widget.
            //
            long width = gap + adobe::width(label_place_data);
            local_place_data.horizontal().position_m += width;
            adobe::width(local_place_data) -= width;
        }

        implementation::set_control_bounds(control_m, local_place_data);
    }

    void listbox_t::enable(bool make_enabled)
    {
        assert(control_m);
        control_m->Disable(!make_enabled);
    }

    void listbox_t::reset_item_set(const array_t& items)
    {
        assert(control_m);
        items_m = items;
        for (array_t::iterator it = items_m.begin(), end_it = items_m.end();
             it != end_it;
             ++it) {
            get_value(it->cast<dictionary_t>(), key_value);
        }
        control_m->Clear();
        ::sort_items(*this);
        ::message_item_set(*this);
    }

    void listbox_t::display(const model_type& value)
    {
        assert(control_m);

        if (value == debounce_m)
            return;

        const bool value_is_array = value.type_info() == adobe::type_info<adobe::array_t>();

        adobe::array_t single_value;
        if (!value_is_array)
            single_value.push_back(value);
        const adobe::array_t& values = value_is_array ? value.cast<adobe::array_t>() : single_value;

        control_m->DeselectAll();

        for (adobe::array_t::const_iterator it = values.begin(), end_it = values.end(); it != end_it; ++it) {
            GG::ListBox::iterator row_it = control_m->begin();
            for (item_set_t::iterator first = items_m.begin(), last = items_m.end();
                 first != last;
                 ++first, ++row_it) {
                if (first->cast<dictionary_t>().find(static_name_t("value"))->second == value) {
                    control_m->SelectRow(row_it);
                    break;
                }
            }
        }
    }

    void listbox_t::monitor(const setter_type& proc)
    {
        assert(control_m);
        value_proc_m = proc;
    }

    void listbox_t::set_item_text_color(GG::Clr color)
    {
        const bool color_changed = color != item_text_color_m;
        item_text_color_m = color;
        if (color_changed && !items_m.empty()) {
            item_set_t items;
            std::swap(items, items_m);
            reset_item_set(items);
        }
    }

    template <>
    platform_display_type insert<listbox_t>(display_t& display,
                                            platform_display_type& parent,
                                            listbox_t& element)
    {
        if (element.using_label_m)
            insert(display, parent, element.name_m);

        assert(!element.control_m);

        int lines = std::min(element.items_m.size(), std::size_t(20u));
        if (1 < element.rows_m)
            lines = element.rows_m;
        element.control_m =
            implementation::Factory().NewListBox(GG::X0, GG::Y0, GG::X1, Height(lines),
                                                 element.color_m, element.interior_color_m);
        element.control_m->SetHiliteColor(element.hilite_color_m);
        element.control_m->SetStyle(element.style_m);

        element.original_height_m = Value(element.control_m->Height());

        for (std::size_t i = 0; i < element.drop_types_m.size(); ++i) {
            element.control_m->AllowDropType(element.drop_types_m[i]);
        }

        element.item_set_view_controller_m.initialize(element);

        GG::Connect(element.control_m->SelChangedSignal,
                    boost::bind(&listbox_selection_changed, boost::ref(element), _1));

        GG::Connect(element.control_m->DroppedSignal,
                    boost::bind(&listbox_dropped, boost::ref(element), _1));

        GG::Connect(element.control_m->DropAcceptableSignal,
                    boost::bind(&listbox_drop_acceptable, boost::ref(element), _1, _2));

        GG::Connect(element.control_m->LeftClickedSignal,
                    boost::bind(&listbox_left_clicked, boost::ref(element), _1, _2));

        GG::Connect(element.control_m->RightClickedSignal,
                    boost::bind(&listbox_right_clicked, boost::ref(element), _1, _2));

        GG::Connect(element.control_m->DoubleClickedSignal,
                    boost::bind(&listbox_double_clicked, boost::ref(element), _1));

        GG::Connect(element.control_m->ErasedSignal,
                    boost::bind(&listbox_erased, boost::ref(element), _1));

        GG::Connect(element.control_m->BrowsedSignal,
                    boost::bind(&listbox_browsed, boost::ref(element), _1));

        if (!element.alt_text_m.empty())
            adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

        ::message_item_set(element);

        element.color_proxy_m.initialize(
            boost::bind(&GG::ListBox::SetColor, element.control_m, _1)
        );
        element.interior_color_proxy_m.initialize(
            boost::bind(&GG::ListBox::SetInteriorColor, element.control_m, _1)
        );
        element.hilite_color_proxy_m.initialize(
            boost::bind(&GG::ListBox::SetHiliteColor, element.control_m, _1)
        );
        element.item_text_color_proxy_m.initialize(
            boost::bind(&listbox_t::set_item_text_color, &element, _1)
        );

        return display.insert(parent, element.control_m);
    }

}
