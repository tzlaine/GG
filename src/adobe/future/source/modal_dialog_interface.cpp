/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/Wnd.h>

#include <GG/adobe/config.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/adam_evaluate.hpp>
#include <GG/adobe/adam_parser.hpp>
#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/platform_window.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/keyboard.hpp>

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/Wnd.h>


/****************************************************************************************************/

namespace GG {

/****************************************************************************************************/

    const int Window::FRAME_WIDTH = 2;
    const Pt Window::BEVEL_OFFSET(X(Window::FRAME_WIDTH), Y(Window::FRAME_WIDTH));

    Window::Window(adobe::window_t& imp) :
        Wnd(X0, Y0, X1, Y1, imp.flags_m),
        m_imp(imp),
        m_title(0)
    {
        if (!m_imp.name_m.empty()) {
            m_title = adobe::implementation::Factory().NewTextControl(
                BEVEL_OFFSET.x, BEVEL_OFFSET.y - adobe::implementation::CharHeight(),
                m_imp.name_m, adobe::implementation::DefaultFont()
            );
            AttachChild(m_title);
        }
    }

    Pt Window::ClientUpperLeft() const
    { return UpperLeft() + BEVEL_OFFSET + Pt(X0, m_title ? m_title->Height() : Y0); }

    Pt Window::ClientLowerRight() const
    { return LowerRight() - BEVEL_OFFSET; }

    WndRegion Window::WindowRegion(const Pt& pt) const
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

    void Window::SizeMove(const Pt& ul, const Pt& lr)
    {
        Wnd::SizeMove(ul, lr);

        Pt client_size = ClientSize();

        if (!m_imp.debounce_m && !m_imp.resize_proc_m.empty()) {
            m_imp.debounce_m = true;

            if (adobe::width(m_imp.place_data_m) != Value(client_size.x) ||
                adobe::height(m_imp.place_data_m) != Value(client_size.y)) {
                m_imp.resize_proc_m(Value(client_size.x), Value(client_size.y));

                adobe::width(m_imp.place_data_m) = Value(client_size.x);
                adobe::height(m_imp.place_data_m) = Value(client_size.y);
            }

            m_imp.debounce_m = false;
        }

        Pt new_title_size((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
        m_title->Resize(new_title_size);
    }

    void Window::Render()
    { BeveledRectangle(UpperLeft(), LowerRight(), CLR_GRAY, CLR_GRAY, true, BEVEL); }

    void Window::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
    {
        adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                          true,
                                          adobe::modifier_state(),
                                          adobe::any_regular_t(this));
    }

    void Window::KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
    {
        adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                          false,
                                          adobe::modifier_state(),
                                          adobe::any_regular_t(this));
    }

    void Window::SetEveModalDialog(adobe::modal_dialog_t* modal_dialog)
    { m_eve_modal_dialog.reset(modal_dialog); }

} // namespace GG

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

std::string getline(std::istream& stream)
{
    std::string result;

    result.reserve(128);

    while (true)
    {
        int c(stream.get());

        if (c == EOF || c == '\n')
        {
            break;
        }
        else if (c == '\r')
        {
            c = stream.get();

            if (c != '\n')
                stream.putback(c);

            break;
        }

        result += static_cast<char>(c);
    }

    return result;
}

/****************************************************************************************************/

std::string mdi_error_getline(std::istream& layout_definition, name_t, std::streampos line_start_position)
{
    std::streampos old_pos(layout_definition.tellg());

    layout_definition.clear();

    layout_definition.seekg(line_start_position);

    std::string result(getline(layout_definition));

    layout_definition.seekg(old_pos);

    return result;
}

/****************************************************************************************************/

modal_dialog_t::modal_dialog_t() :
    display_options_m(dialog_display_s),
    parent_m(platform_display_type()),
    root_behavior_m(false)
{ }

/****************************************************************************************************/

platform_display_type modal_dialog_t::init(std::istream& layout, std::istream& sheet)
{
    resource_context_t res_context(working_directory_m);

    vm_lookup_m.attach_to(sheet_m);
    vm_lookup_m.attach_to(sheet_m.machine_m);

    result_m = dialog_result_t();

    //
    // Parse the proprty model stream
    //

    try
    {
        parse( sheet, line_position_t( "Proprty model sheet definition" ), bind_to_sheet( sheet_m ) );
    }
    catch (const stream_error_t& error)
    {
        throw std::logic_error(format_stream_error(sheet, error));
    }
    catch (...)
    {
        throw; // here for completeness' sake
    }

    //
    // REVISIT (2006/09/28, fbrereto): The sheet initializers don't run until the first update().
    //                                 To keep them from blowing away your sets in a couple of
    //                                 lines we need to add an update before the first set()
    //                                 on the sheet (this will get fixed with the proprty model
    //                                 library rewrite so mark it with a revisit comment).
    //

    sheet_m.update();

    //
    // Set up the sheet with initial inputs, etc.
    //

    sheet_m.set(input_m);
    sheet_m.set(record_m);

    need_ui_m = false;

    sheet_m.update();

    //
    // Save off the contributing set for potential reset action later
    //

    contributing_m = sheet_m.contributing();

    //
    // Set up the callback functions now so the contributing mark is meaningful, then update again
    //

    try
    {
        name_t result_cell(static_name_t("result"));

        attach_view(assemblage_m, result_cell, *this, sheet_m);

        sheet_m.monitor_invariant_dependent(result_cell, boost::bind(&modal_dialog_t::monitor_invariant, boost::ref(*this), _1));
        sheet_m.monitor_contributing(result_cell, sheet_m.contributing(), boost::bind(&modal_dialog_t::monitor_record, boost::ref(*this), _1));
    }
    catch (...)
    {
        // result cell wasn't found. While this isn't a deal-breaker, it's not
        // going to do much in the way of getting results from this dialog.
    }

    sheet_m.update();

    if (display_options_m == dialog_never_display_s && need_ui_m)
        throw std::runtime_error("handle_dialog: Invalid command parameters and UI not permitted.");

    if ((display_options_m == dialog_no_display_s && need_ui_m) ||
        display_options_m == dialog_display_s)
    {
        line_position_t::getline_proc_t getline_proc(new line_position_t::getline_proc_impl_t(boost::bind(&mdi_error_getline, boost::ref(layout), _1, _2)));

        view_m.reset( make_view( static_name_t( "eve definition" ),
                                        getline_proc,
                                        layout,
                                        sheet_m,
                                        root_behavior_m,
                                        boost::bind(&modal_dialog_t::latch_callback, boost::ref(*this), _1, _2),
                                        size_normal_s,
                                        default_widget_factory_proc(),
                                        parent_m).release()
                                        );

        // Set up the view's sheet with display state values, etc.
        //
        view_m->layout_sheet_m.set(display_state_m);

        sheet_m.update();

        //
        // Show the GUI.
        //
        view_m->eve_m.evaluate(eve_t::evaluate_nested);
        view_m->show_window_m();
    }

    return view_m->root_display_m;
}

/****************************************************************************************************/

dialog_result_t modal_dialog_t::go()
{
    if ((display_options_m == dialog_no_display_s && need_ui_m) ||
        display_options_m == dialog_display_s)
    {
        platform_display_type dlg = view_m->root_display_m;
        dlg->Run();
        result_m.display_state_m = view_m->layout_sheet_m.contributing();
        view_m.reset(0);
    }

    return result_m;
}

/****************************************************************************************************/

void modal_dialog_t::latch_callback(name_t action, const any_regular_t& value)
try
{
    assert(view_m);
    assert(callback_m);

    if (action == static_name_t("reset"))
    {
        sheet_m.set(contributing_m);
        sheet_m.update();
    }
    else if (callback_m(action, value))
    {
        result_m.terminating_action_m = action;
        view_m->root_display_m->EndRun();
    }
}
catch(const std::exception& error)
{
    std::cerr << "Exception (modal_dialog_t::latch_callback) : " << error.what() << std::endl;
}
catch(...)
{
    std::cerr << "Unknown exception (modal_dialog_t::latch_callback)" << std::endl;
}

/****************************************************************************************************/

void modal_dialog_t::display(const model_type& value)
{
    result_m.command_m = value.cast<dictionary_t>();
}

/****************************************************************************************************/

void modal_dialog_t::monitor_record(const dictionary_t& record_info)
{
    result_m.record_m = record_info;
}

/****************************************************************************************************/

void modal_dialog_t::monitor_invariant(bool is_set)
{
    need_ui_m = !is_set;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
