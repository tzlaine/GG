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

/** \file Timer.h
    Contains the Timer class, which allows Wnds to receive regular notifications of the passage of time. */

#ifndef _GG_Timer_h_
#define _GG_Timer_h_

#include <GG/Base.h>

#include <set>


namespace GG {

class Wnd;

/** Timer provides a means for one or more Wnds to receive periodic notifications of the passage of time.  The rate at
    which the Timer fires is not realtime.  That is, there are no guarantees on the interval between firings other than
    that a minimum of Interval() ms will have elapsed.  Note that Timers do not rely on Boost signals to propagate
    firing messages, so a Timers's Wnd connections will survive a serialization-deserialization cycle. */
class GG_API Timer
{
public:
    /** \name Structors */ ///@{
    /** Basic ctor.  Takes an interval and a start time in ms; if the start time is ommitted, the start time will be
        immediate. */
    explicit Timer(int interval, int start_time = 0);

    ~Timer(); ///< Dtor.
    //@}

    /** \name Accessors */ ///@{
    bool Connected() const;             ///< Returns true iff this Timer has Wnds listening to it
    int Interval() const;               ///< Returns the interval in ms between firings of the timer
    bool Running() const;               ///< Returns true iff the timer is operating.  When false, this indicates that no firings will occur until Start() is called.
    bool ShouldFire(int ticks) const;   ///< Returns true iff the timer is connected, running, and the last time it fired is is more than Interval() ms ago.
    const std::set<Wnd*>& Wnds() const; ///< Returns the Wnds connected to this timer.  Note that the GUI will disconnect dying Wnds automatically.
    //@}

    /** \name Mutators */ ///@{
    void Reset(int start_time = 0); ///< Resets the last-firing time of the timer to \a start_time (in ms), or the current time if \a start_time is ommitted.
    void SetInterval(int interval); ///< Sets the interval in ms between firings of the timer
    void Connect(Wnd* wnd);         ///< Connects this timer to \a wnd, meaning that \a wnd will be notified when the timer fires.
    void Disconnect(Wnd* wnd);      ///< Disconnects this timer from \a wnd.
    void Start();                   ///< Starts the timer firing; does not reset the timer.
    void Stop();                    ///< Stops the timer firing until Start() is called.
    //@}

private:
    Timer();
    Timer(const Timer&); // disabled

    std::set<Wnd*> m_wnds;
    int            m_interval;
    bool           m_running;
    int            m_last_fire;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::Timer::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_wnds)
        & BOOST_SERIALIZATION_NVP(m_interval)
        & BOOST_SERIALIZATION_NVP(m_last_fire)
        & BOOST_SERIALIZATION_NVP(m_running);
}

#endif
