/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_VIRTUAL_MACHINE_HPP
#define ADOBE_VIRTUAL_MACHINE_HPP

#include <GG/adobe/config.hpp>

#include <bitset>
#include <vector>

#define BOOST_FUNCTION_NO_DEPRECATED
#include <boost/function.hpp>
#include <boost/operators.hpp>

#include <GG/adobe/array_fwd.hpp>
#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/dictionary_fwd.hpp>

#include <GG/adobe/move.hpp>

#include <GG/adobe/any_regular.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

class adam_function_t;

/*************************************************************************************************/

class virtual_machine_t
{
 public:
    typedef array_t             expression_t;

    typedef any_regular_t(variable_lookup_signature_t)(name_t);
    typedef void (lvalue_assign_signature_t)(name_t, const any_regular_t&);
    typedef bool(dictionary_function_lookup_signature_t)(name_t, const dictionary_t&, any_regular_t&);
    typedef bool(array_function_lookup_signature_t)(name_t, const array_t&, any_regular_t&);
    typedef const adam_function_t&(adam_function_lookup_signature_t)(name_t);
    typedef void(create_const_decl_signature_t)(name_t, const array_t&);
    typedef void(create_decl_signature_t)(name_t, const array_t&);

    typedef boost::function<variable_lookup_signature_t>            variable_lookup_t;
    typedef boost::function<lvalue_assign_signature_t>              lvalue_assign_t;
    typedef boost::function<dictionary_function_lookup_signature_t> dictionary_function_lookup_t;
    typedef boost::function<array_function_lookup_signature_t>      array_function_lookup_t;
    typedef boost::function<adam_function_lookup_signature_t>       adam_function_lookup_t;
    typedef boost::function<create_const_decl_signature_t>          create_const_decl_t;
    typedef boost::function<create_decl_signature_t>                create_decl_t;

#if !defined(ADOBE_NO_DOCUMENTATION)
    virtual_machine_t();
    virtual_machine_t(const virtual_machine_t&);

    virtual_machine_t& operator = (const virtual_machine_t& rhs);

    ~virtual_machine_t();
#endif

    void evaluate(const expression_t& expression);
#if 0
    void evaluate_named_arguments(const dictionary_t&);
#endif

    const any_regular_t& back() const;
    any_regular_t& back();
    void pop_back();

    void set_variable_lookup(const variable_lookup_t&);
    void set_lvalue_assign(const lvalue_assign_t&);
    void set_array_function_lookup(const array_function_lookup_t&);
    void set_dictionary_function_lookup(const dictionary_function_lookup_t&);
    void set_adam_function_lookup(const adam_function_lookup_t&);
    void set_create_const_decl(const create_const_decl_t&);
    void set_create_decl(const create_decl_t&);

    class implementation_t;
 private:

    implementation_t* object_m;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
