/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@hotmail.com */

#include <GG/Button.h>

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>

#include <boost/lexical_cast.hpp>


using namespace GG;

namespace {
    struct SetCheckedButtonAction : AttributeChangedAction<int>
    {
        SetCheckedButtonAction(RadioButtonGroup* radio_button_group) : m_radio_button_group(radio_button_group) {}
        void operator()(const int& button)
        {
            int b = button;
            m_radio_button_group->SetCheck(-1);
            m_radio_button_group->SetCheck(b);
        }
        RadioButtonGroup* const m_radio_button_group;
    };
}

////////////////////////////////////////////////
// GG::Button
////////////////////////////////////////////////
Button::Button() :
    TextControl(),
    m_state(BN_UNPRESSED)
{}

Button::Button(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Clr color, 
               Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, text_color, FORMAT_NONE, flags),
    m_state(BN_UNPRESSED)
{
    m_color = color;
}

Button::ButtonState Button::State() const
{
    return m_state;
}

const SubTexture& Button::UnpressedGraphic() const
{
    return m_unpressed_graphic;
}

const SubTexture& Button::PressedGraphic() const
{
    return m_pressed_graphic;
}

const SubTexture& Button::RolloverGraphic() const
{
    return m_rollover_graphic;
}

void Button::Render()
{
    switch (m_state)
    {
    case BN_PRESSED:
        RenderPressed();
        break;
    case BN_UNPRESSED:
    case BN_ROLLOVER:
        if (m_state == BN_UNPRESSED)
            RenderUnpressed();
        else
            RenderRollover();
        break;
    }
}

void Button::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        ButtonState prev_state = m_state;
        m_state = BN_PRESSED;
        if (prev_state == BN_PRESSED && RepeatButtonDown())
            ClickedSignal();
    }
}

void Button::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_PRESSED;
    Wnd::LDrag(pt, move, mod_keys);
}

void Button::LButtonUp(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        m_state = BN_ROLLOVER;
        ClickedSignal();
    }
}

void Button::MouseHere(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        m_state = BN_ROLLOVER;
}

void Button::MouseLeave()
{
    if (!Disabled())
        m_state = BN_UNPRESSED;
}

void Button::SetColor(Clr c)
{
    Control::SetColor(c);
}

void Button::SetState(ButtonState state)
{
    m_state = state;
}

void Button::SetUnpressedGraphic(const SubTexture& st)
{
    m_unpressed_graphic = st;
}

void Button::SetPressedGraphic(const SubTexture& st)
{
    m_pressed_graphic = st;
}

void Button::SetRolloverGraphic(const SubTexture& st)
{
    m_rollover_graphic = st;
}

void Button::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    // TODO: handle setting graphics
}

void Button::RenderPressed()
{
    if (!m_pressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_pressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    OffsetMove(Pt(1, 1));
    TextControl::Render();
    OffsetMove(Pt(-1, -1));
}

void Button::RenderUnpressed()
{
    if (!m_unpressed_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_unpressed_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(Pt(2, 2));
    TextControl::Render();
    OffsetMove(Pt(-2, -2));
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderRollover()
{
    if (!m_rollover_graphic.Empty()) {
        glColor(Disabled() ? DisabledColor(m_color) : m_color);
        m_rollover_graphic.OrthoBlit(UpperLeft(), LowerRight());
    } else {
        RenderDefault();
    }
    // draw text shadow
    Clr temp = TextColor();  // save original color
    SetTextColor(CLR_SHADOW); // shadow color
    OffsetMove(Pt(2, 2));
    TextControl::Render();
    OffsetMove(Pt(-2, -2));
    SetTextColor(temp);    // restore original color
    // draw text
    TextControl::Render();
}

void Button::RenderDefault()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    BeveledRectangle(ul.x, ul.y, lr.x, lr.y,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     Disabled() ? DisabledColor(m_color) : m_color,
                     (m_state != BN_PRESSED), 1);
}


////////////////////////////////////////////////
// GG::StateButton
////////////////////////////////////////////////
StateButton::StateButton() :
    TextControl(),
    m_checked(false),
    m_style(SBSTYLE_3D_XBOX)
{}

StateButton::StateButton(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font, Flags<TextFormat> format, 
                         Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/, StateButtonStyle style/* = SBSTYLE_3D_XBOX*/,
                         Flags<WndFlag> flags/* = CLICKABLE*/) :
    TextControl(x, y, w, h, str, font, text_color, format, flags),
    m_checked(false),
    m_int_color(interior),
    m_style(style)
{
    m_color = color;
    SetDefaultButtonPosition();
}

Pt StateButton::MinUsableSize() const
{
    Pt text_lr = m_text_ul + TextControl::MinUsableSize();
    return Pt(std::max(m_button_lr.x, text_lr.x) - std::min(m_button_ul.x, m_text_ul.x),
              std::max(m_button_lr.y, text_lr.y) - std::min(m_button_ul.y, m_text_ul.y));
}

bool StateButton::Checked() const
{
    return m_checked;
}

Clr StateButton::InteriorColor() const
{
    return m_int_color;
}

StateButtonStyle StateButton::Style() const
{
    return m_style;
}

void StateButton::Render()
{
    const int BEVEL = 2;

    // draw button
    Pt cl_ul = ClientUpperLeft();
    Pt cl_lr = ClientLowerRight();
    Pt bn_ul = cl_ul + m_button_ul;
    Pt bn_lr = cl_ul + m_button_lr;

    Pt additional_text_offset;

    switch (m_style) {
    case SBSTYLE_3D_XBOX:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, BEVEL);
        if (m_checked)
            BeveledX(bn_ul.x + 2 * BEVEL, bn_ul.y + 2 * BEVEL, bn_lr.x - 2 * BEVEL, bn_lr.y - 2 * BEVEL,
                     m_disabled ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_CHECKBOX:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_int_color) : m_int_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         false, BEVEL);
        if (m_checked)
            BeveledCheck(bn_ul.x + 2 * BEVEL, bn_ul.y + 2 * BEVEL, bn_lr.x - 2 * BEVEL, bn_lr.y - 2 * BEVEL,
                         Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_RADIO:
        BeveledCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                      Disabled() ? DisabledColor(m_int_color) : m_int_color,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      false, BEVEL);
        if (m_checked)
            Bubble(bn_ul.x + 2 * BEVEL, bn_ul.y + 2 * BEVEL, bn_lr.x - 2 * BEVEL, bn_lr.y - 2 * BEVEL,
                   Disabled() ? DisabledColor(m_color) : m_color);
        break;
    case SBSTYLE_3D_BUTTON:
        BeveledRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         Disabled() ? DisabledColor(m_color) : m_color,
                         !m_checked, BEVEL);
        break;
    case SBSTYLE_3D_ROUND_BUTTON:
        BeveledCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y,
                      Disabled() ? DisabledColor(m_color) : m_color,
                      Disabled() ? DisabledColor(m_color) : Color(),
                      !m_checked, BEVEL);
        break;
    case SBSTYLE_3D_TOP_ATTACHED_TAB: {
        Clr color_to_use = m_checked ? m_color : DarkColor(m_color);
        color_to_use = Disabled() ? DisabledColor(color_to_use) : color_to_use;
        if (!m_checked) {
            cl_ul.y += BEVEL;
            additional_text_offset.y = BEVEL / 2;
        }
        BeveledRectangle(cl_ul.x, cl_ul.y, cl_lr.x, cl_lr.y,
                         color_to_use, color_to_use,
                         true, BEVEL,
                         true, true, true, false);
        break;
    }
    case SBSTYLE_3D_TOP_DETACHED_TAB: {
        Clr color_to_use = m_checked ? m_color : DarkColor(m_color);
        color_to_use = Disabled() ? DisabledColor(color_to_use) : color_to_use;
        if (!m_checked) {
            cl_ul.y += BEVEL;
            additional_text_offset.y = BEVEL / 2;
        }
        BeveledRectangle(cl_ul.x, cl_ul.y, cl_lr.x, cl_lr.y,
                         color_to_use, color_to_use,
                         true, BEVEL);
        break;
    }
    }

    OffsetMove(m_text_ul + additional_text_offset);
    TextControl::Render();
    OffsetMove(-(m_text_ul + additional_text_offset));
}

void StateButton::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled())
        SetCheck(!m_checked);
}

void StateButton::SizeMove(const Pt& ul, const Pt& lr)
{
    RepositionButton();
    TextControl::SizeMove(ul, lr);
}

void StateButton::Reset()
{
    SetCheck(false);
}

void StateButton::SetCheck(bool b/* = true*/)
{
    CheckedSignal(m_checked = b);
}

void StateButton::RepositionButton()
{
    if (m_style == SBSTYLE_3D_TOP_ATTACHED_TAB ||
        m_style == SBSTYLE_3D_TOP_DETACHED_TAB) {
        m_button_ul = Pt();
        m_button_lr = Pt();
        m_text_ul = Pt();
    } else {
        int w = Width();
        int h = Height();
        const int BN_W = m_button_lr.x - m_button_ul.x;
        const int BN_H = m_button_lr.y - m_button_ul.y;
        int bn_x = m_button_ul.x;
        int bn_y = m_button_ul.y;
        Flags<TextFormat> format = GetTextFormat();
        const double SPACING = 0.5; // the space to leave between the button and text, as a factor of the button's size (width or height)
        if (format & FORMAT_VCENTER)       // center button vertically
            bn_y = static_cast<int>((h - BN_H) / 2.0 + 0.5);
        if (format & FORMAT_TOP) {         // put button at top, text just below
            bn_y = 0;
            m_text_ul.y = BN_H;
        }
        if (format & FORMAT_BOTTOM) {      // put button at bottom, text just above
            bn_y = (h - BN_H);
            m_text_ul.y = static_cast<int>(h - (BN_H * (1 + SPACING)) - ((GetLineData().size() - 1) * GetFont()->Lineskip() + GetFont()->Height()) + 0.5);
        }

        if (format & FORMAT_CENTER) {      // center button horizontally
            if (format & FORMAT_VCENTER) { // if both the button and the text are to be centered, bad things happen
                format |= FORMAT_LEFT;     // so go to the default (FORMAT_CENTER|FORMAT_LEFT)
                format &= ~FORMAT_CENTER;
            } else {
                bn_x = static_cast<int>((w - bn_x) / 2.0 - BN_W / 2.0 + 0.5);
            }
        }
        if (format & FORMAT_LEFT) {        // put button at left, text just to the right
            bn_x = 0;
            if (format & FORMAT_VCENTER)
                m_text_ul.x = static_cast<int>(BN_W * (1 + SPACING) + 0.5);
        }
        if (format & FORMAT_RIGHT) {       // put button at right, text just to the left
            bn_x = (w - BN_W);
            if (format & FORMAT_VCENTER)
                m_text_ul.x = static_cast<int>(-BN_W * (1 + SPACING) + 0.5);
        }
        SetTextFormat(format);
        m_button_ul = Pt(bn_x, bn_y);
        m_button_lr = m_button_ul + Pt(BN_W, BN_H);
    }
}

void StateButton::SetButtonPosition(const Pt& ul, const Pt& lr)
{
    int bn_x = ul.x;
    int bn_y = ul.y;
    int bn_w = lr.x - ul.x;
    int bn_h = lr.y - ul.y;

    if (bn_w <= 0 || bn_h <= 0)               // if one of these is invalid,
        bn_w = bn_h = GetFont()->PointSize(); // set button width and height to text height

    if (bn_x == -1 || bn_y == -1) {
        m_button_ul = Pt(0, 0);
        m_button_lr = Pt(bn_w, bn_h);
        RepositionButton();
    } else {
        m_button_ul = Pt(bn_x, bn_y);
        m_button_lr = m_button_ul + Pt(bn_w, bn_h);
    }
}

void StateButton::SetDefaultButtonPosition()
{
    SetButtonPosition(Pt(-1, -1), Pt(-1, -1));
}

void StateButton::SetColor(Clr c)
{
    Control::SetColor(c);
}

void StateButton::SetInteriorColor(Clr c)
{
    m_int_color = c;
}

void StateButton::SetStyle(StateButtonStyle bs)
{
    m_style = bs;
}

void StateButton::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    TextControl::DefineAttributes(editor);
    editor->Label("StateButton");
    editor->Attribute("Checked", m_checked);
    editor->Attribute("Interior Color", m_int_color);
    editor->Attribute("Button Style", m_style,
                      SBSTYLE_3D_XBOX, SBSTYLE_3D_ROUND_BUTTON);
    editor->Attribute("Button Upper Left", m_button_ul);
    editor->Attribute("Button Lower Right", m_button_lr);
    editor->Attribute("Text Upper Left", m_text_ul);
}

Pt StateButton::ButtonUpperLeft() const
{
    return m_button_ul;
}

Pt StateButton::ButtonLowerRight() const
{
    return m_button_lr;
}

Pt StateButton::TextUpperLeft() const
{
    return m_text_ul;
}


////////////////////////////////////////////////
// GG::RadioButtonGroup
////////////////////////////////////////////////
// ButtonClickedFunctor
RadioButtonGroup::ButtonClickedFunctor::ButtonClickedFunctor(RadioButtonGroup* group, StateButton* button, int index) :
    m_group(group),
    m_button(button),
    m_index(index),
    m_ignore_clicks(false)
{}

void RadioButtonGroup::ButtonClickedFunctor::operator()(bool checked)
{
    if (!m_ignore_clicks) {
        if (checked) {
            m_group->HandleRadioClick(m_index, false);
        } else {
            m_ignore_clicks = true;
            m_button->SetCheck(true);
            m_ignore_clicks = false;
        }
    }
}

// ButtonSlot
RadioButtonGroup::ButtonSlot::ButtonSlot() :
    button(0)
{}

RadioButtonGroup::ButtonSlot::ButtonSlot(StateButton* button_) :
    button(button_)
{}

// RadioButtonGroup
// static(s)
const int RadioButtonGroup::NO_BUTTON = -1;

RadioButtonGroup::RadioButtonGroup() :
    Control(),
    m_orientation(VERTICAL),
    m_checked_button(NO_BUTTON),
    m_expand_buttons(false),
    m_expand_buttons_proportionally(false),
    m_render_outline(false)
{
    SetColor(CLR_YELLOW);
}

RadioButtonGroup::RadioButtonGroup(int x, int y, int w, int h, Orientation orientation) :
    Control(x, y, w, h),
    m_orientation(orientation),
    m_checked_button(NO_BUTTON),
    m_expand_buttons(false),
    m_expand_buttons_proportionally(false),
    m_render_outline(false)
{
    SetColor(CLR_YELLOW);
}

Pt RadioButtonGroup::MinUsableSize() const
{
    Pt retval;
    for (unsigned int i = 0; i < m_button_slots.size(); ++i) {
        Pt min_usable_size = m_button_slots[i].button->MinUsableSize();
        if (m_orientation == VERTICAL) {
            retval.x = std::max(retval.x, min_usable_size.x);
            retval.y += min_usable_size.y;
        } else {
            retval.x += min_usable_size.x;
            retval.y = std::max(retval.y, min_usable_size.y);
        }
    }
    return retval;
}

Orientation RadioButtonGroup::GetOrientation() const
{
    return m_orientation;
}

int RadioButtonGroup::NumButtons() const
{
    return m_button_slots.size();
}

int RadioButtonGroup::CheckedButton() const
{
    return m_checked_button;
}

bool RadioButtonGroup::ExpandButtons() const
{
    return m_expand_buttons;
}

bool RadioButtonGroup::ExpandButtonsProportionally() const
{
    return m_expand_buttons_proportionally;
}

bool RadioButtonGroup::RenderOutline() const
{
    return m_render_outline;
}

void RadioButtonGroup::RaiseCheckedButton()
{
    if (m_checked_button != NO_BUTTON)
        MoveChildUp(m_button_slots[m_checked_button].button);
}

void RadioButtonGroup::Render()
{
    if (m_render_outline) {
        Pt ul = UpperLeft(), lr = LowerRight();
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, CLR_ZERO, color_to_use, 1);
    }
}

void RadioButtonGroup::SetCheck(int index)
{
    if (index == m_checked_button)
        return;
    if (index < 0 || index >= static_cast<int>(m_button_slots.size()))
        index = NO_BUTTON;
    HandleRadioClick(index, true);
}

void RadioButtonGroup::DisableButton(int index, bool b/* = true*/)
{
    if (0 <= index && index < static_cast<int>(m_button_slots.size())) {
        bool was_disabled = m_button_slots[index].button->Disabled();
        m_button_slots[index].button->Disable(b);
        if (b && !was_disabled && index == m_checked_button)
            SetCheck(NO_BUTTON);
    }
}

void RadioButtonGroup::AddButton(StateButton* bn)
{
    InsertButton(m_button_slots.size(), bn);
}

void RadioButtonGroup::AddButton(const std::string& text, const boost::shared_ptr<Font>& font, Flags<TextFormat> format,
                                 Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                 StateButtonStyle style/* = SBSTYLE_3D_RADIO*/)
{
    InsertButton(m_button_slots.size(), text, font, format, color, text_color, interior, style);
}

void RadioButtonGroup::InsertButton(int index, StateButton* bn)
{
    assert(0 <= index && index <= static_cast<int>(m_button_slots.size()));
    if (!m_expand_buttons) {
        Pt min_usable_size = bn->MinUsableSize();
        bn->Resize(Pt(std::max(bn->Width(), min_usable_size.x), std::max(bn->Height(), min_usable_size.y)));
    }
    Pt bn_sz = bn->Size();
    Layout* layout = GetLayout();
    if (!layout) {
        layout = new Layout(0, 0, ClientWidth(), ClientHeight(), 1, 1);
        SetLayout(layout);
    }
    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    const int X_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? bn_sz.x : 1;
    const int Y_STRETCH = (m_expand_buttons && m_expand_buttons_proportionally) ? bn_sz.y : 1;
    if (m_button_slots.empty()) {
        layout->Add(bn, 0, 0);
        if (m_expand_buttons) {
            if (m_orientation == VERTICAL)
                layout->SetRowStretch(0, Y_STRETCH);
            else
                layout->SetColumnStretch(0, X_STRETCH);
        }
    } else {
        if (m_orientation == VERTICAL) {
            layout->ResizeLayout(layout->Rows() + CELLS_PER_BUTTON, 1);
            layout->SetRowStretch(layout->Rows() - CELLS_PER_BUTTON, Y_STRETCH);
        } else {
            layout->ResizeLayout(1, layout->Columns() + CELLS_PER_BUTTON);
            layout->SetColumnStretch(layout->Columns() - CELLS_PER_BUTTON, X_STRETCH);
        }
        for (int i = m_button_slots.size() - 1; index <= i; --i) {
            layout->Remove(m_button_slots[i].button);
            layout->Add(m_button_slots[i].button,
                        m_orientation == VERTICAL ? i * CELLS_PER_BUTTON + CELLS_PER_BUTTON : 0,
                        m_orientation == VERTICAL ? 0 : i * CELLS_PER_BUTTON + CELLS_PER_BUTTON);
            if (m_orientation == VERTICAL)
                layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
            else
                layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON + CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
        layout->Add(bn, m_orientation == VERTICAL ? index * CELLS_PER_BUTTON : 0, m_orientation == VERTICAL ? 0 : index * CELLS_PER_BUTTON);
    }
    if (m_orientation == VERTICAL)
        layout->SetMinimumRowHeight(index * CELLS_PER_BUTTON, bn_sz.y);
    else
        layout->SetMinimumColumnWidth(index * CELLS_PER_BUTTON, bn_sz.x);
    m_button_slots.insert(m_button_slots.begin() + index, ButtonSlot(bn));

    int old_checked_button = m_checked_button;
    if (index <= m_checked_button)
        ++m_checked_button;
    Reconnect();
    if (m_checked_button != old_checked_button)
        ButtonChangedSignal(m_checked_button);
}

void RadioButtonGroup::InsertButton(int index, const std::string& text, const boost::shared_ptr<Font>& font, Flags<TextFormat> format,
                                    Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                    StateButtonStyle style/* = SBSTYLE_3D_RADIO*/)
{
    assert(0 <= index && index <= static_cast<int>(m_button_slots.size()));
    StateButton* button = GetStyleFactory()->NewStateButton(0, 0, 1, 1, text, font, format, color, text_color, interior, style);
    button->Resize(button->MinUsableSize());
    InsertButton(index, button);
}

void RadioButtonGroup::RemoveButton(StateButton* button)
{
    int index = -1;
    for (unsigned int i = 0; i < m_button_slots.size(); ++i) {
        if (m_button_slots[i].button == button) {
            index = i;
            break;
        }
    }
    assert(0 <= index && index < static_cast<int>(m_button_slots.size()));

    const int CELLS_PER_BUTTON = m_expand_buttons ? 1 : 2;
    Layout* layout = GetLayout();
    layout->Remove(m_button_slots[index].button);
    for (unsigned int i = index + 1; i < m_button_slots.size(); ++i) {
        layout->Remove(m_button_slots[i].button);
        if (m_orientation == VERTICAL) {
            layout->Add(m_button_slots[i].button, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, 0);
            layout->SetRowStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->RowStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumRowHeight(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumRowHeight(i * CELLS_PER_BUTTON));
        } else {
            layout->Add(m_button_slots[i].button, 0, i * CELLS_PER_BUTTON - CELLS_PER_BUTTON);
            layout->SetColumnStretch(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->ColumnStretch(i * CELLS_PER_BUTTON));
            layout->SetMinimumColumnWidth(i * CELLS_PER_BUTTON - CELLS_PER_BUTTON, layout->MinimumColumnWidth(i * CELLS_PER_BUTTON));
        }
    }
    m_button_slots[index].connection.disconnect();
    m_button_slots.erase(m_button_slots.begin() + index);
    if (m_button_slots.empty()) {
        layout->ResizeLayout(1, 1);
    } else {
        if (m_orientation == VERTICAL)
            layout->ResizeLayout(layout->Rows() - CELLS_PER_BUTTON, 1);
        else
            layout->ResizeLayout(1, layout->Columns() - CELLS_PER_BUTTON);
    }

    int old_checked_button = m_checked_button;
    if (index == m_checked_button)
        m_checked_button = NO_BUTTON;
    else if (index <= m_checked_button)
        --m_checked_button;
    Reconnect();
    if (m_checked_button != old_checked_button)
        ButtonChangedSignal(m_checked_button);
}

void RadioButtonGroup::ExpandButtons(bool expand)
{
    if (expand != m_expand_buttons) {
        int old_checked_button = m_checked_button;
        std::vector<StateButton*> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            StateButton* button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button);
        }
        m_expand_buttons = expand;
        for (unsigned int i = 0; i < buttons.size(); ++i) {
            AddButton(buttons[i]);
        }
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::ExpandButtonsProportionally(bool proportional)
{
    if (proportional != m_expand_buttons_proportionally) {
        int old_checked_button = m_checked_button;
        std::vector<StateButton*> buttons(m_button_slots.size());
        while (!m_button_slots.empty()) {
            StateButton* button = m_button_slots.back().button;
            buttons[m_button_slots.size() - 1] = button;
            RemoveButton(button);
        }
        m_expand_buttons_proportionally = proportional;
        for (unsigned int i = 0; i < buttons.size(); ++i) {
            AddButton(buttons[i]);
        }
        SetCheck(old_checked_button);
    }
}

void RadioButtonGroup::RenderOutline(bool render_outline)
{
    m_render_outline = render_outline;
}

void RadioButtonGroup::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("RadioButtonGroup");
    boost::shared_ptr<SetCheckedButtonAction> set_checked_button_action(new SetCheckedButtonAction(this));
    editor->Attribute<int>("Checked Button", m_checked_button, set_checked_button_action);
}

const std::vector<RadioButtonGroup::ButtonSlot>& RadioButtonGroup::ButtonSlots() const
{
    return m_button_slots;
}

void RadioButtonGroup::ConnectSignals()
{
    for (unsigned int i = 0; i < m_button_slots.size(); ++i) {
        m_button_slots[i].connection = Connect(m_button_slots[i].button->CheckedSignal, ButtonClickedFunctor(this, m_button_slots[i].button, i));
    }
    SetCheck(m_checked_button);
}

void RadioButtonGroup::HandleRadioClick(int index, bool set_check)
{
    assert(m_checked_button == NO_BUTTON ||
           (0 <= m_checked_button && m_checked_button < static_cast<int>(m_button_slots.size())));
    if (m_checked_button != NO_BUTTON) {
        m_button_slots[m_checked_button].connection.block();
        m_button_slots[m_checked_button].button->SetCheck(false);
        m_button_slots[m_checked_button].connection.unblock();
    }
    if (set_check && index != NO_BUTTON) {
        m_button_slots[index].connection.block();
        m_button_slots[index].button->SetCheck(true);
        m_button_slots[index].connection.unblock();
    }
    ButtonChangedSignal(m_checked_button = index);
}

void RadioButtonGroup::Reconnect()
{
    for (unsigned int i = 0; i < m_button_slots.size(); ++i) {
        m_button_slots[i].connection.disconnect();
    }
    ConnectSignals();
}
