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

#ifndef LISTBOX_HPP
#define LISTBOX_HPP

#include <GG/adobe/config.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/popup_common_fwd.hpp>

#include <GG/ListBox.h>

#include <boost/noncopyable.hpp>


namespace adobe {

    struct listbox_t : boost::noncopyable
    {
        typedef dictionary_t item_t;
        typedef std::vector<item_t> item_set_t;

        typedef any_regular_t model_type;
        typedef boost::function<void (const any_regular_t&)> setter_type;

        listbox_t(const std::string& name,
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
                  name_t signal_id);

        void reset_item_set(const item_t* first, const item_t* last);

        void measure(extents_t& result);
        void place(const place_data_t& place_data);
        void display(const any_regular_t& item);
        void monitor(const setter_type& proc);
        void enable(bool make_enabled);

        GG::ListBox* control_m;
        unsigned int original_height_m;
        label_t name_m;
        std::string alt_text_m;
        bool using_label_m;
        setter_type value_proc_m;
        item_set_t items_m;
        int characters_m;
        int rows_m;
        GG::Flags<GG::ListBoxStyle> style_m;
        GG::Clr color_m;
        GG::Clr interior_color_m;
        GG::Clr hilite_color_m;
        GG::Clr item_text_color_m;
        std::vector<std::string> drop_types_m;
        name_t signal_id_m;

        typedef boost::function<void (const listbox_t& listbox, const GG::ListBox::SelectionSet&)> selection_changed_signal_t;
        typedef boost::function<void (const listbox_t& listbox, GG::ListBox::iterator)> row_signal_t;
        typedef boost::function<void (const listbox_t& listbox, GG::ListBox::const_iterator)> const_row_signal_t;
        typedef boost::function<void (const listbox_t& listbox, GG::ListBox::iterator, const GG::Pt&)> row_click_signal_t;

        selection_changed_signal_t selection_changed_proc_m;
        row_signal_t dropped_proc_m;
        const_row_signal_t drop_acceptable_proc_m;
        row_click_signal_t left_clicked_proc_m;
        row_click_signal_t right_clicked_proc_m;
        row_signal_t double_clicked_proc_m;
        row_signal_t erased_proc_m;
        row_signal_t browsed_proc_m;
    };

    namespace view_implementation {

        inline void set_value_from_model(listbox_t& value, const any_regular_t& new_value)
        { value.display(new_value); }

    }

}

#endif
