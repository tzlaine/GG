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

#ifndef _XMLValidators_h_
#define _XMLValidators_h_

#ifndef _GGBase_h_
#include <GGBase.h>
#endif

#ifndef _XMLDoc_h_
#include <XMLDoc.h>
#endif

#include <boost/lexical_cast.hpp>
#include <boost/limits.hpp>
#include <cmath>
#include <string>

namespace GG {
// these are needed by the StepValidator
namespace details
{
template <class T> inline T mod (T dividend, T divisor)
{
    return (dividend % divisor);
}

template <> inline float mod<float>(float dividend, float divisor)
{
    return std::fmod(dividend, divisor);
}

template <> inline double mod<double>(double dividend, double divisor)
{
    return std::fmod(dividend, divisor);
}

template <> inline long double mod<long double>(long double dividend, long double divisor)
{
    return std::fmod(dividend, divisor);
}
}

/** base class for all OptionsDB validators. Simply provides the basic interface. */
struct ValidatorBase
{
    /** returns normally if \a str is a valid value, or throws otherwise */
    virtual void Validate(const std::string& str) const = 0;

    /** returns a dynamically allocated copy of the object. */
    virtual ValidatorBase *Clone() const = 0;
};

/** determines if a string is a valid value for an OptionsDB option */
template <class T> 
struct Validator : public ValidatorBase
{
    virtual void Validate(const std::string& str) const
    {
        boost::lexical_cast<T>(str);
    }

    virtual Validator *Clone() const
    {
        return new Validator<T>();
    }
};

/** a Validator that constrains the range of valid values */
template <class T>
struct RangedValidator : public Validator<T>
{
    RangedValidator(const T& min, const T& max) : m_min(min), m_max(max) {}

    virtual void Validate(const std::string& str) const
    {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min) {
            throw std::runtime_error("RangedValidator::Validate() : String \"" + str + "\" has a value less than the allowed minimum of " +
				     boost::lexical_cast<std::string>(m_min) + ".");
	} else if (val > m_max) {
            throw std::runtime_error("RangedValidator::Validate() : String \"" + str + "\" has a value greater than the allowed maximum of " +
				     boost::lexical_cast<std::string>(m_max) + ".");
	}
    }
	
    virtual RangedValidator *Clone() const 
    {
        return new RangedValidator<T>(m_min, m_max);
    }

    const T m_min;
    const T m_max;
};

/** a Validator that constrains valid values to certain step-values (eg: 0, 25, 50, ...).  The steps are assumed to
    begin at the validated type's default-constructed value, unless another origin is specified. */
template <class T>
struct StepValidator : public Validator<T>
{
    StepValidator(const T& step, const T& origin = T()) : m_step_size(step), m_origin(origin) {}
	
    virtual void Validate(const std::string& str) const
    {
        T val = boost::lexical_cast<T>(str);
        if (std::abs(details::mod((val - m_origin), m_step_size)) > std::numeric_limits<T>::epsilon()) {
            throw std::runtime_error("StepValidator::Validate() : String \"" + str + "\" has a value that is not a step of the form " +
				     boost::lexical_cast<std::string>(m_origin) + " +/- N * " + m_step_size + ".");
	}
    }
	
    virtual StepValidator *Clone() const
    {
        return new StepValidator<T>(m_step_size, m_origin);
    }

    const T m_step_size;
    const T m_origin;
};

/** a Validator similar to a StepValidator, but that further constrains the valid values to be within a certain range (eg: [25, 50, ..., 200]). */
template <class T>
struct RangedStepValidator : public Validator<T>
{
public:
    RangedStepValidator(const T& step, const T& min, const T& max) : m_step_size(step), m_origin(T()), m_min(min), m_max(max) {}
    RangedStepValidator(const T& step, const T& origin, const T& min, const T& max) : m_step_size (step), m_origin (origin), m_min (min), m_max (max) {}

    virtual void Validate(const std::string& str) const
    {
        T val = boost::lexical_cast<T>(str);
        if (val < m_min) {
            throw std::runtime_error("RangedStepValidator::Validate() : String \"" + str + "\" has a value less than the allowed minimum of " +
				     boost::lexical_cast<std::string>(m_min) + ".");
	} else if (val > m_max) {
            throw std::runtime_error("RangedStepValidator::Validate() : String \"" + str + "\" has a value greater than the allowed maximum of " +
				     boost::lexical_cast<std::string>(m_max) + ".");
	} else if (std::abs(details::mod<T>(val - m_origin, m_step_size)) > std::numeric_limits<T>::epsilon()) {
            throw std::runtime_error("RangedStepValidator::Validate() : String \"" + str + "\" has a value that is not a step of the form " +
				     boost::lexical_cast<std::string>(m_origin) + " +/- N * " + m_step_size + ".");
	}
    }

    virtual RangedStepValidator *Clone() const
    {
        return new RangedStepValidator<T>(m_step_size, m_origin, m_min, m_max);
    }

    const T m_step_size;
    const T m_origin;
    const T m_min;
    const T m_max;
};

/** a Validator that validates a string as belonging to an enumerated type for which there exists an EnumMap. */
template <class T>
struct MappedEnumValidator : public Validator<T>
{
public:
    MappedEnumValidator() {}

    virtual void Validate(const std::string& str) const
    {
        T val = boost::lexical_cast<T>(str);

        if (val == EnumMap<T>::BAD_VALUE) {
            throw std::runtime_error("MappedEnumValidator::Validate() : String \"" + str + "\" does not match any value in the EnumMap for its type.");
	}
    }

    virtual MappedEnumValidator *Clone() const
    {
        return new MappedEnumValidator<T>();
    }
};

/** a Validator similar to a RangedValidator, but that further constrains the valid values to be power-of-two, 
    between 1 (or 0) and \a max, inclusive.  This is intended to be used to validate flag enumeration types 
    (e.g. GG::Wnd::WndFlag), though it will work for any integral type.  If the enumeration includes 0, \a 
    include_zero should be passed into the ctor with a value of true.  Also, note that enumeration types 
    that can be used successfully with this template must have at least a boost::lexical_cast-compatible 
    streaming operator defined, and preferably std::ostream& operator<<(std::ostream&, T&) as well.  Also note 
    that if an EnumMap exists for an enumerated type, MappedEnumValidator is preferred. */
template <class T>
struct FlagEnumValidator : public Validator<T>
{
public:
    FlagEnumValidator(T max, bool include_zero = false) : m_max(max), m_zero(include_zero) {}

    virtual void Validate(const std::string& str) const
    {
        T val = boost::lexical_cast<T>(str);

        if (val < (m_zero ? 0 : 1)) {
            throw std::runtime_error("FlagEnumValidator::Validate() : String \"" + str + "\" has a value less than the allowed minimum of " +
				     boost::lexical_cast<std::string>(m_zero ? T(0) : T(1)) + ".");
	} else if (val > m_max) {
            throw std::runtime_error("FlagEnumValidator::Validate() : String \"" + str + "\" has a value greater than the allowed maximum of " +
				     boost::lexical_cast<std::string>(m_max) + ".");
	}

	bool match = false;
	if (m_zero && val == 0) {
	    match = true;
	} else {
	    int match_val = 1;
	    while (match_val < m_max) {
		if (val == match_val) {
		    match = true;
		    break;
		}
		match_val *= 2;
	    }
	}

        if (!match) {
            throw std::runtime_error("FlagEnumValidator::Validate() : String \"" + str + "\" has a value that is not a power of two " +
				     boost::lexical_cast<std::string>(m_min) + ".");
	}
    }

    virtual FlagEnumValidator *Clone() const
    {
        return new FlagEnumValidator<T>(m_max);
    }

    T    m_max;
    bool m_zero;
};


/** a Validator that validates zero or more values in a whitespace-delimited list. */
template <class T> 
struct ListValidator : public ValidatorBase
{
    virtual void Validate(const std::string& str) const
    {
	vector<string> tokens = Tokenize(str);
	for (unsigned int i = 0; i < tokens.size(); ++i) {
	    boost::lexical_cast<T>(tokens[i]);
	}
    }

    virtual ListValidator *Clone() const
    {
        return new ListValidator<T>();
    }
};

/** a Validator that validates zero or more key-value pairs in a whitespace-delimited list. */
template <class T1, class T2> 
struct MapListValidator : public ValidatorBase
{
    virtual void Validate(const std::string& str) const
    {
	pair<vector<string>, vector<string> > tokens = TokenizeMapString(str);
	for (unsigned int i = 0; i < tokens.size(); ++i) {
	    boost::lexical_cast<T1>(tokens.first[i]);
	}
	for (unsigned int i = 0; i < tokens.size(); ++i) {
	    boost::lexical_cast<T2>(tokens.second[i]);
	}
    }

    virtual MapListValidator *Clone() const
    {
        return new MapListValidator<T1, T2>();
    }
};


} // namespace GG

#endif // _XMLValidators_h_
