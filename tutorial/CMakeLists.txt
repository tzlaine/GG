cmake_minimum_required(VERSION 2.6)

message("-- Configuring Tutorials")

macro (make_gigi_tutorial name)
    if (TARGET GiGiSDL)
        add_executable(${name} ${ARGN})
        set_target_properties(${name}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        target_link_libraries(${name} GiGiSDL ${Boost_LIBRARIES})
    endif ()
endmacro ()

make_gigi_tutorial(minimal minimal.cpp)
make_gigi_tutorial(controls controls.cpp)
make_gigi_tutorial(eve eve.cpp)

file(COPY ${CMAKE_HOME_DIRECTORY}/tutorial/hatchcircle.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_HOME_DIRECTORY}/test/stop.png DESTINATION ${CMAKE_BINARY_DIR})
