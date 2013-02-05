/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#define ADOBE_DLL_SAFE 0

#include <GG/adobe/future/widgets/headers/platform_image.hpp>

#include <GG/adobe/future/widgets/headers/factory.hpp>
#include <GG/adobe/future/widgets/headers/image_factory.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>
#include <GG/adobe/future/widgets/headers/widget_factory.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/GUI.h>


/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void create_widget(const dictionary_t& parameters,
                   size_enum_t         /*size*/,
                   image_t*&           widget)
{
    image_t::view_model_type image;
    int                      width(0);
    int                      height(0);
    name_t                   horizontal;
    name_t                   vertical;
    bool                     fit_graphic(true);
    bool                     shrink_to_fit(false);
    bool                     proportional(true);

    get_value(parameters, key_image, image);
    get_value(parameters, static_name_t("width"), width);
    get_value(parameters, static_name_t("height"), height);
    get_value(parameters, static_name_t("image_horizontal"), horizontal);
    get_value(parameters, static_name_t("image_vertical"), vertical);
    get_value(parameters, static_name_t("fit_graphic"), fit_graphic);
    get_value(parameters, static_name_t("shrink_to_fit"), shrink_to_fit);
    get_value(parameters, static_name_t("proportional"), proportional);

    GG::Flags<GG::GraphicStyle> style;

    if (horizontal == key_align_left)
        style |= GG::GRAPHIC_LEFT;
    else if (horizontal == key_align_center)
        style |= GG::GRAPHIC_CENTER;
    else if (horizontal == key_align_right)
        style |= GG::GRAPHIC_RIGHT;

    if (vertical == key_align_top)
        style |= GG::GRAPHIC_TOP;
    else if (vertical == key_align_center)
        style |= GG::GRAPHIC_VCENTER;
    else if (vertical == key_align_bottom)
        style |= GG::GRAPHIC_BOTTOM;

    if (fit_graphic)
        style |= GG::GRAPHIC_FITGRAPHIC;

    if (shrink_to_fit)
        style |= GG::GRAPHIC_SHRINKFIT;

    if (proportional)
        style |= GG::GRAPHIC_PROPSCALE;

    widget = new image_t(image, width, height, style);
}

/*************************************************************************************************/

void subscribe_view_to_model(image_t&                control,
                             name_t                  cell, 
                             basic_sheet_t*          layout_sheet,
                             sheet_t*                model_sheet,
                             assemblage_t&           assemblage,
                             visible_change_queue_t& visible_queue)
{
    typedef force_relayout_view_adaptor<image_t> adaptor_type;

    adaptor_type* view_adaptor(new adaptor_type(control, visible_queue));

    assemblage_cleanup_ptr(assemblage, view_adaptor);

    if (layout_sheet) {
        attach_view(assemblage, cell, *view_adaptor, *layout_sheet);
        return;
    }

    attach_view(assemblage, cell, *view_adaptor, *model_sheet);
}

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

widget_node_t make_image_hack(const dictionary_t&     parameters, 
                              const widget_node_t&    parent, 
                              const factory_token_t&  token,
                              const widget_factory_t& factory)
    { return create_and_hookup_widget<image_t, poly_placeable_twopass_t>(
        parameters, parent, token, 
        factory.is_container(static_name_t("image")), 
        factory.layout_attributes(static_name_t("image"))); }

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
