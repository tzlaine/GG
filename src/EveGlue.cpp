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

#include <GG/EveGlue.h>

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/adobe/localization.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/future/widgets/headers/platform_window.hpp>

#include <boost/cast.hpp>
#include <boost/filesystem/fstream.hpp>


using namespace GG;

namespace {

    GG::DictionaryFunctions& GetDictionaryFunctions()
    {
        static GG::DictionaryFunctions retval;
        return retval;
    }

    GG::ArrayFunctions& GetArrayFunctions()
    {
        static GG::ArrayFunctions retval;
        return retval;
    }

    adobe::aggregate_name_t key_key = { "key" };
    adobe::aggregate_name_t key_state = { "state" };
    adobe::aggregate_name_t key_transform = { "transform" };
    adobe::aggregate_name_t key_fold = { "fold" };
    adobe::aggregate_name_t key_foldr = { "foldr" };

    // NOTE: Most GG-defined builtin functions are defined in
    // src/adobe/future/widgets/sources/GG/functions.cpp.  These (and all
    // future) higher-order functions must be defined here, since they need
    // access to the function lookup mechanism.

    adobe::any_regular_t transform(const adobe::vm_lookup_t& lookup, const adobe::array_t& arguments)
    {
        adobe::any_regular_t retval;

        if (arguments.size() != 2u)
            throw std::runtime_error("for_each() takes exactly 2 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

        adobe::name_t f;
        if (!arguments[1].cast(f))
            throw std::runtime_error("The second argument to for_each() must be the name of a function.");

        adobe::dictionary_t f_arguments;

        if (arguments[0].type_info() == adobe::type_info<adobe::dictionary_t>()) {
            const adobe::dictionary_t& sequence = arguments[0].cast<adobe::dictionary_t>();
            retval = adobe::any_regular_t(adobe::dictionary_t());
            adobe::dictionary_t& result_elements = retval.cast<adobe::dictionary_t>();
            for (adobe::dictionary_t::const_iterator
                     it = sequence.begin(), end_it = sequence.end();
                 it != end_it;
                 ++it) {
                f_arguments[key_key] = adobe::any_regular_t(it->first);
                f_arguments[adobe::key_value] = it->second;
                result_elements[it->first] = lookup.dproc(f, f_arguments);
            }
        } else if (arguments[0].type_info() == adobe::type_info<adobe::array_t>()) {
            const adobe::array_t& sequence = arguments[0].cast<adobe::array_t>();
            retval = adobe::any_regular_t(adobe::array_t());
            adobe::array_t& result_elements = retval.cast<adobe::array_t>();
            result_elements.reserve(sequence.size());
            for (adobe::array_t::const_iterator
                     it = sequence.begin(), end_it = sequence.end();
                 it != end_it;
                 ++it) {
                f_arguments[adobe::key_value] = *it;
                result_elements.push_back(lookup.dproc(f, f_arguments));
            }
        } else {
            f_arguments[adobe::key_value] = arguments[0];
            retval = lookup.dproc(f, f_arguments);
        }

        return retval;
    }

    template <typename Iter>
    void fold_dictionary_impl(const adobe::vm_lookup_t& lookup,
                              adobe::name_t f,
                              Iter it,
                              Iter end_it,
                              adobe::any_regular_t& retval)
    {
        adobe::dictionary_t f_arguments;
        for (; it != end_it; ++it) {
            f_arguments[key_state] = retval;
            f_arguments[key_key] = adobe::any_regular_t(it->first);
            f_arguments[adobe::key_value] = it->second;
            retval = lookup.dproc(f, f_arguments);
        }
    }

    template <typename Iter>
    void fold_array_impl(const adobe::vm_lookup_t& lookup,
                         adobe::name_t f,
                         Iter it,
                         Iter end_it,
                         adobe::any_regular_t& retval)
    {
        adobe::dictionary_t f_arguments;
        for (; it != end_it; ++it) {
            f_arguments[key_state] = retval;
            f_arguments[adobe::key_value] = *it;
            retval = lookup.dproc(f, f_arguments);
        }
    }

    adobe::any_regular_t fold(const adobe::vm_lookup_t& lookup, const adobe::array_t& arguments)
    {
        adobe::any_regular_t retval;

        if (arguments.size() != 3u)
            throw std::runtime_error("fold() takes exactly 3 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

        adobe::name_t f;
        if (!arguments[2].cast(f))
            throw std::runtime_error("The third argument to fold() must be the name of a function.");

        retval = arguments[1];

        if (arguments[0].type_info() == adobe::type_info<adobe::dictionary_t>()) {
            const adobe::dictionary_t& sequence = arguments[0].cast<adobe::dictionary_t>();
            fold_dictionary_impl(lookup, f, sequence.begin(), sequence.end(), retval);
        } else if (arguments[0].type_info() == adobe::type_info<adobe::array_t>()) {
            const adobe::array_t& sequence = arguments[0].cast<adobe::array_t>();
            fold_array_impl(lookup, f, sequence.begin(), sequence.end(), retval);
        } else {
            adobe::dictionary_t f_arguments;
            f_arguments[key_state] = retval;
            f_arguments[adobe::key_value] = arguments[0];
            retval = lookup.dproc(f, f_arguments);
        }

        return retval;
    }

    adobe::any_regular_t foldr(const adobe::vm_lookup_t& lookup, const adobe::array_t& arguments)
    {
        adobe::any_regular_t retval;

        if (arguments.size() != 3u)
            throw std::runtime_error("foldr() takes exactly 3 arguments; " + boost::lexical_cast<std::string>(arguments.size()) + " given.");

        adobe::name_t f;
        if (!arguments[2].cast(f))
            throw std::runtime_error("The third argument to foldr() must be the name of a function.");

        retval = arguments[1];

        if (arguments[0].type_info() == adobe::type_info<adobe::dictionary_t>()) {
            const adobe::dictionary_t& sequence = arguments[0].cast<adobe::dictionary_t>();
            fold_dictionary_impl(lookup, f, sequence.rbegin(), sequence.rend(), retval);
        } else if (arguments[0].type_info() == adobe::type_info<adobe::array_t>()) {
            const adobe::array_t& sequence = arguments[0].cast<adobe::array_t>();
            fold_array_impl(lookup, f, sequence.rbegin(), sequence.rend(), retval);
        } else {
            adobe::dictionary_t f_arguments;
            f_arguments[key_state] = retval;
            f_arguments[adobe::key_value] = arguments[0];
            retval = lookup.dproc(f, f_arguments);
        }

        return retval;
    }

    void AttachFunctions(const GG::DictionaryFunctions& dictionary_functions,
                         const GG::ArrayFunctions& array_functions,
                         adobe::modal_dialog_t* dialog)
    {
        for (GG::DictionaryFunctions::const_iterator
                 it = GetDictionaryFunctions().begin(), end_it = GetDictionaryFunctions().end();
             it != end_it;
             ++it) {
            dialog->vm_lookup_m.insert_dictionary_function(it->first, it->second);
        }

        for (GG::DictionaryFunctions::const_iterator
                 it = dictionary_functions.begin(), end_it = dictionary_functions.end();
             it != end_it;
             ++it) {
            dialog->vm_lookup_m.insert_dictionary_function(it->first, it->second);
        }

        for (GG::ArrayFunctions::const_iterator
                 it = GetArrayFunctions().begin(), end_it = GetArrayFunctions().end();
             it != end_it;
             ++it) {
            dialog->vm_lookup_m.insert_array_function(it->first, it->second);
        }

        for (GG::ArrayFunctions::const_iterator
                 it = array_functions.begin(), end_it = array_functions.end();
             it != end_it;
             ++it) {
            dialog->vm_lookup_m.insert_array_function(it->first, it->second);
        }

        dialog->vm_lookup_m.insert_array_function(
            key_transform,
            boost::bind(&transform, boost::cref(dialog->vm_lookup_m), _1)
        );

        dialog->vm_lookup_m.insert_array_function(
            key_fold,
            boost::bind(&fold, boost::cref(dialog->vm_lookup_m), _1)
        );

        dialog->vm_lookup_m.insert_array_function(
            key_foldr,
            boost::bind(&foldr, boost::cref(dialog->vm_lookup_m), _1)
        );
    }

    void SetWndRegionCursor(WndRegion region)
    {
        GUI& gui = *GG::GUI::GetGUI();
        switch (region) {
        case WR_TOP:
        case WR_BOTTOM: gui.PushCursor(gui.GetStyleFactory()->GetCursor(RESIZE_UP_DOWN_CURSOR)); break;
        case WR_MIDLEFT:
        case WR_MIDRIGHT: gui.PushCursor(gui.GetStyleFactory()->GetCursor(RESIZE_LEFT_RIGHT_CURSOR)); break;
        case WR_TOPLEFT:
        case WR_BOTTOMRIGHT: gui.PushCursor(gui.GetStyleFactory()->GetCursor(RESIZE_UL_LR_CURSOR)); break; break;
        case WR_TOPRIGHT:
        case WR_BOTTOMLEFT: gui.PushCursor(gui.GetStyleFactory()->GetCursor(RESIZE_LL_UR_CURSOR)); break;
        default:
        case WR_MIDDLE: break;
        }
    }

}

DefaultSignalHandler::HandlerKey::HandlerKey()
{}

DefaultSignalHandler::HandlerKey::HandlerKey(adobe::name_t widget_type,
                                             adobe::name_t signal,
                                             adobe::name_t widget_id) :
    m_widget_type(widget_type),
    m_signal(signal),
    m_widget_id(widget_id)
{}

bool DefaultSignalHandler::HandlerKey::matches(const HandlerKey& rhs) const
{
    return
        (m_widget_type == rhs.m_widget_type || rhs.m_widget_type == DefaultSignalHandler::any_widget_type) &&
        (m_signal == rhs.m_signal || rhs.m_signal == DefaultSignalHandler::any_signal) &&
        (m_widget_id == rhs.m_widget_id || rhs.m_widget_id == DefaultSignalHandler::any_widget_id);
}

bool DefaultSignalHandler::KeyEquals::operator()(const Handler& rhs)
{
    return
        m_lhs.m_widget_type == rhs.first.m_widget_type &&
        m_lhs.m_signal == rhs.first.m_signal &&
        m_lhs.m_widget_id == rhs.first.m_widget_id;
}

const adobe::aggregate_name_t DefaultSignalHandler::any_widget_type = { "any_widget_type" };
const adobe::aggregate_name_t DefaultSignalHandler::any_signal = { "any_signal" };
const adobe::aggregate_name_t DefaultSignalHandler::any_widget_id = { "any_widget_id" };

void DefaultSignalHandler::operator()(adobe::name_t widget_type,
                                      adobe::name_t signal,
                                      adobe::name_t widget_id,
                                      const adobe::any_regular_t& value) const
{
    HandlerKey key(widget_type, signal, widget_id);
    unsigned int best_match_wildcards = 3;
    std::size_t best_match_index = 0;
    for (std::size_t i = 0; i < m_handlers.size(); ++i) {
        if (key.matches(m_handlers[i].first)) {
            unsigned int wildcards =
                static_cast<int>(m_handlers[i].first.m_widget_type == DefaultSignalHandler::any_widget_type) +
                static_cast<int>(m_handlers[i].first.m_signal == DefaultSignalHandler::any_signal) +
                static_cast<int>(m_handlers[i].first.m_widget_id == DefaultSignalHandler::any_widget_id);
            if (wildcards <= best_match_wildcards) {
                best_match_index = i;
                best_match_wildcards = wildcards;
            }
        }
    }
    if (best_match_index < m_handlers.size())
        m_handlers[best_match_index].second(widget_type, signal, widget_id, value);
}

void DefaultSignalHandler::SetHandler(adobe::name_t widget_type,
                                      adobe::name_t signal,
                                      adobe::name_t widget_id,
                                      SignalHandler handler)
{
    HandlerKey key(widget_type, signal, widget_id);
    assert(std::find_if(m_handlers.begin(), m_handlers.end(), KeyEquals(key)) == m_handlers.end());
    m_handlers.push_back(std::make_pair(key, handler));
}

const unsigned int EveDialog::BEVEL = 2;
const int EveDialog::FRAME_WIDTH = 2;
const Pt EveDialog::BEVEL_OFFSET(X(EveDialog::FRAME_WIDTH), Y(EveDialog::FRAME_WIDTH));

EveDialog::EveDialog(adobe::window_t& imp, Clr color, Clr text_color) :
    Wnd(X0, Y0, X1, Y1, imp.flags_m),
    m_imp(imp),
    m_title(0),
    m_color(color),
    m_keyboard(0),
    m_prev_wnd_region(WR_NONE),
    m_left_button_down(false)
{
    if (!m_imp.name_m.empty()) {
        m_title = adobe::implementation::Factory().NewTextControl(
            BEVEL_OFFSET.x, BEVEL_OFFSET.y - adobe::implementation::CharHeight(),
            m_imp.name_m, adobe::implementation::DefaultFont(), text_color
        );
        AttachChild(m_title);
    }
}

Pt EveDialog::ClientUpperLeft() const
{ return UpperLeft() + BEVEL_OFFSET + Pt(X0, m_title ? m_title->Height() : Y0); }

Pt EveDialog::ClientLowerRight() const
{ return LowerRight() - BEVEL_OFFSET; }

WndRegion EveDialog::WindowRegion(const Pt& pt) const
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

adobe::name_t EveDialog::TerminatingAction() const
{ return m_eve_modal_dialog->result().terminating_action_m; }

const adobe::dictionary_t& EveDialog::Result() const
{ return m_eve_modal_dialog->result().command_m; }

void EveDialog::SizeMove(const Pt& ul, const Pt& lr)
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

    if (m_title) {
        Pt new_title_size((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
        m_title->Resize(new_title_size);
    }
}

void EveDialog::Render()
{ BeveledRectangle(UpperLeft(), LowerRight(), m_color, m_color, true, BEVEL); }

void EveDialog::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ m_left_button_down = true; }

void EveDialog::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    m_left_button_down = false;
    MouseLeave();
    MouseHere(pt, mod_keys);
}

void EveDialog::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{ LButtonUp(pt, mod_keys); }

void EveDialog::MouseEnter(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Resizable() && !m_left_button_down)
        SetWndRegionCursor(m_prev_wnd_region = WindowRegion(pt));
}

void EveDialog::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (Resizable() && !m_left_button_down) {
        WndRegion wnd_region = WindowRegion(pt);
        if (wnd_region != m_prev_wnd_region) {
            if (m_prev_wnd_region != WR_NONE && m_prev_wnd_region != WR_MIDDLE)
                GG::GUI::GetGUI()->PopCursor();
            SetWndRegionCursor(m_prev_wnd_region = wnd_region);
        }
    }
}

void EveDialog::MouseLeave()
{
    if (Resizable() && !m_left_button_down) {
        if (m_prev_wnd_region != WR_NONE && m_prev_wnd_region != WR_MIDDLE)
            GG::GUI::GetGUI()->PopCursor();
        m_prev_wnd_region = WR_NONE;
    }
}

void EveDialog::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    assert(m_keyboard);
    m_keyboard->dispatch(adobe::key_type(key, key_code_point),
                         true,
                         adobe::modifier_state());
}

void EveDialog::KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    assert(m_keyboard);
    m_keyboard->dispatch(adobe::key_type(key, key_code_point),
                         false,
                         adobe::modifier_state());
}

void EveDialog::SetKeyboard(adobe::keyboard_t& keyboard)
{ m_keyboard = &keyboard; }

void EveDialog::SetEveModalDialog(adobe::modal_dialog_t* modal_dialog)
{ m_eve_modal_dialog.reset(modal_dialog); }


// GG::DefaultRowFactoryFunction defined in platform_widget_utils.cpp

ModalDialogResult GG::ExecuteModalDialog(const boost::filesystem::path& eve_definition,
                                         const boost::filesystem::path& adam_definition,
                                         ButtonHandler button_handler,
                                         SignalHandler signal_handler/* = SignalHandler()*/,
                                         RowFactory row_factory/* = RowFactory()*/)
{
    return ExecuteModalDialog(eve_definition,
                              adam_definition,
                              DictionaryFunctions(),
                              ArrayFunctions(),
                              button_handler,
                              signal_handler,
                              row_factory);
}

ModalDialogResult GG::ExecuteModalDialog(const boost::filesystem::path& eve_definition,
                                         const boost::filesystem::path& adam_definition,
                                         const DictionaryFunctions& dictionary_functions,
                                         const ArrayFunctions& array_functions,
                                         ButtonHandler button_handler,
                                         SignalHandler signal_handler/* = SignalHandler()*/,
                                         RowFactory row_factory/* = RowFactory()*/)
{
    boost::filesystem::ifstream eve_stream(eve_definition);
    boost::filesystem::ifstream adam_stream(adam_definition);
    return ExecuteModalDialog(eve_stream,
                              eve_definition.string(),
                              adam_stream,
                              adam_definition.string(),
                              dictionary_functions,
                              array_functions,
                              button_handler,
                              signal_handler,
                              row_factory);
}

ModalDialogResult GG::ExecuteModalDialog(std::istream& eve_definition,
                                         const std::string& eve_filename,
                                         std::istream& adam_definition,
                                         const std::string& adam_filename,
                                         ButtonHandler button_handler,
                                         SignalHandler signal_handler/* = SignalHandler()*/,
                                         RowFactory row_factory/* = RowFactory()*/)
{
    return ExecuteModalDialog(eve_definition,
                              eve_filename,
                              adam_definition,
                              adam_filename,
                              DictionaryFunctions(),
                              ArrayFunctions(),
                              button_handler,
                              signal_handler,
                              row_factory);
}

ModalDialogResult GG::ExecuteModalDialog(std::istream& eve_definition,
                                         const std::string& eve_filename,
                                         std::istream& adam_definition,
                                         const std::string& adam_filename,
                                         const DictionaryFunctions& dictionary_functions,
                                         const ArrayFunctions& array_functions,
                                         ButtonHandler button_handler,
                                         SignalHandler signal_handler/* = SignalHandler()*/,
                                         RowFactory row_factory/* = RowFactory()*/)
{
    ModalDialogResult retval;

    std::auto_ptr<adobe::modal_dialog_t> dialog(new adobe::modal_dialog_t);

    dialog->input_m = adobe::dictionary_t();
    dialog->record_m = adobe::dictionary_t();
    dialog->display_state_m = adobe::dictionary_t();
    dialog->display_options_m = adobe::dialog_display_s;
    dialog->button_callback_m = button_handler;
    dialog->signal_notifier_m = signal_handler;
    dialog->row_factory_m = row_factory;
    dialog->working_directory_m = boost::filesystem::path();
    dialog->parent_m = 0;

    AttachFunctions(dictionary_functions, array_functions, dialog.get());

    std::auto_ptr<Wnd> w(dialog->init(eve_definition, eve_filename, adam_definition, adam_filename));
    EveDialog* gg_dialog = boost::polymorphic_downcast<EveDialog*>(w.get());
    gg_dialog->SetKeyboard(dialog->keyboard());
    adobe::dialog_result_t adobe_result = dialog->go();

    swap(adobe_result.command_m, retval.m_result);
    retval.m_terminating_action = adobe_result.terminating_action_m;

    return retval;
}

EveDialog* GG::MakeEveDialog(const boost::filesystem::path& eve_definition,
                             const boost::filesystem::path& adam_definition,
                             ButtonHandler button_handler,
                             SignalHandler signal_handler/* = SignalHandler()*/,
                             RowFactory row_factory/* = RowFactory()*/)
{
    return MakeEveDialog(eve_definition,
                         adam_definition,
                         DictionaryFunctions(),
                         ArrayFunctions(),
                         button_handler,
                         signal_handler,
                         row_factory);
}

EveDialog* GG::MakeEveDialog(const boost::filesystem::path& eve_definition,
                             const boost::filesystem::path& adam_definition,
                             const DictionaryFunctions& dictionary_functions,
                             const ArrayFunctions& array_functions,
                             ButtonHandler button_handler,
                             SignalHandler signal_handler/* = SignalHandler()*/,
                             RowFactory row_factory/* = RowFactory()*/)
{
    boost::filesystem::ifstream eve_stream(eve_definition);
    boost::filesystem::ifstream adam_stream(adam_definition);
    return MakeEveDialog(eve_stream,
                         eve_definition.string(),
                         adam_stream,
                         adam_definition.string(),
                         dictionary_functions,
                         array_functions,
                         button_handler,
                         signal_handler,
                         row_factory);
}

EveDialog* GG::MakeEveDialog(std::istream& eve_definition,
                             const std::string& eve_filename,
                             std::istream& adam_definition,
                             const std::string& adam_filename,
                             ButtonHandler button_handler,
                             SignalHandler signal_handler/* = SignalHandler()*/,
                             RowFactory row_factory/* = RowFactory()*/)
{
    return MakeEveDialog(eve_definition,
                         eve_filename,
                         adam_definition,
                         adam_filename,
                         DictionaryFunctions(),
                         ArrayFunctions(),
                         button_handler,
                         signal_handler,
                         row_factory);
}

EveDialog* GG::MakeEveDialog(std::istream& eve_definition,
                             const std::string& eve_filename,
                             std::istream& adam_definition,
                             const std::string& adam_filename,
                             const DictionaryFunctions& dictionary_functions,
                             const ArrayFunctions& array_functions,
                             ButtonHandler button_handler,
                             SignalHandler signal_handler/* = SignalHandler()*/,
                             RowFactory row_factory/* = RowFactory()*/)
{
    EveDialog* retval = 0;

    std::auto_ptr<adobe::modal_dialog_t> dialog(new adobe::modal_dialog_t);

    dialog->input_m = adobe::dictionary_t();
    dialog->record_m = adobe::dictionary_t();
    dialog->display_state_m = adobe::dictionary_t();
    dialog->display_options_m = adobe::dialog_display_s;
    dialog->button_callback_m = button_handler;
    dialog->signal_notifier_m = signal_handler;
    dialog->row_factory_m = row_factory;
    dialog->working_directory_m = boost::filesystem::path();
    dialog->parent_m = 0;

    AttachFunctions(dictionary_functions, array_functions, dialog.get());

    Wnd* w = dialog->init(eve_definition, eve_filename, adam_definition, adam_filename);
    retval = boost::polymorphic_downcast<EveDialog*>(w);

    retval->SetKeyboard(dialog->keyboard());
    retval->SetEveModalDialog(dialog.release());

    return retval;
}

void GG::RegisterDictionaryFunction(adobe::name_t function_name, const DictionaryFunction& function)
{
    assert(GetDictionaryFunctions().find(function_name) == GetDictionaryFunctions().end());
    GetDictionaryFunctions()[function_name] = function;
}

void GG::RegisterArrayFunction(adobe::name_t function_name, const ArrayFunction& function)
{
    assert(GetArrayFunctions().find(function_name) == GetArrayFunctions().end());
    GetArrayFunctions()[function_name] = function;
}

void GG::RegisterView(adobe::name_t name,
                      const MakeViewFunction& method,
                      bool container/* = false*/,
                      const adobe::layout_attributes_t& layout_attributes/* = adobe::layout_attributes_t()*/)
{ adobe::default_asl_widget_factory().reg(name, method, container, layout_attributes); }

void GG::RegisterLocalizationFunction(const LocalizationFunction& f)
{ adobe::localization_register(f); }
