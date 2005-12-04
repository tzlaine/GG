#include "GGWndEditor.h"

#include "GGLayout.h"

using namespace GG;

namespace {
    const int WND_EDITOR_WIDTH = 400;
    const int WND_EDITOR_TEXT_WIDTH = 125;

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
                m_value = value;
                m_edit->SetTextColor(CLR_BLACK);
                m_signal();
            } catch (const boost::bad_lexical_cast& e) {
                m_edit->SetTextColor(CLR_RED);
            }
        }
    private:
        T& m_value;
        Edit* m_edit;
        boost::signal<void ()>& m_signal;
    };

    template <>
    struct WrappedEditChangedFunctor<Uint8>
    {
        WrappedEditChangedFunctor(Uint8& value, Edit* edit, boost::signal<void ()>& signal) :
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
            } catch (const boost::bad_lexical_cast& e) {
                m_edit->SetTextColor(CLR_RED);
            }
        }
    private:
        Uint8& m_value;
        Edit* m_edit;
        boost::signal<void ()>& m_signal;
    };

    class MultiControlWrapper : public Control
    {
    public:
        MultiControlWrapper() :
            Control(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT),
            m_children(0)
        {
            m_layout = new Layout(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, 1, 1);
            AttachChild(m_layout);
        }
        virtual void Render() {}
        void Add(Wnd* w)
        {
            m_layout->Add(w, 0, m_children++);
        }
    private:
        Layout* m_layout;
        int m_children;
    };
}

namespace GG { namespace detail {
    const int ATTRIBUTE_ROW_HEIGHT = 22;
    const int ATTRIBUTE_ROW_CONTROL_WIDTH = WND_EDITOR_WIDTH - WND_EDITOR_TEXT_WIDTH - 14 - 4;
} }

////////////////////////////////////////////////
// GG::WndEditor
////////////////////////////////////////////////
WndEditor::WndEditor(int h, const std::string& font_filename, int pts) :
    Wnd(0, 0, WND_EDITOR_WIDTH, h),
    m_wnd(0),
    m_list_box(new ListBox(0, 0, WND_EDITOR_WIDTH, h, CLR_GRAY, CLR_WHITE)),
    m_font(App::GetApp()->GetFont(font_filename, pts)),
    m_label_font(App::GetApp()->GetFont(font_filename, pts + 4)),
    m_current_flags(0)
{
    Init();
}

WndEditor::WndEditor(int h, const boost::shared_ptr<Font>& font) :
    Wnd(0, 0, WND_EDITOR_WIDTH, h),
    m_wnd(0),
    m_list_box(new ListBox(0, 0, WND_EDITOR_WIDTH, h, CLR_GRAY, CLR_WHITE)),
    m_font(font),
    m_label_font(App::GetApp()->GetFont(font->FontName(), font->PointSize() + 4)),
    m_current_flags(0)
{
    Init();
}

void WndEditor::Render ()
{
    for (int i = 0; i < m_list_box->NumRows(); ++i) {
        AttributeRowBase* row = 
            dynamic_cast<AttributeRowBase*>(&m_list_box->GetRow(i));
        if (row)
            row->Update();
    }
}

void WndEditor::SetWnd(Wnd* wnd)
{
    m_wnd = wnd;
    m_list_box->Clear();
    if (wnd)
        wnd->DefineAttributes(this);
}

void WndEditor::Label(const std::string& name)
{
    ListBox::Row* label = new ListBox::Row();
    label->push_back(ListBox::Row::CreateControl(name, m_label_font, CLR_BLACK));
    m_list_box->Insert(label);
}

void WndEditor::BeginFlags(Uint32& flags)
{
    m_current_flags = &flags;
    assert(m_current_flags);
}

void WndEditor::EndFlags()
{
    m_current_flags = 0;
}

void WndEditor::Init()
{
    m_list_box->SetStyle(LB_NOSORT | LB_NOSEL);
    m_list_box->SetNumCols(2);
    m_list_box->SetColWidth(0, WND_EDITOR_TEXT_WIDTH - 2);
    m_list_box->SetColWidth(1, WND_EDITOR_WIDTH - WND_EDITOR_TEXT_WIDTH - 14 - 2);
    m_list_box->LockColWidths();
    AttachChild(m_list_box);
}

void WndEditor::AttributeChangedSlot()
{
    WndChangedSignal(m_wnd);
}

////////////////////////////////////////////////
// GG::AttributeRowBase
////////////////////////////////////////////////
void AttributeRowBase::Update()
{
}

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
    m_x_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_y_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    *m_x_edit << m_value.x;
    *m_y_edit << m_value.y;
    edits->Add(m_x_edit);
    edits->Add(m_y_edit);
    Connect(m_x_edit->FocusUpdateSignal, WrappedEditChangedFunctor<int>(m_value.x, m_x_edit, ChangedSignal));
    Connect(m_y_edit->FocusUpdateSignal, WrappedEditChangedFunctor<int>(m_value.y, m_y_edit, ChangedSignal));
    push_back(edits);
}

////////////////////////////////////////////////
// GG::AttributeRow<Clr>
////////////////////////////////////////////////
AttributeRow<Clr>::AttributeRow(const std::string& name, Clr& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_red_edit(0),
    m_green_edit(0),
    m_blue_edit(0),
    m_alpha_edit(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    MultiControlWrapper* edits = new MultiControlWrapper();
    m_red_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_green_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_blue_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_alpha_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    *m_red_edit << static_cast<int>(m_value.r);
    *m_green_edit << static_cast<int>(m_value.g);
    *m_blue_edit << static_cast<int>(m_value.b);
    *m_alpha_edit << static_cast<int>(m_value.a);
    edits->Add(m_red_edit);
    edits->Add(m_green_edit);
    edits->Add(m_blue_edit);
    edits->Add(m_alpha_edit);
    Connect(m_red_edit->FocusUpdateSignal, WrappedEditChangedFunctor<Uint8>(m_value.r, m_red_edit, ChangedSignal));
    Connect(m_green_edit->FocusUpdateSignal, WrappedEditChangedFunctor<Uint8>(m_value.g, m_green_edit, ChangedSignal));
    Connect(m_blue_edit->FocusUpdateSignal, WrappedEditChangedFunctor<Uint8>(m_value.b, m_blue_edit, ChangedSignal));
    Connect(m_alpha_edit->FocusUpdateSignal, WrappedEditChangedFunctor<Uint8>(m_value.a, m_alpha_edit, ChangedSignal));
    push_back(edits);
}

////////////////////////////////////////////////
// GG::AttributeRow<bool>
////////////////////////////////////////////////
AttributeRow<bool>::AttributeRow(const std::string& name, bool& value, const boost::shared_ptr<Font>& font) :
    m_value(value),
    m_bool_drop_list(0)
{
    push_back(CreateControl(name, font, CLR_BLACK));
    m_bool_drop_list = new DropDownList(0, 0, detail::ATTRIBUTE_ROW_CONTROL_WIDTH, detail::ATTRIBUTE_ROW_HEIGHT, detail::ATTRIBUTE_ROW_HEIGHT * 2 + 4, CLR_GRAY, CLR_WHITE);
    m_bool_drop_list->SetStyle(LB_NOSORT);
    Row* row = new ListBox::Row();
    row->push_back(CreateControl("True", font, CLR_BLACK));
    m_bool_drop_list->Insert(row);
    row = new ListBox::Row();
    row->push_back(CreateControl("False", font, CLR_BLACK));
    m_bool_drop_list->Insert(row);
    push_back(m_bool_drop_list);
    m_bool_drop_list->Select(!m_value);
    Connect(m_bool_drop_list->SelChangedSignal, &AttributeRow::SelectionChanged, this);
}

void AttributeRow<bool>::SelectionChanged(int selection)
{
    m_value = !selection;
    ChangedSignal();
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
    m_filename_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    m_points_edit = new Edit(0, 0, 1, 1, "", font, CLR_GRAY, CLR_BLACK, CLR_WHITE);
    *m_filename_edit << m_value->FontName();
    *m_points_edit << m_value->PointSize();
    edits->Add(m_filename_edit);
    edits->Add(m_points_edit);
    Connect(m_filename_edit->FocusUpdateSignal, &AttributeRow::FilenameChanged, this);
    Connect(m_points_edit->FocusUpdateSignal, &AttributeRow::PointsChanged, this);
    push_back(edits);
}

void AttributeRow<boost::shared_ptr<Font> >::FilenameChanged(const std::string& filename_text)
{
    try {
        boost::shared_ptr<Font> font = App::GetApp()->GetFont(filename_text, m_value->PointSize());
        m_value = font;
        m_filename_edit->SetTextColor(CLR_BLACK);
        ChangedSignal();
    } catch (const Font::Exception& e) {
        m_filename_edit->SetTextColor(CLR_RED);
    }
}

void AttributeRow<boost::shared_ptr<Font> >::PointsChanged(const std::string& points_text)
{
    try {
        int points = boost::lexical_cast<int>(points_text);
        if (points < 4 || 150 < points)
            throw boost::bad_lexical_cast();
        boost::shared_ptr<Font> font = App::GetApp()->GetFont(m_value->FontName(), points);
        m_value = font;
        m_points_edit->SetTextColor(CLR_BLACK);
        ChangedSignal();
    } catch (const boost::bad_lexical_cast& e) {
        m_points_edit->SetTextColor(CLR_RED);
    } catch (const Font::Exception& e) {
        m_points_edit->SetTextColor(CLR_RED);
    }
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
    m_value_text = new TextControl(0, 0, value_stream.str(), font, CLR_BLACK);
    push_back(m_value_text);
}

void ConstAttributeRow<Pt>::Update()
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
    m_value_text = new TextControl(0, 0, value_stream.str(), font, CLR_BLACK);
    push_back(m_value_text);
}

void ConstAttributeRow<Clr>::Update()
{
    std::stringstream value_stream;
    value_stream << "(" << m_value.r << ", " << m_value.g << ", " << m_value.b << ", " << m_value.a << ")";
    m_value_text->SetText(value_stream.str());
}
