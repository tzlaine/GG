/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_WINDOW_SERVER_HPP
#define ADOBE_WIDGET_WINDOW_SERVER_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/adam.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve.hpp>
#include <GG/adobe/future/assemblage.hpp>
#include <GG/adobe/future/debounce.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/name.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>

#include <list>
#include <vector>

/*************************************************************************************************/

namespace GG {
    class Wnd;
}

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/
/*
    This is a basic "Window Server" - it can be called to construct a window
    by name. The window is attached to a sheet as a slave (meaning the Window
    must be destructed before the sheet).

    REVISIT (sparent) : Here are some thoughts on the direction this design
    should be going...
    
    We have an assemblage (or package of items with the same lifespace) for
    the window which includes the eve structure and the slave connection (the
    wire bundle to attach the assamblage to a sheet).
*/

struct eve_client_holder;

//
/// The window_server_t class can open Eve definitions from file and
/// input stream, and can display them. It also looks after all of
/// the memory allocated to display an Eve definition.
//
class window_server_t
{
public:
    //
    /// This constructor tells the window_server_t where to find
    /// Eve definitions referenced in @dialog commands, and which
    /// sheet to bind against.
    ///
    /// \param sheet the sheet to bind against. This sheet should contain all
    /// of the cells referenced in the Eve file which is loaded (either via
    /// push_back or @dialog).
    //
    window_server_t(sheet_t& sheet,
                    behavior_t& behavior,
                    vm_lookup_t& vm_lookup,
                    const button_notifier_t& button_notifier,
                    const signal_notifier_t& signal_notifier,
                    const row_factory_t* row_factory,
                    const widget_factory_t& factory);
    //
    /// Load the given file out of the directory_path and display the
    /// dialog it contains. If the file cannot be found, or the file
    /// contains errors (e.g.: syntax errors, references non-existant
    /// widgets) then an exception is thrown.
    ///
    /// \param name the name of the Eve file inside the directory given to the
    /// constructor.
    //
    void run(const char* name);
    //
    /// Load an Eve definition from the given std::istream and display
    /// the dialog it contains. If any errors are found in the data
    /// then an exception is thrown.
    ///
    /// \param data an std::istream open on the Eve definition to be loaded.
    //
    void run(std::istream& data,
             const boost::filesystem::path& file_path,
             const line_position_t::getline_proc_t& getline_proc);
    //
    /// \return The top eve client holder in the window list
    ///
    //
    eve_client_holder& client_holder();

private:
    bool button_handler(adobe::name_t action, const adobe::any_regular_t& value);

    sheet_t&                 sheet_m;
    behavior_t&              behavior_m;
    vm_lookup_t&             vm_lookup_m;
    const button_notifier_t& button_notifier_m;
    const signal_notifier_t& signal_notifier_m;
    const row_factory_t*     row_factory_m;
    widget_factory_t         widget_factory_m;

    boost::scoped_ptr<eve_client_holder> window_m;
};

/*************************************************************************************************/

}

/*************************************************************************************************/

#endif
