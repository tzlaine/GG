/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/Filesystem.h>
#include <GG/adobe/future/widgets/headers/window_server.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/future/resources.hpp>

#include <GG/GUI.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

window_server_t::window_server_t(sheet_t& sheet,
                                 behavior_t& behavior,
                                 vm_lookup_t& vm_lookup,
                                 const button_notifier_t& button_notifier,
                                 const signal_notifier_t& signal_notifier,
                                 const row_factory_t* row_factory,
                                 const widget_factory_t& factory) :
    sheet_m(sheet),
    behavior_m(behavior),
    vm_lookup_m(vm_lookup),
    button_notifier_m(button_notifier),
    signal_notifier_m(signal_notifier),
    row_factory_m(row_factory),
    widget_factory_m(factory)
{}

/*************************************************************************************************/

eve_client_holder& window_server_t::client_holder()
{
    if (!window_m)
        throw std::runtime_error("No top view!");
    return *window_m;
}

/*************************************************************************************************/

void window_server_t::run(const char* name)
{
    boost::filesystem::path path(GG::UTF8ToPath(name));
    boost::filesystem::path file_name;

    try {
        file_name = find_resource(name);
    } catch (...) {
        file_name = path;
    }

    boost::filesystem::ifstream stream(file_name);

    /* Update before attaching the window so that we can correctly capture
       contributing for reset. */

    sheet_m.update();

    window_m.reset(
        make_view(GG::PathToUTF8(file_name),
                  line_position_t::getline_proc_t(),
                  stream,
                  sheet_m,
                  behavior_m,
                  vm_lookup_m,
                  button_notifier_m,
                  boost::bind(&window_server_t::button_handler, this, _1, _2),
                  signal_notifier_m,
                  row_factory_m ? *row_factory_m : row_factory_t(),
                  size_enum_t(),
                  default_widget_factory_proc_with_factory(widget_factory_m)).release()
    );

    sheet_m.update(); // Force values to their correct states.

    window_m->path_m = file_name;
    window_m->eve_m.evaluate(eve_t::evaluate_nested);
    window_m->show_window_m();

    window_m->root_display_m->Run();
}

/*************************************************************************************************/

void window_server_t::run(
    std::istream&                          data,
    const boost::filesystem::path&         path,
    const line_position_t::getline_proc_t& getline_proc
) {
    /* Update before attaching the window so that we can correctly capture
       contributing for reset. */
    sheet_m.update();

    //
    // REVISIT (ralpht): Where does this made-up filename get used? Does it
    // need to be localized or actually be an existing file?
    //
    //  REVISIT (fbrereto) : The file name is for error reporting purposes;
    //  see the const char* push_back API where this is filled in. It should
    //  be valid, lest the user not know the erroneous file.
    //
    window_m.reset(
        make_view(GG::PathToUTF8(path),
                  getline_proc,
                  data,
                  sheet_m,
                  behavior_m,
                  vm_lookup_m,
                  button_notifier_m,
                  boost::bind(&window_server_t::button_handler, this, _1, _2),
                  signal_notifier_m,
                  row_factory_m ? *row_factory_m : row_factory_t(),
                  size_enum_t(),
                  default_widget_factory_proc_with_factory(widget_factory_m)).release()
    );

    sheet_m.update(); // Force values to their correct states.

    window_m->path_m = path;
    window_m->eve_m.evaluate(eve_t::evaluate_nested);
    window_m->show_window_m();

    window_m->root_display_m->Run();
}

/****************************************************************************************************/

bool window_server_t::button_handler(adobe::name_t action,
                                     const adobe::any_regular_t& value)
{
    bool retval = false;

    if (action == static_name_t("reset")) {
        sheet_m.set(window_m->contributing_m);
        sheet_m.update();
    } else if (action == static_name_t("cancel")) {
        sheet_m.set(window_m->contributing_m);
        sheet_m.update();
        retval = true;
    } else if (action == static_name_t("ok")) {
        retval = true;
    } else {
        retval = button_notifier_m(action, value);
    }

    if (retval)
        window_m->root_display_m->EndRun();

    return retval;
}

/****************************************************************************************************/

}

/****************************************************************************************************/
