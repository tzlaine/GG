#include <GG/Timer.h>

#include <GG/GUI.h>
#include <GG/Wnd.h>

#include <map>


using namespace GG;

Timer::Timer() :
    m_interval(0),
    m_running(true),
    m_last_fire(0)
{ GUI::GetGUI()->RegisterTimer(*this); }

Timer::Timer(int interval, int start_time/* = 0*/) :
    m_interval(interval),
    m_running(true),
    m_last_fire(start_time ? start_time : GUI::GetGUI()->Ticks())
{ GUI::GetGUI()->RegisterTimer(*this); }

Timer::~Timer()
{ GUI::GetGUI()->RemoveTimer(*this); }

bool Timer::Connected() const
{ return !m_wnds.empty(); }

int Timer::Interval() const
{ return m_interval; }

bool Timer::Running() const
{ return m_running; }

bool Timer::ShouldFire(int ticks) const
{ return m_running && !m_wnds.empty() && m_interval < ticks - m_last_fire; }

const std::set<Wnd*>& Timer::Wnds() const
{ return m_wnds; }

void Timer::Reset(int start_time/* = 0*/)
{ m_last_fire = start_time ? start_time : GUI::GetGUI()->Ticks(); }

void Timer::SetInterval(int interval)
{ m_interval = interval; }

void Timer::Connect(Wnd* wnd)
{ m_wnds.insert(wnd); }

void Timer::Disconnect(Wnd* wnd)
{ m_wnds.erase(wnd); }

void Timer::Start()
{ m_running = true; }

void Timer::Stop()
{ m_running = false; }
