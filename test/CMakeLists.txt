cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

message("-- Configuring Tests")

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)

macro (make_test name)
    add_executable(${name}-test Test${name}.cpp)
    set_target_properties(${name}-test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${name}-test GiGi ${Boost_LIBRARIES})
    add_test(${name} ${CMAKE_BINARY_DIR}/${name}-test)
    foreach (test_data_file ${ARGN})
        configure_file(${CMAKE_SOURCE_DIR}/test/${test_data_file} ${CMAKE_BINARY_DIR}/test/${test_data_file} COPYONLY)
    endforeach ()
endmacro ()

make_test(StrongTypedef)
make_test(Lexer test_expressions adam_test_expressions_tokens eve_test_expressions_tokens)
