#ifndef ADOBE_ADAM_FUNCTION_HPP
#define ADOBE_ADAM_FUNCTION_HPP

#include <GG/adobe/adam.hpp>

#include <set>


namespace adobe {

class adam_function_t
{
public:
    typedef virtual_machine_t::dictionary_function_lookup_t dictionary_function_lookup_t;
    typedef virtual_machine_t::array_function_lookup_t      array_function_lookup_t;
    typedef virtual_machine_t::adam_function_lookup_t       adam_function_lookup_t;

    adam_function_t();
    adam_function_t(name_t name,
                    const std::vector<name_t>& parameter_names,
                    const std::vector<array_t>& statements);

    name_t name() const;
    const std::vector<name_t>& parameter_names() const;
    const std::vector<array_t>& statements() const;

    any_regular_t operator()(const array_function_lookup_t& array_function_lookup,
                             const dictionary_function_lookup_t& dictionary_function_lookup,
                             const adam_function_lookup_t& adam_function_lookup,
                             const array_t& parameters) const;
    any_regular_t operator()(const array_function_lookup_t& array_function_lookup,
                             const dictionary_function_lookup_t& dictionary_function_lookup,
                             const adam_function_lookup_t& adam_function_lookup,
                             const dictionary_t& parameters) const;

private:
    name_t m_function_name;
    std::vector<name_t> m_parameter_names;
    std::vector<array_t> m_statements;
};

}

#endif
