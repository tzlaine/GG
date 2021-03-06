/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#define ADOBE_DLL_SAFE 0

#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>

#include <GG/GUI.h>


/**************************************************************************************************/

namespace {

/**************************************************************************************************/

void throw_function_not_defined(adobe::name_t function_name)
{
    throw std::logic_error(
        adobe::make_string("Function \'", function_name.c_str(), "\' not defined."));
}

adobe::any_regular_t contributing_adaptor(const adobe::sheet_t& sheet, const adobe::array_t& array)
{
    if (array.size() != 1) throw std::logic_error("contributing takes a single argument.");
    return adobe::any_regular_t(sheet.contributing_to_cell(array[0].cast<adobe::name_t>()));
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

any_regular_t vm_dictionary_image_proc(const dictionary_t& named_argument_set)
{
    if (named_argument_set.empty())
        return any_regular_t(empty_t());

    std::string                    filename;
    boost::shared_ptr<GG::Texture> the_image;

    get_value(named_argument_set, key_name, filename);

    if (!filename.empty()) {
        try {
            the_image = GG::GUI::GetGUI()->GetTexture(filename);
            the_image->SetFilters(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
        } catch (...) {
            return any_regular_t(empty_t());
        }
    }

    return any_regular_t(the_image);
}

/**************************************************************************************************/

any_regular_t vm_array_image_proc(const array_t& argument_set)
{
    if (argument_set.empty())
        return any_regular_t(empty_t());

    std::string                    filename;
    boost::shared_ptr<GG::Texture> the_image;

    argument_set[0].cast(filename);

    if (!filename.empty()) {
        try {
            the_image = GG::GUI::GetGUI()->GetTexture(filename);
            the_image->SetFilters(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
        } catch (...) {
            return any_regular_t(empty_t());
        }
    }

    return any_regular_t(the_image);
}

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/

std::ostream& operator<<(std::ostream& s, const boost::shared_ptr<GG::Texture>& /*image*/)
{
    s << "FIXME: " << __FILE__ << __LINE__ << ": "
      << "operator<<(std::ostream& s, const boost::shared_ptr<GG::Texture>& image)"
      << std::endl;

    return s;
}

/**************************************************************************************************/

bool asl_standard_dictionary_function_lookup(name_t              function_name,
                                             const dictionary_t& named_argument_set,
                                             any_regular_t& result)
{
    if (function_name == static_name_t("image"))
    {
        result = implementation::vm_dictionary_image_proc(named_argument_set);
        return true;
    }
    else
    {
        return false;
    }

    result = any_regular_t(empty_t());
    return false;
}

/**************************************************************************************************/

bool asl_standard_array_function_lookup(name_t         function_name,
                                        const array_t& argument_set,
                                        any_regular_t& result)
{
    if (function_name == static_name_t("image"))
    {
        result = implementation::vm_array_image_proc(argument_set);
        return true;
    }
    else
    {
        return false;
    }

    result = any_regular_t(empty_t());
    return false;
}

/**************************************************************************************************/
#if 0
#pragma mark -
#endif
/**************************************************************************************************/

vm_lookup_t::vm_lookup_t()
{
    insert_dictionary_function(adobe::static_name_t("image"), &implementation::vm_dictionary_image_proc);
    insert_array_function(adobe::static_name_t("image"), &implementation::vm_array_image_proc);
}

/**************************************************************************************************/

void vm_lookup_t::attach_to(sheet_t& sheet)
{
    insert_array_function(adobe::static_name_t("contributing"),
        boost::bind(&contributing_adaptor, boost::cref(sheet), _1));
        
    variable_lookup_m = boost::bind(&sheet_t::get, &sheet, _1);
}

/**************************************************************************************************/

void vm_lookup_t::insert_dictionary_function(name_t name, const dictionary_function_t& proc)
{ dmap_m.insert(dictionary_function_map_t::value_type(name, proc)); }

/**************************************************************************************************/

void vm_lookup_t::insert_array_function(name_t name, const array_function_t& proc)
{ amap_m.insert(array_function_map_t::value_type(name, proc)); }

/**************************************************************************************************/

void vm_lookup_t::add_adam_functions(const adam_function_map_t& map)
{ adam_map_m = map; }

/**************************************************************************************************/

void vm_lookup_t::attach_to(virtual_machine_t& vm)
{
    vm.set_dictionary_function_lookup(boost::bind(&vm_lookup_t::dproc, boost::cref(*this), _1, _2, _3));
    vm.set_array_function_lookup(boost::bind(&vm_lookup_t::aproc, boost::cref(*this), _1, _2, _3));
    vm.set_variable_lookup(boost::ref(variable_lookup_m));
    vm.set_adam_function_lookup(boost::bind(&vm_lookup_t::adamproc, boost::cref(*this), _1));
}

/**************************************************************************************************/

bool vm_lookup_t::dproc(name_t name, const dictionary_t& argument_set, any_regular_t& result) const
{
    dictionary_function_map_t::const_iterator found(dmap_m.find(name));

    if (found != dmap_m.end()) {
        result = found->second(argument_set);
        return true;
    } else {
        return false;
    }
}

/**************************************************************************************************/

bool vm_lookup_t::aproc(name_t name, const array_t& argument_set, any_regular_t& result) const
{
    array_function_map_t::const_iterator found(amap_m.find(name));

    if (found != amap_m.end()) {
        result = found->second(argument_set);
        return true;
    } else {
        return false;
    }
}

/**************************************************************************************************/

const adam_function_t& vm_lookup_t::adamproc(name_t name) const
{
    adam_function_map_t::const_iterator found(adam_map_m.find(name));

    if (found != adam_map_m.end())
        return found->second;

    throw std::runtime_error(adobe::make_string("Adam function ", name.c_str(), " not found."));
}

/**************************************************************************************************/

const vm_lookup_t::dictionary_function_map_t& vm_lookup_t::dictionary_functions() const
{ return dmap_m; }

/**************************************************************************************************/

const vm_lookup_t::array_function_map_t& vm_lookup_t::array_functions() const
{ return amap_m; }

/**************************************************************************************************/

const vm_lookup_t::adam_function_map_t& vm_lookup_t::adam_functions() const
{ return adam_map_m; }

/**************************************************************************************************/

const vm_lookup_t::variable_function_t& vm_lookup_t::var_function() const
{ return variable_lookup_m; }

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
