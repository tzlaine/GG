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

#include <GG/StatementWriter.h>

#include <GG/ExpressionWriter.h>
#include <GG/adobe/array.hpp>
#include <GG/adobe/implementation/token.hpp>


using namespace GG;

namespace {

    void WriteStatementImpl(adobe::array_t::const_reverse_iterator& it,
                            adobe::array_t::const_reverse_iterator end_it,
                            std::string& output,
                            unsigned int indent)
    {
        if (it == end_it)
            return;

        adobe::name_t op;
        it->cast(op);
        assert(op);

        output += std::string(indent * 4, ' ');

        if (op == adobe::const_decl_k || op == adobe::decl_k) {
            ++it;
            if (op == adobe::const_decl_k)
                output += "constant ";
            std::string initializer = WriteExpression(it->cast<adobe::array_t>());
            ++it;
            output += it->cast<adobe::name_t>().c_str();
            ++it;
            if (initializer != "")
                output += ": " + initializer;
            output += ';';
        } else if (op == adobe::assign_k) {
            output += end_it.base()->cast<adobe::name_t>().c_str();
            output += " = ";
            adobe::array_t::const_reverse_iterator expr_end =
                boost::prior(end_it, 2);;
            adobe::array_t expr(expr_end.base(), it.base());
            output += WriteExpression(expr);
            output += ';';
        } else if (op == adobe::return_k) {
            ++it;
            adobe::array_t expr(end_it.base(), it.base());
            output += "return " + WriteExpression(expr);
            output += ';';
        } else if (op == adobe::stmt_ifelse_k) {
            ++it;

            std::string else_case;
            bool else_case_multiline = false;
            bool else_if = false;
            bool else_if_if_multiline = false;
            {
                const adobe::array_t& block = it->cast<adobe::array_t>();
                adobe::name_t first_op;
                if (block.size() == 1u) {
                    const adobe::array_t& block_first_stmt =
                        block[0].cast<adobe::array_t>();
                    else_if =
                        boost::prior(block_first_stmt.end())->cast(first_op) &&
                        first_op == adobe::stmt_ifelse_k;
                    else_if_if_multiline = 1u < block_first_stmt.size();
                }
                else_case_multiline = 1u < block.size();
                for (adobe::array_t::const_iterator block_it = block.begin();
                     block_it != block.end();
                     ++block_it) {
                    if (block_it != block.begin())
                        else_case += '\n';
                    else_case += WriteStatement(block_it->cast<adobe::array_t>(),
                                                indent + (else_if ? 0 : 1));
                }
                if (else_if)
                    else_case = else_case.substr(else_case.find_first_not_of(" "), -1);
            }
            ++it;

            std::string if_case;
            bool if_case_multiline = false;
            {
                const adobe::array_t& block = it->cast<adobe::array_t>();
                if_case_multiline = 1u < block.size();
                for (adobe::array_t::const_iterator block_it = block.begin();
                     block_it != block.end();
                     ++block_it) {
                    if (block_it != block.begin())
                        if_case += '\n';
                    if_case += WriteStatement(block_it->cast<adobe::array_t>(), indent + 1);
                }
            }
            ++it;

            std::string condition = WriteExpression(it->cast<adobe::array_t>());
            ++it;

            const bool use_braces =
                if_case.empty() && else_case.empty() || if_case_multiline || else_case_multiline;

            output += "if ( ";
            output += condition;
            output += ')';
            if (use_braces)
                output += " {";
            output += '\n';
            output += if_case;
            if (use_braces) {
                if (!if_case.empty())
                    output += "\n";
                output += std::string(indent * 4, ' ');
                output += "}";
            }
            if (!else_case.empty()) {
                if (use_braces) {
                    output += ' ';
                } else {
                    output += '\n';
                    output += std::string(indent * 4, ' ');
                }
                output += "else";
                if (else_if) {
                    output += ' ';
                } else {
                    if (use_braces)
                        output += " {";
                    output += '\n';
                }
                output += else_case;
                if (use_braces && !else_if_if_multiline) {
                    if (!else_case.empty())
                        output += "\n";
                    output += std::string(indent * 4, ' ');
                    output += "}";
                }
            }
        } else {
            assert(!"Unknown statement type");
        }
    }

}

std::string GG::WriteStatement(const adobe::array_t& statement, unsigned int indent/* = 0*/)
{
    std::string retval;
    adobe::array_t::const_reverse_iterator it = statement.rbegin();
    adobe::array_t::const_reverse_iterator end_it = statement.rend();
    WriteStatementImpl(it, end_it, retval, indent);
    return retval;
}
