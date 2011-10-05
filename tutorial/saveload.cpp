#include "saveload.h"

#include "serialization.h"

#include <GG/BrowseInfoWnd.h>
#include <GG/Button.h>
#include <GG/TextControl.h>
#include <GG/DropDownList.h>
#include <GG/DynamicGraphic.h>
#include <GG/Edit.h>
#include <GG/Layout.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Scroll.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StaticGraphic.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/dialogs/ColorDlg.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <fstream>

#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>

// Tutorial 3: Serialization

// This file is part of the third tutorial.  The other two files are
// serialization.h, serialization.cpp, and saveload.h.  It extends the
// Tutorial 2 by serializing all the controls to a file called test.xml,
// deleting them, and recreating them from the XML file before showing them on
// the screen.  This demonstrates how GG serialization works in detail.  For
// further reference material, see the boost serialization documentation.

// Below are registrations for all the classes in the GG::Wnd hierarchy.
// Classes must be registered whenever they are to be serialized through a
// pointer to a base class.  Apparently, the boost serialization XML parser
// cannot handle angle brackets (<>) inside of attribute text strings, so
// Spin<int> and Spin<double> must be registered using
// BOOST_CLASS_EXPORT_GUID, with a name that includes no angle brackets.  By
// convention, '_' is used in place of '<' and '>' throughout the GG
// serialization code.

// Note that CustomTextRow is also registered here, along with the GG-defined
// GG::Wnd subclasses.  Any GG::Wnd subclasses you create must also be
// registered if they are to be serialized automatically through GG::Wnd*
// pointers.

BOOST_CLASS_EXPORT(GG::Wnd)
BOOST_CLASS_EXPORT(GG::DropDownList)
BOOST_CLASS_EXPORT(GG::DynamicGraphic)
BOOST_CLASS_EXPORT(GG::Edit)
BOOST_CLASS_EXPORT(GG::Font)
BOOST_CLASS_EXPORT(GG::Layout)
BOOST_CLASS_EXPORT(GG::ListBox)
BOOST_CLASS_EXPORT(GG::ListBox::Row)
BOOST_CLASS_EXPORT(CustomTextRow)
BOOST_CLASS_EXPORT(GG::MenuBar)
BOOST_CLASS_EXPORT(GG::MultiEdit)
BOOST_CLASS_EXPORT(GG::Scroll)
BOOST_CLASS_EXPORT_GUID(GG::Slider<int>, "GG::Slider_int_")
BOOST_CLASS_EXPORT_GUID(GG::Spin<int>, "GG::Spin_int_")
BOOST_CLASS_EXPORT_GUID(GG::Spin<double>, "GG::Spin_double_")
BOOST_CLASS_EXPORT(GG::StaticGraphic)
BOOST_CLASS_EXPORT(GG::TextControl)
BOOST_CLASS_EXPORT(GG::Button)
BOOST_CLASS_EXPORT(GG::StateButton)
BOOST_CLASS_EXPORT(GG::RadioButtonGroup)

BOOST_CLASS_EXPORT(GG::HueSaturationPicker)
BOOST_CLASS_EXPORT(GG::ValuePicker)
BOOST_CLASS_EXPORT(GG::ColorDlg::ColorButton)
BOOST_CLASS_EXPORT(GG::ColorDlg::ColorDisplay)
BOOST_CLASS_EXPORT(GG::ColorDlg)
BOOST_CLASS_EXPORT(GG::FileDlg)
BOOST_CLASS_EXPORT(GG::ThreeButtonDlg)

void SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
{
    ar & boost::serialization::make_nvp(name.c_str(), wnd);
}

void LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    ar & boost::serialization::make_nvp(name.c_str(), wnd);
}
