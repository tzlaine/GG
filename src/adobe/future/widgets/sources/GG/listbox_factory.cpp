/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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

#include "listbox_factory.hpp"

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/EveGlue.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/TextControl.h>


namespace {

    bool register_()
    {
        GG::RegisterView(adobe::static_name_t("listbox"), &adobe::implementation::make_listbox);
        return true;
    }

    bool dummy = register_();

    adobe::any_regular_t row_value(GG::ListBox::const_iterator it,
                                   GG::ListBox::const_iterator end_it);

    adobe::any_regular_t to_any_regular(const GG::Button& button)
    { return adobe::any_regular_t(button.Text()); }

    adobe::any_regular_t to_any_regular(const GG::DropDownList& drop_list,
                                        GG::ListBox::const_iterator end_it)
    { return row_value(drop_list.CurrentItem(), end_it); }

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

    adobe::any_regular_t row_value(GG::ListBox::const_iterator it,
                                   GG::ListBox::const_iterator end_it)
    {
        if (it == end_it)
            return adobe::any_regular_t();

        adobe::array_t elements;

        const std::size_t size = (*it)->size();
        for (std::size_t i = 0; i < size; ++i) {
            GG::Control* element = (**it)[i];
            if (const GG::Button* button = dynamic_cast<const GG::Button*>(element))
                elements.push_back(to_any_regular(*button));
            else if (const GG::DropDownList* drop_list = dynamic_cast<const GG::DropDownList*>(element))
                elements.push_back(to_any_regular(*drop_list, end_it));
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
                elements.push_back(adobe::any_regular_t());
        }

        if (elements.empty())
            return adobe::any_regular_t();
        else if (elements.size() == 1u)
            return adobe::any_regular_t(elements[0]);
        else
            return adobe::any_regular_t(elements);
    }

    void handle_selection_changed_signal(const adobe::listbox_t& listbox,
                                         adobe::signal_notifier_t signal_notifier,
                                         adobe::name_t signal_type,
                                         adobe::name_t widget_id,
                                         adobe::sheet_t& sheet,
                                         adobe::name_t bind,
                                         adobe::array_t expression,
                                         const GG::ListBox::SelectionSet& selections)
    {
        adobe::array_t value;
        for (GG::ListBox::SelectionSet::const_iterator it = selections.begin(); it != selections.end(); ++it) {
            value.push_back(row_value(*it, listbox.control_m->end()));
        }

        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::any_regular_t(value));
    }

    template <typename Iter>
    void handle_row_signal(const adobe::listbox_t& listbox,
                           adobe::signal_notifier_t signal_notifier,
                           adobe::name_t signal_type,
                           adobe::name_t widget_id,
                           adobe::sheet_t& sheet,
                           adobe::name_t bind,
                           adobe::array_t expression,
                           Iter row)
    {
        adobe::dictionary_t value;
        value[adobe::static_name_t("row")] = row_value(row, listbox.control_m->end());
        value[adobe::static_name_t("gg_row_iter")] = adobe::any_regular_t(row);

        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::any_regular_t(value));
    }

    void handle_row_click_signal(const adobe::listbox_t& listbox,
                                 adobe::signal_notifier_t signal_notifier,
                                 adobe::name_t signal_type,
                                 adobe::name_t widget_id,
                                 adobe::sheet_t& sheet,
                                 adobe::name_t bind,
                                 adobe::array_t expression,
                                 GG::ListBox::iterator row,
                                 const GG::Pt& pt)
    {
        adobe::dictionary_t value;

        value[adobe::static_name_t("row")] = row_value(row, listbox.control_m->end());

        {
            adobe::array_t pt_array;
            pt_array.push_back(adobe::any_regular_t(Value(pt.x)));
            pt_array.push_back(adobe::any_regular_t(Value(pt.y)));
            value[adobe::static_name_t("pt")] = adobe::any_regular_t(pt_array);
        }

        value[adobe::static_name_t("gg_row_iter")] = adobe::any_regular_t(row);
        value[adobe::static_name_t("gg_pt")] = adobe::any_regular_t(pt);

        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::any_regular_t(value));
    }

}

namespace adobe {

    void create_widget(const dictionary_t& parameters,
                       size_enum_t,
                       listbox_t*& listbox)
    {
        std::string name;
        std::string alt_text;
        long characters(50);
        long rows(0);
        long width(0);
        long height(0);
        array_t items;
        listbox_t::item_set_t item_set;
        GG::Clr color(GG::CLR_GRAY);
        GG::Clr interior_color(GG::CLR_ZERO);
        name_t signal_id;

        get_value(parameters, key_name, name);
        get_value(parameters, key_alt_text, alt_text);
        get_value(parameters, key_characters, characters);
        get_value(parameters, static_name_t("rows"), rows);
        get_value(parameters, static_name_t("width"), width);
        get_value(parameters, static_name_t("height"), height);
        get_value(parameters, key_items, items);
        implementation::get_color(parameters, static_name_t("color"), color);
        implementation::get_color(parameters, static_name_t("interior_color"), interior_color);
        get_value(parameters, static_name_t("signal_id"), signal_id);

        for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first)
        {
            dictionary_t cur_item(first->cast<dictionary_t>());
            item_set.push_back(
                listbox_t::item_set_t::value_type(
                    get_value(cur_item, key_name).cast<std::string>(), get_value(cur_item, key_value)
                )
            );
        }

        if (!rows && items.empty())
            rows = 8;

        listbox = new listbox_t(name,
                                alt_text,
                                characters,
                                rows,
                                width,
                                height,
                                item_set,
                                color,
                                interior_color,
                                signal_id);
    }

    template <>
    void attach_view_and_controller(listbox_t&             control,
                                    const dictionary_t&    parameters,
                                    const factory_token_t& token,
                                    adobe::name_t,
                                    adobe::name_t,
                                    adobe::name_t)
    {
        if (parameters.count(key_bind) != 0) {
            attach_view_and_controller_direct(control, parameters, token,
                                              key_bind, key_bind_view, key_bind_controller);
        }

        any_regular_t selection_changed_binding;
        if (get_value(parameters, static_name_t("bind_selection_changed_signal"), selection_changed_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(selection_changed_binding, cell, expression);
            control.selection_changed_proc_m =
                boost::bind(&handle_selection_changed_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("selection_changed"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        any_regular_t dropped_binding;
        if (get_value(parameters, static_name_t("bind_dropped_signal"), dropped_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(dropped_binding, cell, expression);
            control.dropped_proc_m =
                boost::bind(&handle_row_signal<GG::ListBox::iterator>,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("dropped"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        any_regular_t drop_acceptable_binding;
        if (get_value(parameters, static_name_t("bind_drop_acceptable_signal"), drop_acceptable_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(drop_acceptable_binding, cell, expression);
            control.drop_acceptable_proc_m =
                boost::bind(&handle_row_signal<GG::ListBox::const_iterator>,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("drop_acceptable"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        any_regular_t left_clicked_binding;
        if (get_value(parameters, static_name_t("bind_left_clicked_signal"), left_clicked_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(left_clicked_binding, cell, expression);
            control.left_clicked_proc_m =
                boost::bind(&handle_row_click_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("left_clicked"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2,
                            _3);
        }

        any_regular_t right_clicked_binding;
        if (get_value(parameters, static_name_t("bind_right_clicked_signal"), right_clicked_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(right_clicked_binding, cell, expression);
            control.right_clicked_proc_m =
                boost::bind(&handle_row_click_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("right_clicked"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2,
                            _3);
        }

        any_regular_t double_clicked_binding;
        if (get_value(parameters, static_name_t("bind_double_clicked_signal"), double_clicked_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(double_clicked_binding, cell, expression);
            control.double_clicked_proc_m =
                boost::bind(&handle_row_signal<GG::ListBox::iterator>,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("double_clicked"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        any_regular_t erased_binding;
        if (get_value(parameters, static_name_t("bind_erased_signal"), erased_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(erased_binding, cell, expression);
            control.erased_proc_m =
                boost::bind(&handle_row_signal<GG::ListBox::iterator>,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("erased"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        any_regular_t browsed_binding;
        if (get_value(parameters, static_name_t("bind_browsed_signal"), browsed_binding)) {
            name_t cell;
            array_t expression;
            implementation::cell_and_expression(browsed_binding, cell, expression);
            control.browsed_proc_m =
                boost::bind(&handle_row_signal<GG::ListBox::iterator>,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("browsed"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }
    }

    namespace implementation {

        widget_node_t make_listbox(const dictionary_t& parameters,
                                   const widget_node_t& parent,
                                   const factory_token_t& token,
                                   const widget_factory_t& factory)
        {
            return create_and_hookup_widget<listbox_t, poly_placeable_t>(
                parameters, parent, token,
                factory.is_container(static_name_t("listbox")),
                factory.layout_attributes(static_name_t("listbox")));
        }

    }

}
