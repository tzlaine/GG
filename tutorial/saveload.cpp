#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>

#include "saveload.h"

#include "serialization.h"

#include "GGBrowseInfoWnd.h"
#include "GGButton.h"
#include "GGTextControl.h"
#include "GGDropDownList.h"
#include "GGDynamicGraphic.h"
#include "GGEdit.h"
#include "GGLayout.h"
#include "GGMenu.h"
#include "GGMultiEdit.h"
#include "GGScroll.h"
#include "GGSlider.h"
#include "GGSpin.h"
#include "GGStaticGraphic.h"
#include "GGFileDlg.h"
#include "GGThreeButtonDlg.h"

#include <fstream>

#include <boost/serialization/export.hpp>

// Tutorial 3: Serialization
// This file is part of the third tutorial.  The other two files are serialization.h, serialization.cpp, and saveload.h.
// It extends the Tutorial 2 by serializing all the controls to a file called test.xml, deleting them, and recreating
// them from the XML file before showing them on the screen.  This demonstrates how GG serialization works in detail.
// For further reference material, see the boost serialization documentation.


// Below are registrations for all the classes in the GG::Wnd hierarchy.  Classes must be registered whenever they are
// to be serialized through a pointer to a base class.  The boost shared_ptr registration macro does not work with the
// scope operator, so GG has its own.  This is supposed to be fixed in boost 1.33.  Apparently, the XML parser cannot
// handle angle-brackets (<>) inside of attribute text strings, so Spin<int> and Spin<double> must be registered as
// typedefs that do not contain these characters.

// Note that CustomTextRow is also registered here, along with the GG-defined GG::Wnd subclasses.  Any GG::Wnd
// subclasses you create must also be registered if they are to be serialized automatically through GG::Wnd* pointers.

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
GG_SHARED_POINTER_EXPORT(CustomTextRow)
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
GG_SHARED_POINTER_EXPORT(GG::FileDlg)
GG_SHARED_POINTER_EXPORT(GG::ThreeButtonDlg)

void SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar)
{
    ar & boost::serialization::make_nvp(name.c_str(), wnd);
}

void LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    ar & boost::serialization::make_nvp(name.c_str(), wnd);
}
