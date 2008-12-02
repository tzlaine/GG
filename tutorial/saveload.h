// -*- C++ -*-
#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <string>

// Tutorial 3: Serialization

// This file is part of the third tutorial.  The other two files are
// serialization.h, serialization.cpp, and saveload.cpp.  It extends the
// Tutorial 2 by serializing all the controls to a file called test.xml,
// deleting them, and recreating them from the XML file before showing them on
// the screen.  This demonstrates how GG serialization works in detail.  For
// further reference material, see the boost serialization documentation.


namespace GG {
    class Wnd;
}

// These functions are placed in a separate compilation unit from the main
// code because they take so long to compile.  saveload.cpp has numberous
// class registration macros that are expanded into a dizzingly large amount
// of code.  Make sure you always hide serialization calls such as those
// inside of SaveWnd() and LoadWnd() behide a header; just a few of these
// calls sprinkled throughout your code will dramatically increase your
// compile times.

// This function saves any GG::Wnd-derived class to XML archive "ar", saving
// it with the name "name".
void SaveWnd(const GG::Wnd* wnd, const std::string& name, boost::archive::xml_oarchive& ar);

// This function loads a GG::Wnd-derived class from XML archive "ar", creating
// a new object on the heap.  The object's GG::Wnd* pointer is stored in wnd.
void LoadWnd(GG::Wnd*& wnd, const std::string& name, boost::archive::xml_iarchive& ar);

// Since the above function will only accept a GG::Wnd*, this one is provided
// to more conveniently accept GG::Wnd subclass pointers.
template <class T>
void LoadWnd(T*& wnd, const std::string& name, boost::archive::xml_iarchive& ar)
{
    GG::Wnd* wnd_as_base = wnd;
    LoadWnd(wnd_as_base, name, ar);
    wnd = dynamic_cast<T*>(wnd_as_base);
    assert(wnd);
}

#endif
