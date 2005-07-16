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

/** \file GGPluginInterface.h
    Contains the PluginInterface class, an interface to custom-control plugins that allow runtime control selection. */

#ifndef _GGPluginInterface_h_
#define _GGPluginInterface_h_

#ifndef _GGBase_h_
#include  <GGBase.h>
#endif

#ifndef LTDL_H
#include <ltdl.h>
#endif

#ifndef _GGButton_h_
#include <GGButton.h>
#endif

#ifndef _GGScroll_h_
#include <GGScroll.h>
#endif

#ifndef _GGSlider_h_
#include <GGSlider.h>
#endif

#ifndef _GGSpin_h_
#include <GGSpin.h>
#endif

#include <string>

namespace GG {

class DropDownList;
class DynamicGraphic;
class Edit;
class ListBox;
class MenuBar;
class MultiEdit;
class StaticGraphic;
class TextControl;

/** the interface to custom-control plugins.  This class is used to access derived GG controls that are unknown until
    runtime, but are available for dynamic loading in shared libraries/DLLs.  The interface basically allows you to
    create custom controls (GG::Button subclasses, GG::ListBox subclasses, etc.) from one or more dynamic libraries,
    which in turn allows you to change the styles of the controls in an application without recompiling, or even
    relinking.  While the interface is in an unloaded state, the functions in the interface are all null, and calling
    any of them will crash your app.  Once a plugin has been loaded, all the functions in the interface should be valid
    (if the plugin author did her job). */
class GG_API PluginInterface
{
public:
    typedef const char*             (*PluginNameFn)();

    typedef const char*             (*DefaultFontNameFn)();
    typedef int                     (*DefaultFontSizeFn)();

    typedef GG::Button*             (*CreateButtonFn)(int, int, int, int, const std::string&, const std::string&, int, GG::Clr, GG::Clr, Uint32);
    typedef GG::StateButton*        (*CreateStateButtonFn)(int, int, int, int, const std::string&, const std::string&, int, Uint32, 
                                                           GG::Clr, GG::Clr, GG::Clr, GG::StateButton::StateButtonStyle, int, int, int, int, Uint32);
    typedef GG::RadioButtonGroup*   (*CreateRadioButtonGroupFn)(int, int);
    typedef GG::DropDownList*       (*CreateDropDownListFn)(int, int, int, int, int, GG::Clr, GG::Clr, Uint32);
    typedef GG::DynamicGraphic*     (*CreateDynamicGraphicFn)(int, int, int, int, bool, int, int, int, const boost::shared_ptr<GG::Texture>&, Uint32, int, Uint32);
    typedef GG::Edit*               (*CreateEditFn)(int, int, int, int, const std::string&, const std::string&, int, GG::Clr, GG::Clr, GG::Clr, Uint32);
    typedef GG::ListBox*            (*CreateListBoxFn)(int, int, int, int, GG::Clr, GG::Clr, Uint32);
    typedef GG::MenuBar*            (*CreateMenuBarFn)(int, int, int, const std::string&, int, GG::Clr, GG::Clr, GG::Clr);
    typedef GG::MultiEdit*          (*CreateMultiEditFn)(int, int, int, int, const std::string&, const std::string&, int, GG::Clr, Uint32, GG::Clr, GG::Clr, Uint32);
    typedef GG::Scroll*             (*CreateScrollFn)(int, int, int, int, GG::Scroll::Orientation, GG::Clr, GG::Clr, Uint32);
    typedef GG::Slider*             (*CreateSliderFn)(int, int, int, int, int, int, GG::Slider::Orientation, GG::Slider::LineStyleType, GG::Clr, int, int, Uint32);
    typedef GG::Spin<int>*          (*CreateIntSpinFn)(int, int, int, int, int, int, int, int, bool, const std::string&, int, GG::Clr, GG::Clr, GG::Clr, Uint32);
    typedef GG::Spin<double>*       (*CreateDoubleSpinFn)(int, int, int, int, double, double, double, double, bool, const std::string&, int, GG::Clr, GG::Clr, GG::Clr, Uint32);
    typedef GG::StaticGraphic*      (*CreateStaticGraphicFn)(int, int, int, int, const boost::shared_ptr<GG::Texture>&, Uint32, Uint32);
    typedef GG::TextControl*        (*CreateTextControlFn)(int, int, int, int, const std::string&, const std::string&, int pts, Uint32, GG::Clr, Uint32);

    typedef void                    (*DestroyControlFn)(GG::Wnd*);

    typedef const std::map<std::string, std::string>& 
                                    (*FactoryGeneratorNamesFn)();

    /** \name Structors */ //@{
    PluginInterface(); ///< default ctor.

    /** ctor that loads the plugin file \a lib_name.  The base filename should be provided, without the extension (i.e. "foo", 
        not "foo.so" or "foo.dll"). */
    PluginInterface(const std::string& lib_name);

    ~PluginInterface(); ///< dtor.
    //@}

    /** \name Accessors */ //@{
    /** returns true iff this PluginInterface has a loaded plugin.  This is a conversion operator, allowing you to test the validity of 
        the interface, as you would a pointer (e.g. if (my_interface) my_interface->PluginName();).  \warning If this method returns false, 
        the functions in the interface are invalid. */
    operator bool() const;
    //@}

    /** \name Mutators */ //@{
    /** loads the plugin \a lib_name, unloading the currently-loaded plugin if necessary. */
    bool Load(const std::string& lib_name);

    PluginNameFn                PluginName;             ///< returns the name of this plugin

    DefaultFontNameFn           DefaultFontName;        ///< returns the default font name that should be used to create controls using this plugin.
    DefaultFontSizeFn           DefaultFontSize;        ///< returns the default font point size that should be used to create controls using this plugin.

    CreateButtonFn              CreateButton;           ///< returns a new GG::Button, or an instance of a plugin-specific subclass of GG::Button.
    CreateStateButtonFn         CreateStateButton;      ///< returns a new GG::StateButton, or an instance of a plugin-specific subclass of GG::StateButton.
    CreateRadioButtonGroupFn    CreateRadioButtonGroup; ///< returns a new GG::DropDownList, or an instance of a plugin-specific subclass of GG::DropDownList.
    CreateDropDownListFn        CreateDropDownList;     ///< returns a new GG::DropDownList, or an instance of a plugin-specific subclass of GG::DropDownList.
    CreateDynamicGraphicFn      CreateDynamicGraphic;   ///< returns a new GG::DynamicGraphic, or an instance of a plugin-specific subclass of GG::DynamicGraphic.
    CreateEditFn                CreateEdit;             ///< returns a new GG::Edit, or an instance of a plugin-specific subclass of GG::Edit.
    CreateListBoxFn             CreateListBox;          ///< returns a new GG::ListBox, or an instance of a plugin-specific subclass of GG::ListBox.
    CreateMenuBarFn             CreateMenuBar;          ///< returns a new GG::MenuBar, or an instance of a plugin-specific subclass of GG::MenuBar.
    CreateMultiEditFn           CreateMultiEdit;        ///< returns a new GG::MultiEdit, or an instance of a plugin-specific subclass of GG::MultiEdit.
    CreateScrollFn              CreateScroll;           ///< returns a new GG::Scroll, or an instance of a plugin-specific subclass of GG::Scroll.
    CreateSliderFn              CreateSlider;           ///< returns a new GG::Slider, or an instance of a plugin-specific subclass of GG::Slider.
    CreateIntSpinFn             CreateIntSpin;          ///< returns a new GG::Spin<int>, or an instance of a plugin-specific subclass of GG::Spin<int>.
    CreateDoubleSpinFn          CreateDoubleSpin;       ///< returns a new GG::Spin<double>, or an instance of a plugin-specific subclass of GG::Spin<double>.
    CreateStaticGraphicFn       CreateStaticGraphic;    ///< returns a new GG::StaticGraphic, or an instance of a plugin-specific subclass of GG::StaticGraphic.
    CreateTextControlFn         CreateTextControl;      ///< returns a new GG::TextControl, or an instance of a plugin-specific subclass of GG::TextControl.

    DestroyControlFn            DestroyControl;         ///< destroys a new GG::Control-derived created by the plugin.
    //@}

private:
    lt_dlhandle m_handle;
};

/** this singleton class is essentially a very thin wrapper around a map of PluginInterface smart pointers, keyed on
    std::string plugin names.  The user need only request a plugin through GetPlugin(); if the plugin is not already
    resident, it will be loaded.*/
class GG_API PluginManager
{
public:
    /** \name Structors */ //@{
    PluginManager(); ///< ctor
    //@}

    /** \name Mutators */ //@{
    boost::shared_ptr<PluginInterface> GetPlugin(const std::string& name);  ///< returns a shared_ptr to the plugin interface created from plugin \a name. If the plugin is not present in the manager's pool, it will be loaded from disk.

    /** removes the manager's shared_ptr to the plugin created from file \a name, if it exists.  \note Due to shared_ptr semantics, 
        the plugin may not be deleted until much later. */
    void                FreePlugin(const std::string& name);
    //@}

    /** initializes the dynamic loader system that loads and unloads plugins.  This is available as a convenience only; it will be called 
        automatically as needed. */
    static void InitDynamicLoader();

    /** adds a directory which should be searched for plugins. */
    static void AddSearchDirectory(const std::string& dir);

    /** cleans up the dynamic loader system that loads and unloads plugins.  This should be called manually when desiredl it will never be called 
        by other PluginInterface code. */
    static void CleanupDynamicLoader();

private:
    std::map<std::string, boost::shared_ptr<PluginInterface> > m_plugins;

    static bool s_created;
    static bool s_lt_dl_initialized;
};

} // namespace GG

#endif // _GGPluginInterface_h_
