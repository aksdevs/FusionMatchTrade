cmake_minimum_required(VERSION 3.10)
project(matchengine_service)

# Use service-local copies of sources and headers via CMAKE_CURRENT_SOURCE_DIR
file(GLOB_RECURSE MATCHENGINE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE MATCHENGINE_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h"
)

add_library(matchengine STATIC ${MATCHENGINE_SRCS} ${MATCHENGINE_HDRS})
target_include_directories(matchengine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(matchengine PROPERTIES
    OUTPUT_NAME "matchengine"
)

enable_testing()

# Prefer repository-level tests under <repo-root>/tests/services/matchengine
set(MATCHENGINE_TEST_DIR "")
if(EXISTS "${CMAKE_SOURCE_DIR}/../../tests/services/matchengine")
    set(MATCHENGINE_TEST_DIR "${CMAKE_SOURCE_DIR}/../../tests/services/matchengine")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
    set(MATCHENGINE_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
endif()

if(MATCHENGINE_TEST_DIR)
    # Try to find GoogleTest; if not found, skip gtest-based tests (they require gtest_main)
    find_package(GTest QUIET)
    file(GLOB MATCHENGINE_TEST_SRCS "${MATCHENGINE_TEST_DIR}/*.cpp")
    foreach(test_src ${MATCHENGINE_TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        # Read file to detect if it uses GoogleTest
        file(READ ${test_src} TEST_CONTENT)
        string(FIND "${TEST_CONTENT}" "gtest/gtest.h" USE_GTEST)
        if(NOT USE_GTEST EQUAL -1)
            if(GTest_FOUND)
                add_executable(${test_name} ${test_src})
                target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
                target_link_libraries(${test_name} PRIVATE matchengine GTest::gtest_main pthread)
                add_test(NAME ${test_name} COMMAND ${test_name})
            else()
                message(WARNING "Skipping ${test_name}: GoogleTest not found on system")
            endif()
        else()
            add_executable(${test_name} ${test_src})
            target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
            target_link_libraries(${test_name} PRIVATE matchengine pthread)
            add_test(NAME ${test_name} COMMAND ${test_name})
        endif()
    endforeach()
endif()
