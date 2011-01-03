#include <GG/WndEditor.h>

#include <GG/Layout.h>

using namespace GG;

namespace {
    const X WND_EDITOR_WIDTH(400);
    const X WND_EDITOR_TEXT_WIDTH(125);

    template <class T>
    struct WrappedEditChangedFunctor
    {
        WrappedEditChangedFunctor(T& value, Edit* edit, boost::signal<void ()>& signal) :
            m_value(value),
            m_edit(edit),
            m_signal(signal)
        {}
        void operator()(const std::string& value_text)
        {
            try {
                int value = boost::lexical_cast<int>(value_text);
                m_value = T(value);
                m_edit->SetTextColor(CLR_BLACK);
                m_signal();
            } catch (const boost::bad_lexical_cast&) {
                m_edit->SetTextColor(CLR_RED);
            }
        }
    private:
        T& m_value;
        Edit* m_edit;
        boost::signal<void ()>& m_signal;
    };

    template <>
    struct WrappedEditChangedFunctor<unsigned char>
    {
        WrappedEditChangedFunctor(unsigned char& value, Edit* edit, boost::signal<void ()>& signal) :
            m_value(value),
            m_edit(edit),
            m_signal(signal)
        {}
        void operator()(const std::string& value_text)
        {
            try {
                int value = boost::lexical_cast<int>(value_text);
                if (value < 0 || 255 < value)
                    throw boost::bad_lexical_cast();
                m_value = value;
                m_edit->SetTextColor(CLR_BLACK);
                m_signal();
            } catch (const boost::bad_lexical_cast&) {
                m_edit->SetTextColor(CLR_RED);
            }
        }
    private:
        unsigned char& m_value;
        Edit* m_edit;
        boost::signal<void ()>& m_signal;
    };

    class MultiControlWrapper : public Control
    {
    public:
        MultiControlWrapper() :
            Control(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT),
            m_children(0)
        {
            m_layout = new Layout(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, 1, 1);
            AttachChild(m_layout);
        }
        virtual void Render() {}
        void Add(Wnd* w)
        { m_layout->Add(w, 0, m_children++); }
    private:
        Layout* m_layout;
        int m_children;
    };
}

namespace GG { namespace detail {
    const Y ATTRIBUTE_ROW_HEIGHT(22);
    const X ATTRIBUTE_ROW_CONTROL_WIDTH = WND_EDITOR_WIDTH - WND_EDITOR_TEXT_WIDTH - 14 - 4;
} }

////////////////////////////////////////////////
// GG::WndEditor
////////////////////////////////////////////////
WndEditor::WndEditor(Y h, const boost::shared_ptr<Font>& font) :
    Wnd(X0, Y0, WND_EDITOR_WIDTH, h),
    m_wnd(0),
    m_list_box(new ListBox(X0, Y0, WND_EDITOR_WIDTH, h, CLR_GRAY, CLR_WHITE)),
    m_font(font),
    m_label_font(GUI::GetGUI()->GetFont(font, font->PointSize() + 4)),
    m_current_flags_and_action()
{ Init(); }

const boost::shared_ptr<Font>& WndEditor::GetFont() const
{ return m_font; }

const Wnd* WndEditor::GetWnd() const
{ return m_wnd; }

void WndEditor::Render ()
{
    for (ListBox::iterator it = m_list_box->begin(); it != m_list_box->end(); ++it) {
        if (AttributeRowBase* row = dynamic_cast<AttributeRowBase*>(*it))
            row->Refresh();
    }
}

void WndEditor::SetWnd(Wnd* wnd, const std::string& name/* = ""*/)
{
    m_wnd = wnd;
    m_list_box->Clear();
    if (name != "") {
        ListBox::Row* row = new ListBox::Row();
        row->push_back("Name", m_font, CLR_BLACK);
        Edit* edit = new Edit(X0, Y0, X1, "", m_font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
        edit->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, edit->Height()));
        row->Resize(edit->Size());
        row->push_back(edit);
        edit->SetText(name);
        Connect(edit->FocusUpdateSignal, &WndEditor::NameChangedSlot, this);
        m_list_box->Insert(row);
    }
    if (wnd)
        wnd->DefineAttributes(this);
}

void WndEditor::Label(const std::string& name)
{
    ListBox::Row* label = new ListBox::Row();
    label->push_back(label->CreateControl(name, m_label_font, CLR_BLACK));
    m_list_box->Insert(label);
}

void WndEditor::Attribute(AttributeRowBase* row)
{
    m_list_box->Insert(row);
    Connect(row->ChangedSignal, &WndEditor::AttributeChangedSlot, this);
}

void WndEditor::EndFlags()
{ m_current_flags_and_action = boost::any(); }

void WndEditor::Init()
{
    m_list_box->SetStyle(LIST_NOSORT | LIST_NOSEL);
    m_list_box->SetNumCols(2);
    m_list_box->SetColWidth(0, WND_EDITOR_TEXT_WIDTH - 2);
    m_list_box->SetColWidth(1, WND_EDITOR_WIDTH - WND_EDITOR_TEXT_WIDTH - 14 - 2);
    m_list_box->LockColWidths();
    AttachChild(m_list_box);
}

void WndEditor::AttributeChangedSlot()
{
    for (ListBox::iterator it = m_list_box->begin(); it != m_list_box->end(); ++it) {
        if (AttributeRowBase* row = dynamic_cast<AttributeRowBase*>(*it))
            row->Update();
    }
    WndChangedSignal(m_wnd);
}

void WndEditor::NameChangedSlot(const std::string& name)
{ WndNameChangedSignal(m_wnd, name); }

////////////////////////////////////////////////
// GG::AttributeRowBase
////////////////////////////////////////////////
void AttributeRowBase::Refresh()
{}

void AttributeRowBase::Update()
{}

////////////////////////////////////////////////
// GG::AttributeRow<Pt>
////////////////////////////////////////////////
AttributeRow<Pt>::AttributeRow(const std::string& name, Pt& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_x_edit(0),
    m_y_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    MultiControlWrapper* edits = new MultiControlWrapper();
    m_x_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_y_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    edits->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, m_x_edit->Height()));
    *m_x_edit << m_value.x;
    *m_y_edit << m_value.y;
    edits->Add(m_x_edit);
    edits->Add(m_y_edit);
    Resize(edits->Size());
    m_x_connection = Connect(m_x_edit->FocusUpdateSignal, WrappedEditChangedFunctor<X>(m_value.x, m_x_edit, ChangedSignal));
    m_y_connection = Connect(m_y_edit->FocusUpdateSignal, WrappedEditChangedFunctor<Y>(m_value.y, m_y_edit, ChangedSignal));
    push_back(edits);
}

void AttributeRow<Pt>::Update()
{
    m_x_connection.block();
    m_y_connection.block();
    *m_x_edit << m_value.x;
    *m_y_edit << m_value.y;
    m_x_connection.unblock();
    m_y_connection.unblock();
}

////////////////////////////////////////////////
// GG::AttributeRow<Clr>
////////////////////////////////////////////////
AttributeRow<Clr>::AttributeRow(const std::string& name, Clr& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_color_button(0),
    m_font(font)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_color_button = new ColorDlg::ColorButton(CLR_GRAY);
    m_color_button->SetRepresentedColor(m_value);
    m_color_button->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, Y(22)));
    Connect(m_color_button->ClickedSignal, &AttributeRow::ColorButtonClicked, this);
    push_back(m_color_button);
}

void AttributeRow<Clr>::ColorButtonClicked()
{
    ColorDlg dlg(X0, Y0, m_value, m_font, CLR_GRAY, CLR_GRAY);
    dlg.MoveTo(Pt((GUI::GetGUI()->AppWidth() - dlg.Width()) / 2,
                  (GUI::GetGUI()->AppHeight() - dlg.Height()) / 2));
    dlg.Run();
    if (dlg.ColorWasSelected()) {
        m_color_button->SetRepresentedColor(dlg.Result());
        m_value = dlg.Result();
        ValueChangedSignal(m_value);
        ChangedSignal();
    }
}

void AttributeRow<Clr>::Update()
{ m_color_button->SetRepresentedColor(m_value); }

////////////////////////////////////////////////
// GG::AttributeRow<bool>
////////////////////////////////////////////////
AttributeRow<bool>::AttributeRow(const std::string& name, bool& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_radio_button_group(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_radio_button_group = new RadioButtonGroup(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, HORIZONTAL);
    m_radio_button_group->AddButton("True", font, FORMAT_LEFT, CLR_GRAY);
    m_radio_button_group->AddButton("False", font, FORMAT_LEFT, CLR_GRAY);
    m_radio_button_group->SetCheck(!value);
    m_button_group_connection = Connect(m_radio_button_group->ButtonChangedSignal, &AttributeRow::SelectionChanged, this);
    push_back(m_radio_button_group);
}

void AttributeRow<bool>::SelectionChanged(std::size_t selection)
{
    m_value = !selection;
    ValueChangedSignal(m_value);
    ChangedSignal();
}

void AttributeRow<bool>::Update()
{
    m_button_group_connection.block();
    m_radio_button_group->SetCheck(!m_value);
    m_button_group_connection.unblock();
}

////////////////////////////////////////////////
// GG::AttributeRow<boost::shared_ptr<Font> >
////////////////////////////////////////////////
AttributeRow<boost::shared_ptr<Font> >::AttributeRow(const std::string& name, boost::shared_ptr<Font>& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_filename_edit(0),
    m_points_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    MultiControlWrapper* edits = new MultiControlWrapper();
    m_filename_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_points_edit = new Edit(X0, Y0, X1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    edits->Resize(Pt(detail::ATTRIBUTE_ROW_CONTROL_WIDTH, m_filename_edit->Height()));
    *m_filename_edit << m_value->FontName();
    *m_points_edit << m_value->PointSize();
    edits->Add(m_filename_edit);
    edits->Add(m_points_edit);
    Resize(edits->Size());
    m_filename_connection = Connect(m_filename_edit->FocusUpdateSignal, &AttributeRow::FilenameChanged, this);
    m_points_connection = Connect(m_points_edit->FocusUpdateSignal, &AttributeRow::PointsChanged, this);
    push_back(edits);
}

void AttributeRow<boost::shared_ptr<Font> >::FilenameChanged(const std::string& filename_text)
{
    try {
        boost::shared_ptr<Font> font = GUI::GetGUI()->GetFont(filename_text, m_value->PointSize());
        m_value = font;
        m_filename_edit->SetTextColor(CLR_BLACK);
        ValueChangedSignal(m_value);
        ChangedSignal();
    } catch (const Font::Exception&) {
        m_filename_edit->SetTextColor(CLR_RED);
    }
}

void AttributeRow<boost::shared_ptr<Font> >::PointsChanged(const std::string& points_text)
{
    try {
        int points = boost::lexical_cast<int>(points_text);
        if (points < 4 || 200 < points)
            throw boost::bad_lexical_cast();
        boost::shared_ptr<Font> font = GUI::GetGUI()->GetFont(m_value, points);
        m_value = font;
        m_points_edit->SetTextColor(CLR_BLACK);
        ValueChangedSignal(m_value);
        ChangedSignal();
    } catch (const boost::bad_lexical_cast&) {
        m_points_edit->SetTextColor(CLR_RED);
    } catch (const Font::Exception&) {
        m_points_edit->SetTextColor(CLR_RED);
    }
}

void AttributeRow<boost::shared_ptr<Font> >::Update()
{
    m_filename_connection.block();
    m_points_connection.block();
    *m_filename_edit << m_value->FontName();
    *m_points_edit << m_value->PointSize();
    m_filename_connection.unblock();
    m_points_connection.unblock();
}

////////////////////////////////////////////////
// GG::ConstAttributeRow<Pt>
////////////////////////////////////////////////
ConstAttributeRow<Pt>::ConstAttributeRow(const std::string& name, const Pt& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_value_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    std::stringstream value_stream;
    value_stream << "(" << m_value.x << ", " << m_value.y << ")";
    m_value_text = new TextControl(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, value_stream.str(), font, CLR_BLACK, FORMAT_LEFT);
    push_back(m_value_text);
}

void ConstAttributeRow<Pt>::Refresh()
{
    std::stringstream value_stream;
    value_stream << "(" << m_value.x << ", " << m_value.y << ")";
    m_value_text->SetText(value_stream.str());
}

////////////////////////////////////////////////
// GG::ConstAttributeRow<Clr>
////////////////////////////////////////////////
ConstAttributeRow<Clr>::ConstAttributeRow(const std::string& name, const Clr& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_value_text(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    std::stringstream value_stream;
    value_stream << "(" << m_value.r << ", " << m_value.g << ", " << m_value.b << ", " << m_value.a << ")";
    m_value_text = new TextControl(X0, Y0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, value_stream.str(), font, CLR_BLACK, FORMAT_LEFT);
    push_back(m_value_text);
}

void ConstAttributeRow<Clr>::Refresh()
{
    std::stringstream value_stream;
    value_stream << "(" << m_value.r << ", " << m_value.g << ", " << m_value.b << ", " << m_value.a << ")";
    m_value_text->SetText(value_stream.str());
}
