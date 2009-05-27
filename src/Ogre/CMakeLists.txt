cmake_minimum_required(VERSION 2.6)

if (BUILD_OGRE_DRIVER)
    message("-- Configuring GiGiOgre")
    find_package(OGRE)
    if (OGRE_FOUND)
        include_directories(${OGRE_INCLUDE_DIRS})
        link_directories(${OGRE_LIB_DIR})
    else ()
        set(BUILD_OGRE_DRIVER OFF)
        message("     Warning: Ogre could not be found.  Disabling the Ogre build.")
    endif ()
endif ()

if (BUILD_OGRE_DRIVER)
    set(sources OgreGUI.cpp)

    macro (make_gigi_ogre link_type)
        add_library(GiGiOgre ${link_type} ${sources})
        target_link_libraries(GiGiOgre GiGi ${OGRE_LIBRARIES})
    endmacro ()

    if (BUILD_SHARED)
        make_gigi_ogre(SHARED)
        set_target_properties(GiGiOgre PROPERTIES DEFINE_SYMBOL GIGI_OGRE_EXPORTS)
    endif ()

    if (BUILD_STATIC)
        make_gigi_ogre(STATIC)
        set_target_properties(GiGiOgre PROPERTIES LINK_SEARCH_END_STATIC true)
    endif ()
endif ()

add_subdirectory(Plugins)
