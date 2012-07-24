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

#include <GG/Filesystem.h>

#include <GG/utf8/checked.h>


namespace fs = boost::filesystem;

namespace {
    std::string ToUTF8(const std::string& path_name)
    { return path_name; }

    std::string ToUTF8(const std::wstring& path_name)
    {
        std::string retval;
#ifdef _WIN32
        utf8::utf16to8(path_name.begin(), path_name.end(), std::back_inserter(retval));
#else
        utf8::utf32to8(path_name.begin(), path_name.end(), std::back_inserter(retval));
#endif
        return retval;
    }

    template <typename PathString>
    PathString FromUTF8(const std::string& utf8);

    template <>
    std::string FromUTF8(const std::string& utf8)
    { return utf8; }

    template <>
    std::wstring FromUTF8(const std::string& utf8)
    {
        std::wstring retval;
#ifdef _WIN32
        utf8::utf8to16(utf8.begin(), utf8.end(), std::back_inserter(retval));
#else
        utf8::utf8to32(utf8.begin(), utf8.end(), std::back_inserter(retval));
#endif
        return retval;
    }
}

std::string GG::PathToUTF8(const fs::path& path)
{ return ToUTF8(path.string()); }

fs::path GG::UTF8ToPath(const std::string& utf8)
{ return fs::path(FromUTF8<fs::path::string_type>(utf8)); }
