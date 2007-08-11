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

#include <GG/DropDownList.h>

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Scroll.h>
#include <GG/StyleFactory.h>
#include <GG/WndEditor.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    const int BORDER_THICK = 2; // should be the same as the BORDER_THICK value in GGListBox.h

    class ModalListPicker : public Wnd
    {
    public:
        ModalListPicker(DropDownList* drop_wnd, ListBox* lb_wnd) :
            Wnd(0, 0, GUI::GetGUI()->AppWidth(), GUI::GetGUI()->AppHeight(), CLICKABLE | MODAL),
            m_drop_wnd(drop_wnd),
            m_lb_wnd(lb_wnd),
            m_old_lb_ul(m_lb_wnd->UpperLeft())
        {
            Connect(m_lb_wnd->SelChangedSignal, &ModalListPicker::LBSelChangedSlot, this);
            Connect(m_lb_wnd->LeftClickedSignal, &ModalListPicker::LBLeftClickSlot, this);
            m_lb_ul = m_old_lb_ul + m_drop_wnd->UpperLeft();
            AttachChild(m_lb_wnd);
        }
        virtual void Render()
        {
            m_lb_wnd->MoveTo(m_lb_ul);
        }
        ~ModalListPicker()
        {
            m_lb_wnd->MoveTo(m_old_lb_ul);
            DetachChild(m_lb_wnd);
        }

    protected:
        virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys) {m_done = true;}

    private:
        void LBSelChangedSlot(const std::set<int>& rows)
        {
            if (!rows.empty()) {
                m_drop_wnd->Select(*rows.begin());
                m_done = true;
            }
        }
        void LBLeftClickSlot(int, ListBox::Row*, const Pt&)
        {
            m_done = true;
        }

        DropDownList*  m_drop_wnd;
        ListBox*       m_lb_wnd;
        Pt             m_old_lb_ul;
        Pt             m_lb_ul;
    };
}

////////////////////////////////////////////////
// GG::DropDownList
////////////////////////////////////////////////
DropDownList::DropDownList() :
    Control(),
    m_current_item_idx(-1),
    m_LB(0)
{}

DropDownList::DropDownList(int x, int y, int w, int h, int drop_ht, Clr color, Flags<WndFlag> flags/* = CLICKABLE*/) :
    Control(x, y, w, h, flags),
    m_current_item_idx(-1),
    m_LB(GetStyleFactory()->NewDropDownListListBox(x, y, w, drop_ht, color, color, flags))
{
    SetStyle(LIST_SINGLESEL);
    // adjust size to keep correct height based on row height, etc.
    Wnd::SizeMove(Pt(x, y), Pt(x + Size().x, y + h + 2 * m_LB->CellMargin() + 2 * BORDER_THICK));
    m_LB->SizeMove(Pt(0, Height()), Pt(Width(), Height() + m_LB->Height()));
}

DropDownList::~DropDownList()
{
    delete m_LB;
}

const DropDownList::Row* DropDownList::CurrentItem() const
{
    return m_current_item_idx == -1 ? 0 : &m_LB->GetRow(m_current_item_idx);
}

int DropDownList::CurrentItemIndex() const
{
    return m_current_item_idx;
}

bool DropDownList::Empty() const
{
    return m_LB->Empty();
}

const DropDownList::Row& DropDownList::GetItem(int n) const
{
    return m_LB->GetRow(n);
}

bool DropDownList::Selected(int n) const
{
    return m_LB->Selected(n);
}

Clr DropDownList::InteriorColor() const
{
    return m_LB->InteriorColor();
}

int DropDownList::DropHeight() const
{
    return m_LB->Height();
}

Flags<ListBoxStyle> DropDownList::Style() const
{
    return m_LB->Style();
}

int DropDownList::NumRows() const
{
    return m_LB->NumRows();
}

int DropDownList::NumCols() const
{
    return m_LB->NumCols();
}

int DropDownList::SortCol() const
{
    return m_LB->SortCol();
}

int DropDownList::ColWidth(int n) const
{
    return m_LB->ColWidth(n);
}

Alignment DropDownList::ColAlignment(int n) const
{
    return m_LB->ColAlignment(n);
}

Alignment DropDownList::RowAlignment(int n) const
{
    return m_LB->RowAlignment(n);
}

Pt DropDownList::ClientUpperLeft() const
{
    return UpperLeft() + Pt(BORDER_THICK, BORDER_THICK);
}

Pt DropDownList::ClientLowerRight() const
{
    return LowerRight() - Pt(BORDER_THICK, BORDER_THICK);
}

void DropDownList::Render()
{
    // draw beveled rectangle around client area
    Pt ul = UpperLeft(), lr = LowerRight();
    Clr color_to_use = Disabled() ? DisabledColor(m_LB->Color()) : m_LB->Color();
    Clr int_color_to_use = Disabled() ? DisabledColor(m_LB->m_int_color) : m_LB->m_int_color;

    BeveledRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, false, BORDER_THICK);

    // draw ListBox::Row of currently displayed item, if any
    if (m_current_item_idx != -1) {
        Row* current_item = &m_LB->GetRow(m_current_item_idx);
        Pt offset = ClientUpperLeft() - current_item->UpperLeft();
        bool visible = current_item->Visible();
        current_item->OffsetMove(offset);
        if (!visible)
            current_item->Show();
        BeginClipping();
        GUI::GetGUI()->RenderWindow(current_item);
        EndClipping();
        current_item->OffsetMove(-offset);
        if (!visible)
            current_item->Hide();
    }
}

void DropDownList::LClick(const Pt& pt, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        ModalListPicker picker(this, m_LB);
        const std::set<int>& LB_sels = m_LB->Selections();
        if (!LB_sels.empty()) {
            if (m_LB->m_vscroll)
                m_LB->m_vscroll->ScrollTo(0);
        }
        m_LB->m_first_col_shown = 0;
        picker.Run();
    }
}

void DropDownList::KeyPress(Key key, Flags<ModKey> mod_keys)
{
    if (!Disabled()) {
        switch (key) {
        case GGK_UP: // arrow-up (not numpad arrow)
            if (1 <= m_current_item_idx)
                Select(std::max(0, m_current_item_idx - 1));
            break;
        case GGK_DOWN: // arrow-down (not numpad arrow)
            if (m_current_item_idx < m_LB->NumRows())
                Select(std::min(m_current_item_idx + 1, m_LB->NumRows() - 1));
            break;
        case GGK_PAGEUP: // page up key (not numpad key)
            if (m_LB->NumRows())
                Select(std::max(0, m_current_item_idx - 10));
            break;
        case GGK_PAGEDOWN: // page down key (not numpad key)
            if (m_LB->NumRows())
                Select(std::min(m_current_item_idx + 10, m_LB->NumRows() - 1));
            break;
        case GGK_HOME: // home key (not numpad)
            if (m_LB->NumRows())
                Select(0);
            break;
        case GGK_END: // end key (not numpad)
            if (m_LB->NumRows())
                Select(m_LB->NumRows() - 1);
            break;
        default:
            Control::KeyPress(key, mod_keys);
        }
    } else {
        Control::KeyPress(key, mod_keys);
    }
}

void DropDownList::SizeMove(const Pt& ul, const Pt& lr)
{
    // adjust size to keep correct height based on row height, etc.
    Wnd::SizeMove(ul, lr);
    m_LB->SizeMove(Pt(0, Height()), Pt(Width(), Height() + m_LB->Height()));
}

void DropDownList::SetColor(Clr c)
{
    m_LB->SetColor(c);
}

int DropDownList::Insert(Row* row, int at/* = -1*/)
{
    return m_LB->Insert(row, at);
}

DropDownList::Row* DropDownList::Erase(int idx)
{
    if (idx == m_current_item_idx)
        m_current_item_idx = -1;
    else if (idx < m_current_item_idx)
        --m_current_item_idx;

    return m_LB->Erase(idx);
}

void DropDownList::Clear()
{
    m_current_item_idx = -1;
    m_LB->Clear();
}

DropDownList::Row& DropDownList::GetRow(int n)
{
    return m_LB->GetRow(n);
}

void DropDownList::Select(int row)
{
    int old_m_current_item_idx = m_current_item_idx;
    if (row <= -1 || m_LB->NumRows() <= row) {
        m_current_item_idx = -1;
        m_LB->DeselectAll();
    } else {
        m_current_item_idx = row;
        m_LB->SelectRow(m_current_item_idx);
    }

    if (m_current_item_idx != old_m_current_item_idx)
        SelChangedSignal(m_current_item_idx);
}

void DropDownList::SetInteriorColor(Clr c)
{
    m_LB->SetInteriorColor(c);
}

void DropDownList::SetDropHeight(int h)
{
    m_LB->Resize(Pt(Width(), h));
}

void DropDownList::SetStyle(Flags<ListBoxStyle> s)
{
    s &= ~(LIST_NOSEL | LIST_QUICKSEL | LIST_USERDELETE | LIST_BROWSEUPDATES);
    s |= LIST_SINGLESEL;
    m_LB->SetStyle(s);
    m_current_item_idx = -1;
}

void DropDownList::SetNumCols(int n)
{
    m_LB->SetNumCols(n);
}

void DropDownList::SetSortCol(int n)
{
    m_LB->SetSortCol(n);
    m_current_item_idx = -1;
}

void DropDownList::SetColWidth(int n, int w)
{
    m_LB->SetColWidth(n, w);
}

void DropDownList::LockColWidths()
{
    m_LB->LockColWidths();
}

void DropDownList::UnLockColWidths()
{
    m_LB->UnLockColWidths();
}

void DropDownList::SetColAlignment(int n, Alignment align) 
{
    m_LB->SetColAlignment(n, align);
}

void DropDownList::SetRowAlignment(int n, Alignment align) 
{
    m_LB->SetRowAlignment(n, align);
}

void DropDownList::DefineAttributes(WndEditor* editor)
{
    if (!editor)
        return;
    Control::DefineAttributes(editor);
    editor->Label("DropDownList");
    editor->Attribute("Current Item", m_current_item_idx);
}

ListBox* DropDownList::LB()
{
    return m_LB;
}
