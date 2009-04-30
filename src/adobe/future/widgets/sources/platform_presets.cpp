/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_presets.hpp>
#include <GG/adobe/future/widgets/headers/presets_common.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/adobe/name.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <GG/Control.h>


/****************************************************************************************************/

namespace {

#if 0 // TODO
/****************************************************************************************************/

HBITMAP bitmap_normal()
{
    static HBITMAP bitmap_s(0);

    if (bitmap_s == 0)
    {
        boost::gil::rgba8_image_t image;

        adobe::image_slurp("preset_button_u_n.tga", image);

        bitmap_s = adobe::to_bitmap(image);
    }

    return bitmap_s;
}

/****************************************************************************************************/

HBITMAP bitmap_clicked()
{
    static HBITMAP bitmap_s(0);

    if (bitmap_s == 0)
    {
        boost::gil::rgba8_image_t image;

        adobe::image_slurp("preset_button_d_n.tga", image);

        bitmap_s = adobe::to_bitmap(image);
    }

    return bitmap_s;
}

/****************************************************************************************************/

HBITMAP bitmap_disabled()
{
    static HBITMAP bitmap_s(0);

    if (bitmap_s == 0)
    {
        boost::gil::rgba8_image_t image;

        adobe::image_slurp("preset_button_u_d.tga", image);

        bitmap_s = adobe::to_bitmap(image);
    }

    return bitmap_s;
}
#endif

/****************************************************************************************************/

void presets_button_clicked(adobe::presets_t& preset)
{
    preset.selected_m = true;

    adobe::name_t append_presets(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_append_preset, "Append Preset...").c_str());
    adobe::name_t add_preset(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_add_preset, "Add Preset...").c_str());
    adobe::name_t delete_preset(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_delete_preset, "Delete Preset...").c_str());
#ifndef NDEBUG
    adobe::static_name_t resave_presets("(debug) Re-save Presets To File");
#endif

    adobe::name_t options[] =
    {
        append_presets,
        add_preset,
        delete_preset,
#ifndef NDEBUG
        resave_presets
#endif
    };
    std::size_t   num_options(sizeof(options) / sizeof(options[0]));
    adobe::name_t choice;

    if (adobe::context_menu(GG::Pt(preset.control_m->UpperLeft().x + 13,
                                   preset.control_m->UpperLeft().y + 10),
                            options, options + num_options,
                            choice)) {
        if (choice == add_preset && preset.snapshot_proc_m)
            adobe::implementation::append_user_preset(preset, preset.snapshot_proc_m());
        else if (choice == append_presets)
            adobe::implementation::append_user_preset_set(preset);
        else if (choice == delete_preset)
            adobe::implementation::delete_user_preset(preset);
#ifndef NDEBUG
        else if (choice == resave_presets)
            adobe::implementation::save_user_preset_set(preset);
#endif
    }

    preset.selected_m = false;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

presets_t::presets_t(const std::string&         name,
                     const std::string&         path,
                     const std::string&         alt_text,
                     const array_t&             bind_set,
                     const dictionary_t&        localization_set,
                     theme_t                    theme) :
    control_m(0),
    localization_set_m(localization_set),
    category_popup_m(implementation::localization_value(localization_set, 
                        key_preset_category_popup_name, "Category:"), 
                     implementation::localization_value(localization_set, 
                        key_preset_category_popup_alt_text, 
                        "Select a category of presets for this dialog"), 
                     implementation::localization_value(localization_set, 
                        key_preset_custom_category_name, "Custom"), 
                     0, 0, theme),
    popup_m(implementation::localization_value(localization_set, 
                key_preset_preset_popup_name, "Preset:"), 
            implementation::localization_value(localization_set, 
                key_preset_preset_popup_alt_text, 
                "Select a preset for settings in this dialog"), 
            implementation::localization_value(localization_set, 
                key_preset_custom_category_name, "Custom"), 
            0, 0, theme),
    theme_m(theme),
    bind_set_m(bind_set),
    name_m(name),
    path_m(path),
    alt_text_m(alt_text),
    selected_m(false),
    type_2_debounce_m(false),
    custom_m(false)
{ }

/****************************************************************************************************/

void presets_t::measure(extents_t& result)
{
    assert(control_m);

    popup_m.measure(result);

    result.width() += 4 + 26; // gap + icon
    result.height() = (std::max)(result.height(), 21L);

    extents_t cat_result;

    category_popup_m.measure(cat_result);

    result.width() = (std::max)(result.width(), cat_result.width());

    // REVISIT (fbrereto) : This presumes the popups are of the same height.

    popup_height_m = cat_result.height();

    result.height() += popup_height_m + 4;

    if (!result.horizontal().guide_set_m.empty() &&
        !cat_result.horizontal().guide_set_m.empty()) {
        result.horizontal().guide_set_m[0] =
            (std::max)(result.horizontal().guide_set_m[0],
                       cat_result.horizontal().guide_set_m[0]);
    }
}

/****************************************************************************************************/

void presets_t::place(const place_data_t& place_data)
{
    assert(control_m);

    place_data_t category_place(place_data);
    place_data_t popup_place(place_data);
    place_data_t icon_place(place_data);

    // set up the top (category) popup
    width(category_place) -= 4 + 26;
    height(category_place) = popup_height_m;

    // set up the bottom popup
    top(popup_place) = bottom(place_data) - popup_height_m;
    width(popup_place) -= 4 + 26;
    height(popup_place) = popup_height_m;

    // set up the icon
    width(icon_place) = 26;
    height(icon_place) = 21;
    left(icon_place) += width(popup_place) + 4;

    popup_m.place(popup_place);
    category_popup_m.place(category_place);

    implementation::set_control_bounds(control_m, icon_place);
}

/****************************************************************************************************/             

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/
//              container idiom for event and drawing system.

template <> platform_display_type insert<presets_t>(display_t&             display,
                                                    platform_display_type& parent,
                                                    presets_t&             element)
{
    assert(!"Cannot create presets_t's until the GG texture issue is resolved.");
#if 0
    HWND parent_hwnd(parent);

    insert(display, parent, element.category_popup_m);

    insert(display, parent, element.popup_m);

    assert(!element.control_m);

    long width(26);
    long height(21);

    element.control_m = ::CreateWindowExW(  WS_EX_COMPOSITED | WS_EX_TRANSPARENT, L"STATIC",
                                            NULL,
                                            WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,
                                            0, 0, width, height,
                                            parent,
                                            0,
                                            ::GetModuleHandle(NULL),
                                            NULL);

    if (element.control_m == NULL) ADOBE_THROW_LAST_ERROR;

    ::SetWindowSubclass(element.control_m, presets_subclass_proc, reinterpret_cast<UINT_PTR>(&element), 0);

    element.popup_m.monitor(boost::bind(&presets_t::do_imbue, boost::ref(element), _1));

    ::SendMessage(element.control_m, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bitmap_normal()); 

    if (!element.alt_text_m.empty())
        adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);
#endif

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

boost::filesystem::path preset_directory(const presets_t& control)
{
    boost::filesystem::path result(control.path_m, boost::filesystem::native);
    boost::filesystem::create_directory(result);
    return result;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void enable(presets_t& control, bool enable)
{
    assert(control.control_m);

    control.control_m->Disable(!enable);

#if 0 // TODO
    ::SendMessage(control.control_m, STM_SETIMAGE, IMAGE_BITMAP, 
        hackery::cast<LPARAM>(enable ? bitmap_normal() : bitmap_disabled()));
#endif
}

    
/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
