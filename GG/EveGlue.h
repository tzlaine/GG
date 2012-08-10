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

#include <GG/ListBox.h>

#include <GG/FunctionParser.h>
#include <GG/adobe/adam_function.hpp>
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
typedef boost::function <
    bool (adobe::name_t, const adobe::any_regular_t&)
> ButtonHandler;

/** The type of signal handler function (optionally) expected by the UI
    creation functions.  Handlers accept the name of the \a widget_type, the
    name of the \a signal, and the name of the particular widget emitting the
    signal, \a widget_id.  \see \ref eve_signal_handler. */
typedef boost::function<
    void (adobe::name_t, adobe::name_t, adobe::name_t, const adobe::any_regular_t&)
> SignalHandler;

/** The type of ListBox::Row factory function, which may be used to create
    rows for both listboxes and popups.  Factory functions accept the
    dictionary of parameters to use to construct the row.  \see \ref
    eve_row_factory. */
typedef boost::function<
    ListBox::Row* (const adobe::dictionary_t&)
> RowFactoryFunction;

/** The type of ListBox::Row factory (optionally) expected by the UI creation
    functions, and used to create rows in both listboxes and popups.  \see
    \ref eve_row_factory. */
typedef std::map<adobe::name_t, RowFactoryFunction> RowFactory;

/** The default RowFactoryFunction, which is used to create rows in both
    listboxes and popups when constructing an item with no \a type parameter,
    when constructing an item with \a type parameter not found in the
    user-defined row factory, or when no user-defined row factory is given.
    The default factory function creates rows consisting of a single string
    (the \a name parameter), rendered in the color specified in the \a color
    parameter (if given).  \see \ref eve_row_factory. */
ListBox::Row* DefaultRowFactoryFunction(const adobe::dictionary_t& parameters);

/** The type of function used to evaluate named-parameter functions in Adam
    and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef boost::function<adobe::any_regular_t (const adobe::dictionary_t&)> DictionaryFunction;

/** The map of functions used to evaluate named-parameter functions in Adam
    and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef std::map<adobe::name_t, DictionaryFunction> DictionaryFunctions;

/** The type of function used to evaluate positional-parameter functions in
    Adam and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef boost::function<adobe::any_regular_t (const adobe::array_t&)> ArrayFunction;

/** The map of functions used to evaluate positional-parameter functions in
    Adam and Eve expressions.  \see \ref eve_adding_user_functions. */
typedef std::map<adobe::name_t, ArrayFunction> ArrayFunctions;


/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  \see ButtonHandler.  \see
    SignalHandler. */
ModalDialogResult ExecuteModalDialog(const boost::filesystem::path& eve_definition,
                                     const boost::filesystem::path& adam_definition,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler(),
                                     RowFactory row_factory = RowFactory());

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  The functions provided in \a
    dictionary_functions and \a array_functions will be available in the
    associated Adam and Eve scripts.  They will override any functions with
    the same name registered with RegisterDictionaryFunction() and
    RegisterArrayFunction(), respectively.  \see ButtonHandler.  \see
    SignalHandler. */
ModalDialogResult ExecuteModalDialog(const boost::filesystem::path& eve_definition,
                                     const boost::filesystem::path& adam_definition,
                                     const DictionaryFunctions& dictionary_functions,
                                     const ArrayFunctions& array_functions,
                                     const AdamFunctions& adam_functions,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler(),
                                     RowFactory row_factory = RowFactory());

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  \see ButtonHandler.  \see
    SignalHandler. */
ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                     const std::string& eve_filename,
                                     std::istream& adam_definition,
                                     const std::string& adam_filename,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler(),
                                     RowFactory row_factory = RowFactory());

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  The functions provided in \a
    dictionary_functions and \a array_functions will be available in the
    associated Adam and Eve scripts.  They will override any functions with
    the same name registered with RegisterDictionaryFunction() and
    RegisterArrayFunction(), respectively.  \see ButtonHandler.  \see
    SignalHandler. */
ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                     const std::string& eve_filename,
                                     std::istream& adam_definition,
                                     const std::string& adam_filename,
                                     const DictionaryFunctions& dictionary_functions,
                                     const ArrayFunctions& array_functions,
                                     const AdamFunctions& adam_functions,
                                     ButtonHandler button_handler,
                                     SignalHandler signal_handler = SignalHandler(),
                                     RowFactory row_factory = RowFactory());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  \see ButtonHandler.  \see SignalHandler. */
EveDialog* MakeEveDialog(const boost::filesystem::path& eve_definition,
                         const boost::filesystem::path& adam_definition,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler(),
                         RowFactory row_factory = RowFactory());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  The functions provided in \a dictionary_functions
    and \a array_functions will be available in the associated Adam and Eve
    scripts.  They will override any functions with the same name registered
    with RegisterDictionaryFunction() and RegisterArrayFunction(),
    respectively.  \see ButtonHandler.  \see SignalHandler. */
EveDialog* MakeEveDialog(const boost::filesystem::path& eve_definition,
                         const boost::filesystem::path& adam_definition,
                         const DictionaryFunctions& dictionary_functions,
                         const ArrayFunctions& array_functions,
                         const AdamFunctions& adam_functions,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler(),
                         RowFactory row_factory = RowFactory());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  \see ButtonHandler.  \see SignalHandler. */
EveDialog* MakeEveDialog(std::istream& eve_definition,
                         const std::string& eve_filename,
                         std::istream& adam_definition,
                         const std::string& adam_filename,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler(),
                         RowFactory row_factory = RowFactory());

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns an EveDialog.  The functions provided in \a dictionary_functions
    and \a array_functions will be available in the associated Adam and Eve
    scripts.  They will override any functions with the same name registered
    with RegisterDictionaryFunction() and RegisterArrayFunction(),
    respectively.  \see ButtonHandler.  \see SignalHandler. */
EveDialog* MakeEveDialog(std::istream& eve_definition,
                         const std::string& eve_filename,
                         std::istream& adam_definition,
                         const std::string& adam_filename,
                         const DictionaryFunctions& dictionary_functions,
                         const ArrayFunctions& array_functions,
                         const AdamFunctions& adam_functions,
                         ButtonHandler button_handler,
                         SignalHandler signal_handler = SignalHandler(),
                         RowFactory row_factory = RowFactory());

/** Usable as a SignalHandler, providing a convenient interface for handling a
    multitude of signals with separate SignalHandlers. */
class DefaultSignalHandler
{
public:
    /** Call operator.  If more than one match is found, due to the use of
        wildcards, the last handler added with SetHandler will be called. */
    void operator()(adobe::name_t widget_type,
                    adobe::name_t signal,
                    adobe::name_t widget_id,
                    const adobe::any_regular_t& value) const;

    /** Sets signal handler \a handler for signals associated with the given
        \a widget_type, \a signal, and \a widget_id.  Special values \a
        any_widget_type, \a any_signal, and \a any_widget_id can be provided
        for \a widget_type, \a signal, and \a widget_id, respectively; these
        act as wildcards when matching an emitted signal to a handler.
        Behavior is undefined if there is already a handler set associated
        with the given names. */
    void SetHandler(adobe::name_t widget_type,
                    adobe::name_t signal,
                    adobe::name_t widget_id,
                    SignalHandler handler);

    /** A special value that matches any widget type. */
    static const adobe::aggregate_name_t any_widget_type;

    /** A special value that matches any signal. */
    static const adobe::aggregate_name_t any_signal;

    /** A special value that matches any widget ID. */
    static const adobe::aggregate_name_t any_widget_id;

private:
    struct HandlerKey
    {
        HandlerKey();
        HandlerKey(adobe::name_t widget_type,
                   adobe::name_t signal,
                   adobe::name_t widget_id);
        bool matches(const HandlerKey& rhs) const;
        adobe::name_t m_widget_type;
        adobe::name_t m_signal;
        adobe::name_t m_widget_id;
    };

    typedef std::pair<HandlerKey, SignalHandler> Handler;

    struct KeyEquals
    {
        KeyEquals(const HandlerKey& lhs) : m_lhs(lhs) {}
        bool operator()(const Handler& rhs);
        const HandlerKey& m_lhs;
    };

    std::vector<Handler> m_handlers;
};

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
    virtual void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void LButtonUp(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void MouseEnter(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void MouseHere(const Pt& pt, Flags<ModKey> mod_keys);
    virtual void MouseLeave();
    virtual void KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);
    virtual void KeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys);

private:
    EveDialog(adobe::window_t& imp, Clr color, Clr text_color);

    void SetKeyboard(adobe::keyboard_t& keyboard);
    void SetEveModalDialog(adobe::modal_dialog_t* modal_dialog);

    adobe::window_t& m_imp;
    Clr m_color;
    TextControl* m_title;
    adobe::keyboard_t* m_keyboard;
    std::auto_ptr<adobe::modal_dialog_t> m_eve_modal_dialog;
    WndRegion m_prev_wnd_region;
    bool m_left_button_down;

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
                                                const DictionaryFunctions& dictionary_functions,
                                                const ArrayFunctions& array_functions,
                                                const AdamFunctions& adam_functions,
                                                ButtonHandler button_handler,
                                                SignalHandler signal_handler,
                                                RowFactory row_factory);

    friend EveDialog* MakeEveDialog(std::istream& eve_definition,
                                    const std::string& eve_filename,
                                    std::istream& adam_definition,
                                    const std::string& adam_filename,
                                    const DictionaryFunctions& dictionary_functions,
                                    const ArrayFunctions& array_functions,
                                    const AdamFunctions& adam_functions,
                                    ButtonHandler button_handler,
                                    SignalHandler signal_handler,
                                    RowFactory row_factory);
};

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
typedef boost::function<
    adobe::widget_node_t (const adobe::dictionary_t&,
                          const adobe::widget_node_t&,
                          const adobe::factory_token_t&,
                          const adobe::widget_factory_t&)
> MakeViewFunction;

/** Registers user-defined Eve view \a name, for use in Eve sheets.  \see \ref
    eve_adding_user_views. */
void RegisterView(adobe::name_t name,
                  const MakeViewFunction& method,
                  bool container = false,
                  const adobe::layout_attributes_t& layout_attributes = adobe::layout_attributes_t());

/** The type of function used to perform user-defined string localization. */
typedef boost::function<std::string (const std::string&)> LocalizationFunction;

/** Registers a user-defined string localization function. */
void RegisterLocalizationFunction(const LocalizationFunction& f);

}

#endif
