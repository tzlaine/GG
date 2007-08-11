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

#include <GG/StyleFactory.h>

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>
#include <GG/TextControl.h>

#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>

using namespace GG;

StyleFactory::StyleFactory()
{}

StyleFactory::~StyleFactory()
{}

Button* StyleFactory::NewButton(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new Button(x, y, w, h, str, font, color, text_color, flags);
}

StateButton* StyleFactory::NewStateButton(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font,
                                          Flags<TextFormat> format, Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                          StateButtonStyle style/* = SBSTYLE_3D_XBOX*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new StateButton(x, y, w, h, str, font, format, color, text_color, interior, style, flags);
}

RadioButtonGroup* StyleFactory::NewRadioButtonGroup(int x, int y, int w, int h, Orientation orientation) const
{
    return new RadioButtonGroup(x, y, w, h, orientation);
}

DropDownList* StyleFactory::NewDropDownList(int x, int y, int w, int h, int drop_ht, Clr color,
                                            Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new DropDownList(x, y, w, h, drop_ht, color, flags);
}

DynamicGraphic* StyleFactory::NewDynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height,
                                                int margin, const std::vector<boost::shared_ptr<Texture> >& textures,
                                                Flags<GraphicStyle> style/* = GRAPHIC_NONE*/, int frames/* = -1*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{
    return new DynamicGraphic(x, y, w, h, loop, frame_width, frame_height, margin, textures, style, frames, flags);
}

Edit* StyleFactory::NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font,
                            Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                            Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new Edit(x, y, w, str, font, color, text_color, interior, flags);
}

ListBox* StyleFactory::NewListBox(int x, int y, int w, int h, Clr color, Clr interior/* = CLR_ZERO*/,
                                  Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new ListBox(x, y, w, h, color, interior, flags);
}

MenuBar* StyleFactory::NewMenuBar(int x, int y, int w, const boost::shared_ptr<Font>& font, Clr text_color/* = CLR_WHITE*/,
                                  Clr color/* = CLR_BLACK*/, Clr interior/* = CLR_SHADOW*/) const
{
    return new MenuBar(x, y, w, font, text_color, color, interior);
}

MultiEdit* StyleFactory::NewMultiEdit(int x, int y, int w, int h, const std::string& str, const boost::shared_ptr<Font>& font,
                                      Clr color, Flags<MultiEditStyle> style/* = MULTI_LINEWRAP*/, Clr text_color/* = CLR_BLACK*/,
                                      Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new MultiEdit(x, y, w, h, str, font, color, style, text_color, interior, flags);
}

Scroll* StyleFactory::NewScroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior,
                                Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return new Scroll(x, y, w, h, orientation, color, interior, flags);
}

Slider* StyleFactory::NewSlider(int x, int y, int w, int h, int min, int max, Orientation orientation,
                                SliderLineStyle style, Clr color, int tab_width, int line_width/* = 5*/,
                                Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new Slider(x, y, w, h, min, max, orientation, style, color, tab_width, line_width, flags);
}

Spin<int>* StyleFactory::NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                    Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new Spin<int>(x, y, w, value, step, min, max, edits, font, color, text_color, interior, flags);
}

Spin<double>* StyleFactory::NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new Spin<double>(x, y, w, value, step, min, max, edits, font, color, text_color, interior, flags);
}

StaticGraphic* StyleFactory::NewStaticGraphic(int x, int y, int w, int h, const boost::shared_ptr<Texture>& texture,
                                              Flags<GraphicStyle> style/* = GRAPHIC_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{
    return new StaticGraphic(x, y, w, h, texture, style, flags);
}

TextControl* StyleFactory::NewTextControl(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                                          Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{
    return new TextControl(x, y, w, h, str, font, color, format, flags);
}

TextControl* StyleFactory::NewTextControl(int x, int y, const std::string& str, const boost::shared_ptr<Font>& font,
                                          Clr color/* = CLR_BLACK*/, Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{
    return new TextControl(x, y, str, font, color, format, flags);
}

TabBar* StyleFactory::NewTabBar(int x, int y, int w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return new TabBar(x, y, w, font, color, text_color, style, flags);
}

ListBox* StyleFactory::NewDropDownListListBox(int x, int y, int w, int h, Clr color, Clr interior/* = CLR_ZERO*/,
                                              Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewListBox(x, y, w, h, color, interior, flags);
}

Scroll* StyleFactory::NewListBoxVScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewScroll(x, y, w, h, VERTICAL, color, interior, flags);
}

Scroll* StyleFactory::NewListBoxHScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags);
}

Scroll* StyleFactory::NewMultiEditVScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewScroll(x, y, w, h, VERTICAL, color, interior, flags);
}

Scroll* StyleFactory::NewMultiEditHScroll(int x, int y, int w, int h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags);
}

Button* StyleFactory::NewScrollUpButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewScrollDownButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewVScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewScrollLeftButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewScrollRightButton(int x, int y, int w, int h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewHScrollTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewVSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewHSliderTabButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewSpinIncrButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewSpinDecrButton(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Edit* StyleFactory::NewSpinEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewEdit(x, y, w, str, font, color, text_color, interior, flags);
}

StateButton* StyleFactory::NewTabBarTab(int x, int y, int w, int h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Flags<TextFormat> format, Clr color,
                                        Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                        StateButtonStyle style/* = SBSTYLE_3D_TOP_ATTACHED_TAB*/, Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    StateButton* retval = NewStateButton(x, y, w, h, str, font, format, color, text_color, interior, style, flags);
    retval->Resize(retval->MinUsableSize() + Pt(12, 0));
    return retval;
}

Button* StyleFactory::NewTabBarLeftButton(int x, int y, int w, int h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

Button* StyleFactory::NewTabBarRightButton(int x, int y, int w, int h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags /*= CLICKABLE*/) const
{
    return NewButton(x, y, w, h, str, font, color, text_color, flags);
}

TabWnd* StyleFactory::NewTabWnd(int x, int y, int w, int h, const boost::shared_ptr<Font>& font, Clr color,
                                Clr text_color/* = CLR_BLACK*/, TabBarStyle style/* = TAB_BAR_ATTACHED*/,
                                Flags<WndFlag> flags/* = CLICKABLE | DRAGABLE*/) const
{
    return new TabWnd(x, y, w, h, font, color, text_color, style, flags);
}

ColorDlg* StyleFactory::NewColorDlg(int x, int y, const boost::shared_ptr<Font>& font,
                                    Clr dialog_color, Clr border_color, Clr text_color/* = CLR_BLACK*/) const
{
    return new ColorDlg(x, y, font, dialog_color, border_color, text_color);
}


ColorDlg* StyleFactory::NewColorDlg(int x, int y, Clr original_color, const boost::shared_ptr<Font>& font,
                                    Clr dialog_color, Clr border_color, Clr text_color/* = CLR_BLACK*/) const
{
    return new ColorDlg(x, y, original_color, font, dialog_color, border_color, text_color);
}


FileDlg* StyleFactory::NewFileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                                  const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                  Clr text_color/* = CLR_BLACK*/) const
{
    return new FileDlg(directory, filename, save, multi, font, color, border_color, text_color);
}


ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(int x, int y, int w, int h, const std::string& msg,
                                                const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                                Clr button_color, Clr text_color, int buttons,
                                                const std::string& zero/* = ""*/, const std::string& one/* = ""*/,
                                                const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(x, y, w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(int w, int h, const std::string& msg, const boost::shared_ptr<Font>& font,
                                                Clr color, Clr border_color, Clr button_color, Clr text_color,
                                                int buttons, const std::string& zero/* = ""*/,
                                                const std::string& one/* = ""*/, const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

void StyleFactory::DeleteWnd(Wnd* wnd) const
{
    delete wnd;
}
