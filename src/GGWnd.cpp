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
#include "GGApp.h"

namespace GG {

///////////////////////////////////////
// class GG::Wnd
///////////////////////////////////////
Wnd::Wnd() :
        m_done(false),
        m_parent(0),
        m_zorder(0),
        m_visible(true),
        m_max_size(1 << 30, 1 << 30),
        m_flags(0)
{
}

Wnd::Wnd(int x, int y, int w, int h, Uint32 flags) :
        m_done(false),
        m_parent(0),
        m_zorder(0),
        m_visible(true),
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
        m_max_size(1 << 30, 1 << 30),
        m_flags(0)
{
    if (elem.Tag() != "GG::Wnd")
        throw std::invalid_argument("Attempted to construct a GG::Wnd from an XMLElement that had a tag other than \"GG::Wnd\"");

    const XMLElement* curr_elem = &elem.Child("m_text");
    m_text = curr_elem->Text();

    curr_elem = &elem.Child("m_children");
    for (int i = 0; i < curr_elem->NumChildren(); ++i) {
        if (Wnd* w = GG::App::GetApp()->GenerateWnd(curr_elem->Child(i)))
            AttachChild(w);
    }

    curr_elem  = &elem.Child("m_zorder");
    m_zorder = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem  = &elem.Child("m_visible");
    m_visible = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem  = &elem.Child("m_upperleft");
    m_upperleft = Pt(curr_elem->Child("GG::Pt"));

    curr_elem  = &elem.Child("m_lowerright");
    m_lowerright = Pt(curr_elem->Child("GG::Pt"));

    curr_elem  = &elem.Child("m_min_size");
    m_min_size = Pt(curr_elem->Child("GG::Pt"));

    curr_elem  = &elem.Child("m_max_size");
    m_max_size = Pt(curr_elem->Child("GG::Pt"));

    curr_elem  = &elem.Child("m_flags");
    m_flags = lexical_cast<Uint32>(curr_elem->Attribute("value"));
}

Wnd::~Wnd()
{
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
    XMLElement temp;

    temp = XMLElement("m_text", m_text);
    retval.AppendChild(temp);

    temp = XMLElement("m_children");
    for (std::list<Wnd*>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
        temp.AppendChild((*it)->XMLEncode());
    }
    retval.AppendChild(temp);

    temp = XMLElement("m_zorder");
    temp.SetAttribute("value", boost::lexical_cast<string>(m_zorder));
    retval.AppendChild(temp);

    temp = XMLElement("m_visible");
    temp.SetAttribute("value", boost::lexical_cast<string>(m_visible));
    retval.AppendChild(temp);

    temp = XMLElement("m_upperleft");
    temp.AppendChild(m_upperleft.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_lowerright");
    temp.AppendChild(m_lowerright.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_min_size");
    temp.AppendChild(m_min_size.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_max_size");
    temp.AppendChild(m_max_size.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_flags");
    temp.SetAttribute("value", boost::lexical_cast<string>(m_flags));
    retval.AppendChild(temp);

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
    Pt min_sz = MinDimensions();
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
        m_children.remove(wnd);
        m_children.push_back(wnd);
    }
}
 
void Wnd::MoveChildDown(Wnd* wnd)
{
    if (wnd) {
        m_children.remove(wnd);
        m_children.push_front(wnd);
    }
}

void Wnd::DetachChild(Wnd* wnd)
{
    if (wnd) {
        std::list<Wnd*>::iterator it = std::find(m_children.begin(), m_children.end(), wnd);
        if (it != m_children.end()) {
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
    if (wnd) {
        std::list<Wnd*>::iterator it = std::find(m_children.begin(), m_children.end(), wnd);
        if (it != m_children.end()) {
            boost::checked_delete(wnd);
            m_children.erase(it);
        }
    }
}

void Wnd::DeleteChildren()
{
    std::list<Wnd*>::iterator it = m_children.begin();
    for (; it != m_children.end(); ++it)
        boost::checked_delete(*it);
    m_children.clear();
}

int Wnd::Render() {return 1;}

int Wnd::LButtonDown(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::LDrag(const Pt& pt, const Pt& move, Uint32 keys) {if (Dragable()) OffsetMove(move); return 1;}

int Wnd::LButtonUp(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::LClick(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::LDoubleClick(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::RButtonDown(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::RClick(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::RDoubleClick(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::MouseEnter(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::MouseHere(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::MouseLeave(const Pt& pt, Uint32 keys) {return 1;}

int Wnd::MouseWheel(const Pt& pt, int move, Uint32 keys) {return 1;}

int Wnd::Keypress(Key key, Uint32 key_mods) {return 1;}

int Wnd::GainingFocus() {return 1;}

int Wnd::LosingFocus() {return 1;}

int Wnd::Run()
{
    int retval = 0;
    if (m_flags & MODAL) {
        App& app = *App::GetApp();
        Wnd* old_focus_wnd = app.FocusWnd();
        app.RegisterModal(this);

        while (!m_done)
            app.PollAndRender();

        app.Remove(this);
        app.SetFocusWnd(old_focus_wnd);

        retval = 1;
    }
    return retval;
}

void Wnd::ValidateFlags()
{
    if ((m_flags & MODAL) && (m_flags & ONTOP))
        m_flags &= ~ONTOP;
}

} // namespace GG

