cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgre)

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
    set(THIS_LIB_SOURCES OgreGUI.cpp)
    set(THIS_LIB_LINK_LIBS GiGi ${OGRE_LIBRARIES})
    library_all_variants(GiGiOgre)

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${THIS_LIB_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiSDL.pc.in
            ${CMAKE_BINARY_DIR}/GiGiSDL.pc
            @ONLY
        )
    endif ()
endif ()

add_subdirectory(Plugins)
