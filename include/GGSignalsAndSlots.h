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

namespace GG {

/** connects a signal to a slot functor of the same type, putting \a _slot in slot group \a grp.  Slot call groups are 
    called in ascending order; slots within each group are called in an undefined order.*/
template <class SigT> inline 
boost::signals::connection 
Connect(SigT& sig, const typename SigT::slot_type& _slot, int grp = 0) {return sig.connect(grp, _slot);}


/** connects a signal to a member function of a specific object that has the same function signature, putting \a R 
    in slot group \a grp.  Slot call groups are called in ascending order; slots within each group are called in an 
    undefined order. Overloads exist for const- and non-const- versions with 0 to 8 arguments.  8 was picked as the max 
    simply because boost::bind only supports up to 8 args as of version 1.29.0.*/
template <class R, class T> inline
boost::signals::connection 
Connect(boost::signal<R ()>& sig, 
        R (T::* fn) (), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj));
}

template <class R, class T> inline
boost::signals::connection 
Connect(boost::signal<R ()>& sig, 
        R (T::* fn) () const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj));
}

template <class R, class T, class A1> inline
boost::signals::connection 
Connect(boost::signal<R (A1)>& sig, 
        R (T::* fn) (A1), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1));
}

template <class R, class T, class A1> inline
boost::signals::connection 
Connect(boost::signal<R (A1)>& sig,
        R (T::* fn) (A1) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1));
}

template <class R, class T, class A1, class A2> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2)>& sig, 
        R (T::* fn) (A1, A2), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2));
}

template <class R, class T, class A1, class A2> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2)>& sig, 
        R (T::* fn) (A1, A2) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2));
}

template <class R, class T, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3)>& sig, 
        R (T::* fn) (A1, A2, A3), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3));
}

template <class R, class T, class A1, class A2, class A3> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3)>& sig, 
        R (T::* fn) (A1, A2, A3) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3));
}

template <class R, class T, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4)>& sig, 
        R (T::* fn) (A1, A2, A3, A4), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4));
}

template <class R, class T, class A1, class A2, class A3, class A4> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4)>& sig, 
        R (T::* fn) (A1, A2, A3, A4) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6, A7)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6, A7), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6, A7)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6, A7) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6, A7, A8), 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8));
}

template <class R, class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8> inline
boost::signals::connection 
Connect(boost::signal<R (A1, A2, A3, A4, A5, A6, A7, A8)>& sig, 
        R (T::* fn) (A1, A2, A3, A4, A5, A6, A7, A8) const, 
        T* obj, 
        int grp = 0)
{
    return sig.connect(grp, boost::bind(fn, obj, _1, _2, _3, _4, _5, _6, _7, _8));
}

} // namespace GG


#endif // _GGSignalsAndSlots_h_

