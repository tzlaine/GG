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

#ifndef _GGZList_h_
#define _GGZList_h_

#ifndef _GGBase_h_
#include "GGBase.h"
#endif

namespace GG {

class Wnd;

/** a Z-ordering (depth-ordering) of the windows in the GUI. Windows being moved up, inserted, or added to the top of the list 
   are checked against other windows at the insertion point and after; if any of these windows are modal or on-top windows, 
   the inserted window is placed after them if it is not also modal or on-top. Z-values decrease into the screen.  Windows 
   in the z-list are kept in front-to-back order.  No windows may share the same z-value.  Add, Remove, MoveUp, and MoveDown 
   all also add/remove/move all descendent windows.*/
class ZList : public list<Wnd*>
{
public:
   GGEXCEPTION(ZListException);   ///< exception class \see GG::GGEXCEPTION

   /** \name Accessors */ //@{
   Wnd* Pick(const Pt& pt, Wnd* modal) const;  ///< returns pointer to the window under the point pt; constrains pick to modal if nonzero
   //@}
   
   /** \name Mutators */ //@{
   /** Add() places \a wnd in the list in front of the first entry with z-value <= wnd->ZOrder(), or at the end of the list, 
      whichever comes first. If wnd->ZOrder() == 0, Add() inserts \a wnd at the front of the list, and updates \a 
      wnd's z-value. */
   void Add(Wnd* wnd);
   bool Remove(Wnd* wnd);   ///< removes \a wnd from z-order
   bool MoveUp(Wnd* wnd);   ///< moves \a wnd from its current position to the beginning of list; updates wnd's z-value
   bool MoveDown(Wnd* wnd); ///< moves \a wnd from its current position to the end of list; updates wnd's z-value
   //@}

private:
   Wnd*     PickWithinWindow(const Pt& pt, Wnd* wnd) const; ///< returns pointer to the window under the point pt; constrains pick to wnd and its decendents
   bool     NeedsRealignment() const;     ///< determines whether list needs rearranging
   void     Realign();                    ///< rearranges z-values of windows in list to compact range of z-values and maintain DESIRED_GAP_SIZE separation
   iterator FirstNonOnTop();              ///< returns iterator to first window in list that is non-on-top (returns end() if none found)
};

} // namespace GG

#endif // _GGZList_h_


