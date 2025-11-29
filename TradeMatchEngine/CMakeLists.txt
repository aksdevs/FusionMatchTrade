cmake_minimum_required(VERSION 3.16)
project(TradeMatchEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/Order.cpp
    src/OrderBook.cpp
    src/Trader.cpp
    src/MatchingEngine.cpp
)

# Header files
set(HEADERS
    include/Order.hpp
    include/OrderBook.hpp
    include/Trader.hpp
    include/MatchingEngine.hpp
)

# Create library
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# Set include directory for the library
target_include_directories(${PROJECT_NAME} PUBLIC include)

# Main executable
add_executable(${PROJECT_NAME}_main src/main.cpp)
target_link_libraries(${PROJECT_NAME}_main ${PROJECT_NAME})

# Enable testing
enable_testing()

# Find Google Test
find_package(GTest QUIET)

if(GTest_FOUND)
    message(STATUS "Found Google Test")
    
    # Test executable
    set(TEST_SOURCES
        tests/test_order.cpp
        tests/test_trader.cpp
        tests/test_orderbook.cpp
        tests/test_matching_engine.cpp
        tests/test_integration.cpp
    )
    
    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_tests 
        ${PROJECT_NAME}
        GTest::gtest 
        GTest::gtest_main
    )
    
    # Add tests to CTest
    add_test(NAME AllTests COMMAND ${PROJECT_NAME}_tests)
    
    # Custom test target
    add_custom_target(test_verbose
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose
        DEPENDS ${PROJECT_NAME}_tests
    )
else()
    message(WARNING "Google Test not found. To install on macOS: brew install googletest")
    message(WARNING "Tests will not be built.")
endif()

# Performance test (optional, separate from unit tests)
if(GTest_FOUND)
    add_executable(performance_test tests/performance_test.cpp)
    target_link_libraries(performance_test 
        ${PROJECT_NAME}
        GTest::gtest 
        GTest::gtest_main
    )
endif()

# Install targets
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_main
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS} DESTINATION include)

# Documentation target (if Doxygen is available)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    if(EXISTS ${DOXYGEN_IN})
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")

# Custom targets for common operations
add_custom_target(clean_build
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Clean and rebuild"
)

# Benchmark target (if available)
add_custom_target(benchmark
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_main --benchmark
    DEPENDS ${PROJECT_NAME}_main
    COMMENT "Running benchmarks"
)
