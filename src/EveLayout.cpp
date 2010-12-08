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
   
#include <GG/EveLayout.h>

#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Menu.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>
#include <GG/ExpressionWriter.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <boost/cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <numeric>


using namespace GG;

#define INSTRUMENT_WINDOW_CREATION 0
#define INSTRUMENT_CREATED_LAYOUT 0

#if INSTRUMENT_WINDOW_CREATION
void verbose_dump(const adobe::array_t& array, std::size_t indent = 0);
void verbose_dump(const adobe::dictionary_t& array, std::size_t indent = 0);

void verbose_dump(const adobe::array_t& array, std::size_t indent)
{
    if (array.empty()) {
        std::cout << std::string(4 * indent, ' ') << "[]\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "[\n";
    ++indent;
    for (adobe::array_t::const_iterator it = array.begin(); it != array.end(); ++it) {
        const adobe::any_regular_t& any = *it;
        if (any.type_info() == adobe::type_info<adobe::array_t>()) {
            verbose_dump(any.cast<adobe::array_t>(), indent);
        } else if (any.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            verbose_dump(any.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "type: " << any.type_info() << " "
                      << "value: " << any << "\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "]\n";
}

void verbose_dump(const adobe::dictionary_t& dictionary, std::size_t indent)
{
    if (dictionary.empty()) {
        std::cout << std::string(4 * indent, ' ') << "{}\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "{\n";
    ++indent;
    for (adobe::dictionary_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it) {
        const adobe::pair<adobe::name_t, adobe::any_regular_t>& pair = *it;
        if (pair.second.type_info() == adobe::type_info<adobe::array_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::array_t>(), indent);
        } else if (pair.second.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "(" << pair.first << ", "
                      << "type: " << pair.second.type_info() << " "
                      << "value: " << pair.second << ")\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "}\n";
}
#endif

namespace {

    adobe::aggregate_name_t key_spacing             = { "spacing" };
    adobe::aggregate_name_t key_indent              = { "indent" };
    adobe::aggregate_name_t key_margin              = { "margin" };

    adobe::aggregate_name_t key_placement           = { "placement" };

    adobe::aggregate_name_t key_horizontal          = { "horizontal" };
    adobe::aggregate_name_t key_vertical            = { "vertical" };

    adobe::aggregate_name_t key_child_horizontal    = { "child_horizontal" };
    adobe::aggregate_name_t key_child_vertical      = { "child_vertical" };

    adobe::aggregate_name_t key_align_left          = { "align_left" };
    adobe::aggregate_name_t key_align_right         = { "align_right" };
    adobe::aggregate_name_t key_align_top           = { "align_top" };
    adobe::aggregate_name_t key_align_bottom        = { "align_bottom" };
    adobe::aggregate_name_t key_align_center        = { "align_center" };
    adobe::aggregate_name_t key_align_proportional  = { "align_proportional" };
    adobe::aggregate_name_t key_align_fill          = { "align_fill" };

    adobe::aggregate_name_t key_place_row           = { "place_row" };
    adobe::aggregate_name_t key_place_column        = { "place_column" };
    adobe::aggregate_name_t key_place_overlay       = { "place_overlay" };

    adobe::aggregate_name_t key_guide_mask          = { "guide_mask" };
    adobe::aggregate_name_t key_guide_balance       = { "guide_balance" };
    
    adobe::aggregate_name_t key_guide_baseline      = { "guide_baseline" };
    adobe::aggregate_name_t key_guide_label         = { "guide_label" };

    adobe::aggregate_name_t key_step                = { "step" };
    adobe::aggregate_name_t key_allow_edits         = { "allow_edits" };

    adobe::aggregate_name_t name_row                = { "row" };
    adobe::aggregate_name_t name_column             = { "column" };
    adobe::aggregate_name_t name_overlay            = { "overlay" };
    adobe::aggregate_name_t name_static_text        = { "static_text" };
    adobe::aggregate_name_t name_radio_button_group = { "radio_button_group" };
    adobe::aggregate_name_t name_menu_bar           = { "menu_bar" };
    adobe::aggregate_name_t name_int_spin           = { "int_spin" };
    adobe::aggregate_name_t name_double_spin        = { "double_spin" };

    StyleFactory& Factory()
    { return *GUI::GetGUI()->GetStyleFactory(); }

    boost::shared_ptr<Font> DefaultFont()
    { return Factory().DefaultFont(); }

    Y CharHeight()
    { return DefaultFont()->Lineskip(); }

    struct Panel :
        public Wnd
    {
        Panel(Orientation orientation, const std::string& name) :
            Wnd(X0, Y0, X1, Y1, Flags<WndFlag>()),
            m_orientation(orientation),
            m_name(name)
            {}

        const std::string& Name() const
            { return m_name; }

        void Add(Wnd* wnd)
            {
                if (Layout* layout = GetLayout()) {
                    layout->Add(wnd,
                                m_orientation == VERTICAL ? layout->Rows() : 0,
                                m_orientation == VERTICAL ? 0 : layout->Columns());
                } else {
                    AttachChild(wnd);
                    if (m_orientation == VERTICAL)
                        VerticalLayout();
                    else
                        HorizontalLayout();
                }
            }

        // TODO

        Orientation m_orientation;
        std::string m_name;
    };

    struct Overlay :
        public Wnd
    {
        Overlay() :
            Wnd(X0, Y0, X1, Y1, Flags<WndFlag>())
            {}

        void AddPanel(Panel* panel)
            { AttachChild(panel); }

        // TODO
    };

    // TODO: Put this into StyleFactory.
    struct Dialog :
        public Wnd
    {
        Dialog(const std::string& name, Flags<WndFlag> flags) :
            Wnd(X(10), Y(10), X(200), Y(200), flags | MODAL | DRAGABLE | INTERACTIVE),
            m_title(Factory().NewTextControl(BEVEL_OFFSET.x, BEVEL_OFFSET.y - CharHeight(),
                                             X(200) - BEVEL_OFFSET.x * 2, CharHeight(),
                                             name, DefaultFont()))
            { AttachChild(m_title); }

        virtual Pt ClientUpperLeft() const
            { return UpperLeft() + BEVEL_OFFSET + Pt(X0, m_title->Height()); }

        virtual Pt ClientLowerRight() const
            { return LowerRight() - BEVEL_OFFSET; }

        virtual WndRegion WindowRegion(const Pt& pt) const
            {
                enum {LEFT = 0, MIDDLE = 1, RIGHT = 2};
                enum {TOP = 0, BOTTOM = 2};

                // window regions look like this:
                // 0111112
                // 3444445   // 4 is client area, 0,2,6,8 are corners
                // 3444445
                // 6777778

                int x_pos = MIDDLE;   // default & typical case is that the mouse is over the (non-border) client area
                int y_pos = MIDDLE;

                Pt ul = UpperLeft() + BEVEL_OFFSET, lr = LowerRight() - BEVEL_OFFSET;

                if (pt.x < ul.x)
                    x_pos = LEFT;
                else if (pt.x > lr.x)
                    x_pos = RIGHT;

                if (pt.y < ul.y)
                    y_pos = TOP;
                else if (pt.y > lr.y)
                    y_pos = BOTTOM;

                return (Resizable() ? WndRegion(x_pos + 3 * y_pos) : WR_NONE);
            }

        virtual void SizeMove(const Pt& ul, const Pt& lr)
            {
                Wnd::SizeMove(ul, lr);
                Pt new_title_size = Pt((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
                m_title->Resize(new_title_size);
            }

        virtual void Render()
            { BeveledRectangle(UpperLeft(), LowerRight(), CLR_GRAY, CLR_GRAY, true); }

        TextControl* m_title;

        static const int FRAME_WIDTH;
        static const Pt BEVEL_OFFSET;
    };

    const int Dialog::FRAME_WIDTH = 2;
    const Pt Dialog::BEVEL_OFFSET(X(Dialog::FRAME_WIDTH), Y(Dialog::FRAME_WIDTH));

    struct MakeWndResult
    {
        enum Alignment
        {
            ALIGN_FILL,
            ALIGN_TOP,
            ALIGN_BOTTOM,
            ALIGN_LEFT,
            ALIGN_RIGHT,
            ALIGN_CENTER,
            ALIGN_PROPORTIONAL
        };

        MakeWndResult(const adobe::dictionary_t& params) :
            m_horizontal(ALIGN_PROPORTIONAL),
            m_vertical(ALIGN_PROPORTIONAL),
            m_child_horizontal(ALIGN_PROPORTIONAL),
            m_child_vertical(ALIGN_PROPORTIONAL),
            m_spacing(0),
            m_indent(0),
            m_margin(5)
            {
                get_value(params, key_horizontal, m_horizontal);
                get_value(params, key_vertical, m_vertical);
                get_value(params, key_child_horizontal, m_child_horizontal);
                get_value(params, key_child_vertical, m_child_vertical);
                get_value(params, key_spacing, m_spacing);
                get_value(params, key_indent, m_indent);
                get_value(params, key_margin, m_margin);
            }

        Alignment m_horizontal;
        Alignment m_vertical;
        Alignment m_child_horizontal;
        Alignment m_child_vertical;
        int m_spacing;
        int m_indent;
        int m_margin;
        boost::ptr_vector<Wnd> m_wnds;
    };

    struct MakeWndResultLess
    {
        std::size_t operator()(const MakeWndResult& lhs, const MakeWndResult& rhs)
            { return lhs.m_wnds.size() < rhs.m_wnds.size(); }
    };

#if INSTRUMENT_CREATED_LAYOUT
    std::string WndTypeStr(Wnd* w)
    {
#define CASE(x) if (dynamic_cast<x*>(w)) return #x;

        using boost::lexical_cast;

        CASE(Dialog)
        else CASE(Panel)
        else CASE(Overlay)
        else CASE(Button)
        else CASE(StateButton)
        else CASE(TextControl)
        else CASE(Edit)
        else CASE(StaticGraphic)
        else CASE(DropDownList)
        else CASE(RadioButtonGroup)
        else CASE(Slider)
        else CASE(TabBar)
        else CASE(MenuBar)
        else CASE(Spin<int>)
        else CASE(Spin<double>)
        else if (Layout* l = dynamic_cast<Layout*>(w)) {
            return
                lexical_cast<std::string>(l->Rows()) + "x" +
                lexical_cast<std::string>(l->Columns()) +
                " Layout";
        } else {
            return "Unknown";
        }

#undef CASE
    }

    void DumpLayout(Wnd* w, int indent = 0)
    {
        std::cout << std::string(indent * 4, ' ') << WndTypeStr(w) << " "
                  << w->RelativeUpperLeft() << " - "
                  << w->RelativeLowerRight()
                  << "\n";
        for (std::list<Wnd*>::const_iterator it = w->Children().begin(); it != w->Children().end(); ++it) {
            DumpLayout(*it, indent + 1);
        }
    }
#endif

    MakeWndResult* Make_dialog(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        bool grow = false;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_grow, grow);

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(new Dialog(name, grow ? RESIZABLE : Flags<WndFlag>()));

        return retval.release();
    }

    MakeWndResult* Make_button(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        bool default_ = false;
        bool cancel = false;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::name_t action;
        adobe::any_regular_t value;
        adobe::name_t bind_output;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_default, default_);
        get_value(params, adobe::key_cancel, cancel);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_action, action);
        get_value(params, adobe::key_value, value);
        get_value(params, adobe::key_bind_output, bind_output);

        // TODO bind_view ?
        // TODO bind_controller ?
        // skipping items
        // skipping modifiers

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewButton(X0, Y0, X1, Y1, name, DefaultFont(), CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_checkbox(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::any_regular_t value_on;
        adobe::any_regular_t value_off;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_value, value_on);
        get_value(params, adobe::key_value, value_off);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(
            Factory().NewStateButton(X0, Y0, X1, Y1, name, DefaultFont(), FORMAT_NONE, CLR_GRAY)
        );

        return retval.release();
    }

    MakeWndResult* Make_display_number(const adobe::dictionary_t& params,
                                       const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t characters;
        adobe::array_t units;
        adobe::string_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_units, units);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, X1, Y1, "", DefaultFont()));

        return retval.release();
    }

    MakeWndResult* Make_edit_number(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t digits;
        adobe::string_t format;
        double min_value;
        double max_value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_digits, digits);
        get_value(params, adobe::key_format, format);
        get_value(params, adobe::key_min_value, min_value);
        get_value(params, adobe::key_max_value, max_value);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
        retval->m_wnds.push_back(Factory().NewEdit(X0, Y0, X1, "", DefaultFont(), CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_edit_text(const adobe::dictionary_t& params,
                                  const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t characters;
        std::size_t lines;
        bool scrollable;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_lines, lines);
        get_value(params, adobe::key_scrollable, scrollable);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO password ?
        // skipping max_characters
        // skipping monospaced

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
        retval->m_wnds.push_back(Factory().NewEdit(X0, Y0, X1, "", DefaultFont(), CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_image(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t image;
        adobe::name_t bind;

        get_value(params, adobe::key_image, image);
        get_value(params, adobe::key_bind, bind);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        boost::shared_ptr<Texture> texture = GG::GUI::GetGUI()->GetTexture(image);
        retval->m_wnds.push_back(Factory().NewStaticGraphic(X0, Y0, X1, Y1, texture));

        return retval.release();
    }

    MakeWndResult* Make_popup(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::array_t items;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_items, items);

        // TODO bind_view ?
        // TODO bind_controller ?
        // skipping custom_item_name
        // skipping popup_bind
        // skipping popup_placement

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));

        const std::size_t MAX_LINES = 10;
        Y drop_height = CharHeight() * static_cast<int>(std::min(items.size(), MAX_LINES));
        retval->m_wnds.push_back(Factory().NewDropDownList(X0, Y0, X1, Y1, drop_height, CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_radio_button(const adobe::dictionary_t& params,
                                     const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::any_regular_t value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_value, value);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO touch ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(
            Factory().NewStateButton(X0, Y0, X1, Y1, name, DefaultFont(), FORMAT_NONE,
                                     CLR_GRAY, CLR_BLACK, CLR_ZERO, SBSTYLE_3D_RADIO)
        );

        return retval.release();
    }

    MakeWndResult* Make_radio_button_group(const adobe::dictionary_t& params,
                                           const adobe::line_position_t& position)
    {
        adobe::name_t placement = key_place_column;

        get_value(params, key_placement, placement);

        if (placement == key_place_overlay) {
            throw adobe::stream_error_t(
                "place_overlay placement is not compatible with radio_button_group",
                position
            );
        }

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        Orientation orientation = placement == key_place_column ? VERTICAL : HORIZONTAL;
        retval->m_wnds.push_back(Factory().NewRadioButtonGroup(X0, Y0, X1, Y1, orientation));

        return retval.release();
    }

    MakeWndResult* Make_slider(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::dictionary_t format;
        adobe::name_t orientation = adobe::key_vertical;
        double slider_ticks;

        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_format, format);
        get_value(params, adobe::key_orientation, orientation);
        get_value(params, adobe::key_slider_ticks, slider_ticks);

        // skipping slider_point

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        double min = 1;
        double max = 100;
        get_value(format, adobe::key_first, min);
        get_value(format, adobe::key_last, max);
        Orientation orientation_ = orientation == adobe::key_vertical ? VERTICAL : HORIZONTAL;
        const int TAB_WIDTH = 5;
        retval->m_wnds.push_back(
            Factory().NewSlider(X0, Y0, X1, Y1, min, max, orientation_, GROOVED, CLR_GRAY, TAB_WIDTH)
        );

        return retval.release();
    }

    MakeWndResult* Make_tab_group(const adobe::dictionary_t& params,
                                  const adobe::line_position_t& position)
    {
        adobe::string_t name; // TODO: is this useful?
        adobe::name_t bind;
        adobe::any_regular_t value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_value, value);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTabWnd(X0, Y0, X1, Y1, DefaultFont(), CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_static_text(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::string_t alt;
        std::size_t characters = 25;
        bool wrap = true;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_wrap, wrap);

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        X char_width = DefaultFont()->TextExtent("W").x;
        X w = static_cast<int>(characters) * char_width;
        Flags<TextFormat> format = wrap ? FORMAT_WORDBREAK : FORMAT_NONE;
        Pt extent = DefaultFont()->TextExtent(name, format, w);
        retval->m_wnds.push_back(
            Factory().NewTextControl(X0, Y0, extent.x, extent.y, name, DefaultFont(), CLR_BLACK, format)
        );

        return retval.release();
    }

    MakeWndResult* Make_menu_bar(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        // TODO

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        return retval.release();
    }

    MakeWndResult* Make_int_spin(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::dictionary_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));

        int step = 1;
        int min = 1;
        int max = 100;
        bool allow_edits = false;
        get_value(format, key_step, step);
        get_value(format, adobe::key_first, min);
        get_value(format, adobe::key_last, max);
        get_value(format, key_allow_edits, allow_edits);
        retval->m_wnds.push_back(
            Factory().NewIntSpin(X0, Y0, X1, 0, step, min, max, allow_edits, DefaultFont(), CLR_GRAY)
        );

        return retval.release();
    }

    MakeWndResult* Make_double_spin(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::dictionary_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));

        double step = 1.0;
        double min = 1.0;
        double max = 100.0;
        bool allow_edits = false;
        get_value(format, key_step, step);
        get_value(format, adobe::key_first, min);
        get_value(format, adobe::key_last, max);
        get_value(format, key_allow_edits, allow_edits);
        retval->m_wnds.push_back(
            Factory().NewDoubleSpin(X0, Y0, X1, 0.0, step, min, max, allow_edits, DefaultFont(), CLR_GRAY)
        );

        return retval.release();
    }

    MakeWndResult* Make_overlay(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::name_t placement = key_place_overlay;

        get_value(params, key_placement, placement);

        if (placement == key_place_row || placement == key_place_column) {
            std::string placement_ = key_place_overlay.name_m;
            throw adobe::stream_error_t(placement_ + " placement is not compatible with overlay",
                                        position);
        }

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(new Overlay);

        return retval.release();
    }

    MakeWndResult* Make_panel(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t value;
        adobe::name_t bind;
        adobe::name_t placement = key_place_column;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_value, value);
        get_value(params, adobe::key_bind, bind);
        get_value(params, key_placement, placement);

        if (name.empty())
            throw adobe::stream_error_t("panel requires a name parameter", position);

        if (placement == key_place_overlay)
            throw adobe::stream_error_t("place_overlay placement is not compatible with panel", position);

        Orientation orientation = placement == key_place_column ? VERTICAL : HORIZONTAL;

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(new Panel(orientation, name));

        return retval.release();
    }

    MakeWndResult* Make_layout(adobe::name_t wnd_type,
                               const adobe::dictionary_t& params,
                               const adobe::line_position_t& position)
    {
        adobe::name_t placement = key_place_overlay;

        if (get_value(params, key_placement, placement)) {
            if (!(placement == key_place_row && wnd_type == name_row ||
                  placement == key_place_column && wnd_type == name_column)) {
                std::string placement_ = key_place_overlay.name_m;
                throw adobe::stream_error_t(
                    placement_ + " placement is not compatible with " + wnd_type.c_str(),
                    position
                );
            }
        }

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params));

        retval->m_wnds.push_back(new Layout(X0, Y0, X1, Y1, 1, 1, 1, retval->m_margin));

        return retval.release();
    }

#if INSTRUMENT_WINDOW_CREATION
    int indent = 0;
#endif

    MakeWndResult* MakeWnd(adobe::name_t wnd_type,
                           const adobe::dictionary_t& params,
                           const adobe::line_position_t& position)
    {
        using namespace adobe;

#if INSTRUMENT_WINDOW_CREATION
#define IF_CASE(x) if (wnd_type == name_##x)                            \
        {                                                               \
            std::cout << std::string(indent * 4, ' ') << "    Make_"#x"(" << wnd_type << ")\n"; \
            MakeWndResult* retval = Make_##x(params, position);         \
            std::cout << std::string(indent * 4, ' ') << "    [ ";      \
            for (std::size_t i = 0; i < retval->m_wnds.size(); ++i) {   \
                std::cout << &retval->m_wnds[i] << " ";                 \
            }                                                           \
            std::cout << "]\n";                                         \
            return retval;                                              \
        }
#else
#define IF_CASE(x) if (wnd_type == name_##x) { return Make_##x(params, position); }
#endif

        IF_CASE(dialog)
        else IF_CASE(button)
        else IF_CASE(checkbox)
        else IF_CASE(display_number)
        else IF_CASE(edit_number)
        else IF_CASE(edit_text)
        else IF_CASE(image)
        else IF_CASE(popup)
        else IF_CASE(radio_button)
        else IF_CASE(radio_button_group)
        else IF_CASE(slider)
        else IF_CASE(tab_group)
        else IF_CASE(static_text)
        else IF_CASE(menu_bar)
        else IF_CASE(int_spin)
        else IF_CASE(double_spin)
        else IF_CASE(overlay)
        else IF_CASE(panel)
        else if (wnd_type == name_row || wnd_type == name_column) {
#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "    Make_layout(" << wnd_type << ", " << params << ", ...)\n";
            MakeWndResult* retval = Make_layout(wnd_type, params, position);
            std::cout << std::string(indent * 4, ' ') << "    [ ";
            for (std::size_t i = 0; i < retval->m_wnds.size(); ++i) {
                std::cout << &retval->m_wnds[i] << " ";
            }
            std::cout << "]\n";
            return retval;
#else
            return Make_layout(wnd_type, params, position);
#endif
        } else {
            std::string wnd_type_ = wnd_type.c_str();
            throw adobe::stream_error_t(wnd_type_ + " is not a supported view type", position);
        }

#undef IF_CASE
    }

    bool IsContainer(adobe::name_t wnd_type)
    {
        using namespace adobe;

        return
            wnd_type == name_dialog ||
            wnd_type == name_radio_button_group ||
            wnd_type == name_tab_group ||
            wnd_type == name_overlay ||
            wnd_type == name_panel ||
            wnd_type == name_row ||
            wnd_type == name_column;
    }

    adobe::any_regular_t VariableLookup(const adobe::basic_sheet_t& layout_sheet, adobe::name_t name)
    {
        static bool s_once = true;
        static adobe::name_t s_reflected_names[] =
            {
                key_align_left,
                key_align_right,
                key_align_top,
                key_align_bottom,
                key_align_center,
                key_align_proportional,
                key_align_fill,
                key_place_row,
                key_place_column,
                key_place_overlay
            };
        static std::pair<adobe::name_t*, adobe::name_t*> s_reflected_names_range;

        if (s_once) {
            adobe::sort(s_reflected_names);
            s_reflected_names_range.first = boost::begin(s_reflected_names);
            s_reflected_names_range.second = boost::end(s_reflected_names);
            s_once = false;
        }

        adobe::name_t* it =
            std::lower_bound(s_reflected_names_range.first, s_reflected_names_range.second, name);

        if (it != s_reflected_names_range.second && *it == name)
            return adobe::any_regular_t(name);

        return layout_sheet[name];
    }

}

struct EveLayout::Impl
{
    Impl(adobe::sheet_t& sheet) :
        m_sheet(sheet),
        m_current_nested_view(0),
        m_wnd(0)
        { m_lookup.attach_to(m_evaluator); }

    ~Impl()
        { delete m_wnd; }

    adobe::dictionary_t Contributing() const
        { return m_sheet.contributing(); }

    void Print(std::ostream& os) const
        {
            os << "layout name_ignored\n"
               << "{\n";
            for (std::size_t i = 0; i < m_added_cells.size(); ++i) {
                const AddedCellSet& cell_set = m_added_cells[i];
                if (i)
                    os << '\n';
                switch (cell_set.m_access) {
                case adobe::eve_callback_suite_t::constant_k: os << "constant:\n"; break;
                case adobe::eve_callback_suite_t::interface_k: os << "interface:\n"; break;
                }
                for (std::size_t j = 0; j < cell_set.m_cells.size(); ++j) {
                    const CellParameters& params = cell_set.m_cells[j];
                    // TODO: print detailed comment
                    os << "    " << params.m_name << " : "
                       << WriteExpression(params.m_initializer) << ";\n";
                    // TODO: print brief comment
                }
            }
            os << "    view ";
            PrintNestedView(os, m_nested_views, 1);
            os << "}\n";
        }

    adobe::eve_callback_suite_t BindCallbacks()
        {
            adobe::eve_callback_suite_t retval;
            m_evaluator.set_variable_lookup(
                boost::bind(VariableLookup, boost::cref(m_layout_sheet), _1)
            );
            retval.add_view_proc_m =
                boost::bind(&EveLayout::Impl::AddView, this, _1, _2, _3, _4, _5, _6);
            retval.add_cell_proc_m =
                boost::bind(&EveLayout::Impl::AddCell, this, _1, _2, _3, _4, _5, _6);
            return retval;
        }

    void AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                 adobe::name_t name,
                 const adobe::line_position_t& position,
                 const adobe::array_t& initializer,
                 const std::string& brief,
                 const std::string& detailed)
        {
            if (m_added_cells.empty() || m_added_cells.back().m_access != type)
                m_added_cells.push_back(AddedCellSet(type));

            m_added_cells.back().m_cells.push_back(
                CellParameters(type, name, position, initializer, brief, detailed)
            );

            m_evaluator.evaluate(initializer);
            adobe::any_regular_t value(m_evaluator.back());
            m_evaluator.pop_back();

            if (type == adobe::eve_callback_suite_t::constant_k)
                m_layout_sheet.add_constant(name, value);
            else if (type == adobe::eve_callback_suite_t::interface_k)
                m_layout_sheet.add_interface(name, value);
            else
                assert(0 && "Cell type not supported");
        }

    boost::any AddView(const boost::any& parent_,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed)
        {
            ViewParameters params(parent_, position, name, parameters, brief, detailed);

            // TODO: Don't do this -- instead, just generate a unique integer
            // to allow the nesting logic to work right.  The actual creation
            // of all the Wnds in the layout will be postponed until the end.
            Wnd* parent = boost::any_cast<Wnd*>(parent_);

            if (!m_current_nested_view) {
                m_nested_views = NestedViews(params, 0);
                m_current_nested_view = &m_nested_views;
            } else {
                while (boost::any_cast<Wnd*>(m_current_nested_view->m_view_parameters.m_parent) !=
                       parent &&
                       m_current_nested_view->m_nested_view_parent) {
                    m_current_nested_view = m_current_nested_view->m_nested_view_parent;
                }
                assert(m_current_nested_view);
                bool is_container = IsContainer(name);
                if (is_container)
                    params.m_parent = ++parent;
                m_current_nested_view->m_children.push_back(NestedViews(params, m_current_nested_view));
                if (is_container)
                    m_current_nested_view = &m_current_nested_view->m_children.back();
            }

            return parent;
        }

    struct CellParameters
    {
        CellParameters(adobe::eve_callback_suite_t::cell_type_t type,
                       adobe::name_t name,
                       const adobe::line_position_t& position,
                       const adobe::array_t& initializer,
                       const std::string& brief,
                       const std::string& detailed) :
            m_type(type),
            m_name(name),
            m_position(position),
            m_initializer(initializer),
            m_brief(brief),
            m_detailed(detailed)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_type;
        adobe::name_t m_name;
        adobe::line_position_t m_position;
        adobe::array_t m_initializer;
        std::string m_brief;
        std::string m_detailed;
    };

    struct AddedCellSet
    {
        AddedCellSet(adobe::eve_callback_suite_t::cell_type_t access) :
            m_access(access)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_access;
        std::vector<CellParameters> m_cells;
    };

    typedef std::vector<AddedCellSet> AddedCells;

    struct ViewParameters
    {
        ViewParameters() {}
        ViewParameters(const boost::any& parent,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed) :
            m_parent(parent),
            m_position(position),
            m_name(name),
            m_parameters(parameters),
            m_brief(brief),
            m_detailed(detailed)
            {}
        boost::any m_parent;
        adobe::line_position_t m_position;
        adobe::name_t m_name;
        adobe::array_t m_parameters;
        std::string m_brief;
        std::string m_detailed;
    };

    struct NestedViews
    {
        NestedViews() :
            m_nested_view_parent(0)
            {}
        NestedViews(const ViewParameters& view_parameters, NestedViews* parent) :
            m_view_parameters(view_parameters),
            m_nested_view_parent(parent)
            {}
        ViewParameters m_view_parameters;
        NestedViews* m_nested_view_parent;
        std::vector<NestedViews> m_children;
    };

    void AddChildrenToHorizontalLayout(Layout& layout,
                                       boost::ptr_vector<MakeWndResult>& children,
                                       const std::size_t MAX_SIZE)
    {
        std::vector<Y> max_single_row_heights(MAX_SIZE, Y0);
        Y max_all_rows_height = Y0;
        for (std::size_t i = 0; i < children.size(); ++i) {
            if (children[i].m_wnds.size()) {
                boost::ptr_vector<Wnd>::auto_type child_0 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 0);
                Pt min_usable_size = child_0->MinUsableSize();
                max_all_rows_height = std::max(max_all_rows_height, min_usable_size.y);
                layout.SetMinimumColumnWidth(i, min_usable_size.x);
                layout.Add(child_0.release(), 0, i * MAX_SIZE, 1, MAX_SIZE);
            } else {
                boost::ptr_vector<Wnd>::auto_type child_1 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 1);
                boost::ptr_vector<Wnd>::auto_type child_0 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 0);
                Pt min_usable_size = child_0->MinUsableSize();
                max_single_row_heights[0] =
                    std::max(max_single_row_heights[0], min_usable_size.y);
                layout.SetMinimumColumnWidth(i, min_usable_size.x);
                min_usable_size = child_1->MinUsableSize();
                max_single_row_heights[1] =
                    std::max(max_single_row_heights[1], min_usable_size.y);
                layout.SetMinimumColumnWidth(i, min_usable_size.x);
                layout.Add(child_0.release(), 0, i * 2 + 0);
                layout.Add(child_1.release(), 0, i * 2 + 1);
            }
        }
        Y_d summed_single_row_heights =
            1.0 * std::accumulate(max_single_row_heights.begin(), max_single_row_heights.end(), Y0);
        bool all_rows_height_larger = max_all_rows_height > summed_single_row_heights;
        Y_d difference = max_all_rows_height - summed_single_row_heights;
        for (std::size_t i = 0; i < max_single_row_heights.size(); ++i) {
            Y_d stretch =
                summed_single_row_heights ?
                max_single_row_heights[i] / summed_single_row_heights :
                Y_d(0.0);
            Y_d min_height = 1.0 * max_single_row_heights[i];
            if (all_rows_height_larger)
                min_height += difference * stretch;
            layout.SetRowStretch(i, Value(stretch));
            layout.SetMinimumRowHeight(i, Y(std::ceil(Value(min_height))));
        }
    }

    void AddChildrenToVerticalLayout(Layout& layout,
                                     boost::ptr_vector<MakeWndResult>& children,
                                     const std::size_t MAX_SIZE)
    {
        std::vector<X> max_single_column_widths(MAX_SIZE, X0);
        X max_all_columns_width = X0;
        for (std::size_t i = 0; i < children.size(); ++i) {
            if (children[i].m_wnds.size()) {
                boost::ptr_vector<Wnd>::auto_type child_0 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 0);
                Pt min_usable_size = child_0->MinUsableSize();
                max_all_columns_width = std::max(max_all_columns_width, min_usable_size.x);
                layout.SetMinimumRowHeight(i, min_usable_size.y);
                layout.Add(child_0.release(), i, 0, 1, MAX_SIZE);
            } else {
                boost::ptr_vector<Wnd>::auto_type child_1 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 1);
                boost::ptr_vector<Wnd>::auto_type child_0 =
                    children[i].m_wnds.release(children[i].m_wnds.begin() + 0);
                Pt min_usable_size = child_0->MinUsableSize();
                max_single_column_widths[0] =
                    std::max(max_single_column_widths[0], min_usable_size.x);
                layout.SetMinimumRowHeight(i, min_usable_size.y);
                min_usable_size = child_1->MinUsableSize();
                max_single_column_widths[1] =
                    std::max(max_single_column_widths[1], min_usable_size.x);
                layout.SetMinimumRowHeight(i, min_usable_size.y);
                layout.Add(child_0.release(), i, 0);
                layout.Add(child_1.release(), i, 1);
            }
        }
        X_d summed_single_column_widths =
            1.0 * std::accumulate(max_single_column_widths.begin(), max_single_column_widths.end(), X0);
        bool all_columns_width_larger = max_all_columns_width > summed_single_column_widths;
        X_d difference = max_all_columns_width - summed_single_column_widths;
        for (std::size_t i = 0; i < max_single_column_widths.size(); ++i) {
            X_d stretch =
                summed_single_column_widths ?
                max_single_column_widths[i] / summed_single_column_widths :
                X_d(0.0);
            X_d min_width = 1.0 * max_single_column_widths[i];
            if (all_columns_width_larger)
                min_width += difference * stretch;
            layout.SetColumnStretch(i, Value(stretch));
            layout.SetMinimumColumnWidth(i, X(std::ceil(Value(min_width))));
        }
    }

    void AddChildren(Wnd* wnd,
                     boost::ptr_vector<MakeWndResult>& children,
                     adobe::name_t placement,
                     const ViewParameters& wnd_view_params)
    {
        using namespace adobe;

#if INSTRUMENT_WINDOW_CREATION
        std::cout << std::string(indent * 4, ' ') << "    AddChildren(children = [ ";
        for (std::size_t i = 0; i < children.size(); ++i) {
            std::cout << "[ ";
            for (std::size_t j = 0; j < children[i].m_wnds.size(); ++j) {
                std::cout << &children[i].m_wnds[j] << " ";
            }
            std::cout << "] ";
        }
        std::cout << "])\n";
#endif

        if (wnd_view_params.m_name == name_dialog) {
            if (placement == key_place_overlay) {
                throw stream_error_t("place_overlay placement is not compatible with dialog",
                                     wnd_view_params.m_position);
            }
            Orientation orientation = (placement == key_place_row) ? HORIZONTAL : VERTICAL;
            const std::size_t MAX_SIZE =
                std::max_element(children.begin(), children.end(), MakeWndResultLess())->m_wnds.size();
            assert(MAX_SIZE == 1u || MAX_SIZE == 2u);
            std::auto_ptr<Layout>
                layout(new Layout(X0, Y0, X1, Y1,
                                  orientation == VERTICAL ? children.size() : 1,
                                  orientation == VERTICAL ? MAX_SIZE : children.size() * MAX_SIZE));
            if (orientation == VERTICAL)
                AddChildrenToVerticalLayout(*layout, children, MAX_SIZE);
            else
                AddChildrenToHorizontalLayout(*layout, children, MAX_SIZE);
            wnd->SetLayout(layout.release());
        } else if (wnd_view_params.m_name == name_radio_button_group) {
            RadioButtonGroup* radio_button_group = boost::polymorphic_downcast<RadioButtonGroup*>(wnd);
            for (std::size_t i = 0; i < children.size(); ++i) {
                assert(children[i].m_wnds.size() == 1u);
                StateButton* state_button = dynamic_cast<StateButton*>(&children[i].m_wnds[0]);
                if (!state_button) {
                    throw stream_error_t("non-radio_buttons are not compatible with radio_button_group",
                                         wnd_view_params.m_position);
                }
                children[i].m_wnds.release(children[i].m_wnds.begin() + 0).release();
                radio_button_group->AddButton(state_button);
            }
        } else if (wnd_view_params.m_name == name_tab_group) {
            TabWnd* tab_wnd = boost::polymorphic_downcast<TabWnd*>(wnd);
            assert(children[0].m_wnds.size() == 1u);
            for (std::size_t i = 0; i < children.size(); ++i) {
                assert(children[i].m_wnds.size() == 1u);
                Panel* panel = dynamic_cast<Panel*>(&children[i].m_wnds[0]);
                if (!panel) {
                    throw stream_error_t("non-panels are not compatible with tab_group",
                                         wnd_view_params.m_position);
                }
                children[i].m_wnds.release(children[i].m_wnds.begin() + 0).release();
                tab_wnd->AddWnd(panel, panel->Name());
            }
        } else if (wnd_view_params.m_name == name_overlay) {
            Overlay* overlay = boost::polymorphic_downcast<Overlay*>(wnd);
            assert(children[0].m_wnds.size() == 1u);
            for (std::size_t i = 0; i < children.size(); ++i) {
                assert(children[i].m_wnds.size() == 1u);
                Panel* panel = dynamic_cast<Panel*>(&children[i].m_wnds[0]);
                if (!panel) {
                    throw stream_error_t("non-panels are not compatible with overlay",
                                         wnd_view_params.m_position);
                }
                children[i].m_wnds.release(children[i].m_wnds.begin() + 0).release();
                overlay->AddPanel(panel);
            }
        } else if (wnd_view_params.m_name == name_panel) {
            Panel* panel = boost::polymorphic_downcast<Panel*>(wnd);
            for (std::size_t i = 0; i < children.size(); ++i) {
                assert(children[i].m_wnds.size() == 1u);
                panel->Add(children[i].m_wnds.release(children[i].m_wnds.begin() + 0).release());
            }
        } else if (wnd_view_params.m_name == name_row) {
            Layout* layout = boost::polymorphic_downcast<Layout*>(wnd);
            const std::size_t MAX_SIZE =
                std::max_element(children.begin(), children.end(), MakeWndResultLess())->m_wnds.size();
            assert(MAX_SIZE == 1u || MAX_SIZE == 2u);
            layout->ResizeLayout(1, children.size() * MAX_SIZE);
            AddChildrenToHorizontalLayout(*layout, children, MAX_SIZE);
        } else if (wnd_view_params.m_name == name_column) {
            Layout* layout = boost::polymorphic_downcast<Layout*>(wnd);
            const std::size_t MAX_SIZE =
                std::max_element(children.begin(), children.end(), MakeWndResultLess())->m_wnds.size();
            assert(MAX_SIZE == 1u || MAX_SIZE == 2u);
            layout->ResizeLayout(children.size(), MAX_SIZE);
            AddChildrenToVerticalLayout(*layout, children, MAX_SIZE);
        } else {
            throw stream_error_t("attempted to attach children to a non-container",
                                 wnd_view_params.m_position);
        }
    }

    Wnd& Finish()
        {
            std::auto_ptr<MakeWndResult> dialog(CreateChild(m_nested_views));
            m_wnd = dialog->m_wnds.release(dialog->m_wnds.begin() + 0).release();
#if INSTRUMENT_CREATED_LAYOUT
            DumpLayout(m_wnd);
#endif
            return *m_wnd;
        }

    MakeWndResult* CreateChild(const NestedViews& nested_view)
        {
            const ViewParameters& view_params = nested_view.m_view_parameters;

#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "CreateChild(" << view_params.m_name << ")\n";
#endif

            m_evaluator.evaluate(view_params.m_parameters);
            adobe::dictionary_t parameters(move(m_evaluator.back().cast<adobe::dictionary_t>()));
            m_evaluator.pop_back();

#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "    MakeWnd(" << view_params.m_name << ")\n";
#endif
            std::auto_ptr<MakeWndResult> retval(
                MakeWnd(view_params.m_name, parameters, view_params.m_position)
            );

#if INSTRUMENT_WINDOW_CREATION
            ++indent;
#endif
            boost::ptr_vector<MakeWndResult> children;
            for (std::size_t i = 0; i < nested_view.m_children.size(); ++i) {
                children.push_back(CreateChild(nested_view.m_children[i]));
            }
#if INSTRUMENT_WINDOW_CREATION
            --indent;
#endif

            assert(children.empty() || IsContainer(view_params.m_name));

            adobe::name_t placement;
            get_value(parameters, key_placement, placement);

            if (!children.empty()) {
                assert(retval->m_wnds.size() == 1u);
                AddChildren(&retval->m_wnds[0], children, placement, view_params);
            }

            return retval.release();
        }

    static void PrintNestedView(std::ostream& os, const NestedViews& nested_view, unsigned int indent)
        {
            const ViewParameters& params = nested_view.m_view_parameters;
            // TODO: print detailed comment
            std::string initial_indent(indent * 4, ' ');
            if (indent == 1u)
                initial_indent.clear();
            std::string param_string = WriteExpression(params.m_parameters);
            os << initial_indent << params.m_name << '('
               << param_string.substr(1, param_string.size() - 3)
               << ')';
            if (nested_view.m_children.empty()) {
                if (indent  == 1u) {
                    os << "\n" // TODO: print brief comment
                       << "    {}\n";
                } else {
                    os << ";\n"; // TODO: print brief comment
                }
            } else {
                // TODO: print brief comment
                os << '\n'
                   << std::string(indent * 4, ' ') << "{\n";
                for (std::size_t i = 0; i < nested_view.m_children.size(); ++i) {
                    PrintNestedView(os, nested_view.m_children[i], indent + 1);
                }
                os << std::string(indent * 4, ' ') << "}\n";
            }
        }

    adobe::sheet_t& m_sheet;

    adobe::basic_sheet_t m_layout_sheet;
    adobe::virtual_machine_t m_evaluator;
    adobe::vm_lookup_t m_lookup;

    AddedCells m_added_cells;
    NestedViews m_nested_views;
    NestedViews* m_current_nested_view;

    Wnd* m_wnd;
};


////////////////////////////////////////////////////////////
// EveLayout                                              //
////////////////////////////////////////////////////////////
EveLayout::EveLayout(adobe::sheet_t& sheet) :
    m_impl(new Impl(sheet))
{}

EveLayout::~EveLayout()
{ delete m_impl; }

adobe::dictionary_t EveLayout::Contributing() const
{ return m_impl->Contributing(); }

void EveLayout::Print(std::ostream& os) const
{ m_impl->Print(os); }

adobe::eve_callback_suite_t EveLayout::BindCallbacks()
{ return m_impl->BindCallbacks(); }

void EveLayout::AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                        adobe::name_t name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& initializer,
                        const std::string& brief,
                        const std::string& detailed)
{ m_impl->AddCell(type, name, position, initializer, brief, detailed); }

boost::any EveLayout::AddView(const boost::any& parent,
                              const adobe::line_position_t& position,
                              adobe::name_t name,
                              const adobe::array_t& parameters,
                              const std::string& brief,
                              const std::string& detailed)
{ return m_impl->AddView(parent, position, name, parameters, brief, detailed); }

Wnd& EveLayout::Finish()
{ return m_impl->Finish(); }
