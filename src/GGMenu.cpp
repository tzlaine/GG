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

#include "GGMenu.h"
#include "GGApp.h"
#include "GGTextControl.h"
#include "GGDrawUtil.h"

namespace GG {

namespace {
const int BORDER_THICKNESS = 1; // thickness with which to draw menu borders
const int MENU_SEPARATION = 10; // distance between menu texts in a MenuBar, in pixels
}

////////////////////////////////////////////////
// GG::MenuItem
////////////////////////////////////////////////
MenuItem::MenuItem(const XMLElement& elem) : 
    selected_signal(new SelectedSignalType())
{
    if (elem.Tag() != "GG::MenuItem")
	throw std::invalid_argument("Attempted to construct a GG::MenuItem from an XMLElement that had a tag other than \"GG::MenuItem\"");
      
    const XMLElement* curr_elem = &elem.Child("label");
    label = curr_elem->Text();
   
    curr_elem = &elem.Child("item_ID");
    item_ID = lexical_cast<int>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("disabled");
    disabled = lexical_cast<bool>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("checked");
    checked = lexical_cast<bool>(curr_elem->Attribute("value"));
   
    curr_elem = &elem.Child("next_level");
    for (int i = 0; i < curr_elem->NumChildren(); ++i)
	next_level.push_back(MenuItem(curr_elem->Child(i)));
}

XMLElement MenuItem::XMLEncode() const
{
    XMLElement retval("GG::MenuItem");
   
    XMLElement temp;
   
    temp = XMLElement("label", label);
    retval.AppendChild(temp);

    temp = XMLElement("item_ID");
    temp.SetAttribute("value", lexical_cast<string>(item_ID));
    retval.AppendChild(temp);

    temp = XMLElement("disabled");
    temp.SetAttribute("value", lexical_cast<string>(disabled));
    retval.AppendChild(temp);

    temp = XMLElement("checked");
    temp.SetAttribute("value", lexical_cast<string>(checked));
    retval.AppendChild(temp);

    temp = XMLElement("next_level");
    for (unsigned int i = 0; i < next_level.size(); ++i) {
	temp.AppendChild(next_level[i].XMLEncode());
    }
    retval.AppendChild(temp);

    return retval;
}


////////////////////////////////////////////////
// GG::MenuBar
////////////////////////////////////////////////
MenuBar::MenuBar(int x, int y, int w, const shared_ptr<Font>& font, Clr text_color/* = GG::CLR_WHITE*/, 
                 Clr color/* = GG::CLR_BLACK*/, Clr interior/* = GG::CLR_SHADOW*/) :
    Control(x, y, w, font->Lineskip()),
    m_font(font),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_caret(-1)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();
}

MenuBar::MenuBar(int x, int y, int w, const string& font_filename, int pts, Clr text_color/* = GG::CLR_WHITE*/, 
                 Clr color/* = GG::CLR_BLACK*/, Clr interior/* = GG::CLR_SHADOW*/) :
    Control(x, y, w, App::GetApp()->GetFont(font_filename, pts)->Lineskip()),
    m_font(App::GetApp()->GetFont(font_filename, pts)),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_caret(-1)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();
}

MenuBar::MenuBar(int x, int y, int w, const shared_ptr<Font>& font, const MenuItem& m, 
                 Clr text_color/* = GG::CLR_WHITE*/, Clr color/* = GG::CLR_BLACK*/, Clr interior/* = GG::CLR_SHADOW*/) :
    Control(x, y, w, font->Lineskip()),
    m_font(font),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_menu_data(m),
    m_caret(-1)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();
}

MenuBar::MenuBar(int x, int y, int w, const string& font_filename, int pts, const MenuItem& m, 
                 Clr text_color/* = GG::CLR_WHITE*/, Clr color/* = GG::CLR_BLACK*/, Clr interior/* = GG::CLR_SHADOW*/) :
    Control(x, y, w, App::GetApp()->GetFont(font_filename, pts)->Lineskip()),
    m_font(App::GetApp()->GetFont(font_filename, pts)),
    m_border_color(color),
    m_int_color(interior),
    m_text_color(text_color),
    m_sel_text_color(text_color),
    m_menu_data(m),
    m_caret(-1)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    AdjustLayout();
}

MenuBar::MenuBar(const XMLElement& elem) : 
    Control(elem.Child("GG::Control")),
    m_caret(-1)
{
    if (elem.Tag() != "GG::MenuBar")
	throw std::invalid_argument("Attempted to construct a GG::MenuBar from an XMLElement that had a tag other than \"GG::MenuBar\"");
   
    const XMLElement* curr_elem = &elem.Child("m_font").Child("GG::Font");
    string font_filename = curr_elem->Child("m_font_filename").Text();
    int pts = lexical_cast<int>(curr_elem->Child("m_pt_sz").Attribute("value"));
    m_font = App::GetApp()->GetFont(font_filename, pts);
   
    curr_elem = &elem.Child("m_border_color");
    m_border_color = Clr(curr_elem->Child("GG::Clr"));
   
    curr_elem = &elem.Child("m_int_color");
    m_int_color = Clr(curr_elem->Child("GG::Clr"));
   
    curr_elem = &elem.Child("m_text_color");
    m_text_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_hilite_color");
    m_hilite_color = Clr(curr_elem->Child("GG::Clr"));
   
    curr_elem = &elem.Child("m_sel_text_color");
    m_sel_text_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_menu_data");
    m_menu_data = MenuItem(curr_elem->Child("GG::MenuItem"));

    AdjustLayout();
}

bool MenuBar::ContainsMenu(const string& str) const
{
    bool retval = false;
    for (vector<MenuItem>::const_iterator it = m_menu_data.next_level.begin(); it != m_menu_data.next_level.end(); ++it) {
	if (it->label == str) {
	    retval = true;
	    break;
	}
    }
    return retval;
}

const MenuItem& MenuBar::GetMenu(const string& str) const
{
    vector<MenuItem>::const_iterator it = m_menu_data.next_level.begin();
    for (; it != m_menu_data.next_level.end(); ++it) {
	if (it->label == str)
	    break;
    }
    return *it;
}
   
const MenuItem& MenuBar::GetMenu(int n) const 
{
    return *(m_menu_data.next_level.begin() + n);
}

int MenuBar::Render()
{
    Pt ul = UpperLeft();
    Pt lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, m_int_color, m_border_color, BORDER_THICKNESS);

    // paint caret, if any
    if (m_caret != -1) {
	Pt caret_ul = m_menu_labels[m_caret]->UpperLeft() + Pt((!m_caret ? BORDER_THICKNESS : 0), BORDER_THICKNESS);
	Pt caret_lr = m_menu_labels[m_caret]->LowerRight() - Pt((m_caret == static_cast<int>(m_menu_labels.size()) - 1 ? BORDER_THICKNESS : 0), BORDER_THICKNESS);
	FlatRectangle(caret_ul.x, caret_ul.y, caret_lr.x, caret_lr.y, m_hilite_color, GG::CLR_ZERO, 0);
    }
   
    return 1;
}
   
int MenuBar::LButtonDown(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
	for (int i = 0; i < static_cast<int>(m_menu_labels.size()); ++i) {
	    if (m_menu_labels[i]->InWindow(pt)) {
		m_caret = -1;
		m_browsed_signal(0);
		// since a MenuBar is usually modeless, but becomes modal when a menu is opened, we do something kludgey here:
		// we launch a PopupMenu whenever a menu item is selected, then use the ID returned from it to find the
		// menu item that was chosen; we then emit a signal from that item
		if (m_menu_data.next_level[i].next_level.empty()) {
		    m_menu_data.next_level[i].SelectedSignal()(m_menu_data.next_level[i].item_ID);
		} else {
		    MenuItem popup_data;
		    PopupMenu menu(m_menu_labels[i]->UpperLeft().x, m_menu_labels[i]->LowerRight().y, m_font, m_menu_data.next_level[i], m_text_color, m_border_color, m_int_color);
		    menu.SetHiliteColor(m_hilite_color);
		    menu.SetSelectedTextColor(m_sel_text_color);
		    Connect(menu.BrowsedSignal(), &MenuBar::BrowsedSlot, this);
		    menu.Run();
		}
	    }
	}
    }
    return 1;
}

int MenuBar::MouseHere(const Pt& pt, Uint32 keys)
{
    if (!Disabled()) {
	m_caret = -1;
	for (unsigned int i = 0; i < m_menu_data.next_level.size(); ++i) {
	    if (m_menu_labels[i]->InWindow(pt)) {
		m_caret = i;
		break;
	    }
	}
    }
    return 1;
}

void MenuBar::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    AdjustLayout();
}

MenuItem& MenuBar::GetMenu(const string& str)
{
    vector<MenuItem>::iterator it = m_menu_data.next_level.begin();
    for (; it != m_menu_data.next_level.end(); ++it) {
	if (it->label == str)
	    break;
    }
    return *it;
}

void MenuBar::AddMenu(const MenuItem& menu)
{
    m_menu_data.next_level.push_back(menu);
    AdjustLayout();
}

XMLElement MenuBar::XMLEncode() const
{
    XMLElement retval("GG::MenuBar");
    retval.AppendChild(Control::XMLEncode());
   
    // remove children; they will be recreated at reload time
    while (retval.Child("GG::Control").Child("GG::Wnd").Child("m_children").NumChildren())
	retval.Child("GG::Control").Child("GG::Wnd").Child("m_children").RemoveChild(0);
   
    XMLElement temp;
   
    temp = XMLElement("m_font");
    temp.AppendChild(m_font->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_int_color");
    temp.AppendChild(m_int_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_text_color");
    temp.AppendChild(m_text_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_hilite_color");
    temp.AppendChild(m_hilite_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_sel_text_color");
    temp.AppendChild(m_sel_text_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_menu_data");
    temp.AppendChild(m_menu_data.XMLEncode());
    retval.AppendChild(temp);

    return retval;
}

void MenuBar::AdjustLayout()
{
    // create any needed labels
    for (unsigned int i = m_menu_labels.size(); i < m_menu_data.next_level.size(); ++i) {
	m_menu_labels.push_back(new StaticText(0, 0, m_menu_data.next_level[i].label, m_font->FontName(), m_font->PointSize(), m_text_color));
	m_menu_labels.back()->Resize(m_menu_labels.back()->Wnd::Width() + 2 * MENU_SEPARATION, m_font->Lineskip());
	AttachChild(m_menu_labels.back());
    }

    // determine rows layout
    vector<int> menu_rows; // each element is the last + 1 index displayable on that row
    int space = Width();
    for (unsigned int i = 0; i < m_menu_labels.size(); ++i) {
	space -= m_menu_labels[i]->Wnd::Width();
	if (space < 0) { // if this menu's text won't fit in the available space
	    space = Width();
	    // if moving this menu to the next row would leave an empty row, leave it here even though it won't quite fit
	    if (!menu_rows.empty() && menu_rows.back() == static_cast<int>(i) - 1) {
		menu_rows.push_back(i + 1);
	    } else {
		menu_rows.push_back(i);
		space -= m_menu_labels[i]->Wnd::Width();
	    }
	}
    }
    if (menu_rows.empty() || menu_rows.back() < static_cast<int>(m_menu_labels.size()))
	menu_rows.push_back(m_menu_labels.size());

    // place labels
    int label_i = 0;
    for (unsigned int row = 0; row < menu_rows.size(); ++row) {
	int x = 0;
	for (; label_i < menu_rows[row]; ++label_i) {
	    m_menu_labels[label_i]->MoveTo(x, row * m_font->Lineskip());
	    x += m_menu_labels[label_i]->Wnd::Width();
	}
    }
   
    // resize MenuBar if needed
    int desired_ht = std::max(1u, menu_rows.size()) * m_font->Lineskip();
    if (Wnd::Height() != desired_ht)
	Resize(Wnd::Width(), desired_ht);
}



////////////////////////////////////////////////
// GG::PopupMenu
////////////////////////////////////////////////
namespace {
const int HORIZONTAL_MARGIN = 3; // distance to leave between edge of PopupMenu contents and the control's border
}

PopupMenu::PopupMenu(int x, int y, const shared_ptr<Font>& font, const MenuItem& m, Clr text_color/* = GG::CLR_WHITE*/, 
                     Clr color/* = GG::CLR_BLACK*/, Clr interior/* = GG::CLR_SHADOW*/) :
    Wnd(0, 0, GG::App::GetApp()->AppWidth() - 1, GG::App::GetApp()->AppHeight() - 1, CLICKABLE | MODAL), 
    m_font(font), 
    m_border_color(color), 
    m_int_color(interior), 
    m_text_color(text_color), 
    m_sel_text_color(text_color), 
    m_menu_data(m), 
    m_open_levels(), 
    m_caret(vector<int>(1, -1)), 
    m_origin(Pt(x, y)), 
    m_item_selected(0)
{
    // use opaque interior color as hilite color
    interior.a = 255;
    m_hilite_color = interior;
    m_open_levels.resize(1);
}

int PopupMenu::Render()
{
    if (m_menu_data.next_level.size())
    {
	Pt ul = ClientUpperLeft();

	const int INDICATOR_VERTICAL_MARGIN = 3;
	const int INDICATOR_HEIGHT = m_font->Lineskip() - 2 * INDICATOR_VERTICAL_MARGIN;
	const int CHECK_HEIGHT = INDICATOR_HEIGHT;
	const int CHECK_WIDTH = CHECK_HEIGHT;
         
	int next_menu_x_offset = 0;
	int next_menu_y_offset = 0;
	for (unsigned int i = 0; i < m_caret.size(); ++i) {
	    bool needs_indicator = false;

	    // get the correct submenu
	    MenuItem* menu_ptr = &m_menu_data;
	    for (unsigned int j = 0; j < i; ++j)
		menu_ptr = &menu_ptr->next_level[m_caret[j]];
	    MenuItem& menu = *menu_ptr;

	    // determine the total size of the menu, render it, and record its bounding rect
	    string str;
	    for (unsigned int j = 0; j < menu.next_level.size(); ++j) {
		str += menu.next_level[j].label + (static_cast<int>(j) < static_cast<int>(menu.next_level.size()) - 1 ? "\n" : "");
		if (menu.next_level[j].next_level.size() || menu.next_level[j].checked)
		    needs_indicator = true;
	    }
	    vector<Font::LineData> lines;
	    Uint32 fmt = TF_LEFT | TF_TOP;
	    Pt menu_sz = m_font->DetermineLines(str, fmt, 0, lines); // get dimensions of text in menu
	    menu_sz.x += 2 * HORIZONTAL_MARGIN;
	    if (needs_indicator) 
		menu_sz.x += CHECK_WIDTH + 2 * HORIZONTAL_MARGIN; // make room for the little arrow
	    Rect r(ul.x + next_menu_x_offset, ul.y + next_menu_y_offset, 
		   ul.x + next_menu_x_offset + menu_sz.x, ul.y + next_menu_y_offset + menu_sz.y);

	    if (r.lr.x > App::GetApp()->AppWidth()) {
		int offset = r.lr.x - App::GetApp()->AppWidth();
		r.ul.x -= offset;
		r.lr.x -= offset;
	    }
	    if (r.lr.y > App::GetApp()->AppHeight()) {
		int offset = r.lr.y - App::GetApp()->AppHeight();
		r.ul.y -= offset;
		r.lr.y -= offset;
	    }
	    next_menu_x_offset = menu_sz.x;
	    next_menu_y_offset = m_caret[i] * m_font->Lineskip();
	    FlatRectangle(r.ul.x, r.ul.y, r.lr.x, r.lr.y, m_int_color, m_border_color, BORDER_THICKNESS);
	    m_open_levels[i] = r;

	    // paint caret, if any
	    if (m_caret[i] != -1) {
		Rect tmp_r = r;
		tmp_r.ul.y += m_caret[i] * m_font->Lineskip();
		tmp_r.lr.y = tmp_r.ul.y + m_font->Lineskip() + 3;
		tmp_r.ul.x += BORDER_THICKNESS;
		tmp_r.lr.x -= BORDER_THICKNESS;
		if (m_caret[i] == 0) 
		    tmp_r.ul.y += BORDER_THICKNESS;
		if (m_caret[i] == static_cast<int>(menu.next_level.size()) - 1) 
		    tmp_r.lr.y -= BORDER_THICKNESS;
		FlatRectangle(tmp_r.ul.x, tmp_r.ul.y, tmp_r.lr.x, tmp_r.lr.y, m_hilite_color, GG::CLR_ZERO, 0);
	    }

	    // paint menu text and submenu indicator arrows
	    Rect line_rect = r;
	    line_rect.ul.x += HORIZONTAL_MARGIN;
	    line_rect.lr.x -= HORIZONTAL_MARGIN;
	    for (unsigned int j = 0; j < menu.next_level.size(); ++j) {
		Clr clr = (m_caret[i] == static_cast<int>(j)) ? 
		    (menu.next_level[j].disabled ? DisabledColor(m_sel_text_color) : m_sel_text_color) :
		    (menu.next_level[j].disabled ? DisabledColor(m_text_color) : m_text_color);
		glColor3ubv(clr.v);
		m_font->RenderText(line_rect.ul.x, line_rect.ul.y, line_rect.lr.x, line_rect.lr.y, menu.next_level[j].label, fmt);
		if (menu.next_level[j].checked) {
		    FlatCheck(line_rect.lr.x - CHECK_WIDTH - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_VERTICAL_MARGIN,
			      line_rect.lr.x - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_VERTICAL_MARGIN + CHECK_HEIGHT, clr);
		}
		if (menu.next_level[j].next_level.size()) {
		    glDisable(GL_TEXTURE_2D);
		    glBegin(GL_TRIANGLES);
		    glVertex2d(line_rect.lr.x - INDICATOR_HEIGHT / 2.0 - HORIZONTAL_MARGIN, line_rect.ul.y + INDICATOR_VERTICAL_MARGIN);
		    glVertex2d(line_rect.lr.x - INDICATOR_HEIGHT / 2.0 - HORIZONTAL_MARGIN, line_rect.ul.y + m_font->Lineskip() - INDICATOR_VERTICAL_MARGIN);
		    glVertex2d(line_rect.lr.x - HORIZONTAL_MARGIN,                          line_rect.ul.y + m_font->Lineskip() / 2.0);
		    glEnd();
		}
		line_rect.ul.y += m_font->Lineskip();
	    }
	}
    }

    return 1;
}

int PopupMenu::LButtonUp(const Pt& pt, Uint32 keys)
{
    if (m_caret[0] != -1) {
	MenuItem* menu_ptr = &m_menu_data;
	for (unsigned int i = 0; i < m_caret.size(); ++i)
	    menu_ptr = &menu_ptr->next_level[m_caret[i]];
	if (!menu_ptr->disabled) {
	    m_item_selected = menu_ptr;
	}
    }
    m_browsed_signal(0);
    m_done = true;
    return 1;
}

int PopupMenu::LDrag(const Pt& pt, const GG::Pt& move, Uint32 keys)
{
    bool cursor_is_in_menu = false;
    for (int i = static_cast<int>(m_open_levels.size()) - 1; i >= 0; --i) {
	// get the correct submenu
	MenuItem* menu_ptr = &m_menu_data;
	for (int j = 0; j < i; ++j)
	    menu_ptr = &menu_ptr->next_level[m_caret[j]];
	MenuItem& menu = *menu_ptr;

	if (pt.x >= m_open_levels[i].ul.x && pt.x <= m_open_levels[i].lr.x && 
	    pt.y >= m_open_levels[i].ul.y && pt.y <= m_open_levels[i].lr.y) {
	    int row_selected = (pt.y - m_open_levels[i].ul.y) / m_font->Lineskip();
	    if (row_selected == m_caret[i]) {
		cursor_is_in_menu = true;
	    } else if (row_selected >= 0 && row_selected < static_cast<int>(menu.next_level.size())) {
		m_caret[i] = row_selected;
		m_open_levels.resize(i + 1);
		m_caret.resize(i + 1);
		if (!menu.next_level[row_selected].disabled && menu.next_level[row_selected].next_level.size()) {
		    m_caret.push_back(0);
		    m_open_levels.push_back(Rect());
		}
		cursor_is_in_menu = true;
	    }
	}
    }
    if (!cursor_is_in_menu) {
	m_open_levels.resize(1);
	m_caret.resize(1);
	m_caret[0] = -1;
    }
    int update_ID = 0;
    if (m_caret[0] != -1) {
	MenuItem* menu_ptr = &m_menu_data;
	for (unsigned int i = 0; i < m_caret.size(); ++i)
	    menu_ptr = &menu_ptr->next_level[m_caret[i]];
	update_ID = menu_ptr->item_ID;
    }
    m_browsed_signal(update_ID);
    return 1;
}

int PopupMenu::Run()
{
    int retval = Wnd::Run();
    if (m_item_selected)
	m_item_selected->SelectedSignal()(m_item_selected->item_ID);
    return retval;
}

} // namespace GG

