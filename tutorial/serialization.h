// -*- C++ -*-
#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <GG/ListBox.h>

// Tutorial 3: Serialization

// This file is part of the third tutorial.  The other two files are
// serialization.cpp, saveload.h, and saveload.cpp.  It extends the Tutorial 2
// by serializing all the controls to a file called test.xml, deleting them,
// and recreating them from the XML file before showing them on the screen.
// This demonstrates how GG serialization works in detail.  For further
// reference material, see the boost serialization documentation.


// This is the same custom GG::ListBox::Row from Tutorial 2, now with
// serialization code added.  All classes derived from serializable GG classes
// need to have a serialize method defined in a similar fashion if they are to
// be serialized automatically.  Note also that such classes must be
// registered as well; see saveload.cpp for details.
struct CustomTextRow : GG::ListBox::Row
{
    CustomTextRow();
    CustomTextRow(const std::string& text);

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void CustomTextRow::serialize(Archive& ar, const unsigned int version)
{
    // Note that we cannot use
    // BOOST_SERIALIZATION_BASE_OBJECT_NVP(GG::ListBox::Row).  This is because
    // this would create a tag called <GG::ListBox::Row>, which is malformed
    // XML.  Instead, we declare the base-class subobject's name to be
    // GG_ListBox_Row.  Also note that this does not apply to
    // BOOST_CLASS_EXPORT declarations, which do not produce a tag of the same
    // name; they produce a class name stored as an attribute string, which is
    // fine (e.g.  <TAG ... class_name="GG::Layout" ...>).
    ar  & boost::serialization::make_nvp("GG_ListBox_Row",
                                         boost::serialization::base_object<GG::ListBox::Row>(*this));
}


#endif // SERIALIZATION_H
