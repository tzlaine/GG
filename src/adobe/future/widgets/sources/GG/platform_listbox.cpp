/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

#include "platform_listbox.hpp"

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
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
            adobe::any_regular_t value;
            if (selections.size() == 1u) {
                std::ptrdiff_t index = std::distance(listbox.control_m->begin(), *selections.begin());
                value = listbox.items_m.at(index).find(adobe::static_name_t("value"))->second;
            } else {
                value.assign(adobe::array_t());
                adobe::array_t& array = value.cast<adobe::array_t>();
                for (GG::ListBox::SelectionSet::const_iterator it = selections.begin(), end_it = selections.end();
                     it != end_it;
                     ++it) {
                    std::ptrdiff_t index = std::distance(listbox.control_m->begin(), *it);
                    array.push_back(listbox.items_m.at(index).find(adobe::static_name_t("value"))->second);
                }
            }
            listbox.value_proc_m(value);
        }

        if (listbox.selection_changed_proc_m)
            listbox.selection_changed_proc_m(listbox, selections);
    }

    void listbox_dropped(adobe::listbox_t& listbox, GG::ListBox::iterator row)
    {
        if (listbox.dropped_proc_m)
            listbox.dropped_proc_m(listbox, row);
    }

    void listbox_drop_acceptable(adobe::listbox_t& listbox, GG::ListBox::const_iterator row)
    {
        if (listbox.drop_acceptable_proc_m)
            listbox.drop_acceptable_proc_m(listbox, row);
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
            listbox.control_m->Insert(adobe::implementation::item_to_row(*first, listbox.item_text_color_m));
        }
    }

}

namespace adobe {

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
        signal_id_m(signal_id)
    {}

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
        GG::Y baseline =
            (static_cast<int>(result.height()) - implementation::CharHeight()) / 2 +
            implementation::DefaultFont()->Ascent();
        result.vertical().guide_set_m.push_back(Value(baseline));

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
        label_bounds.vertical().guide_set_m.push_back(Value(font->Ascent()));
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

    void listbox_t::reset_item_set(const item_t* first, const item_t* last)
    {
        assert(control_m);
        clear_items(*this);
        for (; first != last; ++first) {
            items_m.push_back(*first);
        }
        ::message_item_set(*this);
    }

    void listbox_t::display(const model_type& value)
    {
        assert(control_m);

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
                if (first->find(static_name_t("value"))->second == value) {
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

        GG::Connect(element.control_m->SelChangedSignal,
                    boost::bind(&listbox_selection_changed, boost::ref(element), _1));

        GG::Connect(element.control_m->DroppedSignal,
                    boost::bind(&listbox_dropped, boost::ref(element), _1));

        GG::Connect(element.control_m->DropAcceptableSignal,
                    boost::bind(&listbox_drop_acceptable, boost::ref(element), _1));

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

        return display.insert(parent, element.control_m);
    }

}
