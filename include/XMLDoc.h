// -*- C++ -*-
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

#ifndef _XMLDoc_h_
#define _XMLDoc_h_

#include <iostream>       
#include <fstream>       
#include <string>
#include <vector>
#include <map>

namespace GG {

using std::ostream;
using std::istream;
using std::ofstream;
using std::ifstream;
using std::string;
using std::vector;
using std::map;

/** encapsulates an XML element (from a <> tag to a </> tag).  XMLElement is a simplified XML element, 
    consisting only of a tag, a single text string, attributes and child elements.  It is designed to represent 
    C++ objects to allow them to easily be saved to/loaded from disk and serialized to be sent over a network 
    connection. It is *not* designed to represent documents. So this:
    \verbatim
      <burns>Say <quote>Goodnight</quote> Gracie.</burns> 
    \endverbatim
    may not work as you might think. You will end up with both the "Say " and the " Gracie." together.  
    So burns.Text() == "Say  Gracie.". If, however, you wanted to represent an object bar of class foo:
    \verbatim
      class foo
      {
         int ref_ct;
         double data;
      };
    \endverbatim
    you might do something like this: 
    \verbatim
      <bar>
         <foo>
            <ref_ct value="13"/>
            <data value="0.364951"/>
         </foo>
      </bar>
    \endverbatim
    Further, while that example is standard XML, an XMLElement optionally accepts its single text string in quotes, 
    and strips off trailing whitespace, in direct contrary to the XML standard.  
    So "burns" from above is equivalent to:
    \verbatim
      <burns>"Say  Gracie."<quote>Goodnight</quote></burns> 
    \endverbatim
    or:
    \verbatim
      <burns>Say  Gracie.<quote>Goodnight</quote></burns> 
    \endverbatim
    or:
    \verbatim
      <burns>"Say  Gracie."
         <quote>Goodnight</quote>
      </burns> 
    \endverbatim
    or:
    \verbatim
      <burns>Say  Gracie.
         <quote>Goodnight</quote>
      </burns> 
    \endverbatim
    Each of these yields a burns.Text() of "Say  Gracie.".  When an XMLElement is saved, its text is saved within a CDATA section.
    Any string can be put inside one of these quoted text fields, even text that includes an arbitrary number of quotes.  So you 
    can assign any std::string or c-string to an element.  However, when hand-editing an XML file containing such text strings, you
    need to be a bit careful.  The closing quote must be either the last thing other than whitespace.  Adding 
    more than one quoted text string to the XML element, with each string separated by other elements, will result in a 
    single concatenated string, as illustrated above.
    This is not the most time- or space-efficient way to organize object data, but it may just be one of the simplest 
    and most easily read. */
class XMLElement
{
public:
    typedef vector<XMLElement>::iterator         child_iterator;
    typedef vector<XMLElement>::const_iterator   const_child_iterator;
    typedef map<string, string>::iterator        attr_iterator;
    typedef map<string, string>::const_iterator  const_attr_iterator;

    /** \name Structors */ //@{
    XMLElement() : m_root(false) {} ///< default ctor
    XMLElement(const string& tag) : m_tag(tag), m_text(""), m_root(false) {}  ///< ctor that constructs an XMLElement with a tag-name \a tag
    XMLElement(const string& tag, const string& text) : m_tag(tag), m_text(text), m_root(false) {}  ///< ctor that constructs an XMLElement with a tag-name \a tag and text \a text
    //@}

    /** \name Accessors */ //@{
    const string& Tag() const {return m_tag;}                ///< returns the tag-name of the XMLElement
    const string& Text() const {return m_text;}              ///< returns the text of this XMLElement
    int NumChildren() const {return m_children.size();}      ///< returns the number of children in the XMLElement
    int NumAttributes() const {return m_attributes.size();}  ///< returns the number of attributes in the XMLElement
    bool ContainsChild(const string& child) const;           ///< returns true if the element contains a child called \a name
    bool ContainsAttribute(const string& attrib) const;      ///< returns true if the element contains an attribute called \a name
    int  ChildIndex(const string& child) const;              ///< returns the index of the child called \a name, or -1 if not found

    /**  returns the child in the \a idx-th position of the child list of the XMLElement.  \note This function is not 
        range-checked; be sure there are at least idx+1 elements before calling. */
    const XMLElement& Child(unsigned int idx) const {return m_children[idx];}

    /**  returns the child in child list of the XMLElement that has the tag-name \a str.  \note This function is not 
        checked; be sure there is such a child before calling. */   
    const XMLElement& Child(const string& child) const;

    /**  returns the last child in child list of the XMLElement.  \note This function is not checked; be sure there is 
        at least one child before calling. */   
    const XMLElement& LastChild() const {return m_children.back();}

    /** returns the value of the attribute with name \a key, or "" if no such named attribute is found */
    const string& XMLElement::Attribute(const string& attrib) const;

    /** writes the XMLElement to an output stream; returns the stream */
    ostream& WriteElement(ostream& os, int indent = 0, bool whitespace = true) const;

    const_child_iterator child_begin() const {return m_children.begin();}   ///< const_iterator to the first child in the XMLElement
    const_child_iterator child_end() const   {return m_children.end();}     ///< const_iterator to the last + 1 child in the XMLElement
    const_attr_iterator  attr_begin() const  {return m_attributes.begin();} ///< const_iterator to the first attribute in the XMLElement
    const_attr_iterator  attr_end() const    {return m_attributes.end();}   ///< const_iterator to the last + 1 attribute in the XMLElement
    //@}

    /** \name Mutators */ //@{
    /**  returns the child in the \a idx-th position of the child list of the XMLElement.  \note This function is not 
        range-checked; be sure there are at least idx+1 elements before calling. */
    XMLElement& Child(unsigned int idx) {return m_children[idx];}

    /**  returns the child in child list of the XMLElement that has the tag-name \a str.  \note This function is not 
        checked; be sure there is such a child before calling. */   
    XMLElement& Child(const string& child);

    /** sets an attribute \a attrib, whose value is \a val in the XMLElement.  No two attributes can have the same name. */
    void SetAttribute(const string& attrib, const string& val) {m_attributes[attrib] = val;} 

    /** sets the tag to \a tag */
    void SetTag(const string& tag) {m_tag = tag;} 

    /** sets the text to \a text */
    void SetText(const string& text) {m_text = text;} 

    /** removes attribute \a attrib from the XMLElement*/
    void RemoveAttribute(const string& attrib) {m_attributes.erase(attrib);}

    /** removes all attributes from the XMLElement*/
    void RemoveAttributes() {m_attributes.clear();}

    /** adds child XMLElement \a e to the end of the child list of the XMLElement */
    void AppendChild(const XMLElement& e) {m_children.push_back(e);}

    /** creates an empty XMLElement with tag-name \a s, and adds it to the end of the child list of the XMLElement */
    void AppendChild(const string& child) {m_children.push_back(XMLElement(child));}

    /** adds a child \a e in the \a idx-th position of the child list of the XMLElement.  \note This function is not 
   range-checked; be sure there are at least idx+1 elements before calling. */
    void AddChildBefore(const XMLElement& e, unsigned int idx) {m_children.insert(m_children.begin() + idx, e);}

    /** removes the child in the \a idx-th position of the child list of the XMLElement.  \note This function is not 
        range-checked; be sure there are at least idx+1 elements before calling. */
    void RemoveChild(unsigned int idx) {m_children.erase(m_children.begin() + idx);}

    /** removes the child called \a shild from the XMLElement.  \note This function is not value-checked; be sure 
        the desired element exists before calling. */
    void RemoveChild(const string& child) {m_children.erase(m_children.begin() + ChildIndex(child));}

    /** removes all children from the XMLElement*/
    void RemoveChildren() {m_children.clear();}

    child_iterator child_begin()  {return m_children.begin();}     ///< iterator to the first child in the XMLElement
    child_iterator child_end()    {return m_children.end();}       ///< iterator to the last + 1 child in the XMLElement
    attr_iterator  attr_begin()   {return m_attributes.begin();}   ///< iterator to the first attribute in the XMLElement
    attr_iterator  attr_end()     {return m_attributes.end();}     ///< iterator to the last + 1 attribute in the XMLElement
    //@}

private:
    /** ctor that constructs an XMLElement from a tag-name \a t and a bool \a r indicating whether it is the root XMLElement 
        in an XMLDoc document*/
    XMLElement(const string& t, bool r) : m_tag(t), m_root(r) {}

    /** returns the last child in child list of the XMLElement.  \note This function is not checked; be sure there is 
        at least one child before calling. */   
    XMLElement& LastChild() {return m_children.back();}

    string               m_tag;        ///< the tag-name of the XMLElement
    string               m_text;       ///< the text of this XMLElement
    map<string, string>  m_attributes; ///< the attributes of the XMLElement, stored as key-value pairs
    vector<XMLElement>   m_children;   ///< the XMLElement children of this XMLElement

    bool                 m_root;       ///< true if this XMLElement is the root element of an XMLDoc document

    /** allows XMLDoc to create root XMLElements and call the non-const overload of LastChild() */
    friend class XMLDoc;
};

/** encapsulates an entire XML document.  Each XMLDoc is assumed to take up an entire file, and to contain an arbitrary
    number of XMLElements within its root_node member. */
class XMLDoc
{
public:
    /** \name Structors */ //@{
    /** ctor that constructs an empty XML document with a root node with tag-name \a root_tag */
    XMLDoc(const string& root_tag = "GG::XMLDoc") : root_node(XMLElement(root_tag, true)) {}

    /** ctor that constructs an XML document from an input stream \a is */
    XMLDoc(const istream& is) : root_node(XMLElement()) {}
    //@}

    /** \name Accessors */ //@{
    /** writes the XMLDoc to an output stream; returns the stream.  If \a whitespace is false, the document is written 
        without whitespace (one long line with no spaces between XMLElements).  Otherwise, the document is formatted in a 
        more standard human-readable form. */
    ostream& WriteDoc(ostream& os, bool whitespace = true) const;
    //@}

    /** \name Mutators */ //@{
    /** destroys the current contents of the XMLDoc, and replaces tham with the constents of the document in the input 
        stream \a is; returns the stream*/
    istream& ReadDoc(istream& is);
    //@}

    XMLElement root_node;  ///< the single XMLElement in the document, under which all other XMLElement are children

private:
    struct RuleDefiner {RuleDefiner();} s_rule_definer; ///< used to create XML parsing rules at static initialization time

    /** holds the XMLDoc to which the code should add elements */
    static XMLDoc* s_curr_parsing_doc;

    /** maintains the current environment for reading XMLElements (the current enclosing XMLElement) */
    static vector<XMLElement*> s_element_stack;

    // these are used along with the static members above during XML parsing
    static void PushElem(const char* first, const char* last);
    static void PopElem(const char*, const char*);
    static void SetAttributeName(const char* first, const char* last);
    static void AddAttribute(const char* first, const char* last);
    static void AppendToText(const char* first, const char* last);
};

} // namespace GG

#endif // _XMLDoc_h_

