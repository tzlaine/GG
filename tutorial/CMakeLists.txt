cmake_minimum_required(VERSION 2.6)

message("-- Configuring Tutorials")

if (TARGET GiGiSDL)
    set(tutorial_backend SDL)
    message("     Using the SDL driver for the tutorials.")
elseif (TARGET GiGiOgre)
    set(tutorial_backend Ogre)
    message("     Using the Ogre driver for the tutorials.")
else ()
    message("     Neither the SDL nor the Ogre backend was built; skipping tutorials.")
    return()
endif()

if (tutorial_backend STREQUAL SDL)
    include_directories(${SDL_INCLUDE_DIR})
    add_definitions(-DUSE_SDL_BACKEND=1)
elseif (tutorial_backend STREQUAL Ogre)
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

macro (make_gigi_tutorial name)
    if (TARGET GiGiSDL)
        add_executable(${name} ${ARGN})
        set_target_properties(${name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        target_link_libraries(${name} GiGi ${Boost_LIBRARIES})
        if (tutorial_backend STREQUAL SDL)
            target_link_libraries(${name} GiGiSDL ${Boost_LIBRARIES})
        elseif (tutorial_backend STREQUAL Ogre)
            target_link_libraries(${name} GiGiOgre ${Boost_LIBRARIES})
            get_target_property(ois_lib GiGiOgrePlugin_OIS LOCATION)
            if (NOT TARGET copy_ois_lib)
                add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ois_lib}
                                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ois_lib} ${CMAKE_CURRENT_BINARY_DIR}
                                   DEPENDS GiGiOgrePlugin_OIS ${name})
                add_custom_target(copy_ois_lib ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${ois_lib})
            endif ()
        endif ()
        if (1)
            set_target_properties(${name}
                PROPERTIES
                COMPILE_FLAGS "${DEBUG_COMPILE_FLAGS}"
            )
        endif ()
    endif ()
endmacro ()

make_gigi_tutorial(minimal minimal.cpp)
make_gigi_tutorial(controls controls.cpp)
make_gigi_tutorial(eve eve.cpp)

file(COPY ${CMAKE_HOME_DIRECTORY}/tutorial/hatchcircle.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_HOME_DIRECTORY}/test/stop.png DESTINATION ${CMAKE_BINARY_DIR})
