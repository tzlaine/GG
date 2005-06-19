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
#include <GGLayout.h>
#include <XMLValidators.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace GG;

#define DEBUG_GRID_LAYOUT 0

namespace {
    using namespace boost::multi_index;
    struct GridLayoutWnd
    {
        GridLayoutWnd() : wnd(0) {}
        GridLayoutWnd(Wnd* wnd_, const Pt& ul_, const Pt& lr_) : wnd(wnd_), ul(ul_), lr(lr_) {}
        Wnd* wnd;
        Pt ul;
        Pt lr;
    };
    struct IsLeft
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return lhs.x < rhs.x;}
        bool operator()(int x, const Pt& pt) const          {return x < pt.x;}
        bool operator()(const Pt& pt, int x) const          {return pt.x < x;}
    };
    struct IsTop
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return lhs.y < rhs.y;}
        bool operator()(int y, const Pt& pt) const          {return y < pt.y;}
        bool operator()(const Pt& pt, int y) const          {return pt.y < y;}
    };
    struct IsRight
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return rhs.x < lhs.x;}
        bool operator()(int x, const Pt& pt) const          {return pt.x < x;}
        bool operator()(const Pt& pt, int x) const          {return x < pt.x;}
    };
    struct IsBottom
    {
        bool operator()(const Pt& lhs, const Pt& rhs) const {return rhs.y < lhs.y;}
        bool operator()(int y, const Pt& pt) const          {return pt.y < y;}
        bool operator()(const Pt& pt, int y) const          {return y < pt.y;}
    };
    struct Pointer {};
    struct Left {};
    struct Top {};
    struct Right {};
    struct Bottom {};
    typedef multi_index_container<
        GridLayoutWnd,
        indexed_by<
            ordered_unique<tag<Pointer>, member<GridLayoutWnd, Wnd*, &GridLayoutWnd::wnd> >,
            ordered_non_unique<tag<Left>, member<GridLayoutWnd, Pt, &GridLayoutWnd::ul>, IsLeft>,
            ordered_non_unique<tag<Top>, member<GridLayoutWnd, Pt, &GridLayoutWnd::ul>, IsTop>,
            ordered_non_unique<tag<Right>, member<GridLayoutWnd, Pt, &GridLayoutWnd::lr>, IsRight>,
            ordered_non_unique<tag<Bottom>, member<GridLayoutWnd, Pt, &GridLayoutWnd::lr>, IsBottom>
        >
    > GridLayoutWndContainer;
    typedef GridLayoutWndContainer::index<Pointer>::type::iterator PointerIter;
    typedef GridLayoutWndContainer::index<Left>::type::iterator    LeftIter;
    typedef GridLayoutWndContainer::index<Top>::type::iterator     TopIter;
    typedef GridLayoutWndContainer::index<Right>::type::iterator   RightIter;
    typedef GridLayoutWndContainer::index<Bottom>::type::iterator  BottomIter;

    struct WndHorizontalLess
    {
        bool operator()(const Wnd* lhs, const Wnd* rhs) const {return lhs->UpperLeft().x < rhs->UpperLeft().x;}
    };

    struct WndVerticalLess
    {
        bool operator()(const Wnd* lhs, const Wnd* rhs) const {return lhs->UpperLeft().y < rhs->UpperLeft().y;}
    };

    const int DEFAULT_LAYOUT_BORDER_MARGIN = 0;
    const int DEFAULT_LAYOUT_CELL_MARGIN = 5;

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
// Wnd::Event
Wnd::Event::Event(EventType type, const GG::Pt& pt, Uint32 keys) :
    m_type(type),
    m_point(pt),
    m_key_mods(keys),
    m_wheel_move(0)
{}

Wnd::Event::Event(EventType type, const Pt& pt, const Pt& move, Uint32 keys) :
    m_type(type),
    m_point(pt),
    m_key_mods(keys),
    m_drag_move(move),
    m_wheel_move(0)
{}

Wnd::Event::Event(EventType type, const Pt& pt, int move, Uint32 keys) :
    m_type(type),
    m_point(pt),
    m_key_mods(keys),
    m_wheel_move(move)
{}

Wnd::Event::Event(EventType type, Key key, Uint32 key_mods) :
    m_type(type),
    m_keypress(key),
    m_key_mods(key_mods),
    m_wheel_move(0)
{}

Wnd::Event::Event(EventType type) :
    m_type(type),
    m_key_mods(0), 
    m_wheel_move(0)
{}

Wnd::Event::EventType Wnd::Event::Type() const
{
    return m_type;
}

const GG::Pt& Wnd::Event::Point() const
{
    return m_point;
}

Key Wnd::Event::KeyPress() const
{
    return m_keypress;
}

Uint32 Wnd::Event::KeyMods() const
{
    return m_key_mods;
}

const GG::Pt& Wnd::Event::DragMove() const
{
    return m_drag_move;
}

int Wnd::Event::WheelMove() const
{
    return m_wheel_move;
}

// Wnd
Wnd::Wnd() :
    m_done(false),
    m_parent(0),
    m_zorder(0),
    m_visible(true),
    m_clip_children(false),
    m_max_size(1 << 30, 1 << 30),
    m_layout(0),
    m_containing_layout(0),
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
    m_layout(0),
    m_containing_layout(0),
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
    m_layout(0),
    m_containing_layout(0),
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

bool Wnd::Clickable() const
{
    return m_flags & CLICKABLE;
}

bool Wnd::Dragable() const
{
    return m_flags & DRAGABLE;
}

bool Wnd::DragKeeper() const
{
    return m_flags & DRAG_KEEPER;
}

bool Wnd::Resizable() const
{
    return m_flags & RESIZABLE;
}

bool Wnd::OnTop() const
{
    return m_flags & ONTOP;
}

bool Wnd::Modal() const
{
    return m_flags & MODAL;
}

bool Wnd::ClipChildren() const
{
    return m_clip_children;
}

bool Wnd::Visible() const
{
    return m_visible;
}

const string& Wnd::WindowText() const
{
    return m_text;
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

int Wnd::Width() const
{
    return m_lowerright.x - m_upperleft.x;
}

int Wnd::Height() const
{
    return m_lowerright.y - m_upperleft.y;
}

int Wnd::ZOrder() const
{
    return m_zorder;
}

Pt Wnd::Size() const
{
    return Pt(m_lowerright.x - m_upperleft.x, m_lowerright.y - m_upperleft.y);
}

Pt Wnd::MinSize() const
{
    return m_min_size;
}

Pt Wnd::MaxSize() const
{
    return m_max_size;
}

Pt Wnd::ClientUpperLeft() const
{
    return UpperLeft();
}

Pt Wnd::ClientLowerRight() const
{
    return LowerRight();
}

Pt Wnd::ClientSize() const
{
    return ClientLowerRight() - ClientUpperLeft();
}

int Wnd::ClientWidth() const
{
    return ClientLowerRight().x - ClientUpperLeft().x;
}

int Wnd::ClientHeight() const
{
    return ClientLowerRight().y - ClientUpperLeft().y;
}

Pt Wnd::ScreenToWindow(const Pt& pt) const
{
    return pt - UpperLeft();
}

Pt Wnd::ScreenToClient(const Pt& pt) const
{
    return pt - ClientUpperLeft();
}

bool Wnd::InWindow(const Pt& pt) const
{
    return pt >= UpperLeft() && pt < LowerRight();
}

bool Wnd::InClient(const Pt& pt) const
{
    return pt >= ClientUpperLeft() && pt < ClientLowerRight();
}

Wnd* Wnd::Parent() const
{
    return m_parent;
}

Wnd* Wnd::RootParent() const
{
    Wnd* retval = m_parent;
    while (retval && retval->Parent()) {
        retval = retval->Parent();
    }
    return retval;
}

const Layout* Wnd::GetLayout() const
{
    return m_layout;
}

const Layout* Wnd::ContainingLayout() const
{
    return m_containing_layout;
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

void Wnd::SetText(const string& str)
{
    m_text = str;
}

void Wnd::SetText(const char* str)
{
    m_text = str;
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

void Wnd::ModalInit()
{
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

void Wnd::MoveTo(const Pt& pt)
{
    MoveTo(pt.x, pt.y);
}

void Wnd::MoveTo(int x, int y)
{
    m_lowerright.x = (m_lowerright.x - m_upperleft.x) + x;
    m_lowerright.y = (m_lowerright.y - m_upperleft.y) + y;
    m_upperleft.x = x;
    m_upperleft.y = y;
}

void Wnd::OffsetMove(const Pt& pt)
{
    OffsetMove(pt.x, pt.y);
}

void Wnd::OffsetMove(int x, int y)
{
    SizeMove(m_upperleft.x + x, m_upperleft.y + y, m_lowerright.x + x, m_lowerright.y + y);
}

void Wnd::SizeMove(const Pt& ul, const Pt& lr)
{
    SizeMove(ul.x, ul.y, lr.x, lr.y);
}

void Wnd::SizeMove(int x1, int y1, int x2, int y2)
{
    Pt original_sz = Size();
    Pt min_sz = MinSize();
    Pt max_sz = MaxSize();
    if (m_layout) {
        Pt layout_min_sz = m_layout->MinSize() + (Size() - ClientSize());
        min_sz.x = std::max(min_sz.x, layout_min_sz.x);
        min_sz.y = std::max(min_sz.y, layout_min_sz.y);
    }
    if (x2 - x1 < min_sz.x) {
        if (x1 != m_upperleft.x)
            x1 = x2 - min_sz.x;
        else if (x2 != m_lowerright.x)
            x2 = x1 + min_sz.x;
    } else if (max_sz.x < x2 - x1) {
        if (x2 != m_lowerright.x)
            x2 = x1 + max_sz.x;
        else
            x1 = x2 - max_sz.x;
    }
    if (y2 - y1 < min_sz.y) {
        if (y1 != m_upperleft.y)
            y1 = y2 - min_sz.y;
        else if (y2 != m_lowerright.y)
            y2 = y1 + min_sz.y;
    } else if (max_sz.y < y2 - y1) {
        if (y2 != m_lowerright.y)
            y2 = y1 + max_sz.y;
        else
            y1 = y2 - max_sz.y;
    }
    m_upperleft = Pt(x1, y1);
    m_lowerright = Pt(x2, y2);
    bool size_changed = Size() != original_sz;
    if (m_layout && size_changed)
        m_layout->Resize(ClientSize());
    if (m_containing_layout && size_changed && !dynamic_cast<Layout*>(this))
        m_containing_layout->ChildSizeOrMinSizeOrMaxSizeChanged();
}

void Wnd::Resize(const Pt& sz)
{
    Resize(sz.x, sz.y);
}

void Wnd::Resize(int x, int y)
{
    SizeMove(m_upperleft.x, m_upperleft.y, m_upperleft.x + x, m_upperleft.y + y);
}

void Wnd::SetMinSize(const Pt& sz)
{
    bool min_size_changed = m_min_size != sz;
    m_min_size = sz;
    if (Width() < m_min_size.x || Height() < m_min_size.y)
        Resize(std::max(Width(), m_min_size.x), std::max(Height(), m_min_size.y));
    else if (m_containing_layout && min_size_changed && !dynamic_cast<Layout*>(this))
        m_containing_layout->ChildSizeOrMinSizeOrMaxSizeChanged();
}

void Wnd::SetMinSize(int x, int y)
{
    SetMinSize(Pt(x, y));
}

void Wnd::SetMaxSize(const Pt& sz)
{
    m_max_size = sz;
    if (m_max_size.x < Width() || m_max_size.y < Height())
        Resize(std::min(Width(), m_max_size.x), std::min(Height(), m_max_size.y));
}

void Wnd::SetMaxSize(int x, int y)
{
    SetMaxSize(Pt(x, y));
}

void Wnd::AttachChild(Wnd* wnd)
{
    if (wnd) {
        // remove from previous parent, if any
        if (wnd->Parent())
            wnd->Parent()->DetachChild(wnd);
        m_children.push_back(wnd);
        wnd->m_parent = this;
        if (Layout* this_as_layout = dynamic_cast<Layout*>(this))
            wnd->m_containing_layout = this_as_layout;
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
            if (dynamic_cast<Layout*>(this))
                wnd->m_containing_layout = 0;
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

void Wnd::HorizontalLayout()
{
    RemoveLayout();

    std::multiset<Wnd*, WndHorizontalLess> wnds;
    for (list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        wnds.insert(*it);
    }

    m_layout = new Layout(0, 0, ClientSize().x, ClientSize().y,
                          1, wnds.size(),
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    int i = 0;
    for (std::multiset<Wnd*, WndHorizontalLess>::iterator it = wnds.begin(); it != wnds.end(); ++it) {
        m_layout->Add(*it, 0, i++);
    }
}

void Wnd::VerticalLayout()
{
    RemoveLayout();

    std::multiset<Wnd*, WndVerticalLess> wnds;
    for (list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        wnds.insert(*it);
    }

    m_layout = new Layout(0, 0, ClientSize().x, ClientSize().y,
                          wnds.size(), 1,
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    int i = 0;
    for (std::multiset<Wnd*, WndVerticalLess>::iterator it = wnds.begin(); it != wnds.end(); ++it) {
        m_layout->Add(*it, i++, 0);
    }
}

void Wnd::GridLayout()
{
#if DEBUG_GRID_LAYOUT
    std::cerr << "********************************************************************************\n"
              << "Wnd::GridLayout()";
#endif

    RemoveLayout();

    Pt cl_ul = ClientUpperLeft(), cl_lr = ClientLowerRight();
    Pt client_sz = ClientSize();

    GridLayoutWndContainer grid_layout;

    // validate existing children and place them in a grid with one cell per pixel
    for (list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        Wnd* wnd = *it;
        Pt wnd_ul = wnd->UpperLeft(), wnd_lr = wnd->LowerRight();
        if (wnd_ul < cl_ul || cl_lr < wnd_lr)
            throw std::runtime_error("Wnd::GridLayout() : A child window lies at least partially outside the client area");

        list<Wnd*>::const_iterator it2 = it;
        ++it2;
        for (; it2 != m_children.end(); ++it2) {
            Rect other_wnd_rect((*it2)->UpperLeft(), (*it2)->LowerRight());
            if (other_wnd_rect.Contains(wnd_ul) || other_wnd_rect.Contains(wnd_lr - Pt(1, 1)))
                throw std::runtime_error("Wnd::GridLayout() : Two or more child windows overlap");
        }

        wnd_ul = ScreenToClient(wnd_ul);
        wnd_lr = ScreenToClient(wnd_lr);
        grid_layout.insert(GridLayoutWnd(wnd, wnd_ul, wnd_lr));
#if DEBUG_GRID_LAYOUT
        std::cerr << "  Adding window \"" << wnd->WindowText() << "\" @" << wnd << " (" << wnd_ul.x << ", " << wnd_ul.y << ") - (" << wnd_lr.x << ", " << wnd_lr.y << ")\n";
#endif
    }


    // align left sides of windows
#if DEBUG_GRID_LAYOUT
    std::cerr << "  Expanding left...\n";
#endif
    for (LeftIter it = grid_layout.get<Left>().begin(); it != grid_layout.get<Left>().end(); ++it) {
#if DEBUG_GRID_LAYOUT
        std::cerr << "    \"" << it->wnd->WindowText() << "\"...\n";
#endif
        Pt ul = it->ul;
        Pt lr = it->lr;
        for (int x = ul.x - 1; x >= 0; --x) {
            if (grid_layout.get<Right>().find(x + 1, IsRight()) != grid_layout.get<Right>().end()) {
                break;
            } else if (grid_layout.get<Left>().find(x, IsLeft()) != grid_layout.get<Left>().end()) {
                GridLayoutWnd grid_wnd = *it;
#if DEBUG_GRID_LAYOUT
                std::cerr << "    Expanded \"" << grid_wnd.wnd->WindowText() << "\" left " << grid_wnd.ul.x << " -> " << x << "\n";
#endif
                grid_wnd.ul.x = x;
                grid_layout.get<Left>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align right sides of windows
#if DEBUG_GRID_LAYOUT
    std::cerr << "  Expanding right...\n";
#endif
    for (RightIter it = grid_layout.get<Right>().begin(); it != grid_layout.get<Right>().end(); ++it) {
#if DEBUG_GRID_LAYOUT
        std::cerr << "    \"" << it->wnd->WindowText() << "\"...\n";
#endif
        Pt ul = it->ul;
        Pt lr = it->lr;
        for (int x = lr.x + 1; x < client_sz.x; ++x) {
            if (grid_layout.get<Left>().find(x - 1, IsLeft()) != grid_layout.get<Left>().end()) {
                break;
            } else if (grid_layout.get<Right>().find(x, IsRight()) != grid_layout.get<Right>().end()) {
                GridLayoutWnd grid_wnd = *it;
#if DEBUG_GRID_LAYOUT
                std::cerr << "    Expanded \"" << grid_wnd.wnd->WindowText() << "\" right " << grid_wnd.lr.x << " -> " << x << "\n";
#endif
                grid_wnd.lr.x = x;
                grid_layout.get<Right>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align tops of windows
#if DEBUG_GRID_LAYOUT
    std::cerr << "  Expanding up...\n";
#endif
    for (TopIter it = grid_layout.get<Top>().begin(); it != grid_layout.get<Top>().end(); ++it) {
#if DEBUG_GRID_LAYOUT
        std::cerr << "    \"" << it->wnd->WindowText() << "\"...\n";
#endif
        Pt ul = it->ul;
        Pt lr = it->lr;
        for (int y = ul.y - 1; y >= 0; --y) {
            if (grid_layout.get<Bottom>().find(y + 1, IsBottom()) != grid_layout.get<Bottom>().end()) {
                break;
            } else if (grid_layout.get<Top>().find(y, IsTop()) != grid_layout.get<Top>().end()) {
                GridLayoutWnd grid_wnd = *it;
#if DEBUG_GRID_LAYOUT
                std::cerr << "    Expanded \"" << grid_wnd.wnd->WindowText() << "\" up " << grid_wnd.ul.y << " -> " << y << "\n";
#endif
                grid_wnd.ul.y = y;
                grid_layout.get<Top>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // align bottoms of windows
#if DEBUG_GRID_LAYOUT
    std::cerr << "  Expanding down...\n";
#endif
    for (BottomIter it = grid_layout.get<Bottom>().begin(); it != grid_layout.get<Bottom>().end(); ++it) {
#if DEBUG_GRID_LAYOUT
        std::cerr << "    \"" << it->wnd->WindowText() << "\"...\n";
#endif
        Pt ul = it->ul;
        Pt lr = it->lr;
        for (int y = lr.y + 1; y < client_sz.y; ++y) {
            if (grid_layout.get<Top>().find(y - 1, IsTop()) != grid_layout.get<Top>().end()) {
                break;
            } else if (grid_layout.get<Bottom>().find(y, IsBottom()) != grid_layout.get<Bottom>().end()) {
                GridLayoutWnd grid_wnd = *it;
#if DEBUG_GRID_LAYOUT
                std::cerr << "    Expanded \"" << grid_wnd.wnd->WindowText() << "\" down " << grid_wnd.lr.y << " -> " << y << "\n";
#endif
                grid_wnd.lr.y = y;
                grid_layout.get<Bottom>().replace(it, grid_wnd);
                break;
            }
        }
    }

    // create an actual layout with a more reasonable number of cells from the pixel-grid layout
    set<int> unique_lefts;
    set<int> unique_tops;
    for (LeftIter it = grid_layout.get<Left>().begin(); it != grid_layout.get<Left>().end(); ++it) {
        unique_lefts.insert(it->ul.x);
    }
    for (TopIter it = grid_layout.get<Top>().begin(); it != grid_layout.get<Top>().end(); ++it) {
        unique_tops.insert(it->ul.y);
    }

#if DEBUG_GRID_LAYOUT
    std::cerr << "  Final left positions:\n";
    for (set<int>::iterator it = unique_lefts.begin(); it != unique_lefts.end(); ++it) {
        std::cerr << "    " << *it << "\n";
    }
    std::cerr << "  Final top positions:\n";
    for (set<int>::iterator it = unique_tops.begin(); it != unique_tops.end(); ++it) {
        std::cerr << "    " << *it << "\n";
    }
#endif

    if (unique_lefts.empty() || unique_tops.empty())
        return;

    m_layout = new Layout(0, 0, ClientSize().x, ClientSize().y,
                          unique_tops.size(), unique_lefts.size(),
                          DEFAULT_LAYOUT_BORDER_MARGIN, DEFAULT_LAYOUT_CELL_MARGIN);
    AttachChild(m_layout);

    // populate this new layout with the child windows, based on their placements in the pixel-grid layout
    for (PointerIter it = grid_layout.get<Pointer>().begin(); it != grid_layout.get<Pointer>().end(); ++it) {
        Wnd* wnd = it->wnd;
        Pt ul = it->ul;
        Pt lr = it->lr;
        int left = std::distance(unique_lefts.begin(), unique_lefts.find(ul.x));
        int top = std::distance(unique_tops.begin(), unique_tops.find(ul.y));
        int right = std::distance(unique_lefts.begin(), unique_lefts.lower_bound(lr.x));
        int bottom = std::distance(unique_tops.begin(), unique_tops.lower_bound(lr.y));
        m_layout->Add(wnd, top, left, bottom, right);
    }
}

void Wnd::SetLayout(Layout* layout)
{
    if (layout == m_layout && layout == m_containing_layout)
        throw std::runtime_error("Wnd::SetLayout() : Attempted to set a Wnd's layout to be its current layout or the layout that contains the wind");
    RemoveLayout();
    DeleteChildren();
    AttachChild(layout);
    m_layout = layout;
}

void Wnd::RemoveLayout()
{
    if (m_layout) {
        list<Wnd*> layout_children = m_layout->Children();
        for (list<Wnd*>::iterator it = layout_children.begin(); it != layout_children.end(); ++it) {
            AttachChild(*it);
        }
        DeleteChild(m_layout);
        m_layout = 0;
    }
}

void Wnd::SetLayoutBorderMargin(int margin)
{
    if (m_layout)
        m_layout->SetBorderMargin(margin);
}

void Wnd::SetLayoutCellMargin(int margin)
{
    if (m_layout)
        m_layout->SetCellMargin(margin);
}

bool Wnd::Render() {return true;}

void Wnd::LButtonDown(const Pt& pt, Uint32 keys) {}

void Wnd::LDrag(const Pt& pt, const Pt& move, Uint32 keys) {if (Dragable()) OffsetMove(move);}

void Wnd::LButtonUp(const Pt& pt, Uint32 keys) {}

void Wnd::LClick(const Pt& pt, Uint32 keys) {}

void Wnd::LDoubleClick(const Pt& pt, Uint32 keys) {}

void Wnd::RButtonDown(const Pt& pt, Uint32 keys) {}

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
        app->RegisterModal(this);
        ModalInit();
        ModalEventPump pump(m_done);
        pump();
        app->Remove(this);
        retval = 1;
    }
    return retval;
}

const list<Wnd*>& Wnd::Children() const
{
    return m_children;
}

bool Wnd::EventFilter(Wnd* w, const Event& event)
{
    return false;
}

Layout* Wnd::GetLayout()
{
    return m_layout;
}

Layout* Wnd::ContainingLayout()
{
    return m_containing_layout;
}

void Wnd::ValidateFlags()
{
    if ((m_flags & MODAL) && (m_flags & ONTOP))
        m_flags &= ~ONTOP;
}

void Wnd::HandleEvent(const Event& event)
{
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
