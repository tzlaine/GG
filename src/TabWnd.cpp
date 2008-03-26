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

#include <GG/TabWnd.h>

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    int TabHeightFromFont(const boost::shared_ptr<Font>& font)
    { return font->Lineskip() + 10; }
}

////////////////////////////////////////////////
// GG::TabWnd
////////////////////////////////////////////////
// static(s)
const int TabWnd::NO_WND = RadioButtonGroup::NO_BUTTON;

TabWnd::TabWnd() :
    m_tab_bar(0),
    m_current_wnd(0)
{}

TabWnd::~TabWnd()
{
    for (unsigned int i = 0; i < m_wnds.size(); ++i) {
        delete m_wnds[i].first;
    }
}

TabWnd::TabWnd(int x, int y, int w, int h, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
               TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = CLICKABLE | DRAGABLE*/) :
    Wnd(x, y, w, h, flags),
    m_tab_bar(GetStyleFactory()->NewTabBar(0, 0, w, font, color, text_color, style, CLICKABLE)),
    m_current_wnd(0)
{
    Layout* layout = new Layout(0, 0, w, h, 2, 1);
    layout->SetRowStretch(1, 1.0);
    layout->Add(m_tab_bar, 0, 0);
    SetLayout(layout);
    Connect(m_tab_bar->TabChangedSignal, &TabWnd::TabChanged, this);
}

Pt TabWnd::MinUsableSize() const
{
    Pt retval = m_tab_bar->MinUsableSize();
    retval.y *= 2;
    return retval;
}

Wnd* TabWnd::CurrentWnd() const
{ return m_current_wnd; }

int TabWnd::CurrentWndIndex() const
{ return m_tab_bar->CurrentTabIndex(); }

void TabWnd::Render()
{}

int TabWnd::AddWnd(Wnd* wnd, const std::string& name)
{
    int retval = m_wnds.size();
    InsertWnd(m_wnds.size(), wnd, name);
    return retval;
}

void TabWnd::InsertWnd(int index, Wnd* wnd, const std::string& name)
{
    m_wnds.insert(m_wnds.begin() + index, std::make_pair(wnd, name));
    m_tab_bar->InsertTab(index, name);
    GetLayout()->SetMinimumRowHeight(0, m_tab_bar->MinUsableSize().y + 2 * 5);
}

Wnd* TabWnd::RemoveWnd(const std::string& name)
{
    Wnd* retval = 0;
    int index = NO_WND;
    for (unsigned int i = 0; i < m_wnds.size(); ++i) {
        if (m_wnds[i].second == name) {
            index = i;
            break;
        }
    }
    if (index != NO_WND) {
        retval = m_wnds[index].first;
        m_wnds.erase(m_wnds.begin() + index);
        m_tab_bar->RemoveTab(name);
        GetLayout()->SetMinimumRowHeight(0, m_tab_bar->MinUsableSize().y + 2 * 5);
    }
    return retval;
}

void TabWnd::SetCurrentWnd(int index)
{ m_tab_bar->SetCurrentTab(index); }

const TabBar* TabWnd::GetTabBar() const
{ return m_tab_bar; }

const std::vector<std::pair<Wnd*, std::string> >& TabWnd::Wnds() const
{ return m_wnds; }

void TabWnd::TabChanged(int index)
{
    assert(0 <= index && index < static_cast<int>(m_wnds.size()));
    Wnd* old_current_wnd = m_current_wnd;
    m_current_wnd = m_wnds[index].first;
    if (m_current_wnd != old_current_wnd) {
        Layout* layout = GetLayout();
        layout->Remove(old_current_wnd);
        layout->Add(m_current_wnd, 1, 0);
    }
    WndChangedSignal(index);
}


////////////////////////////////////////////////
// GG::TabBar
////////////////////////////////////////////////
// static(s)
const int TabBar::NO_TAB = TabWnd::NO_WND;
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
               TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = CLICKABLE*/) :
    Control(x, y, w, TabHeightFromFont(font), flags),
    m_tabs(0),
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

    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();

    m_tabs = style_factory->NewRadioButtonGroup(0, 0, w, TabHeightFromFont(font), HORIZONTAL);
    m_tabs->ExpandButtons(true);
    m_tabs->ExpandButtonsProportionally(true);

    m_left_right_button_layout->SetColumnStretch(0, 1);
    m_left_right_button_layout->SetColumnStretch(1, 0);
    m_left_right_button_layout->SetColumnStretch(2, 0);

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

int TabBar::CurrentTabIndex() const
{ return m_tabs->CheckedButton(); }

void TabBar::SizeMove(const Pt& ul, const Pt& lr)
{
    m_tabs->Resize(Pt(m_tabs->Size().x, lr.y -  ul.y));
    m_left_right_button_layout->SizeMove(Pt(0, 0), lr - ul);
    Control::SizeMove(ul, lr);
}

void TabBar::Render()
{}

int TabBar::AddTab(const std::string& name)
{
    int retval = m_tab_buttons.size();
    InsertTab(m_tab_buttons.size(), name);
    return retval;
}

void TabBar::InsertTab(int index, const std::string& name)
{
    assert(0 <= index && index <= static_cast<int>(m_tab_buttons.size()));
    boost::shared_ptr<StyleFactory> style_factory = GetStyleFactory();
    StateButton* button = style_factory->NewTabBarTab(0, 0, 1, 1, name,
                                                      m_font, FORMAT_CENTER, Color(),
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
        int right_side = m_left_right_button_layout->Visible() ?
            m_left_button->UpperLeft().x :
            LowerRight().x;
        m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
    }
    if (m_tabs->CheckedButton() == RadioButtonGroup::NO_BUTTON)
        m_tabs->SetCheck(0);
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
    if (m_tabs->CheckedButton() == RadioButtonGroup::NO_BUTTON && !m_tab_buttons.empty())
        m_tabs->SetCheck(0);
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
    if (index != RadioButtonGroup::NO_BUTTON) {
        BringTabIntoView(index);
        DistinguishCurrentTab(m_tab_buttons);
        TabChangedSignal(index);
    }
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
    int right_side = m_left_right_button_layout->Visible() ?
        m_left_button->UpperLeft().x :
        LowerRight().x;
    m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
    m_left_button->Disable(false);
}

void TabBar::BringTabIntoView(int index)
{
    while (m_tab_buttons[index]->UpperLeft().x < UpperLeft().x) {
        LeftClicked();
    }
    int right_side = m_left_right_button_layout->Visible() ?
        m_left_button->UpperLeft().x :
        LowerRight().x;
    if (m_tab_buttons[index]->Width() < Width()) {
        while (right_side < m_tab_buttons[index]->LowerRight().x && index != m_first_tab_shown) {
            RightClicked();
        }
    } else {
        m_tabs->OffsetMove(Pt(m_tab_buttons[m_first_tab_shown]->UpperLeft().x - m_tab_buttons[index]->UpperLeft().x, 0));
        m_right_button->Disable(m_tab_buttons.back()->LowerRight().x <= right_side);
        m_left_button->Disable(false);
    }
}

bool TabBar::EventFilter(Wnd* w, const WndEvent& event)
{
    if (event.Type() == WndEvent::LButtonDown ||
        event.Type() == WndEvent::RButtonDown)
        MoveChildUp(m_left_right_button_layout);
    return false;
}
