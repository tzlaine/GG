#ifndef _GGEnum_h_
#define _GGEnum_h_

#include <map>
#include <string>

namespace GG {
/** A base type for all templated EnumMap types. */
struct GG_API EnumMapBase
{
    enum {BAD_VALUE = -5000000};

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
template <> struct EnumMap< name > : EnumMapBase                                \
{                                                                               \
    typedef name EnumType;                                                      \
    typedef std::map<EnumType, std::string> MapType;                            \
    EnumMap ()                                                                  \
    {

/** Adds a single value from an enumerated type, and its corresponding string representation, to an EnumMap. */
#define ENUM_MAP_INSERT( value ) map_[ value ] = #value ;

/** Declares the end of a template specialization of EnumMap, for enumerated type \a name. */
#define ENUM_MAP_END                                                            \
    }                                                                           \
    virtual const std::string& FromEnum(int i) const                            \
    {                                                                           \
        return map_.find(EnumType(i))->second;                                  \
    }                                                                           \
    int FromString (const std::string &str) const                               \
    {                                                                           \
        for (MapType::const_iterator it = map_.begin();                         \
             it != map_.end();                                                  \
             ++it) {                                                            \
            if (it->second == str) {                                            \
                return it->first;                                               \
            }                                                                   \
        }                                                                       \
        return BAD_VALUE;                                                       \
    }                                                                           \
    MapType map_;                                                               \
};

/** Defines an input stream operator for enumerated type \a name.  Note that the generated function requires that EnumMap<name> be defined. */
#define ENUM_STREAM_IN( name )                                                  \
GG_API inline std::istream& operator>>(std::istream& is, name& v)               \
{                                                                               \
    std::string str;                                                            \
    is >> str;                                                                  \
    v = name (GetEnumMap< name >().FromString(str));                            \
    return is;                                                                  \
}

/** Defines an output stream operator for enumerated type \a name.  Note that the generated function requires that EnumMap<name> be defined. */
#define ENUM_STREAM_OUT( name )                                                 \
GG_API inline std::ostream& operator<<(std::ostream& os, name v)                \
{                                                                               \
    os << GetEnumMap< name >().FromEnum(v);                                     \
    return os;                                                                  \
}

#endif // _GGEnum_h_


