// -*- C++ -*-
#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "GGListBox.h"

// Tutorial 3: Serialization
// This file is part of the third tutorial.  The other two files are serialization.cpp, saveload.h, and saveload.cpp.
// It extends the Tutorial 2 by serializing all the controls to a file called test.xml, deleting them, and recreating
// them from the XML file before showing them on the screen.  This demonstrates how GG serialization works in detail.
// For further reference material, see the boost serialization documentation.


// This is the same custom GG::ListBox::Row from Tutorial 2, now with serialization code added.  All classes derived
// from serializable GG classes need to have a serialize method defined in a similar fashion if they are to be
// serialized automatically.  Note also that such classes must be registered as well; see saveload.cpp for details.
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
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GG::ListBox::Row);
}


#endif // SERIALIZATION_H
