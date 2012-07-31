cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

message("-- Configuring Tests")

if (TARGET GiGiSDL)
    set(test_backend SDL)
    message("     Using the SDL driver for the tests.")
elseif (TARGET GiGiOgre)
    set(test_backend Ogre)
    message("     Using the Ogre driver for the tests.")
else ()
    message("     Neither the SDL nor the Ogre backend was built; skipping tests.")
    return()
endif()

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

if (test_backend STREQUAL SDL)
    include_directories(${SDL_INCLUDE_DIR})
    add_definitions(-DUSE_SDL_BACKEND=1)
elseif (test_backend STREQUAL Ogre)
    include_directories(${OGRE_INCLUDE_DIR})
    link_directories(${OGRE_LIB_DIR})
    add_definitions(-DUSE_SDL_BACKEND=0)
    file(GLOB rendersystem_gl ${OGRE_LIB_DIR}/OGRE/*RenderSystem_GL*)
    file(GLOB plugin_octreescenemanager ${OGRE_LIB_DIR}/OGRE/*Plugin_OctreeSceneManager*)
    file(COPY
             ${rendersystem_gl}
             ${plugin_octreescenemanager}
             ${CMAKE_HOME_DIRECTORY}/src/Ogre/Plugins/OISInput.cfg
             ogre_plugins.cfg
         DESTINATION
             ${CMAKE_CURRENT_BINARY_DIR}
    )
endif ()

macro (make_test_exec name)
    add_executable(${name}-test Test${name}.cpp Action.cpp)
    set_target_properties(${name}-test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${name}-test GiGi ${Boost_LIBRARIES})
    if (test_backend STREQUAL SDL)
        target_link_libraries(${name}-test GiGiSDL ${Boost_LIBRARIES})
    elseif (test_backend STREQUAL Ogre)
        target_link_libraries(${name}-test GiGiOgre ${Boost_LIBRARIES})
        get_target_property(ois_lib GiGiOgrePlugin_OIS LOCATION)
        if (NOT TARGET copy_ois_lib)
            add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ois_lib}
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ois_lib} ${CMAKE_CURRENT_BINARY_DIR}
                               DEPENDS GiGiOgrePlugin_OIS ${name}-test)
            add_custom_target(copy_ois_lib ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${ois_lib})
        endif ()
    endif ()
    if (1)
        set_target_properties(${name}-test
            PROPERTIES
            COMPILE_FLAGS "${DEBUG_COMPILE_FLAGS}"
        )
    endif ()
endmacro ()

macro (add_test_and_data_files name)
    set(suffix "")
    set(args "")
    foreach (arg ${ARGN})
        if (EXISTS ${CMAKE_SOURCE_DIR}/test/${arg})
            get_filename_component(filename_without_path ${arg} NAME)
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${arg}
                           ${CMAKE_CURRENT_BINARY_DIR}/${filename_without_path}
                           COPYONLY)
            set(suffix ${suffix}-${filename_without_path})
            set(args "${args};${filename_without_path}")
        else ()
            set(suffix ${suffix}-${arg})
            set(args "${args};${arg}")
        endif ()
    endforeach ()
    add_test (${name}${suffix} ${CMAKE_BINARY_DIR}/${name}-test ${args})
endmacro ()

make_test_exec(StrongIntegralTypedef)
make_test_exec(StrongSizeTypedef)
make_test_exec(Lexer)
make_test_exec(ExpressionParser)
make_test_exec(ExpressionWriter)
make_test_exec(AdamParser)
make_test_exec(AdamWriter)
make_test_exec(EveParser)
make_test_exec(EveWriter)
make_test_exec(EveLayout)
make_test_exec(DefaultSignalHandler)
make_test_exec(Functions)

add_test_and_data_files(StrongIntegralTypedef)
add_test_and_data_files(StrongSizeTypedef)
add_test_and_data_files(Lexer test_expressions adam_test_expressions_tokens)
add_test_and_data_files(Lexer test_expressions eve_test_expressions_tokens)
add_test_and_data_files(ExpressionParser test_expressions adam)
add_test_and_data_files(ExpressionParser test_expressions eve)
add_test_and_data_files(ExpressionWriter test_expressions adam)
add_test_and_data_files(ExpressionWriter test_expressions eve)
add_test_and_data_files(DefaultSignalHandler)

file(GLOB function_test_adam_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} gg_adam_files/function_test_*.adm)
add_test_and_data_files(Functions gg_eve_files/function_test_dialog.eve ${function_test_adam_test_files})

file(GLOB adam_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} asl_1.0.43_adam_files/*.adm)

foreach (test_file ${adam_test_files})
    add_test_and_data_files(AdamParser ${test_file})
endforeach ()

foreach (test_file ${adam_test_files})
    add_test_and_data_files(AdamWriter ${test_file})
endforeach ()

file(GLOB eve_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} asl_1.0.43_eve_files/*.eve)

foreach (test_file ${eve_test_files})
    add_test_and_data_files(EveParser ${test_file})
endforeach ()

foreach (test_file ${eve_test_files})
    string(REPLACE asl_1.0.43_eve_files asl_1.0.43_adam_files tmp ${test_file})
    string(REPLACE .eve .adm adam_test_file ${tmp})
    add_test_and_data_files(EveWriter ${test_file} ${adam_test_file})
endforeach ()

file(GLOB pngs *.png)
file(COPY EveLayout_vs_Adobe_Begin.html begin_images ${pngs} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

foreach (test_file ${eve_test_files})
    string(REPLACE asl_1.0.43_eve_files asl_1.0.43_adam_files tmp ${test_file})
    string(REPLACE .eve .adm adam_test_file ${tmp})
    if (test_file MATCHES "optional_panel")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} 18,36 18,72 28,141)
    elseif (test_file MATCHES "mini_image_size_2")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} 19,137)
    elseif (test_file MATCHES "mini_image_size_3")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} 19,240)
    elseif (test_file MATCHES "widget_palette")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} 201,42,200,92 201,42,200,114 201,42,200,136 201,42,200,158 201,42,200,180 201,42,200,202 201,42,200,224)
    else ()
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR})
    endif ()
endforeach ()

file(GLOB presentation_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} gg_eve_files/*_presentation_test.eve)

file(COPY PresentationTests.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
foreach (test_file ${presentation_test_files})
    add_test_and_data_files(EveLayout ${test_file} gg_adam_files/presentation_test.adm ${CMAKE_CURRENT_BINARY_DIR})
endforeach ()

file(GLOB signal_binding_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} gg_eve_files/*_signal_binding_test.eve)

file(COPY SignalBindingTests.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
foreach (test_file ${signal_binding_test_files})
    string(REPLACE gg_eve_files gg_adam_files tmp ${test_file})
    string(REPLACE .eve .adm adam_test_file ${tmp})
    if (test_file MATCHES "edit_text_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 208,74 keys{foo} 208,254 keys{foo} 518,74 keys{foo} 518,254 keys{foo} 802,74 keys{f} 802,158 keys{g})
    elseif (test_file MATCHES "radio_button_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 134,68 134,193 134,314 134,437 444,69 444,142)
    elseif (test_file MATCHES "tab_group_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 96,74 96,284 96,436 96,590 424,76 424,178)
    elseif (test_file MATCHES "checkbox_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 28,69 28,193 28,315 28,437 326,69 326,143)
    elseif (test_file MATCHES "popup_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 250,75,250,125 250,225,250,275 250,360,250,410 250,495,250,545 535,75,535,125 535,160,535,210)
    elseif (test_file MATCHES "edit_number_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 256,75,164,75d keys{314} 336,75,336,125 256,325,164,325d keys{314} 336,325,336,375 600,75,500,75d keys{1} 600,160,500,160d keys{2})
    elseif (test_file MATCHES "slider_signal_binding_test")
       add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 35,102,83,102d 35,269,83,269d 35,436,83,436d 304,102,352,102d 304,317,352,317d 594,102,618,102d 594,221,666,221d)
    elseif (test_file MATCHES "button_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 40,72 40,230 40,360 40,485 320,72 320,150)
    elseif (test_file MATCHES "listbox_1_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 215,288 215,439 548,73 548,270 548,422 548,644 891,73 891,95 891,249 891,270 891,422 891,468 891,623 891,644)
    elseif (test_file MATCHES "listbox_2_signal_binding_test")
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals 212,70 212,222 212,370 520,70r 520,222r 520,370r 840,70b 840,222b 840,370b)
    else ()
        add_test_and_data_files(EveLayout ${test_file} ${adam_test_file} ${CMAKE_CURRENT_BINARY_DIR} test_signals)
    endif ()
endforeach ()

file(COPY
         gg_eve_files/eve_only_subdialog_0.eve
         gg_eve_files/eve_only_subdialog_1.eve
         gg_eve_files/eve_only_subdialog_2.eve
         gg_eve_files/eve_only_subdialog_3.eve
         gg_adam_files/adam_and_eve_subdialog.adm
         gg_eve_files/adam_and_eve_subdialog.eve
     DESTINATION
         ${CMAKE_CURRENT_BINARY_DIR}
)
add_test_and_data_files(
    EveLayout
    gg_eve_files/dialog_button_test.eve
    gg_adam_files/dialog_button_test.adm
    ${CMAKE_CURRENT_BINARY_DIR}
    54,70,574,406
    54,147,574,375
    91,226,420,372,574,406
    76,305,420,372,574,375
    294,70,574,406
    294,147,574,375
    331,226,420,372,574,406
    316,305,420,372,574,375
    576,71,597,514
    561,150,581,251,665,252d,597,481
    568,228,655,536
    #556,308, # TODO: need to pick gigi_cursor.png
    619,387,477,403
    619,466,533,403
)
