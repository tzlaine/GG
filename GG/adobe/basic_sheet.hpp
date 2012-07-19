/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_BASIC_SHEET_HPP
#define ADOBE_BASIC_SHEET_HPP

#include <GG/adobe/config.hpp>

#include <deque>
#include <map>
#include <vector>

#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <boost/variant.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/array_fwd.hpp>
#include <GG/adobe/dictionary_fwd.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>

/*************************************************************************************************/

namespace GG { class Wnd; }

/*************************************************************************************************/

namespace adobe {
/*!
\defgroup basic_property_model Basic Property Model
\ingroup property_model
 */

/*************************************************************************************************/

/*!
\ingroup basic_property_model
 */
class basic_sheet_t : boost::noncopyable
{
 public:
    
    typedef boost::signals::connection connection_t;
    typedef boost::function<void (const any_regular_t&)> monitor_value_t;
    typedef boost::signal<void (const any_regular_t&)>   monitor_value_list_t;
    void add_constant(name_t name,
                      const line_position_t& position,
                      const array_t& initializer,
                      const any_regular_t& value);
    void add_interface(name_t name,
                       const line_position_t& position,
                       const array_t& initializer,
                       const any_regular_t& value);
    void add_view(const boost::any& parent,
                  name_t name,
                  const line_position_t& position,
                  const array_t& initializer,
                  const boost::any& view);
    
    std::size_t count_interface(name_t) const;
    
    connection_t monitor_value(name_t name, const monitor_value_t& monitor);
    
    void set(name_t, const any_regular_t&); // interface cell
    void set(const dictionary_t& cell_set); // interface cell set

    const any_regular_t& operator[](name_t) const; // variable lookup

    dictionary_t contributing() const;

    void print(std::ostream& os) const;

 private:
    
    struct cell_t
    {
        cell_t(const any_regular_t& value) : value_m(value) { }
        any_regular_t value_m;
    };
    
    struct interface_cell_t : cell_t
    {
        interface_cell_t(const any_regular_t& value) : cell_t(value) { }
        
        interface_cell_t(const interface_cell_t& x) : cell_t(x.value_m) { }
        interface_cell_t& operator=(const interface_cell_t& x)
        {
            value_m = x.value_m; // copying a value could throw so it goes first
            // monitor_m is not copied - nor can it be
            return *this;
        }
                
                monitor_value_list_t monitor_value_m;
    };
    
    typedef std::map<const char*, interface_cell_t*, str_less_t>    interface_index_t;
    typedef std::map<const char*, const cell_t*, str_less_t>        variable_index_t;

    struct nested_views_t;

    void print_nested_view(std::ostream& os,
                           const nested_views_t& nested_view,
                           unsigned int indent) const;

    interface_cell_t* lookup_interface(name_t);
    
    variable_index_t                variable_index_m;
    interface_index_t               interface_index_m;

    std::deque<cell_t>              constant_cell_set_m;
    std::deque<interface_cell_t>    interface_cell_set_m;

    struct cell_parameters_t
    {
        cell_parameters_t (name_t name,
                           const line_position_t& position,
                           const array_t& initializer) :
            name_m(name),
            position_m(position),
            initializer_m(initializer)
            {}
        name_t name_m;
        line_position_t position_m;
        array_t initializer_m;
    };

    enum access_specifier_t
    {
        access_interface,
        access_constant
    };

    struct added_cell_set_t
    {
        added_cell_set_t(access_specifier_t access) :
            access_m(access)
            {}
        access_specifier_t access_m;
        std::vector<cell_parameters_t> added_cells_m;
    };

    struct view_parameters_t
    {
        view_parameters_t() {}
        view_parameters_t(GG::Wnd* parent,
                          const adobe::line_position_t& position,
                          adobe::name_t name,
                          const adobe::array_t& parameters) :
            m_parent(parent),
            m_position(position),
            m_name(name),
            m_parameters(parameters)
            {}
        GG::Wnd* m_parent;
        adobe::line_position_t m_position;
        adobe::name_t m_name;
        adobe::array_t m_parameters;
    };

    struct nested_views_t
    {
        nested_views_t() :
            m_nested_view_parent(0)
            {}
        nested_views_t(const view_parameters_t& view_parameters, nested_views_t* parent) :
            m_view_parameters(view_parameters),
            m_nested_view_parent(parent)
            {}
        view_parameters_t m_view_parameters;
        nested_views_t* m_nested_view_parent;
        std::vector<nested_views_t> m_children;
    };

    struct added_view_set_t
    {
        added_view_set_t() :
            m_current_nested_view(0)
            {}
        nested_views_t m_nested_views;
        nested_views_t* m_current_nested_view;
    };

    typedef boost::variant<
        added_cell_set_t,
        added_view_set_t
    > add_element_set_t;

    typedef std::vector<add_element_set_t> added_elements_t;

    added_elements_t added_elements_m;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
