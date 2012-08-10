#include <GG/adobe/adam_function.hpp>

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>


namespace {

    struct lvalue_adapter
    {
        lvalue_adapter(adobe::sheet_t& sheet) :
            m_sheet(sheet)
            {}
        adobe::any_regular_t* operator()(adobe::name_t name)
            { return &m_sheet.get(name); }
        adobe::sheet_t& m_sheet;
    };

}

namespace adobe {

adam_function::adam_function()
{}

adam_function::adam_function(name_t name,
                             const std::vector<name_t>& parameter_names,
                             const std::vector<array_t>& statements) :
    m_function_name(name),
    m_parameter_names(parameter_names),
    m_statements(statements)
{
    for (std::size_t i = 0; i < m_statements.size(); ++i) {
        for (array_t::const_iterator
                 it = m_statements[i].begin(),
                 end_it = m_statements[i].end();
             it != end_it;
             ++it) {
            if (it->type_info() == type_info<name_t>()) {
                array_t::const_iterator next_it = boost::next(it);
                name_t name;
                if (next_it != end_it && next_it->cast(name) && name == variable_k)
                    m_variables.insert(it->cast<name_t>());
            }
        }
    }
}

any_regular_t adam_function::operator()(
    const variable_lookup_t& variable_lookup,
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const array_t& parameters
) const
{
    sheet_t local_scope;
    common_init(variable_lookup,
                array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                local_scope);
    for (std::size_t i = 0; i < m_parameter_names.size(); ++i) {
        if (i < parameters.size()) {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(1, parameters[i]),
                                      line_position_t(),
                                      array_t());
        } else {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(),
                                      line_position_t(),
                                      array_t());
        }
    }
    return common_impl(local_scope);
}

any_regular_t adam_function::operator()(
    const variable_lookup_t& variable_lookup,
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const dictionary_t& parameters
) const
{
    sheet_t local_scope;
    common_init(variable_lookup,
                array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                local_scope);
    for (std::size_t i = 0; i < m_parameter_names.size(); ++i) {
        dictionary_t::const_iterator it =
            parameters.find(m_parameter_names[i]);
        if (it != parameters.end()) {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(1, it->second),
                                      line_position_t(),
                                      array_t());
        } else {
            local_scope.add_interface(m_parameter_names[i],
                                      false,
                                      line_position_t(),
                                      array_t(),
                                      line_position_t(),
                                      array_t());
        }
    }
    return common_impl(local_scope);
}

void adam_function::dump(std::ostream& os) const
{
    os << m_function_name << " (";
    for (std::size_t i = 0; i < m_parameter_names.size(); ++i) {
        if (i)
            os << ", ";
        os << m_parameter_names[i];
    }
    os << ") {\n";
    for (std::size_t i = 0; i < m_statements.size(); ++i) {
        os << "    " << m_statements[i] << " ;\n";
    }
    os << "}\n"
       << "variables:";
    for (std::set<name_t>::const_iterator it = m_variables.begin();
         it != m_variables.end();
         ++it) {
        os << ' ' << *it;
    }
    os << '\n';
}

void adam_function::common_init(
    const variable_lookup_t& variable_lookup,
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    sheet_t& local_scope
) const
{
    local_scope.machine_m.set_array_function_lookup(array_function_lookup);
    local_scope.machine_m.set_dictionary_function_lookup(dictionary_function_lookup);
    local_scope.machine_m.set_adam_function_lookup(adam_function_lookup);
    local_scope.machine_m.set_variable_lookup(boost::bind(&sheet_t::get, &local_scope, _1));
    local_scope.machine_m.set_lvalue_lookup(lvalue_adapter(local_scope));
    local_scope.machine_m.set_create_const_decl(
        boost::bind(&sheet_t::add_constant,
                    &local_scope,
                    _1,
                    line_position_t(),
                    _2)
    );
    local_scope.machine_m.set_create_decl(
        boost::bind(&sheet_t::add_interface,
                    &local_scope,
                    _1,
                    false,
                    line_position_t(),
                    _2,
                    line_position_t(),
                    array_t())
    );
    for (std::set<name_t>::const_iterator
             it = m_variables.begin(), end_it = m_variables.end();
         it != end_it;
         ++it) {
        try {
            any_regular_t value = variable_lookup(*it);
            local_scope.add_constant(*it, line_position_t(), array_t(1, value));
        } catch (const std::logic_error&) {}
    }
}

any_regular_t adam_function::common_impl(sheet_t& local_scope) const
{
    for (std::vector<array_t>::const_iterator
             it = m_statements.begin(),
             end_it = m_statements.end();
         it != end_it;
         ++it) {
        any_regular_t value = local_scope.inspect(*it);
        name_t last_op_name;
        if (it->back().cast(last_op_name) && last_op_name == return_k)
            return value;
    }
    return any_regular_t();
}

}
