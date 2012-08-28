#if USE_SDL_BACKEND
#include "SDLBackend.h"
#else
#include "OgreBackend.h"
#endif

#include <GG/EveGlue.h>
#include <GG/EveParser.h>
#include <GG/Filesystem.h>
#include <GG/StyleFactory.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/Timer.h>
#include <GG/Wnd.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/future/cursor.hpp>
#include <GG/adobe/future/modal_dialog_interface.hpp>

#include <boost/filesystem.hpp>

#include <fstream>


using namespace GG;

bool ButtonHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return false; }

ListBox::Row* MakeKeyValueItem(const adobe::dictionary_t& parameters)
{
    boost::shared_ptr<Font> font = GUI::GetGUI()->GetStyleFactory()->DefaultFont();
    ListBox::Row* retval = new ListBox::Row(X(100), font->Lineskip() + 16, "");
    retval->push_back(new Edit(X0, Y0, X(100), "", font, CLR_GRAY));
    retval->push_back(new Edit(X0, Y0, X(100), "", font, CLR_GRAY));
    return retval;
}

void CustomInit(const boost::filesystem::path& scripts_path)
{
    boost::filesystem::path functions_path = scripts_path / "functions.fn";
    adobe::file_slurp functions_file_content(functions_path);

    AdamFunctions adam_functions;
    ParseFunctions(functions_file_content.c_str(), functions_path.string(), adam_functions);

    RowFactory row_factory;
    row_factory[adobe::static_name_t("dialog_parameter")] = &MakeKeyValueItem;
    row_factory[adobe::static_name_t("tab_group_item")] = &MakeKeyValueItem;
    row_factory[adobe::static_name_t("relate_clause")] = &MakeKeyValueItem;
    row_factory[adobe::static_name_t("output_cell_element")] = &MakeKeyValueItem;

#if 0 // TODO: Launch designer UI.
    std::auto_ptr<EveDialog> eve_dialog(
        g_test_signals ?
        MakeEveDialog(eve, adam, DictionaryFunctions(), ArrayFunctions(), adam_functions, &ButtonHandler, &SignalTester, row_factory) :
        MakeEveDialog(eve, adam, DictionaryFunctions(), ArrayFunctions(), adam_functions, &ButtonHandler, &foo, row_factory)
    );
    eve_dialog->Run();
#endif

    GUI::GetGUI()->Exit(0);
}

int main(int argc, char* argv[])
{
#if USE_SDL_BACKEND
    MinimalSDLGUI::CustomInit = &CustomInit;
    MinimalSDLMain();
#else
    MinimalOgreGUI::CustomInit = &CustomInit;
    MinimalOgreMain();
#endif
}
