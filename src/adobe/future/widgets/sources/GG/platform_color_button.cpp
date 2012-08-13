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

#include "platform_color_button.hpp"

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/ClrConstants.h>
#include <GG/Control.h>
#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/dialogs/ColorDlg.h>


namespace {

    class ColorButton : public GG::Control
    {
    public:
        /** \name Structors */ ///@{
        ColorButton(GG::X width,
                    GG::Y height,
                    GG::Clr color,
                    unsigned int border,
                    GG::Clr dialog_color,
                    GG::Clr dialog_border_color,
                    GG::Clr dialog_text_color) :
            Control(GG::X0, GG::Y0, width, height),
            m_border(border),
            m_dialog_color(dialog_color),
            m_dialog_border_color(dialog_border_color),
            m_dialog_text_color(dialog_text_color)
            { SetColor(color); }
        //@}

        /** \name Accessors */ ///@{
        /** returns the custom color represented by the button */
        GG::Clr RepresentedColor() const
            { return m_represented_color; }
        //@}

        /** \name Mutators */ ///@{
        /** sets the custom color represented by the button */
        void SetRepresentedColor(GG::Clr color)
            { m_represented_color = color; }

        virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
            {
                if (!Disabled()) {
                    GG::ColorDlg dlg(GG::X0, GG::Y0,
                                     m_represented_color,
                                     adobe::implementation::DefaultFont(),
                                     m_dialog_color,
                                     m_dialog_border_color,
                                     m_dialog_text_color);
                    dlg.Run();
                    if (dlg.ColorWasSelected()) {
                        m_represented_color = dlg.Result();
                        ColorChanged();
                    }
                }
            }
        virtual void Render()
            {
                GG::Pt ul = UpperLeft(), lr = LowerRight();
                GG::Clr color_to_use = Disabled() ? GG::DisabledColor(Color()) : Color();
                GG::FlatRectangle(ul, lr, GG::CLR_ZERO, m_color, m_border);
                GG::FlatRectangle(ul, lr, m_represented_color, GG::CLR_ZERO, m_border);
            }
        //@}

        mutable boost::signal<void ()> ColorChanged;

    private:
        GG::Clr m_represented_color;
        GG::Clr m_dialog_color;
        GG::Clr m_dialog_border_color;
        GG::Clr m_dialog_text_color;
        unsigned int m_border;
    };

    void color_changed(adobe::color_button_t& color_button)
    {
        if (color_button.value_proc_m) {
            color_button.value_proc_m(
                static_cast<ColorButton*>(color_button.control_m)->RepresentedColor()
            );
        }
    }

    const unsigned int gap = 4;

}

namespace adobe {

    color_button_t::color_button_t(const std::string& name,
                                   const std::string& alt_text,
                                   unsigned int width,
                                   unsigned int height,
                                   name_t signal_id,
                                   GG::Clr border_color,
                                   GG::Clr label_color,
                                   GG::Clr dialog_color,
                                   GG::Clr dialog_border_color,
                                   GG::Clr dialog_text_color) :
        control_m(0),
        width_m(width),
        height_m(height),
        name_m(name, alt_text, 0, GG::FORMAT_LEFT | GG::FORMAT_TOP, label_color),
        alt_text_m(alt_text),
        using_label_m(!name.empty()),
        signal_id_m(signal_id),
        border_color_m(border_color),
        dialog_color_m(dialog_color),
        dialog_border_color_m(dialog_border_color),
        dialog_text_color_m(dialog_text_color)
    {}

    void color_button_t::measure(extents_t& result)
    {
        assert(control_m);
        result.width() = Value(original_size_m.x);
        result.height() = Value(original_size_m.y);

        boost::shared_ptr<GG::Font> font = implementation::DefaultFont();

        GG::Y baseline_offset = (control_m->Height() - font->Height()) / 2;
        result.vertical().guide_set_m.push_back(Value(baseline_offset));

        if (!using_label_m)
            return;

        extents_t label_bounds(measure_text(name_m.name_m, font));
        label_bounds.vertical().guide_set_m.push_back(0);

        align_slices(result.vertical(), label_bounds.vertical());

        result.width() += gap + label_bounds.width();

        result.horizontal().guide_set_m.push_back(label_bounds.width());
    }

    void color_button_t::place(const place_data_t& place_data)
    {
        assert(control_m);
        implementation::set_control_bounds(control_m, place_data);
    }

    void color_button_t::enable(bool make_enabled)
    {
        assert(control_m);
        control_m->Disable(!make_enabled);
    }

    void color_button_t::display(model_type value)
    {
        assert(control_m);
        static_cast<ColorButton*>(control_m)->SetRepresentedColor(value);
    }

    void color_button_t::monitor(const setter_type& proc)
    {
        assert(control_m);
        value_proc_m = proc;
    }

    template <>
    platform_display_type insert<color_button_t>(display_t& display,
                                                 platform_display_type& parent,
                                                 color_button_t& element)
    {
        if (element.using_label_m)
            insert(display, parent, element.name_m);

        assert(!element.control_m);

        ColorButton* button = new ColorButton(GG::X(element.width_m),
                                              GG::Y(element.height_m),
                                              element.border_color_m,
                                              2,
                                              element.dialog_color_m,
                                              element.dialog_border_color_m,
                                              element.dialog_text_color_m);
        element.original_size_m = GG::Pt(button->Width(), button->Height());

        element.control_m = button;

        GG::Connect(button->ColorChanged, boost::bind(&color_changed, boost::ref(element)));

        if (!element.alt_text_m.empty())
            adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

        return display.insert(parent, element.control_m);
    }

}
