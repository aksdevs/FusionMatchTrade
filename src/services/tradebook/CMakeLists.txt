cmake_minimum_required(VERSION 3.10)
project(tradebook_service)

# Use service-local copies of sources and headers via CMAKE_CURRENT_SOURCE_DIR
file(GLOB_RECURSE TRADEBOOK_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE TRADEBOOK_HDRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h"
)

add_library(tradebook STATIC ${TRADEBOOK_SRCS} ${TRADEBOOK_HDRS})
target_include_directories(tradebook PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(tradebook PROPERTIES
    OUTPUT_NAME "tradebook"
)

enable_testing()

# Prefer repository-level tests under <repo-root>/tests/services/tradebook
set(TRADEBOOK_TEST_DIR "")
if(EXISTS "${CMAKE_SOURCE_DIR}/../../tests/services/tradebook")
    set(TRADEBOOK_TEST_DIR "${CMAKE_SOURCE_DIR}/../../tests/services/tradebook")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
    set(TRADEBOOK_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
endif()

if(TRADEBOOK_TEST_DIR)
    find_package(GTest QUIET)
    file(GLOB TRADEBOOK_TEST_SRCS "${TRADEBOOK_TEST_DIR}/*.cpp")
    foreach(test_src ${TRADEBOOK_TEST_SRCS})
        get_filename_component(test_name ${test_src} NAME_WE)
        file(READ ${test_src} TEST_CONTENT)
        string(FIND "${TEST_CONTENT}" "gtest/gtest.h" USE_GTEST)
        if(NOT USE_GTEST EQUAL -1)
            if(GTest_FOUND)
                add_executable(${test_name} ${test_src})
                target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
                target_link_libraries(${test_name} PRIVATE tradebook GTest::gtest_main pthread)
                add_test(NAME ${test_name} COMMAND ${test_name})
            else()
                message(WARNING "Skipping ${test_name}: GoogleTest not found on system")
            endif()
        else()
            add_executable(${test_name} ${test_src})
            target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
            target_link_libraries(${test_name} PRIVATE tradebook pthread)
            add_test(NAME ${test_name} COMMAND ${test_name})
        endif()
    endforeach()
endif()
