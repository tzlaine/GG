/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_image.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/memory.hpp>

#include <string>
#include <cassert>

#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

class ImageFilter :
    public GG::Wnd
{
public:
    ImageFilter(image_t& image) : m_image(image) {}

    virtual bool EventFilter(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::LDrag) {
                GG::Pt cur_point(event.Point());

                if (m_image.last_point_m != cur_point) {
                    double x(0);
                    double y(0);
                    double delta_x(Value(m_image.last_point_m.x - cur_point.x));
                    double delta_y(Value(m_image.last_point_m.y - cur_point.y));

                    get_value(m_image.metadata_m, static_name_t("x"), x);
                    get_value(m_image.metadata_m, static_name_t("y"), y);

                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"),
                                                                       any_regular_t(delta_x)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"),
                                                                       any_regular_t(delta_y)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"),
                                                                       any_regular_t(true)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("x"),
                                                                       any_regular_t(x + delta_x)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("y"),
                                                                       any_regular_t(y + delta_y)));

                    m_image.callback_m(m_image.metadata_m);
                }

                m_image.last_point_m = cur_point;

                retval = true;
            } else if (event.Type() == GG::WndEvent::LButtonUp ||
                       event.Type() == GG::WndEvent::LClick) {
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"),
                                                                   any_regular_t(0)));
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"),
                                                                   any_regular_t(0)));
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"),
                                                                   any_regular_t(false)));
                m_image.callback_m(m_image.metadata_m);
                retval = true;
            }
            return retval;
        }

    image_t& m_image;
};

} // implementation

} // adobe

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

const GG::X fixed_width(250);
const GG::Y fixed_height(Value(fixed_width));

/****************************************************************************************************/

void reset_image(adobe::image_t& image, const adobe::image_t::view_model_type& view)
{
    delete image.window_m;

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    image.window_m =
        style->NewStaticGraphic(
            GG::X0, GG::Y0,
            image.image_m->DefaultWidth(), image.image_m->DefaultHeight(),
            view, GG::GRAPHIC_NONE, GG::CLICKABLE
        );
    image.filter_m.reset(new adobe::implementation::ImageFilter(image));
    image.window_m->InstallEventFilter(image.filter_m.get());
}

/****************************************************************************************************/

} // namespace


namespace adobe {

/****************************************************************************************************/

image_t::image_t(const view_model_type& image) :
    window_m(0),
    image_m(image),
    enabled_m(false)
{
    metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"), any_regular_t(false)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("x"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("y"), any_regular_t(0)));
}

/****************************************************************************************************/

void image_t::display(const view_model_type& value)
{
    image_m = value;
    reset_image(*this, image_m);
}

/****************************************************************************************************/

void image_t::monitor(const setter_proc_type& proc)
{ callback_m = proc; }

/****************************************************************************************************/

void image_t::enable(bool make_enabled)
{ enabled_m = make_enabled; }

/****************************************************************************************************/

void place(image_t& value, const place_data_t& place_data)
{
    implementation::set_control_bounds(value.window_m, place_data);

    if (value.callback_m)
    {
        dictionary_t old_metadata(value.metadata_m);

        double width(Value(std::min(fixed_width, value.image_m->DefaultWidth())));
        double height(Value(std::min(fixed_height, value.image_m->DefaultHeight())));

        value.metadata_m.insert(dictionary_t::value_type(static_name_t("width"), any_regular_t(width)));
        value.metadata_m.insert(dictionary_t::value_type(static_name_t("height"), any_regular_t(height)));

        if (old_metadata != value.metadata_m)
            value.callback_m(value.metadata_m);
    }
}

/****************************************************************************************************/

void measure(image_t& value, extents_t& result)
{
    if (value.callback_m)
        result.horizontal().length_m = Value(fixed_width);
    else
        result.horizontal().length_m = Value(value.image_m->DefaultWidth());
}

/****************************************************************************************************/

void measure_vertical(image_t& value, extents_t& result,
                      const place_data_t& placed_horizontal)
{
    if (value.callback_m) {
        result.vertical().length_m = Value(fixed_height);
    } else {
        // NOTE (fbrereto) : We calculate and use the aspect ratio here to
        //                   maintain a proper initial height and width in
        //                   the case when the image grew based on how it
        //                   is being laid out.

        float aspect_ratio =
            Value(value.image_m->DefaultHeight()) /
            static_cast<float>(Value(value.image_m->DefaultWidth()));

        result.vertical().length_m =
            static_cast<long>(placed_horizontal.horizontal().length_m * aspect_ratio);
    }
}

/****************************************************************************************************/

void enable(image_t& value, bool make_enabled)
{ value.window_m->Disable(!make_enabled); }

/*************************************************************************************************/

template <>
platform_display_type insert<image_t>(display_t&             display,
                                      platform_display_type& parent,
                                      image_t&               element)
{
    assert(!element.window_m);
    reset_image(element, element.image_m);
    return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
