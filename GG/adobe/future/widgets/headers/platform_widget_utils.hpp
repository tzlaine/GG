/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_UI_CORE_OS_UTILITIES_HPP
#define ADOBE_UI_CORE_OS_UTILITIES_HPP

/****************************************************************************************************/

#include <GG/ListBox.h>

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array_fwd.hpp>
#include <GG/adobe/dictionary_fwd.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/widget_attributes.hpp>
#include <GG/adobe/future/widgets/headers/popup_common_fwd.hpp>

#include <boost/filesystem/path.hpp>

#include <string>


namespace GG {
    struct Clr;
    class Control;
    class Edit;
    class Font;
    class MultiEdit;
    class StyleFactory;
    class SubTexture;
    class TextControl;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

typedef boost::function<
    void (name_t widget_type_name, name_t signal_name, name_t widget_id, const any_regular_t&)
> signal_notifier_t;

class sheet_t;

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

typedef GG::Wnd* platform_control_type;

/****************************************************************************************************/

void get_control_bounds(GG::Wnd* control, GG::Rect& bounds);

/****************************************************************************************************/

void set_control_bounds(GG::Wnd* control, const place_data_t& place_data);

/****************************************************************************************************/

template <typename T>
const std::string& get_field_text(T& x)
{ return get_field_text(x.control_m); }

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::Edit*>(GG::Edit*& x);
template <>
const std::string& get_field_text<GG::MultiEdit*>(GG::MultiEdit*& x);
template <>
const std::string& get_field_text<GG::TextControl*>(GG::TextControl*& x);

/****************************************************************************************************/

bool is_focused(GG::Wnd* w);

/****************************************************************************************************/

GG::StyleFactory& Factory();

/****************************************************************************************************/

boost::shared_ptr<GG::Font> DefaultFont();

/****************************************************************************************************/

GG::X CharWidth();

/****************************************************************************************************/

GG::Y CharHeight();

/****************************************************************************************************/

GG::Y StandardHeight();

/****************************************************************************************************/

GG::Y RowHeight();

/****************************************************************************************************/

GG::Pt NonClientSize(GG::Wnd& w);

/****************************************************************************************************/

dictionary_t color_dictionary(const GG::Clr& color);

/****************************************************************************************************/

bool get_color(const dictionary_t& parameters, name_t name, GG::Clr& color);

/****************************************************************************************************/

bool get_subtexture(const dictionary_t& parameters, name_t name, GG::SubTexture& subtexture);

/****************************************************************************************************/

bool get_subtexture(const any_regular_t& value, GG::SubTexture& subtexture);

/****************************************************************************************************/

GG::StateButtonStyle name_to_style(name_t name);

/****************************************************************************************************/

void replace_placeholder(array_t& expression, name_t name, const any_regular_t& value);

/****************************************************************************************************/

void replace_placeholders(array_t& expression,
                          const any_regular_t& _,
                          const any_regular_t& _1,
                          const any_regular_t& _2 = any_regular_t(),
                          const any_regular_t& _3 = any_regular_t(),
                          const any_regular_t& _4 = any_regular_t());

/****************************************************************************************************/

void handle_signal(signal_notifier_t signal_notifier,
                   name_t widget_name,
                   name_t signal_name,
                   name_t widget_id,
                   sheet_t& sheet,
                   name_t bind,
                   array_t expression,
                   const any_regular_t& _1,
                   name_t _1_name = name_t(),
                   const any_regular_t& _2 = any_regular_t(),
                   name_t _2_name = name_t(),
                   const any_regular_t& _3 = any_regular_t(),
                   name_t _3_name = name_t(),
                   const any_regular_t& _4 = any_regular_t(),
                   name_t _4_name = name_t());

/****************************************************************************************************/

void cell_and_expression(const any_regular_t& value, name_t& cell, array_t& expression);

/****************************************************************************************************/

GG::ListBox::Row* item_to_row(const dictionary_t& item,
                              const row_factory_t* row_factory,
                              const GG::Clr& default_item_color);

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void set_control_visible(GG::Wnd* control, bool make_visible);

/****************************************************************************************************/

bool context_menu(const GG::Pt& pt,
                  const name_t* first,
                  const name_t* last,
                  name_t& result);

/****************************************************************************************************/

modifiers_t convert_modifiers(GG::Flags<GG::ModKey> modifiers);

/****************************************************************************************************/

modifiers_t modifier_state();

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#ifndef ADOBE_THROW_LAST_ERROR
    #define ADOBE_THROW_LAST_ERROR adobe::implementation::throw_last_error_exception(__FILE__, __LINE__)
#endif

/****************************************************************************************************/

namespace GG {

/****************************************************************************************************/

// Note: This is a redeclaration of the function from EveGlue.h.

ListBox::Row* DefaultRowFactoryFunction(const adobe::dictionary_t& parameters);

/****************************************************************************************************/

} // namespace GG

/****************************************************************************************************/

#endif

/****************************************************************************************************/
