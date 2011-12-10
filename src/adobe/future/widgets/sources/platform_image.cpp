/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_image.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/memory.hpp>

#include <string>
#include <cassert>

#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

adobe::aggregate_name_t key_delta_x = { "delta_x" };
adobe::aggregate_name_t key_delta_y = { "delta_y" };
adobe::aggregate_name_t key_dragging = { "dragging" };
adobe::aggregate_name_t key_x = { "x" };
adobe::aggregate_name_t key_y = { "y" };
adobe::aggregate_name_t key_width = { "width" };
adobe::aggregate_name_t key_height = { "height" };

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

ImageFilter::ImageFilter(image_t& image) :
    Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1),
    m_image(image)
{}

bool ImageFilter::EventFilter(GG::Wnd*, const GG::WndEvent& event)
{
    bool retval = false;

    typedef dictionary_t::value_type value_type;

    if (event.Type() == GG::WndEvent::LButtonDown) {
        m_image.last_point_m = event.Point();
        retval = true;
    } else if (event.Type() == GG::WndEvent::LDrag) {
        GG::Pt cur_point(event.Point());
        double x(Value(cur_point.x));
        double y(Value(cur_point.y));

        if (m_image.last_point_m != cur_point && m_image.callback_m) {
            double delta_x(Value(m_image.last_point_m.x - cur_point.x));
            double delta_y(Value(m_image.last_point_m.y - cur_point.y));

            m_image.metadata_m.insert(value_type(key_delta_x, any_regular_t(delta_x)));
            m_image.metadata_m.insert(value_type(key_delta_y, any_regular_t(delta_y)));
            m_image.metadata_m.insert(value_type(key_dragging, any_regular_t(true)));
            m_image.metadata_m.insert(value_type(key_x, any_regular_t(x)));
            m_image.metadata_m.insert(value_type(key_y, any_regular_t(y)));

            m_image.callback_m(m_image.metadata_m);
        }

        m_image.last_point_m = cur_point;

        retval = true;
    } else if (event.Type() == GG::WndEvent::LButtonUp ||
               event.Type() == GG::WndEvent::LClick) {
        m_image.metadata_m.insert(value_type(key_delta_x, any_regular_t(0)));
        m_image.metadata_m.insert(value_type(key_delta_y, any_regular_t(0)));
        m_image.metadata_m.insert(value_type(key_dragging, any_regular_t(false)));

        if (m_image.callback_m)
            m_image.callback_m(m_image.metadata_m);

        retval = true;
    }
    return retval;
}

} // implementation

} // adobe

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

const GG::X fixed_width(250);
const GG::Y fixed_height(Value(fixed_width));

/****************************************************************************************************/

GG::X get_width(adobe::image_t& image)
{
    GG::SubTexture subtexture;
    adobe::implementation::get_subtexture(image.image_m, subtexture);
    return subtexture.Empty() ? fixed_width : subtexture.Width();
}

/****************************************************************************************************/

GG::Y get_height(adobe::image_t& image)
{
    GG::SubTexture subtexture;
    adobe::implementation::get_subtexture(image.image_m, subtexture);
    return subtexture.Empty() ? fixed_height : subtexture.Height();
}

/****************************************************************************************************/

void reset_image(adobe::image_t& image, const adobe::image_t::view_model_type& view)
{
    assert(image.window_m);

    GG::SubTexture subtexture;
    adobe::implementation::get_subtexture(view, subtexture);

    if (!subtexture.Empty()) {
        GG::Wnd* parent = image.window_m->Parent();
        GG::Pt ul = image.window_m->RelativeUpperLeft();
        GG::Pt size = image.window_m->Size();

        assert(parent);

        delete image.window_m;
        image.window_m = 0;

        image.window_m =
            adobe::implementation::Factory().NewStaticGraphic(
                ul.x, ul.y, size.x, size.y, subtexture,
                GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE, GG::INTERACTIVE
            );
        parent->AttachChild(image.window_m);
        image.window_m->InstallEventFilter(&image.filter_m);
    } else {
        image.window_m->Hide();
    }
}

/****************************************************************************************************/

} // namespace


namespace adobe {

/****************************************************************************************************/

image_t::image_t(const any_regular_t& image) :
    window_m(0),
    image_m(image),
    filter_m(*this)
{
    typedef dictionary_t::value_type value_type;
    metadata_m.insert(value_type(key_delta_x, any_regular_t(0)));
    metadata_m.insert(value_type(key_delta_y, any_regular_t(0)));
    metadata_m.insert(value_type(key_dragging, any_regular_t(false)));
    metadata_m.insert(value_type(key_x, any_regular_t(0)));
    metadata_m.insert(value_type(key_y, any_regular_t(0)));
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

void place(image_t& value, const place_data_t& place_data)
{
    implementation::set_control_bounds(value.window_m, place_data);

    if (value.callback_m)
    {
        dictionary_t old_metadata(value.metadata_m);

        double width(Value(std::min(fixed_width, get_width(value))));
        double height(Value(std::min(fixed_height, get_height(value))));

        typedef dictionary_t::value_type value_type;
        value.metadata_m.insert(value_type(key_width, any_regular_t(width)));
        value.metadata_m.insert(value_type(key_height, any_regular_t(height)));

        if (old_metadata != value.metadata_m)
            value.callback_m(value.metadata_m);
    }
}

/****************************************************************************************************/

void measure(image_t& value, extents_t& result)
{
    // TODO: figure out why a set monitor implies a fixed size
    if (value.callback_m)
        result.horizontal().length_m = Value(fixed_width);
    else
        result.horizontal().length_m = Value(get_width(value));
}

/****************************************************************************************************/

void measure_vertical(image_t& value, extents_t& result,
                      const place_data_t& placed_horizontal)
{
    if (value.callback_m) {
        result.vertical().length_m = Value(fixed_height);
    } else {
        // TODO: handle graphics flags for which non-aspect-ratio-preserving stretches are ok

        double aspect_ratio =
            Value(get_height(value)) / static_cast<double>(Value(get_width(value)));

        result.vertical().length_m =
            static_cast<long>(placed_horizontal.horizontal().length_m * aspect_ratio);
    }
}

/****************************************************************************************************/

void enable(image_t& value, bool make_enabled)
{
    if (value.window_m)
        value.window_m->Disable(!make_enabled);
}

/*************************************************************************************************/

template <>
platform_display_type insert<image_t>(display_t&             display,
                                      platform_display_type& parent,
                                      image_t&               element)
{
    assert(!element.window_m);
    GG::SubTexture subtexture;
    adobe::implementation::get_subtexture(element.image_m, subtexture);
    if (subtexture.Empty()) {
        element.window_m =
            adobe::implementation::Factory().NewStaticGraphic(
                GG::X0, GG::Y0, fixed_width, fixed_height,
                boost::shared_ptr<GG::Texture>(), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                GG::INTERACTIVE
            );
    } else {
        element.window_m =
            adobe::implementation::Factory().NewStaticGraphic(
                GG::X0, GG::Y0, subtexture.Width(), subtexture.Height(),
                subtexture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE,
                GG::INTERACTIVE
            );
    }
    element.window_m->InstallEventFilter(&element.filter_m);
    return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
