// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2006 T. Zachary Laine

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
   
/** \file TabWnd.h
    Contains the TabWnd class, which encapsulates a set of tabbed windows. */

#ifndef _GG_TabWnd_h_
#define _GG_TabWnd_h_

#include <GG/Button.h>


namespace GG {

class TabBar;
class WndEvent;

/** Contains several Wnds and a TabBar, and only displays the Wnd currently
    selected in the TabBar. */
class GG_API TabWnd : public Wnd
{
public:
    /** \name Signal Types */ ///@{
    /** Emitted when the currently-selected Wnd has changed; the new selected Wnd's index in the group is provided (this
        may be NO_WND if no Wnd is currently selected). */
    typedef boost::signal<void (int)> WndChangedSignalType;
    //@}

    /** \name Slot Types */ ///@{
    typedef WndChangedSignalType::slot_type WndChangedSlotType; ///< Type of functor(s) invoked on a WndChangedSignalType.
    //@}

    /** \name Structors */ ///@{
    /** Basic ctor. */
    TabWnd(int x, int y, int w, int h, const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
           TabBarStyle style = TAB_BAR_ATTACHED, Flags<WndFlag> flags = CLICKABLE | DRAGABLE);
    ~TabWnd();
    //@}

    /** \name Accessors */ ///@{
    virtual Pt MinUsableSize() const;

    /** Returns the Wnd currently visible in the TabWnd, or 0 if there is none. */
    Wnd* CurrentWnd() const;

    /** Returns the index into the sequence of Wnds in this TabWnd of the Wnd
        currently shown.  NO_WND is returned if there is no Wnd currently
        visible. */
    int  CurrentWndIndex() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void Render();

    /** Adds \a wnd to the sequence of Wnds in this TabWnd, with name \a name.
        \a name can be used later to remove the Wnd (\a name is not checked for
        uniqueness).  Returns the index at which \a wnd is placed. */
    int  AddWnd(Wnd* wnd, const std::string& name);

    /** Adds \a wnd to the sequence of Wnds in this TabWnd, inserting it at the
        \a index location within the sequence.  \a name can be used later to
        remove the Wnd (\a name is not checked for uniqueness).  Not range
        checked. */
    void InsertWnd(int index, Wnd* wnd, const std::string& name);

    /** Removes and returns the first Wnd previously added witht he name \a name
        from the sequence of Wnds in this TabWnd. */
    Wnd* RemoveWnd(const std::string& name);

    /** Sets the currently visible Wnd in the sequence to the Wnd in the \a
        index position within the sequence.  Not range checked. */
    void SetCurrentWnd(int index);
    //@}

    mutable WndChangedSignalType WndChangedSignal; ///< The Wnd change signal object for this Button

    /** The invalid Wnd position index that there is no currently-selected Wnd. */
    static const int NO_WND;

protected:
    /** \name Structors */ ///@{
    TabWnd(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    const TabBar*                                     GetTabBar() const;
    const std::vector<std::pair<Wnd*, std::string> >& Wnds() const;
    //@}

private:
    void TabChanged(int tab_index);

    TabBar*                                    m_tab_bar;
    std::vector<std::pair<Wnd*, std::string> > m_wnds;
    Wnd*                                       m_current_wnd;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Contains a sequence of buttons (hereafter "tabs") that act together in a
    RadioButtonGroup.  This class is intended to be used to select the current
    Wnd in a TabWnd. */
class GG_API TabBar : public Control
{
public:
    /** \name Signal Types */ ///@{
    /** Emitted when the currently-selected tab has changed; the new selected tab's index in the group is provided (this
        may be NO_TAB if no tab is currently selected). */
    typedef boost::signal<void (int)> TabChangedSignalType;
    //@}

    /** \name Slot Types */ ///@{
    typedef TabChangedSignalType::slot_type TabChangedSlotType; ///< Type of functor(s) invoked on a TabChangedSignalType.
    //@}

    /** \name Structors */ ///@{
    /** Basic ctor. */
    TabBar(int x, int y, int w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
           TabBarStyle style = TAB_BAR_ATTACHED, Flags<WndFlag> flags = CLICKABLE);
    //@}

    /** \name Accessors */ ///@{
    virtual Pt MinUsableSize() const;

    /** Returns the index into the sequence of tabs in this TabBar of the tab
        currently selected.  NO_TAB is returned if there is no tab currently
        selected. */
    int CurrentTabIndex() const;
    //@}

    /** \name Mutators */ ///@{
    virtual void SizeMove(const Pt& ul, const Pt& lr);
    virtual void Render();

    /** Adds a tab called \a name to the sequence of tabs in this TabBar.  \a
        name can be used later to remove the tab (\a name is not checked for
        uniqueness).  Returns the index at which the tab is placed. */
    int  AddTab(const std::string& name);

    /** Adds tab to the sequence of tabs in this TabBar, inserting it at the \a
        index location within the sequence.  \a name can be used later to remove
        the tab (\a name is not checked for uniqueness).  Not range checked. */
    void InsertTab(int index, const std::string& name);

    /** Removes the first tab previously added witht he name \a name from the
        sequence of tab in this TabBar. */
    void RemoveTab(const std::string& name);

    /** Sets the current tab in the sequence to the tab in the \a index position
        within the sequence.  Not range checked. */
    void SetCurrentTab(int index);
    //@}

    mutable TabChangedSignalType TabChangedSignal; ///< The tab change signal object for this Button

    /** The invalid tab position index that there is no currently-selected tab. */
    static const int NO_TAB;

    /** The default width to use for the left and right buttons. */
    static const int BUTTON_WIDTH;

protected:
    /** \name Structors */ ///@{
    TabBar(); ///< default ctor
    //@}

    /** \name Accessors */ ///@{
    const Button* LeftButton() const;
    const Button* RightButton() const;
    //@}

    /** \name Mutators */ ///@{
    virtual bool EventFilter(Wnd* w, const WndEvent& event);
    //@}

private:
    virtual void DistinguishCurrentTab(const std::vector<StateButton*>& tab_buttons);

    void TabChanged(int index);
    void LeftClicked();
    void RightClicked();
    void BringTabIntoView(int index);

    RadioButtonGroup*         m_tabs;
    std::vector<StateButton*> m_tab_buttons;
    boost::shared_ptr<Font>   m_font;
    Button*                   m_left_button;
    Button*                   m_right_button;
    Layout*                   m_left_right_button_layout;
    Flags<TextFormat>         m_format;
    Clr                       m_text_color;
    TabBarStyle               m_style;
    int                       m_first_tab_shown;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::TabWnd::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd)
        & BOOST_SERIALIZATION_NVP(m_tab_bar)
        & BOOST_SERIALIZATION_NVP(m_wnds)
        & BOOST_SERIALIZATION_NVP(m_current_wnd);
}

template <class Archive>
void GG::TabBar::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Control)
        & BOOST_SERIALIZATION_NVP(m_tabs)
        & BOOST_SERIALIZATION_NVP(m_tab_buttons)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_left_button)
        & BOOST_SERIALIZATION_NVP(m_right_button)
        & BOOST_SERIALIZATION_NVP(m_left_right_button_layout)
        & BOOST_SERIALIZATION_NVP(m_format)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_style)
        & BOOST_SERIALIZATION_NVP(m_first_tab_shown);
}

#endif
