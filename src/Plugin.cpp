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

/** \file GGPlugin.cpp
    This is a sample plugin implementation.  It can only create and serialize the default types of GG controls.  Extend
    it to create a plugin suitable for a user-defned GG control a hierarchy. */

#include <GG/Button.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/Layout.h>
#include <GG/ListBox.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>

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
    { return PLUGIN_NAME; }

    // provides the name of the default font to be used with this plugin
    GG_PLUGIN_API 
    const char* DefaultFontName()
    { return DEFAULT_FONT_NAME; }

    // provides the size of the default font to be used with this plugin
    GG_PLUGIN_API 
    int DefaultFontSize()
    { return DEFAULT_FONT_SIZE; }


    //  TODO: Override this with your own StyleFactory subclass.
    GG_PLUGIN_API
    boost::shared_ptr<GG::StyleFactory> GetStyleFactory()
    {
        static boost::shared_ptr<GG::StyleFactory> style_factory(new GG::StyleFactory());
        return style_factory;
    }

    // Serialization functions
    GG_PLUGIN_API
    void SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
    { ar & boost::serialization::make_nvp(name.c_str(), wnd); }

    GG_PLUGIN_API
    void LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
    { ar & boost::serialization::make_nvp(name.c_str(), wnd); }
}
