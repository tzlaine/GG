/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
#include <GG/EveLayout.h>

#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/GroupBox.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>
#include <GG/TextControl.h>
#include <GG/ExpressionWriter.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve_evaluate.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <boost/cast.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <numeric>


using namespace GG;

namespace {

    bool IsContainer(adobe::name_t wnd_type)
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

    adobe::any_regular_t VariableLookup(const adobe::basic_sheet_t& layout_sheet, adobe::name_t name)
    {
        static bool s_once = true;
        static adobe::name_t s_reflected_names[] =
            {
                adobe::key_align_left,
                adobe::key_align_right,
                adobe::key_align_top,
                adobe::key_align_bottom,
                adobe::key_align_center,
                adobe::key_align_proportional,
                adobe::key_align_fill,
                adobe::key_place_row,
                adobe::key_place_column,
                adobe::key_place_overlay
            };
        static std::pair<adobe::name_t*, adobe::name_t*> s_reflected_names_range;

        if (s_once) {
            adobe::sort(s_reflected_names);
            s_reflected_names_range.first = boost::begin(s_reflected_names);
            s_reflected_names_range.second = boost::end(s_reflected_names);
            s_once = false;
        }

        adobe::name_t* it =
            std::lower_bound(s_reflected_names_range.first, s_reflected_names_range.second, name);

        if (it != s_reflected_names_range.second && *it == name)
            return adobe::any_regular_t(name);

        return layout_sheet[name];
    }

}

struct EveLayout::Impl
{
    Impl(adobe::sheet_t& sheet) :
        m_sheet(sheet),
        m_current_nested_view(0)
        { m_lookup.attach_to(m_evaluator); }

    void Print(std::ostream& os) const
        {
            os << "layout name_ignored\n"
               << "{\n";
            for (std::size_t i = 0; i < m_added_cells.size(); ++i) {
                const AddedCellSet& cell_set = m_added_cells[i];
                if (i)
                    os << '\n';
                switch (cell_set.m_access) {
                case adobe::eve_callback_suite_t::constant_k: os << "constant:\n"; break;
                case adobe::eve_callback_suite_t::interface_k: os << "interface:\n"; break;
                }
                for (std::size_t j = 0; j < cell_set.m_cells.size(); ++j) {
                    const CellParameters& params = cell_set.m_cells[j];
                    // TODO: print detailed comment
                    os << "    " << params.m_name << " : "
                       << WriteExpression(params.m_initializer) << ";\n";
                    // TODO: print brief comment
                }
            }
            os << "    view ";
            PrintNestedView(os, m_nested_views, 1);
            os << "}\n";
        }

    adobe::eve_callback_suite_t BindCallbacks()
        {
            adobe::eve_callback_suite_t retval;
            m_evaluator.set_variable_lookup(
                boost::bind(VariableLookup, boost::cref(m_layout_sheet), _1)
            );
            retval.add_view_proc_m =
                boost::bind(&EveLayout::Impl::AddView, this, _1, _2, _3, _4, _5, _6);
            retval.add_cell_proc_m =
                boost::bind(&EveLayout::Impl::AddCell, this, _1, _2, _3, _4, _5, _6);
            return retval;
        }

    void AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                 adobe::name_t name,
                 const adobe::line_position_t& position,
                 const adobe::array_t& initializer,
                 const std::string& brief,
                 const std::string& detailed)
        {
            if (m_added_cells.empty() || m_added_cells.back().m_access != type)
                m_added_cells.push_back(AddedCellSet(type));

            m_added_cells.back().m_cells.push_back(
                CellParameters(type, name, position, initializer, brief, detailed)
            );

            m_evaluator.evaluate(initializer);
            adobe::any_regular_t value(m_evaluator.back());
            m_evaluator.pop_back();

            if (type == adobe::eve_callback_suite_t::constant_k)
                m_layout_sheet.add_constant(name, value);
            else if (type == adobe::eve_callback_suite_t::interface_k)
                m_layout_sheet.add_interface(name, value);
            else
                assert(0 && "Cell type not supported");
        }

    boost::any AddView(const boost::any& parent_,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed)
        {
            ViewParameters params(parent_, position, name, parameters, brief, detailed);

            // TODO: Don't do this -- instead, just generate a unique integer
            // to allow the nesting logic to work right.  The actual creation
            // of all the Wnds in the layout will be postponed until the end.
            Wnd* parent = boost::any_cast<Wnd*>(parent_);

            if (!m_current_nested_view) {
                m_nested_views = NestedViews(params, 0);
                m_current_nested_view = &m_nested_views;
            } else {
                while (boost::any_cast<Wnd*>(m_current_nested_view->m_view_parameters.m_parent) !=
                       parent &&
                       m_current_nested_view->m_nested_view_parent) {
                    m_current_nested_view = m_current_nested_view->m_nested_view_parent;
                }
                assert(m_current_nested_view);
                bool is_container = IsContainer(name);
                if (is_container)
                    params.m_parent = ++parent;
                m_current_nested_view->m_children.push_back(NestedViews(params, m_current_nested_view));
                if (is_container)
                    m_current_nested_view = &m_current_nested_view->m_children.back();
            }

            return parent;
        }

    struct CellParameters
    {
        CellParameters(adobe::eve_callback_suite_t::cell_type_t type,
                       adobe::name_t name,
                       const adobe::line_position_t& position,
                       const adobe::array_t& initializer,
                       const std::string& brief,
                       const std::string& detailed) :
            m_type(type),
            m_name(name),
            m_position(position),
            m_initializer(initializer),
            m_brief(brief),
            m_detailed(detailed)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_type;
        adobe::name_t m_name;
        adobe::line_position_t m_position;
        adobe::array_t m_initializer;
        std::string m_brief;
        std::string m_detailed;
    };

    struct AddedCellSet
    {
        AddedCellSet(adobe::eve_callback_suite_t::cell_type_t access) :
            m_access(access)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_access;
        std::vector<CellParameters> m_cells;
    };

    typedef std::vector<AddedCellSet> AddedCells;

    struct ViewParameters
    {
        ViewParameters() {}
        ViewParameters(const boost::any& parent,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed) :
            m_parent(parent),
            m_position(position),
            m_name(name),
            m_parameters(parameters),
            m_brief(brief),
            m_detailed(detailed)
            {}
        boost::any m_parent;
        adobe::line_position_t m_position;
        adobe::name_t m_name;
        adobe::array_t m_parameters;
        std::string m_brief;
        std::string m_detailed;
    };

    struct NestedViews
    {
        NestedViews() :
            m_nested_view_parent(0)
            {}
        NestedViews(const ViewParameters& view_parameters, NestedViews* parent) :
            m_view_parameters(view_parameters),
            m_nested_view_parent(parent)
            {}
        ViewParameters m_view_parameters;
        NestedViews* m_nested_view_parent;
        std::vector<NestedViews> m_children;
    };

    static void PrintNestedView(std::ostream& os, const NestedViews& nested_view, unsigned int indent)
        {
            const ViewParameters& params = nested_view.m_view_parameters;
            // TODO: print detailed comment
            std::string initial_indent(indent * 4, ' ');
            if (indent == 1u)
                initial_indent.clear();
            std::string param_string = WriteExpression(params.m_parameters);
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
                    PrintNestedView(os, nested_view.m_children[i], indent + 1);
                }
                os << std::string(indent * 4, ' ') << "}\n";
            }
        }

    adobe::sheet_t& m_sheet;

    adobe::basic_sheet_t m_layout_sheet;
    adobe::virtual_machine_t m_evaluator;
    adobe::vm_lookup_t m_lookup;

    AddedCells m_added_cells;
    NestedViews m_nested_views;
    NestedViews* m_current_nested_view;
};

////////////////////////////////////////////////////////////
// EveLayout                                              //
////////////////////////////////////////////////////////////

// statics
EveLayout::EveLayout(adobe::sheet_t& sheet) :
    m_impl(new Impl(sheet))
{}

EveLayout::~EveLayout()
{ delete m_impl; }

void EveLayout::Print(std::ostream& os) const
{ m_impl->Print(os); }

adobe::eve_callback_suite_t EveLayout::BindCallbacks()
{ return m_impl->BindCallbacks(); }

void EveLayout::AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                        adobe::name_t name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& initializer,
                        const std::string& brief,
                        const std::string& detailed)
{ m_impl->AddCell(type, name, position, initializer, brief, detailed); }

boost::any EveLayout::AddView(const boost::any& parent,
                              const adobe::line_position_t& position,
                              adobe::name_t name,
                              const adobe::array_t& parameters,
                              const std::string& brief,
                              const std::string& detailed)
{ return m_impl->AddView(parent, position, name, parameters, brief, detailed); }
