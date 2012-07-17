/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_edit_text.hpp>

#include <GG/adobe/controller_concept.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/ClrConstants.h>
#include <GG/GUI.h>
#include <GG/MultiEdit.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/utf8/checked.h>


/*************************************************************************************************/

namespace {

/*************************************************************************************************/

void edit_edited(adobe::edit_text_t& edit_text, const std::string& str)
{
    edit_text.value_m = str;

    edit_text.post_edit_proc_m(str);

    if (edit_text.edited_proc_m)
        edit_text.edited_proc_m(str);
}

/*************************************************************************************************/

void edit_focus_update(adobe::edit_text_t& edit_text, const std::string& str)
{
    if (edit_text.focus_update_proc_m)
        edit_text.focus_update_proc_m(str);
}

/*************************************************************************************************/

const int gap = 4;

} // namespace

/*************************************************************************************************/

namespace adobe {

namespace implementation {

/*************************************************************************************************/

class EditTextFilter :
    public GG::EventFilter
{
public:
    EditTextFilter(edit_text_t& edit_text) :
        m_edit_text(edit_text)
        {}

private:
    virtual bool FilterImpl(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::MouseWheel) {
                bool squelch;
                m_edit_text.pre_edit_proc_m(std::string(1, 0 < event.WheelMove() ? 30 : 31), squelch);
                retval = true;
            } else if (event.Type() == GG::WndEvent::MouseEnter) {
                boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
                GG::GUI::GetGUI()->PushCursor(style->GetCursor(GG::TEXT_CURSOR));
                retval = true;
            } else if (event.Type() == GG::WndEvent::MouseLeave) {
                GG::GUI::GetGUI()->PopCursor();
                retval = true;
            } else if (event.Type() == GG::WndEvent::KeyPress) {
                bool nontext =
                    event.GetKey() == GG::GGK_HOME ||
                    event.GetKey() == GG::GGK_LEFT ||
                    event.GetKey() == GG::GGK_RIGHT ||
                    event.GetKey() == GG::GGK_END ||
                    event.GetKey() == GG::GGK_PAGEUP ||
                    event.GetKey() == GG::GGK_PAGEDOWN ||
                    event.GetKey() == GG::GGK_BACKSPACE ||
                    event.GetKey() == GG::GGK_DELETE ||
                    event.GetKey() == GG::GGK_RETURN ||
                    event.GetKey() == GG::GGK_KP_ENTER;

                if (nontext)
                    return false;

                if (event.GetKey() == GG::GGK_UP || event.GetKey() == GG::GGK_DOWN) {
                    bool squelch;
                    m_edit_text.pre_edit_proc_m(std::string(1, event.GetKey() == GG::GGK_UP ? 30 : 31), squelch);
                    return true;
                }

                const std::string& text = m_edit_text.control_m->Text();

                bool squelch =
                    m_edit_text.rows_m == 1 &&
                    0 < m_edit_text.max_cols_m &&
                    static_cast<std::size_t>(m_edit_text.max_cols_m) <
                    utf8::distance(text.begin(), text.end()) + 1;

                if (squelch)
                    return true;

                std::string translated_code_point;
                GG::GetTranslatedCodePoint(event.GetKey(), event.KeyCodePoint(),
                                           event.ModKeys(), translated_code_point);

                std::string new_value = m_edit_text.control_m->Text() + translated_code_point;

                if (m_edit_text.pre_edit_proc_m)
                    m_edit_text.pre_edit_proc_m(new_value, squelch);

                if (squelch)
                    retval = true;
            }
            return retval;
        }

    edit_text_t& m_edit_text;
};

}

/*************************************************************************************************/

void edit_text_t::display(const model_type& value) // values that come in from Adam
{
    if (value != value_m)
        set_field_text(value_m = value);
}

/****************************************************************************************************/

edit_text_t::edit_text_t(const edit_text_ctor_block_t& block) :
    color_m(block.color_m),
    text_color_m(block.text_color_m),
    interior_color_m(block.interior_color_m),
    name_m(block.name_m, block.alt_text_m, 0, GG::FORMAT_LEFT | GG::FORMAT_TOP, block.label_color_m),
    alt_text_m(block.alt_text_m),
    field_text_m(),
    using_label_m(!block.name_m.empty()),
    rows_m(block.num_lines_m),
    cols_m(block.min_characters_m),
    max_cols_m(block.max_characters_m),
    read_only_m(block.read_only_m),
    terminal_style_m(block.terminal_style_m),
    wrap_m(block.wrap_m),
    scrollable_m(block.scrollable_m),
    password_m(block.password_m),
    signal_id_m(block.signal_id_m)
{}

/****************************************************************************************************/

extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int rows);

void edit_text_t::measure(extents_t& result)
{
    assert(control_m);
    //
    // The calculate_edit_bounds function can figure out the size this edit box
    // should be, based on the number of rows and columns.
    //
    result = calculate_edit_bounds(control_m, cols_m, original_height_m);
    //
    // If we have a label then we need to make extra space
    // for it.
    //
    if (!using_label_m)
        return;
    const boost::shared_ptr<GG::Font>& font = implementation::DefaultFont();
    extents_t label_bounds(measure_text(name_m.name_m, font));
    label_bounds.vertical().guide_set_m.push_back(Value(font->Ascent()));
    //
    // Make sure that the height can accomodate both the label
    // and the edit widget.
    //
    align_slices(result.vertical(), label_bounds.vertical());
    //
    // We put the label on the left side of the edit box, and
    // place a point of interest at the end of the label, so
    // that colon alignment can be performed.
    //

    result.width() += gap + label_bounds.width();
    result.horizontal().guide_set_m.push_back(label_bounds.width());
}

/****************************************************************************************************/

void edit_text_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    place_data_t local_place_data(place_data);

    if (using_label_m)
    {
        //
        // The vertical offset of the label is the geometry's
        // baseline - the label's baseline.
        //
        assert(place_data.vertical().guide_set_m.empty() == false);

        place_data_t label_place_data;
        label_place_data.horizontal().position_m = left(local_place_data);
        label_place_data.vertical().position_m = top(local_place_data);

        //
        // The width of the label is the first horizontal
        // point of interest.
        //
        assert(place_data.horizontal().guide_set_m.empty() == false);
        width(label_place_data) = place_data.horizontal().guide_set_m[0];
        height(label_place_data) = height(place_data);;

        //
        // Set the label dimensions.
        //
        place(get_label(), label_place_data);

        //
        // Now we need to adjust the position of the popup
        // widget.
        //
        long width = gap + adobe::width(label_place_data);
        local_place_data.horizontal().position_m += width;
        adobe::width(local_place_data) -= width;
    }

    implementation::set_control_bounds(control_m, local_place_data);
}

/****************************************************************************************************/

label_t& edit_text_t::get_label()
{ return name_m; }

/****************************************************************************************************/

void edit_text_t::enable(bool active)
{
    assert(control_m);

    control_m->Disable(!active);

    if (using_label_m) {
        using adobe::enable;
        enable(get_label(), active);
    }
}

/****************************************************************************************************/

void edit_text_t::set_field_text(const std::string& text)
{
    assert(control_m);
    control_m->SetText(text);
}

/****************************************************************************************************/

void edit_text_t::signal_pre_edit(edit_text_pre_edit_proc_t proc)
{
    assert(control_m);

    if (!pre_edit_proc_m)
        pre_edit_proc_m = proc;
}

/****************************************************************************************************/

void edit_text_t::monitor(setter_type proc)
{
    if (!post_edit_proc_m)
        post_edit_proc_m = proc;
}

/****************************************************************************************************/
/// This function is used by the edit widget, as well as the popup widget
/// (which contains an edit widget).
///
/// \param  control the edit control to obtain the best bounds for.
/// \param  cols    the number of columns (or characters across) the edit control should contain
/// \param  rows    the number of rows of text to contain.
///
/// \return the best bounds for the edit control, including baseline.
//
extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int original_height)
{
    extents_t result;

    assert(edit);
    assert(0 < cols);
    assert(0 < original_height);

    GG::Pt non_client_size = implementation::NonClientSize(*edit);

    result.width() = Value(cols * implementation::CharWidth() + non_client_size.x);
    result.height() = original_height;
    GG::Y baseline =
        (static_cast<int>(result.height()) - implementation::CharHeight()) / 2 +
        implementation::DefaultFont()->Ascent();
    result.vertical().guide_set_m.push_back(Value(baseline));

    return result;
}

/****************************************************************************************************/

template <>
platform_display_type insert<edit_text_t>(display_t&             display,
                                          platform_display_type& parent,
                                          edit_text_t&           element)
{
    if (element.using_label_m)
        insert(display, parent, element.get_label());

    assert(0 < element.rows_m);
    if (element.rows_m <= 1) {
        element.control_m =
            implementation::Factory().NewEdit(GG::X0, GG::Y0, GG::X1,
                                              "",
                                              implementation::DefaultFont(),
                                              element.color_m,
                                              element.text_color_m,
                                              element.interior_color_m);
    } else {
        GG::Y height =
            implementation::CharHeight() * static_cast<int>(element.rows_m) +
            static_cast<int>(2 * GG::MultiEdit::BORDER_THICK);
        GG::Flags<GG::MultiEditStyle> style;
        if (element.read_only_m)
            style |= GG::MULTI_READ_ONLY;
        if (element.terminal_style_m)
            style |= GG::MULTI_TERMINAL_STYLE;
        if (element.wrap_m)
            style |= GG::MULTI_WORDBREAK;
        if (!element.scrollable_m)
            style |= GG::MULTI_NO_SCROLL;
        element.control_m =
            implementation::Factory().NewMultiEdit(GG::X0, GG::Y0, GG::X1,
                                                   height,
                                                   "",
                                                   implementation::DefaultFont(),
                                                   element.color_m,
                                                   style,
                                                   element.text_color_m,
                                                   element.interior_color_m);
    }

    GG::Connect(element.control_m->EditedSignal,
                boost::bind(&edit_edited, boost::ref(element), _1));
    GG::Connect(element.control_m->FocusUpdateSignal,
                boost::bind(&edit_focus_update, boost::ref(element), _1));

    if (element.password_m)
        element.control_m->PasswordMode(true);

    element.original_height_m = Value(element.control_m->Height());

    element.filter_m.reset(new implementation::EditTextFilter(element));
    element.control_m->InstallEventFilter(element.filter_m.get());

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    element.color_proxy_m.initialize(
        boost::bind(&GG::Edit::SetColor, element.control_m, _1)
    );
    element.text_color_proxy_m.initialize(
        boost::bind(&GG::Edit::SetTextColor, element.control_m, _1)
    );
    element.interior_color_proxy_m.initialize(
        boost::bind(&GG::Edit::SetInteriorColor, element.control_m, _1)
    );

   return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
