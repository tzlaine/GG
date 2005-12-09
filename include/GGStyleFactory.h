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

/* $Header$ */

/** \file GGStyleFactory.h
    Contains the StyleFactory class, which creates new controls for internal use by dialogs and other controls. */

#ifndef _GGStyleFactory_h_
#define _GGStyleFactory_h_

#ifndef _GGBase_h_
#include  <GGBase.h>
#endif

namespace GG {

class Button;
class ColorDlg;
class DropDownList;
class DynamicGraphic;
class Edit;
class FileDlg;
class Font;
class ListBox;
class MenuBar;
class MultiEdit;
class RadioButtonGroup;
class Scroll;
class Slider;
template <class T>
class Spin;
class StateButton;
class StaticGraphic;
class TextControl;
class Texture;
class ThreeButtonDlg;
class Wnd;

/** Creates new dialogs and Controls.  This class can be used to create a look for the entire GUI by providing
    user-defined subclasses of the standard Controls.  A Control or dialog can then use the StyleFactory to create the
    dialogs/controls it needs (e.g. Scroll can use NewButton() to create its tab and/or resize buttons).  This reduces
    the amount of subclassing that is required to produce a set of custom GG classes. */
class GG_API StyleFactory
{
public:
    /** \name Structors */ //@{
    StyleFactory(); ///< Default ctor.
    virtual ~StyleFactory(); ///< Virtual dtor.
    //@}

    /** \name Controls */ //@{
    /** Returns a new GG Button. */
    virtual Button*            NewButton(int x, int y, int w, int h, const std::string& str,
                                         const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                         Uint32 flags = CLICKABLE) const;

    /** Returns a new GG StateButton. */
    virtual StateButton*       NewStateButton(int x, int y, int w, int h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Uint32 text_fmt, Clr color,
                                              Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                              StateButtonStyle style = SBSTYLE_3D_XBOX, Uint32 flags = CLICKABLE) const;

    /** Returns a new GG RadioButtonGroup. */
    virtual RadioButtonGroup*  NewRadioButtonGroup(int x, int y) const;

    /** Returns a new GG DropDownList. */
    virtual DropDownList*      NewDropDownList(int x, int y, int w, int row_ht, int drop_ht, Clr color,
                                               Uint32 flags = CLICKABLE) const;

    /** Returns a new GG DynamicGraphic. */
    virtual DynamicGraphic*    NewDynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height,
                                                 int margin, const std::vector<boost::shared_ptr<Texture> >& textures,
                                                 Uint32 style = 0, int frames = -1, Uint32 flags = 0) const;

    /** Returns a new GG Edit. */
    virtual Edit*              NewEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<Font>& font,
                                       Clr color, Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                       Uint32 flags = CLICKABLE | DRAG_KEEPER) const;

    /** Returns a new GG ListBox. */
    virtual ListBox*           NewListBox(int x, int y, int w, int h, Clr color, Clr interior = CLR_ZERO,
                                          Uint32 flags = CLICKABLE | DRAG_KEEPER) const;

    /** Returns a new GG MenuBar. */
    virtual MenuBar*           NewMenuBar(int x, int y, int w, const boost::shared_ptr<Font>& font,
                                          Clr text_color = CLR_WHITE, Clr color = CLR_BLACK,
                                          Clr interior = CLR_SHADOW) const;

    /** Returns a new GG MultiEdit. */
    virtual MultiEdit*         NewMultiEdit(int x, int y, int w, int h, const std::string& str,
                                            const boost::shared_ptr<Font>& font, Clr color, Uint32 style = TF_LINEWRAP,
                                            Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO,
                                            Uint32 flags = CLICKABLE | DRAG_KEEPER) const;

    /** Returns a new GG Scroll. */
    virtual Scroll*            NewScroll(int x, int y, int w, int h, Orientation orientation, Clr color, Clr interior,
                                         Uint32 flags = CLICKABLE) const;

    /** Returns a new GG Slider. */
    virtual Slider*            NewSlider(int x, int y, int w, int h, int min, int max, Orientation orientation,
                                         SliderLineStyle style, Clr color, int tab_width, int line_width = 5,
                                         Uint32 flags = CLICKABLE) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<int>*         NewIntSpin(int x, int y, int w, int value, int step, int min, int max, bool edits,
                                          const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                          Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER) const;

    /** Returns a new GG Spin<int>. */
    virtual Spin<double>*      NewDoubleSpin(int x, int y, int w, double value, double step, double min, double max, bool edits,
                                             const boost::shared_ptr<Font>& font, Clr color, Clr text_color = CLR_BLACK,
                                             Clr interior = CLR_ZERO, Uint32 flags = CLICKABLE | DRAG_KEEPER) const;

    /** Returns a new GG StaticGraphic. */
    virtual StaticGraphic*     NewStaticGraphic(int x, int y, int w, int h, const boost::shared_ptr<Texture>& texture,
                                                Uint32 style = 0, Uint32 flags = 0) const;

    /** Returns a new GG TextControl. */
    virtual TextControl*       NewTextControl(int x, int y, int w, int h, const std::string& str,
                                              const boost::shared_ptr<Font>& font, Clr color = CLR_BLACK,
                                              Uint32 text_fmt = 0, Uint32 flags = 0) const;

    /** Returns a new GG TextControl whose size is exactly that required to hold its text. */
    virtual TextControl*       NewTextControl(int x, int y, const std::string& str, const boost::shared_ptr<Font>& font,
                                              Clr color = CLR_BLACK, Uint32 text_fmt = 0, Uint32 flags = 0) const;
    //@}

    /** \name Dialogs */ //@{
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

#endif // _GGStyleFactory_h_
