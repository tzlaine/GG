#include <GG/adobe/adam_function.hpp>

#include <GG/adobe/dictionary.hpp>


namespace adobe {

function::function() :
    m_global_scope(0)
{}

function::function(name_t name, sheet_t& property_sheet) :
    m_function_name(name),
    m_global_scope(&property_sheet)
{}

void function::add_parameter(name_t parameter)
{ m_parameter_names.push_back(parameter); }

void function::add_statement(const array_t& statement)
{
    m_statements.resize(m_statements.size() + 1);
    m_statements.back().m_statement = statement;
    for (array_t::const_iterator
             it = statement.begin(),
             end_it = statement.end();
         it != end_it;
         ++it) {
        if (it->type_info() == type_info<name_t>()) {
            array_t::const_iterator next_it = boost::next(it);
            name_t name;
            if (next_it != end_it &&
                next_it->cast(name) &&
                name == static_name_t(".variable")) {
                m_statements.back().m_global_variables.push_back(
                    it->cast<name_t>()
                );
            }
        }
    }
}

any_regular_t function::array_function(const array_t& parameters)
{
    sheet_t local_scope;
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

any_regular_t function::dictionary_function(const dictionary_t& parameters)
{
    sheet_t local_scope;
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

void function::add_global_variables(sheet_t& local_scope,
                                    const std::vector<name_t>& global_variables)
{
    for (std::vector<name_t>::const_iterator
             it = global_variables.begin(),
             end_it = global_variables.end();
         it != end_it;
         ++it) {
        local_scope.add_constant(
            *it,
            line_position_t(),
            array_t(1, m_global_scope->get(*it))
        );
    }
}

any_regular_t function::common_impl(sheet_t& local_scope)
{
    local_scope.add_interface(
        static_name_t("dont_name_your_cells_retval_okay"),
        false,
        line_position_t(),
        array_t(),
        line_position_t(),
        array_t()
    );
    for (std::vector<statement>::const_iterator
             it = m_statements.begin(),
             end_it = m_statements.end();
         it != end_it;
         ++it) {
        add_global_variables(local_scope, it->m_global_variables);
        local_scope.inspect(it->m_statement);
    }
    return local_scope.get(
        static_name_t("dont_name_your_cells_retval_okay")
    );
}

}
