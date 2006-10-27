#include <GG/WndEvent.h>


using namespace GG;

///////////////////////////////////////
// class GG::WndEvent
///////////////////////////////////////
WndEvent::WndEvent(EventType type, const Pt& pt, Uint32 key_mods) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_mods(key_mods),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const Pt& move, Uint32 key_mods) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_mods(key_mods),
    m_drag_move(move),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, int move, Uint32 key_mods) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_mods(key_mods),
    m_wheel_move(move),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const std::map<Wnd*, Pt>& drag_drop_wnds, Uint32 key_mods) :
    m_type(type),
    m_point(pt),
    m_key(GGK_UNKNOWN),
    m_key_mods(key_mods),
    m_wheel_move(0),
    m_drag_drop_wnds(drag_drop_wnds),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::WndEvent(EventType type, Key key, Uint32 key_mods) :
    m_type(type),
    m_key(key),
    m_key_mods(key_mods),
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::WndEvent(EventType type, int ticks, Timer* timer) :
    m_type(type),
    m_key(GGK_UNKNOWN),
    m_key_mods(0),
    m_wheel_move(0),
    m_ticks(ticks),
    m_timer(timer)
{}

WndEvent::WndEvent(EventType type) :
    m_type(type),
    m_key(GGK_UNKNOWN),
    m_key_mods(0), 
    m_wheel_move(0),
    m_ticks(0),
    m_timer(0)
{}

WndEvent::EventType WndEvent::Type() const
{ return m_type; }

const Pt& WndEvent::Point() const
{ return m_point; }

Key WndEvent::GetKey() const
{ return m_key; }

Uint32 WndEvent::KeyMods() const
{ return m_key_mods; }

const Pt& WndEvent::DragMove() const
{ return m_drag_move; }

int WndEvent::WheelMove() const
{ return m_wheel_move; }

const std::map<Wnd*, Pt>& WndEvent::DragDropWnds() const
{ return m_drag_drop_wnds; }

int WndEvent::Ticks() const
{ return m_ticks; }

Timer* WndEvent::GetTimer() const
{ return m_timer; }
