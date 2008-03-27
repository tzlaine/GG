// -*- C++ -*-
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

/** \file StyleFactory.h
    Contains the StyleFactory class, which creates new controls for internal use by dialogs and other controls. */

#ifndef _GG_StyleFactory_h_
#define _GG_StyleFactory_h_

#include  <GG/Font.h>
#include  <GG/MultiEdit.h>
#include  <GG/StaticGraphic.h>


namespace GG {

class Button;
class ColorDlg;
class DropDownList;
class DynamicGraphic;
class FileDlg;
class ListBox;
class MenuBar;
class RadioButtonGroup;
class Scroll;
class Slider;
template <class T>
class Spin;
class StateButton;
class TabBar;
class TabWnd;
class TextControl;
class Texture;
class ThreeButtonDlg;

/** Creates new dialogs and Controls.  This class can be used to create a look for the entire GUI by providing
    user-defined subclasses of the standard Controls.  A Control or dialog can then use the StyleFactory to create the
    dialogs/controls it needs (e.g. a vertical Scroll uses NewVScrollTabButton() to create its tab).  This reduces the
    amount of subclass code that is required to produce a set of custom GG classes.  Note that the subcontrol factory
    methods below may be the same as or different from their generic counterparts, allowing greater flexibility in which
    controls are created in different contexts.  For example, NewButton() may create a generic, basic GG Button, but
    NewHSliderTabButton() may produce a specialized button that looks better on horizontal sliders.  By default, all
    subcontrol methods invoke the more generic control method for the type of control they each return. */
class GG_API StyleFactory
{
public:
    /** \name Structors */ ///@{
    StyleFactory(); ///< Default ctor.
    virtual ~StyleFactory(); ///< Virtual dtor.
    //@}

    /** \name Controls */ ///@{
    /** Returns a new GG Button. */
    virtual Button*            NewButton(int x, int y, int w, int h, const std::string& str,
                                         const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                         Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG StateButton. */
    virtual StateButton*       NewStateButton(int x, int y, int w, int h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                              Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                              StateButtonStyle style = SBSTYLE_3D_XBOX, Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG RadioButtonGroup. */
    virtual RadioButtonGroup*  NewRadioButtonGroup(int x, int y, int w, int h, Orientation orientation) const;

    /** Returns a new GG DropDownList. */
    virtual DropDownList*      NewDropDownList(int x, int y, int w, int h, int drop_ht, Clr color,
                                               Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG DynamicGraphic. */
    virtual DynamicGraphic*    NewDynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height,
                                                 int margin, const std::vector<boost::shared_ptr<Texture> >& textures,
                                                 Flags<GraphicStyle> style = GRAPHIC_NONE, int frames = -1, Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG Edit. */
    virtual Edit*              NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font,
                                       Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                       Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG ListBox. */
    virtual ListBox*           NewListBox(int x, int y, int w, int h, Clr color, Clr interior = CLR_ZERO,
                                          Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG MenuBar. */
    virtual MenuBar*           NewMenuBar(int x, int y, int w, const boost::shared_ptr<Font>& font,
                                          Clr text_color = CLR_WHITE, Clr color = CLR_BLACK,
                                          Clr interior = CLR_SHADOW) const;

    /** Returns a new GG MultiEdit. */
    virtual MultiEdit*         NewMultiEdit(int x, int y, int w, int h, const std::string& str,
                                            const boost::shared_ptr<Font>& font, Clr color, Flags<MultiEditStyle> style = MULTI_LINEWRAP,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG Scroll. */
    virtual Scroll*            NewScroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior,
                                         Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new GG Slider. */
    virtual Slider*            NewSlider(int x, int y, int w, int h, int min, int max, Orientation orientation,
                                         SliderLineStyle style, Clr color, int tab_width, int line_width = 5,
                                         Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<int>*         NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                          Clr interior = CLR_ZERO, Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<double>*      NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                             const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                             Clr interior = CLR_ZERO, Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG StaticGraphic. */
    virtual StaticGraphic*     NewStaticGraphic(int x, int y, int w, int h, const boost::shared_ptr<Texture>& texture,
                                                Flags<GraphicStyle> style = GRAPHIC_NONE, Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG TabBar. */
    virtual TabBar*            NewTabBar(int x, int y, int w, const boost::shared_ptr<Font>& font, Clr color,
                                         Clr text_color = CLR_BLACK, TabBarStyle style = TAB_BAR_ATTACHED,
                                         Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new GG TextControl. */
    virtual TextControl*       NewTextControl(int x, int y, int w, int h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK,
                                              Flags<TextFormat> format = FORMAT_NONE, Flags<WndFlag> flags = Flags<WndFlag>()) const;

    /** Returns a new GG TextControl whose size is exactly that required to hold its text. */
    virtual TextControl*       NewTextControl(int x, int y, const std::string& str, const boost::shared_ptr<Font>& font,
                                              Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
                                              Flags<WndFlag> flags = Flags<WndFlag>()) const;
    //@}

    /** \name Subcontrols */ ///@{
    /** Returns a new ListBox, to be used in a DropDownList. */
    virtual ListBox*           NewDropDownListListBox(int x, int y, int w, int h, Clr color, Clr interior = CLR_ZERO,
                                                      Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new vertical Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxVScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                                 Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a ListBox. */
    virtual Scroll*            NewListBoxHScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                                 Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new vertical Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditVScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                                   Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new horizontal Scroll, to be used in a MultiEdit. */
    virtual Scroll*            NewMultiEditHScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                                   Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new up (decrease) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollUpButton(int x, int y, int w, int h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new down (increase) Button, to be used in a vertical Scroll. */
    virtual Button*            NewScrollDownButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new tab Button, to be used in a vertical Scroll. */
    virtual Button*            NewVScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new left (decrease) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollLeftButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new right (increase) Button, to be used in a horizontal Scroll. */
    virtual Button*            NewScrollRightButton(int x, int y, int w, int h, const std::string& str,
                                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                    Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new tab Button, to be used in a horizontal Scroll. */
    virtual Button*            NewHScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new tab Button, to be used in a vertical Slider. */
    virtual Button*            NewVSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new tab Button, to be used in a horizontal Slider. */
    virtual Button*            NewHSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new increase Button, to be used in a Spin. */
    virtual Button*            NewSpinIncrButton(int x, int y, int w, int h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new decrease Button, to be used in a Spin. */
    virtual Button*            NewSpinDecrButton(int x, int y, int w, int h, const std::string& str,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                 Flags<WndFlag> flags = CLICKABLE | REPEAT_BUTTON_DOWN) const;

    /** Returns a new Edit, to be used in an editable Spin. */
    virtual Edit*              NewSpinEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font,
                                           Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                           Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new StateButton, to be used in a TabBar. */
    virtual StateButton*       NewTabBarTab(int x, int y, int w, int h, const std::string& str,
                                            const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            StateButtonStyle style = SBSTYLE_3D_TOP_ATTACHED_TAB, Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarLeftButton(int x, int y, int w, int h, const std::string& str,
                                                   const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                   Flags<WndFlag> flags = CLICKABLE) const;

    /** Returns a new left Button, to be used in a TabBar. */
    virtual Button*            NewTabBarRightButton(int x, int y, int w, int h, const std::string& str,
                                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                                    Flags<WndFlag> flags = CLICKABLE) const;
    //@}

    /** \name Wnds */ ///@{
    /** Returns a new GG TabWnd. */
    virtual TabWnd*            NewTabWnd(int x, int y, int w, int h, const boost::shared_ptr<Font>& font, Clr color,
                                         Clr text_color = CLR_BLACK, TabBarStyle style = TAB_BAR_ATTACHED,
                                         Flags<WndFlag> flags = CLICKABLE | DRAGABLE) const;
    //@}

    /** \name Dialogs */ ///@{
    /** Returns a new GG ColorDlg. */
    virtual ColorDlg*          NewColorDlg(int x, int y, const boost::shared_ptr<Font>& font,
                                           Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK) const;

    /** Returns a new GG ColorDlg that has a starting color specified. */
    virtual ColorDlg*          NewColorDlg(int x, int y, Clr original_color, const boost::shared_ptr<Font>& font,
                                           Clr dialog_color, Clr border_color, Clr text_color = CLR_BLACK) const;

    /** Returns a new GG FileDlg. */
    virtual FileDlg*           NewFileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                          Clr text_color = CLR_BLACK) const;

    /** Returns a new GG ThreeButtonDlg. */
    virtual ThreeButtonDlg*    NewThreeButtonDlg(int x, int y, int w, int h, const std::string& msg,
                                                 const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                                 Clr button_color, Clr text_color, int buttons, const std::string& zero = "",
                                                 const std::string& one = "", const std::string& two = "") const;

    /** Returns a new GG ThreeButtonDlg that automatically centers itself in the app. */
    virtual ThreeButtonDlg*    NewThreeButtonDlg(int w, int h, const std::string& msg, const boost::shared_ptr<Font>& font,
                                                 Clr color, Clr border_color, Clr button_color, Clr text_color, int buttons,
                                                 const std::string& zero = "", const std::string& one = "",
                                                 const std::string& two = "") const;
    //@}

    /** Deletes \a wnd.  It is only necessary to use this method to destroy Wnds when the factory that created them
        exists in a plugin. */
    virtual void               DeleteWnd(Wnd* wnd) const;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {}
};

} // namespace GG

#endif // _GG_StyleFactory_h_
