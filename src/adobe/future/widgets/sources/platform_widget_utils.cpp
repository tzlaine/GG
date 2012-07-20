/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/localization.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <boost/cstdint.hpp>

#include <vector>

#include <GG/GUI.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/Texture.h>
#include <GG/dialogs/FileDlg.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

boost::filesystem::path to_path(const std::string& path_string)
{ return boost::filesystem::path(path_string); }

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

GG::StyleFactory& Factory()
{ return *GG::GUI::GetGUI()->GetStyleFactory(); }

/****************************************************************************************************/

boost::shared_ptr<GG::Font> DefaultFont()
{ return Factory().DefaultFont(); }

/****************************************************************************************************/

GG::X CharWidth()
{
    static GG::X retval = DefaultFont()->TextExtent("W").x;
    return retval;
}

/****************************************************************************************************/

GG::Y CharHeight()
{ return DefaultFont()->Lineskip(); }

/****************************************************************************************************/

GG::Y StandardHeight()
{ return CharHeight() * 3 / 2; }

/****************************************************************************************************/

GG::Pt NonClientSize(GG::Wnd& w)
{ return w.Size() - w.ClientSize(); }

/****************************************************************************************************/

GG::Y RowHeight()
{
    static GG::Y retval = GG::Y0;
    if (!retval) {
        const GG::Y DEFAULT_MARGIN(8); // DEFAULT_ROW_HEIGHT from ListBox.cpp, minus the default font's lineskip of 14
        retval = adobe::implementation::DefaultFont()->Lineskip() + DEFAULT_MARGIN;
    }
    return retval;
}

/****************************************************************************************************/

void set_control_alt_text(GG::Wnd* control, const std::string& alt_text)
{
    if (control->BrowseModes().empty())
        control->SetBrowseModeTime(100);
    if (!control->BrowseModes().front().wnd)
        control->SetBrowseInfoWnd(Factory().DefaultBrowseInfoWnd());
    control->SetBrowseText(alt_text);
    
}

/****************************************************************************************************/

void get_control_bounds(GG::Wnd* control, GG::Rect& bounds)
{
    assert(control);
    bounds = GG::Rect(control->UpperLeft(), control->LowerRight());
}

/****************************************************************************************************/

void set_control_bounds(GG::Wnd* control, const place_data_t& place_data)
{
    if (control) {
        GG::Pt ul(GG::X(left(place_data)), GG::Y(top(place_data)));
        control->SizeMove(ul,
                          ul + GG::Pt(GG::X(width(place_data)),
                                      GG::Y(height(place_data))));
    }
}

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::Edit*>(GG::Edit*& x)
{ return x->Text(); }

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::MultiEdit*>(GG::MultiEdit*& x)
{ return x->Text(); }

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::TextControl*>(GG::TextControl*& x)
{ return x->Text(); }

/****************************************************************************************************/

bool is_focused(GG::Wnd* w)
{ return GG::GUI::GetGUI()->FocusWnd() == w; }

/****************************************************************************************************/

bool get_color(const dictionary_t& parameters, name_t name, GG::Clr& color)
{
    any_regular_t color_;
    if (!get_value(parameters, name, color_))
        return false;

    if (!color_.cast<GG::Clr>(color))
        return false;

    return true;
}

/****************************************************************************************************/

bool get_localized_string(const dictionary_t& parameters, name_t key, std::string& value)
{
    dictionary_t::const_iterator i = parameters.find(key);
    if (i == parameters.end()) return false;
    bool retval = i->second.cast(value);
    if (retval)
        value = localization_invoke(value);
    return retval;
}

/****************************************************************************************************/

bool get_subtexture(const dictionary_t& parameters, name_t name, GG::SubTexture& subtexture)
{
    any_regular_t value;
    if (!get_value(parameters, name, value))
        return false;

    return get_subtexture(value, subtexture);
}

/****************************************************************************************************/

bool get_subtexture(const any_regular_t& value, GG::SubTexture& subtexture)
{
    bool retval = false;

    if (value.cast(subtexture)) {
        retval = true;
    } else {
        boost::shared_ptr<GG::Texture> texture;
        std::string texture_name;
        if (value.cast(texture_name)) {
            try {
                texture = GG::GUI::GetGUI()->GetTexture(texture_name);
                texture->SetFilters(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
            } catch (...) {}
            retval = true;
        } else if (value.cast(texture)) {
            retval = true;
        }
        subtexture = GG::SubTexture(texture);
    }

    return retval;
}

/****************************************************************************************************/

GG::StateButtonStyle name_to_style(adobe::name_t name)
{
    if (name == adobe::static_name_t("xbox"))
        return GG::SBSTYLE_3D_XBOX;
    else if (name == adobe::static_name_t("checkbox"))
        return GG::SBSTYLE_3D_CHECKBOX;
    else if (name == adobe::static_name_t("radio"))
        return GG::SBSTYLE_3D_RADIO;
    else if (name == adobe::static_name_t("button"))
        return GG::SBSTYLE_3D_BUTTON;
    else if (name == adobe::static_name_t("round_button"))
        return GG::SBSTYLE_3D_ROUND_BUTTON;
    else if (name == adobe::static_name_t("top_attached_tab"))
        return GG::SBSTYLE_3D_TOP_ATTACHED_TAB;
    else if (name == adobe::static_name_t("top_detached_tab"))
        return GG::SBSTYLE_3D_TOP_DETACHED_TAB;
    else throw std::runtime_error(adobe::make_string("Unknown StateButtonStyle name ", name.c_str()));
}

/****************************************************************************************************/

void replace_placeholder(array_t& expression, name_t name, const any_regular_t& value)
{
    for (std::size_t i = 0; i < expression.size(); ++i) {
        name_t element_name;
        if (expression[i].cast<name_t>(element_name) && element_name == name) {
            expression[i] = value;
            expression.erase(expression.begin() + i + 1);
        }
    }
}

/****************************************************************************************************/

void replace_placeholders(array_t& expression,
                          const any_regular_t& _,
                          const any_regular_t& _1,
                          const any_regular_t& _2/* = any_regular_t()*/,
                          const any_regular_t& _3/* = any_regular_t()*/,
                          const any_regular_t& _4/* = any_regular_t()*/)
{
    replace_placeholder(expression, static_name_t("_"), _);
    if (!empty(_1))
        replace_placeholder(expression, static_name_t("_1"), _1);
    if (!empty(_2))
        replace_placeholder(expression, static_name_t("_2"), _2);
    if (!empty(_3))
        replace_placeholder(expression, static_name_t("_3"), _3);
    if (!empty(_4))
        replace_placeholder(expression, static_name_t("_4"), _4);
}

/****************************************************************************************************/

void handle_signal(signal_notifier_t signal_notifier,
                   name_t widget_name,
                   name_t signal_name,
                   name_t widget_id,
                   sheet_t& sheet,
                   name_t bind,
                   array_t expression,
                   const any_regular_t& _1,
                   name_t _1_name/* = name_t()*/,
                   const any_regular_t& _2/* = any_regular_t()*/,
                   name_t _2_name/* = name_t()*/,
                   const any_regular_t& _3/* = any_regular_t()*/,
                   name_t _3_name/* = name_t()*/,
                   const any_regular_t& _4/* = any_regular_t()*/,
                   name_t _4_name/* = name_t()*/)
{
    if (!bind && !signal_notifier)
        return;

    any_regular_t _;
    {
        dictionary_t dict;

        std::size_t count = 0;
        if (!empty(_1)) {
            dict[_1_name] = _1;
            ++count;
        }
        if (!empty(_2)) {
            assert(_2_name);
            dict[_2_name] = _2;
            ++count;
        }
        if (!empty(_3)) {
            assert(_3_name);
            dict[_3_name] = _3;
            ++count;
        }
        if (!empty(_4)) {
            assert(_4_name);
            dict[_4_name] = _4;
            ++count;
        }

        if (count <= 1) {
            _ = _1;
        } else {
            assert(_1_name);
            _ = any_regular_t(dict);
        }
    }

    any_regular_t value;
    if (expression.empty()) {
        value = _;
    } else {
        replace_placeholders(expression, _, _1, _2, _3, _4);
        value = sheet.inspect(expression);
    }

    if (bind) {
        sheet.set(bind, value);
        sheet.update();
    } else if (signal_notifier) {
        signal_notifier(widget_name, signal_name, widget_id, value);
    }
}

/****************************************************************************************************/

void cell_and_expression(const any_regular_t& value, name_t& cell, array_t& expression)
{
    array_t cell_and_expression;
    value.cast<name_t>(cell);
    if (!cell && value.cast<array_t>(cell_and_expression)) {
        cell = cell_and_expression[0].cast<name_t>();
        const std::string& expression_string = cell_and_expression[1].cast<std::string>();
        expression = parse_adam_expression(expression_string);
    }
}

/****************************************************************************************************/

GG::ListBox::Row* item_to_row(const dictionary_t& item,
                              const row_factory_t* row_factory,
                              const GG::Clr& default_item_color)
{
    const bool contains_color = item.count(static_name_t("color"));
    dictionary_t colorful_dictionary;
    if (!contains_color) {
        colorful_dictionary = item;
        colorful_dictionary[static_name_t("color")] = any_regular_t(default_item_color);
    }
    const dictionary_t& dictionary = contains_color ? item : colorful_dictionary;
    name_t type;
    get_value(dictionary, static_name_t("type"), type);
    row_factory_t::const_iterator it;
    if (row_factory && (it = row_factory->find(type)) != row_factory->end())
        return it->second(dictionary);
    else
        return GG::DefaultRowFactoryFunction(dictionary);
}

/****************************************************************************************************/

void color_proxy_t::initialize(const setter_proc_t& setter_proc)
{
    assert(setter_proc);
    setter_m = setter_proc;
}

/****************************************************************************************************/

void color_proxy_t::display(const model_type& value)
{ setter_m(value); }

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void set_control_visible(GG::Wnd* control, bool make_visible)
{
    if (make_visible)
        control->Show();
    else
        control->Hide();
}

/****************************************************************************************************/

bool context_menu(const GG::Pt& pt,
                  const name_t* first,
                  const name_t* last,
                  name_t& result)
{
    GG::StyleFactory& style(implementation::Factory());
    assert(style.DefaultFont());

    int id = 1;
    GG::MenuItem items;
    for (const name_t* it = first; it != last; ++it, ++id) {
        items.next_level.push_back(
            GG::MenuItem(static_cast<std::string>(it->c_str()), id, false, false)
        );
    }

    boost::shared_ptr<GG::PopupMenu> popup =
        style.NewPopupMenu(pt.x, pt.y, style.DefaultFont(), items);
    int result_id = popup->Run();

    if (result_id)
        result = *(first + result_id - 1);

    return result_id;
}

/****************************************************************************************************/

modifiers_t convert_modifiers(GG::Flags<GG::ModKey> mods)
{
    modifiers_t result(modifiers_none_s);

#define MAP_GG_TO_ADOBE_MOD(gg_mod, adobe_mod) if (mods & gg_mod) result |= adobe_mod

    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LSHIFT, modifiers_left_shift_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RSHIFT, modifiers_right_shift_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LALT, modifiers_left_option_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RALT, modifiers_right_option_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LCTRL, modifiers_left_control_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RCTRL, modifiers_right_control_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_CAPS, modifiers_caps_lock_s);

#undef MAP_GG_TO_ADOBE_MOD

    return result;
}

/****************************************************************************************************/

modifiers_t modifier_state()
{ return convert_modifiers(GG::GUI::GetGUI()->ModKeys()); }

/****************************************************************************************************/

platform_display_type get_top_level_window(platform_display_type thing)
{ return thing->RootParent(); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

namespace GG {

/****************************************************************************************************/

ListBox::Row* DefaultRowFactoryFunction(const adobe::dictionary_t& parameters)
{
    GG::ListBox::Row* retval = new GG::ListBox::Row(GG::X1, adobe::implementation::RowHeight(), "");
    std::string name;
    adobe::get_value(parameters, adobe::static_name_t("name"), name);
    Clr color;
    adobe::implementation::get_color(parameters, adobe::static_name_t("color"), color);
    retval->push_back(
        adobe::implementation::Factory().NewTextControl(GG::X0, GG::Y0, name,
                                                        adobe::implementation::DefaultFont(),
                                                        color, GG::FORMAT_LEFT)
    );
    return retval;
}

} // namespace GG

/****************************************************************************************************/
