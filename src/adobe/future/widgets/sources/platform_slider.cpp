/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_slider.hpp>

#include <GG/GUI.h>
#include <GG/Slider.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void slider_slid(adobe::slider_t& slider, int tab, int min, int max)
{
    if (slider.value_proc_m) {
        double new_value(slider.format_m.at(tab).cast<double>());
        if (new_value != slider.value_m) {
            slider.value_m = new_value;
            slider.value_proc_m(static_cast<adobe::slider_t::model_type>(slider.value_m));
        }
    } else if (slider.slid_proc_m) {
        slider.slid_proc_m(tab, min, max);
    }
}

/****************************************************************************************************/

void slider_slid_and_stopped(adobe::slider_t& slider, int tab, int min, int max)
{
    if (slider.slid_and_stopped_proc_m)
        slider.slid_and_stopped_proc_m(tab, min, max);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

slider_t::slider_t(const std::string&          alt_text,
                   bool                        is_vertical,
                   std::size_t                 num_ticks,
                   const value_range_format_t& format,
                   int                         length,
                   const GG::Clr&              color,
                   int                         tab_width,
                   int                         tab_length,
                   int                         line_width,
                   GG::SliderLineStyle         line_style,
                   name_t                      signal_id) :
    control_m(0),
    alt_text_m(alt_text),
    is_vertical_m(is_vertical),
    num_ticks_m(num_ticks),
    format_m(format),
    length_m(length),
    color_m(color),
    tab_width_m(tab_width),
    tab_length_m(tab_length),
    line_width_m(line_width),
    line_style_m(line_style),
    signal_id_m(signal_id)
{ }

/****************************************************************************************************/

void slider_t::measure(extents_t& result)
{
    GG::Pt min_size = control_m->MinUsableSize();
    result.width() = Value(min_size.x);
    result.height() = Value(min_size.y);
}

/****************************************************************************************************/

void slider_t::place(const place_data_t& place_data)
{ implementation::set_control_bounds(control_m, place_data); }

/****************************************************************************************************/

void slider_t::enable(bool make_enabled)
{
    assert(control_m);
    control_m->Disable(!make_enabled);
}

/****************************************************************************************************/

void slider_t::monitor(const setter_type& proc)
{ value_proc_m = proc; }

/****************************************************************************************************/

void slider_t::display(const model_type& value)
{
    value_m = value;
    if (value_m != last_m) {
        last_m = value_m;
        control_m->SlideTo(value_m);
    }
}

/****************************************************************************************************/

template <>
platform_display_type insert<slider_t>(display_t&             display,
                                       platform_display_type& parent,
                                       slider_t&              element)
{
    assert(!element.control_m);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    double min = element.format_m.at(0).cast<slider_t::model_type>();
    double max = element.format_m.at(element.format_m.size() - 1).cast<slider_t::model_type>();
    const int tab_width = 0 < element.tab_width_m ? element.tab_width_m : 6;
    const int line_width = 0 < element.line_width_m ? element.line_width_m : 6;
    const int length = 0 < element.length_m ? element.length_m : 100;
    element.control_m =
        style->NewDoubleSlider(GG::X0, GG::Y0, GG::X(length), GG::Y(length),
                               min, max, element.is_vertical_m ? GG::VERTICAL : GG::HORIZONTAL,
                               element.line_style_m, element.color_m, tab_width, line_width);

    const int tab_length = 0 < element.tab_length_m ? element.tab_length_m : 3 * tab_width;
    element.control_m->SetMinSize(GG::Pt(element.is_vertical_m ? GG::X(tab_length) : GG::X0,
                                         element.is_vertical_m ? GG::Y0 : GG::Y(tab_length)));

    GG::Connect(element.control_m->SlidSignal,
                boost::bind(&slider_slid, boost::ref(element), _1, _2, _3));

    GG::Connect(element.control_m->SlidAndStoppedSignal,
                boost::bind(&slider_slid_and_stopped, boost::ref(element), _1, _2, _3));

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
