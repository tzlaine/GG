cmake_minimum_required(VERSION 2.6)

message("-- Configuring GiGi")

set(THIS_LIB_SOURCES

    AdamParser.cpp
    AlignmentFlags.cpp
    Base.cpp
    BrowseInfoWnd.cpp
    Button.cpp
    ClrConstants.cpp
    Control.cpp
    Cursor.cpp
    DrawUtil.cpp
    DropDownList.cpp
    DynamicGraphic.cpp
    Edit.cpp
    EventFilter.cpp
    EventPump.cpp
    EveGlue.cpp
    EveParser.cpp
    ExpressionParser.cpp
    ExpressionWriter.cpp
    FunctionParser.cpp
    FunctionWriter.cpp
    Filesystem.cpp
    Font.cpp
    GroupBox.cpp
    GUI.cpp
    Layout.cpp
    Lexer.cpp
    ListBox.cpp
    Menu.cpp
    MultiEdit.cpp
    PluginInterface.cpp
    ProgressBar.cpp
    PtRect.cpp
    ReportParseError.cpp
    Scroll.cpp
    StatementParser.cpp
    StatementWriter.cpp
    StaticGraphic.cpp
    StyleFactory.cpp
    TabWnd.cpp
    TextControl.cpp
    Texture.cpp
    Timer.cpp
    UnicodeCharsets.cpp
    Wnd.cpp
    WndEvent.cpp
    ZList.cpp

    dialogs/ColorDlg.cpp
    dialogs/FileDlg.cpp
    dialogs/ThreeButtonDlg.cpp

    ../libltdl/ltdl.c

    adobe/adam.cpp
    adobe/adam_evaluate.cpp
    adobe/adam_function.cpp
    adobe/adam_parser.cpp
    adobe/any_regular.cpp
    adobe/basic_sheet.cpp
    adobe/eve.cpp
    adobe/eve_evaluate.cpp
    adobe/eve_parser.cpp
    adobe/expression_filter.cpp
    adobe/expression_parser.cpp
    adobe/iomanip.cpp
    adobe/iomanip_asl_cel.cpp
    adobe/istream.cpp
    adobe/keyboard.cpp
    adobe/lex_shared.cpp
    adobe/lex_stream.cpp
    adobe/localization.cpp
    adobe/memory.cpp
    adobe/name.cpp
    adobe/parser_shared.cpp
    adobe/string.cpp
    adobe/string_pool.cpp
    adobe/token.cpp
    adobe/typeinfo.cpp
    adobe/virtual_machine.cpp

    adobe/future/source/behavior.cpp
    adobe/future/source/locale.cpp
    adobe/future/source/modal_dialog_interface.cpp
    adobe/future/source/platform_locale_data.cpp
    adobe/future/source/platform_number_formatter_data.cpp

    adobe/future/widgets/sources/edit_number.cpp
    adobe/future/widgets/sources/panel_factory.cpp
    adobe/future/widgets/sources/number_unit.cpp
    adobe/future/widgets/sources/edit_text_factory.cpp
    adobe/future/widgets/sources/checkbox_factory.cpp
    adobe/future/widgets/sources/tab_group_factory.cpp
    adobe/future/widgets/sources/group_factory.cpp
    adobe/future/widgets/sources/image_factory.cpp
    adobe/future/widgets/sources/label_factory.cpp
    adobe/future/widgets/sources/reveal_factory.cpp
    adobe/future/widgets/sources/button_factory.cpp
    adobe/future/widgets/sources/progress_bar_factory.cpp
    adobe/future/widgets/sources/display_number_factory.cpp
    adobe/future/widgets/sources/slider_factory.cpp
    adobe/future/widgets/sources/default_factory.cpp
    adobe/future/widgets/sources/window_server.cpp
    adobe/future/widgets/sources/popup_factory.cpp
    adobe/future/widgets/sources/window_factory.cpp
    adobe/future/widgets/sources/widget_factory_registry.cpp
    adobe/future/widgets/sources/widget_utils.cpp
    adobe/future/widgets/sources/optional_panel_factory.cpp
    adobe/future/widgets/sources/control_button.cpp
    adobe/future/widgets/sources/control_button_factory.cpp
    adobe/future/widgets/sources/widget_factory.cpp
    adobe/future/widgets/sources/button_helper.cpp
    adobe/future/widgets/sources/edit_number_factory.cpp
    adobe/future/widgets/sources/radio_button_factory.cpp
    adobe/future/widgets/sources/virtual_machine_extension.cpp
    adobe/future/widgets/sources/widget_tokens.cpp
    adobe/future/widgets/sources/platform_button.cpp
    adobe/future/widgets/sources/platform_checkbox.cpp
    adobe/future/widgets/sources/platform_display.cpp
    adobe/future/widgets/sources/platform_display_number.cpp
    adobe/future/widgets/sources/platform_edit_number.cpp
    adobe/future/widgets/sources/platform_edit_text.cpp
    adobe/future/widgets/sources/platform_group.cpp
    adobe/future/widgets/sources/platform_image.cpp
    adobe/future/widgets/sources/platform_label.cpp
    adobe/future/widgets/sources/platform_metrics.cpp
    adobe/future/widgets/sources/platform_optional_panel.cpp
    adobe/future/widgets/sources/platform_panel.cpp
    adobe/future/widgets/sources/platform_popup.cpp
    adobe/future/widgets/sources/platform_progress_bar.cpp
    adobe/future/widgets/sources/platform_radio_button.cpp
    adobe/future/widgets/sources/platform_reveal.cpp
    adobe/future/widgets/sources/platform_slider.cpp
    adobe/future/widgets/sources/platform_tab_group.cpp
    adobe/future/widgets/sources/platform_widget_utils.cpp
    adobe/future/widgets/sources/platform_window.cpp

    # GG extensions to the Adobe Eve widget set
    adobe/future/widgets/sources/GG/color_button_factory.cpp
    adobe/future/widgets/sources/GG/display_text_factory.cpp
    adobe/future/widgets/sources/GG/listbox_factory.cpp

    adobe/future/widgets/sources/GG/platform_color_button.cpp
    adobe/future/widgets/sources/GG/platform_display_text.cpp
    adobe/future/widgets/sources/GG/platform_listbox.cpp

    adobe/future/widgets/sources/GG/functions.cpp
)

set_source_files_properties(${CMAKE_HOME_DIRECTORY}/libltdl/ltdl.c PROPERTIES COMPILE_DEFINITIONS HAVE_CONFIG_H=1)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/libltdl/ltdl.h ${CMAKE_HOME_DIRECTORY}/GG/ltdl.h
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/libltdl/config.h ${CMAKE_HOME_DIRECTORY}/GG/ltdl_config.h
)
set_source_files_properties(${CMAKE_HOME_DIRECTORY}/GG/ltdl.h ${CMAKE_HOME_DIRECTORY}/GG/ltdl_config.h PROPERTIES GENERATED true)

set(THIS_LIB_LINK_LIBS ${Boost_LIBRARIES} ${OPENGL_LIBRARIES} ${FREETYPE_LIBRARIES})
if (USE_DEVIL)
    list(APPEND THIS_LIB_LINK_LIBS ${IL_LIBRARY} ${ILUT_LIBRARY})
else ()
    if (JPEG_FOUND)
        list(APPEND THIS_LIB_LINK_LIBS ${JPEG_LIBRARIES})
    endif ()
    if (PNG_FOUND)
        list(APPEND THIS_LIB_LINK_LIBS ${PNG_LIBRARIES})
    endif ()
    if (TIFF_FOUND)
        list(APPEND THIS_LIB_LINK_LIBS ${TIFF_LIBRARIES})
    endif ()
endif ()

if (WIN32)
    add_definitions(-DADOBE_TEST_MICROSOFT_NO_DEPRECATE=0)
    if (MSVC)
        add_definitions(/wd4146 /wd4099 /wd4251 /wd4800 /wd4267 /wd4275
                        /wd4244 /wd4101 /wd4258 /wd4351 /wd4996)
    endif ()
endif ()

library_all_variants(GiGi)

if (WIN32)
    if (TARGET GiGi)
        set_target_properties(GiGi
            PROPERTIES
            COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
            LINK_FLAGS /NODEFAULTLIB:LIBCMT
        )
    elseif (TARGET GiGi_static)
        set_target_properties(GiGi_static
            PROPERTIES
            LINK_FLAGS /NODEFAULTLIB:LIBCMT
        )
    endif ()
endif ()

if (UNIX)
    get_pkg_config_libs(pkg_config_libs ${THIS_LIB_LINK_LIBS})
    configure_file(
        ${CMAKE_HOME_DIRECTORY}/cmake/GiGi.pc.in
        ${CMAKE_BINARY_DIR}/GiGi.pc
        @ONLY
    )
endif ()

add_subdirectory(SDL)
add_subdirectory(Ogre)
