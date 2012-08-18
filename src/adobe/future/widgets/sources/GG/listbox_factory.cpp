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

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/localization.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <GG/EveGlue.h>


namespace {

    bool register_()
    {
        GG::RegisterView(adobe::static_name_t("listbox"), &adobe::implementation::make_listbox);
        return true;
    }

    bool dummy = register_();

    void handle_selection_changed_signal(const adobe::listbox_t& listbox,
                                         adobe::signal_notifier_t signal_notifier,
                                         adobe::name_t signal_type,
                                         adobe::name_t widget_id,
                                         adobe::sheet_t& sheet,
                                         adobe::name_t bind,
                                         adobe::array_t expression,
                                         const GG::ListBox::SelectionSet& selections)
    {
        adobe::array_t array;
        for (GG::ListBox::SelectionSet::const_iterator it = selections.begin(); it != selections.end(); ++it) {
            array.push_back(adobe::listbox_t::row_value(*it, listbox.control_m->end()));
        }

        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::any_regular_t(array));
    }

    void handle_row_signal(const adobe::listbox_t& listbox,
                           adobe::signal_notifier_t signal_notifier,
                           adobe::name_t signal_type,
                           adobe::name_t widget_id,
                           adobe::sheet_t& sheet,
                           adobe::name_t bind,
                           adobe::array_t expression,
                           GG::ListBox::iterator row)
    {
        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::listbox_t::row_value(row, listbox.control_m->end()),
                                             adobe::static_name_t("row"),
                                             adobe::any_regular_t(row),
                                             adobe::static_name_t("gg_row_iter"));
    }

    void handle_drop_acceptable_signal(const adobe::listbox_t& listbox,
                                       adobe::signal_notifier_t signal_notifier,
                                       adobe::name_t signal_type,
                                       adobe::name_t widget_id,
                                       adobe::sheet_t& sheet,
                                       adobe::name_t bind,
                                       adobe::array_t expression,
                                       const GG::ListBox::Row& row,
                                       GG::ListBox::const_iterator insertion_it)
    {
        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::listbox_t::row_value(row),
                                             adobe::static_name_t("row"),
                                             adobe::any_regular_t(&row),
                                             adobe::static_name_t("gg_row"),
                                             adobe::listbox_t::row_value(insertion_it, listbox.control_m->end()),
                                             adobe::static_name_t("insertion_iter"),
                                             adobe::any_regular_t(insertion_it),
                                             adobe::static_name_t("gg_insertion_iter"));
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
        adobe::array_t pt_array;
        pt_array.push_back(adobe::any_regular_t(Value(pt.x)));
        pt_array.push_back(adobe::any_regular_t(Value(pt.y)));

        adobe::implementation::handle_signal(signal_notifier,
                                             adobe::static_name_t("listbox"),
                                             signal_type,
                                             widget_id,
                                             sheet,
                                             bind,
                                             expression,
                                             adobe::listbox_t::row_value(row, listbox.control_m->end()),
                                             adobe::static_name_t("row"),
                                             adobe::any_regular_t(pt_array),
                                             adobe::static_name_t("pt"),
                                             adobe::any_regular_t(row),
                                             adobe::static_name_t("gg_row_iter"),
                                             adobe::any_regular_t(pt),
                                             adobe::static_name_t("gg_pt"));
    }

    void get_sort_order_style(adobe::name_t sort_order, GG::Flags<GG::ListBoxStyle>& style)
    {
        if (sort_order == adobe::static_name_t("descending"))
            style |= GG::LIST_SORTDESCENDING;
        else if (sort_order != adobe::static_name_t("ascending"))
            throw std::runtime_error("Unknown listbox sort order.");
    }

    void get_selection_style(adobe::name_t selections, GG::Flags<GG::ListBoxStyle>& style)
    {
        if (selections == adobe::static_name_t("none"))
            style |= GG::LIST_NOSEL;
        else if (selections == adobe::static_name_t("single"))
            style |= GG::LIST_SINGLESEL;
        else if (selections == adobe::static_name_t("quick"))
            style |= GG::LIST_QUICKSEL;
        else if (selections != adobe::static_name_t("multiple"))
            throw std::runtime_error("Unknown listbox selections type.");
    }

}

namespace adobe {

    void create_widget(const dictionary_t& parameters,
                       size_enum_t,
                       listbox_t*& listbox)
    {
        std::string name;
        std::string alt_text;
        long characters(25);
        long rows(0);
        bool sort(false);
        name_t sort_order("ascending");
        name_t selections("single");
        bool user_delete(false);
        bool browse_updates(false);
        array_t items;
        listbox_t::item_set_t item_set;
        GG::Clr color(GG::CLR_GRAY);
        GG::Clr interior_color(GG::CLR_ZERO);
        GG::Clr hilite_color(GG::CLR_SHADOW);
        GG::Clr label_color(GG::CLR_BLACK);
        GG::Clr item_text_color(GG::CLR_BLACK);
        name_t signal_id;
        array_t allowed_drop_types;

        implementation::get_localized_string(parameters, key_name, name);
        implementation::get_localized_string(parameters, key_alt_text, alt_text);
        get_value(parameters, key_characters, characters);
        get_value(parameters, static_name_t("rows"), rows);
        get_value(parameters, static_name_t("sort"), sort);
        get_value(parameters, static_name_t("sort_order"), sort_order);
        get_value(parameters, static_name_t("selections"), selections);
        get_value(parameters, static_name_t("user_delete"), user_delete);
        get_value(parameters, static_name_t("browse_updates"), browse_updates);
        get_value(parameters, key_items, items);
        implementation::get_color(parameters, static_name_t("color"), color);
        implementation::get_color(parameters, static_name_t("interior_color"), interior_color);
        implementation::get_color(parameters, static_name_t("hilite_color"), hilite_color);
        implementation::get_color(parameters, static_name_t("label_color"), label_color);
        implementation::get_color(parameters, static_name_t("item_text_color"), item_text_color);
        get_value(parameters, static_name_t("signal_id"), signal_id);
        get_value(parameters, static_name_t("allowed_drop_types"), allowed_drop_types);

        GG::Flags<GG::ListBoxStyle> style;
        if (!sort)
            style |= GG::LIST_NOSORT;

        get_sort_order_style(sort_order, style);
        get_selection_style(selections, style);

        if (user_delete)
            style |= GG::LIST_USERDELETE;

        if (browse_updates)
            style |= GG::LIST_BROWSEUPDATES;

        for (array_t::iterator first(items.begin()), last(items.end()); first != last; ++first) {
            item_set.push_back(any_regular_t(first->cast<dictionary_t>()));
            dictionary_t& back = item_set.back().cast<dictionary_t>();
            get_value(back, key_value);
            dictionary_t::iterator it = back.find(key_name);
            if (it != back.end() && it->second.type_info() == type_info<adobe::string_t>()) {
                it->second.assign(localization_invoke(it->second.cast<std::string>()));
            }
        }

        if (!rows && items.empty())
            rows = 8;

        std::vector<std::string> drop_types;
        for (array_t::iterator first(allowed_drop_types.begin()), last(allowed_drop_types.end()); first != last; ++first) {
            drop_types.push_back(first->cast<std::string>());
        }

        listbox = new listbox_t(name,
                                alt_text,
                                characters,
                                rows,
                                style,
                                item_set,
                                color,
                                interior_color,
                                hilite_color,
                                label_color,
                                item_text_color,
                                drop_types,
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
        basic_sheet_t& layout_sheet(token.client_holder_m.layout_sheet_m);
        assemblage_t&  assemblage(token.client_holder_m.assemblage_m);

        if (parameters.count(key_bind) != 0) {
            name_t cell(get_value(parameters, key_bind).cast<name_t>());
            attach_view_and_controller_direct(control, parameters, token, cell);
        }

        if (parameters.count(key_items) && 
            get_value(parameters, key_items).type_info() == type_info<name_t>()) {
            name_t cell(get_value(parameters, key_items).cast<name_t>());
            listbox_t::item_set_view_controller_t& tmp = control.item_set_view_controller_m;
            if (layout_sheet.count_interface(cell)) {
                attach_view(assemblage, cell, tmp, layout_sheet);
                attach_controller_direct(tmp, parameters, token, cell);
            } else {
                attach_view(assemblage, cell, tmp, token.sheet_m);
                attach_controller_direct(tmp, parameters, token, cell);
            }
        }

        {
            any_regular_t selection_changed_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_selection_changed_signal"), selection_changed_binding))
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

        {
            any_regular_t dropped_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_dropped_signal"), dropped_binding))
                implementation::cell_and_expression(dropped_binding, cell, expression);
            control.dropped_proc_m =
                boost::bind(&handle_row_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("dropped"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        {
            any_regular_t drop_acceptable_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_drop_acceptable_signal"), drop_acceptable_binding))
                implementation::cell_and_expression(drop_acceptable_binding, cell, expression);
            control.drop_acceptable_proc_m =
                boost::bind(&handle_drop_acceptable_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("drop_acceptable"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2,
                            _3);
        }

        {
            any_regular_t left_clicked_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_left_clicked_signal"), left_clicked_binding))
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

        {
            any_regular_t right_clicked_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_right_clicked_signal"), right_clicked_binding))
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

        {
            any_regular_t double_clicked_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_double_clicked_signal"), double_clicked_binding))
                implementation::cell_and_expression(double_clicked_binding, cell, expression);
            control.double_clicked_proc_m =
                boost::bind(&handle_row_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("double_clicked"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        {
            any_regular_t erased_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_erased_signal"), erased_binding))
                implementation::cell_and_expression(erased_binding, cell, expression);
            control.erased_proc_m =
                boost::bind(&handle_row_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("erased"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        {
            any_regular_t browsed_binding;
            name_t cell;
            array_t expression;
            if (get_value(parameters, static_name_t("bind_browsed_signal"), browsed_binding))
                implementation::cell_and_expression(browsed_binding, cell, expression);
            control.browsed_proc_m =
                boost::bind(&handle_row_signal,
                            _1,
                            token.signal_notifier_m,
                            static_name_t("browsed"),
                            control.signal_id_m,
                            boost::ref(token.sheet_m),
                            cell,
                            expression,
                            _2);
        }

        control.row_factory_m = token.row_factory_m;

        adobe::attach_view(control.name_m.color_proxy_m, parameters, token, adobe::static_name_t("bind_label_color"));
#define BIND_COLOR(name)                                                \
        adobe::attach_view(control.name##_proxy_m, parameters, token, adobe::static_name_t("bind_" #name))
        BIND_COLOR(color);
        BIND_COLOR(interior_color);
        BIND_COLOR(hilite_color);
        BIND_COLOR(item_text_color);
#undef BIND_COLOR
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
                factory.layout_attributes(static_name_t("listbox"))
            );
        }

    }

}
