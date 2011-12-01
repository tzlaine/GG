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

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>

#include <boost/function.hpp>


namespace GG {

class Wnd;

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

/** The type of button click handle function expected by the UI creation
    functions.  Handlers accept the name of the \a action associated with the
    button click and the \a value emitted by the click as specified in the
    Adam and Eve scripts, and return true if the click should result in the
    closure of the dialog.  \see \ref eve_button_handler. */
typedef boost::function <bool (adobe::name_t, const adobe::any_regular_t&)> ButtonHandler;

/** Returns the result of executing the modal dialog described by \a
    eve_definition and \a adam_definition.  \see ButtonHandler. */
ModalDialogResult ExecuteModalDialog(std::istream& eve_definition,
                                     std::istream& adam_definition,
                                     ButtonHandler handler);

/** Parses \a eve_definition and \a adam_definition, then instantiates and
    returns a dialog.  \see ButtonHandler. */
Wnd* MakeDialog(std::istream& eve_definition,
                std::istream& adam_definition,
                ButtonHandler handler);

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

}

#endif
