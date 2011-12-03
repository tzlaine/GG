/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/keyboard.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

keyboard_t::iterator keyboard_t::insert(iterator parent,
                                        const adobe::poly_key_handler_t& element)
{
    parent = parent == iterator() ?
        forest_m.end() :     // last child of root, or
        trailing_of(parent); // the last child of the parent

    return forest_m.insert(parent, element);
}

/*************************************************************************************************/

void keyboard_t::erase(iterator position)
{
    assert(forest_m.size());

    forest_m.erase(position);
}

/*************************************************************************************************/

bool keyboard_t::dispatch(key_type                    virtual_key,
                          bool                        pressed,
                          adobe::modifiers_t          modifiers)
{
    iterator parent(forest_m.begin());

    typedef keyboard_forest_t::postorder_iterator postorder_iterator;

    for (postorder_iterator iter(leading_of(parent)), last(++trailing_of(parent));
         iter != last;
         ++iter) {
        if (iter->handle_key(virtual_key, pressed, modifiers))
            return true;
    }

    return false;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
