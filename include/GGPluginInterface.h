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

#ifndef _GGApp_h_
#include  <GGApp.h>
#endif

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

namespace boost { namespace archive {
    class xml_oarchive;
    class xml_iarchive;
} }

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
    (if the plugin author did everything correctly).  The plugin interface also provides Save- and LoadWnd() methods to
    serialize all types in the GG::Wnd hierarchy.  Note that this means all types, not just the ones added by the
    plugin; if Wnd is not serializable, none of its descendents are either. */
class GG_API PluginInterface
{
public:
    typedef const char*         (*PluginNameFn)();

    typedef const char*         (*DefaultFontNameFn)();
    typedef int                 (*DefaultFontSizeFn)();

    typedef Button*             (*CreateButtonFn)(int, int, int, int, const std::string&, const std::string&, int, Clr, Clr, Uint32);
    typedef StateButton*        (*CreateStateButtonFn)(int, int, int, int, const std::string&, const std::string&, int, Uint32, 
                                                       Clr, Clr, Clr, StateButton::StateButtonStyle, int, int, int, int, Uint32);
    typedef RadioButtonGroup*   (*CreateRadioButtonGroupFn)(int, int);
    typedef DropDownList*       (*CreateDropDownListFn)(int, int, int, int, int, Clr, Clr, Uint32);
    typedef DynamicGraphic*     (*CreateDynamicGraphicFn)(int, int, int, int, bool, int, int, int, const boost::shared_ptr<Texture>&, Uint32, int, Uint32);
    typedef Edit*               (*CreateEditFn)(int, int, int, int, const std::string&, const std::string&, int, Clr, Clr, Clr, Uint32);
    typedef ListBox*            (*CreateListBoxFn)(int, int, int, int, Clr, Clr, Uint32);
    typedef MenuBar*            (*CreateMenuBarFn)(int, int, int, const std::string&, int, Clr, Clr, Clr);
    typedef MultiEdit*          (*CreateMultiEditFn)(int, int, int, int, const std::string&, const std::string&, int, Clr, Uint32, Clr, Clr, Uint32);
    typedef Scroll*             (*CreateScrollFn)(int, int, int, int, Scroll::Orientation, Clr, Clr, Uint32);
    typedef Slider*             (*CreateSliderFn)(int, int, int, int, int, int, Slider::Orientation, Slider::LineStyleType, Clr, int, int, Uint32);
    typedef Spin<int>*          (*CreateIntSpinFn)(int, int, int, int, int, int, int, int, bool, const std::string&, int, Clr, Clr, Clr, Uint32);
    typedef Spin<double>*       (*CreateDoubleSpinFn)(int, int, int, int, double, double, double, double, bool, const std::string&, int, Clr, Clr, Clr, Uint32);
    typedef StaticGraphic*      (*CreateStaticGraphicFn)(int, int, int, int, const boost::shared_ptr<Texture>&, Uint32, Uint32);
    typedef TextControl*        (*CreateTextControlFn)(int, int, int, int, const std::string&, const std::string&, int pts, Uint32, Clr, Uint32);

    typedef void                (*DestroyControlFn)(Wnd*);

    typedef App::SaveWndFn      SaveWndFn;
    typedef App::LoadWndFn      LoadWndFn;

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

    CreateButtonFn              CreateButton;           ///< returns a new Button, or an instance of a plugin-specific subclass of Button.
    CreateStateButtonFn         CreateStateButton;      ///< returns a new StateButton, or an instance of a plugin-specific subclass of StateButton.
    CreateRadioButtonGroupFn    CreateRadioButtonGroup; ///< returns a new DropDownList, or an instance of a plugin-specific subclass of DropDownList.
    CreateDropDownListFn        CreateDropDownList;     ///< returns a new DropDownList, or an instance of a plugin-specific subclass of DropDownList.
    CreateDynamicGraphicFn      CreateDynamicGraphic;   ///< returns a new DynamicGraphic, or an instance of a plugin-specific subclass of DynamicGraphic.
    CreateEditFn                CreateEdit;             ///< returns a new Edit, or an instance of a plugin-specific subclass of Edit.
    CreateListBoxFn             CreateListBox;          ///< returns a new ListBox, or an instance of a plugin-specific subclass of ListBox.
    CreateMenuBarFn             CreateMenuBar;          ///< returns a new MenuBar, or an instance of a plugin-specific subclass of MenuBar.
    CreateMultiEditFn           CreateMultiEdit;        ///< returns a new MultiEdit, or an instance of a plugin-specific subclass of MultiEdit.
    CreateScrollFn              CreateScroll;           ///< returns a new Scroll, or an instance of a plugin-specific subclass of Scroll.
    CreateSliderFn              CreateSlider;           ///< returns a new Slider, or an instance of a plugin-specific subclass of Slider.
    CreateIntSpinFn             CreateIntSpin;          ///< returns a new Spin<int>, or an instance of a plugin-specific subclass of Spin<int>.
    CreateDoubleSpinFn          CreateDoubleSpin;       ///< returns a new Spin<double>, or an instance of a plugin-specific subclass of Spin<double>.
    CreateStaticGraphicFn       CreateStaticGraphic;    ///< returns a new StaticGraphic, or an instance of a plugin-specific subclass of StaticGraphic.
    CreateTextControlFn         CreateTextControl;      ///< returns a new TextControl, or an instance of a plugin-specific subclass of TextControl.

    DestroyControlFn            DestroyControl;         ///< destroys a new Control-derived created by the plugin.

    SaveWndFn                   SaveWnd;                ///< serializes a Wnd to the given XML archive.
    LoadWndFn                   LoadWnd;                ///< creates a new Wnd from the next one found in the given XML archive.

    /** Since LoadWnd() will only accept a referemce to a Wnd*, this method is provided to more conveniently accept
        Wnd subclass pointers.  It unfortunately cannot overload the name of LoadWnd, which is a data member. */
    template <class T>
    void LoadWndT(T*& wnd, const std::string& name, boost::archive::xml_iarchive& ar);
    //@}

private:
    lt_dlhandle m_handle;
    boost::archive::xml_oarchive* m_out_archive;
    boost::archive::xml_iarchive* m_in_archive;
};

/** this singleton class is essentially a very thin wrapper around a map of PluginInterface smart pointers, keyed on
    std::string plugin names.  The user need only request a plugin through GetPlugin(); if the plugin is not already
    resident, it will be loaded.*/
class GG_API PluginManager
{
public:
    /** \name Mutators */ //@{
    boost::shared_ptr<PluginInterface> GetPlugin(const std::string& name);  ///< returns a shared_ptr to the plugin interface created from plugin \a name. If the plugin is not present in the manager's pool, it will be loaded from disk.

    /** removes the manager's shared_ptr to the plugin created from file \a name, if it exists.  \note Due to shared_ptr semantics, 
        the plugin may not be deleted until much later. */
    void FreePlugin(const std::string& name);
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
    PluginManager();

    std::map<std::string, boost::shared_ptr<PluginInterface> > m_plugins;

    static bool s_lt_dl_initialized;

    friend PluginManager& GetPluginManager();
};

/** Returns the singleton PluginManager instance. */
PluginManager& GetPluginManager();

// template implementations
template <class T>
void PluginInterface::LoadWndT(T*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    Wnd* wnd_as_base = wnd;
    LoadWnd(wnd_as_base, name, ar);
    wnd = dynamic_cast<T*>(wnd_as_base);
    assert(wnd);
}

} // namespace GG

#endif // _GGPluginInterface_h_
