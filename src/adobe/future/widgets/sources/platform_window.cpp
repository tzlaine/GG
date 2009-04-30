/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_window.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/keyboard.hpp>

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/Wnd.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

GG::Pt get_window_client_offsets(GG::Wnd* w)
{
    assert(w);
    return w->ClientUpperLeft() - w->UpperLeft();
}

/****************************************************************************************************/

class Window :
    public GG::Wnd
{
public:
    Window(adobe::window_t& imp) :
        Wnd(GG::X0, GG::Y0, GG::X(100), GG::Y(100), DetermineFlags(imp)),
        m_imp(imp),
        m_font(GG::GUI::GetGUI()->GetStyleFactory()->DefaultFont()),
        m_name()
        {
            if (!m_imp.name_m.empty()) {
                m_name = GG::GUI::GetGUI()->GetStyleFactory()->NewTextControl(
                    -ClientULOffset(true).x, -ClientULOffset(true).y,
                    GG::X(100), m_font->Lineskip(),
                    m_imp.name_m, m_font, GG::CLR_BLACK, GG::FORMAT_CENTER
                );
                AttachChild(m_name);
            }
            GG::Pt min_client_size(GG::X(m_imp.min_size_m.x_m), GG::Y(m_imp.min_size_m.y_m));
            SetMinSize(min_client_size + ClientULOffset(m_name) + ClientLROffset());
        }

    virtual GG::Pt ClientUpperLeft() const
        { return UpperLeft() + ClientULOffset(m_name); }

    virtual GG::Pt ClientLowerRight() const
        { return LowerRight() - ClientLROffset(); }

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr)
        {
            GG::Wnd::SizeMove(ul, lr);

            GG::Pt size(lr - ul);
            size -= ClientULOffset(m_name) + ClientLROffset();

            if (!m_imp.debounce_m && !m_imp.resize_proc_m.empty()) {
                m_imp.debounce_m = true;

                if (adobe::width(m_imp.place_data_m) != Value(size.x) ||
                    adobe::height(m_imp.place_data_m) != Value(size.y)) {
                    m_imp.resize_proc_m(Value(size.x), Value(size.y));

                    adobe::width(m_imp.place_data_m) = Value(size.x);
                    adobe::height(m_imp.place_data_m) = Value(size.y);
                }

                m_imp.debounce_m = false;
            }

            m_name->Resize(GG::Pt(Width(), m_name->Height()));
        }

    virtual void Render()
        { GG::BeveledRectangle(UpperLeft(), LowerRight(), GG::CLR_GRAY, GG::CLR_GRAY, true); }

    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point,
                          GG::Flags<GG::ModKey> mod_keys)
        {
            adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                              true,
                                              adobe::modifier_state(),
                                              adobe::any_regular_t(this));
        }

    virtual void KeyRelease(GG::Key key, boost::uint32_t key_code_point,
                            GG::Flags<GG::ModKey> mod_keys)
        {
            adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                              false,
                                              adobe::modifier_state(),
                                              adobe::any_regular_t(this));
        }

private:
    adobe::window_t& m_imp;
    boost::shared_ptr<GG::Font> m_font;
    GG::TextControl* m_name;

    GG::Flags<GG::WndFlag> DetermineFlags(const adobe::window_t& imp) const
        {
            GG::Flags<GG::WndFlag> retval;

            if (imp.style_m == adobe::window_style_moveable_modal_s) {
                assert(imp.modality_m != adobe::window_modality_none_s);
                retval |= GG::MODAL;
                retval |= GG::DRAGABLE;
            } else { // imp.style_m == adobe::window_style_floating_s
                assert(imp.modality_m == adobe::window_modality_none_s);
                retval |= GG::ONTOP;
                retval |= GG::DRAGABLE;
            }

            if (imp.attributes_m == adobe::window_attributes_resizeable_s)
                retval |= GG::RESIZABLE;

            return retval;
        }

    GG::Pt ClientULOffset(bool with_title) const
        { return GG::Pt(GG::X(2), with_title ? m_font->Lineskip() : GG::Y(2)); }

    GG::Pt ClientLROffset() const
        { return GG::Pt(GG::X(2), GG::Y(2)); }
};

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

window_t::window_t(const std::string&  name,
                   window_style_t      style,
                   window_attributes_t attributes,
                   window_modality_t   modality,
                   theme_t             theme) :
    window_m(0),
    name_m(name),
    style_m(style),
    attributes_m(attributes),
    modality_m(modality),
    theme_m(theme),
    debounce_m(false),
    placed_once_m(false)
{ }

/****************************************************************************************************/

window_t::~window_t()
{ delete window_m; }

/****************************************************************************************************/

void window_t::measure(extents_t& result)
{
    assert(window_m);

    if (name_m.empty())
    {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    result = metrics::measure_text(name_m, style->DefaultFont());

    result.width() = static_cast<long>(result.width() * 1.5);
}

/****************************************************************************************************/

void window_t::place(const place_data_t& place_data)
{
    assert(window_m);

    if (placed_once_m)
    {
        set_size(point_2d_t(width(place_data), height(place_data)));
    }
    else
    {
        placed_once_m = true;

        place_data_m = place_data;

        GG::Pt window_ul(window_m->UpperLeft());
        GG::Pt extra(get_window_client_offsets(window_m));

        min_size_m.x_m = width(place_data);
        min_size_m.y_m = height(place_data);

        GG::Pt new_ul(static_cast<int>(left(place_data)) + window_ul.x,
                      static_cast<int>(top(place_data)) + window_ul.y);
        GG::Pt new_size(static_cast<int>(width(place_data)) + extra.x,
                        static_cast<int>(height(place_data)) + extra.y);

        window_m->SizeMove(new_ul, new_ul + new_size);
    }
}

/****************************************************************************************************/

void window_t::set_size(const point_2d_t& size)
{
    assert(window_m);

    if (debounce_m)
        return;

    debounce_m = true;

    width(place_data_m) = size.x_m;
    height(place_data_m) = size.y_m;

    GG::Pt extra(get_window_client_offsets(window_m));

    window_m->Resize(GG::Pt(static_cast<int>(width(place_data_m)) + extra.x,
                            static_cast<int>(height(place_data_m)) + extra.y));

    debounce_m = false;
}

/****************************************************************************************************/

void window_t::reposition(window_reposition_t position)
{
    assert(window_m);

    const GG::X width(window_m->Width());
    const GG::Y height(window_m->Height());

    GG::X app_width(GG::GUI::GetGUI()->AppWidth());
    GG::Y app_height(GG::GUI::GetGUI()->AppHeight());
    
    GG::X left(std::max(GG::X(10), (app_width - width) / 2));
    GG::Y top;

    if (position == window_reposition_center_s)
        top = std::max(GG::Y(10), (app_height - height) / 2);
    else //if (position == window_reposition_alert_s)
        top = std::max(GG::Y(10), static_cast<GG::Y>((app_height * 0.6 - height) / 2));

    window_m->MoveTo(GG::Pt(left, top));
}

/****************************************************************************************************/

void window_t::set_visible(bool make_visible)
{
    assert(window_m);

    if (!window_m->Visible())
        reposition(window_reposition_center_s);

    set_control_visible(window_m, make_visible);
}

/****************************************************************************************************/

void window_t::monitor_resize(const window_resize_proc_t& proc)
{ resize_proc_m = proc; }

/****************************************************************************************************/

any_regular_t window_t::underlying_handler()
{ return any_regular_t(window_m); }

/****************************************************************************************************/

bool window_t::handle_key(key_type /*key*/, bool /*pressed*/, modifiers_t /*modifiers*/)
{ return false; }

/****************************************************************************************************/

template <>
platform_display_type insert<window_t>(display_t&             display,
                                       platform_display_type& parent,
                                       window_t&              element)
{
    assert(!element.window_m);
    assert(!parent);

    element.window_m = new Window(element);

    return display.insert(parent, element.window_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
