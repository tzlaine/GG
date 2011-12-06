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
#include <GG/adobe/future/modal_dialog_interface.hpp>
#include <GG/adobe/future/widgets/headers/platform_window.hpp>

#include <boost/cast.hpp>


using namespace GG;

namespace {

    typedef std::list<std::pair<adobe::name_t, DictionaryFunction> > DictionaryFunctionList;
    DictionaryFunctionList& DictionaryFunctions()
    {
        static DictionaryFunctionList retval;
        return retval;
    }

    typedef std::list<std::pair<adobe::name_t, ArrayFunction> > ArrayFunctionList;
    ArrayFunctionList& ArrayFunctions()
    {
        static ArrayFunctionList retval;
        return retval;
    }

}

const unsigned int EveDialog::BEVEL = 2;
const int EveDialog::FRAME_WIDTH = 2;
const Pt EveDialog::BEVEL_OFFSET(X(EveDialog::FRAME_WIDTH), Y(EveDialog::FRAME_WIDTH));

EveDialog::EveDialog(adobe::window_t& imp) :
    Wnd(X0, Y0, X1, Y1, imp.flags_m),
    m_imp(imp),
    m_title(0),
    m_keyboard(0)
{
    if (!m_imp.name_m.empty()) {
        m_title = adobe::implementation::Factory().NewTextControl(
            BEVEL_OFFSET.x, BEVEL_OFFSET.y - adobe::implementation::CharHeight(),
            m_imp.name_m, adobe::implementation::DefaultFont()
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

    Pt new_title_size((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
    m_title->Resize(new_title_size);
}

void EveDialog::Render()
{ BeveledRectangle(UpperLeft(), LowerRight(), CLR_GRAY, CLR_GRAY, true, BEVEL); }

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


ModalDialogResult GG::ExecuteModalDialog(std::istream& eve_definition,
                                         std::istream& adam_definition,
                                         ButtonHandler handler)
{
    ModalDialogResult retval;

    std::auto_ptr<adobe::modal_dialog_t> dialog(new adobe::modal_dialog_t);

    dialog->input_m = adobe::dictionary_t();
    dialog->record_m = adobe::dictionary_t();
    dialog->display_state_m = adobe::dictionary_t();
    dialog->display_options_m = adobe::dialog_display_s;
    dialog->callback_m = handler;
    dialog->working_directory_m = boost::filesystem::path();
    dialog->parent_m = 0;

    for (DictionaryFunctionList::const_iterator
             it = DictionaryFunctions().begin(), end_it = DictionaryFunctions().end();
         it != end_it;
         ++it) {
        dialog->vm_lookup_m.insert_dictionary_function(it->first, it->second);
    }

    for (ArrayFunctionList::const_iterator
             it = ArrayFunctions().begin(), end_it = ArrayFunctions().end();
         it != end_it;
         ++it) {
        dialog->vm_lookup_m.insert_array_function(it->first, it->second);
    }

    std::auto_ptr<Wnd> w(dialog->init(eve_definition, adam_definition));
    EveDialog* gg_dialog = boost::polymorphic_downcast<EveDialog*>(w.get());
    gg_dialog->SetKeyboard(dialog->keyboard());
    adobe::dialog_result_t adobe_result = dialog->go();

    swap(adobe_result.command_m, retval.m_result);
    retval.m_terminating_action = adobe_result.terminating_action_m;

    return retval;
}

EveDialog* GG::MakeEveDialog(std::istream& eve_definition,
                             std::istream& adam_definition,
                             ButtonHandler handler)
{
    EveDialog* retval = 0;

    std::auto_ptr<adobe::modal_dialog_t> dialog(new adobe::modal_dialog_t);

    dialog->input_m = adobe::dictionary_t();
    dialog->record_m = adobe::dictionary_t();
    dialog->display_state_m = adobe::dictionary_t();
    dialog->display_options_m = adobe::dialog_display_s;
    dialog->callback_m = handler;
    dialog->working_directory_m = boost::filesystem::path();
    dialog->parent_m = 0;

    for (DictionaryFunctionList::const_iterator
             it = DictionaryFunctions().begin(), end_it = DictionaryFunctions().end();
         it != end_it;
         ++it) {
        dialog->vm_lookup_m.insert_dictionary_function(it->first, it->second);
    }

    for (ArrayFunctionList::const_iterator
             it = ArrayFunctions().begin(), end_it = ArrayFunctions().end();
         it != end_it;
         ++it) {
        dialog->vm_lookup_m.insert_array_function(it->first, it->second);
    }

    Wnd* w = dialog->init(eve_definition, adam_definition);
    retval = boost::polymorphic_downcast<EveDialog*>(w);

    retval->SetKeyboard(dialog->keyboard());
    retval->SetEveModalDialog(dialog.release());

    return retval;
}

void GG::RegisterDictionaryFunction(adobe::name_t function_name, const DictionaryFunction& function)
{ DictionaryFunctions().push_back(std::make_pair(function_name, function)); }

void GG::RegisterArrayFunction(adobe::name_t function_name, const ArrayFunction& function)
{ ArrayFunctions().push_back(std::make_pair(function_name, function)); }

void GG::RegisterView(adobe::name_t name,
                      const MakeViewFunction& method,
                      bool container/* = false*/,
                      const adobe::layout_attributes_t& layout_attributes/* = adobe::layout_attributes_t()*/)
{ adobe::default_asl_widget_factory().reg(name, method, container, layout_attributes); }
