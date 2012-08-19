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
#include <GG/DropDownList.h>
#include <GG/ListBox.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


namespace {

    adobe::aggregate_name_t key_state = {"state"};

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

    struct notify_row_change
    {
        notify_row_change(adobe::listbox_t& listbox, GG::ListBox::iterator it) :
            m_listbox(listbox),
            m_it(it)
            {}

        void operator()() const
            {
                std::size_t i = std::distance(m_listbox.control_m->begin(), m_it);
                adobe::dictionary_t& item = m_listbox.items_m[i].cast<adobe::dictionary_t>();
                adobe::any_regular_t& state = item[key_state];
                adobe::any_regular_t row_value = adobe::listbox_t::row_value(**m_it);
                if (state != row_value) {
                    state = row_value;
                    (*m_it)->SetItem(item);
                    if (m_listbox.item_set_view_controller_m.value_proc_m)
                        m_listbox.item_set_view_controller_m.value_proc_m(m_listbox.items_m);
                }
            }

        adobe::listbox_t& m_listbox;
        GG::ListBox::iterator m_it;
    };

    struct notifier_handle
    {
        typedef void result_type;

        notifier_handle(const boost::shared_ptr<notify_row_change>& notifier) :
            m_notifier(notifier)
            {}

        void operator()() const
            { (*m_notifier)(); }

        boost::shared_ptr<notify_row_change> m_notifier;
    };

    void connect_row_change_signals(adobe::listbox_t& listbox, GG::ListBox::iterator it)
    {
        boost::shared_ptr<notify_row_change> notifier(new notify_row_change(listbox, it));
        const std::size_t size = (*it)->size();
        for (std::size_t i = 0; i < size; ++i) {
            GG::Control* element = (**it)[i];
            if (const GG::DropDownList* drop_list = dynamic_cast<const GG::DropDownList*>(element))
                GG::Connect(drop_list->SelChangedSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::Edit* edit = dynamic_cast<const GG::Edit*>(element))
                GG::Connect(edit->FocusUpdateSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::Slider<double>* double_slider = dynamic_cast<const GG::Slider<double>*>(element))
                GG::Connect(double_slider->SlidAndStoppedSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::Slider<int>* int_slider = dynamic_cast<const GG::Slider<int>*>(element))
                GG::Connect(int_slider->SlidAndStoppedSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::Spin<double>* double_spin = dynamic_cast<const GG::Spin<double>*>(element))
                GG::Connect(double_spin->ValueChangedSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::Spin<int>* int_spin = dynamic_cast<const GG::Spin<int>*>(element))
                GG::Connect(int_spin->ValueChangedSignal, boost::bind(notifier_handle(notifier)));
            else if (const GG::StateButton* state_button = dynamic_cast<const GG::StateButton*>(element))
                GG::Connect(state_button->CheckedSignal, boost::bind(notifier_handle(notifier)));
        }
    }

    void message_item_set(adobe::listbox_t& listbox)
    {
        assert(listbox.control_m);

        for (adobe::listbox_t::item_set_t::iterator
                 first = listbox.items_m.begin(), last = listbox.items_m.end();
             first != last;
             ++first) {
            adobe::dictionary_t& item = first->cast<adobe::dictionary_t>();
            GG::ListBox::Row* row =
                adobe::implementation::item_to_row(item,
                                                   listbox.row_factory_m,
                                                   listbox.item_text_color_m);
            GG::ListBox::iterator it = listbox.control_m->Insert(row);
            if (item.count(key_state))
                adobe::listbox_t::populate_row_values(*row, item[key_state].cast<adobe::array_t>());
            else
                item[key_state] = adobe::listbox_t::row_value(*row);
            connect_row_change_signals(listbox, it);
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

    adobe::any_regular_t to_any_regular(const GG::Button& button)
    { return adobe::any_regular_t(button.Text()); }

    adobe::any_regular_t to_any_regular(const GG::DropDownList& drop_list)
    { return adobe::listbox_t::row_value(drop_list.CurrentItem(), drop_list.end()); }

    adobe::any_regular_t to_any_regular(const GG::Edit& edit)
    { return adobe::any_regular_t(edit.Text()); }

    template <typename T>
    adobe::any_regular_t to_any_regular(const GG::Slider<T>& slider)
    { return adobe::any_regular_t(slider.Posn()); }

    template <typename T>
    adobe::any_regular_t to_any_regular(const GG::Spin<T>& spin)
    { return adobe::any_regular_t(spin.Value()); }

    adobe::any_regular_t to_any_regular(const GG::StateButton& button)
    { return adobe::any_regular_t(button.Checked()); }

    adobe::any_regular_t to_any_regular(const GG::StaticGraphic& graphic)
    { return adobe::any_regular_t(graphic.GetSubTexture()); }

    adobe::any_regular_t to_any_regular(const GG::TextControl& text_control)
    { return adobe::any_regular_t(text_control.RawText()); }

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

        const bool value_is_array = value.type_info() == type_info<array_t>();

        array_t single_value;
        if (!value_is_array)
            single_value.push_back(value);
        const array_t& values = value_is_array ? value.cast<array_t>() : single_value;

        control_m->DeselectAll();

        for (array_t::const_iterator it = values.begin(), end_it = values.end(); it != end_it; ++it) {
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

    any_regular_t listbox_t::row_value(GG::ListBox::const_iterator it, GG::ListBox::const_iterator end_it)
    { return it == end_it ? any_regular_t(array_t()) : row_value(**it); }

    any_regular_t listbox_t::row_value(const GG::ListBox::Row& row)
    {
        array_t elements;

        const std::size_t size = row.size();
        for (std::size_t i = 0; i < size; ++i) {
            GG::Control* element = row[i];
            if (const GG::Button* button = dynamic_cast<const GG::Button*>(element))
                elements.push_back(to_any_regular(*button));
            else if (const GG::DropDownList* drop_list = dynamic_cast<const GG::DropDownList*>(element))
                elements.push_back(to_any_regular(*drop_list));
            else if (const GG::Edit* edit = dynamic_cast<const GG::Edit*>(element))
                elements.push_back(to_any_regular(*edit));
            else if (const GG::Slider<double>* double_slider = dynamic_cast<const GG::Slider<double>*>(element))
                elements.push_back(to_any_regular(*double_slider));
            else if (const GG::Slider<int>* int_slider = dynamic_cast<const GG::Slider<int>*>(element))
                elements.push_back(to_any_regular(*int_slider));
            else if (const GG::Spin<double>* double_spin = dynamic_cast<const GG::Spin<double>*>(element))
                elements.push_back(to_any_regular(*double_spin));
            else if (const GG::Spin<int>* int_spin = dynamic_cast<const GG::Spin<int>*>(element))
                elements.push_back(to_any_regular(*int_spin));
            else if (const GG::StateButton* state_button = dynamic_cast<const GG::StateButton*>(element))
                elements.push_back(to_any_regular(*state_button));
            else if (const GG::StaticGraphic* static_graphic = dynamic_cast<const GG::StaticGraphic*>(element))
                elements.push_back(to_any_regular(*static_graphic));
            else if (const GG::TextControl* text_control = dynamic_cast<const GG::TextControl*>(element))
                elements.push_back(to_any_regular(*text_control));
            else
                elements.push_back(any_regular_t());
        }

        return any_regular_t(elements);
    }

    void listbox_t::populate_row_values(GG::ListBox::Row& row, array_t& state)
    {
        assert(row.size() == state.size());
        const std::size_t size = row.size();
        for (std::size_t i = 0; i < size; ++i) {
            GG::Control* element = row[i];
            if (GG::Button* button = dynamic_cast<GG::Button*>(element))
                button->SetText(state[i].cast<std::string>());
            else if (GG::DropDownList* drop_list = dynamic_cast<GG::DropDownList*>(element)) {
                for (GG::DropDownList::iterator it = drop_list->begin(); it != drop_list->end(); ++it) {
                    if (row_value(**it) == state[i]) {
                        drop_list->Select(it);
                        break;
                    }
                }
            } else if (GG::Edit* edit = dynamic_cast<GG::Edit*>(element))
                edit->SetText(state[i].cast<std::string>());
            else if (GG::Slider<double>* double_slider = dynamic_cast<GG::Slider<double>*>(element))
                double_slider->SlideTo(state[i].cast<double>());
            else if (GG::Slider<int>* int_slider = dynamic_cast<GG::Slider<int>*>(element))
                int_slider->SlideTo(static_cast<int>(state[i].cast<double>()));
            else if (GG::Spin<double>* double_spin = dynamic_cast<GG::Spin<double>*>(element))
                double_spin->SetValue(state[i].cast<double>());
            else if (GG::Spin<int>* int_spin = dynamic_cast<GG::Spin<int>*>(element))
                int_spin->SetValue(static_cast<int>(state[i].cast<double>()));
            else if (GG::StateButton* state_button = dynamic_cast<GG::StateButton*>(element))
                state_button->SetCheck(state[i].cast<bool>());
            else if (GG::TextControl* text_control = dynamic_cast<GG::TextControl*>(element))
                text_control->SetText(state[i].cast<std::string>());
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
            implementation::set_control_alt_text(element.control_m, element.alt_text_m);

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
