/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_edit_text.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/controller_concept.hpp>

#include <GG/GUI.h>
#include <GG/MultiEdit.h>
#include <GG/StyleFactory.h>


/*************************************************************************************************/

namespace {

/*************************************************************************************************/

const int gap = 4;

} // namespace

/*************************************************************************************************/

namespace adobe {

namespace implementation {

/*************************************************************************************************/

class EditTextFilter :
    public GG::Wnd
{
public:
    EditTextFilter(edit_text_t& edit_text) : m_edit_text(edit_text) {}

    virtual bool EventFilter(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::MouseWheel) {
                bool squelch;
                if (event.WheelMove() < 0)
                    m_edit_text.pre_edit_proc_m(std::string(30, 1), squelch);
                else if (0 < event.WheelMove())
                    m_edit_text.pre_edit_proc_m(std::string(31, 1), squelch);
                retval = true;
            } else if (event.Type() == GG::WndEvent::KeyPress) {
                bool nontext =
                    event.GetKey() == GG::GGK_HOME ||
                    event.GetKey() == GG::GGK_LEFT ||
                    event.GetKey() == GG::GGK_RIGHT ||
                    event.GetKey() == GG::GGK_END ||
                    event.GetKey() == GG::GGK_BACKSPACE ||
                    event.GetKey() == GG::GGK_DELETE ||
                    event.GetKey() == GG::GGK_RETURN ||
                    event.GetKey() == GG::GGK_KP_ENTER;

                if (nontext)
                    return false;

                bool squelch =
                    m_edit_text.rows_m == 1 &&
                    0 < m_edit_text.max_cols_m &&
                    static_cast<std::size_t>(m_edit_text.max_cols_m) <
                    m_edit_text.control_m->Length() + 1;

                if (squelch)
                    return true;

                std::string translated_code_point;
                GG::GetTranslatedCodePoint(event.GetKey(), event.KeyCodePoint(),
                                           event.ModKeys(), translated_code_point);

                std::string new_value = m_edit_text.control_m->Text() + translated_code_point;

                if (m_edit_text.pre_edit_proc_m)
                    m_edit_text.pre_edit_proc_m(new_value, squelch);

                if (squelch) {
                    retval = true;
                } else {
                    *m_edit_text.control_m += translated_code_point;
                    m_edit_text.value_m = new_value;
                    m_edit_text.post_edit_proc_m(new_value);
                    retval = true;
                }
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
    name_m(block.name_m, block.alt_text_m, 0, block.theme_m),
    alt_text_m(block.alt_text_m),
    field_text_m(),
    using_label_m(!block.name_m.empty()),
    rows_m(block.num_lines_m),
    cols_m(block.min_characters_m),
    max_cols_m(block.max_characters_m),
    scrollable_m(block.scrollable_m),
    password_m(block.password_m),
    edit_baseline_m(0),
    edit_height_m(0),
    static_baseline_m(0),
    static_height_m(0)
{ /* TODO: Address password_m == true. */ }

/****************************************************************************************************/
extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int rows);


void edit_text_t::measure(extents_t& result)
{
    assert(control_m);
    //
    // The calculate_edit_bounds function can figure out the size this edit box
    // should be, based on the number of rows and columns.
    //
    result = calculate_edit_bounds(control_m, cols_m, rows_m);
    //
    // Store the height and baseline so that we can correctly align the edit widget
    // in set_bounds.
    //
    edit_height_m = result.height();
    if (!result.vertical().guide_set_m.empty())
        edit_baseline_m = result.vertical().guide_set_m[0];
    //
    // If we have a label then we need to make extra space
    // for it.
    //
    if (!using_label_m) return;
    extents_t label_bounds;
    ::adobe::measure(get_label(), label_bounds);
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
    //
    // We use the height and baseline of the label to size and
    // align it in set_bounds.
    //
    static_height_m = label_bounds.height();
    static_baseline_m = label_bounds.vertical().guide_set_m[0];
}

/****************************************************************************************************/

void edit_text_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    place_data_t local_place_data(place_data);

    long baseline = local_place_data.vertical().guide_set_m[0];

    if (using_label_m)
    {
        //
        // We're using a label. We need to extract the label width from the
        // points of interest in the given geometry, and make sure that we
        // correctly translate the label for baseline alignment.
        //
        place_data_t    label_place_data;
        label_place_data.horizontal().position_m = left(local_place_data);
        label_place_data.vertical().position_m = top(local_place_data);

        //
        // We stored the height of the label in best_bounds. The width of
        // the label can be discovered via the first horizontal point of
        // interest.
        //
        height(label_place_data) = static_height_m;
        width(label_place_data) = local_place_data.horizontal().guide_set_m[0];
        //
        // Translate the label vertically for baseline alignment with the
        // edit widget. We stored the label's baseline in best_bounds.
        //
        label_place_data.vertical().position_m += baseline - static_baseline_m;
        place(get_label(), label_place_data);

        local_place_data.horizontal().position_m += width(label_place_data) + gap;
        width(local_place_data) -= width(label_place_data) + gap;
    }
    //
    // We might need to translate the edit widget vertically, for baseline
    // alignment.
    //
    local_place_data.vertical().position_m += baseline - edit_baseline_m;

    // REVISIT (thw) : Do we need to adapt the height for baseline alignment?

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

void edit_text_t::set_theme(theme_t theme)
{ theme_m = theme; }


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
extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int rows)
{
    extents_t result;

    assert(edit);
    assert(0 < cols);
    assert(0 < rows);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();

    GG::X horizontal_margin = edit->Width() - edit->ClientWidth();
    GG::Y vertical_margin = edit->Height() - edit->ClientHeight();

    result.width() =
        Value(horizontal_margin) + cols * Value(style->DefaultFont()->SpaceWidth());
    result.height() =
        Value(vertical_margin) + rows * Value(style->DefaultFont()->Lineskip());
    result.vertical().guide_set_m.push_back(
        Value(vertical_margin / 2 + style->DefaultFont()->Ascent())
    );

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

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    assert(0 < element.rows_m);
    if (element.rows_m == 1) {
        element.control_m = style->NewEdit(GG::X0, GG::Y0, GG::X(100), element.field_text_m,
                                           style->DefaultFont(), GG::CLR_GRAY);
    } else {
        GG::Flags<GG::MultiEditStyle> multi_edit_style = GG::MULTI_LINEWRAP;
        if (!element.scrollable_m)
            multi_edit_style |= GG::MULTI_NO_VSCROLL | GG::MULTI_NO_HSCROLL;
        element.control_m =
            style->NewMultiEdit(
                GG::X0, GG::Y0, GG::X(100),
                GG::Y(style->DefaultFont()->Lineskip() * static_cast<int>(element.rows_m + 1) - 1),
                element.field_text_m, style->DefaultFont(), GG::CLR_GRAY, multi_edit_style
            );
    }

    element.filter_m.reset(new implementation::EditTextFilter(element));
    element.control_m->InstallEventFilter(element.filter_m.get());

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

   return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
