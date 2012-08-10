#ifndef ADOBE_ADAM_FUNCTION_HPP
#define ADOBE_ADAM_FUNCTION_HPP

#include <GG/adobe/adam.hpp>

#include <set>


namespace adobe {

class adam_function
{
public:
    typedef virtual_machine_t::variable_lookup_t            variable_lookup_t;
    typedef virtual_machine_t::dictionary_function_lookup_t dictionary_function_lookup_t;
    typedef virtual_machine_t::array_function_lookup_t      array_function_lookup_t;
    typedef virtual_machine_t::adam_function_lookup_t       adam_function_lookup_t;
    typedef virtual_machine_t::create_const_decl_t          create_const_decl_t;
    typedef virtual_machine_t::create_decl_t                create_decl_t;

    adam_function();
    adam_function(name_t name,
                  const std::vector<name_t>& parameter_names,
                  const std::vector<array_t>& statements);

    any_regular_t operator()(const variable_lookup_t& variable_lookup,
                             const array_function_lookup_t& array_function_lookup,
                             const dictionary_function_lookup_t& dictionary_function_lookup,
                             const adam_function_lookup_t& adam_function_lookup,
                             const array_t& parameters) const;
    any_regular_t operator()(const variable_lookup_t& variable_lookup,
                             const array_function_lookup_t& array_function_lookup,
                             const dictionary_function_lookup_t& dictionary_function_lookup,
                             const adam_function_lookup_t& adam_function_lookup,
                             const dictionary_t& parameters) const;

    void dump(std::ostream& os) const;

private:
    name_t m_function_name;
    std::vector<name_t> m_parameter_names;
    std::vector<array_t> m_statements;
    std::set<name_t> m_variables;

    void common_init(const variable_lookup_t& variable_lookup,
                     const array_function_lookup_t& array_function_lookup,
                     const dictionary_function_lookup_t& dictionary_function_lookup,
                     const adam_function_lookup_t& adam_function_lookup,
                     sheet_t& local_scope) const;
    any_regular_t common_impl(sheet_t& local_scope) const;
};

}

#endif
