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

#include "GGZList.h"
#include "GGWnd.h"

namespace GG {

namespace {
const int DESIRED_GAP_SIZE = 5;        // leaves room for 4-deep nested child windows (and that should be plenty)
const int DESIRED_LOWEST_Z = 1 << 30;  // start z-values at 1/4 of 32-bit int's range (2^32 / 4 = 2^30)
const int MIN_Z = 1 << 29;             // set low end of z-value range at 1/8 of 32-bit int's range (2^32 / 8 = 2^29)
const int MAX_Z = 7 * MIN_Z;           // set high end of z-value range at 7/8 of 32-bit int's range (7 * 2^32 / 8 = 7 * 2^29)
const int MAX_AVG_GAP_SIZE = 10;       // if things get too spread out..
const int MIN_AVG_GAP_SIZE = 2;        // ..or too packed, compresion may be needed
const int MAX_SPAN = 1 << 31;          // windows should be laid out over no more than 1/2 of possible z-value range (2^32 / 2 = 2^31)
};

///////////////////////////////////////
// class GG::ZList
///////////////////////////////////////
Wnd* ZList::Pick(const Pt& pt, Wnd* modal) const
{
    Wnd* retval = 0;
    if (modal) { // if a modal window is active, only look there
        retval = modal->InWindow(pt) ? PickWithinWindow(pt, modal) : 0;
    } else { // otherwise, look in the z-list
        const_iterator end_it = end();
        for (const_iterator it = begin(); it != end_it; ++it)
            if ((*it)->InWindow(pt)) {
                retval = PickWithinWindow(pt, *it);
                break;
            }
    }
    return retval;
}

void ZList::Add(Wnd* wnd)
{
    // add wnd to the end of the list...
    if (empty()) { // list empty
        wnd->m_zorder = DESIRED_LOWEST_Z; // by default, add first element at DESIRED_LOWEST_Z
        insert(begin(), wnd);
    } else { // list not empty
        wnd->m_zorder = back()->m_zorder - DESIRED_GAP_SIZE - 1;
        insert(end(), wnd);
    }
    // then move it up to its proper place
    MoveUp(wnd);
    if (NeedsRealignment()) Realign();
}

bool ZList::Remove(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) {
        erase(it);
        retval = true;
    }
    if (NeedsRealignment()) Realign();
    return retval;
}

bool ZList::MoveUp(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) { // note that this also implies !empty()..
        int top_z = front()->m_zorder; // ..so this is okay to do without checking
        if (!front()->OnTop() || wnd->OnTop()) { // if there are no on-top windows, or wnd is an on-top window..
            (*it)->m_zorder = top_z + DESIRED_GAP_SIZE + 1; // ..just slap wnd on top of the topmost element
            splice(begin(), *this, it);
        } else { // front()->OnTop() && !wnd->OnTop(), so only move wnd up to just below the bottom of the on-top range
            iterator first_non_ontop_it = FirstNonOnTop();
            int prev_z = (*--first_non_ontop_it)->m_zorder;
            int curr_z = (*++first_non_ontop_it)->m_zorder;
            if (prev_z - curr_z - 1 >= 3) { // if there's at least 3 positions in between..
                (*it)->m_zorder = (*first_non_ontop_it)->m_zorder + (prev_z - curr_z - 1) / 2; // ..just stick wnd in the middle
                splice(first_non_ontop_it, *this, it);
            } else { // make room by bumping up all the on-top windows
                iterator it2 = first_non_ontop_it;
                (*--it2)->m_zorder += 2 * (DESIRED_GAP_SIZE + 1); // double the gap before first_non_ontop_it, to leave the right gap on either side of wnd
                while (it2 != begin()) {
                    --it2;
                    (*it2)->m_zorder += DESIRED_GAP_SIZE + 1;
                }
                (*it)->m_zorder = (*first_non_ontop_it)->m_zorder + DESIRED_GAP_SIZE + 1;
                splice(first_non_ontop_it, *this, it);
            }
        }
        retval = true;
    }
    if (NeedsRealignment()) Realign();
    return retval;
}

bool ZList::MoveDown(Wnd* wnd)
{
    bool retval = false;
    iterator it = std::find(begin(), end(), wnd);
    if (it != end()) {
        int bottom_z = back()->m_zorder;
        if (back()->OnTop() || !wnd->OnTop()) { // if there are only on-top windows, or wnd is not an on-top window..
            (*it)->m_zorder = bottom_z - DESIRED_GAP_SIZE - 1; // ..just put wnd below the bottom element
            splice(end(), *this, it);
        } else { // !back()->OnTop() && wnd->OnTop(), so only move wnd up to just below the bottom of the on-top range
            iterator first_non_ontop_it = FirstNonOnTop();
            int prev_z = (*--first_non_ontop_it)->m_zorder;
            int curr_z = (*++first_non_ontop_it)->m_zorder;
            if (prev_z - curr_z - 1 >= 3) { // if there's at least 3 positions in between..
                (*it)->m_zorder = (*first_non_ontop_it)->m_zorder + (prev_z - curr_z - 1) / 2; // ..just stick wnd in the middle
                splice(first_non_ontop_it, *this, it);
            } else { // make room by bumping up all the on-top windows
                iterator it2 = first_non_ontop_it;
                (*--it2)->m_zorder += 2 * (DESIRED_GAP_SIZE + 1); // double the gap before first_non_ontop_it, to leave the right gap on either side of wnd
                while (it2 != begin()) {
                    --it2;
                    (*it2)->m_zorder += DESIRED_GAP_SIZE + 1;
                }
                (*it)->m_zorder = (*first_non_ontop_it)->m_zorder + DESIRED_GAP_SIZE + 1;
                splice(first_non_ontop_it, *this, it);
            }
        }
        retval = true;
    }
    if (NeedsRealignment()) Realign();
    return retval;
}

Wnd* ZList::PickWithinWindow(const Pt& pt, Wnd* wnd) const
{
    Wnd* retval = wnd->Clickable() ? wnd : 0; // if wnd is clickable, return it if no child windows also catch pt
    // look through all the children of wnd, and determine whether pt lies in any of them (or their children)
    std::list<Wnd*>::iterator end_it = wnd->m_children.end();
    for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != end_it; ++it) {
        if ((*it)->Clickable() && (*it)->InWindow(pt)) {
            retval = PickWithinWindow(pt, *it);
            break;
        }
    }
    return retval;
}

bool ZList::NeedsRealignment() const
{
    bool retval = false;
    if (unsigned int sz = size()) {
        int front_z = front()->m_zorder;
        int back_z = back()->m_zorder;
        int range = front_z - back_z + 1;
        int empty_slots = range - sz;
        double avg_gap = empty_slots / double(sz - 1); // empty slots over the number of spaces in between the elements
        bool max_span_impossible = double(DESIRED_GAP_SIZE) * double(sz) > MAX_SPAN; // done with floats to avoid integer overflow
        retval = ((range > MAX_SPAN && !max_span_impossible) ||
                  avg_gap > MAX_AVG_GAP_SIZE ||
                  avg_gap < MIN_AVG_GAP_SIZE ||
                  front_z > MAX_Z ||
                  back_z < MIN_Z);
    }
    return retval;
}

void ZList::Realign()
{
    int z = DESIRED_LOWEST_Z; // z-value to place next element at
    for (reverse_iterator it = rbegin(); it != rend(); ++it) {
        (*it)->m_zorder = z;
        z += DESIRED_GAP_SIZE + 1;
    }
}

ZList::iterator ZList::FirstNonOnTop()
{
    iterator retval = begin();
    for (; retval != end(); ++retval)
        if (!(*retval)->OnTop())
            break;
    return retval;
}

} // namespace GG

