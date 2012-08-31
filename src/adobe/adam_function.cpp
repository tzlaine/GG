#include <GG/adobe/adam_function.hpp>

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/lex_shared.hpp>
#include <GG/adobe/implementation/token.hpp>


namespace {

    typedef std::pair<adobe::any_regular_t, bool> stack_variable_t;
    typedef adobe::closed_hash_map<adobe::name_t, stack_variable_t> stack_frame_t;

    struct variable_declarer
    {
        variable_declarer(adobe::sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        void operator()(adobe::name_t name, const adobe::array_t& expression, bool const_) const
            {
                if (!m_stack.empty()) {
                    if (m_stack.back().count(name)) {
                        throw std::runtime_error(
                            adobe::make_string("Attempted to re-declare variable ", name.c_str())
                        );
                    }
                    stack_variable_t& stack_variable = m_stack.back()[name];
                    if (!expression.empty())
                        stack_variable.first = m_sheet.inspect(expression);
                    stack_variable.second = const_;
                } else {
                    if (const_) {
                        m_sheet.add_constant(name, adobe::line_position_t(), expression);
                    } else {
                        m_sheet.add_interface(name,
                                              false,
                                              adobe::line_position_t(),
                                              expression,
                                              adobe::line_position_t(),
                                              adobe::array_t());
                    }
                }
            }

        adobe::sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct variable_getter
    {
        variable_getter(adobe::sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        adobe::any_regular_t operator()(adobe::name_t name) const
            {
                for (std::vector<stack_frame_t>::const_reverse_iterator
                         it = m_stack.rbegin(), end_it = m_stack.rend();
                     it != end_it;
                     ++it) {
                    stack_frame_t::const_iterator frame_it = it->find(name);
                    if (frame_it != it->end())
                        return frame_it->second.first;
                }
                return m_sheet.get(name);
            }

        adobe::sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct variable_setter
    {
        variable_setter(adobe::sheet_t& sheet, std::vector<stack_frame_t>& stack) :
            m_sheet(sheet),
            m_stack(stack)
            {}

        void operator()(adobe::name_t name, const adobe::any_regular_t& value) const
            {
                for (std::vector<stack_frame_t>::reverse_iterator
                         it = m_stack.rbegin(), end_it = m_stack.rend();
                     it != end_it;
                     ++it) {
                    stack_frame_t::iterator frame_it = it->find(name);
                    if (frame_it == it->end())
                        continue;
                    if (frame_it->second.second) {
                        throw std::runtime_error(
                            adobe::make_string("Attempted to set the value of const variable ", name.c_str())
                        );
                    }
                    frame_it->second.first = value;
                    return;
                }
                m_sheet.set(name, value);
                m_sheet.update();
            }

        adobe::sheet_t& m_sheet;
        std::vector<stack_frame_t>& m_stack;
    };

    struct scoped_stack_frame
    {
        scoped_stack_frame(std::vector<stack_frame_t>& stack) :
            m_stack(stack)
            { m_stack.push_back(stack_frame_t()); }
        ~scoped_stack_frame()
            { m_stack.pop_back(); }

        std::vector<stack_frame_t>& m_stack;
    };

    struct lvalue
    {
        lvalue(const variable_getter& get, const adobe::array_t& expression) :
            m_cell_name(expression[0].cast<adobe::name_t>()),
            m_cell_value(new adobe::any_regular_t(get(m_cell_name))),
            m_lvalue(m_cell_value.get())
            {}
        adobe::name_t m_cell_name;
        boost::shared_ptr<adobe::any_regular_t> m_cell_value;
        adobe::any_regular_t* m_lvalue;
    };

    lvalue evaluate_lvalue_expression(adobe::sheet_t& sheet,
                                      const variable_getter& get,
                                      const adobe::array_t& expression)
    {
        lvalue retval(get, expression);
        adobe::array_t value_stack;
        for (adobe::array_t::const_iterator it(expression.begin()); it != expression.end(); ++it) {
            adobe::name_t op;
            if (it->cast(op) && op.c_str()[0] == '.') {                
                if (op == adobe::variable_k) {
                    value_stack.pop_back();
                } else if (op == adobe::bracket_index_k) {
                    adobe::any_regular_t index = sheet.inspect(value_stack.back().cast<adobe::array_t>());
                    value_stack.pop_back();
                    if (index.type_info() == adobe::type_info<adobe::name_t>()) {
                        retval.m_lvalue =
                            &retval.m_lvalue->cast<adobe::dictionary_t>()[index.cast<adobe::name_t>()];
                    } else {
                        adobe::array_t& array = retval.m_lvalue->cast<adobe::array_t>();
                        std::size_t i = index.cast<std::size_t>();
                        if (array.size() <= i)
                            throw std::runtime_error("lvalue index: array index out of range");
                        retval.m_lvalue = &array[i];
                    }
                } else if (op == adobe::dot_index_k) {
                    retval.m_lvalue =
                        &retval.m_lvalue->cast<adobe::dictionary_t>()[
                            value_stack.back().cast<adobe::name_t>()
                        ];
                    value_stack.pop_back();
                } else if (op == adobe::ifelse_k) {
                    lvalue else_ = evaluate_lvalue_expression(sheet, get, value_stack.back().cast<adobe::array_t>());
                    value_stack.pop_back();
                    lvalue if_ = evaluate_lvalue_expression(sheet, get, value_stack.back().cast<adobe::array_t>());
                    value_stack.pop_back();
                    bool condition = sheet.inspect(value_stack.back().cast<adobe::array_t>()).cast<bool>();
                    value_stack.pop_back();
                    retval = condition ? if_ : else_;
                }
            } else {
                value_stack.push_back(*it);
            }
        }
        return retval;
    }

    adobe::any_regular_t exec_statement(const adobe::array_t& statement,
                                        adobe::sheet_t& local_scope,
                                        std::vector<stack_frame_t>& stack,
                                        bool in_loop,
                                        bool& block_continue,
                                        bool& block_break,
                                        bool& function_done);

    adobe::any_regular_t exec_block(adobe::array_t::const_iterator first,
                                    adobe::array_t::const_iterator last,
                                    adobe::sheet_t& local_scope,
                                    std::vector<stack_frame_t>& stack,
                                    bool in_loop,
                                    bool& block_continue,
                                    bool& block_break,
                                    bool& function_done)
    {
        for (; first != last; ++first) {
            adobe::any_regular_t value = exec_statement(first->cast<adobe::array_t>(),
                                                        local_scope,
                                                        stack,
                                                        in_loop,
                                                        block_continue,
                                                        block_break,
                                                        function_done);
            if (block_continue || block_break)
                break;
            if (function_done)
                return value;
        }
        return adobe::any_regular_t();
    }

    adobe::any_regular_t exec_statement(const adobe::array_t& statement,
                                        adobe::sheet_t& local_scope,
                                        std::vector<stack_frame_t>& stack,
                                        bool in_loop,
                                        bool& block_continue,
                                        bool& block_break,
                                        bool& function_done)
    {
        variable_declarer declare(local_scope, stack);
        variable_getter get(local_scope, stack);
        variable_setter set(local_scope, stack);

        adobe::name_t op;
        statement.back().cast(op);

        if (op == adobe::assign_k) {
            adobe::any_regular_t value =
                local_scope.inspect(adobe::array_t(statement.begin() + 1, statement.end() - 1));
            lvalue l_value =
                evaluate_lvalue_expression(local_scope, get, statement[0].cast<adobe::array_t>());
            *l_value.m_lvalue = value;
            set(l_value.m_cell_name, *l_value.m_cell_value);
        } else if (op == adobe::const_decl_k || op == adobe::decl_k) {
            declare(statement[0].cast<adobe::name_t>(),
                    statement[1].cast<adobe::array_t>(),
                    op == adobe::const_decl_k);
        } else if (op == adobe::stmt_ifelse_k) {
            const adobe::array_t& condition_expr =
               statement[0].cast<adobe::array_t>();
            const bool condition =
                local_scope.inspect(condition_expr).cast<bool>();
            const adobe::array_t& stmt_block =
                (condition ? statement[1] : statement[2]).cast<adobe::array_t>();
            scoped_stack_frame local_stack_frame(stack);
            adobe::any_regular_t value = exec_block(stmt_block.begin(),
                                                    stmt_block.end(),
                                                    local_scope,
                                                    stack,
                                                    false,
                                                    block_continue,
                                                    block_break,
                                                    function_done);
            assert(!block_continue && !block_break);
            if (function_done)
                return value;
        } else if (op == adobe::simple_for_k) {
            scoped_stack_frame local_stack_frame(stack);
            adobe::name_t loop_var_0 = statement[0].cast<adobe::name_t>();
            declare(loop_var_0, adobe::array_t(), false);
            adobe::name_t loop_var_1 = statement[1].cast<adobe::name_t>();
            if (loop_var_1)
                declare(loop_var_1, adobe::array_t(), false);
            const adobe::any_regular_t sequence =
                local_scope.inspect(statement[2].cast<adobe::array_t>());
            const adobe::array_t& stmt_block = statement[3].cast<adobe::array_t>();
            if (sequence.type_info() == adobe::type_info<adobe::array_t>()) {
                if (loop_var_1)
                    throw std::runtime_error("Two loop variables passed to a for loop over an array");
                const adobe::array_t& array = sequence.cast<adobe::array_t>();
                for (adobe::array_t::const_iterator it = array.begin(), end_it = array.end();
                     it != end_it;
                     ++it) {
                    scoped_stack_frame local_stack_frame(stack);
                    set(loop_var_0, *it);
                    adobe::any_regular_t value = exec_block(stmt_block.begin(),
                                                            stmt_block.end(),
                                                            local_scope,
                                                            stack,
                                                            true,
                                                            block_continue,
                                                            block_break,
                                                            function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
            } else if (sequence.type_info() == adobe::type_info<adobe::dictionary_t>()) {
                const adobe::dictionary_t& dictionary =
                    sequence.cast<adobe::dictionary_t>();
                for (adobe::dictionary_t::const_iterator
                         it = dictionary.begin(), end_it = dictionary.end();
                     it != end_it;
                     ++it) {
                    if (loop_var_1) {
                        set(loop_var_0, adobe::any_regular_t(it->first));
                        set(loop_var_1, it->second);
                    } else {
                        adobe::dictionary_t value;
                        value[adobe::static_name_t("key")] =
                            adobe::any_regular_t(it->first);
                        value[adobe::static_name_t("value")] = it->second;
                        set(loop_var_0, adobe::any_regular_t(value));
                    }
                    scoped_stack_frame local_stack_frame(stack);
                    adobe::any_regular_t value = exec_block(stmt_block.begin(),
                                                            stmt_block.end(),
                                                            local_scope,
                                                            stack,
                                                            true,
                                                            block_continue,
                                                            block_break,
                                                            function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
            }
        } else if (op == adobe::complex_for_k) {
            scoped_stack_frame local_stack_frame(stack);
            const adobe::array_t& vars_array = statement[0].cast<adobe::array_t>();
            for (std::size_t i = 0; i < vars_array.size(); i += 3) {
                adobe::name_t var_name = vars_array[i + 0].cast<adobe::name_t>();
                const adobe::array_t& initializer =
                    vars_array[i + 1].cast<adobe::array_t>();
                declare(var_name, initializer, false);
            }
            const adobe::array_t& condition = statement[1].cast<adobe::array_t>();
            const adobe::array_t& assignments_array =
                statement[2].cast<adobe::array_t>();
            const adobe::array_t& stmt_block = statement[3].cast<adobe::array_t>();
            const adobe::any_regular_t assign_token(adobe::assign_k);
            adobe::any_regular_t condition_result = local_scope.inspect(condition);
            while (condition_result.cast<bool>()) {
                {
                    scoped_stack_frame local_stack_frame(stack);
                    adobe::any_regular_t value = exec_block(stmt_block.begin(),
                                                            stmt_block.end(),
                                                            local_scope,
                                                            stack,
                                                            true,
                                                            block_continue,
                                                            block_break,
                                                            function_done);
                    block_continue = false;
                    if (block_break) {
                        block_break = false;
                        break;
                    }
                    if (function_done)
                        return value;
                }
                adobe::array_t::const_iterator it = assignments_array.begin();
                const adobe::array_t::const_iterator end_it =
                    assignments_array.end();
                while (it != end_it) {
                    adobe::array_t::const_iterator assign_it =
                        std::find(it, end_it, assign_token);
                    ++assign_it;
                    exec_statement(adobe::array_t(it, assign_it),
                                   local_scope,
                                   stack,
                                   false,
                                   block_continue,
                                   block_break,
                                   function_done);
                    assert(!block_continue && !block_break && !function_done);
                    it = assign_it;
                }
                condition_result = local_scope.inspect(condition);
            }
        } else if (op == adobe::continue_k) {
            if (!in_loop)
                throw std::runtime_error("continue statement outside of loop");
            block_continue = true;
        } else if (op == adobe::break_k) {
            if (!in_loop)
                throw std::runtime_error("break statement outside of loop");
            block_break = true;
        } else if (op == adobe::return_k) {
            adobe::any_regular_t value =
                local_scope.inspect(adobe::array_t(statement.begin(),
                                                   statement.end() - 1));
            function_done = true;
            return value;
        }

        return adobe::any_regular_t();
    }

    void common_init(
        const adobe::adam_function_t::array_function_lookup_t& array_function_lookup,
        const adobe::adam_function_t::dictionary_function_lookup_t& dictionary_function_lookup,
        const adobe::adam_function_t::adam_function_lookup_t& adam_function_lookup,
        const variable_getter& get,
        adobe::sheet_t& local_scope
    ) {
        local_scope.machine_m.set_array_function_lookup(array_function_lookup);
        local_scope.machine_m.set_dictionary_function_lookup(dictionary_function_lookup);
        local_scope.machine_m.set_adam_function_lookup(adam_function_lookup);
        local_scope.machine_m.set_variable_lookup(get);
    }

    adobe::any_regular_t common_impl(adobe::sheet_t& local_scope,
                                     std::vector<stack_frame_t>& stack,
                                     const std::vector<adobe::array_t>& statements)
    {
        for (std::vector<adobe::array_t>::const_iterator
                 it = statements.begin(),
                 end_it = statements.end();
             it != end_it;
             ++it) {
            bool function_done = false;
            bool block_continue = false;
            bool block_break = false;
            adobe::any_regular_t value = exec_statement(*it,
                                                        local_scope,
                                                        stack,
                                                        false,
                                                        block_continue,
                                                        block_break,
                                                        function_done);
            if (function_done)
                return value;
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
    std::set<name_t> declared_vars(m_parameter_names.begin(),
                                   m_parameter_names.end());
    for (std::size_t i = 0; i < m_statements.size(); ++i) {
        name_t op_name;
        m_statements[i][m_statements[i].size() - 1].cast(op_name);
        if (op_name == decl_k || op_name == const_decl_k) {
            declared_vars.insert(m_statements[i][0].cast<name_t>());
        } else if (op_name == assign_k) {
            const array_t lvalue = m_statements[i][0].cast<array_t>();
            for (std::size_t j = 0; j < lvalue.size(); ++j) {
                name_t op;
                if (lvalue[j].cast(op) && op == variable_k) {
                    assert(j);
                    name_t var = lvalue[j - 1].cast<name_t>();
                    if (declared_vars.find(var) == declared_vars.end()) {
                        throw_parser_exception(
                            make_string(
                                m_function_name.c_str(),
                                "(): Assignment to unknown variable ",
                                var.c_str()
                            ).c_str(),
                            line_position_t()
                        );
                    }
                }
            }
        }
        for (array_t::const_iterator
                 it = m_statements[i].begin(),
                 end_it = m_statements[i].end();
             it != end_it;
             ++it) {
            if (it->type_info() == type_info<name_t>()) {
                array_t::const_iterator next_it = boost::next(it);
                name_t name;
                if (next_it != end_it && next_it->cast(name) && name == variable_k) {
                    name_t var = it->cast<name_t>();
                    if (declared_vars.find(var) == declared_vars.end()) {
                        throw_parser_exception(
                            make_string(
                                m_function_name.c_str(),
                                "(): Use of unknown variable ",
                                var.c_str()
                            ).c_str(),
                            line_position_t()
                        );
                    }
                }
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
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const array_t& parameters
) const
{
    sheet_t local_scope;
    std::vector<stack_frame_t> stack;

    common_init(array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                variable_getter(local_scope, stack),
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

    return common_impl(local_scope, stack, m_statements);
}

any_regular_t adam_function_t::operator()(
    const array_function_lookup_t& array_function_lookup,
    const dictionary_function_lookup_t& dictionary_function_lookup,
    const adam_function_lookup_t& adam_function_lookup,
    const dictionary_t& parameters
) const
{
    sheet_t local_scope;
    std::vector<stack_frame_t> stack;

    common_init(array_function_lookup,
                dictionary_function_lookup,
                adam_function_lookup,
                variable_getter(local_scope, stack),
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

    return common_impl(local_scope, stack, m_statements);
}

}
