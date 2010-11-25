cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

message("-- Configuring Tests")

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

macro (make_test_exec name)
    add_executable(${name}-test Test${name}.cpp)
    set_target_properties(${name}-test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${name}-test GiGi ${Boost_LIBRARIES})
endmacro ()

macro (add_test_with_data_files name)
    set(suffix "")
    foreach (test_data_file ${ARGN})
        configure_file(${CMAKE_SOURCE_DIR}/test/${test_data_file} ${CMAKE_BINARY_DIR}/test/${test_data_file} COPYONLY)
        set(suffix ${suffix}-${test_data_file})
    endforeach ()
    add_test (${name}${suffix} ${CMAKE_BINARY_DIR}/${name}-test ${ARGN})
endmacro ()

make_test_exec(StrongTypedef)
make_test_exec(Lexer)

add_test_with_data_files(StrongTypedef)
add_test_with_data_files(Lexer test_expressions adam_test_expressions_tokens)
add_test_with_data_files(Lexer test_expressions eve_test_expressions_tokens)