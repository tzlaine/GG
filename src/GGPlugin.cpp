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

/** \file GGPlugin.cpp
    This is a sample plugin implementation.  It can only create and serialize the default types of GG controls.  Extend
    it to create a plugin suitable for a user-defned GG control a hierarchy. */

#include <GGButton.h>
#include <GGDropDownList.h>
#include <GGDynamicGraphic.h>
#include <GGEdit.h>
#include <GGLayout.h>
#include <GGListBox.h>
#include <GGMenu.h>
#include <GGMultiEdit.h>
#include <GGScroll.h>
#include <GGSlider.h>
#include <GGSpin.h>
#include <GGStaticGraphic.h>
#include <GGTextControl.h>

// TODO : add include files for custom Wnd subclasses to be included in your plugin

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

#ifdef _MSC_VER
# define GG_PLUGIN_API __declspec(dllexport)
#else
# define GG_PLUGIN_API
#endif

// These typedefs exist only to ensure that '<' and '>' do not appear in the XML attribute strings, which chokes the
// boost.serialization XML parser.
typedef GG::Spin<int> Spin_int;
typedef GG::Spin<double> Spin_double;
GG_SHARED_POINTER_EXPORT(GG::Wnd)
GG_SHARED_POINTER_EXPORT(GG::DropDownList)
GG_SHARED_POINTER_EXPORT(GG::DynamicGraphic)
GG_SHARED_POINTER_EXPORT(GG::Edit)
GG_SHARED_POINTER_EXPORT(GG::Font)
GG_SHARED_POINTER_EXPORT(GG::Layout)
GG_SHARED_POINTER_EXPORT(GG::ListBox)
GG_SHARED_POINTER_EXPORT(GG::ListBox::Row)
GG_SHARED_POINTER_EXPORT(GG::MenuBar)
GG_SHARED_POINTER_EXPORT(GG::MultiEdit)
GG_SHARED_POINTER_EXPORT(GG::Scroll)
GG_SHARED_POINTER_EXPORT(GG::Slider)
GG_SHARED_POINTER_EXPORT(Spin_int)
GG_SHARED_POINTER_EXPORT(Spin_double)
GG_SHARED_POINTER_EXPORT(GG::StaticGraphic)
GG_SHARED_POINTER_EXPORT(GG::TextControl)
GG_SHARED_POINTER_EXPORT(GG::Button)
GG_SHARED_POINTER_EXPORT(GG::StateButton)
GG_SHARED_POINTER_EXPORT(GG::RadioButtonGroup)
// TODO : add export declarations for all user-defined Wnd subclasses

using namespace GG;
using namespace boost;
using namespace std;

// TODO: add declarations for custom classes that must be serialized in SaveWnd() and LoadWnd().


namespace {
    // TODO: edit these constants to reflect your plugin's requirements
    const char* PLUGIN_NAME = "GG Basic Controls";
    const char* DEFAULT_FONT_NAME = "Vera.ttf";
    const int DEFAULT_FONT_SIZE = 12;
}

extern "C" {
    // provides the name of the plugin
    GG_PLUGIN_API 
    const char* PluginName()
    {
        return PLUGIN_NAME;
    }

    // provides the name of the default font to be used with this plugin
    GG_PLUGIN_API 
    const char* DefaultFontName()
    {
        return DEFAULT_FONT_NAME;
    }

    // provides the size of the default font to be used with this plugin
    GG_PLUGIN_API 
    int DefaultFontSize()
    {
        return DEFAULT_FONT_SIZE;
    }


    //  Control creation functions
    //  TODO: Override these with calls to ctors for the subclasses that you wish to use in the place of these GG control classes.
    GG_PLUGIN_API
    Button* CreateButton(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, Clr text_color, Uint32 flags)
    {
        return new Button(x, y, w, h, str, font_filename, pts, color, text_color, flags);
    }

    GG_PLUGIN_API
    StateButton* CreateStateButton(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Uint32 text_fmt, 
                                   Clr color, Clr text_color, Clr interior, StateButton::StateButtonStyle style,
                                   int bn_x, int bn_y, int bn_w, int bn_h, Uint32 flags)
    {
        return new StateButton(x, y, w, h, str, font_filename, pts, text_fmt, color, text_color, interior, style, bn_x, bn_y, bn_w, bn_h, flags);
    }

    GG_PLUGIN_API
    RadioButtonGroup* CreateRadioButtonGroup(int x, int y)
    {
        return new RadioButtonGroup(x, y);
    }

    GG_PLUGIN_API
    DropDownList* CreateDropDownList(int x, int y, int w, int row_ht, int drop_ht, Clr color, Clr interior, Uint32 flags)
    {
        return new DropDownList(x, y, w, row_ht, drop_ht, color, interior, 0, flags);
    }

    GG_PLUGIN_API
    DynamicGraphic* CreateDynamicGraphic(int x, int y, int w, int h, bool loop, int frame_width, int frame_height, int margin, 
                                         const boost::shared_ptr<Texture>& texture, Uint32 style, int frames, Uint32 flags)
    {
        return new DynamicGraphic(x, y, w, h, loop, frame_width, frame_height, margin, texture, style, frames, flags);
    }

    GG_PLUGIN_API
    Edit* CreateEdit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, Clr text_color, 
                     Clr interior, Uint32 flags)
    {
        return new Edit(x, y, w, h, str, font_filename, pts, color, text_color, interior, flags);
    }

    GG_PLUGIN_API
    ListBox* CreateListBox(int x, int y, int w, int h, Clr color, Clr interior, Uint32 flags)
    {
        return new ListBox(x, y, w, h, color, interior, flags);
    }

    GG_PLUGIN_API
    MenuBar* CreateMenuBar(int x, int y, int w, const string& font_filename, int pts, Clr text_color, Clr color, Clr interior)
    {
        return new MenuBar(x, y, w, font_filename, pts, text_color, color, interior);
    }

    GG_PLUGIN_API
    MultiEdit* CreateMultiEdit(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, 
                               Uint32 style, Clr text_color, Clr interior, Uint32 flags)
    {
        return new MultiEdit(x, y, w, h, str, font_filename, pts, color, style, text_color, interior, flags);
    }

    GG_PLUGIN_API
        Scroll* CreateScroll(int x, int y, int w, int h, Scroll::Orientation orientation, Clr color, Clr interior, Uint32 flags)
    {
        return new Scroll(x, y, w, h, orientation, color, interior, 0, 0, 0, flags);
    }

    GG_PLUGIN_API
    Slider* CreateSlider(int x, int y, int w, int h, int min, int max, Slider::Orientation orientation, Slider::LineStyleType style, Clr color, int tab_width, int line_width, Uint32 flags)
    {
        return new Slider(x, y, w, h, min, max, orientation, style, color, tab_width, line_width, flags);
    }

    GG_PLUGIN_API
    Spin<int>* CreateIntSpin(int x, int y, int w, int h, int value, int step, int min, int max, bool edits, const string& font_filename, int pts, Clr color, 
                             Clr text_color, Clr interior, Uint32 flags)
    {
        return new Spin<int>(x, y, w, h, value, step, min, max, edits, font_filename, pts, color, text_color, interior, 0, 0, flags);
    }

    GG_PLUGIN_API
    Spin<double>* CreateDoubleSpin(int x, int y, int w, int h, double value, double step, double min, double max, bool edits, const string& font_filename, int pts, Clr color, 
                                   Clr text_color, Clr interior, Uint32 flags)
    {
        return new Spin<double>(x, y, w, h, value, step, min, max, edits, font_filename, pts, color, text_color, interior, 0, 0, flags);
    }

    GG_PLUGIN_API
    StaticGraphic* CreateStaticGraphic(int x, int y, int w, int h, const shared_ptr<Texture>& texture, Uint32 style, Uint32 flags)
    {
        return new StaticGraphic(x, y, w, h, texture, style, flags);
    }

    GG_PLUGIN_API
    TextControl* CreateTextControl(int x, int y, int w, int h, const string& str, const string& font_filename, int pts, Clr color, Uint32 text_fmt, Uint32 flags)
    {
        return new TextControl(x, y, w, h, str, font_filename, pts, color, text_fmt, flags);
    }


    // Destruction function
    GG_PLUGIN_API
    void DestroyControl(Wnd* w) {delete w;}


    // Serialization functions
    GG_PLUGIN_API
    void SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
    {
        ar & boost::serialization::make_nvp(name.c_str(), wnd);
    }

    GG_PLUGIN_API
    void LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
    {
        ar & boost::serialization::make_nvp(name.c_str(), wnd);
    }
}
