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

    adobe::any_regular_t exec_statement(const adobe::array_t& statement,
                                        adobe::sheet_t& local_scope,
                                        bool& done);

    adobe::any_regular_t exec_block(adobe::array_t::const_iterator first,
                                    adobe::array_t::const_iterator last,
                                    adobe::sheet_t& local_scope,
                                    bool& done)
    {
        for (; first != last; ++first) {
            bool done_ = false;
            adobe::any_regular_t value =
                exec_statement(first->cast<adobe::array_t>(), local_scope, done);
            if (done_) {
                done = true;
                return value;
            }
        }
        return adobe::any_regular_t();
    }

    adobe::any_regular_t exec_statement(const adobe::array_t& statement,
                                        adobe::sheet_t& local_scope,
                                        bool& done)
    {
        adobe::name_t op;
        statement.back().cast(op);
        if (op == adobe::stmt_ifelse_k) {
            bool done_ = false;
            const adobe::array_t& condition_expr =
               statement[0].cast<adobe::array_t>();
            const bool condition =
                local_scope.inspect(condition_expr).cast<bool>();
            const adobe::array_t& stmt_block =
                (condition ? statement[1] : statement[2]).cast<adobe::array_t>();
            adobe::any_regular_t value =
                exec_block(stmt_block.begin(), stmt_block.end(), local_scope, done_);
            if (done_) {
                done = true;
                return value;
            }
        } else {
            adobe::any_regular_t value = local_scope.inspect(statement);
            if (op == adobe::return_k) {
                done = true;
                return value;
            }
        }
        return adobe::any_regular_t();
    }

}

namespace adobe {

adam_function_t::adam_function_t()
{}

adam_function_t::adam_function_t(name_t name,
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

name_t adam_function_t::name() const
{ return m_function_name; }

const std::vector<name_t>& adam_function_t::parameter_names() const
{ return m_parameter_names; }

const std::vector<array_t>& adam_function_t::statements() const
{ return m_statements; }

any_regular_t adam_function_t::operator()(
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

any_regular_t adam_function_t::operator()(
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

void adam_function_t::common_init(
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

any_regular_t adam_function_t::common_impl(sheet_t& local_scope) const
{
    for (std::vector<array_t>::const_iterator
             it = m_statements.begin(),
             end_it = m_statements.end();
         it != end_it;
         ++it) {
        bool done = false;
        any_regular_t value = exec_statement(*it, local_scope, done);
        if (done)
            return value;
    }
    return any_regular_t();
}

}
