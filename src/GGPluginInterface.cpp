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

#include "GGPluginInterface.h"

#include <GGDropDownList.h>
#include <GGDynamicGraphic.h>
#include <GGEdit.h>
#include <GGListBox.h>
#include <GGMenu.h>
#include <GGMultiEdit.h>
#include <GGStaticGraphic.h>
#include <GGTextControl.h>

using namespace GG;

///////////////////////////////////////
// class GG::PluginInterface
///////////////////////////////////////
PluginInterface::PluginInterface() : 
    m_handle(0) 
{
}

PluginInterface::PluginInterface(const std::string& lib_name) : 
    m_handle(0) 
{
    Load(lib_name);
}

PluginInterface::~PluginInterface()
{
    if (m_handle) {
        GG::App* app = GG::App::GetApp();
        const std::map<std::string, std::string>& generator_names = FactoryGeneratorNames();
        for (std::map<std::string, std::string>::const_iterator it = generator_names.begin(); it != generator_names.end(); ++it) {
            app->RemoveWndGenerator(it->first);
        }

        // plugin name
        PluginName = 0;

        // plugin defaults
        DefaultFontName = 0;
        DefaultFontSize = 0;

        // direct object creation functions, and direct object destruction function
        CreateButton = 0;
        CreateStateButton = 0;
        CreateRadioButtonGroup = 0;
        CreateDropDownList = 0;
        CreateDynamicGraphic = 0;
        CreateEdit = 0;
        CreateListBox = 0;
        CreateMenuBar = 0;
        CreateMultiEdit = 0;
        CreateScroll = 0;
        CreateSlider = 0;
        CreateIntSpin = 0;
        CreateDoubleSpin = 0;
        CreateStaticGraphic = 0;
        CreateTextControl = 0;
        DestroyControl = 0;

        FactoryGeneratorNames = 0;

        lt_dlclose(m_handle);
        m_handle = 0;
    }
}

PluginInterface::operator bool() const
{
    return m_handle;
}

bool PluginInterface::Load(const std::string& lib_name)
{
    PluginManager::InitDynamicLoader();

    bool retval = true;

    int err = 0;
    if (m_handle) {
        if ((err = lt_dlclose(m_handle))) {
            retval = false;
            GG::App::GetApp()->Logger().errorStream() 
                << "PluginInterface::Load : lt_dlclose() call failed; load of new dynamic library aborted (error #" 
                << err << ": " << lt_dlerror() << ").";
        }
    }

    if (!err) {
        m_handle = lt_dlopenext(lib_name.c_str());
        if (m_handle) {
            // plugin name
            PluginName = (PluginNameFn)(lt_dlsym(m_handle, "PluginName"));

            // plugin defaults
            DefaultFontName = (DefaultFontNameFn)(lt_dlsym(m_handle, "DefaultFontName"));
            DefaultFontSize = (DefaultFontSizeFn)(lt_dlsym(m_handle, "DefaultFontSize"));

            // direct object creation functions, and direct object destruction function
            CreateButton = (CreateButtonFn)(lt_dlsym(m_handle, "CreateButton"));
            CreateStateButton = (CreateStateButtonFn)(lt_dlsym(m_handle, "CreateStateButton"));
            CreateRadioButtonGroup = (CreateRadioButtonGroupFn)(lt_dlsym(m_handle, "CreateRadioButtonGroup"));
            CreateDropDownList = (CreateDropDownListFn)(lt_dlsym(m_handle, "CreateDropDownList"));
            CreateDynamicGraphic = (CreateDynamicGraphicFn)(lt_dlsym(m_handle, "CreateDynamicGraphic"));
            CreateEdit = (CreateEditFn)(lt_dlsym(m_handle, "CreateEdit"));
            CreateListBox = (CreateListBoxFn)(lt_dlsym(m_handle, "CreateListBox"));
            CreateMenuBar = (CreateMenuBarFn)(lt_dlsym(m_handle, "CreateMenuBar"));
            CreateMultiEdit = (CreateMultiEditFn)(lt_dlsym(m_handle, "CreateMultiEdit"));
            CreateScroll = (CreateScrollFn)(lt_dlsym(m_handle, "CreateScroll"));
            CreateSlider = (CreateSliderFn)(lt_dlsym(m_handle, "CreateSlider"));
            CreateIntSpin = (CreateIntSpinFn)(lt_dlsym(m_handle, "CreateIntSpin"));
            CreateDoubleSpin = (CreateDoubleSpinFn)(lt_dlsym(m_handle, "CreateDoubleSpin"));
            CreateStaticGraphic = (CreateStaticGraphicFn)(lt_dlsym(m_handle, "CreateStaticGraphic"));
            CreateTextControl = (CreateTextControlFn)(lt_dlsym(m_handle, "CreateTextControl"));
            DestroyControl = (DestroyControlFn)(lt_dlsym(m_handle, "DestroyControl"));

            // get the names of the XML factory generator functions required to create GG::Wnd-derived classes included in the plugin from XML descriptions
            FactoryGeneratorNames = (FactoryGeneratorNamesFn)(lt_dlsym(m_handle, "FactoryGeneratorNames"));
            typedef GG::XMLObjectFactory<GG::Wnd>::Generator GeneratorFn;
            GG::App* app = GG::App::GetApp();
            const std::map<std::string, std::string>& generator_names = FactoryGeneratorNames();
            for (std::map<std::string, std::string>::const_iterator it = generator_names.begin(); it != generator_names.end(); ++it) {
                app->AddWndGenerator(it->first, (GeneratorFn)(lt_dlsym(m_handle, it->second.c_str())));
            }
        } else {
            retval = false;
            GG::App::GetApp()->Logger().errorStream() 
                << "PluginInterface::Load : Failed to load dynamic library \"" << lib_name << "\" (error was: " << lt_dlerror() << ").";
        }
    }
    return retval;
}

///////////////////////////////////////
// class GG::PluginManager
///////////////////////////////////////
// static member(s)
bool PluginManager::s_created = false;
bool PluginManager::s_lt_dl_initialized = false;

PluginManager::PluginManager()
{
    if (s_created)
        throw std::runtime_error("Attempted to create a second instance of GG::PluginManager");
    s_created = true;
}

shared_ptr<PluginInterface> PluginManager::GetPlugin(const string& name)
{
    std::map<string, shared_ptr<PluginInterface> >::iterator it = m_plugins.find(name);
    if (it == m_plugins.end()) { // if no such plugin was found, attempt to load it now, using name as the filename
        m_plugins[name].reset(new PluginInterface);
        m_plugins[name]->Load(name);
        return m_plugins[name];
    } else { // otherwise, just return the plugin we found
        return it->second;
    }
}

void PluginManager::FreePlugin(const string& name)
{
    std::map<string, shared_ptr<PluginInterface> >::iterator it = m_plugins.find(name);
    if (it != m_plugins.end())
        m_plugins.erase(it);
}

void PluginManager::InitDynamicLoader()
{
    if (s_lt_dl_initialized)
        return;

    int err = lt_dlinit();
    if (err) {
        GG::App::GetApp()->Logger().errorStream() 
            << "PluginManager::InitDynamicLoader : lt_dlinit() call failed. (error #" << err << ": " << lt_dlerror() << ").";
    } else {
        s_lt_dl_initialized = true;
    }
}

void PluginManager::AddSearchDirectory(const std::string& dir)
{
    if (!s_lt_dl_initialized)
        InitDynamicLoader();

    int err = lt_dladdsearchdir(dir.c_str());
    if (err) {
        GG::App::GetApp()->Logger().errorStream() 
            << "PluginManager::AddSearchDirectory : lt_dladdsearchdir() call failed for directory \"" << dir << "\". (error #" << err 
            << ": " << lt_dlerror() << ").";
    }
}

void PluginManager::CleanupDynamicLoader()
{
    if (!s_lt_dl_initialized)
        return;

    int err = lt_dlexit();
    if (err) {
        GG::App::GetApp()->Logger().errorStream() 
            << "PluginManager::CleanupDynamicLoader : lt_dlexit() call failed. (error #" << err << ": " << lt_dlerror() << ").";
    } else {
        s_lt_dl_initialized = false;
    }
}
