/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/basic_sheet.hpp>

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/future/widgets/headers/factory.hpp>

#include <GG/ExpressionWriter.h>

#include <stdexcept>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

bool is_container(adobe::name_t wnd_type)
{
    return
        wnd_type == adobe::name_dialog ||
        wnd_type == adobe::name_group ||
        wnd_type == adobe::name_radio_button_group ||
        wnd_type == adobe::name_tab_group ||
        wnd_type == adobe::name_overlay ||
        wnd_type == adobe::name_panel ||
        wnd_type == adobe::name_row ||
        wnd_type == adobe::name_column;
}

/*************************************************************************************************/

}

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void basic_sheet_t::add_constant(name_t name,
                                 const line_position_t& position,
                                 const array_t& initializer,
                                 const any_regular_t& value)
{
    if (added_elements_m.empty() ||
        added_elements_m.back().which() != 0 ||
        boost::get<added_cell_set_t>(added_elements_m.back()).access_m != access_constant) {
        added_elements_m.push_back(added_cell_set_t(access_constant));
    }
    boost::get<added_cell_set_t>(added_elements_m.back()).added_cells_m.push_back(
        cell_parameters_t(name, position, initializer)
    );

    constant_cell_set_m.push_back(cell_t(value));
    
    const cell_t* cell(&constant_cell_set_m.back());
    
    variable_index_m.insert(std::make_pair(name.c_str(), cell));
}

/*************************************************************************************************/

void basic_sheet_t::add_interface(name_t name,
                                  const line_position_t& position,
                                  const array_t& initializer,
                                  const any_regular_t& value)
{
    if (added_elements_m.empty() ||
        added_elements_m.back().which() != 0 ||
        boost::get<added_cell_set_t>(added_elements_m.back()).access_m != access_interface) {
        added_elements_m.push_back(added_cell_set_t(access_constant));
    }
    boost::get<added_cell_set_t>(added_elements_m.back()).added_cells_m.push_back(
        cell_parameters_t(name, position, initializer)
    );

    interface_cell_set_m.push_back(interface_cell_t(value));
    
    interface_cell_t* cell(&interface_cell_set_m.back());
    
    variable_index_m.insert(std::make_pair(name.c_str(), cell));
    interface_index_m.insert(std::make_pair(name.c_str(), cell));
}

/*************************************************************************************************/

void basic_sheet_t::add_view(const boost::any& parent_,
                             name_t name,
                             const line_position_t& position,
                             const array_t& initializer,
                             const boost::any& view_)
{
    if (added_elements_m.empty() || added_elements_m.back().which() != 1)
        added_elements_m.push_back(added_view_set_t());
    added_view_set_t& added_view_set = boost::get<added_view_set_t>(added_elements_m.back());

    GG::Wnd* parent = boost::any_cast<widget_node_t>(parent_).display_token_m;
    GG::Wnd* view = boost::any_cast<widget_node_t>(view_).display_token_m;

    view_parameters_t params(view, position, name, initializer);

    if (!added_view_set.m_current_nested_view) {
        added_view_set.m_nested_views = nested_views_t(params, 0);
        added_view_set.m_current_nested_view = &added_view_set.m_nested_views;
    } else {
        while (added_view_set.m_current_nested_view->m_view_parameters.m_parent != parent &&
               added_view_set.m_current_nested_view->m_nested_view_parent) {
            added_view_set.m_current_nested_view =
                added_view_set.m_current_nested_view->m_nested_view_parent;
        }
        assert(added_view_set.m_current_nested_view);
        const bool container = is_container(name);
        if (container)
            params.m_parent = parent;
        added_view_set.m_current_nested_view->m_children.push_back(
            nested_views_t(params, added_view_set.m_current_nested_view)
        );
        if (container) {
            added_view_set.m_current_nested_view =
                &added_view_set.m_current_nested_view->m_children.back();
        }
    }
}

/*************************************************************************************************/

std::size_t basic_sheet_t::count_interface(name_t name) const
{ return interface_index_m.count(name.c_str()); }

/*************************************************************************************************/

basic_sheet_t::connection_t basic_sheet_t::monitor_value(name_t name,
        const monitor_value_t& monitor)
{
    interface_cell_t* cell(lookup_interface(name));

    monitor(cell->value_m);
    return (cell->monitor_value_m.connect(monitor));
}

/*************************************************************************************************/

void basic_sheet_t::set(const dictionary_t& cell_set)
{
    dictionary_t::const_iterator iter(cell_set.begin());
    dictionary_t::const_iterator last(cell_set.end());

    for (; iter != last; ++iter)    
        set(iter->first, iter->second);
}

/*************************************************************************************************/

void basic_sheet_t::set(name_t name, const any_regular_t& value)
{
    interface_cell_t* cell(lookup_interface(name));

    cell->value_m = value;

    cell->monitor_value_m(value);
}

/*************************************************************************************************/
    
const any_regular_t& basic_sheet_t::operator[](name_t name) const
{
    variable_index_t::const_iterator iter(variable_index_m.find(name.c_str()));

    if (iter == variable_index_m.end())
    {
        std::string error("basic_sheet_t variable cell does not exist: ");
        error << name.c_str();
        throw std::logic_error(error);
    }
    
    return iter->second->value_m;
}

/*************************************************************************************************/

adobe::dictionary_t basic_sheet_t::contributing() const
{
    interface_index_t::const_iterator iter(interface_index_m.begin());
    interface_index_t::const_iterator last(interface_index_m.end());
    adobe::dictionary_t         result;

    for (; iter != last; ++iter)
        result.insert(std::make_pair(adobe::name_t(iter->first), iter->second->value_m));

    return result;
}

/*************************************************************************************************/

void basic_sheet_t::print(std::ostream& os) const
{
    os << "layout name_ignored\n"
       << "{\n";
    for (std::size_t i = 0; i < added_elements_m.size(); ++i) {
        if (i)
            os << '\n';
        if (added_elements_m[i].which() == 0) {
            const added_cell_set_t& cell_set = boost::get<added_cell_set_t>(added_elements_m[i]);
            switch (cell_set.access_m) {
            case access_constant: os << "constant:\n"; break;
            case access_interface: os << "interface:\n"; break;
            }
            for (std::size_t j = 0; j < cell_set.added_cells_m.size(); ++j) {
                const cell_parameters_t& params = cell_set.added_cells_m[j];
                // TODO: print detailed comment
                os << "    " << params.name_m << " : "
                   << GG::WriteExpression(params.initializer_m) << ";\n";
                // TODO: print brief comment
            }
        } else {
            const added_view_set_t& view_set = boost::get<added_view_set_t>(added_elements_m[i]);
            os << "    view ";
            print_nested_view(os, view_set.m_nested_views, 1);
        }
    }
    os << "}\n";
}

/*************************************************************************************************/

void basic_sheet_t::print_nested_view(std::ostream& os,
                                      const nested_views_t& nested_view,
                                      unsigned int indent) const
{
    const view_parameters_t& params = nested_view.m_view_parameters;
    // TODO: print detailed comment
    std::string initial_indent(indent * 4, ' ');
    if (indent == 1u)
        initial_indent.clear();
    std::string param_string = GG::WriteExpression(params.m_parameters);
    os << initial_indent << params.m_name << '('
       << param_string.substr(1, param_string.size() - 3)
       << ')';
    if (nested_view.m_children.empty()) {
        if (indent  == 1u) {
            os << "\n" // TODO: print brief comment
               << "    {}\n";
        } else {
            os << ";\n"; // TODO: print brief comment
        }
    } else {
        // TODO: print brief comment
        os << '\n'
           << std::string(indent * 4, ' ') << "{\n";
        for (std::size_t i = 0; i < nested_view.m_children.size(); ++i) {
            print_nested_view(os, nested_view.m_children[i], indent + 1);
        }
        os << std::string(indent * 4, ' ') << "}\n";
    }
}

/*************************************************************************************************/

basic_sheet_t::interface_cell_t* basic_sheet_t::lookup_interface(name_t name)
{
    interface_index_t::iterator iter(interface_index_m.find(name.c_str()));

    if (iter == interface_index_m.end())
    {
        std::string error("basic_sheet_t interface cell does not exist: ");
        error << name.c_str();
        throw std::logic_error(error);
    }
    
    return iter->second;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
