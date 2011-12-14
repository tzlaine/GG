// -*- C++ -*-
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

/** \file EveGlue.h \brief Contains the ExecuteModalDialog() and MakelDialog()
    functions and associated typs; these automate the parsing, instantiation,
    binding to GG Wnds, and evaluation of Adobe Adam- and Eve- based
    dialogs. */

#ifndef _EveGlue_h_
#define _EveGlue_h_

#include <GG/Wnd.h>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/layout_attributes.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>


namespace adobe {
    struct factory_token_t;
    struct keyboard_t;
    struct modal_dialog_t;
    struct widget_factory_t;
    struct widget_node_t;
    struct window_t;
    template <>
    platform_display_type insert<window_t>(display_t& display,
                                           platform_display_type& parent,
                                           window_t& element);
}

namespace GG {

class EveDialog;
class TextControl;

/** Contains the result of a modal dialog created by ExecuteModalDialog(). */
struct ModalDialogResult
{
    /** A map from adobe::name_t to value (adobe::any_regular_t) of the
        expected results.  The contents of m_results will depend on the value
        specified for "result" in the Adam script associated with the modal
        dialog. */
    adobe::dictionary_t m_result;

    /** Indicates the name of the action that terminated the dialog (e.g. "ok"
        or "cancel"). */
    adobe::name_t m_terminating_action;
};

/** The type of button click handler function expected by the UI creation
    functions.  Handlers accept the name of the \a action associated with the
    button click and the \a value emitted by the click as specified in the
    Adam and Eve scripts, and return true if the click should result in the
    closure of the dialog.  \see \ref eve_button_handler. */
typedef boost::function <bool (adobe::name_t, const adobe::any_regular_t&)> ButtonHandler;

/** The type of signal handler function (optionally) expected by the UI
    creation functions.  Handlers accept the name of the \a widget_type, the
    name of the \a signal, and the name of the particular widget emitting the
    signal, \a widget_id.  \see \ref eve_signal_handler. */
typedef boost::function<void (adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t&)> SignalHandler;

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  \see ButtonHandler. */
ModalDialogResult ExecuteModalDialog(const boost::filesystem::path& eve_definition,
                                     const boost::filesystem::path& adam_definition,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler());

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  \see ButtonHandler. */
ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                     const std::string& eve_filename,
                                     std::istream& adam_definition,
                                     const std::string& adam_filename,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  \see ButtonHandler. */
EveDialog* MakeEveDialog(const boost::filesystem::path& eve_definition,
                         const boost::filesystem::path& adam_definition,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  \see ButtonHandler. */
EveDialog* MakeEveDialog(std::istream& eve_definition,
                         const std::string& eve_filename,
                         std::istream& adam_definition,
                         const std::string& adam_filename,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler());

/** A GG Eve dialog that handles all the interaction with the Eve engine
    (e.g. relayout on resize).  Must be created via MakeEveDialog(). */
class EveDialog :
    public Wnd
{
public:
    virtual Pt ClientUpperLeft() const;
    virtual Pt ClientLowerRight() const;
    virtual WndRegion WindowRegion(const Pt& pt) const;

    /** Returns the action that terminated the execution of the dialog, or an
        empty adobe::name_t if the dialog has not yet been terminated. */
    adobe::name_t TerminatingAction() const;

    /** Returns the action that terminated the execution of the dialog.
        Results are undefined if TerminatingAction().empty() is true. */
    const adobe::dictionary_t& Result() const;

    virtual void SizeMove(const Pt& ul, const Pt& lr);
    virtual void Render();
    virtual void KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);
    virtual void KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);

private:
    EveDialog(adobe::window_t& imp, Clr color);

    void SetKeyboard(adobe::keyboard_t& keyboard);
    void SetEveModalDialog(adobe::modal_dialog_t* modal_dialog);

    adobe::window_t& m_imp;
    Clr m_color;
    TextControl* m_title;
    adobe::keyboard_t* m_keyboard;
    std::auto_ptr<adobe::modal_dialog_t> m_eve_modal_dialog;

    static const unsigned int BEVEL;
    static const int FRAME_WIDTH;
    static const Pt BEVEL_OFFSET;

    friend adobe::platform_display_type
    adobe::insert<adobe::window_t>(adobe::display_t&,
                                   adobe::platform_display_type&,
                                   adobe::window_t&);


    friend ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                                const std::string& eve_filename,
                                                std::istream& adam_definition,
                                                const std::string& adam_filename,
                                                ButtonHandler button_handler,
                                                SignalHandler signal_handler);

    friend EveDialog* MakeEveDialog(std::istream& eve_definition,
                                    const std::string& eve_filename,
                                    std::istream& adam_definition,
                                    const std::string& adam_filename,
                                    ButtonHandler button_handler,
                                    SignalHandler signal_handler);
};

/** The type of function used to evaluate named-parameter functions in Adam
    and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef boost::function<adobe::any_regular_t (const adobe::dictionary_t&)> DictionaryFunction;

/** The type of function used to evaluate positional-parameter functions in
    Adam and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef boost::function<adobe::any_regular_t (const adobe::array_t&)> ArrayFunction;

/** Registers user-defined function \a function, callable as a named-parameter
    function in Adam and Eve expressions.  \see \ref
    eve_adding_user_functions. */
void RegisterDictionaryFunction(adobe::name_t function_name, const DictionaryFunction& function);

/** Registers user-defined function \a function, callable as a
    positional-parameter function in Adam and Eve expressions.  \see \ref
    eve_adding_user_functions. */
void RegisterArrayFunction(adobe::name_t function_name, const ArrayFunction& function);

/** The type of function used to instantiate a user-defined Eve view.  \see
    \ref eve_adding_user_views. */
typedef boost::function<adobe::widget_node_t (const adobe::dictionary_t&,
                                              const adobe::widget_node_t&,
                                              const adobe::factory_token_t&,
                                              const adobe::widget_factory_t&)> MakeViewFunction;

/** Registers user-defined Eve view \a name, for use in Eve sheets.  \see \ref
    eve_adding_user_views. */
void RegisterView(adobe::name_t name,
                  const MakeViewFunction& method,
                  bool container = false,
                  const adobe::layout_attributes_t& layout_attributes = adobe::layout_attributes_t());

}

#endif
