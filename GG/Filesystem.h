// -*- C++ -*-
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

/** \file Filesystem.h \brief Contains utility functions that make the UTF-8
    strings used in GG interoperable with Boost.Filesystem paths. */

#ifndef _GG_Filesystem_h_
#define _GG_Filesystem_h_

#include <boost/filesystem.hpp>


namespace GG {

    /** Returns the name of \a path as a UTF-8 encoded string. */
    std::string PathToUTF8(const boost::filesystem::path& path);

    /** Constructs a path from a UTF-8 encoded string. */
    boost::filesystem::path UTF8ToPath(const std::string& utf8);

}

#endif
