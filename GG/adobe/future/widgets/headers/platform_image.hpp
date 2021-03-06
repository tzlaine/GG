/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_IMAGE_T_HPP
#define ADOBE_IMAGE_T_HPP

/****************************************************************************************************/

#include <GG/StaticGraphic.h>
#include <GG/WndEvent.h>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/layout_attributes.hpp>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <string>


namespace GG {
    class Wnd;
}

/****************************************************************************************************/

namespace adobe {

struct image_t;

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

class ImageFilter : public GG::EventFilter
{
public:
    ImageFilter(image_t& image);

private:
    virtual bool FilterImpl(GG::Wnd*, const GG::WndEvent& event);
    image_t& m_image;
};

} // implementation

/****************************************************************************************************/

struct image_t : boost::noncopyable
{
    /// model types for this widget
    typedef dictionary_t                                         controller_model_type;
    typedef any_regular_t                                        view_model_type;
    typedef boost::function<void (const controller_model_type&)> setter_proc_type;

    image_t(const view_model_type&      image,
            int                         width,
            int                         height,
            GG::Flags<GG::GraphicStyle> style);

    void display(const view_model_type& value);
    void monitor(const setter_proc_type& proc);

    GG::StaticGraphic*                 window_m;
    view_model_type                    image_m;
    int                                width_m;
    int                                height_m;
    int                                original_width_m;
    int                                original_height_m;
    GG::Flags<GG::GraphicStyle>        style_m;
    setter_proc_type                   callback_m;
    dictionary_t                       metadata_m;
    GG::Pt                             last_point_m;
    implementation::ImageFilter        filter_m;
};

/****************************************************************************************************/

inline GG::StaticGraphic* get_display(image_t& widget)
{ return widget.window_m; }

/****************************************************************************************************/

void measure(image_t& value, extents_t& result);

void measure_vertical(image_t& value, extents_t& calculated_horizontal, 
                      const place_data_t& placed_horizontal);

void place(image_t& value, const place_data_t& place_data);

void enable(image_t& value, bool make_enabled);

/****************************************************************************************************/

template <typename T> struct controller_model_type;
template <>
struct controller_model_type<adobe::image_t>
{ typedef adobe::image_t::controller_model_type type; };

template <typename T> struct view_model_type;
template <>
struct view_model_type<adobe::image_t>
{ typedef adobe::image_t::view_model_type type; };

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif 

/****************************************************************************************************/
