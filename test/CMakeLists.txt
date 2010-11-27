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

macro (add_test_and_data_files name)
    set(suffix "")
    foreach (arg ${ARGN})
        if (EXISTS ${CMAKE_SOURCE_DIR}/test/${arg})
            get_filename_component(filename_without_path ${arg} NAME)
            configure_file(${CMAKE_SOURCE_DIR}/test/${arg} ${CMAKE_BINARY_DIR}/test/${filename_without_path} COPYONLY)
            set(suffix ${suffix}-${filename_without_path})
        else ()
            set(suffix ${suffix}-${arg})
        endif ()
    endforeach ()
    add_test (${name}${suffix} ${CMAKE_BINARY_DIR}/${name}-test ${ARGN})
endmacro ()

make_test_exec(StrongTypedef)
make_test_exec(Lexer)
make_test_exec(ExpressionParser)
make_test_exec(ExpressionWriter)
make_test_exec(AdamParser)

add_test_and_data_files(StrongTypedef)
add_test_and_data_files(Lexer test_expressions adam_test_expressions_tokens)
add_test_and_data_files(Lexer test_expressions eve_test_expressions_tokens)
add_test_and_data_files(ExpressionParser test_expressions adam)
add_test_and_data_files(ExpressionParser test_expressions eve)
add_test_and_data_files(ExpressionWriter test_expressions adam)
add_test_and_data_files(ExpressionWriter test_expressions eve)

file(GLOB adam_test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} asl_1.0.43_adam_files/*.adm)
foreach (test_file ${adam_test_files})
    add_test_and_data_files(AdamParser ${test_file})
endforeach ()
