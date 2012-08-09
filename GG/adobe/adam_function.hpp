#include <GG/adobe/adam.hpp>


namespace adobe {

class function
{
public:
    function();
    function(name_t name, sheet_t& property_sheet);

    void add_parameter(name_t parameter);
    void add_statement(const array_t& statement);
    any_regular_t array_function(const array_t& parameters);
    any_regular_t dictionary_function(const dictionary_t& parameters);

private:
    struct statement
    {
        array_t m_statement;
        std::vector<name_t> m_global_variables;
    };

    name_t m_function_name;
    std::vector<name_t> m_parameter_names;
    std::vector<statement> m_statements;
    sheet_t* m_global_scope;

    void add_global_variables(sheet_t& local_scope,
                              const std::vector<name_t>& global_variables);
    any_regular_t common_impl(sheet_t& local_scope);
};

}
