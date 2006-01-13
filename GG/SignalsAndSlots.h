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

#ifndef _GGSignalsAndSlots_h_
#define _GGSignalsAndSlots_h_

#ifndef BOOST_BIND_HPP_INCLUDED
#include <boost/bind.hpp>
#endif

#ifndef BOOST_PREPROCESSOR_CAT_HPP
#include <boost/preprocessor/cat.hpp>
#endif

#include "GGSignal0.h"
#include "GGSignal1.h"
#include "GGSignal2.h"
#include "GGSignal3.h"
#include "GGSignal4.h"
#include "GGSignal5.h"
#include "GGSignal6.h"
#include "GGSignal7.h"
#include "GGSignal8.h"

/** \file GGSignalsAndSlots.h
    Contains the Connect() functions, which simplify the connection of boost signals and slots. */

namespace GG {

/** connects a signal to a slot functor of the same signature, putting \a _slot in slot group 0, at position \a at
    within group 0.  Slot call groups are called in ascending order. */
template <class SigT> inline 
boost::signals::connection 
Connect(SigT& sig, const typename SigT::slot_type& _slot, boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(_slot, at);
}

/** connects a signal to a slot functor of the same signature, putting \a _slot in slot group \a grp, at position \a at
    within group \a grp.  Slot call groups are called in ascending order. */
template <class SigT> inline 
boost::signals::connection 
Connect(SigT& sig, const typename SigT::slot_type& _slot, int grp, boost::signals::connect_position at = boost::signals::at_back)
{
    return sig.connect(grp, _slot, at);
}

} // namespace GG


#endif // _GGSignalsAndSlots_h_


