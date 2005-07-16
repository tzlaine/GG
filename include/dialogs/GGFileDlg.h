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

/** \file GGFileDlg.h
    Contains the standard GG file dialog. */

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
    /** exception class \see GG::GGEXCEPTION */
    GGEXCEPTION(InitialDirectoryDoesNotExistException);

    /** \name Structors */ //@{
    /** basic ctor.  Parameters \a directory and \a filename pass an initial directory and filename to the dialog, 
        if desired (such as when "Save As" is called in an app, and there is a current filename).  If \a directory is "", 
        the initial directory is the WorkingDirectory(), otherwise the directory given is taken to be relative to 
        boost::filesystem::initial_path().  \a save indicates whether this is a save or load dialog; \a multi indicates whether 
        multiple file selections are allowed.  \throw GG::InitialDirectoryDoesNotExistException Will throw 
        GG::InitialDirectoryDoesNotExistException when \a directory is invalid. */
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi, const boost::shared_ptr<Font>& font, Clr color, 
            Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

    /** basic ctor.  Parameters \a directory and \a filename pass an initial directory and filename to the dialog, 
        if desired (such as when "Save As" is called in an app, and there is a current filename).  If \a directory is "", 
        the initial directory is the WorkingDirectory(), otherwise the directory given is taken to be relative to 
        boost::filesystem::initial_path().  \a save indicates whether this is a save or load dialog; \a multi indicates whether 
        multiple file selections are allowed.  \throw GG::InitialDirectoryDoesNotExistException Will throw 
        GG::InitialDirectoryDoesNotExistException when \a directory is invalid. */
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi, const std::string& font_filename, int pts, Clr color, 
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
        Note that an empty filter is considered to match all files, so ("All Files", "") is perfectly correct.  
        \throw GG::InitialDirectoryDoesNotExistException Will throw GG::InitialDirectoryDoesNotExistException when \a directory 
        is invalid. */
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi, const std::vector<std::pair<std::string, std::string> >& types,
            const boost::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);

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
        Note that an empty filter is considered to match all files, so ("All Files", "") is perfectly correct.  
        \throw GG::InitialDirectoryDoesNotExistException Will throw GG::InitialDirectoryDoesNotExistException when \a directory is 
        invalid. */
    FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi, const std::vector<std::pair<std::string, std::string> >& types,
            const std::string& font_filename, int pts, Clr color, Clr border_color, Clr text_color = CLR_BLACK, Button* ok = 0, Button* cancel = 0);
    //@}

    /** \name Accessors */ //@{
    Clr ButtonColor() const; ///< returns the color of the buttons in the dialog

    std::set<std::string> Result() const; ///< returns a set of strings that contains the files chosen by the user; there will be only one file if \a multi == false was passed to the ctor
    //@}

    /** \name Mutators */ //@{
    virtual bool Render();
    virtual void Keypress(Key key, Uint32 key_mods);

    void SetButtonColor(Clr color);                              ///< sets the color used to render the dialog's buttons
    void SetFilesString(const std::string& files_str);           ///< sets the text label next to the files edit box to \a files_str (this is "File(s):" by default)
    void SetFileTypesString(const std::string& files_types_str); ///< sets the text label next to the file types dropdown list to \a file_types_str (this is "Type(s):" by default)
    void SetSaveString(const std::string& save_str);             ///< sets the text of the ok button in its "save" state to \a save_str (this is "Save" by default)
    void SetOpenString(const std::string& open_str);             ///< sets the text of the ok button in its "open" state to \a open_str (this is "Open" by default)
    void SetCancelString(const std::string& cancel_str);         ///< sets the text of the cancel button to \a cancel_str (this is "Cancel" by default)
    //@}

    /** returns the current directory (the one that will be used by default on the next invocation of FileDlg::Run()) */
    static const boost::filesystem::path& WorkingDirectory();

protected:
    enum {WIDTH = 400, HEIGHT = 350}; ///< default width and height values for the dialog, in pixels

    /** \name Structors */ //@{
    FileDlg(); ///< default ctor
    //@}

private:
    void CreateChildren(const std::string& filename, bool multi, const std::string& font_filename, int pts);
    void PlaceLabelsAndEdits(int button_width, int button_height);
    void AttachSignalChildren();
    void DetachSignalChildren();
    void Init(const std::string& directory);
    void ConnectSignals();
    void OkClicked();
    void CancelClicked();
    void FileSetChanged(const std::set<int>& files);
    void FileDoubleClicked(int n, const boost::shared_ptr<ListBox::Row>& row);
    void FilesEditChanged(const std::string& str);
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
    boost::shared_ptr<Font>
                     m_font;

    bool             m_save;
    std::vector<std::pair<std::string, std::string> > 
                     m_file_filters;
    std::set<std::string>
                     m_result;

    std::string      m_save_str;
    std::string      m_open_str;
    std::string      m_cancel_str;

    TextControl*     m_curr_dir_text;
    ListBox*         m_files_list;
    Edit*            m_files_edit;
    DropDownList*    m_filter_list;
    Button*          m_ok_button;
    Button*          m_cancel_button;
    TextControl*     m_files_label;
    TextControl*     m_file_types_label;

    static boost::filesystem::path s_working_dir; ///< declared static so each instance of FileDlg opens up the same directory

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

} // namespace GG

// template implementations
template <class Archive>
void GG::FileDlg::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Wnd)
        & BOOST_SERIALIZATION_NVP(m_color)
        & BOOST_SERIALIZATION_NVP(m_border_color)
        & BOOST_SERIALIZATION_NVP(m_text_color)
        & BOOST_SERIALIZATION_NVP(m_button_color)
        & BOOST_SERIALIZATION_NVP(m_font)
        & BOOST_SERIALIZATION_NVP(m_save)
        & BOOST_SERIALIZATION_NVP(m_file_filters)
        & BOOST_SERIALIZATION_NVP(m_result)
        & BOOST_SERIALIZATION_NVP(m_save_str)
        & BOOST_SERIALIZATION_NVP(m_open_str)
        & BOOST_SERIALIZATION_NVP(m_cancel_str)
        & BOOST_SERIALIZATION_NVP(m_curr_dir_text)
        & BOOST_SERIALIZATION_NVP(m_files_list)
        & BOOST_SERIALIZATION_NVP(m_files_edit)
        & BOOST_SERIALIZATION_NVP(m_filter_list)
        & BOOST_SERIALIZATION_NVP(m_ok_button)
        & BOOST_SERIALIZATION_NVP(m_cancel_button)
        & BOOST_SERIALIZATION_NVP(m_files_label)
        & BOOST_SERIALIZATION_NVP(m_file_types_label);

    if (Archive::is_loading::value)
        ConnectSignals();
}

#endif // _GGFileDlg_h_
