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

#ifndef _GGBase_h_
#define _GGBase_h_

typedef unsigned char Uint8;    ///< unsigned char from SDL.h; provided here in case GG is not being driven by SDL
typedef signed char Sint8;      ///< signed char from SDL.h; provided here in case GG is not being driven by SDL
typedef unsigned short Uint16;  ///< unsigned short from SDL.h; provided here in case GG is not being driven by SDL
typedef signed short Sint16;    ///< signed short from SDL.h; provided here in case GG is not being driven by SDL
typedef unsigned int Uint32;    ///< unsigned int from SDL.h; provided here in case GG is not being driven by SDL
typedef signed int Sint32;      ///< signed int from SDL.h; provided here in case GG is not being driven by SDL

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# ifdef GIGI_EXPORTS
#  define GG_API __declspec(dllexport)
# else
#  define GG_API __declspec(dllimport)
# endif
#else
# define GG_API
#endif

// include OpenGL headers
#include <GL/gl.h>
#include <GL/glu.h>

// include useful boost headers
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef BOOST_SHARED_ARRAY_HPP_INCLUDED
#include <boost/shared_array.hpp>
#endif

#ifndef BOOST_LEXICAL_CAST_INCLUDED
#include <boost/lexical_cast.hpp>
#endif

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

#ifndef BOOST_SIGNAL_HPP
#include <boost/signal.hpp>
#endif

// other project headers
#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#ifndef _XMLObjectFactory_h_
#include "XMLObjectFactory.h"
#endif

#ifndef _GGSignalsAndSlots_h_
#include "GGSignalsAndSlots.h"
#endif

#ifndef _GGPtRect_h_
#include "GGPtRect.h"
#endif

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <cmath>
#include <valarray>
#include <numeric>

namespace GG {
/** A base type for all templated EnumMap types. */
struct GG_API EnumMapBase
{
    virtual ~EnumMapBase() {}
    virtual const std::string& FromEnum(int) const = 0;
    virtual int FromString (const std::string&) const = 0;
};

/** A mapping between the values of an enum and the string representations of the enum's values.  A 
    specialization should be declared for each enumerated type for which an EnumMap is desired. */
template <class E> struct EnumMap : EnumMapBase
{
    virtual ~EnumMap() {}
    virtual const std::string& FromEnum(int) const {return "";}
    virtual int FromString (const std::string&) const {return 0;}
};

/** Returns a map of the values of an enum to the corresponding string representation of that value. */
template <class E> EnumMap<E> GetEnumMap()
{
    static EnumMap<E> enum_map;
    return enum_map;
}
}


/** Declares the beginning of a template specialization of EnumMap, for enumerated type \a name.
    Text-to-enum conversion is one of those places that calls for macro magic.  To use these for 
    e.g. "enum Foo {FOO, BAR};", call: 
    \verbatim 
    ENUM_MAP_BEGIN( Foo ) 
        ENUM_MAP_INSERT( FOO )
	ENUM_MAP_INSERT( BAR )
	...
    ENUM_MAP_END\endverbatim */
#define ENUM_MAP_BEGIN( name )                                                  \
template <> struct EnumMap< name > : EnumMapBase                       \
{                                                                               \
    typedef name EnumType;                                                      \
    EnumMap ()                                                                  \
    {

/** Adds a single value from an enumerated type, and its corresponding string representation, to an EnumMap. */
#define ENUM_MAP_INSERT( value ) map_[ value ] = #value ;

/** Declares the end of a template specialization of EnumMap, for enumerated type \a name. */
#define ENUM_MAP_END                                                            \
    }                                                                           \
    virtual ~EnumMap() {}                                                       \
    virtual const std::string& FromEnum(int i) const                            \
    {                                                                           \
        return map_.find(EnumType(i))->second;                                  \
    }                                                                           \
    int FromString (const std::string &str) const                               \
    {                                                                           \
        for (std::map<EnumType, std::string>::const_iterator it = map_.begin(); \
             it != map_.end();                                                  \
             ++it) {                                                            \
            if (it->second == str) {                                            \
                return it->first;                                               \
            }                                                                   \
        }                                                                       \
        return BAD_VALUE;                                                       \
    }                                                                           \
	enum {BAD_VALUE = -5000000};                                                \
    std::map<EnumType, std::string> map_;                                       \
};

/** Defines an input stream operator for enumerated type \a name.  Note that the generated function requires that EnumMap<name> be defined. */
#define ENUM_STREAM_IN( name )                                                  \
GG_API inline std::istream& operator>>(std::istream& is, name& v)             \
{                                                                               \
    std::string str;                                                            \
    is >> str;                                                                  \
    v = name (GetEnumMap< name >().FromString(str));                            \
    return is;                                                                  \
}

/** Defines an output stream operator for enumerated type \a name.  Note that the generated function requires that EnumMap<name> be defined. */
#define ENUM_STREAM_OUT( name )                                                 \
GG_API inline std::ostream& operator<<(std::ostream& os, name v)              \
{                                                                               \
    os << GetEnumMap< name >().FromEnum(v);                                     \
    return os;                                                                  \
}



/** \namespace GG
    The namespace that encloses all GG classes, functions, typedefs, enums, etc.*/
namespace GG {

using boost::shared_ptr;
using boost::shared_array;
using boost::lexical_cast;
using std::ostream;
using std::istream;
using std::stringstream;
using std::vector;
using std::string;
using std::stack;
using std::map;
using std::set;
using std::multimap;
using std::list;
using std::pair;
using std::valarray;


/** This is a base class for all GG exceptions.  It is based on the std::exception class.  Since it is preferable that exceptions
    not throw other exceptions, "throw()" (which means "throws nothing") has been appended to every member function.*/
class GG_API GGException : public std::exception
{
public:
   GGException() throw() {}                                    ///< a default ctor
   GGException(const string& msg) throw() : m_message(msg) {}  ///< a ctor that allows the throwing code to include a text message
   ~GGException() throw() {}                                   ///< dtor required by std::exception

   const string& Message() const throw() {return m_message;} ///< returns text message of this Exception

private:
   string m_message; ///< the text message associated with this Exception (may be "")
};

/** Okay, I \a hate macros, but this one is just too useful, since I want all the GG exception classes to be uniform and simple.*/
#define GGEXCEPTION( x ) class GG_API x : public GGException                  \
{                                                                               \
public:                                                                         \
    x () throw() : GGException() {}                                             \
    x (const string& msg) throw() : GGException(msg) {}                         \
    virtual const char* what() const throw() {return #x ;}                      \
};

/** "Regions" of a window; used eg to determine direction(s) of drag when a window that has a drag-frame is clicked*/
enum WndRegion {
    WR_NONE = -1, 
    WR_TOPLEFT = 0, 
    WR_TOP, 
    WR_TOPRIGHT, 
    WR_MIDLEFT, 
    WR_MIDDLE, 
    WR_MIDRIGHT, 
    WR_BOTTOMLEFT, 
    WR_BOTTOM, 
    WR_BOTTOMRIGHT
};

/** These flags are packed (via logical or) into a 32-bit unsigned int.  Bits 16-23 of the uint specify the number of 
   characters for each tab. The default number of characters per tab is 8.*/
enum TextFormat {
    TF_NONE =      0,
    TF_VCENTER =   1 << 0,     ///< Centers text vertically.
    TF_TOP =       1 << 1,     ///< Top-justifies text.
    TF_BOTTOM =    1 << 2,     ///< Justifies the text to the bottom of the rectangle.
    
    TF_CENTER =    1 << 3,     ///< Centers text horizontally in the rectangle. 
    TF_LEFT =      1 << 4,     ///< Aligns text to the left. 
    TF_RIGHT =     1 << 5,     ///< Aligns text to the right. 

    TF_WORDBREAK = 1 << 6,     ///< Breaks words. Lines are automatically broken between words if a word would extend past the edge of the control's bounding rectangle. (As always, a '\\n' also breaks the line.)
    TF_LINEWRAP =  1 << 7      ///< Lines are automatically broken when the next character (or space) would be drawn outside the the text rectangle.
};

// define EnumMap and stream operators for TextFormat
ENUM_MAP_BEGIN(TextFormat)
    ENUM_MAP_INSERT(TF_NONE)
    ENUM_MAP_INSERT(TF_VCENTER)
    ENUM_MAP_INSERT(TF_TOP)
    ENUM_MAP_INSERT(TF_BOTTOM)
    ENUM_MAP_INSERT(TF_CENTER)
    ENUM_MAP_INSERT(TF_LEFT)
    ENUM_MAP_INSERT(TF_RIGHT)
    ENUM_MAP_INSERT(TF_WORDBREAK)
    ENUM_MAP_INSERT(TF_LINEWRAP)
ENUM_MAP_END

ENUM_STREAM_IN(TextFormat)
ENUM_STREAM_OUT(TextFormat)


/** styles for StaticGraphic controls*/
enum GraphicStyle {
    GR_NONE =      0,
    GR_VCENTER =   1 << 0,     ///< Centers graphic vertically.
    GR_TOP =       1 << 1,     ///< Top-justifies graphic.
    GR_BOTTOM =    1 << 2,     ///< Justifies the graphic to the bottom of the rectangle.

    GR_CENTER =    1 << 3,     ///< Centers graphic horizontally in the rectangle.
    GR_LEFT =      1 << 4,     ///< Aligns graphic to the left.
    GR_RIGHT =     1 << 5,     ///< Aligns graphic to the right.

    GR_FITGRAPHIC =1 << 6,     ///< Scales graphic to fit within the StaticGraphic's window dimensions.
    GR_SHRINKFIT = 1 << 7,     ///< Like GR_FITGRAPHIC, but this one only scales the image if it otherwise would not fit in the window.
    GR_PROPSCALE = 1 << 8      ///< If GR_FITGRAPHIC or GR_SHRINKFIT is used, this ensures scaling is done proportionally.
};

// define EnumMap and stream operators for GraphicStyle
ENUM_MAP_BEGIN(GraphicStyle)
    ENUM_MAP_INSERT(GR_NONE)
    ENUM_MAP_INSERT(GR_VCENTER)
    ENUM_MAP_INSERT(GR_TOP)
    ENUM_MAP_INSERT(GR_BOTTOM)
    ENUM_MAP_INSERT(GR_CENTER)
    ENUM_MAP_INSERT(GR_LEFT)
    ENUM_MAP_INSERT(GR_RIGHT)
    ENUM_MAP_INSERT(GR_FITGRAPHIC)
    ENUM_MAP_INSERT(GR_SHRINKFIT)
    ENUM_MAP_INSERT(GR_PROPSCALE)
ENUM_MAP_END

ENUM_STREAM_IN(GraphicStyle)
ENUM_STREAM_OUT(GraphicStyle)


/** styles for ListBox controls*/
enum ListBoxStyle {
    LB_NONE =            0,
    LB_VCENTER =         1 << 0,  ///< Cells are aligned with the top of the list box control.
    LB_TOP =             1 << 1,  ///< Cells are aligned with the top of the list box control. This is the default.
    LB_BOTTOM =          1 << 2,  ///< Cells are aligned with the bottom of the list box control.

    LB_CENTER =          1 << 3,  ///< Cells are center-aligned.
    LB_LEFT =            1 << 4,  ///< Cells are left-aligned. This is the default.
    LB_RIGHT =           1 << 5,  ///< Cells are right-aligned.

    LB_NOSORT =          1 << 10, ///< List items are not sorted. Items are sorted by default.  When combined with LB_DRAGDROP, this style allows arbitrary rearrangement of list elements by dragging.
    LB_SORTDESCENDING =  1 << 11, ///< Items are sorted based on item text in ascending order. Ascending order is the default.

    LB_NOSEL =           1 << 13, ///< No selection, dragging, or dropping allowed.  This makes the list box effectively read-only.
    LB_SINGLESEL =       1 << 14, ///< Only one item at a time can be selected. By default, multiple items may be selected.
    LB_QUICKSEL =        1 << 15, ///< Each click toggles an item without affecting any others; ignored when used with LB_SINGLESEL.

    LB_DRAGDROP =        1 << 16, ///< Items can be dragged from or dropped into the list box.  Only specified drop types are allowed, but anything can be dragged.  By default drag-n-drop is disabled.   When combined with LB_NOSORT, this style allows arbitrary rearrangement of list elements by dragging.
    LB_USERDELETE =      1 << 18, ///< Allows user to remove selected items by pressing the delete key.

    LB_BROWSEUPDATES =   1 << 19, ///< Causes a signal to be emitted whenever the mouse moves over ("browses") a row.
};   

// define EnumMap and stream operators for ListBoxStyle
ENUM_MAP_BEGIN(ListBoxStyle)
    ENUM_MAP_INSERT(LB_NONE)
    ENUM_MAP_INSERT(LB_VCENTER)
    ENUM_MAP_INSERT(LB_TOP)
    ENUM_MAP_INSERT(LB_BOTTOM)
    ENUM_MAP_INSERT(LB_CENTER)
    ENUM_MAP_INSERT(LB_LEFT)
    ENUM_MAP_INSERT(LB_RIGHT)
    ENUM_MAP_INSERT(LB_NOSORT)
    ENUM_MAP_INSERT(LB_SORTDESCENDING)
    ENUM_MAP_INSERT(LB_NOSEL)
    ENUM_MAP_INSERT(LB_SINGLESEL)
    ENUM_MAP_INSERT(LB_QUICKSEL)
    ENUM_MAP_INSERT(LB_DRAGDROP)
    ENUM_MAP_INSERT(LB_USERDELETE)
    ENUM_MAP_INSERT(LB_BROWSEUPDATES)
ENUM_MAP_END

ENUM_STREAM_IN(ListBoxStyle)
ENUM_STREAM_OUT(ListBoxStyle)

/** adpated from SDLKey enum in SDL_keysym.h of the SDL library; capital letter keys added*/
enum Key {
    // The keyboard symbols have been cleverly chosen to map to ASCII
    GGK_UNKNOWN      = 0,
    GGK_FIRST        = 0,
    GGK_BACKSPACE    = 8,
    GGK_TAB          = 9,
    GGK_CLEAR        = 12,
    GGK_RETURN       = 13,
    GGK_PAUSE        = 19,
    GGK_ESCAPE       = 27,
    GGK_SPACE        = 32,
    GGK_EXCLAIM      = 33,
    GGK_QUOTEDBL     = 34,
    GGK_HASH         = 35,
    GGK_DOLLAR       = 36,
    GGK_AMPERSAND    = 38,
    GGK_QUOTE        = 39,
    GGK_LEFTPAREN    = 40,
    GGK_RIGHTPAREN   = 41,
    GGK_ASTERISK     = 42,
    GGK_PLUS         = 43,
    GGK_COMMA        = 44,
    GGK_MINUS        = 45,
    GGK_PERIOD       = 46,
    GGK_SLASH        = 47,
    GGK_0            = 48,
    GGK_1            = 49,
    GGK_2            = 50,
    GGK_3            = 51,
    GGK_4            = 52,
    GGK_5            = 53,
    GGK_6            = 54,
    GGK_7            = 55,
    GGK_8            = 56,
    GGK_9            = 57,
    GGK_COLON        = 58,
    GGK_SEMICOLON    = 59,
    GGK_LESS         = 60,
    GGK_EQUALS       = 61,
    GGK_GREATER      = 62,
    GGK_QUESTION     = 63,
    GGK_AT           = 64,
    GGK_A            = 65,
    GGK_B            = 66,
    GGK_C            = 67,
    GGK_D            = 68,
    GGK_E            = 69,
    GGK_F            = 70,
    GGK_G            = 71,
    GGK_H            = 72,
    GGK_I            = 73,
    GGK_J            = 74,
    GGK_K            = 75,
    GGK_L            = 76,
    GGK_M            = 77,
    GGK_N            = 78,
    GGK_O            = 79,
    GGK_P            = 80,
    GGK_Q            = 81,
    GGK_R            = 82,
    GGK_S            = 83,
    GGK_T            = 84,
    GGK_U            = 85,
    GGK_V            = 86,
    GGK_W            = 87,
    GGK_X            = 88,
    GGK_Y            = 89,
    GGK_Z            = 90,
    GGK_LEFTBRACKET  = 91,
    GGK_BACKSLASH    = 92,
    GGK_RIGHTBRACKET = 93,
    GGK_CARET        = 94,
    GGK_UNDERSCORE   = 95,
    GGK_BACKQUOTE    = 96,
    GGK_a            = 97,
    GGK_b            = 98,
    GGK_c            = 99,
    GGK_d            = 100,
    GGK_e            = 101,
    GGK_f            = 102,
    GGK_g            = 103,
    GGK_h            = 104,
    GGK_i            = 105,
    GGK_j            = 106,
    GGK_k            = 107,
    GGK_l            = 108,
    GGK_m            = 109,
    GGK_n            = 110,
    GGK_o            = 111,
    GGK_p            = 112,
    GGK_q            = 113,
    GGK_r            = 114,
    GGK_s            = 115,
    GGK_t            = 116,
    GGK_u            = 117,
    GGK_v            = 118,
    GGK_w            = 119,
    GGK_x            = 120,
    GGK_y            = 121,
    GGK_z            = 122,
    GGK_DELETE       = 127,
    // End of ASCII mapped keysyms

    // International keyboard syms
    GGK_WORLD_0      = 160,      ///< 0xA0
    GGK_WORLD_1      = 161,
    GGK_WORLD_2      = 162,
    GGK_WORLD_3      = 163,
    GGK_WORLD_4      = 164,
    GGK_WORLD_5      = 165,
    GGK_WORLD_6      = 166,
    GGK_WORLD_7      = 167,
    GGK_WORLD_8      = 168,
    GGK_WORLD_9      = 169,
    GGK_WORLD_10     = 170,
    GGK_WORLD_11     = 171,
    GGK_WORLD_12     = 172,
    GGK_WORLD_13     = 173,
    GGK_WORLD_14     = 174,
    GGK_WORLD_15     = 175,
    GGK_WORLD_16     = 176,
    GGK_WORLD_17     = 177,
    GGK_WORLD_18     = 178,
    GGK_WORLD_19     = 179,
    GGK_WORLD_20     = 180,
    GGK_WORLD_21     = 181,
    GGK_WORLD_22     = 182,
    GGK_WORLD_23     = 183,
    GGK_WORLD_24     = 184,
    GGK_WORLD_25     = 185,
    GGK_WORLD_26     = 186,
    GGK_WORLD_27     = 187,
    GGK_WORLD_28     = 188,
    GGK_WORLD_29     = 189,
    GGK_WORLD_30     = 190,
    GGK_WORLD_31     = 191,
    GGK_WORLD_32     = 192,
    GGK_WORLD_33     = 193,
    GGK_WORLD_34     = 194,
    GGK_WORLD_35     = 195,
    GGK_WORLD_36     = 196,
    GGK_WORLD_37     = 197,
    GGK_WORLD_38     = 198,
    GGK_WORLD_39     = 199,
    GGK_WORLD_40     = 200,
    GGK_WORLD_41     = 201,
    GGK_WORLD_42     = 202,
    GGK_WORLD_43     = 203,
    GGK_WORLD_44     = 204,
    GGK_WORLD_45     = 205,
    GGK_WORLD_46     = 206,
    GGK_WORLD_47     = 207,
    GGK_WORLD_48     = 208,
    GGK_WORLD_49     = 209,
    GGK_WORLD_50     = 210,
    GGK_WORLD_51     = 211,
    GGK_WORLD_52     = 212,
    GGK_WORLD_53     = 213,
    GGK_WORLD_54     = 214,
    GGK_WORLD_55     = 215,
    GGK_WORLD_56     = 216,
    GGK_WORLD_57     = 217,
    GGK_WORLD_58     = 218,
    GGK_WORLD_59     = 219,
    GGK_WORLD_60     = 220,
    GGK_WORLD_61     = 221,
    GGK_WORLD_62     = 222,
    GGK_WORLD_63     = 223,
    GGK_WORLD_64     = 224,
    GGK_WORLD_65     = 225,
    GGK_WORLD_66     = 226,
    GGK_WORLD_67     = 227,
    GGK_WORLD_68     = 228,
    GGK_WORLD_69     = 229,
    GGK_WORLD_70     = 230,
    GGK_WORLD_71     = 231,
    GGK_WORLD_72     = 232,
    GGK_WORLD_73     = 233,
    GGK_WORLD_74     = 234,
    GGK_WORLD_75     = 235,
    GGK_WORLD_76     = 236,
    GGK_WORLD_77     = 237,
    GGK_WORLD_78     = 238,
    GGK_WORLD_79     = 239,
    GGK_WORLD_80     = 240,
    GGK_WORLD_81     = 241,
    GGK_WORLD_82     = 242,
    GGK_WORLD_83     = 243,
    GGK_WORLD_84     = 244,
    GGK_WORLD_85     = 245,
    GGK_WORLD_86     = 246,
    GGK_WORLD_87     = 247,
    GGK_WORLD_88     = 248,
    GGK_WORLD_89     = 249,
    GGK_WORLD_90     = 250,
    GGK_WORLD_91     = 251,
    GGK_WORLD_92     = 252,
    GGK_WORLD_93     = 253,
    GGK_WORLD_94     = 254,
    GGK_WORLD_95     = 255,      ///< 0xFF

    // Numeric keypad
    GGK_KP0          = 256,
    GGK_KP1          = 257,
    GGK_KP2          = 258,
    GGK_KP3          = 259,
    GGK_KP4          = 260,
    GGK_KP5          = 261,
    GGK_KP6          = 262,
    GGK_KP7          = 263,
    GGK_KP8          = 264,
    GGK_KP9          = 265,
    GGK_KP_PERIOD      = 266,
    GGK_KP_DIVIDE    = 267,
    GGK_KP_MULTIPLY  = 268,
    GGK_KP_MINUS     = 269,
    GGK_KP_PLUS      = 270,
    GGK_KP_ENTER     = 271,
    GGK_KP_EQUALS    = 272,

    // Arrows + Home/End pad
    GGK_UP           = 273,
    GGK_DOWN         = 274,
    GGK_RIGHT        = 275,
    GGK_LEFT         = 276,
    GGK_INSERT       = 277,
    GGK_HOME         = 278,
    GGK_END          = 279,
    GGK_PAGEUP       = 280,
    GGK_PAGEDOWN     = 281,

    // Function keys
    GGK_F1           = 282,
    GGK_F2           = 283,
    GGK_F3           = 284,
    GGK_F4           = 285,
    GGK_F5           = 286,
    GGK_F6           = 287,
    GGK_F7           = 288,
    GGK_F8           = 289,
    GGK_F9           = 290,
    GGK_F10          = 291,
    GGK_F11          = 292,
    GGK_F12          = 293,
    GGK_F13          = 294,
    GGK_F14          = 295,
    GGK_F15          = 296,

    // Key state modifier keys
    GGK_NUMLOCK      = 300,
    GGK_CAPSLOCK     = 301,
    GGK_SCROLLOCK    = 302,
    GGK_RSHIFT       = 303,
    GGK_LSHIFT       = 304,
    GGK_RCTRL        = 305,
    GGK_LCTRL        = 306,
    GGK_RALT         = 307,
    GGK_LALT         = 308,
    GGK_RMETA        = 309,
    GGK_LMETA        = 310,
    GGK_LSUPER       = 311,      ///< Left "Windows" key
    GGK_RSUPER       = 312,      ///< Right "Windows" key
    GGK_MODE         = 313,      ///< "Alt Gr" key
    GGK_COMPOSE      = 314,      ///< Multi-key compose key

    // Miscellaneous function keys
    GGK_HELP         = 315,
    GGK_PRINT        = 316,
    GGK_SYSREQ       = 317,
    GGK_BREAK        = 318,
    GGK_MENU         = 319,
    GGK_POWER        = 320,      ///< Power Macintosh power key
    GGK_EURO         = 321,      ///< Some european keyboards
    GGK_UNDO         = 322,      ///< Atari keyboard has Undo

    // Add any other keys here

    GGK_LAST
};

/** adpated from SDLKey enum in SDL_keysym.h of the SDL library; enumeration of valid key mods (possibly |'d together)*/
enum Mod {
    GGKMOD_NONE       = 0x0000,
    GGKMOD_LSHIFT     = 0x0001,
    GGKMOD_RSHIFT     = 0x0002,
    GGKMOD_LCTRL      = 0x0040,
    GGKMOD_RCTRL      = 0x0080,
    GGKMOD_LALT       = 0x0100,
    GGKMOD_RALT       = 0x0200,
    GGKMOD_LMETA      = 0x0400,
    GGKMOD_RMETA      = 0x0800,
    GGKMOD_NUM        = 0x1000,
    GGKMOD_CAPS       = 0x2000,
    GGKMOD_MODE       = 0x4000,
    GGKMOD_RESERVED   = 0x8000,
    GGKMOD_CTRL       = (GGKMOD_LCTRL | GGKMOD_RCTRL),    ///< either control key
    GGKMOD_SHIFT      = (GGKMOD_LSHIFT | GGKMOD_RSHIFT),  ///< either shift key
    GGKMOD_ALT        = (GGKMOD_LALT | GGKMOD_RALT),      ///< either alt key
    GGKMOD_META       = (GGKMOD_LMETA | GGKMOD_RMETA)     ///< either meta key
};

} // namespace GG

#endif // _GGBase_h_

