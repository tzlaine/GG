/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003 T. Zachary Laine

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
   whatwasthataddress@hotmail.com */

/* $Header$ */

#include "XMLDoc.h"

namespace GG {

namespace {
const string INDENT_STR = "  "; // indents are 2 spaces
bool found_first_quote = false;
bool found_last_quote = false;
}

XMLDoc*              XMLDoc::s_curr_parsing_doc = 0;
vector<XMLElement*>  XMLDoc::s_element_stack;

////////////////////////////////////////////////
// GG::XMLElement
////////////////////////////////////////////////
bool XMLElement::ContainsChild(const string& child) const
{
   return ChildIndex(child) != -1;
}

bool XMLElement::ContainsAttribute(const string& attrib) const
{
   return m_attributes.find(attrib) != m_attributes.end();
}

int XMLElement::ChildIndex(const string& child) const
{
   int retval = -1;
   for (unsigned int i = 0; i < m_children.size(); ++i) {
      if (m_children[i].m_tag == child) {
         retval = i;
         break;
      }
   }
   return retval;
}

const XMLElement& XMLElement::Child(const string& child) const 
{
   unsigned int i = 0; 
   for (; i < m_children.size(); ++i) 
      if (m_children[i].m_tag == child) 
         break; 
   return m_children[i];
}

const string& XMLElement::Attribute(const string& attrib) const
{
   static const string empty_str("");
   map<string, string>::const_iterator it = m_attributes.find(attrib);
   if (it != m_attributes.end())
      return it->second;
   else
      return empty_str;
}

ostream& XMLElement::WriteElement(ostream& os, int indent/* = 0*/, bool whitespace/* = true*/) const
{
   if (whitespace) 
      for (int i = 0; i < indent; ++i)
         os << INDENT_STR;
   os << '<' << m_tag;
   for (std::map<string, string>::const_iterator it = m_attributes.begin(); it != m_attributes.end(); ++it)
      os << ' ' << it->first << "=\"" << it->second << "\"";
   if (m_children.empty() && m_text.empty() && !m_root) {
      os << "/>";
      if (whitespace) 
         os << "\n";
   } else {
      os << ">";
      if (!m_text.empty())
         os << "\"<![CDATA[" << m_text << "]]>\"";
      if (whitespace && !m_children.empty())
         os << "\n";
      for (unsigned int i = 0; i < m_children.size(); ++i)
         m_children[i].WriteElement(os, indent + 1, whitespace);
      if (whitespace && !m_children.empty()) {
         for (int i = 0; i < indent; ++i) {
            os << INDENT_STR;
         }
      }
      os << "</" << m_tag << ">";
      if (whitespace) os << "\n";
   }
   return os;
}

XMLElement& XMLElement::Child(const string& child) 
{
   unsigned int i = 0; 
   for (; i < m_children.size(); ++i) 
      if (m_children[i].m_tag == child)
         break; 
   return m_children[i];
}


////////////////////////////////////////////////
// GG::XMLDoc
////////////////////////////////////////////////
ostream& XMLDoc::WriteDoc(ostream& os, bool whitespace/* = true*/) const
{
   os << "<?xml version=\"1.0\"?>";
   if (whitespace) os << "\n";
   return root_node.WriteElement(os, 0, whitespace);
}

istream& XMLDoc::ReadDoc(istream& is)
{
   root_node = XMLElement(); // clear doc contents
   s_element_stack.clear();  // clear this to start a fresh read
   s_curr_parsing_doc = this;  // tell ProcessElement where to add elements
   XML_Parser p = XML_ParserCreate(0);
   XML_SetElementHandler(p, &XMLDoc::BeginElement, &XMLDoc::EndElement);
   XML_SetCharacterDataHandler(p, &XMLDoc::CharacterData);
   string parse_str;
   while (is) {
      string str;
      getline(is, str);
      parse_str += str + '\n';
   }
   XML_Parse(p, parse_str.c_str(), parse_str.size(), true);
   XML_ParserFree(p);
   s_curr_parsing_doc = 0;
   return is;
}

void XMLDoc::BeginElement(void* user_data, const char* name, const char** attrs) 
{
   if (XMLDoc* this_ptr = XMLDoc::s_curr_parsing_doc) {
      if (!s_element_stack.empty()) {
         s_element_stack.back()->AppendChild(name);
         s_element_stack.push_back(&s_element_stack.back()->LastChild());
      } else {
         this_ptr->root_node = XMLElement(name, true);
         s_element_stack.push_back(&this_ptr->root_node);
      }
      while (attrs && *attrs) {
         s_element_stack.back()->SetAttribute(*attrs, *(attrs + 1));
         attrs += 2;
      }
      found_first_quote = found_last_quote = false;
   }
}

void XMLDoc::EndElement(void* user_data, const char* name)
{
   if (!s_element_stack.empty())
      s_element_stack.pop_back();
}

void XMLDoc::CharacterData(void *user_data, const char *s, int len)
{
   if (s_element_stack.back()->Tag() == "ELMTEXT") {
      string str;
      for (int i = 0; i < len; ++i, ++s) {
         char c = *s;
         if (c == '"') {
            if (!found_first_quote) {
               found_first_quote = true;
               continue;
            } else if (found_first_quote && !found_last_quote && i == len - 1) {
               found_last_quote = true;
               break;
            }
         }
         if (found_first_quote)
            str += c;
      }
      s_element_stack.back()->m_text += str;
   }
}

} // namespace GG



