cmake_minimum_required(VERSION 2.6)

message("-- Configuring Tutorials")

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS serialization)

macro (make_gigi_tutorial name)
    add_executable(${name} ${ARGN})
    set_target_properties(${name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${name} GiGiSDL ${Boost_LIBRARIES})
endmacro ()

make_gigi_tutorial(minimal minimal.cpp)
make_gigi_tutorial(controls controls.cpp)
make_gigi_tutorial(serialization serialization.cpp saveload.cpp)
make_gigi_tutorial(adam adam.cpp)