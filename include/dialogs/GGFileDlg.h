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

#ifndef _GGFileDlg_h_
#define _GGFileDlg_h_

#ifndef _GGWnd_h_
#include "../GGWnd.h"
#endif

#ifndef _GGListBox_h_
#include "../GGListBox.h"
#endif

#include <boost/filesystem/path.hpp>

namespace GG {

class TextControl;
class Edit;
class DropDownList;
class Button;
class Font;

/** the default file open/save dialog box.  This dialog, like all the common GG dialogs, is modal.  It asks the user
    for one or more filenames, which the caller may retrieve with a call to Result() after the dialog is closed.*/
class GG_API FileDlg : public Wnd
{
public:
    /** \name Structors */ //@{
    /** basic ctor.  Parameters \a directory and \a filename pass an initial directory and filename to the dialog, 
        if desired (such as when "Save As" is called in an app, and there is a current filename).  If \a directory is "", 
        the initial directory is the WorkingDirectory(), otherwise the directory given is taken to be relative to 
        boost::filesystem::initial_path().  \a save indicates whether this is a save or load dialog; \a multi indicates whether 
        multiple file selections are allowed.*/
    FileDlg(const string& directory, const string& filename, bool save, bool multi, const shared_ptr<Font>& font, Clr color, 
	    Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

    /** basic ctor.  Parameters \a directory and \a filename pass an initial directory and filename to the dialog, 
        if desired (such as when "Save As" is called in an app, and there is a current filename).  If \a directory is "", 
        the initial directory is the WorkingDirectory(), otherwise the directory given is taken to be relative to 
        boost::filesystem::initial_path().  \a save indicates whether this is a save or load dialog; \a multi indicates whether 
        multiple file selections are allowed.*/
    FileDlg(const string& directory, const string& filename, bool save, bool multi, const string& font_filename, int pts, Clr color, 
	    Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

    /** ctor that allows specification of allowed file types.  Parameters \a directory and \a filename pass an initial directory 
        and filename to the dialog, if desired (such as when "Save As" is called in an app, and there is a 
        current filename).  If \a directory is "", the initial directory is the WorkingDirectory(), otherwise the directory given 
        is taken to be relative to boost::filesystem::initial_path().  \a save indicates whether 
        this is a save or load dialog; \a multi indicates whether multiple file selections are allowed; \a types is a vector
        of pairs of strings containing the allowed file types; \a user_edit_types indicates whether the user should be allowed 
        to edit the file types.  Each pair in the \a types parameter contains a description of the file type in its .first 
        member, and wildcarded file types in its .second member.  For example, an entry might be ("Text Files (*.txt)", 
        "*.txt"). Only the '*' character is supported as a wildcard.  More than one wildcard expression can be specified in 
        a filter; if so, they must be separated by a comma and exactly one space (", ").  Each filter is considered OR-ed 
        together with the others, so passing "*.tga, *.png" specifies listing any file that is either a Targa or a PNG file.  
        Note that an empty filter is considered to match all files, so ("All Files", "") is perfectly correct.*/
    FileDlg(const string& directory, const string& filename, bool save, bool multi, const vector<pair<string, string> >& types,
	    const shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

    /** ctor that allows specification of allowed file types.  Parameters \a directory and \a filename pass an initial directory 
        and filename to the dialog, if desired (such as when "Save As" is called in an app, and there is a 
        current filename).  If \a directory is "", the initial directory is the WorkingDirectory(), otherwise the directory given 
        is taken to be relative to boost::filesystem::initial_path().  \a save indicates whether 
        this is a save or load dialog; \a multi indicates whether multiple file selections are allowed; \a types is a vector
        of pairs of strings containing the allowed file types; \a user_edit_types indicates whether the user should be allowed 
        to edit the file types.  Each pair in the \a types parameter contains a description of the file type in its .first 
        member, and wildcarded file types in its .second member.  For example, an entry might be ("Text Files (*.txt)", 
        "*.txt"). Only the '*' character is supported as a wildcard.  More than one wildcard expression can be specified in 
        a filter; if so, they must be separated by a comma and exactly one space (", ").  Each filter is considered OR-ed 
        together with the others, so passing "*.tga, *.png" specifies listing any file that is either a Targa or a PNG file.  
        Note that an empty filter is considered to match all files, so ("All Files", "") is perfectly correct.*/
    FileDlg(const string& directory, const string& filename, bool save, bool multi, const vector<pair<string, string> >& types,
	    const string& font_filename, int pts, Clr color, Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

    FileDlg(const XMLElement& elem); ///< ctor that constructs a StateButton object from an FileDlg. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a FileDlg object
    //@}

    /** \name Accessors */ //@{
    Clr ButtonColor() const     {return m_button_color;}    ///< returns the color of the buttons in the dialog
    set<string> Result() const  {return m_result;}          ///< returns a set of strings that contains the files chosen by the user; there will be only one file if \a multi == false was passed to the ctor

    virtual XMLElement XMLEncode() const; ///< constructs an XMLElement from a FileDlg object
    //@}
   
    /** \name Mutators */ //@{
    virtual bool Render();
    virtual void Keypress(Key key, Uint32 key_mods);

    void SetButtonColor(Clr color);  ///< sets the color used to render the dialog's buttons
    //@}

    /** returns the current directory (the one that will be used by default on the next invocation of FileDlg::Run()) */
    static const boost::filesystem::path& WorkingDirectory() {return s_working_dir;}

private:
    enum {WIDTH = 400, HEIGHT = 350}; ///< default width and height values for the dialog, in pixels
   
    void CreateChildren(const string& filename, bool multi, const string& font_filename, int pts);
    void AttachSignalChildren();
    void DetachSignalChildren();
    void Init(const string& directory);
    void OkClicked();
    void CancelClicked() {m_done = true; m_result.clear();}
    void FileSetChanged(const set<int>& files);
    void FileDoubleClicked(int n, const ListBox::Row* row);
    void FilesEditChanged(const string& str);
    void FilterChanged(int idx);
    void SetWorkingDirectory(const boost::filesystem::path& p);
    void PopulateFilters();
    void UpdateList();
    void UpdateDirectoryText();
    void OpenDirectory();

    Clr              m_color;
    Clr              m_border_color;
    Clr              m_text_color;
    Clr              m_button_color;
    shared_ptr<Font> m_font;
   
    bool             m_save;
    vector<pair<string, string> > 
                     m_file_filters;
    set<string>      m_result;
   
    TextControl*     m_curr_dir_text;
    ListBox*         m_files_list;
    Edit*            m_files_edit;
    DropDownList*    m_filter_list;
    Button*          m_ok_button;
    Button*          m_cancel_button;
    TextControl*     m_files_label;
    TextControl*     m_file_types_label;
   
    static boost::filesystem::path s_working_dir; ///< declared static so each instance of FileDlg opens up the same directory
};

} // namspace GG

#endif // _GGFileDlg_h_



