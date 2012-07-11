/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/edit_number.hpp>

#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

class EditNumberLabelFilter :
    public GG::EventFilter
{
public:
    EditNumberLabelFilter(edit_number_platform_data_t& data) :
        m_data(data),
        m_left_button_down(false)
        {}

private:
    virtual bool FilterImpl(GG::Wnd* w, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::MouseEnter) {
                if (!m_left_button_down) {
                    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
                    GG::GUI::GetGUI()->PushCursor(style->GetCursor(GG::RESIZE_UP_DOWN_CURSOR));
                }
                retval = true;
            } else if (event.Type() == GG::WndEvent::MouseLeave) {
                if (!m_left_button_down)
                    GG::GUI::GetGUI()->PopCursor();
                retval = true;
            } else if (event.Type() == GG::WndEvent::LDrag) {
                GG::Pt point = event.Point();
                GG::Y delta(m_data.last_point_m.y - point.y);

                if (m_data.last_point_m.y != 0 && delta != 0) {
                    modifiers_t modifiers(modifier_state());
                    if (modifiers & modifiers_any_shift_s)
                        delta *= 10;
                    m_data.increment_n(Value(delta));
                }

                m_data.last_point_m = point;
                retval = true;
            } else if (event.Type() == GG::WndEvent::LButtonDown) {
                m_data.last_point_m = GG::Pt();
                m_left_button_down = true;
                retval = true;
            } else if (event.Type() == GG::WndEvent::LButtonUp || event.Type() == GG::WndEvent::LClick) {
                m_left_button_down = false;
                if (!w->InWindow(event.Point()))
                    GG::GUI::GetGUI()->PopCursor();
                retval = true;
            }
            return retval;
        }

    edit_number_platform_data_t& m_data;
    bool m_left_button_down;
};

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

edit_number_platform_data_t::edit_number_platform_data_t(edit_number_t* edit_number) :
    control_m(edit_number),
    last_point_m()
{ }

/****************************************************************************************************/

edit_number_platform_data_t::~edit_number_platform_data_t()
{ }

/****************************************************************************************************/

void edit_number_platform_data_t::initialize()
{
    if (!control_m->edit_text_m.using_label_m)
        return;

    filter_m.reset(new implementation::EditNumberLabelFilter(*this));
    control_m->edit_text_m.name_m.window_m->InstallEventFilter(filter_m.get());
}

/****************************************************************************************************/

void edit_number_platform_data_t::increment_n(long n)
{ control_m->increment_n(n); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
