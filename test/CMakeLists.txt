include(CTest)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address --coverage -O0 -fno-inline")

# Boost
find_package (Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS})
    add_definitions (-DBOOST_TEST_DYN_LINK)
    set(BUILD_TESTING ON)
else()
    set(BUILD_TESTING OFF)
endif()

#Tests
if (BUILD_TESTING)
    message(STATUS "Test targets enabled")

    function(test_target test_name)
        add_executable(${test_name} EXCLUDE_FROM_ALL ${test_name}.cpp)
        target_link_libraries(${test_name} ${ARGN})
        target_link_libraries(${test_name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
        add_test (NAME ${test_name} COMMAND ${test_name})
        add_dependencies(tests_run ${test_name})
    endfunction(test_target)

    test_target(test_string melano_stringutils)
    test_target(test_time melano_time)
    test_target(test_geo)
    test_target(test_gsl)
    test_target(test_math)
    test_target(test_functional)

    test_target(test_dynlib melano_dynlib)
    add_library(dynlib_example MODULE dynlib_example.cpp)
    add_dependencies(test_dynlib dynlib_example)
    set_property(TARGET test_dynlib PROPERTY COMPILE_DEFINITIONS LIB_FILE="$<TARGET_FILE:dynlib_example>")

else()
    message(STATUS "Test targets disabled")
endif()
