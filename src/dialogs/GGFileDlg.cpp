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

#include "GGFileDlg.h"

#include "../GGApp.h"
#include "../GGTextControl.h"
#include "../GGButton.h"
#include "../GGEdit.h"
#include "../GGDropDownList.h"
#include "../GGDrawUtil.h"

#include "GGThreeButtonDlg.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>

// HACK! this keeps gcc 3.2 from barfing when it sees "typedef long long uint64_t;"
// in boost/cstdint.h when compiling under windows
#ifdef WIN32
#  define BOOST_MSVC -1
#endif
#include <boost/cregex.hpp>
#ifdef WIN32
#  undef BOOST_MSVC
#endif


namespace GG {

// static member definition(s)
boost::filesystem::path FileDlg::m_working_dir = boost::filesystem::initial_path();


FileDlg::FileDlg(const string& filename, bool save, bool multi, const string& font_filename,
                 int pts, Clr color, Clr border_color, Clr text_color/* = CLR_BLACK*/,
                 Button* ok/* = 0*/, Button* cancel/* = 0*/) :
        Wnd((App::GetApp()->AppWidth() - WIDTH) / 2, (App::GetApp()->AppHeight() - HEIGHT) / 2, WIDTH, HEIGHT, CLICKABLE | DRAGABLE | MODAL),
        m_color(color),
        m_border_color(border_color),
        m_text_color(text_color),
        m_button_color(color),
        m_font(App::GetApp()->GetFont(font_filename, pts)),
        m_save(save),
        m_curr_dir_text(0),
        m_files_list(0),
        m_files_edit(0),
        m_filter_list(0),
        m_ok_button(ok),
        m_cancel_button(cancel),
        m_files_label(0),
        m_file_types_label(0)
{
    CreateChildren(filename, multi, font_filename, pts);
    Init();
}

FileDlg::FileDlg(const string& filename, bool save, bool multi, const vector<pair<string, string> >& types,
                 const string& font_filename, int pts, Clr color, Clr border_color,
                 Clr text_color/* = CLR_BLACK*/, Button* ok/* = 0*/, Button* cancel/* = 0*/) :
        Wnd((App::GetApp()->AppWidth() - WIDTH) / 2, (App::GetApp()->AppHeight() - HEIGHT) / 2, WIDTH, HEIGHT, CLICKABLE | DRAGABLE | MODAL),
        m_color(color),
        m_border_color(border_color),
        m_text_color(text_color),
        m_button_color(color),
        m_font(App::GetApp()->GetFont(font_filename, pts)),
        m_save(save),
        m_file_filters(types),
        m_curr_dir_text(0),
        m_files_list(0),
        m_files_edit(0),
        m_filter_list(0),
        m_ok_button(ok),
        m_cancel_button(cancel),
        m_files_label(0),
        m_file_types_label(0)
{
    CreateChildren(filename, multi, font_filename, pts);
    Init();
}

FileDlg::FileDlg(const XMLElement& elem) :
        Wnd(elem.Child("GG::Wnd"))
{
    if (elem.Tag() != "GG::FileDlg")
        throw std::invalid_argument("Attempted to construct a GG::FileDlg from an XMLElement that had a tag other than \"GG::FileDlg\"");

    const XMLElement* curr_elem = &elem.Child("m_color");
    m_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_border_color");
    m_border_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_text_color");
    m_text_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_button_color");
    m_button_color = Clr(curr_elem->Child("GG::Clr"));

    curr_elem = &elem.Child("m_font").Child("GG::Font");
    string font_filename = curr_elem->Child("m_font_filename").Text();
    int pts = lexical_cast<int>(curr_elem->Child("m_pt_sz").Attribute("value"));
    m_font = App::GetApp()->GetFont(font_filename, pts);

    curr_elem = &elem.Child("m_save");
    m_save = lexical_cast<bool>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_file_filters");
    for (int i = 0; i < curr_elem->NumChildren(); i += 2)
        m_file_filters.push_back(std::make_pair(curr_elem->Child(i).Text(), curr_elem->Child(i + 1).Text()));

    curr_elem = &elem.Child("m_curr_dir_text");
    m_curr_dir_text = new DynamicText(curr_elem->Child("GG::DynamicText"));

    curr_elem = &elem.Child("m_files_list");
    if (!(m_files_list = dynamic_cast<ListBox*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("FileDlg::FileDlg : Attempted to use a non-ListBox object as the files list box.");

    curr_elem = &elem.Child("m_files_edit");
    if (!(m_files_edit = dynamic_cast<Edit*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("FileDlg::FileDlg : Attempted to use a non-Edit object as the filename edit box when constructing a GG::FileDlg");

    curr_elem = &elem.Child("m_filter_list");
    if (!(m_filter_list = dynamic_cast<DropDownList*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("FileDlg::FileDlg : Attempted to use a non-DropDownList object as the file filters drop list.");

    curr_elem = &elem.Child("m_ok_button");
    if (!(m_ok_button = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("FileDlg::FileDlg : Attempted to use a non-Button object as the ok button.");

    curr_elem = &elem.Child("m_cancel_button");
    if (!(m_cancel_button = dynamic_cast<Button*>(App::GetApp()->GenerateWnd(curr_elem->Child(0)))))
        throw std::runtime_error("FileDlg::FileDlg : Attempted to use a non-Button object as the cancel button.");

    curr_elem = &elem.Child("m_files_label");
    m_files_label = new StaticText(curr_elem->Child("GG::StaticText"));

    curr_elem = &elem.Child("m_file_types_label");
    m_file_types_label = new StaticText(curr_elem->Child("GG::StaticText"));

    Init();
}

XMLElement FileDlg::XMLEncode() const
{
    XMLElement retval("GG::FileDlg");
    const_cast<FileDlg*>(this)->DetachSignalChildren();
    retval.AppendChild(Wnd::XMLEncode());

    XMLElement temp;

    temp = XMLElement("m_color");
    temp.AppendChild(m_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_border_color");
    temp.AppendChild(m_border_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_text_color");
    temp.AppendChild(m_text_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_button_color");
    temp.AppendChild(m_button_color.XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_font");
    temp.AppendChild(m_font->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_save");
    temp.SetAttribute("value", lexical_cast<string>(m_save));
    retval.AppendChild(temp);

    temp = XMLElement("m_file_filters");
    for (unsigned int i = 0; i < m_file_filters.size(); ++i) {
        XMLElement loop_temp("first");
        loop_temp.SetText(m_file_filters[i].first);
        temp.AppendChild(loop_temp);
        loop_temp = XMLElement("second");
        loop_temp.SetText(m_file_filters[i].second);
        temp.AppendChild(loop_temp);
    }
    retval.AppendChild(temp);

    temp = XMLElement("m_curr_dir_text");
    temp.AppendChild(m_curr_dir_text->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_files_list");
    m_files_list->Clear();
    temp.AppendChild(m_files_list->XMLEncode());
    retval.AppendChild(temp);
    const_cast<FileDlg*>(this)->UpdateList();

    temp = XMLElement("m_files_edit");
    m_files_edit->Clear();
    temp.AppendChild(m_files_edit->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_filter_list");
    m_filter_list->Clear();
    temp.AppendChild(m_filter_list->XMLEncode());
    retval.AppendChild(temp);
    const_cast<FileDlg*>(this)->PopulateFilters();

    temp = XMLElement("m_ok_button");
    temp.AppendChild(m_ok_button->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_cancel_button");
    temp.AppendChild(m_cancel_button->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_files_label");
    temp.AppendChild(m_files_label->XMLEncode());
    retval.AppendChild(temp);

    temp = XMLElement("m_file_types_label");
    temp.AppendChild(m_file_types_label->XMLEncode());
    retval.AppendChild(temp);

    const_cast<FileDlg*>(this)->AttachSignalChildren();

    return retval;
}

int FileDlg::Render()
{
    FlatRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_color, m_border_color, 1);
    return 1;
}

int FileDlg::Keypress(Key key, Uint32 key_mods)
{
    if (key == GGK_RETURN || key == GGK_KP_ENTER)
        OkClicked();
    else if (key == GGK_ESCAPE)
        CancelClicked();
    return 1;
}

void FileDlg::SetButtonColor(Clr color)
{
    m_button_color = color;
    m_ok_button->SetColor(color);
    m_cancel_button->SetColor(color);
}

void FileDlg::CreateChildren(const string& filename, bool multi, const string& font_filename, int pts)
{
    if (m_save)
        multi = false;

    const int H_SPACING = 10;
    const int V_SPACING = 10;
    const int USABLE_WIDTH = Width() - 4 * H_SPACING;
    const int BUTTON_WIDTH = USABLE_WIDTH / 4;

    m_files_edit = new Edit(0, 0, 1, filename, m_font, m_color); // use Edit's necessary-height calcs to determine height of the edit
    m_filter_list = new DropDownList(0, 0, 1, m_font->Lineskip(), m_font->Lineskip() * 3, m_color);
    m_filter_list->SetStyle(LB_NOSORT);

    const int BUTTON_HEIGHT = m_files_edit->Height(); // use the edit's height for the buttons as well

    m_curr_dir_text = new DynamicText(H_SPACING, V_SPACING / 2, "", m_font, m_text_color );
    m_files_label = new StaticText(0, Height() - (BUTTON_HEIGHT + V_SPACING) * 2, Width() - (3 * BUTTON_WIDTH + 3 * H_SPACING), BUTTON_HEIGHT, "File(s):", font_filename, pts, TF_RIGHT | TF_VCENTER, m_text_color);
    m_file_types_label = new StaticText(0, Height() - (BUTTON_HEIGHT + V_SPACING) * 1, Width() - (3 * BUTTON_WIDTH + 3 * H_SPACING), BUTTON_HEIGHT, "Type(s):", font_filename, pts, TF_RIGHT | TF_VCENTER, m_text_color);

    // determine the space needed to display both text labels in the chosen font; use this to expand the
    // edit as far as possible
    int labels_width = std::max(m_files_label->TextWidth(), m_file_types_label->TextWidth()) + H_SPACING;
    m_files_label->OffsetMove(labels_width - m_files_label->Wnd::Width() - H_SPACING / 2, 0);
    m_file_types_label->OffsetMove(labels_width - m_file_types_label->Wnd::Width() - H_SPACING / 2, 0);
    m_files_edit->SizeMove(labels_width, Height() - (BUTTON_HEIGHT + V_SPACING) * 2, Width() - (BUTTON_WIDTH + 2 * H_SPACING), Height() - (BUTTON_HEIGHT + 2 * V_SPACING));
    m_filter_list->SizeMove(labels_width, Height() - (BUTTON_HEIGHT + V_SPACING),     Width() - (BUTTON_WIDTH + 2 * H_SPACING), Height() - V_SPACING);

    if (!m_ok_button)
        m_ok_button = new Button(Width() - (BUTTON_WIDTH + H_SPACING), Height() - (BUTTON_HEIGHT + V_SPACING) * 2, BUTTON_WIDTH, BUTTON_HEIGHT, m_save ? "Save" : "Open", font_filename, pts, m_button_color);
    if (!m_cancel_button)
        m_cancel_button = new Button(Width() - (BUTTON_WIDTH + H_SPACING), Height() - (BUTTON_HEIGHT + V_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel", font_filename, pts, m_button_color);

    // finally, we can create the listbox with the files in it, sized to fill the available space
    int file_list_top = m_curr_dir_text->Height() + V_SPACING;
    m_files_list = new ListBox(H_SPACING, file_list_top, Width() - 2 * H_SPACING, Height() - (BUTTON_HEIGHT + V_SPACING) * 2 - file_list_top - V_SPACING, m_color);
    m_files_list->SetStyle(LB_NOSORT | (multi ? 0 : LB_SINGLESEL));

}

void FileDlg::AttachSignalChildren()
{
    AttachChild(m_files_edit);
    AttachChild(m_filter_list);
    AttachChild(m_ok_button);
    AttachChild(m_cancel_button);
    AttachChild(m_files_list);
}

void FileDlg::DetachSignalChildren()
{
    DetachChild(m_files_edit);
    DetachChild(m_filter_list);
    DetachChild(m_ok_button);
    DetachChild(m_cancel_button);
    DetachChild(m_files_list);
}

void FileDlg::Init()
{
    AttachSignalChildren();
    AttachChild(m_curr_dir_text);
    AttachChild(m_files_label);
    AttachChild(m_file_types_label);

    UpdateDirectoryText();
    PopulateFilters();
    UpdateList();

    Connect(m_ok_button->ClickedSignal(), &FileDlg::OkClicked, this);
    Connect(m_cancel_button->ClickedSignal(), &FileDlg::CancelClicked, this);
    Connect(m_files_list->SelChangedSignal(), &FileDlg::FileSetChanged, this);
    Connect(m_files_list->DoubleClickedSignal(), &FileDlg::FileDoubleClicked, this);
    Connect(m_files_edit->EditedSignal(), &FileDlg::FilesEditChanged, this);
    Connect(m_filter_list->SelChangedSignal(), &FileDlg::FilterChanged, this);
}

void FileDlg::OkClicked()
{
    bool results_valid = false;
    namespace fs = boost::filesystem;

    // parse contents of edit control to determine file names
    m_result.clear();
    string filenames;
    *m_files_edit >> filenames;
    typedef boost::tokenizer<boost::char_separator<char> > tok;
    boost::char_separator<char> sep(" \t");
    tok tokens(filenames, sep);
    set<string> files;
    for (tok::iterator it = tokens.begin(); it != tokens.end(); ++it)
        files.insert(*it);

    if (m_save) { // file save case
        if (m_ok_button->WindowText() != "Save") {
            OpenDirectory();
        } else if (files.size() == 1) {
            results_valid = true;
            string save_file = *files.begin();
            fs::path p = m_working_dir / save_file;
            m_result.insert(p.native_directory_string());
            // check to see if file already exists; if so, ask if it's ok to overwrite
            if (fs::exists(p)) {
                ThreeButtonDlg dlg(200, 125, save_file + " exists.\nOk to overwrite it?", m_font->FontName(), m_font->PointSize(), m_color, m_border_color, m_button_color, m_text_color, 2);
                dlg.Run();
                results_valid = (dlg.Result() == 0);
            } else { // ensure that the filename is valid
                // valid filenames are (for portability) one or more alphanumeric characters and underscores
                // in any combination, containing at most one period:
                boost::RegEx expr("[\\w]+[\\.]?[\\w]*");
                if (save_file != "." && !expr.Match(save_file)) { // HACK! !- "." guards against unknown segfault
                    // invalid file name; indicate this to the user
                    ThreeButtonDlg dlg(150, 75, "Invalid file name.", m_font->FontName(), m_font->PointSize(), m_color, m_border_color, m_button_color, m_text_color, 1);
                    dlg.Run();
                    results_valid = false;
                }
            }
        }
    } else { // file open case
        if (files.empty()) {
            OpenDirectory();
        } else { // ensure the file(s) are valid before returning them
            for (set<string>::iterator it = files.begin(); it != files.end(); ++it) {
                fs::path p = m_working_dir / *it;
                if (fs::exists(p)) {
                    m_result.insert(p.native_directory_string());
                    results_valid = true; // indicate validity only if at least one good file was found
                } else {
                    ThreeButtonDlg dlg(300, 75, "File \"" + (*it) + "\" does not exist.", m_font->FontName(), m_font->PointSize(), m_color, m_border_color, m_button_color, m_text_color, 1);
                    dlg.Run();
                    results_valid = false;
                    break;
                }
            }
        }
    }
    if (results_valid)
        m_done = true;
}

void FileDlg::FileSetChanged(const set<int>& files)
{
    string all_files;
    bool dir_selected = false;
    for (set<int>::const_iterator it = files.begin(); it != files.end(); ++it) {
        string filename = m_files_list->GetRow(*it)[0]->WindowText();
        if (filename[0] != '[') {
            if (!all_files.empty())
                all_files += " ";
            all_files += filename;
        } else {
            dir_selected = true;
        }
    }
    *m_files_edit << all_files;
    if (m_save && !dir_selected && m_ok_button->WindowText() != "Save")
        m_ok_button->SetText("Save");
    else if (m_save && dir_selected && m_ok_button->WindowText() == "Save")
        m_ok_button->SetText("Open");
}

void FileDlg::FileDoubleClicked(int n, const ListBox::Row& row)
{
    string filename = row[0]->WindowText();
    m_files_list->ClearSelection();
    m_files_list->SelectRow(n);
    OkClicked();
}

void FileDlg::FilesEditChanged(const string& str)
{
    if (m_save && m_ok_button->WindowText() != "Save")
        m_ok_button->SetText("Save");
}

void FileDlg::FilterChanged(int idx)
{
    UpdateList();
}

void FileDlg::SetWorkingDirectory(const boost::filesystem::path& p)
{
    m_files_edit->Clear();
    m_working_dir = p;
    UpdateDirectoryText();
    UpdateList();
}

void FileDlg::PopulateFilters()
{
    m_filter_list->Clear();
    if (m_file_filters.empty()) {
        m_file_types_label->Disable();
        m_filter_list->Disable();
    } else {
        for (unsigned int i = 0; i < m_file_filters.size(); ++i) {
            ListBox::Row row;
            row.push_back(m_file_filters[i].first, m_font, m_text_color);
            m_filter_list->Insert(row);
        }
        m_filter_list->Select(0);
    }
}

void FileDlg::UpdateList()
{
    m_files_list->Clear();
    namespace fs = boost::filesystem;
    fs::directory_iterator end_it;
    // filter out files and directories beginning with a period
    boost::RegEx period_regex("[^\\.]+.*");
    // create file filter regular expressions
    vector<boost::RegEx> filter_regexes;
    int idx = -1;
    if ((idx = m_filter_list->CurrentItemIndex()) != -1) {
        typedef boost::tokenizer<boost::char_separator<char> > tok;
        boost::char_separator<char> sep(", ");
        tok tokens(m_file_filters[idx].second, sep);
        string wildcard_str = "[\\.\\w]*"; // any combination of periods, underscores, and alphanumerics
        for (tok::iterator it = tokens.begin(); it != tokens.end(); ++it) {
            string str = *it;
            int posn = string::npos;
            int look_at = 0;
            while ((posn = str.find('*', look_at)) != static_cast<int>(string::npos)) {
                str.replace(posn, 1, wildcard_str);
                look_at = posn + wildcard_str.size();
            }
            filter_regexes.push_back(boost::RegEx(str));
        }
    }

    if (m_working_dir.string() != m_working_dir.root_path().string() &&
            m_working_dir.branch_path().string() != "") {
        ListBox::Row row;
        row.push_back("[..]", m_font);
        m_files_list->Insert(row);
    }
    for (fs::directory_iterator it(m_working_dir); it != end_it; ++it) {
        if (fs::exists(*it) && fs::is_directory(*it) && period_regex.Match(it->leaf())) {
            ListBox::Row row;
            row.push_back("[" + it->leaf() + "]", m_font);
            m_files_list->Insert(row);
        }
    }
    for (fs::directory_iterator it(m_working_dir); it != end_it; ++it) {
        if (fs::exists(*it) && !fs::is_directory(*it) && period_regex.Match(it->leaf())) {
            bool meets_filters = filter_regexes.empty();
            for (vector<boost::RegEx>::iterator regex_it = filter_regexes.begin();
                    regex_it != filter_regexes.end() && !meets_filters;
                    ++regex_it) {
                if (regex_it->Match(it->leaf()))
                    meets_filters = true;
            }
            if (meets_filters) {
                ListBox::Row row;
                row.push_back(it->leaf(), m_font);
                m_files_list->Insert(row);
            }
        }
    }
}

void FileDlg::UpdateDirectoryText()
{
    string str = m_working_dir.native_directory_string();
    const int H_SPACING = 10;
    while (m_font->TextExtent(str).x > Width() - 2 * H_SPACING) {
        unsigned int slash_idx = str.find('/', 1);
        unsigned int backslash_idx = str.find('\\', 1);
        if (slash_idx != string::npos) {
            slash_idx = str.find_first_not_of('/', slash_idx);
            str = "..." + str.substr(slash_idx);
        } else if (backslash_idx != string::npos) {
            backslash_idx = str.find_first_not_of('\\', backslash_idx);
            str = "..." + str.substr(backslash_idx);
        } else {
            break;
        }
    }
    *m_curr_dir_text << str;
}

void FileDlg::OpenDirectory()
{
    // see if there is a directory selected; if so open the directory.
    // if more than one is selected, take the first one
    const set<int>& sels = m_files_list->Selections();
    string directory;
    if (!sels.empty()) {
        directory = m_files_list->GetRow(*sels.begin())[0]->WindowText();
        if (directory.size() < 2 || directory[0] != '[')
            return;
        directory = directory.substr(1, directory.size() - 2); // strip off '[' and ']'
        if (directory == "..") {
            SetWorkingDirectory(m_working_dir.branch_path());
        } else {
            SetWorkingDirectory(m_working_dir / directory);
        }
        if (m_save && m_ok_button->WindowText() != "Save")
            m_ok_button->SetText("Save");
    }
}

} // namspace GG

