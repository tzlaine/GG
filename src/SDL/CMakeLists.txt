cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiSDL)

if (BUILD_SDL_DRIVER)
    message("-- Configuring GiGiSDL")
    set(SDL_BUILDING_LIBRARY true)
    find_package(SDL)
    if (SDL_FOUND)
        include_directories(${SDL_INCLUDE_DIR})
    else ()
        set(BUILD_SDL_DRIVER OFF)
        message("     Warning: SDL could not be found.  Disabling the SDL build.")
    endif ()
endif ()

if (BUILD_SDL_DRIVER)
    set(THIS_LIB_SOURCES SDLGUI.cpp)
    set(THIS_LIB_LINK_LIBS GiGi ${SDL_LIBRARY})
    library_all_variants(GiGiSDL)

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${THIS_LIB_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiSDL.pc.in
            ${CMAKE_BINARY_DIR}/GiGiSDL.pc
            @ONLY
        )
    endif ()
endif ()
