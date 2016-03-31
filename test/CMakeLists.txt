include(CTest)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address --coverage -O0 -fno-inline")

# Boost
find_package (Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
    include_directories (${Boost_INCLUDE_DIRS})
    add_definitions (-DBOOST_TEST_DYN_LINK)
    set(ENABLE_TESTS ON)
else()
    set(ENABLE_TESTS OFF)
endif()

#Tests
if (ENABLE_TESTS)
    message(STATUS "Test targets enabled")
    enable_testing()
    set(TEST_TARGETS "")

    function(test_target test_name)
        add_executable(${test_name} EXCLUDE_FROM_ALL ${test_name}.cpp)
        target_link_libraries(${test_name} ${ARGN})
        target_link_libraries(${test_name} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
        add_test (NAME ${test_name} COMMAND ${test_name})
        set(TEST_TARGETS ${TEST_TARGETS} ${test_name} PARENT_SCOPE)
    endfunction(test_target)

    test_target(test_string melano_stringutils)
    test_target(test_time melano_stringutils melano_time)
    test_target(test_time melano_stringutils melano_time)


    # Coverage
    add_custom_target(test_all
        DEPENDS ${TEST_TARGETS}
        COMMENT "Building all tests"
    )

    set(TRACEFILE "${CMAKE_CURRENT_BINARY_DIR}/melanolib_coverage.info")
    set(COVERAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/melanolib_lcov")
    add_custom_target(coverage
        COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ctest -V
        COMMAND lcov -c -d "${CMAKE_CURRENT_BINARY_DIR}" -b "${CMAKE_SOURCE_DIR}" -o ${TRACEFILE} --no-external
        COMMAND genhtml ${TRACEFILE} -o ${COVERAGE_DIR} -p "${CMAKE_SOURCE_DIR}" --demangle-cpp
        DEPENDS ${TEST_TARGETS}
    )

    add_custom_target(coverage_view
        COMMAND xdg-open "${COVERAGE_DIR}/index.html"
    )
else()
    message(STATUS "Test targets disabled")
endif()
