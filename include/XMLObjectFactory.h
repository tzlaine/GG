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

#ifndef _XMLObjectFactory_h_
#define _XMLObjectFactory_h_

#include <map>
#include <string>

namespace GG {

class XMLElement;

/** This class creates polymorphic subclasses of base class T from XML-formatted text.  For any polymorphic class hierarchy,
    you can instantiate XMLObjectFactory with the type of the hierarchy's base class, and provide functions that can each 
    construct one specific class in the hierarchy.  By providing a string that identifies each class, and creating XML objects
    with that string as a tag, you can ensure a method for creating the correct polymorphic subclass object at run-time. */
template <class T> class XMLObjectFactory
{
public:
    typedef T* (*Generator)(const XMLElement&); ///< this defines the function signature for XMLObjectFactory object generators

    /** \name Structors */ //@{
    XMLObjectFactory() {} ///< ctor
    //@}

    /** \name Accessors */ //@{
    T* GenerateObject(const XMLElement& elem) const ///< returns a heap-allocated subclass object of the appropriate type
    {
        T* retval = 0;
        typename std::map<std::string, Generator>::const_iterator it = m_generators.find(elem.Tag());
        if (it != m_generators.end())
            retval = it->second(elem);
        return retval;
    }
    //@}
   
    /** \name Mutators */ //@{
    /** adds (or overrides) a new generator that can generate subclass objects described by \a name */
    void AddGenerator(const std::string& name, Generator gen) {m_generators[name] = gen;}
    //@}

private:
    /** mapping from strings to functions that can create the type of object that corresponds to the string */
    std::map<std::string, Generator> m_generators;
};

} // namespace GG

#endif // _XMLObjectFactory_h_

