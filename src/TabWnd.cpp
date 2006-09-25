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

/* $Id: Edit.cpp 528 2006-03-04 02:56:33Z tzlaine $ */

#include <GG/TabWnd.h>

#ifndef _GG_Layout_h_
#include <GG/Layout.h>
#endif

#ifndef _GG_StyleFactory_h_
#include <GG/StyleFactory.h>
#endif


using namespace GG;

namespace {
    int TabHeightFromFont(const boost::shared_ptr<Font>& font)
    { return font->Lineskip() + 10; }
}

////////////////////////////////////////////////
// GG::TabWnd
////////////////////////////////////////////////

////////////////////////////////////////////////
// GG::TabBar
////////////////////////////////////////////////
// static(s)
const int TabBar::NO_TAB = -1;
const int TabBar::BUTTON_WIDTH = 10;

TabBar::TabBar() :
    Control(),
    m_tabs(0),
    m_left_button(0),
    m_right_button(0),
    m_left_right_button_layout(0),
    m_text_color(CLR_BLACK),
    m_style(TAB_BAR_ATTACHED),
    m_first_tab_shown(0)
{}

TabBar::TabBar(int x, int y, int w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
               TabBarStyle style/* = TAB_BAR_ATTACHED*/, Uint32 flags/* = CLICKABLE*/) :
    Control(x, y, w, TabHeightFromFont(font), flags),
    m_tabs(new RadioButtonGroup(0, 0, w, TabHeightFromFont(font), HORIZONTAL)),
    m_font(font),
    m_left_button(0),
    m_right_button(0),
    m_left_right_button_layout(new Layout(0, 0, w, TabHeightFromFont(font), 1, 3)),
    m_text_color(text_color),
    m_style(style),
    m_first_tab_shown(0)
{
    SetColor(color);

    EnableChildClipping(true);

    m_tabs->ExpandButtons(true);
    m_tabs->ExpandButtonsProportionally(true);

    m_left_right_button_layout->SetColumnStretch(0, 1);
    m_left_right_button_layout->SetColumnStretch(1, 0);
    m_left_right_button_layout->SetColumnStretch(2, 0);

    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();
    m_left_button = style_factory->NewTabBarLeftButton(0, 0, BUTTON_WIDTH, Height(), "-", m_font, Color(), m_text_color);
    m_right_button = style_factory->NewTabBarRightButton(0, 0, BUTTON_WIDTH, Height(), "+", m_font, Color(), m_text_color);
    m_left_right_button_layout->SetMinimumColumnWidth(1, m_left_button->Width());
    m_left_right_button_layout->SetMinimumColumnWidth(2, m_right_button->Width());
    m_left_right_button_layout->Add(m_left_button, 0, 1);
    m_left_right_button_layout->Add(m_right_button, 0, 2);
    m_left_right_button_layout->Hide();

    AttachChild(m_tabs);
    AttachChild(m_left_right_button_layout);

    Connect(m_tabs->ButtonChangedSignal, &TabBar::TabChanged, this);
    Connect(m_left_button->ClickedSignal, &TabBar::LeftClicked, this);
    Connect(m_right_button->ClickedSignal, &TabBar::RightClicked, this);
}

Pt TabBar::MinUsableSize() const
{
    int y = 0;
    for (unsigned int i = 0; i < m_tab_buttons.size(); ++i) {
        int button_min_y = m_tab_buttons[i]->MinUsableSize().y;
        if (y < button_min_y)
            y = button_min_y;
    }
    return Pt(4 * BUTTON_WIDTH, y);
}

int TabBar::CurrentTab() const
{ return m_tabs->CheckedButton(); }

void TabBar::SizeMove(const Pt& ul, const Pt& lr)
{
    m_left_right_button_layout->SizeMove(Pt(0, 0), lr - ul);
    Control::SizeMove(ul, lr);
}

void TabBar::Render()
{}

void TabBar::AddTab(const std::string& name)
{ InsertTab(m_tab_buttons.size(), name); }

void TabBar::InsertTab(int index, const std::string& name)
{
    assert(0 <= index && index <= static_cast<int>(m_tab_buttons.size()));
    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();
    StateButton* button = style_factory->NewTabBarTab(0, 0, 1, 1, name,
                                                      m_font, TF_CENTER, Color(),
                                                      m_text_color, CLR_ZERO,
                                                      m_style == TAB_BAR_ATTACHED ?
                                                      SBSTYLE_3D_TOP_ATTACHED_TAB :
                                                      SBSTYLE_3D_TOP_DETACHED_TAB);
    button->InstallEventFilter(this);
    m_tab_buttons.insert(m_tab_buttons.begin() + index, button);
    m_tabs->InsertButton(index, m_tab_buttons[index]);
    if (Width() < m_tabs->Width()) {
        m_left_right_button_layout->Show();
        m_left_button->Disable(m_first_tab_shown == 0);
        m_right_button->Disable(m_first_tab_shown == static_cast<int>(m_tab_buttons.size()) - 1);
    }
}

void TabBar::RemoveTab(const std::string& name)
{
    int index = NO_TAB;
    for (unsigned int i = 0; i < m_tab_buttons.size(); ++i) {
        if (m_tab_buttons[i]->WindowText() == name) {
            index = i;
            break;
        }
    }
    assert(0 <= index && index < static_cast<int>(m_tab_buttons.size()));

    m_tab_buttons[index]->RemoveEventFilter(this);
    m_tabs->RemoveButton(m_tab_buttons[index]);
    delete m_tab_buttons[index];
    m_tab_buttons.erase(m_tab_buttons.begin() + index);
    if (m_tabs->Width() <= Width())
        m_left_right_button_layout->Hide();
}

void TabBar::SetCurrentTab(int index)
{ m_tabs->SetCheck(index); }

const Button* TabBar::LeftButton() const
{ return m_left_button; }

const Button* TabBar::RightButton() const
{ return m_right_button; }

void TabBar::DistinguishCurrentTab(const std::vector<StateButton*>& tab_buttons)
{ m_tabs->RaiseCheckedButton(); }

void TabBar::TabChanged(int index)
{
    BringTabIntoView(index);
    DistinguishCurrentTab(m_tab_buttons);
    TabChangedSignal(index);
}

void TabBar::LeftClicked()
{
    assert(0 < m_first_tab_shown);
    m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[m_first_tab_shown - 1]->UpperLeft().x, 0));
    --m_first_tab_shown;
    m_left_button->Disable(m_first_tab_shown == 0);
    m_right_button->Disable(false);
}

void TabBar::RightClicked()
{
    assert(m_first_tab_shown < static_cast<int>(m_tab_buttons.size()) - 1);
    m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[m_first_tab_shown + 1]->UpperLeft().x, 0));
    ++m_first_tab_shown;
    m_right_button->Disable(m_first_tab_shown == static_cast<int>(m_tab_buttons.size()) - 1);
    m_left_button->Disable(false);
}

void TabBar::BringTabIntoView(int index)
{
    while (m_tab_buttons[index]->UpperLeft().x < UpperLeft().x) {
        LeftClicked();
    }
    if (m_tab_buttons[index]->Width() < Width()) {
        int right_side = m_left_right_button_layout->Visible() ?
            m_left_button->UpperLeft().x :
            LowerRight().x;
        while (right_side < m_tab_buttons[index]->LowerRight().x && index != m_first_tab_shown) {
            RightClicked();
        }
    } else {
        m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[index]->UpperLeft().x, 0));
        m_right_button->Disable(m_first_tab_shown == static_cast<int>(m_tab_buttons.size()) - 1);
        m_left_button->Disable(false);
    }
}

bool TabBar::EventFilter(Wnd* w, const Event& event)
{
    if (event.Type() == Event::LButtonDown ||
        event.Type() == Event::RButtonDown)
        MoveChildUp(m_left_right_button_layout);
    return false;
}
