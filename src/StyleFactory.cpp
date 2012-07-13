/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   whatwasthataddress@gmail.com */

#include <GG/StyleFactory.h>

#include <GG/BrowseInfoWnd.h>
#include <GG/Button.h>
#include <GG/Cursor.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/GroupBox.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/ProgressBar.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/TabWnd.h>
#include <GG/TextControl.h>
#include <GG/adobe/name.hpp>
#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>


#include "DefaultFont.h"
#include "DefaultCursors.h"


using namespace GG;

namespace {

    boost::shared_ptr<Texture> CursorTexture()
    { return GUI::GetGUI()->GetTexture(CursorsFilename()); }

    boost::shared_ptr<Cursor> GetCursor(unsigned int row,
                                        unsigned int column,
                                        unsigned int hotspot_x,
                                        unsigned int hotspot_y)
    {
        boost::shared_ptr<Texture> texture = CursorTexture();
        const unsigned int cursor_size = 32u;
        return boost::shared_ptr<Cursor>(
            new TextureCursor(
                SubTexture(texture,
                           X(column * cursor_size),
                           Y(row * cursor_size),
                           X((column + 1) * cursor_size),
                           Y((row + 1) * cursor_size)),
                Pt(X(hotspot_x), Y(hotspot_y))
            )
        );
    }

}

adobe::aggregate_name_t GG::POINTER_CURSOR = { "pointer" };
adobe::aggregate_name_t GG::HELP_CURSOR = { "help" };
adobe::aggregate_name_t GG::CROSSHAIR_CURSOR = { "crosshair" };
adobe::aggregate_name_t GG::MOVE_CURSOR = { "move" };
adobe::aggregate_name_t GG::LINK_CURSOR = { "link" };
adobe::aggregate_name_t GG::GRABABLE_CURSOR = { "grabable" };
adobe::aggregate_name_t GG::GRABBING_CURSOR = { "grabbing" };
adobe::aggregate_name_t GG::TEXT_CURSOR = { "text" };
adobe::aggregate_name_t GG::RESIZE_LEFT_RIGHT_CURSOR = { "resize_left_right" };
adobe::aggregate_name_t GG::RESIZE_UP_DOWN_CURSOR = { "resize_up_down" };
adobe::aggregate_name_t GG::RESIZE_UL_LR_CURSOR = { "resize_ul_lr" };
adobe::aggregate_name_t GG::RESIZE_LL_UR_CURSOR = { "resize_ll_ur" };
adobe::aggregate_name_t GG::ZOOM_IN_CURSOR = { "zoom_in" };
adobe::aggregate_name_t GG::ZOOM_OUT_CURSOR = { "zoom_out" };
adobe::aggregate_name_t GG::DROP_CURSOR = { "drop" };
adobe::aggregate_name_t GG::DISALLOW_CURSOR = { "disallow" };

StyleFactory::~StyleFactory()
{}

boost::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts/* = 12*/) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<unsigned char>());
    } else {
        std::vector<unsigned char> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes);
    }
}

boost::shared_ptr<Font> StyleFactory::DefaultFont(unsigned int pts,
                                                  const UnicodeCharset* first,
                                                  const UnicodeCharset* last) const
{
    if (GetFontManager().HasFont(DefaultFontName(), pts, first, last)) {
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, std::vector<unsigned char>(), first, last);
    } else {
        std::vector<unsigned char> bytes;
        VeraTTFBytes(bytes);
        return GUI::GetGUI()->GetFont(DefaultFontName(), pts, bytes, first, last);
    }
}

const std::string& StyleFactory::DefaultFontName() const
{
    static std::string retval = DEFAULT_FONT_NAME;
    return retval;
}

const boost::shared_ptr<BrowseInfoWnd>& StyleFactory::DefaultBrowseInfoWnd() const
{
    if (!m_browse_info_wnd)
        m_browse_info_wnd.reset(new TextBoxBrowseInfoWnd(X1, DefaultFont(), CLR_GRAY, CLR_BLACK, CLR_BLACK));
    return m_browse_info_wnd;
}

boost::shared_ptr<Cursor> StyleFactory::GetCursor(adobe::name_t name) const
{
    if (m_cursors.empty()) {
        m_cursors[POINTER_CURSOR] = ::GetCursor(0, 0, 3, 4);
        m_cursors[HELP_CURSOR] = ::GetCursor(0, 1, 3, 4);
        m_cursors[CROSSHAIR_CURSOR] = ::GetCursor(0, 2, 15, 15);
        m_cursors[MOVE_CURSOR] = ::GetCursor(0, 3, 15, 15);
        m_cursors[LINK_CURSOR] = ::GetCursor(1, 0, 3, 11);
        m_cursors[GRABABLE_CURSOR] = ::GetCursor(1, 1, 15, 12);
        m_cursors[GRABBING_CURSOR] = ::GetCursor(1, 2, 14, 12);
        m_cursors[TEXT_CURSOR] = ::GetCursor(1, 3, 12, 14);
        m_cursors[RESIZE_LEFT_RIGHT_CURSOR] = ::GetCursor(2, 0, 16, 16);
        m_cursors[RESIZE_UP_DOWN_CURSOR] = ::GetCursor(2, 1, 16, 17);
        m_cursors[RESIZE_UL_LR_CURSOR] = ::GetCursor(2, 2, 16, 16);
        m_cursors[RESIZE_LL_UR_CURSOR] = ::GetCursor(2, 3, 16, 17);
        m_cursors[ZOOM_IN_CURSOR] = ::GetCursor(3, 0, 14, 15);
        m_cursors[ZOOM_OUT_CURSOR] = ::GetCursor(3, 1, 14, 15);
        m_cursors[DROP_CURSOR] = ::GetCursor(3, 2, 15, 15);
        m_cursors[DISALLOW_CURSOR] = ::GetCursor(3, 3, 16, 16);
    }
    CursorMap::const_iterator it = m_cursors.find(name);
    return it == m_cursors.end() ? boost::shared_ptr<Cursor>() : it->second;
}

boost::shared_ptr<PopupMenu> StyleFactory::NewPopupMenu(X x, Y y,
                                                        const boost::shared_ptr<Font>& font,
                                                        const MenuItem& m,
                                                        Clr text_color/* = CLR_WHITE*/,
                                                        Clr color/* = CLR_BLACK*/,
                                                        Clr interior/* = CLR_SHADOW*/) const
{ return boost::shared_ptr<PopupMenu>(new PopupMenu(x, y, font, m, text_color, color, interior)); }

Button* StyleFactory::NewButton(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Button(x, y, w, h, str, font, color, text_color, flags); }

StateButton* StyleFactory::NewStateButton(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                                          Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                          StateButtonStyle style/* = SBSTYLE_3D_XBOX*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new StateButton(x, y, w, h, str, font, color, text_color, interior, style, flags); }

RadioButtonGroup* StyleFactory::NewRadioButtonGroup(X x, Y y, X w, Y h, Orientation orientation) const
{ return new RadioButtonGroup(x, y, w, h, orientation); }

DropDownList* StyleFactory::NewDropDownList(X x, Y y, X w, Y h, Y drop_ht, Clr color,
                                            Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new DropDownList(x, y, w, h, drop_ht, color, flags); }

DynamicGraphic* StyleFactory::NewDynamicGraphic(X x, Y y, X w, Y h, bool loop, X frame_width, Y frame_height,
                                                int margin, const std::vector<boost::shared_ptr<Texture> >& textures,
                                                Flags<GraphicStyle> style/* = GRAPHIC_NONE*/,
                                                int frames/* = DynamicGraphic::ALL_FRAMES*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new DynamicGraphic(x, y, w, h, loop, frame_width, frame_height, margin, textures, style, frames, flags); }

Edit* StyleFactory::NewEdit(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font,
                            Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                            Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Edit(x, y, w, str, font, color, text_color, interior, flags); }

ListBox* StyleFactory::NewListBox(X x, Y y, X w, Y h, Clr color, Clr interior/* = CLR_ZERO*/,
                                  Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new ListBox(x, y, w, h, color, interior, flags); }

MenuBar* StyleFactory::NewMenuBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr text_color/* = CLR_WHITE*/,
                                  Clr color/* = CLR_BLACK*/, Clr interior/* = CLR_SHADOW*/) const
{ return new MenuBar(x, y, w, font, text_color, color, interior); }

MultiEdit* StyleFactory::NewMultiEdit(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                                      Clr color, Flags<MultiEditStyle> style/* = MULTI_LINEWRAP*/, Clr text_color/* = CLR_BLACK*/,
                                      Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new MultiEdit(x, y, w, h, str, font, color, style, text_color, interior, flags); }

Scroll* StyleFactory::NewScroll(X x, Y y, X w, Y h, Orientation orientation, Clr color, Clr interior,
                                Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return new Scroll(x, y, w, h, orientation, color, interior, flags); }

Slider<int>* StyleFactory::NewIntSlider(X x, Y y, X w, Y h, int min, int max, Orientation orientation,
                                        SliderLineStyle style, Clr color, int tab_width, int line_width/* = 5*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Slider<int>(x, y, w, h, min, max, orientation, style, color, tab_width, line_width, flags); }

Slider<double>* StyleFactory::NewDoubleSlider(X x, Y y, X w, Y h, double min, double max, Orientation orientation,
                                             SliderLineStyle style, Clr color, int tab_width, int line_width/* = 5*/,
                                            Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Slider<double>(x, y, w, h, min, max, orientation, style, color, tab_width, line_width, flags); }

Spin<int>* StyleFactory::NewIntSpin(X x, Y y, X w, int value, int step, int min, int max, bool edits,
                                    const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                    Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Spin<int>(x, y, w, value, step, min, max, edits, font, color, text_color, interior, flags); }

Spin<double>* StyleFactory::NewDoubleSpin(X x, Y y, X w, double value, double step, double min, double max, bool edits,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Clr interior/* = CLR_ZERO*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new Spin<double>(x, y, w, value, step, min, max, edits, font, color, text_color, interior, flags); }

StaticGraphic* StyleFactory::NewStaticGraphic(X x, Y y, X w, Y h, const boost::shared_ptr<Texture>& texture,
                                              Flags<GraphicStyle> style/* = GRAPHIC_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new StaticGraphic(x, y, w, h, texture, style, flags); }

StaticGraphic* StyleFactory::NewStaticGraphic(X x, Y y, X w, Y h, const SubTexture& subtexture,
                                              Flags<GraphicStyle> style/* = GRAPHIC_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new StaticGraphic(x, y, w, h, subtexture, style, flags); }

TextControl* StyleFactory::NewTextControl(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color/* = CLR_BLACK*/,
                                          Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new TextControl(x, y, w, h, str, font, color, format, flags); }

TextControl* StyleFactory::NewTextControl(X x, Y y, const std::string& str, const boost::shared_ptr<Font>& font,
                                          Clr color/* = CLR_BLACK*/, Flags<TextFormat> format/* = FORMAT_NONE*/, Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new TextControl(x, y, str, font, color, format, flags); }

GroupBox* StyleFactory::NewGroupBox(X x, Y y, X w, Y h, const std::string& label, const boost::shared_ptr<Font>& font,
                                    Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                    Flags<WndFlag> flags/* = Flags<WndFlag>()*/) const
{ return new GroupBox(x, y, w, h, label, font, color, text_color, interior, flags); }

ProgressBar* StyleFactory::NewProgressBar(X x, Y y, X w, Y h, Orientation orientation,
                                          unsigned int bar_width, Clr color,
                                          Clr bar_color/* = CLR_BLUE*/, Clr interior_color/* = CLR_ZERO*/) const
{ return new ProgressBar(x, y, w, h, orientation, bar_width, color, bar_color, interior_color); }

TabBar* StyleFactory::NewTabBar(X x, Y y, X w, const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                TabBarStyle style/* = TAB_BAR_ATTACHED*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return new TabBar(x, y, w, font, color, text_color, style, flags); }

ListBox* StyleFactory::NewDropDownListListBox(X x, Y y, X w, Y h, Clr color, Clr interior/* = CLR_ZERO*/,
                                              Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewListBox(x, y, w, h, color, interior, flags); }

Scroll* StyleFactory::NewListBoxVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, VERTICAL, color, interior, flags); }

Scroll* StyleFactory::NewListBoxHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags); }

Scroll* StyleFactory::NewMultiEditVScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, VERTICAL, color, interior, flags); }

Scroll* StyleFactory::NewMultiEditHScroll(X x, Y y, X w, Y h, Clr color, Clr interior,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewScroll(x, y, w, h, HORIZONTAL, color, interior, flags); }

Button* StyleFactory::NewScrollUpButton(X x, Y y, X w, Y h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollDownButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewVScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewScrollRightButton(X x, Y y, X w, Y h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewHScrollTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewVSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewHSliderTabButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewSpinIncrButton(X x, Y y, X w, Y h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewSpinDecrButton(X x, Y y, X w, Y h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                        Flags<WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Edit* StyleFactory::NewSpinEdit(X x, Y y, X w, const std::string& str, const boost::shared_ptr<Font>& font,
                                Clr color, Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewEdit(x, y, w, str, font, color, text_color, interior, flags); }

StateButton* StyleFactory::NewTabBarTab(X x, Y y, X w, Y h, const std::string& str,
                                        const boost::shared_ptr<Font>& font, Clr color,
                                        Clr text_color/* = CLR_BLACK*/, Clr interior/* = CLR_ZERO*/,
                                        StateButtonStyle style/* = SBSTYLE_3D_TOP_ATTACHED_TAB*/, Flags<WndFlag> flags/* = INTERACTIVE*/) const
{
    StateButton* retval = NewStateButton(x, y, w, h, str, font, color, text_color, interior, style, flags);
    retval->Resize(retval->MinUsableSize() + Pt(X(12), Y0));
    return retval;
}

Button* StyleFactory::NewTabBarLeftButton(X x, Y y, X w, Y h, const std::string& str,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                          Flags<WndFlag> flags/* = INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

Button* StyleFactory::NewTabBarRightButton(X x, Y y, X w, Y h, const std::string& str,
                                           const boost::shared_ptr<Font>& font, Clr color, Clr text_color/* = CLR_BLACK*/,
                                           Flags<WndFlag> flags /*= INTERACTIVE*/) const
{ return NewButton(x, y, w, h, str, font, color, text_color, flags); }

TabWnd* StyleFactory::NewTabWnd(X x, Y y, X w, Y h, const boost::shared_ptr<Font>& font, Clr color,
                                Clr text_color/* = CLR_BLACK*/, TabBarStyle style/* = TAB_BAR_ATTACHED*/,
                                Flags<WndFlag> flags/* = INTERACTIVE | DRAGABLE*/) const
{ return new TabWnd(x, y, w, h, font, color, text_color, style, flags); }

ColorDlg* StyleFactory::NewColorDlg(X x, Y y, const boost::shared_ptr<Font>& font,
                                    Clr dialog_color, Clr border_color, Clr text_color/* = CLR_BLACK*/) const
{ return new ColorDlg(x, y, font, dialog_color, border_color, text_color); }


ColorDlg* StyleFactory::NewColorDlg(X x, Y y, Clr original_color, const boost::shared_ptr<Font>& font,
                                    Clr dialog_color, Clr border_color, Clr text_color/* = CLR_BLACK*/) const
{ return new ColorDlg(x, y, original_color, font, dialog_color, border_color, text_color); }


FileDlg* StyleFactory::NewFileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                                  const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                  Clr text_color/* = CLR_BLACK*/) const
{ return new FileDlg(directory, filename, save, multi, font, color, border_color, text_color); }


ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(X x, Y y, X w, Y h, const std::string& msg,
                                                const boost::shared_ptr<Font>& font, Clr color, Clr border_color,
                                                Clr button_color, Clr text_color, int buttons,
                                                const std::string& zero/* = ""*/, const std::string& one/* = ""*/,
                                                const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(x, y, w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

ThreeButtonDlg* StyleFactory::NewThreeButtonDlg(X w, Y h, const std::string& msg, const boost::shared_ptr<Font>& font,
                                                Clr color, Clr border_color, Clr button_color, Clr text_color,
                                                int buttons, const std::string& zero/* = ""*/,
                                                const std::string& one/* = ""*/, const std::string& two/* = ""*/) const
{
    return new ThreeButtonDlg(w, h, msg, font, color, border_color, button_color, text_color,
                              buttons, zero, one, two);
}

void StyleFactory::DeleteWnd(Wnd* wnd) const
{ delete wnd; }
