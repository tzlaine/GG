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

#include "GGWnd.h"

#include <GGApp.h>
#include <GGDrawUtil.h>
#include <GGEventPump.h>
#include <XMLValidators.h>


using namespace GG;

namespace {
    // an EventPump that terminates when its m_done reference member is true
    class ModalEventPump : public EventPump
    {
    public:
        ModalEventPump(const bool& done) : m_done(done) {}
        virtual void operator()()
        {
            App* app = App::GetApp();
            EventPumpState& state = State();
            while (!m_done) {
                app->HandleSystemEvents(state.last_mouse_event_time);
                LoopBody(app, state, true, true);
            }
        }
    private:
        const bool& m_done;
    };
}

///////////////////////////////////////
// class GG::Wnd
///////////////////////////////////////
Wnd::Wnd() :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_clip_children(false),
    m_max_size(1 << 30, 1 << 30),
    m_flags(0)
{
}

Wnd::Wnd(int x, int y, int w, int h, Uint32 flags) :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_clip_children(false),
    m_upperleft(x, y),
    m_lowerright(x + w, y + h),
    m_max_size(1 << 30, 1 << 30),
    m_flags(flags)
{
    ValidateFlags();
}

Wnd::Wnd(const XMLElement& elem) :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_clip_children(false),
    m_max_size(1 << 30, 1 << 30),
    m_flags(0)
{
    if (elem.Tag() != "GG::Wnd")
        throw std::invalid_argument("Attempted to construct a GG::Wnd from an XMLElement that had a tag other than \"GG::Wnd\"");

    m_text = elem.Child("m_text").Text();

    const XMLElement* curr_elem = &elem.Child("m_children");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        if (Wnd* w = GG::App::GetApp()->GenerateWnd(curr_elem->Child(i)))
            AttachChild(w);
    }

    m_zorder = lexical_cast<int>(elem.Child("m_zorder").Text());
    m_visible = lexical_cast<bool>(elem.Child("m_visible").Text());
    m_clip_children = lexical_cast<bool>(elem.Child("m_clip_children").Text());
    m_upperleft = Pt(elem.Child("m_upperleft").Child("GG::Pt"));
    m_lowerright = Pt(elem.Child("m_lowerright").Child("GG::Pt"));
    m_min_size = Pt(elem.Child("m_min_size").Child("GG::Pt"));
    m_max_size = Pt(elem.Child("m_max_size").Child("GG::Pt"));
    m_flags = FlagsFromString<Wnd::WndFlag>(elem.Child("m_flags").Text());
}

Wnd::~Wnd()
{
    // remove this-references from Wnds that this Wnd filters
    for (std::set<Wnd*>::iterator it1 = m_filtering.begin(); it1 != m_filtering.end(); ++it1) {
        std::vector<Wnd*>::iterator it2 = std::find((*it1)->m_filters.begin(), (*it1)->m_filters.end(), this);
        if (it2 != (*it1)->m_filters.end())
            (*it1)->m_filters.erase(it2);
    }

    // remove this-references from Wnds that filter this Wnd
    for (std::vector<Wnd*>::iterator it1 = m_filters.begin(); it1 != m_filters.end(); ++it1) {
        (*it1)->m_filtering.erase(this);
    }

    if (Parent()) {
        Parent()->DetachChild(this);
    } else {
        App::GetApp()->Remove(this);
    }

    DeleteChildren();
}

Pt Wnd::UpperLeft() const
{
    Pt retval = m_upperleft;
    if (m_parent)
        retval += m_parent->ClientUpperLeft();
    return retval;
}

Pt Wnd::LowerRight() const
{
    Pt retval = m_lowerright;
    if (m_parent)
        retval += m_parent->ClientUpperLeft();
    return retval;
}

Wnd* Wnd::RootParent() const
{
    Wnd* retval = m_parent;
    while (retval && retval->Parent()) {
        retval = retval->Parent();
    }
    return retval;
}

WndRegion Wnd::WindowRegion(const Pt& pt) const
{
    enum {LEFT=0, MIDDLE=1, RIGHT=2};
    enum {TOP=0, BOTTOM=2};

    // window regions look like this:
    // 0111112
    // 3444445   // 4 is client area, 0,2,6,8 are corners
    // 3444445
    // 6777778

    int x_pos = MIDDLE;   // default & typical case is that the mouse is over the (non-border) client area
    int y_pos = MIDDLE;

    if (pt.x < ClientUpperLeft().x)
        x_pos = LEFT;
    else if (pt.x > ClientLowerRight().x)
        x_pos = RIGHT;

    if (pt.y < ClientUpperLeft().y)
        y_pos = TOP;
    else if (pt.y > ClientLowerRight().y)
        y_pos = BOTTOM;

    return (Resizable() ? WndRegion(x_pos + 3 * y_pos) : WR_NONE);
}

XMLElement Wnd::XMLEncode() const
{
    XMLElement retval("GG::Wnd");
    retval.AppendChild(XMLElement("m_text", m_text));
    retval.LastChild().SetAttribute("edit", "always");

    XMLElement temp("m_children");
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        temp.AppendChild((*it)->XMLEncode());
    }
    retval.AppendChild(temp);

    retval.AppendChild(XMLElement("m_zorder", boost::lexical_cast<string>(m_zorder)));
    retval.LastChild().SetAttribute("edit", "never");
    retval.AppendChild(XMLElement("m_visible", boost::lexical_cast<string>(m_visible)));
    retval.LastChild().SetAttribute("edit", "never");
    retval.AppendChild(XMLElement("m_clip_children", boost::lexical_cast<string>(m_clip_children)));
    retval.AppendChild(XMLElement("m_upperleft", m_upperleft.XMLEncode()));
    retval.AppendChild(XMLElement("m_lowerright", m_lowerright.XMLEncode()));
    retval.AppendChild(XMLElement("m_min_size", m_min_size.XMLEncode()));
    retval.AppendChild(XMLElement("m_max_size", m_max_size.XMLEncode()));
    retval.AppendChild(XMLElement("m_flags", StringFromFlags<Wnd::WndFlag>(m_flags)));
    retval.LastChild().SetAttribute("edit", "never");

    return retval;
}

XMLElementValidator Wnd::XMLValidator() const
{
    XMLElementValidator retval("GG::Wnd");
    retval.AppendChild(XMLElementValidator("m_text"));
    retval.LastChild().SetAttribute("edit", 0);

#if 1
    retval.AppendChild(XMLElementValidator("m_children")); // this will be filled in by subclasses as needed
#else
    XMLElementValidator temp("m_children");
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        temp.AppendChild((*it)->XMLValidator());
    }
    retval.AppendChild(temp);
#endif

    retval.AppendChild(XMLElementValidator("m_zorder", new Validator<int>()));
    retval.LastChild().SetAttribute("edit", 0);
    retval.AppendChild(XMLElementValidator("m_visible", new Validator<bool>()));
    retval.LastChild().SetAttribute("edit", 0);
    retval.AppendChild(XMLElementValidator("m_clip_children", new Validator<bool>()));
    retval.AppendChild(XMLElementValidator("m_upperleft", m_upperleft.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_lowerright", m_lowerright.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_min_size", m_min_size.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_max_size", m_max_size.XMLValidator()));
    retval.AppendChild(XMLElementValidator("m_flags", new ListValidator<Wnd::WndFlag>()));
    retval.LastChild().SetAttribute("edit", 0);
    return retval;
}

void Wnd::Hide(bool children/* = true*/)
{
    m_visible = false;
    if (children) {
        std::list<Wnd*>::iterator it = m_children.begin();
        for (; it != m_children.end(); ++it)
            (*it)->Hide(children);
    }
}

void Wnd::Show(bool children/* = true*/)
{
    m_visible = true;
    if (children) {
        std::list<Wnd*>::iterator it = m_children.begin();
        for (; it != m_children.end(); ++it)
            (*it)->Show(children);
    }
}

void Wnd::EnableChildClipping(bool enable/* = true*/)
{
    m_clip_children = enable;
}

void Wnd::BeginClipping()
{
    BeginScissorClipping(ClientUpperLeft(), ClientLowerRight());
}

void Wnd::EndClipping()
{
    EndScissorClipping();
}

void Wnd::MoveTo(int x, int y)
{
    m_lowerright.x = (m_lowerright.x - m_upperleft.x) + x;
    m_lowerright.y = (m_lowerright.y - m_upperleft.y) + y;
    m_upperleft.x = x;
    m_upperleft.y = y;
}

void Wnd::MoveTo(const Pt& pt)
{
    MoveTo(pt.x, pt.y);
}

void Wnd::OffsetMove(int x, int y)
{
    SizeMove(m_upperleft.x + x, m_upperleft.y + y, m_lowerright.x + x, m_lowerright.y + y);
}

void Wnd::OffsetMove(const Pt& pt)
{
    OffsetMove(pt.x, pt.y);
}

void Wnd::SizeMove(const Pt& ul, const Pt& lr)
{
    SizeMove(ul.x, ul.y, lr.x, lr.y);
}

void Wnd::SizeMove(int x1, int y1, int x2, int y2)
{
    Pt min_sz = MinSize();
    if (x1 > x2 - min_sz.x) {
        if (x1 != m_upperleft.x)
            x1 = x2 - min_sz.x;
        else if (x2 != m_lowerright.x)
            x2 = x1 + min_sz.x;
    }
    if (y1 > y2 - min_sz.y) {
        if (y1 != m_upperleft.y)
            y1 = y2 - min_sz.y;
        else if (y2 != m_lowerright.y)
            y2 = y1 + min_sz.y;
    }
    m_upperleft = Pt(x1, y1);
    m_lowerright = Pt(x2, y2);
}

void Wnd::Resize(const Pt& sz)
{
    Resize(sz.x, sz.y);
}

void Wnd::Resize(int x, int y)
{
    SizeMove(m_upperleft.x, m_upperleft.y, m_upperleft.x + x, m_upperleft.y + y);
}

void Wnd::AttachChild(Wnd* wnd)
{
    if (wnd) {
        // remove from previous parent, if any
        if (wnd->Parent())
            wnd->Parent()->DetachChild(wnd);
        m_children.push_back(wnd);
        wnd->m_parent = this;
    }
}

void Wnd::MoveChildUp(Wnd* wnd)
{
    if (wnd) {
        if (std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
            m_children.remove(wnd);
            m_children.push_back(wnd);
        }
    }
}
 
void Wnd::MoveChildDown(Wnd* wnd)
{
    if (wnd) {
        if (std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
            m_children.remove(wnd);
            m_children.push_front(wnd);
        }
    }
}

void Wnd::DetachChild(Wnd* wnd)
{
    if (wnd) {
        std::list<Wnd*>::iterator it = std::find(m_children.begin(), m_children.end(), wnd);
        if (it != m_children.end()) {
	    // though child windows are never Register()ed, this Remove() makes sure that any 
	    // GG::App state pointers to the child do not outlive the child
	    App::GetApp()->Remove(wnd);
            m_children.erase(it);
            wnd->m_parent = 0;
        }
    }
}

void Wnd::DetachChildren()
{
    for (std::list<Wnd*>::iterator it = m_children.begin(); it != m_children.end();) {
        std::list<Wnd*>::iterator temp = it;
        ++it;
        DetachChild(*temp);
    }
}

void Wnd::DeleteChild(Wnd* wnd)
{
    if (wnd && std::find(m_children.begin(), m_children.end(), wnd) != m_children.end()) {
	delete wnd;
    }
}

void Wnd::DeleteChildren()
{
    for (std::list<Wnd*>::iterator it = m_children.begin(); it != m_children.end();) {
	Wnd* wnd = *it++;
        delete wnd;
    }
}

void Wnd::InstallEventFilter(Wnd* wnd)
{
    RemoveEventFilter(wnd);
    m_filters.push_back(wnd);
    wnd->m_filtering.insert(this);
}

void Wnd::RemoveEventFilter(Wnd* wnd)
{
    std::vector<Wnd*>::iterator it = std::find(m_filters.begin(), m_filters.end(), wnd);
    if (it != m_filters.end())
        m_filters.erase(it);
    wnd->m_filtering.erase(this);
}

bool Wnd::Render() {return true;}

void Wnd::LButtonDown(const Pt& pt, Uint32 keys) {}

void Wnd::LDrag(const Pt& pt, const Pt& move, Uint32 keys) {if (Dragable()) OffsetMove(move);}

void Wnd::LButtonUp(const Pt& pt, Uint32 keys) {}

void Wnd::LClick(const Pt& pt, Uint32 keys) {}

void Wnd::LDoubleClick(const Pt& pt, Uint32 keys) {}

void Wnd::RButtonDown(const Pt& pt, Uint32 keys) {;}

void Wnd::RClick(const Pt& pt, Uint32 keys) {}

void Wnd::RDoubleClick(const Pt& pt, Uint32 keys) {}

void Wnd::MouseEnter(const Pt& pt, Uint32 keys) {}

void Wnd::MouseHere(const Pt& pt, Uint32 keys) {}

void Wnd::MouseLeave(const Pt& pt, Uint32 keys) {}

void Wnd::MouseWheel(const Pt& pt, int move, Uint32 keys) {}

void Wnd::Keypress(Key key, Uint32 key_mods) {}

void Wnd::GainingFocus() {}

void Wnd::LosingFocus() {}

int Wnd::Run()
{
    int retval = 0;
    if (m_flags & MODAL) {
        App* app = App::GetApp();
        Wnd* old_focus_wnd = app->FocusWnd();
        app->RegisterModal(this);

        ModalEventPump pump(m_done);
        pump();

        app->Remove(this);
        app->SetFocusWnd(old_focus_wnd);

        retval = 1;
    }
    return retval;
}

bool Wnd::EventFilter(Wnd* w, const Event& event) {return false;}

void Wnd::ValidateFlags()
{
    if ((m_flags & MODAL) && (m_flags & ONTOP))
        m_flags &= ~ONTOP;
}

void Wnd::HandleEvent(const Event& event)
{
    bool done = false;

    for (int i = static_cast<int>(m_filters.size()) - 1; i >= 0; --i) {
        if (m_filters[i]->EventFilter(this, event))
            return;
    }

    switch (event.Type()) {
	case Event::LButtonDown:
	    LButtonDown(event.Point(), event.KeyMods());
	    break;
	case Event::LDrag:
	    LDrag(event.Point(), event.DragMove(), event.KeyMods());
	    break;
	case Event::LButtonUp:
	    LButtonUp(event.Point(), event.KeyMods());
	    break;
	case Event::LClick:
	    LClick(event.Point(), event.KeyMods());
	    break;
	case Event::LDoubleClick:
	    LDoubleClick(event.Point(), event.KeyMods());
	    break;
	case Event::RButtonDown:
	    RButtonDown(event.Point(), event.KeyMods());
	    break;
	case Event::RClick:
	    RClick(event.Point(), event.KeyMods());
	    break;
	case Event::RDoubleClick:
	    RDoubleClick(event.Point(), event.KeyMods());
	    break;
	case Event::MouseEnter:
	    MouseEnter(event.Point(), event.KeyMods());
	    break;
	case Event::MouseHere:
	    MouseHere(event.Point(), event.KeyMods());
	    break;
	case Event::MouseLeave:
	    MouseLeave(event.Point(), event.KeyMods());
	    break;
	case Event::MouseWheel:
	    MouseWheel(event.Point(), event.WheelMove(), event.KeyMods());
	    break;
	case Event::Keypress:
	    Keypress(event.KeyPress(), event.KeyMods());
	    break;
	case Event::GainingFocus:
	    GainingFocus();
	    break;
	case Event::LosingFocus:
	    LosingFocus();
	    break;
	default:
	    break;
    }
}
